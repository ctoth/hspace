#pragma once
#include "HSObject.h"
#include "HSCargoContainer.h"

class HSResource 
  : public HSObject
  , public HSCargoContainer
{
  ATTRIBUTE(Generates, std::vector<HSCargoItem>)
  ATTRIBUTE(Difficulty, double)
  ATTRIBUTE(Destructs, bool)
public:
  HSResource(void);
  ~HSResource(void);

  void Cycle();
};
