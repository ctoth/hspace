#pragma once

class HSTexture
{
public:
  virtual ~HSTexture() {}

  virtual unsigned int GetWidth() = 0;
  virtual unsigned int GetHeight() = 0;
};
