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
#include <winsock2.h>

#include "HSSocketManager.h"
#include "HSSocketWin32.h"
#include "HSTools.h"
#include "HSThread.h"

class HSSocketManagerPriv
{
public:
  HSSocketManagerPriv() : mIOCP(NULL) {}

  HANDLE mIOCP;
};

class HSWorkerThread
  : public HSThread
{
public:
  HSWorkerThread(HANDLE aCompletionPort)
    : mCompletionPort(aCompletionPort)
  {
  }

  void Run()
  {
    DWORD bytesTransferred;
    ULONG_PTR param;
    OV *overlapped;
    while (true) {
      int rv = ::GetQueuedCompletionStatus(mCompletionPort,
      &bytesTransferred,
      &param,
      (OVERLAPPED**)&overlapped,
      INFINITE);

      if (rv && overlapped && overlapped->mOpType == SO_WSARECV
        && !bytesTransferred) {
        rv = 0;
      }

      if (rv) {
        if (overlapped->mOpType == SO_ACCEPTEX) {
          ((HSSocketWin32*)param)->Accepted(overlapped->mAcceptedSocket);
        } else if (overlapped->mOpType == SO_WSARECV) {
          ((HSSocketWin32*)param)->Received(HSByteArray(overlapped->mBuffer.buf, bytesTransferred));
        } else if (overlapped->mOpType == SO_CONNECTEX) {
          ((HSSocketWin32*)param)->Connected();
        }
      } else if (!rv && overlapped) {
        if (overlapped->mOpType == SO_CONNECTEX) {
          if  (((HSSocketWin32*)param)->mMessageReceiver) {
            ((HSSocketWin32*)param)->mMessageReceiver->ConnectionAttempt(false);
          }
        } else {
          ((HSSocketWin32*)param)->Disconnected();
        }
      } else {
        break;
      }
      delete overlapped;
    }

  }

private:
  HANDLE mCompletionPort;
};

HSSocketManager::HSSocketManager()
  : p(NULL)
{
  p = new HSSocketManagerPriv();
  if (!p) {
    HSLog() << "Failed to create HSSocketManagerPriv.";
  }

  p->mIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE,
    NULL,
    (ULONG_PTR)0,
    0);
  if (!p->mIOCP) {
    HSLog() << "Failed to create IO completion port: " << ::GetLastError();
    return;
  }

  SYSTEM_INFO sysInfo;
  ::GetSystemInfo(&sysInfo);
  mNumWorkers = sysInfo.dwNumberOfProcessors;
  mWorkerThreads = new HSWorkerThread*[sysInfo.dwNumberOfProcessors];

  for (unsigned int i = 0; i < sysInfo.dwNumberOfProcessors; i++) {
    mWorkerThreads[i] = new HSWorkerThread(p->mIOCP);
    mWorkerThreads[i]->Start();
  }

  WSADATA wsData;

  int success = WSAStartup(MAKEWORD(2,2), &wsData);

  if (success) {
    HSLog() << "Failed to initialise WinSock";
    return;
  } else {
    HSLog() << "Succesfully initialised WinSock " << LOBYTE(wsData.wVersion) << "." << HIBYTE(wsData.wVersion);
  }
}

HSSocketManager::~HSSocketManager()
{
  ::CloseHandle(p->mIOCP);
  for (int i = 0; i < mNumWorkers; i++) {
    mWorkerThreads[i]->Wait();
    delete mWorkerThreads[i];
  }
  WSACleanup();
  delete [] mWorkerThreads;
}

HSSocket*
HSSocketManager::CreateSocket()
{
  SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (socket == INVALID_SOCKET) {
    HSLog() << "Error creating socket: " << ::WSAGetLastError();
    return NULL;
  }

  HSSocket *sock = new HSSocketWin32(socket, p->mIOCP);
  HANDLE iocp = ::CreateIoCompletionPort((HANDLE)socket, p->mIOCP, (ULONG_PTR)sock, 0);

  return sock;
}
