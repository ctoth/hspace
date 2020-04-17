#pragma once
#include "HSObject.h"
#include "HSCargoContainer.h"

/**
 * \ingroup HS_OBJECTs
 * \brief A temporary object that represents ejected cargo.
 */
class HSCargoPod 
  : public HSObject
  , public HSCargoContainer
{
  /** How long the pod lasts */
  ATTRIBUTE(LifeTime, int)
public:
  HSCargoPod(void);
  ~HSCargoPod(void);

  void Cycle();
  void Destroy();
};
