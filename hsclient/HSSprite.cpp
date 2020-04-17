#include "HSSprite.h"
#include "HSRenderer.h"
#include "HSTexture.h"
#include "HSVertexBuffer.h"

HSSprite::HSSprite(HSRenderer *aRenderer)
  : mRenderer(aRenderer)
  , mTexture(NULL)
  , mVB(NULL)
{
}

HSSprite::~HSSprite(void)
{
  delete mTexture;
  delete mVB;
}

bool
HSSprite::Init(std::string aFileName)
{
  mTexture = mRenderer->CreateTextureFromFile(aFileName.c_str());
  if (!mTexture) {
    return false;
  }

  delete mVB;
  mVB = mRenderer->CreateVertexBuffer(HSVT_COLOREDTFTEX, 4);

  HSVertexColoredTFTexCoords *vertices = 
    (HSVertexColoredTFTexCoords*)mVB->Lock();

  if (!vertices) {
    return false;
  }

  for (int i = 0; i < 4; i++) {
    vertices[i].color = HSColor(255, 255, 255).ARGB();
    vertices[i].rhw = 1;
  }
  vertices[0].texcoords = HSTextureCoordinate(0, 0);
  vertices[1].texcoords = HSTextureCoordinate(1, 0);
  vertices[2].texcoords = HSTextureCoordinate(0, 1);
  vertices[3].texcoords = HSTextureCoordinate(1, 1);
  mVB->Unlock();
  return true;
}

unsigned int
HSSprite::GetWidth()
{
  if (mTexture) {
    return mTexture->GetWidth();
  }
  return 0;
}

unsigned int
HSSprite::GetHeight()
{
  if (mTexture) {
    return mTexture->GetHeight();
  }
  return 0;
}

void
HSSprite::SetTextureCoords(float u0, float v0, float u1, float v1)
{
  HSVertexColoredTFTexCoords *vertices = 
    (HSVertexColoredTFTexCoords*)mVB->Lock();

  if (!vertices) {
    return;
  }

  for (int i = 0; i < 4; i++) {
    vertices[i].color = HSColor(255, 255, 255).ARGB();
    vertices[i].rhw = 1;
  }
  vertices[0].texcoords = HSTextureCoordinate(u0, v0);
  vertices[1].texcoords = HSTextureCoordinate(u1, v0);
  vertices[2].texcoords = HSTextureCoordinate(u0, v1);
  vertices[3].texcoords = HSTextureCoordinate(u1, v1);
  mVB->Unlock();
  SetPosition(mCurrentPosition);
}

void
HSSprite::SetPosition(HSVector3D aPosition)
{
  mCurrentPosition = aPosition;
  if (!mVB) {
    return;
  }
  HSVertexColoredTFTexCoords *vertices = 
    (HSVertexColoredTFTexCoords*)mVB->Lock();

  if (!vertices) {
    return;
  }


  for (int i = 0; i < 4; i++) {
    vertices[i].v = (aPosition + 
      HSVector3D(mTexture->GetWidth() * vertices[i].texcoords.mTU - 0.5, mTexture->GetHeight() * vertices[i].texcoords.mTV - 0.5, 0)).RenderVector();
  }
  mVB->Unlock();
}

void
HSSprite::DrawSprite()
{
  mRenderer->SetVertexFormat(HSVT_COLOREDTFTEX);
  mRenderer->SetStreamSource(mVB);
  mRenderer->SetTexture(mTexture);
  mRenderer->DrawPrimitive(HSPRIM_TRIANGLESTRIP, 0, 2);
  mRenderer->SetTexture(NULL);
}
