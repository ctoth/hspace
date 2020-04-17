#include "HSTextureWin32.h"
#include "D3D9.h"

HSTextureWin32::HSTextureWin32(IDirect3DTexture9 *aTexture)
  : mTexture(aTexture)
{
  D3DSURFACE_DESC desc;
  mTexture->GetLevelDesc(0, &desc);
  mWidth = desc.Width;
  mHeight = desc.Height;
}

HSTextureWin32::~HSTextureWin32(void)
{
  mTexture->Release();
}

unsigned int
HSTextureWin32::GetHeight()
{
  return mHeight;
}

unsigned int
HSTextureWin32::GetWidth()
{
  return mWidth;
}
