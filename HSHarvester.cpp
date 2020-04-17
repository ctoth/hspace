#include "HSHarvester.h"
#include "HSDB.h"
#include "HSResource.h"
#include "HSTools.h"
#include "HSShip.h"
#include "HSCargoBay.h"
#include "HSConf.h"

HSHarvester::HSHarvester(void)
  : mHarvestSpeed(0.00)
{
  mType = HSST_HARVESTER;
  ADD_ATTRIBUTE("HARVESTSPEED", AT_DOUBLE, mHarvestSpeed)
  ADD_ATTRIBUTE("HARVESTCOMMODS", AT_INTLIST, mHarvestCommods)
}

HSHarvester::~HSHarvester(void)
{
}

HSHarvesterInstance::HSHarvesterInstance(void)
  : mTarget(-1)
  , mCommod(-1)
  , mTargetObj(NULL)
  , mCommodObj(NULL)
  , mBuiltUp(0.0)
{
  mType = HSST_HARVESTER;
  ADD_ATTRIBUTE_INHERIT("HARVESTSPEED", AT_DOUBLE, mHarvestSpeed)
  ADD_ATTRIBUTE_INHERIT("HARVESTCOMMODS", AT_INTLIST, mHarvestCommods)
  ADD_ATTRIBUTE_INTERNAL("TARGET", AT_INTEGER, mTarget)
  ADD_ATTRIBUTE_INTERNAL("COMMOD", AT_INTEGER, mCommod)
}

HSHarvesterInstance::~HSHarvesterInstance(void)
{
}

void
HSHarvesterInstance::Cycle()
{
  HSSystemInstance::Cycle();
  if (!mTargetObj || !mCommodObj) {
    return;
  }

  if (!GetCurrentPower() && GetOptimalPower()) {
    mShip->NotifyConsolesFormatted(GetName(), "No power, harvesting operations cancelled");
    CancelHarvesting();
    return;
  }

  HSVector3D diffVector = mTargetObj->GetLocation() - mShip->GetLocation();
  if (diffVector.length() > sHSConf.HarvestDistance) {
    mShip->NotifyConsolesFormatted(GetName(), 
      "Resource out of range, harvesting operations cancelled");
    CancelHarvesting();
    return;
  }

  double toHarvest = mCommodObj->GetHarvestDifficulty() * 
    mTargetObj->GetDifficulty() * GetHarvestSpeed(true);

  HSCargoBayInstance *bayToUse = NULL;
  foreach (HSSystemInstance*, sys, mShip->FindSystemsByType(HSST_CARGOBAY)) {
    HSCargoBayInstance *bay = static_cast<HSCargoBayInstance*>(sys);
    if (bay->GetFreeSpace() > toHarvest) {
      bayToUse = bay;
      break;
    }
  }

  if (!bayToUse) {
    mShip->NotifyConsolesFormatted(GetName(),
      "No cargo space available to store harvested resources, harvesting operations cancelled");
    CancelHarvesting();
    return;
  }

  double onTarget = mTargetObj->GetCargoForCommodity(mCommodObj->GetID());

  if (onTarget < 0) {
    mBuiltUp += toHarvest;
  } else {
    if (onTarget < toHarvest) {
      mShip->NotifyConsolesFormatted(GetName(),
        "Resource depleted, harvesting operations cancelled");
      CancelHarvesting();
      return;
    }
    mBuiltUp += toHarvest;
    mTargetObj->SetCargoForCommodity(mCommodObj->GetID(),
      mTargetObj->GetCargoForCommodity(mCommodObj->GetID()) - toHarvest);
  }

  if (mBuiltUp > 1.0) {
    bayToUse->SetCargoForCommodity(mCommodObj->GetID(),
      bayToUse->GetCargoForCommodity(mCommodObj->GetID()) + floor(mBuiltUp));
    mBuiltUp -= floor(mBuiltUp);
  }
}

void
HSHarvesterInstance::AttributeChanged(std::string aName)
{
  if (aName == "TARGET") {
    if (mTarget == -1) {
      mTargetObj = NULL;
      return;
    }
    HSObject *target = sHSDB.FindObject(mTarget);
    if (!target) {
      mTargetObj = NULL;
    } else {
      if (target->GetType() != HSOT_RESOURCE) {
        mTargetObj = NULL;
      } else {
        mTargetObj = static_cast<HSResource*>(target);
      }
    }
    if (!mTargetObj) {
      mTarget = -1;
    }
  } else if (aName == "COMMOD") {
    if (mCommod == -1) {
      mCommodObj = NULL;
      return;
    }
    mCommodObj = sHSDB.FindCommodity(mCommod);
    
    if (!mCommodObj) {
      mCommod = -1;
    }
  }
}

void
HSHarvesterInstance::CancelHarvesting()
{
  mBuiltUp = 0.0;
  mCommod = -1;
  mTarget = -1;
  mTargetObj = NULL;
  mCommodObj = NULL;
}
