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

class HSSocketState;
class HSSocketStateClosed;
class HSSocketStateListening;
class HSSocketStateConnected;
class HSSocket;

#include "HSByteArray.h"

/**
 * \ingroup HS_PORTAB
 * \brief This class is reimplemented by an object and registered
 * with a socket. It will then receive messages from the socket on
 * the SocketManager worker threads.
 */
class HSSocketMessageReceiver
{
public:
  virtual ~HSSocketMessageReceiver() {}

  virtual void ConnectionAccepted(HSSocket *aSocket) {}
  virtual void DataRead(const HSByteArray &aData) {}
  virtual void ConnectionAttempt(bool aResult) {}
  virtual void Disconnected() {}
};

/**
 * \ingroup HS_PORTAB
 * \brief This class is used to use sockets.
 */
class HSSocket
{
public:
  HSSocket() : mMessageReceiver(0) {}
  virtual ~HSSocket() {}

  static const short Closed = 0;
  static const short Listening = 1;
  static const short Connected = 2;

  virtual void ChangeToState(HSSocketState *aState) = 0;
  
  virtual bool Listen(int aPort) = 0;
  virtual bool Accept(HSSocket *aAcceptSocket) = 0;
  virtual bool Read() = 0;
  virtual bool Send(const HSByteArray &aData) = 0;
  virtual bool Connect(const char *aAddress, int aPort) = 0;

  void RegisterMessageReceiver(HSSocketMessageReceiver *aReceiver) { mMessageReceiver = aReceiver; }
protected:

  HSSocketMessageReceiver *mMessageReceiver;
  HSSocketState *mState;
};
