#include "HSGraphicsWindow.h"

#include "HSRenderer.h"
#include "HSTexture.h"
#include "HSVertexBuffer.h"
#include "HSFont.h"

#define CORNER_WIDTH 20
#define CORNER_HEIGHT 20

HSGraphicsWindow::HSGraphicsWindow(HSRenderer *aRenderer)
  : mWidth(200)
  , mHeight(100)
  , mRenderer(aRenderer)
  , mTexture(NULL)
  , mVB(NULL)
  , mFont(NULL)
  , mColor(30, 80, 170)
{
}

HSGraphicsWindow::~HSGraphicsWindow(void)
{
  delete mTexture;
  delete mVB; 
}

bool
HSGraphicsWindow::Init(std::string aFileName, HSFont *aFont)
{
  mTexture = mRenderer->CreateTextureFromFile(aFileName.c_str());
  if (!mTexture) {
    return false;
  }

  mVB = mRenderer->CreateVertexBuffer(HSVT_COLOREDTFTEX, 48);

  HSVertexColoredTFTexCoords *vertices = 
    (HSVertexColoredTFTexCoords*)mVB->Lock();

  if (!vertices) {
    return false;
  }

  for (int i = 0; i < 48; i++) {
    vertices[i].color = mColor.ARGB();
    vertices[i].rhw = 1;
  }
  vertices[0].texcoords = HSTextureCoordinate(0, 0);
  vertices[1].texcoords = vertices[4].texcoords = vertices[12].texcoords = HSTextureCoordinate(0.3516f, 0);
  vertices[16].texcoords = vertices[2].texcoords = HSTextureCoordinate(0, 0.3516f);
  vertices[28].texcoords = vertices[24].texcoords = vertices[17].texcoords = vertices[14].texcoords = vertices[6].texcoords = vertices[3].texcoords = HSTextureCoordinate(0.3516f, 0.3516f);
  vertices[20].texcoords = vertices[8].texcoords = vertices[5].texcoords = vertices[13].texcoords = HSTextureCoordinate(0.6406f, 0);
  vertices[36].texcoords = vertices[29].texcoords = vertices[25].texcoords = vertices[22].texcoords = vertices[15].texcoords = vertices[10].texcoords = vertices[7].texcoords = HSTextureCoordinate(0.6406f, 0.3516f);
  vertices[21].texcoords = vertices[9].texcoords = HSTextureCoordinate(1, 0);
  vertices[37].texcoords = vertices[23].texcoords = vertices[11].texcoords = HSTextureCoordinate(1, 0.3516f);
  vertices[32].texcoords = vertices[18].texcoords = HSTextureCoordinate(0, 0.6406f);
  vertices[40].texcoords = vertices[33].texcoords = vertices[30].texcoords = vertices[26].texcoords = vertices[19].texcoords = HSTextureCoordinate(0.3516f, 0.6406f);
  vertices[44].texcoords = vertices[41].texcoords = vertices[38].texcoords = vertices[31].texcoords = vertices[27].texcoords = HSTextureCoordinate(0.6406f, 0.6406f);
  vertices[34].texcoords = HSTextureCoordinate(0, 1);
  vertices[42].texcoords = vertices[35].texcoords = HSTextureCoordinate(0.3516f, 1);
  vertices[45].texcoords = vertices[39].texcoords = HSTextureCoordinate(1, 0.6406f);
  vertices[46].texcoords = vertices[43].texcoords = HSTextureCoordinate(0.6406f, 1);
  vertices[47].texcoords = HSTextureCoordinate(1, 1);
  
  UpdateCoords();

  mFont = aFont;
  return true;
}

void
HSGraphicsWindow::DrawWindow()
{
  mRenderer->SetVertexFormat(HSVT_COLOREDTFTEX);
  mRenderer->SetStreamSource(mVB);
  mRenderer->SetTexture(mTexture);
  for (int i = 0; i < 48; i += 4) {
    mRenderer->DrawPrimitive(HSPRIM_TRIANGLESTRIP, i, 2);
  }
  mRenderer->SetTexture(NULL);
  mFont->RenderText(mTitle, mPosition + HSVector3D(CORNER_WIDTH, 3, 0));
}

void
HSGraphicsWindow::UpdateCoords()
{
  if (!mVB) {
    return;
  }
  HSVertexColoredTFTexCoords *vertices = 
    (HSVertexColoredTFTexCoords*)mVB->Lock();

  if (!vertices) {
    return;
  }

  vertices[0].v = mPosition.RenderVector();
  vertices[1].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH, 0, 0)).RenderVector();
  vertices[16].v = vertices[2].v = HSVector3D(mPosition + HSVector3D(0, CORNER_HEIGHT, 0)).RenderVector();
  vertices[17].v = vertices[3].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH, CORNER_HEIGHT, 0)).RenderVector();
  vertices[4].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH, 0, 0)).RenderVector();
  vertices[5].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH + mHeaderSize, 0, 0)).RenderVector();
  vertices[6].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH, CORNER_HEIGHT, 0)).RenderVector();
  vertices[7].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH + mHeaderSize, CORNER_HEIGHT, 0)).RenderVector();
  vertices[8].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH + mHeaderSize, 0, 0)).RenderVector();
  vertices[9].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH * 2 + mHeaderSize, 0, 0)).RenderVector();
  vertices[10].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH + mHeaderSize, CORNER_HEIGHT, 0)).RenderVector();
  vertices[11].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH * 2 + mHeaderSize, CORNER_HEIGHT, 0)).RenderVector();
  vertices[12].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH * 2 + mHeaderSize - 2, CORNER_HEIGHT - 3, 0)).RenderVector();
  vertices[20].v = vertices[13].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH + mWidth, CORNER_HEIGHT - 3, 0)).RenderVector();
  vertices[14].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH * 2 + mHeaderSize - 2, CORNER_HEIGHT * 2 - 3, 0)).RenderVector();
  vertices[36].v = vertices[22].v = vertices[15].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH + mWidth, CORNER_HEIGHT * 2 - 3, 0)).RenderVector();
  vertices[32].v = vertices[18].v = HSVector3D(mPosition + HSVector3D(0, CORNER_HEIGHT * 2 + mHeight, 0)).RenderVector();
  vertices[33].v = vertices[19].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH, CORNER_HEIGHT * 2 + mHeight, 0)).RenderVector();
  vertices[21].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH * 2 + mWidth, CORNER_HEIGHT - 3, 0)).RenderVector();
  vertices[37].v = vertices[23].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH * 2 + mWidth, CORNER_HEIGHT * 2 - 3, 0)).RenderVector();
  vertices[24].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH, CORNER_HEIGHT, 0)).RenderVector();
  vertices[25].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH * 2 + mHeaderSize, CORNER_HEIGHT, 0)).RenderVector();
  vertices[40].v = vertices[26].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH, CORNER_HEIGHT * 2 + mHeight, 0)).RenderVector();
  vertices[27].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH * 2 + mHeaderSize, CORNER_HEIGHT * 2 + mHeight, 0)).RenderVector();
  vertices[28].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH * 2 + mHeaderSize, CORNER_HEIGHT * 2 - 3, 0)).RenderVector();
  vertices[29].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH + mWidth, CORNER_HEIGHT * 2 - 3, 0)).RenderVector();
  vertices[30].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH * 2 + mHeaderSize, CORNER_HEIGHT * 2 + mHeight, 0)).RenderVector();
  vertices[44].v = vertices[41].v = vertices[38].v = vertices[31].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH + mWidth, CORNER_HEIGHT * 2 + mHeight, 0)).RenderVector();
  vertices[34].v = HSVector3D(mPosition + HSVector3D(0, CORNER_HEIGHT * 3 + mHeight, 0)).RenderVector();
  vertices[42].v = vertices[35].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH, CORNER_HEIGHT * 3 + mHeight, 0)).RenderVector();
  vertices[45].v = vertices[39].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH * 2 + mWidth, CORNER_HEIGHT * 2 + mHeight, 0)).RenderVector();
  vertices[46].v = vertices[43].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH + mWidth, CORNER_HEIGHT * 3 + mHeight, 0)).RenderVector();
  vertices[47].v = HSVector3D(mPosition + HSVector3D(CORNER_WIDTH * 2 + mWidth, CORNER_HEIGHT * 3 + mHeight, 0)).RenderVector();

  mVB->Unlock();
}

void
HSGraphicsWindow::SetTitle(std::string aTitle)
{
  mTitle = aTitle;
  mHeaderSize = mFont->CalculateWidth(mTitle);
  UpdateCoords();
}

void
HSGraphicsWindow::SetPosition(HSVector3D &aPosition)
{
  mPosition = aPosition;
  UpdateCoords();
}
