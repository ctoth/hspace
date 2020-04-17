#pragma once

#include <string>

#include <HSColor.h>
#include <HSVector3D.h>

class HSTexture;
class HSVertexBuffer;
class HSRenderer;
class HSFont;

class HSGraphicsWindow
{
public:
  HSGraphicsWindow(HSRenderer *aRenderer);
  ~HSGraphicsWindow(void);

  bool Init(std::string aFileName, HSFont *aFont);

  void DrawWindow();

  void SetTitle(std::string aTitle);

  void SetPosition(HSVector3D &aPosition);
private:
  void UpdateCoords();

  HSVector3D mPosition;
  unsigned int mWidth;
  unsigned int mHeight;
  int mHeaderSize;
  HSRenderer *mRenderer;
  HSTexture *mTexture;
  HSVertexBuffer *mVB;
  HSFont *mFont;

  std::string mTitle;

  HSColor mColor;
};
