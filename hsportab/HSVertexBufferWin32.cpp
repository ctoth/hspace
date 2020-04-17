#include "HSVertexBufferWin32.h"
#include "D3D9.h"

HSVertexBufferWin32::HSVertexBufferWin32(IDirect3DVertexBuffer9 *aVertexBuffer)
  : mVB(aVertexBuffer)
{
}

HSVertexBufferWin32::~HSVertexBufferWin32(void)
{
}

void*
HSVertexBufferWin32::Lock()
{
  HRESULT hr;

  void *ptr;
  hr = mVB->Lock(0, 0, &ptr, 0);

  if (FAILED(hr)) {
    return NULL;
  }
  return ptr;
}

void
HSVertexBufferWin32::Unlock()
{
  mVB->Unlock();
}
