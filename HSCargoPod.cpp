#include "HSCargoPod.h"
#include "HSUniverse.h"
#include "HSDB.h"
#include "HSTools.h"

HSCargoPod::HSCargoPod(void)
  : mLifeTime(86400)
{
  mType = HSOT_CARGOPOD;
  ADD_ATTRIBUTE("CARGO", AT_CARGOLIST, mCargo)
  ADD_ATTRIBUTE("LIFETIME", AT_INTEGER, mLifeTime)
}

HSCargoPod::~HSCargoPod(void)
{
}

void
HSCargoPod::Cycle(void)
{
  if (!--mLifeTime) {
    Destroy();
    return;
  }
  HSObject::Cycle();
}

void
HSCargoPod::Destroy()
{
  HSUniverse *univ = sHSDB.FindUniverse(mUniverse);
  if (!univ) {
    HSLog() << "Temporary object with invalid universe " << univ;
    return;
  }
  univ->RemoveObject(this);
  delete this;
}
