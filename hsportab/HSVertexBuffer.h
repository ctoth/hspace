#pragma once

#include "HSVector3D.h"

class HSTextureCoordinate
{
public:
  HSTextureCoordinate(float aTU, float aTV)
    : mTU(aTU)
    , mTV(aTV)
  {
  }

  float mTU;
  float mTV;
};

struct HSVertexColored
{
  HSVector3DRender v;
  unsigned long color;
};

struct HSVertexColoredTexCoords
{
  HSVector3DRender v;
  unsigned long color;
  HSTextureCoordinate texcoords;
};

struct HSVertexColoredTFTexCoords
{
  HSVector3DRender v;
  float rhw;
  unsigned long color;
  HSTextureCoordinate texcoords;
};

class HSVertexBuffer
{
public:
  virtual void *Lock() = 0;
  virtual void Unlock() = 0;
};
