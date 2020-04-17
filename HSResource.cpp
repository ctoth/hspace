#include "HSResource.h"
#include "HSTools.h"
#include "HSCommodity.h"
#include "HSUniverse.h"
#include "HSDB.h"

HSResource::HSResource(void)
  : mDifficulty(1.0)
  , mDestructs(false)
{
  mType = HSOT_RESOURCE;
  ADD_ATTRIBUTE("RESOURCES", AT_CARGOLIST, mCargo)
  ADD_ATTRIBUTE("GENERATES", AT_CARGOLIST, mGenerates)
  ADD_ATTRIBUTE("DIFFICULTY", AT_DOUBLE, mDifficulty)
  ADD_ATTRIBUTE("DESTRUCTS", AT_BOOLEAN, mDestructs)
}

HSResource::~HSResource(void)
{
}

void
HSResource::Cycle()
{
  int generatesItems = 0;
  foreach(HSCargoItem, item, mGenerates) {
    if (item.amount > 0) {
      generatesItems++;
    }
    SetCargoForCommodity(item.commod->GetID(),
      GetCargoForCommodity(item.commod->GetID()) + item.amount);
  }
  if (!generatesItems) {
    foreach(HSCargoItem, item, mCargo) {
      if (item.amount > 0) {
        return;
      }
    }
    // This object does not generate anything, and has no more cargo left.
    // If it destructs now is the time.
    if (mDestructs) {
      HSUniverse *univ = sHSDB.FindUniverse(mUniverse);
      if (!univ) {
        HSLog() << this << "Invalid mUniverse!";
        return;
      }
      HSIFDestroy(mID);
      univ->RemoveObject(this);
      delete this;
    }
  }
}
