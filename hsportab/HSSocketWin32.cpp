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
#include "HSSocketWin32.h"
#include "HSTools.h"

#include <mswsock.h>
#include <Ws2tcpip.h>

class HSSocketState
{
public:
  virtual ~HSSocketState() {}

  virtual short GetStateId() = 0;
  virtual bool Listen(HSSocketWin32 *aSocket, int aPort) = 0;
  virtual bool Accept(HSSocketWin32 *aSocket, HSSocketWin32 *aAcceptSocket) = 0;
  virtual void Accepted(HSSocketWin32 *aSocket, HSSocketWin32 *aAccepted) = 0;
  virtual void Received(HSSocketWin32 *aSocket, const HSByteArray &aData) = 0;
  virtual bool Read(HSSocketWin32 *aSocket) = 0;
  virtual bool Send(HSSocketWin32 *aSocket, const HSByteArray& aData) = 0;
  virtual bool Connect(HSSocketWin32 *aSocket, const char *aAddress, int aPort) = 0;
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

bool Listen(HSSocketWin32 *aSocket, int aPort)
{
  sockaddr_in saServer;
  saServer.sin_family = AF_INET;
  saServer.sin_addr.S_un.S_addr = INADDR_ANY;
  saServer.sin_port = htons(aPort);

  int rv = bind(aSocket->mSocket,(SOCKADDR*) &saServer, sizeof(saServer));

  if (rv) {
    HSLog() << "Failed to bind socket to port: " << ::WSAGetLastError();
    return false;
  }
  rv = listen(aSocket->mSocket, SOMAXCONN);
  if (rv) {
    HSLog() << "Failed to start listening on socket: " << ::WSAGetLastError();
    return false;
  }
  HSLog() << "Now listening on port: " << aPort;

  aSocket->ChangeToState(GetHSSocketStateListening());
  return true;
}

bool Accept(HSSocketWin32 *aSocket, HSSocketWin32 *aAcceptSocket)
{
  // Not listening, cannot have connections accepted.
  return false;
}

void Accepted(HSSocketWin32 *aSocket, HSSocketWin32 *aAccepted)
{
  // We shouldn't get these, ignore but log.
  HSLog() << "Accepted completion reported on closed socket.";
}

void Received(HSSocketWin32 *aSocket, const HSByteArray &aData)
{
  HSLog() << "Data received reported on closed socket.";
}

bool Read(HSSocketWin32 *aSocket)
{
  // Cannot read from closed socket.
  return false;
}

bool Send(HSSocketWin32 *aSocket, const HSByteArray &aData)
{
  // Cannot send to closed socket.
  return false;
}

bool Connect(HSSocketWin32 *aSocket, const char *aAddress, int aPort)
{
  int rv;
  char tbuf[16];
  sprintf(tbuf, "%d", aPort);

  addrinfo *addr;
  
  rv = ::getaddrinfo(aAddress, tbuf, NULL, &addr);

  if (FAILED(rv) || !addr) {
    HSLog() << "Failed to get address info: " << rv << " : " << ::WSAGetLastError();
    return false;
  }

  sockaddr_in saServer;
  saServer.sin_family = AF_INET;
  saServer.sin_addr.S_un.S_addr = INADDR_ANY;
  saServer.sin_port = 0;

  rv = bind(aSocket->mSocket,(SOCKADDR*) &saServer, sizeof(saServer));

  if (rv) {
    HSLog() << "Failed to bind socket to port: " << ::WSAGetLastError();
    return false;
  }

  // Load ConnectEx
  GUID GuidConnectEx = WSAID_CONNECTEX;
  LPFN_CONNECTEX lpfnConnectEx = NULL;
  DWORD dwBytes = 0;

  rv = WSAIoctl(
    aSocket->mSocket,
    SIO_GET_EXTENSION_FUNCTION_POINTER,
    &GuidConnectEx,
    sizeof(GuidConnectEx),
    &lpfnConnectEx,
    sizeof(lpfnConnectEx),
    &dwBytes,
    NULL,
    NULL
    );

  if (FAILED(rv)) {
    HSLog() << "Failed to get pointer to connectex.";
    ::freeaddrinfo(addr);
    return false;
  }
    
  OV *ol = new OV(SO_CONNECTEX);
  ol->mBuffer.buf = (char*)new sockaddr;
  memcpy(ol->mBuffer.buf, addr->ai_addr, sizeof(sockaddr));
  rv = lpfnConnectEx(aSocket->mSocket, (sockaddr*)ol->mBuffer.buf, sizeof(sockaddr), NULL, 0, NULL, ol);

  if (!rv) {
    if (::WSAGetLastError() != ERROR_IO_PENDING) {
      HSLog() << "Call to connectex failed: " << rv << " : " << ::WSAGetLastError();
      delete ol;
      ::freeaddrinfo(addr);
      return false;
    }
  }

  ::freeaddrinfo(addr);
  return true;
}

STATEDEF_END(Closed)

STATEDEF_BEGIN(Listening)

bool Listen(HSSocketWin32 *aSocket, int aPort)
{
  // Already listening.
  return false;
}


bool Accept(HSSocketWin32 *aSocket, HSSocketWin32 *aAcceptSocket)
{
  // Create the new OV object. This will identify the operation uniquely
  // to the worker thread. This needs to be deleted by the worker thread.
  OV *ol = new OV(SO_ACCEPTEX);
  ol->mBuffer.buf = new char[(sizeof(sockaddr_in) + 16) * 2];
  ol->mBuffer.len = (sizeof(sockaddr_in) + 16) * 2;

  memset(ol->mBuffer.buf, 0, ol->mBuffer.len);
  ol->mAcceptedSocket = aAcceptSocket;

  DWORD bytesReceived = 0;

  BOOL rv = ::AcceptEx(aSocket->mSocket, aAcceptSocket->mSocket,
    ol->mBuffer.buf, 0, 
    sizeof(sockaddr_in) + 16,
    sizeof(sockaddr_in) + 16,
    &bytesReceived,
    ol);

  return true;
}

void Accepted(HSSocketWin32 *aSocket, HSSocketWin32 *aAccepted)
{
  HSLog() << "Incoming connection accepted.";
  aAccepted->ChangeToState(GetHSSocketStateConnected());
  if (aSocket->mMessageReceiver) {
    aSocket->mMessageReceiver->ConnectionAccepted(aAccepted);
  }
  return;
}

void Received(HSSocketWin32 *aSocket, const HSByteArray &aData)
{
  HSLog() << "Data received reported on closed socket.";
}

bool Read(HSSocketWin32 *aSocket)
{
  // Cannot read from listening socket.
  return false;
}

bool Send(HSSocketWin32 *aSocket, const HSByteArray &aData)
{
  // Cannot send to listening socket.
  return false;
}

bool Connect(HSSocketWin32 *aSocket, const char *aAddress, int aPort)
{
  // Cannot connect a listening socket.
  return false;
}

STATEDEF_END(Listening)

STATEDEF_BEGIN(Connected)

bool Listen(HSSocketWin32 *aSocket, int aPort)
{
  // Can't go from connected > listening.
  return false;
}


bool Accept(HSSocketWin32 *aSocket, HSSocketWin32 *aAcceptSocket)
{
  return false;
}

void Accepted(HSSocketWin32 *aSocket, HSSocketWin32 *aAccepted)
{
  HSLog() << "Inconsistent state: accepted reported on connected socket.";
  return;
}

void Received(HSSocketWin32 *aSocket, const HSByteArray &aData)
{
  if (aSocket->mMessageReceiver) {
    aSocket->mMessageReceiver->DataRead(aData);
  }
}

bool Read(HSSocketWin32 *aSocket)
{
  OV *ol = new OV(SO_WSARECV);
  ol->mBuffer.buf = new char[READ_CHUNK_SIZE];
  ol->mBuffer.len = READ_CHUNK_SIZE;
  DWORD flags = MSG_PARTIAL;
  int rv = ::WSARecv(aSocket->mSocket, &ol->mBuffer, 1, NULL, &flags, ol, NULL);

  if (FAILED(rv)) {
    if (::WSAGetLastError() != WSA_IO_PENDING) {
      HSLog() << "WSARecv call failed: " << rv << " : " << ::WSAGetLastError();
      return false;
    }
  }
  return true;
}

bool Send(HSSocketWin32 *aSocket, const HSByteArray &aData)
{
  OV *ol = new OV(SO_WSASEND);
  ol->mBuffer.buf = new char[aData.size()];
  memcpy(ol->mBuffer.buf, aData.constData(), aData.size());
  ol->mBuffer.len = (u_long)aData.size();

  int rv = ::WSASend(aSocket->mSocket, &ol->mBuffer, 1, NULL, 0, ol, NULL);
  if (FAILED(rv)) {
    if (::WSAGetLastError() != WSA_IO_PENDING) {
      HSLog() << "WSASend call failed: " << rv << " : " << ::WSAGetLastError();
      return false;
    }
  }
  return false;
}

bool Connect(HSSocketWin32 *aSocket, const char *aAddress, int aPort)
{
  // Cannot connect an already connected socket.
  return false;
}

STATEDEF_END(Connected)

HSSocketWin32::HSSocketWin32(SOCKET aSocket, HANDLE aIOCP)
  : mSocket(aSocket)
  , mIOCP(aIOCP)
{
  mState = GetHSSocketStateClosed();
}

HSSocketWin32::~HSSocketWin32(void)
{
  closesocket(mSocket);
}

void
HSSocketWin32::ChangeToState(HSSocketState *aState)
{
  mState = aState;
}

bool
HSSocketWin32::Listen(int aPort)
{
  return mState->Listen(this, aPort);
}

bool
HSSocketWin32::Accept(HSSocket *aAcceptSocket)
{
  return mState->Accept(this, static_cast<HSSocketWin32*>(aAcceptSocket));
}

void
HSSocketWin32::Accepted(HSSocketWin32 *aAccepted)
{
  mState->Accepted(this, aAccepted);
}

void
HSSocketWin32::Connected()
{
  ChangeToState(GetHSSocketStateConnected());
  if (mMessageReceiver) {
    mMessageReceiver->ConnectionAttempt(true);
  }
}

void
HSSocketWin32::Received(const HSByteArray &aData)
{
  mState->Received(this, aData);
}

void
HSSocketWin32::Disconnected()
{
  ChangeToState(GetHSSocketStateClosed());
  if (mMessageReceiver) {
    mMessageReceiver->Disconnected();
  }
}

bool
HSSocketWin32::Read()
{
  return mState->Read(this);
}

bool
HSSocketWin32::Send(const HSByteArray &aData)
{
  return mState->Send(this, aData);
}

bool
HSSocketWin32::Connect(const char *aAddress, int aPort)
{
  return mState->Connect(this, aAddress, aPort);
}
