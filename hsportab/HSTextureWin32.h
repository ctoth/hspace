#pragma once
#include "HSTexture.h"

struct IDirect3DTexture9;

class HSTextureWin32 :
  public HSTexture
{
public:
  HSTextureWin32(IDirect3DTexture9 *aTexture);
  ~HSTextureWin32(void);

  unsigned int GetWidth();
  unsigned int GetHeight();

private:
  friend class HSRenderer;

  unsigned int mWidth, mHeight;

  IDirect3DTexture9 *mTexture;
};
