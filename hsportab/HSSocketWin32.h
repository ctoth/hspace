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
#pragma once

#include "HSSocket.h"
#include "HSByteArray.h"

#include <winsock2.h>

enum OpType
{
  SO_ACCEPTEX = 100,
  SO_WSARECV,
  SO_WSASEND,
  SO_CONNECTEX
};

class HSSocketWin32;
class HSWorkerThread;

class OV : public OVERLAPPED
{
public:
  OV(OpType aOpType) : mOpType(aOpType) 
  {
    Internal = 0;
    InternalHigh = 0;
    Pointer = 0;
    hEvent = (HANDLE)0; 
    mBuffer.buf = NULL;
    mBuffer.len = 0;
  }
  ~OV()
  {
    delete mBuffer.buf;
  }
  OpType mOpType;
  WSABUF mBuffer;
  HSSocketWin32 *mAcceptedSocket;
};

class HSSocketWin32
  : public HSSocket
{
public:
  HSSocketWin32(SOCKET aSocket, HANDLE aIOCP);
  ~HSSocketWin32(void);

  void ChangeToState(HSSocketState *aState);

  bool Listen(int aPort);
  bool Accept(HSSocket *aAcceptSocket);
  bool Read();
  bool Send(const HSByteArray &aData);
  bool Connect(const char *aAddress, int aPort);

  void Accepted(HSSocketWin32 *aAccepted);
  void Connected();
  void Received(const HSByteArray &aData);
  void Disconnected();

private:
  friend class HSSocketStateClosed;
  friend class HSSocketStateListening;
  friend class HSSocketStateConnected;
  friend class HSWorkerThread;

  SOCKET mSocket;
  HANDLE mIOCP;
};
