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
#include "HSSocket.h"
#include "HSSocketPosix.h"
#include "HSSocketManager.h"
#include "HSSocketManagerPosix.h"
#include "HSTools.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

class HSSocketState
{
public:
  virtual ~HSSocketState() {}

  virtual short GetStateId() = 0;
  virtual bool Listen(HSSocketPosix *aSocket, int aPort) = 0;
  virtual bool Accept(HSSocketPosix *aSocket, HSSocketPosix *aAcceptSocket) = 0;
  virtual bool Read(HSSocketPosix *aSocket) = 0;
  virtual bool Send(HSSocketPosix *aSocket, const HSByteArray& aData) = 0;
  virtual bool Connect(HSSocketPosix *aSocket, const char *aAddress, int aPort) = 0;
  virtual void PollinReceived(HSSocketPosix *aSocket) = 0;
  virtual void PolloutReceived(HSSocketPosix *aSocket) = 0;
};

#define READ_CHUNK_SIZE 8192

//----------------------------------------------------------------------
// State declarations.

#define STATE(_state) (GetHSSocketState##_state())

#define STATEDECL(_state) HSSocketState* GetHSSocketState##_state();


#define STATEDEF_BEGIN(_state)                              \
  class HSSocketState##_state : HSSocketState               \
  {                                                         \
  public:                                                   \
    static HSSocketState* mInstance;                        \
    short GetStateId() { return HSSocket::_state; }  

#define STATEDEF_END(_state)                                        \
  };                                                                \
                                                                    \
  HSSocketState* HSSocketState##_state::mInstance = NULL;           \
  HSSocketState* GetHSSocketState##_state()                         \
  {                                                                 \
    if (!HSSocketState##_state::mInstance)                          \
      HSSocketState##_state::mInstance =                            \
        (HSSocketState*) new HSSocketState##_state;                 \
    return HSSocketState##_state::mInstance;                        \
  }


STATEDECL(Closed)
STATEDECL(Listening)
STATEDECL(Connected)

STATEDEF_BEGIN(Closed)

// How many retries to do on addrinuse.
#define MAX_RETRIES 10

bool Listen(HSSocketPosix *aSocket, int aPort)
{
  if (aSocket->mSocket == -1) {
    aSocket->InitSocket();
  }
  sockaddr_in saServer;
  saServer.sin_family = AF_INET;
  inet_pton(AF_INET, "0.0.0.0", &saServer.sin_addr);
  saServer.sin_port = htons(aPort);

  int rv = bind(aSocket->mSocket,(struct sockaddr*) &saServer, sizeof(saServer));

  if (rv) {
    int retries = 0;
    while(errno == EADDRINUSE && retries < MAX_RETRIES) {
      // Probably an @shutdown/reboot with the old socket lingering.
      // try again in a second!
      sleep(1);
      HSLog() << "Trying to rebind to socket attempt " << retries << ", received: " << errno;
      rv = bind(aSocket->mSocket,(struct sockaddr*) &saServer, sizeof(saServer));
      ++retries;
    }
  }
  if (rv) {
    HSLog() << "Failed to bind socket(" << aSocket->mSocket << ") to port: " << errno;
    return false;
  }
  rv = listen(aSocket->mSocket, SOMAXCONN);
  if (rv) {
    HSLog() << "Failed to start listening on socket: " << errno;
    return false;
  }
  HSLog() << "Now listening on port: " << aPort;
  aSocket->ChangeToState(GetHSSocketStateListening());
  return true;
}

bool Accept(HSSocketPosix *aSocket, HSSocketPosix *aAcceptSocket)
{
  // Not listening, cannot have connections accepted.
  return false;
}

bool Read(HSSocketPosix *aSocket)
{
  // Cannot read from closed socket.
  return false;
}

bool Send(HSSocketPosix *aSocket, const HSByteArray &aData)
{
  // Cannot send to closed socket.
  return false;
}

bool Connect(HSSocketPosix *aSocket, const char *aAddress, int aPort)
{
  /// \todo Not implemented for POSIX.
  return false;
}

void PollinReceived(HSSocketPosix*)
{
  HSLog() << "Pollin reported on closed socket.";
}

void PolloutReceived(HSSocketPosix*)
{
  HSLog() << "Pollout reported on closed socket.";
}

STATEDEF_END(Closed)

STATEDEF_BEGIN(Listening)

bool Listen(HSSocketPosix *aSocket, int aPort)
{
  // Already listening.
  return false;
}


bool Accept(HSSocketPosix *aSocket, HSSocketPosix *aAcceptSocket)
{
  aSocket->mAcceptingSockets.push_back(aAcceptSocket);
  return true;
}

bool Read(HSSocketPosix *aSocket)
{
  // Cannot read from listening socket.
  return false;
}

bool Send(HSSocketPosix *aSocket, const HSByteArray &aData)
{
  // Cannot send to listening socket.
  return false;
}

bool Connect(HSSocketPosix *aSocket, const char *aAddress, int aPort)
{
  // Cannot connect a listening socket.
  return false;
}

void PollinReceived(HSSocketPosix *aSocket)
{
  // In our case a pollin means the socket is ready to accept a connection.
  HSSocketPosix *posix = *aSocket->mAcceptingSockets.begin();
  aSocket->mAcceptingSockets.pop_front();
  
  posix->mSocket = accept(aSocket->mSocket, NULL, NULL);
  fcntl(posix->mSocket, F_SETFL, O_NONBLOCK);
  fcntl(posix->mSocket, F_SETFL, O_ASYNC);
  fcntl(posix->mSocket, F_SETOWN,getpid());
  fcntl(posix->mSocket, F_SETSIG, SIGRTMIN+10);

  posix->mSocketManager->mActiveSockets[posix->mSocket] = posix;
  sSocketAssociation[posix->mSocket] = posix->mSocketManager;
  
  posix->ChangeToState(GetHSSocketStateConnected());
  if (aSocket->mMessageReceiver) {
    aSocket->mMessageReceiver->ConnectionAccepted(posix);
  }
}

void PolloutReceived(HSSocketPosix *aSocket)
{
  HSLog() << "Pollout received on listening socket.";
}

STATEDEF_END(Listening)

STATEDEF_BEGIN(Connected)

bool Listen(HSSocketPosix *aSocket, int aPort)
{
  // Can't go from connected > listening.
  return false;
}


bool Accept(HSSocketPosix *aSocket, HSSocketPosix *aAcceptSocket)
{
  return false;
}

void Accepted(HSSocketPosix *aSocket, HSSocketPosix *aAccepted)
{
  HSLog() << "Inconsistent state: accepted reported on connected socket.";
  return;
}

bool Read(HSSocketPosix *aSocket)
{
  aSocket->mWantRead = true;
  aSocket->mSocketManager->PostActivity(aSocket->mSocket);
  
  return true;
}

bool Send(HSSocketPosix *aSocket, const HSByteArray &aData)
{
  aSocket->mSendBufferMutex.Lock();
  aSocket->mSendBuffer += aData;
  aSocket->mSendBufferMutex.Release();
  aSocket->mSocketManager->PostActivity(aSocket->mSocket);
  return true;
}

bool Connect(HSSocketPosix *aSocket, const char *aAddress, int aPort)
{
  // Cannot connect an already connected socket.
  return false;
}

void PollinReceived(HSSocketPosix *aSocket)
{
  char tbuf[READ_CHUNK_SIZE];

  if (aSocket->mWantRead) {
    int bytesReceived = recv(aSocket->mSocket, tbuf, READ_CHUNK_SIZE, 0);
  
    if (bytesReceived > 0) {
      aSocket->mWantRead = false;
      if (aSocket->mMessageReceiver) {
	aSocket->mMessageReceiver->DataRead(HSByteArray(tbuf, bytesReceived));
      }
    }
  }
}

void PolloutReceived(HSSocketPosix *aSocket)
{
  aSocket->mSendBufferMutex.Lock();
  if (aSocket->mSendBuffer.size() > 0) {
    int bytesSent = send(aSocket->mSocket, aSocket->mSendBuffer.constData(), aSocket->mSendBuffer.size(), 0);
    aSocket->mSendBuffer.Remove(0, bytesSent);
  }
  aSocket->mSendBufferMutex.Release();
}

STATEDEF_END(Connected)

HSSocketPosix::HSSocketPosix(HSSocketManagerPriv *aSocketManager)
  : mWantRead(false)
{
  mState = GetHSSocketStateClosed();
  mSocketManager = aSocketManager;
  mSocket = -1;
}

HSSocketPosix::~HSSocketPosix(void)
{
  /// \todo Should really grab usage mutex here in case
  /// case we are deleted from another thread than a worker
  /// thread working on us. Would need to implement cross
  /// platform re-entrant mutex though.
  mSocketManager->mActivityMutex.Lock();
  mSocketManager->mActiveSockets.erase(mSocket);
  sSocketAssociation.erase(mSocket);
  close(mSocket);
  mSocketManager->mActivityMutex.Release();
}

void
HSSocketPosix::InitSocket()
{
  mSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (mSocket == -1) {
    HSLog() << "Failed to create socket: " << errno;
  }
  // Enable reuse of the address so that an @shutdown/reboot will
  // not cause trouble because of the listening socket being in
  // TIME_WAIT.
  int on = 1;
  setsockopt( mSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

  fcntl(mSocket, F_SETFL, O_NONBLOCK);
  fcntl(mSocket, F_SETFL, O_ASYNC);
  fcntl(mSocket, F_SETOWN,getpid());
  fcntl(mSocket, F_SETSIG, SIGRTMIN+10);

  mSocketManager->mActivityMutex.Lock();
  mSocketManager->mActiveSockets[mSocket] = this;
  mSocketManager->mActivityMutex.Release();
  sSocketAssociation[mSocket] = mSocketManager;
}

void
HSSocketPosix::ChangeToState(HSSocketState *aState)
{
  mState = aState;
}

bool
HSSocketPosix::Listen(int aPort)
{
  return mState->Listen(this, aPort);
}

bool
HSSocketPosix::Accept(HSSocket *aAcceptSocket)
{
  return mState->Accept(this, static_cast<HSSocketPosix*>(aAcceptSocket));
}

void
HSSocketPosix::Disconnected()
{
  mSocketManager->mActiveSockets.erase(mSocket);
  sSocketAssociation.erase(mSocket);
  ChangeToState(GetHSSocketStateClosed());
  if (mMessageReceiver) {
    mMessageReceiver->Disconnected();
  }
}

bool
HSSocketPosix::Read()
{
  return mState->Read(this);
}

bool
HSSocketPosix::Send(const HSByteArray &aData)
{
  return mState->Send(this, aData);
}

bool
HSSocketPosix::Connect(const char *aAddress, int aPort)
{
  return mState->Connect(this, aAddress, aPort);
}

void
HSSocketPosix::PollinReceived()
{
  mState->PollinReceived(this);
}

void
HSSocketPosix::PolloutReceived()
{
  mState->PolloutReceived(this);
}
