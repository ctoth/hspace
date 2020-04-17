#pragma once

#include "HSVector3D.h"

class HSRenderer;
class HSTexture;
class HSVertexBuffer;

#include <string>

class HSSprite
{
public:
  HSSprite(HSRenderer *aRenderer);
  ~HSSprite(void);

  bool Init(std::string aFileName);

  unsigned int GetWidth();
  unsigned int GetHeight();

  void SetTextureCoords(float u0, float v0, float u1, float v1);

  void SetPosition(HSVector3D aPosition);
  void DrawSprite();

private:
  HSTexture *mTexture;
  HSRenderer *mRenderer;
  HSVertexBuffer *mVB;
  HSVector3D mCurrentPosition;
};
