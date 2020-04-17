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
#include "HSClientWindow.h"
#include "HSConnectDialog.h"

#include "HSShipStatus.h"
#include "HSTextBox.h"
#include "HSButton.h"
#include "HSSocket.h"
#include "HSSocketManager.h"
#include "HSNetworkPacket.h"
#include "HSNetworkPacketMessage.h"
#include "HSNetworkPacketLogin.h"
#include "HSNetworkPacketShipInfo.h"
#include "HSNetworkPacketNavStat.h"
#include "HSNetworkPacketEngStat.h"
#include "HSNetworkPacketContact.h"
#include "HSRenderer.h"
#include "HSThread.h"
#include "HSCondVar.h"
#include "HSMutex.h"
#include "HSVertexBuffer.h"
#include "HSLog.h"
#include "HSHUD.h"
#include "HSSettings.h"

#define ID_CONNECT 100
#define ID_DISCONNECT 101
#define ID_EXIT 102
#define ID_ALWAYSONTOP 103

class HSRenderThread
  : public HSThread
{
public:
  HSRenderThread(HSClientWindow *aClientWindow)
    : mClientWindow(aClientWindow)
  {
  }

  void Run()
  {
    mClientWindow->RenderThread();
  }
private:
  HSClientWindow *mClientWindow;
};

HSClientWindow::HSClientWindow()
  : HSWindow(NULL)
  , mShuttingDown(FALSE)
  , mRenderer(NULL)
  , mRenderThread(NULL)
  , mRenderCondVar(NULL)
  , mHUD(NULL)
  , mWasResized(false)
{
  mTopMenu = AddMenu("File");
  AddMenuItem(mTopMenu, "Connect", ID_CONNECT);
  AddMenuItem(mTopMenu, "Disconnect", ID_DISCONNECT);
  AddMenuItem(mTopMenu, "Always on top", ID_ALWAYSONTOP);
  AddMenuItem(mTopMenu, "-", 0);
  AddMenuItem(mTopMenu, "Exit", ID_EXIT);
  SetMenuItemChecked(mTopMenu, ID_ALWAYSONTOP, GetAlwaysOnTop());
  mConnectDialog = new HSConnectDialog(this);
  mAddress = new HSTextBox(mConnectDialog, 1000);
  mUser = new HSTextBox(mConnectDialog, 1001);
  mPasswd = new HSTextBox(mConnectDialog, 1002, true);
  mConnectButton = new HSButton(mConnectDialog, 1003);
  mSocketManager = new HSSocketManager();
  mSocket = mSocketManager->CreateSocket();
  mSocket->RegisterMessageReceiver(this);
  mShipStatus = new HSShipStatus(this);
}

void
HSClientWindow::OnLoad()
{
  mRenderer = new HSRenderer(this);
  mRenderer->Init();
  mRenderThread = new HSRenderThread(this);
  mRenderThread->Start();
  SetRect(HSRect(100, 900, 100, 500));
  SetName("HSpace Graphical Client");
}

void
HSClientWindow::OnPaint()
{
  if (mRenderCondVar) {
    mRenderCondVar->Signal();
  }
}

void
HSClientWindow::OnResize()
{
  mWasResized = true;
}

void
HSClientWindow::OnKeyPress(HSKeyCode aKey)
{
  if (aKey == HSKC_NUMPLUS) {
    if (mShipStatus->mRadarRange < 1000000000) {
      mShipStatus->mRadarRange *= 10;
    }
  } else if (aKey == HSKC_NUMMINUS) {
    mShipStatus->mRadarRange /= 10;
    if (mShipStatus->mRadarRange <  10) {
      mShipStatus->mRadarRange = 10;
    }
  }
  mRenderCondVar->Signal();
}

HSClientWindow::~HSClientWindow(void)
{
  mShuttingDown = true;
  mRenderCondVar->Signal();
  if (mRenderThread) {
    mRenderThread->Wait();
    delete mRenderThread;
  }
  delete mSocketManager;
}

void
HSClientWindow::MenuItemClicked(int aID)
{
  if (aID == ID_CONNECT) {
    HSSettings settings("HSpace\\HSClient");
    mConnectDialog->Show(); 
    mConnectDialog->SetRect(HSRect(50, 250, 50, 200));
    mAddress->Show();
    mAddress->SetText(settings.GetValue("ADDRESS", "Address"));
    mAddress->SetRect(HSRect(20, 150, 5, 27));
    mUser->Show();
    mUser->SetText(settings.GetValue("USERNAME", "Username"));
    mUser->SetRect(HSRect(20, 150, 35, 57));
    mPasswd->Show();
    mPasswd->SetRect(HSRect(20, 150, 65, 87));
    mConnectButton->Show();
    mConnectButton->SetRect(HSRect(20, 100, 95, 115));
    mConnectButton->SetCaption("Connect");
  } else if (aID == ID_DISCONNECT) {
    delete mSocket;
    mShipStatus->mConnected = false;
    mRenderCondVar->Signal();
    mSocket = mSocketManager->CreateSocket();
    mSocket->RegisterMessageReceiver(this);    
  } else if (aID == ID_ALWAYSONTOP) {
    SetAlwaysOnTop(!GetAlwaysOnTop());  
    SetMenuItemChecked(mTopMenu, aID, GetAlwaysOnTop());
  } else if (aID == ID_EXIT) {
    delete this;
  }
}

void
HSClientWindow::Connect()
{
  HSSettings settings("HSpace\\HSClient");
  settings.SetValue("ADDRESS", mAddress->GetText());
  settings.SetValue("USERNAME", mUser->GetText());
  mSocket->Connect(mAddress->GetText().c_str(), 5463);
}

void
HSClientWindow::ConnectionAttempt(bool aResult)
{
  if (!aResult) {
    delete mSocket;
    mShipStatus->mConnected = false;
    mRenderCondVar->Signal();
    mSocket = mSocketManager->CreateSocket();
    mSocket->RegisterMessageReceiver(this);
#ifdef WIN32
    ::MessageBox((HWND)Handle(), "Connection failed.", "Error", MB_OK);
#endif
  }
  if (aResult) {
    // Connected, start listening to the server for a welcome.
    mSocket->Read();
    mShipStatus->mConnected = true;
  }
}

void
HSClientWindow::DataRead(const HSByteArray &aData)
{
  mBuffer += aData;

  while (mBuffer.size() > 0) {
    size_t readSize;
    HSNetworkPacket *incomingPacket = HSNetworkPacket::ReadPacket(mBuffer, &readSize);
    if (!readSize) {
      break;
    }
    HandlePacket(incomingPacket);
    delete incomingPacket;
    mRenderCondVar->Signal();
    mBuffer.Remove(0, readSize);
  }
  // Get more input.
  mSocket->Read();
}

void
HSClientWindow::HandlePacket(HSNetworkPacket *aPacket)
{
  if (aPacket->mType == HSPT_MESSAGE) {
    HSNetworkPacketMessage *msgPacket = 
      static_cast<HSNetworkPacketMessage*>(aPacket);
    HandleMessage(msgPacket->mMessage);
  } else if (aPacket->mType == HSPT_SHIPINFO) {
    HSNetworkPacketShipInfo *infoPacket =
      static_cast<HSNetworkPacketShipInfo*>(aPacket);
    mShipStatus->mManned = true;
    mShipStatus->mName = infoPacket->mShipName;
    mShipStatus->mClass = infoPacket->mClassName;
  } else if (aPacket->mType == HSPT_NAVSTAT) {
    HSNetworkPacketNavStat *navStatPacket =
      static_cast<HSNetworkPacketNavStat*>(aPacket);
    if (navStatPacket->mContentFlag & 0x1) {
      mShipStatus->mPosition = navStatPacket->mPosition;
    }
    if (navStatPacket->mContentFlag & 0x4) {
      mShipStatus->mHeading = navStatPacket->mHeading;
    }
    if (navStatPacket->mContentFlag & 0x2) {
      mShipStatus->mVelocity = navStatPacket->mVelocity;
    }
    if (navStatPacket->mContentFlag & 0x8) {
      mShipStatus->mDesiredHeading = navStatPacket->mDesiredHeading;
    }
    if (navStatPacket->mContentFlag & 0x10) {
      mShipStatus->mDesiredSpeed = navStatPacket->mDesiredSpeed;
    }
    if (navStatPacket->mContentFlag & 0x40) {
      mShipStatus->mHullCurrent = navStatPacket->mHullCurrent;
    }
    if (navStatPacket->mContentFlag & 0x20) {
      mShipStatus->mHullMax = navStatPacket->mHullMax;
    }
  } else if (aPacket->mType == HSPT_ENGSTAT) {
    HSNetworkPacketEngStat *engStatPacket =
      static_cast<HSNetworkPacketEngStat*>(aPacket);
    mShipStatus->mTurnRate = engStatPacket->mTurnRate;
    mShipStatus->mAcceleration = engStatPacket->mAcceleration;
    mShipStatus->mTotalMass = engStatPacket->mTotalMass;
  } else if (aPacket->mType == HSPT_CONTACT) {
    HSNetworkPacketContact *contactPacket =
      static_cast<HSNetworkPacketContact*>(aPacket);
    mShipStatus->HandleContactPacket(contactPacket);
  }
}

void
HSClientWindow::HandleMessage(int aMsg)
{
  if (aMsg == HSPM_WELCOME) {
    HSNetworkPacketLogin packet;
    packet.mUserName = mUser->GetText();
    packet.mPassword = mPasswd->GetText();
    mSocket->Send(packet.ByteStream());
  } else if (aMsg == HSPM_LOGINDENIED) {
#ifdef WIN32
    ::MessageBox((HWND)Handle(), "Username or password incorrect", "Error", MB_OK);
#endif
    delete mSocket;
    mSocket = mSocketManager->CreateSocket();
    mSocket->RegisterMessageReceiver(this);
    mShipStatus->mConnected = false;
  } else if (aMsg == HSPM_LOGINACCEPTED) {
    SendMessage(HSPM_REQUESTSHIPINFO);
  } else if (aMsg == HSPM_NOCONSOLEMANNED) {
    mShipStatus->mManned = false;
  }
}

void
HSClientWindow::SendMessage(int aMsg)
{
  HSNetworkPacketMessage packet((HSPacketMessage)aMsg);
  mSocket->Send(packet.ByteStream());
}

void
HSClientWindow::Disconnected()
{
  delete mSocket;
  mShipStatus->mConnected = false;
  mRenderCondVar->Signal();
  mSocket = mSocketManager->CreateSocket();
  mSocket->RegisterMessageReceiver(this);
}

void
HSClientWindow::RenderThread()
{
  mRenderCondVar = new HSCondVar();
  mRenderer->CreateDevice();
  HSMutex mtx;

  mHUD = new HSHUD(mShipStatus, mRenderer);
  mHUD->mWidth = mRenderer->GetWidth();
  mHUD->mHeight = mRenderer->GetHeight();
  mHUD->UpdatePositions();

  while (!mShuttingDown) {
    if (mWasResized) {
      mRenderer->Reset();
      mHUD->mWidth = mRenderer->GetWidth();
      mHUD->mHeight = mRenderer->GetHeight();
      mHUD->UpdatePositions();
      mWasResized = false;
    }
    mRenderer->Clear(HSColor(0, 0, 0));
    mRenderer->StartScene();
    mHUD->Render();
    mRenderer->EndScene();
    mRenderer->Present();
    mtx.Lock();
    mRenderCondVar->Wait(&mtx);
    mtx.Release();
  }
}
