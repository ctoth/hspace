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

#include "HSColor.h"
#include "HSMatrix3D.h"

class HSRendererPriv;
class HSWindow;
class HSVertexBuffer;
class HSTexture;

enum HSVertexType
{
  HSVT_COLORED = 0,
  HSVT_COLOREDTEX,
  HSVT_COLOREDTFTEX
};

enum HSPrimitiveType
{
  HSPRIM_TRIANGLELIST = 0,
  HSPRIM_TRIANGLESTRIP,
  HSPRIM_LINELIST,
  HSPRIM_LINESTRIP
};

enum HSTransformType
{
  HSTF_VIEW = 0,
  HSTF_PROJECTION,
  HSTF_WORLD
};

enum HSLightType
{
  HSLT_POINT = 0,
  HSLT_SPOT,
  HSLT_DIRECTIONAL
};

struct HSLight
{
  HSLightType mType;
  HSColor mDiffuse;
  HSColor mSpecular;
  HSColor mAmbient;
  HSVector3DRender mPosition;
  HSVector3DRender mDirection;
  float mRange;
  float mFallOff;
  float mAttenuation0;
  float mAttenuation1;
  float mAttenuation2;
  float mTheta;
  float mPhi;
};

class HSRenderer
{
public:
  HSRenderer(HSWindow *aWindow);
  ~HSRenderer();

  bool Init();
  bool CreateDevice();

  unsigned int GetWidth();
  unsigned int GetHeight();

  void Clear(const HSColor &aColor);
  void StartScene();
  void EndScene();
  void Present();
  void Reset();

  HSVertexBuffer *CreateVertexBuffer(HSVertexType aType, int aVertices);
  HSTexture *CreateTextureFromFile(const char *aFileName);

  void SetVertexFormat(HSVertexType aType);
  void SetStreamSource(HSVertexBuffer *aVB);
  void SetTexture(HSTexture *aTexture);
  void SetTransform(HSTransformType aTransform, const HSMatrix3D &aVector);
  void SetLight(int aIndex, const HSLight &aLight);
  void SetZBufferWrite(bool aWrite);

  HSMatrix3D GetTransform(HSTransformType aTransform);

  void DrawPrimitive(HSPrimitiveType aType, 
    unsigned int aFirstVertex,
    unsigned int aPrimitiveCount);

private:
  HSRendererPriv *p;
};
