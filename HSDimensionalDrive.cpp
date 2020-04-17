#include "HSDimensionalDrive.h"

// Local
#include "HSAnsi.h"
#include "HSShip.h"
#include "HSConf.h"
#include "HSTools.h"

HSDimensionalDrive::HSDimensionalDrive(void)
  : mRequiredCharge(0)
  , mChargeSpeed(0.0)
{
  mType = HSST_DIMENSIONALDRIVE;
  ADD_ATTRIBUTE("CHARGESPEED", AT_DOUBLE, mChargeSpeed)
  ADD_ATTRIBUTE("REQUIREDCHARGE", AT_INTEGER, mRequiredCharge)
  mMoreAllowed = false;
}

HSDimensionalDrive::~HSDimensionalDrive(void)
{
}

HSDimensionalDriveInstance::HSDimensionalDriveInstance(void)
  : mCurrentCharge(0.0)
  , mEngaged(false)
{
  mType = HSST_DIMENSIONALDRIVE;
  ADD_ATTRIBUTE_INHERIT("CHARGESPEED", AT_DOUBLE, mChargeSpeed)
  ADD_ATTRIBUTE_INHERIT("REQUIREDCHARGE", AT_INTEGER, mRequiredCharge)
  ADD_ATTRIBUTE("CURRENTCHARGE", AT_DOUBLE, mCurrentCharge)
  ADD_ATTRIBUTE("ENGAGED", AT_BOOLEAN, mEngaged)
}

HSDimensionalDriveInstance::~HSDimensionalDriveInstance(void)
{
}

void
HSDimensionalDriveInstance::Cycle()
{
  HSSystemInstance::Cycle();

  if (!GetCurrentPower()) {
    if (mEngaged) {
      Disengage();
    }
    mCurrentCharge = 0;
    return;
  }
  if (mEngaged) {
    return;
  }

  if (mCurrentCharge == GetRequiredCharge()) {
    return;
  }
  if (mCurrentCharge > GetRequiredCharge()) {
    mCurrentCharge = mRequiredCharge;
  }

  if (mCurrentCharge < GetRequiredCharge()) {
    double oldCharge = mCurrentCharge;
    mCurrentCharge += GetChargeSpeed(true);
    if (mCurrentCharge >= GetRequiredCharge()) {
      mCurrentCharge = GetRequiredCharge();
      char tbuf[128];
      sprintf(tbuf, "%s>%s %s now fully charged.", ANSI_GREEN, ANSI_NORMAL,
        GetName().c_str());
      mShip->NotifyConsoles(tbuf);     
      return;
    }
    for (int i = sHSConf.DDChargeNotifySteps; i > 0; i--) {
      if (oldCharge < (double)GetRequiredCharge() * ((1.0f / (double)sHSConf.DDChargeNotifySteps) * (double)i) &&
        mCurrentCharge >= (double)GetRequiredCharge() * ((1.0f / (double)sHSConf.DDChargeNotifySteps) * (double)i)) {
          char tbuf[128];
          sprintf(tbuf, "%s>%s %s now charged to %d%%", ANSI_GREEN, ANSI_NORMAL,
            GetName().c_str(), (int)((double)i * (100.0f / (double)sHSConf.DDChargeNotifySteps)));
          mShip->NotifyConsoles(tbuf);
          return;
      }
    }
  }
}

void
HSDimensionalDriveInstance::Engage()
{
  char tbuf[256];
  sprintf(tbuf, "%s engaging", GetName().c_str());
  mShip->NotifyConsolesFormatted("Navigation", tbuf);
  mShip->NotifySRooms(sHSConf.DDEngaged);
  mCurrentCharge = 0;
  mEngaged = true;
}

void
HSDimensionalDriveInstance::Disengage()
{
  char tbuf[256];
  sprintf(tbuf, "%s disengaging", GetName().c_str());
  mShip->NotifyConsolesFormatted("Navigation", tbuf);
  mShip->NotifySRooms(sHSConf.DDDisengaged);
  mEngaged = false;
}
