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
#include "HSRenderer.h"
#include "HSRendererPrivWin32.h"
#include "HSVertexBufferWin32.h"
#include "HSTextureWin32.h"
#include "HSWindow.h"
#include "HSThread.h"
#include "D3DX9.h"

class HSD3DMATRIX : public D3DMATRIX
{
public:
  HSD3DMATRIX()
  {
  }

  HSD3DMATRIX(const HSMatrix3D &aMatrix)
  {
    memcpy(m, aMatrix.m, sizeof(float) * 16);
  }

  operator HSMatrix3D ()
  {
    HSMatrix3D retMatrix;
    memcpy(retMatrix.m, m, sizeof(float) * 16);
    return retMatrix;
  }
};

class HSD3DVECTOR : public D3DVECTOR
{
public:
  HSD3DVECTOR()
  {
  }

  HSD3DVECTOR(const HSVector3DRender &aVector)
  {
    x = aVector._x;
    y = aVector._y;
    z = aVector._z;
  }

};

class HSD3DCOLORVALUE : public D3DCOLORVALUE
{
public:
  HSD3DCOLORVALUE()
  {
  }

  HSD3DCOLORVALUE(const HSColor &aColor)
  {
    r = (float)aColor._r / 255;
    g = (float)aColor._g / 255;
    b = (float)aColor._b / 255;
    a = (float)aColor._a / 255;
  }
};

HSRenderer::HSRenderer(HSWindow *aWindow)
{
  p = new HSRendererPriv(aWindow);
}

HSRenderer::~HSRenderer()
{
  delete p;
}

bool
HSRenderer::Init()
{
  p->mD3D = ::Direct3DCreate9(D3D_SDK_VERSION);

  if (!p->mD3D) {
    return false;
  }

  return true;
}

bool
HSRenderer::CreateDevice()
{
  HRESULT hr;

  D3DPRESENT_PARAMETERS d3dpp;

  memset(&d3dpp, 0, sizeof(d3dpp));
  d3dpp.hDeviceWindow = p->mWnd;
  d3dpp.Windowed = TRUE;
  d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
  d3dpp.BackBufferCount = D3DFMT_UNKNOWN;
  d3dpp.EnableAutoDepthStencil = TRUE;
  d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

  hr = p->mD3D->CreateDevice(D3DADAPTER_DEFAULT,
    D3DDEVTYPE_HAL,
    p->mWnd,
    D3DCREATE_MULTITHREADED | D3DCREATE_SOFTWARE_VERTEXPROCESSING,
    &d3dpp,
    &p->mDevice);

  if (FAILED(hr)) {
    return false;
  }

  p->mDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
  p->mDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
  p->mDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
  p->mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,   TRUE );
  p->mDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
  p->mDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
  p->mDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

  p->mDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);  
  p->mDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  p->mDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  p->mDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  p->mDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);  
  p->mDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  p->mDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);  
  p->mDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);

  p->mDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  p->mDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

  // Make sure that there's no mismaps at the edges of our quads.
  p->mDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  p->mDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

  return true;
}

unsigned int
HSRenderer::GetWidth()
{
  if (!p->mDevice) {
    return 0;
  }
  IDirect3DSurface9 *pBackBuf;
  HRESULT hr = p->mDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuf);
  if (FAILED(hr)) {
    return 0;
  }
  D3DSURFACE_DESC desc;
  pBackBuf->GetDesc(&desc);
  pBackBuf->Release();

  float plane[4];
  hr = p->mDevice->GetClipPlane(0, plane);
  return desc.Width;
}

unsigned int
HSRenderer::GetHeight()
{
  if (!p->mDevice) {
    return 0;
  }
  IDirect3DSurface9 *pBackBuf;
  HRESULT hr = p->mDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuf);
  if (FAILED(hr)) {
    return 0;
  }
  D3DSURFACE_DESC desc;
  pBackBuf->GetDesc(&desc);
  pBackBuf->Release();
  return desc.Height;
}

void
HSRenderer::Clear(const HSColor &aColor)
{
  p->mDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, aColor.ARGB(), 1.0f, 0);
}

void
HSRenderer::StartScene()
{
  p->mDevice->BeginScene();
}

void
HSRenderer::EndScene()
{
  p->mDevice->EndScene();
}

void
HSRenderer::Present()
{
  p->mDevice->Present(NULL, NULL, NULL, NULL);
}

void
HSRenderer::Reset()
{
  D3DPRESENT_PARAMETERS d3dpp;

  memset(&d3dpp, 0, sizeof(d3dpp));
  d3dpp.hDeviceWindow = p->mWnd;
  d3dpp.Windowed = TRUE;
  d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
  d3dpp.BackBufferCount = D3DFMT_UNKNOWN;
  d3dpp.EnableAutoDepthStencil = TRUE;
  d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

  p->mDevice->Reset(&d3dpp);
  p->mDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
  p->mDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
  p->mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,   TRUE );
  p->mDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
  p->mDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
  p->mDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );

  // Lighting
  p->mDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(50,50,50));
  p->mDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
  p->mDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
  p->mDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
  p->mDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR1);

  p->mDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);  
  p->mDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  p->mDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  p->mDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  p->mDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);  
  p->mDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  p->mDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);  
  p->mDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);

  p->mDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  p->mDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

  // Make sure that there's no mismaps at the edges of our quads.
  p->mDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  p->mDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
}

HSVertexBuffer *
HSRenderer::CreateVertexBuffer(HSVertexType aType, int aVertices)
{
  DWORD fvf;
  UINT size;
  switch (aType) {
    case HSVT_COLORED:
      size = sizeof(HSVertexColored);
      fvf = D3DFVF_DIFFUSE | D3DFVF_XYZ;
      break;
    case HSVT_COLOREDTEX:
      size = sizeof(HSVertexColoredTexCoords);
      fvf = D3DFVF_DIFFUSE | D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE1(1);
      break;
    case HSVT_COLOREDTFTEX:
      size = sizeof(HSVertexColoredTFTexCoords);
      fvf = D3DFVF_DIFFUSE | D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE1(1);
      break;
  }
  HRESULT hr;

  IDirect3DVertexBuffer9 *vb;
  hr = p->mDevice->CreateVertexBuffer(size * aVertices,
    0,
    fvf,
    D3DPOOL_MANAGED,
    &vb,
    NULL);
  if (FAILED(hr)) {
    return NULL;
  }

  return new HSVertexBufferWin32(vb);
}

HSTexture*
HSRenderer::CreateTextureFromFile(const char *aFileName)
{
  IDirect3DTexture9 *tex = NULL;
  HRESULT hr = D3DXCreateTextureFromFile(p->mDevice, aFileName, &tex);
  if (FAILED(hr) || !tex) {
    return NULL;
  }
  return new HSTextureWin32(tex);
}

void
HSRenderer::SetVertexFormat(HSVertexType aType)
{
  DWORD fvf;
  UINT size;
  switch (aType) {
    case HSVT_COLORED:
      size = sizeof(HSVertexColored);
      fvf = D3DFVF_DIFFUSE | D3DFVF_XYZ;
      break;
    case HSVT_COLOREDTEX:
      size = sizeof(HSVertexColoredTexCoords);
      fvf = D3DFVF_DIFFUSE | D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE1(1);
      break;
    case HSVT_COLOREDTFTEX:
      size = sizeof(HSVertexColoredTFTexCoords);
      fvf = D3DFVF_DIFFUSE | D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE1(1);
      break;
  }
  p->mDevice->SetFVF(fvf);
  p->mFVFStride = size;
}

void
HSRenderer::SetStreamSource(HSVertexBuffer *aVB)
{
  HSVertexBufferWin32 *vb =
    static_cast<HSVertexBufferWin32*>(aVB);

  p->mDevice->SetStreamSource(0, vb->mVB, 0, p->mFVFStride);
}

void
HSRenderer::SetTexture(HSTexture *aTexture)
{
  if (aTexture) {
    HSTextureWin32 *tex =
      static_cast<HSTextureWin32*>(aTexture);
    p->mDevice->SetTexture(0, tex->mTexture);
  } else {
    p->mDevice->SetTexture(0, NULL);
  }
}

void
HSRenderer::SetTransform(HSTransformType aTransform, const HSMatrix3D &aMatrix)
{
  D3DTRANSFORMSTATETYPE transformType;
  switch (aTransform) {
    case HSTF_WORLD:
      transformType = D3DTS_WORLD;
      break;
    case HSTF_PROJECTION:
      transformType = D3DTS_PROJECTION;
      break;
    case HSTF_VIEW:
      transformType = D3DTS_VIEW;
      break;
  }

  HSD3DMATRIX matrix(aMatrix);
  p->mDevice->SetTransform(transformType, &matrix);
}

void
HSRenderer::SetLight(int aIndex, const HSLight &aLight)
{
  D3DLIGHTTYPE lightType;
  switch (aLight.mType) {
    case HSLT_POINT:
      lightType = D3DLIGHT_POINT;
      break;
    case HSLT_SPOT:
      lightType = D3DLIGHT_SPOT;
      break;
    case HSLT_DIRECTIONAL:
      lightType = D3DLIGHT_DIRECTIONAL;
      break;
  }

  D3DLIGHT9 light;
  light.Type = lightType;
  light.Ambient = HSD3DCOLORVALUE(aLight.mAmbient);
  light.Diffuse = HSD3DCOLORVALUE(aLight.mDiffuse);
  light.Specular = HSD3DCOLORVALUE(aLight.mSpecular);
  light.Position = HSD3DVECTOR(aLight.mPosition);
  light.Direction = HSD3DVECTOR(aLight.mDirection);
  light.Falloff = aLight.mFallOff;
  light.Range = aLight.mRange;
  light.Theta = aLight.mTheta;
  light.Phi = aLight.mPhi;
  light.Attenuation0 = aLight.mAttenuation0;
  light.Attenuation1 = aLight.mAttenuation1;
  light.Attenuation2 = aLight.mAttenuation2;

  p->mDevice->SetLight(aIndex, &light);
  p->mDevice->LightEnable(aIndex, TRUE);
}

void
HSRenderer::SetZBufferWrite(bool aWrite)
{
  p->mDevice->SetRenderState(D3DRS_ZWRITEENABLE, aWrite ? TRUE : FALSE);
}

HSMatrix3D
HSRenderer::GetTransform(HSTransformType aTransform)
{
  D3DTRANSFORMSTATETYPE transformType;
  switch (aTransform) {
    case HSTF_WORLD:
      transformType = D3DTS_WORLD;
      break;
    case HSTF_PROJECTION:
      transformType = D3DTS_PROJECTION;
      break;
    case HSTF_VIEW:
      transformType = D3DTS_VIEW;
      break;
  }

  HSD3DMATRIX matrix;
  p->mDevice->GetTransform(transformType, &matrix);
  return matrix;
}

void
HSRenderer::DrawPrimitive(HSPrimitiveType aType,
                          unsigned int aFirstVertex,
                          unsigned int aPrimitiveCount)
{
  HRESULT hr;

  D3DPRIMITIVETYPE type;

  switch (aType) {
    case HSPRIM_TRIANGLELIST:
      type = D3DPT_TRIANGLELIST;
      break;
    case HSPRIM_TRIANGLESTRIP:
      type = D3DPT_TRIANGLESTRIP;
      break;
    case HSPRIM_LINELIST:
      type = D3DPT_LINELIST;
      break;
    case HSPRIM_LINESTRIP:
      type = D3DPT_LINESTRIP;
      break;
  }
  hr = p->mDevice->DrawPrimitive(type, aFirstVertex, aPrimitiveCount);
  int a = 1;
}

HSRendererPriv::HSRendererPriv(HSWindow *aWindow)
  : mD3D(NULL)
  , mDevice(NULL)
{
  mWnd = (HWND)aWindow->Handle();
}

HSRendererPriv::~HSRendererPriv()
{
  if (mDevice) {
    mDevice->Release();
  }
  if (mD3D) {
    mD3D->Release();
  }
}
