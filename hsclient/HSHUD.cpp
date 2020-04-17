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
#include "HSHUD.h"
#include "HSFont.h"
#include "HSLog.h"
#include "HSShipStatus.h"
#include "HSSprite.h"
#include "HSTools.h"
#include "HSMatrix3D.h"
#include "HSRadarScreen.h"
#include "HSRenderer.h"
#include "HSSphere.h"

#include <direct.h>
#include "HSGraphicsWindow.h"

#ifdef WIN32
#define sprintf sprintf_s
#endif

HSHUD::HSHUD(HSShipStatus *aShipStatus, HSRenderer *aRenderer)
  : mShipStatus(aShipStatus)
  , mRenderer(aRenderer)
{
  mHeaderFont = new HSFont(mRenderer);
  char buf[256];
  ::_getcwd(buf, 256);
  std::string currentPath = buf;
  std::string headFont = currentPath;
  headFont.append("\\assets\\serpentineHead");
  std::string smallFont = currentPath;
  smallFont.append("\\assets\\serpentine");
  if (!mHeaderFont->LoadFont(headFont)) {
    HSLog() << "Failed to load header font.";
  }
  mSmallFont = new HSFont(mRenderer);
  if (!mSmallFont->LoadFont(smallFont)) {
    HSLog() << "Failed to load small font.";
  }

  std::string uiStatus = std::string(buf).append("\\assets\\hsui_status.png");
  mStatSprite = new HSSprite(mRenderer);
  mStatSprite->Init(uiStatus);
  std::string navStatus = std::string(buf).append("\\assets\\hsui_nav.png");
  mNavSprite = new HSSprite(mRenderer);
  mNavSprite->Init(navStatus);
  std::string engStatus = std::string(buf).append("\\assets\\hsui_eng.png");
  mEngSprite = new HSSprite(mRenderer);
  mEngSprite->Init(engStatus);
  std::string coord = std::string(buf).append("\\assets\\hsui_coord.png");
  mCoordSprite = new HSSprite(mRenderer);
  mCoordSprite->Init(coord);
  std::string hullMeter = std::string(buf).append("\\assets\\hsui_meter_001.png");
  mBarSprite = new HSSprite(mRenderer);
  mBarSprite->Init(hullMeter);
  std::string splash = std::string(buf).append("\\assets\\hsui_splash.png");
  mSplashSprite = new HSSprite(mRenderer);
  mSplashSprite->Init(splash);
  mRadar = new HSRadarScreen(mShipStatus, mRenderer);
}


  //std::string window = std::string(buf).append("\\assets\\window-1.png");
  //mWindow = new HSGraphicsWindow(mRenderer);
  //mWindow->Init(window, mSmallFont);
  //mWindow->SetTitle("Connect");
  //mWindow->SetPosition(HSVector3D(100, 100, 0));
HSHUD::~HSHUD(void)
{
  delete mHeaderFont;
  delete mSmallFont;
  delete mStatSprite;
  delete mNavSprite;
  delete mEngSprite;
  delete mCoordSprite;
  delete mBarSprite;
  delete mSplashSprite;
  delete mRadar;
}

void
HSHUD::UpdatePositions()
{
  mSplashSprite->SetPosition(HSVector3D((int)mWidth / 2 - (int)mSplashSprite->GetWidth() / 2,
    (int)mHeight / 2 - (int)mSplashSprite->GetHeight() / 2, 0));
  mStatSprite->SetPosition(HSVector3D((int)mWidth / 2 - (int)mStatSprite->GetWidth() / 2, 0, 0));
  mNavSprite->SetPosition(HSVector3D((int)mWidth - 440, mHeight - 101, 0));
  mEngSprite->SetPosition(HSVector3D(-2, mHeight - 99, 0));
  mBarSprite->SetPosition(HSVector3D((int)mWidth / 2 - (int)mBarSprite->GetWidth() / 2, 60, 0));
  mRadar->SetLocation((mWidth / 2) - mWidth / 6, (mHeight / 2) - mWidth / 6, mWidth / 14);
}

void
HSHUD::Render()
{
  if (!mShipStatus->mConnected || !mShipStatus->mManned) {
    mSplashSprite->DrawSprite();
  }
  if (!mShipStatus->mConnected) {
    mHeaderFont->RenderText("Not connected", HSVector3D(mWidth / 2, mHeight - 20, 0), HSALIGN_CENTER);
  //mWindow->DrawWindow();
    return;
  }
  if (!mShipStatus->mManned) {
    mHeaderFont->RenderText("No ship manned", HSVector3D(mWidth / 2, mHeight - 20, 0), HSALIGN_CENTER);
    return;
  }
  /* Place windows around the screen with dynamic positioning */
  mStatSprite->DrawSprite();
  mNavSprite->DrawSprite();
  mEngSprite->DrawSprite();
  mCoordSprite->SetPosition(HSVector3D((int)mWidth / 2 - (int)mCoordSprite->GetWidth() / 2, -3, 0));
  mCoordSprite->DrawSprite();
  mCoordSprite->SetPosition(HSVector3D((int)mWidth / 2 - 250 - (int)mCoordSprite->GetWidth() / 2, -3, 0));
  mCoordSprite->DrawSprite();
  mCoordSprite->SetPosition(HSVector3D((int)mWidth / 2 + 250 - (int)mCoordSprite->GetWidth() / 2, -3, 0));
  mCoordSprite->DrawSprite();

  /* Draw hull status bar */
  float hullPerc = (float)mShipStatus->mHullCurrent / mShipStatus->mHullMax;
  mBarSprite->SetTextureCoords(0, 0, (5 + 250 * hullPerc) / mBarSprite->GetWidth(), 1);
  mBarSprite->DrawSprite();

  /* Draw ship name and class */
  std::string head = mShipStatus->mName;
  head.append(" - ").append(mShipStatus->mClass);
  mSmallFont->RenderText(head, HSVector3D(mWidth / 2, 44, 0), HSALIGN_CENTER);
  char tbuf[256];

  /* Draw coordinates for X Y and Z */
  sprintf(tbuf, "%.0f", mShipStatus->mPosition.mX);
  mSmallFont->RenderText(tbuf, HSVector3D(mWidth / 2 - 250, 11, 0), HSALIGN_CENTER);
  sprintf(tbuf, "%.0f", mShipStatus->mPosition.mY);
  mSmallFont->RenderText(tbuf, HSVector3D(mWidth / 2, 11, 0), HSALIGN_CENTER);
  sprintf(tbuf, "%.0f", mShipStatus->mPosition.mZ);
  mSmallFont->RenderText(tbuf, HSVector3D(mWidth / 2 + 250, 11, 0), HSALIGN_CENTER);

  /* Draw Speed and Heading information into the nav panel */
  sprintf(tbuf, "%d km/s", (int)mShipStatus->mVelocity.length());
  mSmallFont->RenderText(tbuf, HSVector3D(mWidth - 222, mHeight - 58, 0), HSALIGN_CENTER);
  sprintf(tbuf, "%d km/s", (int)mShipStatus->mDesiredSpeed);
  mSmallFont->RenderText(tbuf, HSVector3D(mWidth - 222, mHeight - 41, 0), HSALIGN_CENTER);
  sprintf(tbuf, "%dm%d", (int)mShipStatus->mVelocity.HeadingXY(), (int)mShipStatus->mVelocity.HeadingZ());
  mSmallFont->RenderText(tbuf, HSVector3D(mWidth - 70, mHeight - 24, 0), HSALIGN_CENTER);
  sprintf(tbuf, "%dm%d", (int)mShipStatus->mHeading.HeadingXY(), (int)mShipStatus->mHeading.HeadingZ());
  mSmallFont->RenderText(tbuf, HSVector3D(mWidth - 70, mHeight - 58, 0), HSALIGN_CENTER);
  sprintf(tbuf, "%dm%d", (int)mShipStatus->mDesiredHeading.HeadingXY(), (int)mShipStatus->mDesiredHeading.HeadingZ());
  mSmallFont->RenderText(tbuf, HSVector3D(mWidth - 70, mHeight - 41, 0), HSALIGN_CENTER);
  if (mShipStatus->mVelocity.length() > 0) {
    double distToStop = pow(mShipStatus->mVelocity.length() + (mShipStatus->mAcceleration / 2.0), 2) / (2.0 * mShipStatus->mAcceleration);
    sprintf(tbuf, "%s", HSDistanceString(distToStop).c_str());
  } else {
    sprintf(tbuf, "N/A");
  }

  /* Draw eng window info */
  mSmallFont->RenderText(tbuf, HSVector3D(mWidth - 225, mHeight - 24, 0), HSALIGN_CENTER);
  sprintf(tbuf, "%d deg/s", (int)mShipStatus->mTurnRate);
  mSmallFont->RenderText(tbuf, HSVector3D(165, mHeight - 42, 0), HSALIGN_CENTER);
  sprintf(tbuf, "%s/s^2", HSDistanceString(mShipStatus->mAcceleration).c_str());
  mSmallFont->RenderText(tbuf, HSVector3D(165, mHeight - 59, 0), HSALIGN_CENTER);
  sprintf(tbuf, "%s", HSMassString(mShipStatus->mTotalMass).c_str());
  mSmallFont->RenderText(tbuf, HSVector3D(165, mHeight - 25, 0), HSALIGN_CENTER);

  /* Draw contacts */
  unsigned int y = 100;
  foreach (HSSensorContact, contact, mShipStatus->mContacts) {
    if (y > mHeight - 100) {
      break;
    }
    sprintf(tbuf, "%d", contact.contactID);
    mSmallFont->RenderText(tbuf, HSVector3D(100, y, 0));
    mSmallFont->RenderText(contact.name, HSVector3D(200, y, 0));
    HSVector3D diff = mShipStatus->mPosition - contact.position;
    mSmallFont->RenderText(HSDistanceString(diff.length()), HSVector3D(500, y, 0), HSALIGN_RIGHT);
    sprintf(tbuf, "%s/s", HSDistanceString(contact.velocity.length()).c_str());
    mSmallFont->RenderText(tbuf, HSVector3D(600, y, 0));
    y += 16;
  }
  mRenderer->SetTransform(HSTF_PROJECTION, HSMatrix3D::ProjectionFOV((float)M_PI / 4, (float)mWidth / mHeight, 2, 3000));
  double dist = ((float)mWidth / 2) / 0.41421f;
  HSMatrix3D change;
  change._21 = 1;
  change._32 = 1;
  change._13 = 1;
  change._44 = 1;
  mRenderer->SetTransform(HSTF_VIEW, change * HSMatrix3D::Translate(0, 0, (float)dist));
  mRadar->Draw();

}