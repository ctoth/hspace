/**
 *
 * Hemlock Space 5 (HSpace 5)
 * Copyright (c) 2009, Bas Schouten and Shawn Sagady
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 * 
 *    * Redistribution in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the HSpace 5 Development Team nor the names
 *      of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS  SOFTWARE IS PROVIDED BY THE HSPACE DEVELOPMENT TEAM AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FINTESS FOR A PARTICULAR
 * PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  COPYRIGHT  OWNERS OR
 * CONTRIBUTORS  BE  LIABLE  FOR  ANY  DIRECT,  INDIRECT, INCIDENTAL, SPECIAL
 * EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT  LIMITED TO,
 * PRODUCEMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR  BUSINESS  INTERUPTION)  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER  IN  CONTRACT,  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)  ARISING  IN  ANY  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Original author(s):
 *   Bas Schouten
 *
 */
#include "HSSocketManager.h"
#include "HSSocketManagerPosix.h"
#include "HSSocketPosix.h"
#include "HSTools.h"
#include "HSThread.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <poll.h>

std::map<int,HSSocketManagerPriv*> sSocketAssociation;

void SigIOAction(int aSignal, siginfo_t *aSigInfo, void *aContext)
{
  if (sSocketAssociation.count(aSigInfo->si_fd) > 0) {
    HSSocketManagerPriv *priv = sSocketAssociation[aSigInfo->si_fd];
    priv->PostActivity(aSigInfo->si_fd);
  }
}

class HSWorkerThread
  : public HSThread
{
public:
  HSWorkerThread(HSSocketManagerPriv* aSocketManagerPriv)
    : p(aSocketManagerPriv)
  {
  }

  void Run()
  {
    while(true) {
      p->mActivityMutex.Lock();
      if (!p->mSocketTasks.size() && !p->mShuttingDown) {
	p->mActivityCondvar.Wait(&p->mActivityMutex);
      }
      if (p->mShuttingDown) {
	break;
      }
      int sock = *p->mSocketTasks.begin();
      if (!p->mActiveSockets.count(sock)) {
	p->mSocketTasks.pop_front();
	p->mActivityMutex.Release();
	continue;
      }
      HSSocketPosix *socket = p->mActiveSockets[sock];
      if (!socket->mSocketUsageMutex.TryLock()) {
	// Socket is in use. Try task again later.
	/// \todo This will cause thread free-spinning for a while
	/// if waiting for another thread doing a long lasting socket
	/// operation and this being the only task on the queue.
	p->mActivityMutex.Release();
	continue;
      }
      p->mSocketTasks.pop_front();
      p->mActivityMutex.Release();
      pollfd pollnfd;
      memset(&pollnfd, 0, sizeof(pollfd));
      pollnfd.fd = sock;
      pollnfd.events = POLLIN | POLLOUT;
      poll(&pollnfd, 1, 0);
      if (pollnfd.revents & POLLIN &&
	  pollnfd.revents & POLLHUP) {
	socket->Disconnected();
      } else {
	if (pollnfd.revents & POLLIN) {
	  socket->PollinReceived();
	  pollnfd.revents ^= POLLIN;
	}
	// Double check if received data left socket alive.
	if (p->mActiveSockets.count(sock)) {
	  if (pollnfd.revents & POLLOUT) {
	    socket->PolloutReceived();
	    pollnfd.revents ^= POLLOUT;
	  }
	}
      }
      // Check if the socket still exits after executed actions.
      if (p->mActiveSockets.count(sock)) {
	socket->mSocketUsageMutex.Release();
      }
    }
  }

private:
  HSSocketManagerPriv *p;
};

void
HSSocketManagerPriv::PostActivity(int aSocket)
{
  mActivityMutex.Lock();
  mSocketTasks.push_back(aSocket);
  mActivityCondvar.Signal();
  mActivityMutex.Release();
}

HSSocketManager::HSSocketManager()
  : p(NULL)
{
  p = new HSSocketManagerPriv();
  p->mShuttingDown = false;

  // Single worker thread since there is some race
  // conditions we haven't quite dealt with.
  mNumWorkers = 1;
  mWorkerThreads = new HSWorkerThread*[mNumWorkers];

  for (int i = 0; i < mNumWorkers; i++) {
    mWorkerThreads[i] = new HSWorkerThread(p);
    mWorkerThreads[i]->Start();
  }
  struct sigaction sigAction;
  sigAction.sa_handler = NULL;
  sigAction.sa_sigaction = SigIOAction;
  sigemptyset (&sigAction.sa_mask);
  sigAction.sa_flags = SA_SIGINFO;
  sigAction.sa_restorer = NULL;
  sigaction(SIGRTMIN+10, &sigAction, NULL);
}

HSSocketManager::~HSSocketManager()
{
  p->mActivityMutex.Lock();
  p->mShuttingDown = true;
  p->mActivityCondvar.Signal();
  p->mActivityMutex.Release();
  for (int i = 0; i < mNumWorkers; i++) {
    mWorkerThreads[i]->Wait();
    delete mWorkerThreads[i];
  }
  delete [] mWorkerThreads;
}

HSSocket*
HSSocketManager::CreateSocket()
{
  HSSocketPosix *sock = new HSSocketPosix(p);

  return sock;
}
