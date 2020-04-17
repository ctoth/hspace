#pragma once
#include "HSVertexbuffer.h"

struct IDirect3DVertexBuffer9;

class HSVertexBufferWin32 :
  public HSVertexBuffer
{
public:
  HSVertexBufferWin32(IDirect3DVertexBuffer9 *aVertexBuffer);
  ~HSVertexBufferWin32(void);

  void *Lock();
  void Unlock();

private:
  friend class HSRenderer;

  IDirect3DVertexBuffer9 *mVB;
};
