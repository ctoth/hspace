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
#include "HSRadarScreen.h"
#include "HSRenderer.h"
#include "HSVertexBuffer.h"
#include "HSFont.h"
#include "HSTools.h"
#include "HSShipStatus.h"
#include "HSSphere.h"

#include <direct.h>

HSRadarScreen::HSRadarScreen(HSShipStatus *aShipStatus, HSRenderer *aRenderer)
  : mX(0)
  , mY(0)
  , mAxis(NULL)
  , mFont(NULL)
  , mShipStatus(aShipStatus)
  , mRenderer(aRenderer)
{
  mContact = new HSSphere(mRenderer);
  mContact->SetColor(HSColor(0, 190, 0));
  mBounds = new HSSphere(mRenderer);
  HSColor boundsColor(100, 100, 200);
  boundsColor._a = 80;
  mBounds->SetColor(boundsColor);
}

HSRadarScreen::~HSRadarScreen(void)
{
}

void
HSRadarScreen::SetLocation(int aX, int aY, int aRadius)
{
  mX = aX;
  mY = aY;
  mRadius = aRadius;
  mBounds->SetRadius(mRadius);
  mContact->SetRadius(2.0f * aRadius / 50.0);
  delete mAxis;

  mAxis = mRenderer->CreateVertexBuffer(HSVT_COLORED, 6);
  
  if (!mAxis) {
    return;
  }

  HSVertexColored *vertices = 
    reinterpret_cast<HSVertexColored*>(mAxis->Lock());

  HSColor color(100, 100, 200);
  for (int i = 0; i < 6; i++) {
    vertices[i].color = color.ARGB();
  }
  vertices[0].v = HSVector3D(-mRadius, 0, 0).RenderVector();
  vertices[1].v = HSVector3D(mRadius, 0, 0).RenderVector();
  vertices[2].v = HSVector3D(0, -mRadius, 0).RenderVector();
  vertices[3].v = HSVector3D(0, mRadius, 0).RenderVector();
  vertices[4].v = HSVector3D(0, 0, -mRadius).RenderVector();
  vertices[5].v = HSVector3D(0, 0, mRadius).RenderVector();

  mAxis->Unlock();
}

void
HSRadarScreen::Draw()
{
  if (!mAxis) {
    // Probably minimized or something.
    return;
  }
  if (!mFont) {
    char buf[256];
    ::_getcwd(buf, 256);
    std::string currentPath = buf;
    std::string smallFont = currentPath;
    smallFont.append("\\assets\\serpentine");
    mFont = new HSFont(mRenderer);
    if (!mFont->LoadFont(smallFont)) {
      HSLog() << "Failed to load small font.";
    }
  }

  // Setup some nice lighting.
  HSLight light;
  light.mType = HSLT_POINT;
  light.mPosition = HSVector3D(-mRadius * 3, mX - mRadius * 2, mY - mRadius * 2).RenderVector();
  light.mAmbient = HSColor(120, 120, 120);
  light.mRange = (float)mRadius * 6;
  light.mDirection = HSVector3D(1, 0, 0).RenderVector();
  light.mFallOff = 0.0;
  light.mAttenuation0 = 0.0;
  light.mAttenuation1 = 0.0;
  light.mAttenuation2 = 0.00003f * (pow(50.0f, 2.0f) / pow((float)mRadius, 2.0f));
  light.mTheta = 0;
  light.mPhi = 0;
  mRenderer->SetLight(0, light);

  mRenderer->SetTransform(HSTF_WORLD, HSMatrix3D::FromVector(mShipStatus->mHeading) * 
    HSMatrix3D::Translate((float)-mRadius, (float)mX, (float)mY));

  mRenderer->SetVertexFormat(HSVT_COLORED);
  mRenderer->SetStreamSource(mAxis);
  mRenderer->DrawPrimitive(HSPRIM_LINELIST, 0, 3);

  foreach (HSSensorContact, contact, mShipStatus->mContacts) {
    HSVector3D diffVector = contact.position - mShipStatus->mPosition;
    if (diffVector.length() > mShipStatus->mRadarRange) {
      continue;
    }
    diffVector *= ((double)mRadius / mShipStatus->mRadarRange);
    mRenderer->SetTransform(HSTF_WORLD, 
      HSMatrix3D::Translate((float)diffVector.mX, (float)diffVector.mY, (float)diffVector.mZ) *
      HSMatrix3D::FromVector(mShipStatus->mHeading) * 
      HSMatrix3D::Translate(-(float)mRadius, (float)mX, (float)mY));
    mContact->Draw();
  }
  mRenderer->SetTransform(HSTF_WORLD, HSMatrix3D::FromVector(mShipStatus->mHeading) * 
    HSMatrix3D::Translate(-(float)mRadius, (float)mX, (float)mY));
  mRenderer->SetZBufferWrite(false);
  mFont->RenderText3D(HSDistanceString(mShipStatus->mRadarRange).append(" X"), HSVector3D(mRadius, 0, 0));
  mFont->RenderText3D(HSDistanceString(mShipStatus->mRadarRange).append(" Y"), HSVector3D(0, mRadius, 0));
  mFont->RenderText3D(HSDistanceString(mShipStatus->mRadarRange).append(" Z"), HSVector3D(0, 0, mRadius));
  mRenderer->SetZBufferWrite(true);
  mBounds->Draw();
}
