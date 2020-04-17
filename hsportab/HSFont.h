#pragma once

class HSRenderer;
class HSTexture;
class HSVertexBuffer;

#include <string>
#include "HSVector3D.h"

enum HSAlignment
{
  HSALIGN_LEFT = 0,
  HSALIGN_CENTER,
  HSALIGN_RIGHT
};

class HSFont
{
public:
  HSFont(HSRenderer *aRenderer);
  ~HSFont(void);

  bool LoadFont(std::string aName);

  void RenderText(std::string aText, 
    const HSVector3D &aPosition, 
    HSAlignment aAlignment = HSALIGN_LEFT);

  void RenderText3D(std::string aText, 
    HSVector3D aPosition, 
    HSAlignment aAlignment = HSALIGN_LEFT);

  int CalculateWidth(std::string aText);

private:
  HSRenderer *mRenderer;
  HSTexture *mTexture;
  HSVertexBuffer *mVB;
  HSVertexBuffer *m3DVB;
  unsigned short mWidths[255];
};
