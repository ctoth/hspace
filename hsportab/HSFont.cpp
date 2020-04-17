#include "HSFont.h"
#include "HSRenderer.h"
#include "HSTexture.h"
#include "HSVertexBuffer.h"

#include <windows.h>

#define MAXCHARS 100

HSFont::HSFont(HSRenderer *aRenderer)
  : mRenderer(aRenderer)
{
}

HSFont::~HSFont(void)
{
  delete mTexture;
  delete mVB;
}

bool
HSFont::LoadFont(std::string aName)
{
  std::string img = aName;
  mTexture = mRenderer->CreateTextureFromFile(img.append(".tga").c_str());
  if (!mTexture) {
    return false;
  }

#ifdef WIN32
  FILE *fd = NULL;
  errno_t err = fopen_s(&fd, aName.append(".dat").c_str(), "r");
#else
  FILE *fd = fopen(aName.append(".dat").c_str(), "r");
#endif
  if (!fd) {
    delete mTexture;
    mTexture = NULL;
    return false;
  }
  for (int i = 0; i < 256; i++) {
    ::fread(mWidths + i, 1, 2, fd);
  }
  ::fclose(fd);

  mVB = mRenderer->CreateVertexBuffer(HSVT_COLOREDTFTEX, 4 * MAXCHARS);
  return true;
}

void
HSFont::RenderText(std::string aText, const HSVector3D &aPosition, HSAlignment aAlign)
{
  if (!mTexture) {
    return;
  }
  HSVector3D placement = aPosition;
  HSVertexColoredTFTexCoords *vertices = 
    (HSVertexColoredTFTexCoords*)mVB->Lock();

  if (!vertices) {
    return;
  }

  float texWidth = (float)mTexture->GetWidth();
  float texHeight = (float)mTexture->GetHeight();
  unsigned int height = mTexture->GetHeight() / 16;
  unsigned int width = mTexture->GetWidth() / 16;
  float pos = 0.0f;
  if (aAlign == HSALIGN_CENTER) {
    size_t totalWidth = 0;
    for (size_t i = 0; i < aText.length(); i++) {
      totalWidth += mWidths[aText[i]];
    }
    placement -= HSVector3D(totalWidth / 2, height / 2, 0);
  } else if (aAlign == HSALIGN_RIGHT) {
    size_t totalWidth = 0;
    for (size_t i = 0; i < aText.length(); i++) {
      totalWidth += mWidths[aText[i]];
    }
    placement -= HSVector3D(totalWidth, 0, 0);
  }
  for (size_t i = 0; i < aText.length(); i++) {
    short charWidth = mWidths[aText[i]];
    for (int c = 0; c < 4; c++) {
      vertices[i * 4 + c].color = HSColor(255,255,255).ARGB();
      vertices[i * 4 + c].rhw = 1;
    }
    charWidth += charWidth % 2;
    HSVector3D topLeft = placement + HSVector3D(pos + 0.5, 0.5, 0);
    HSVector3D bottomLeft = placement + HSVector3D(pos + 0.5, height + 0.5, 0);
    pos += charWidth;
    HSVector3D topRight = placement + HSVector3D(pos + 0.5, 0.5, 0);
    HSVector3D bottomRight = placement + HSVector3D(pos + 0.5, height + 0.5, 0);
    
    short charLine = aText[i] / 16;
    short charPos = aText[i] % 16;
    float v1 = charLine * height / texHeight;
    float u1 = (float)(charPos * width + width / 2 - charWidth / 2) / texWidth;
    float v2 = (charLine + 1) * height / texHeight;
    float u2 = (float)(charPos * width + width / 2 + charWidth / 2) / texWidth;

    vertices[i * 4].v = topLeft.RenderVector();
    vertices[i * 4].texcoords = HSTextureCoordinate(u1, v1);
    vertices[i * 4 + 1].v = topRight.RenderVector();
    vertices[i * 4 + 1].texcoords = HSTextureCoordinate(u2, v1);
    vertices[i * 4 + 2].v = bottomLeft.RenderVector();
    vertices[i * 4 + 2].texcoords = HSTextureCoordinate(u1, v2);
    vertices[i * 4 + 3].v = bottomRight.RenderVector();
    vertices[i * 4 + 3].texcoords = HSTextureCoordinate(u2, v2);
  }
  mVB->Unlock();
  mRenderer->SetVertexFormat(HSVT_COLOREDTFTEX);
  mRenderer->SetStreamSource(mVB);
  mRenderer->SetTexture(mTexture);
  for (unsigned int i = 0; i < aText.length(); i++) {
    mRenderer->DrawPrimitive(HSPRIM_TRIANGLESTRIP, i * 4, 2);
  }
  mRenderer->SetTexture(NULL);
}

void
HSFont::RenderText3D(std::string aText, HSVector3D aPosition, HSAlignment aAlign)
{
  if (!mTexture) {
    return;
  }
  HSVertexColoredTFTexCoords *vertices = 
    (HSVertexColoredTFTexCoords*)mVB->Lock();

  if (!vertices) {
    return;
  }
  HSMatrix3D world = mRenderer->GetTransform(HSTF_WORLD);
  aPosition = world * aPosition;
  HSMatrix3D view = mRenderer->GetTransform(HSTF_VIEW);
  aPosition = view * aPosition;
  double W = aPosition.mZ;
  HSMatrix3D projection = mRenderer->GetTransform(HSTF_PROJECTION);
  aPosition = projection * aPosition;
  aPosition.mZ /= W;
  aPosition.mX /= W;
  aPosition.mY /= W;
  aPosition.mX += (double)1;
  aPosition.mX *= 0.5;
  aPosition.mY -= (double)1;
  aPosition.mY *= -0.5;

  aPosition.mX *= (double)mRenderer->GetWidth();
  aPosition.mY *= (double)mRenderer->GetHeight();

  HSVector3D placement = aPosition;
  float texWidth = (float)mTexture->GetWidth();
  float texHeight = (float)mTexture->GetHeight();
  unsigned int height = mTexture->GetHeight() / 16;
  unsigned int width = mTexture->GetWidth() / 16;
  float pos = 0.0f;
  if (aAlign == HSALIGN_CENTER) {
    size_t totalWidth = 0;
    for (size_t i = 0; i < aText.length(); i++) {
      totalWidth += mWidths[aText[i]];
    }
    placement -= HSVector3D(totalWidth / 2, height / 2, 0);
  } else if (aAlign == HSALIGN_RIGHT) {
    size_t totalWidth = 0;
    for (size_t i = 0; i < aText.length(); i++) {
      totalWidth += mWidths[aText[i]];
    }
    placement -= HSVector3D(totalWidth, 0, 0);
  }
  for (size_t i = 0; i < aText.length(); i++) {
    short charWidth = mWidths[aText[i]];
    for (int c = 0; c < 4; c++) {
      vertices[i * 4 + c].color = HSColor(255,255,255).ARGB();
      vertices[i * 4 + c].rhw = 1;
    }
    charWidth += charWidth % 2;
    HSVector3D topLeft = placement + HSVector3D(pos + 0.5, 0.5, 0);
    HSVector3D bottomLeft = placement + HSVector3D(pos + 0.5, height + 0.5, 0);
    pos += charWidth;
    HSVector3D topRight = placement + HSVector3D(pos + 0.5, 0.5, 0);
    HSVector3D bottomRight = placement + HSVector3D(pos + 0.5, height + 0.5, 0);
    
    short charLine = aText[i] / 16;
    short charPos = aText[i] % 16;
    float v1 = charLine * height / texHeight;
    float u1 = (float)(charPos * width + width / 2 - charWidth / 2) / texWidth;
    float v2 = (charLine + 1) * height / texHeight;
    float u2 = (float)(charPos * width + width / 2 + charWidth / 2) / texWidth;

    vertices[i * 4].v = topLeft.RenderVector();
    vertices[i * 4].texcoords = HSTextureCoordinate(u1, v1);
    vertices[i * 4 + 1].v = topRight.RenderVector();
    vertices[i * 4 + 1].texcoords = HSTextureCoordinate(u2, v1);
    vertices[i * 4 + 2].v = bottomLeft.RenderVector();
    vertices[i * 4 + 2].texcoords = HSTextureCoordinate(u1, v2);
    vertices[i * 4 + 3].v = bottomRight.RenderVector();
    vertices[i * 4 + 3].texcoords = HSTextureCoordinate(u2, v2);
  }
  mVB->Unlock();
  mRenderer->SetVertexFormat(HSVT_COLOREDTFTEX);
  mRenderer->SetStreamSource(mVB);
  mRenderer->SetTexture(mTexture);
  for (unsigned int i = 0; i < aText.length(); i++) {
    mRenderer->DrawPrimitive(HSPRIM_TRIANGLESTRIP, i * 4, 2);
  }
  mRenderer->SetTexture(NULL);
}

int
HSFont::CalculateWidth(std::string aText)
{
  int totalWidth = 0;
  for (size_t i = 0; i < aText.length(); i++) {
    totalWidth += (int)mWidths[aText[i]];
  }
  return totalWidth;
}
