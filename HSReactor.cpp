/**
 *
 * Hemlock Space 5 (HSpace 5)
 * Copyright (c) 2009, Bas Schouten and Shawn Sagady
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 * 
 *    * Redistribution in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the HSpace 5 Development Team nor the names
 *      of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS  SOFTWARE  IS  PROVIDED  BY  THE HSPACE DEVELOPMENT TEAM AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FINTESS FOR A PARTICULAR
 * PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  COPYRIGHT  OWNERS OR
 * CONTRIBUTORS  BE  LIABLE  FOR  ANY  DIRECT,  INDIRECT, INCIDENTAL, SPECIAL
 * EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT  LIMITED TO,
 * PRODUCEMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR  BUSINESS  INTERUPTION)  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER  IN  CONTRACT,  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)  ARISING  IN  ANY  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Original author(s):
 *   Bas Schouten
 *
 */
#include "HSReactor.h"
#include "HSTools.h"
#include "HSShip.h"
#include "HSFuelTank.h"
#include <sstream>

using namespace std;

#define NOTIFICATIONSTEPS 10

HSReactor::HSReactor(void)
  : mMaxOutput(0)
  , mFuelEfficiency(0)
  , mChargeSpeed(0)
{
  ADD_ATTRIBUTE("MAXOUTPUT", AT_INTEGER, mMaxOutput)
  ADD_ATTRIBUTE("FUELEFFICIENCY", AT_DOUBLE, mFuelEfficiency)
  ADD_ATTRIBUTE("CHARGESPEED", AT_DOUBLE, mChargeSpeed)
  ADD_ATTRIBUTE("FUELTYPE", AT_STRING, mFuelType)
  mType = HSST_REACTOR;
}

HSReactor::~HSReactor(void)
{
}

std::string
HSReactor::PowerString(int aPower)
{
  string unit = " W";
  double displayed = aPower;

  if (displayed >= 1000000000) {
    displayed /= 1000000000;
    unit = " GW";
  } else if (displayed >= 1000000) {
    displayed /= 1000000;
    unit = " MW";
  } else if (displayed >= 1000) {
    unit = " kW";
    displayed /= 1000;
  }
  char tbuf[32];
  sprintf(tbuf, "%.3f", displayed);

  string retval = tbuf;
  if (retval.length() > 5) {
    retval = retval.substr(0, 5);
  }
  return retval.append(unit);
}

HSReactorInstance::HSReactorInstance()
  : mDesiredOutput(0)
  , mCurrentOutput(0)
{
  ADD_ATTRIBUTE("DESIREDOUTPUT", AT_INTEGER, mDesiredOutput)
  ADD_ATTRIBUTE("CURRENTOUTPUT", AT_DOUBLE, mCurrentOutput)
  ADD_ATTRIBUTE_INHERIT("MAXOUTPUT", AT_INTEGER, mMaxOutput)
  ADD_ATTRIBUTE_INHERIT("FUELEFFICIENCY", AT_DOUBLE, mFuelEfficiency)
  ADD_ATTRIBUTE_INHERIT("CHARGESPEED", AT_DOUBLE, mChargeSpeed)
  ADD_ATTRIBUTE_INHERIT("FUELTYPE", AT_STRING, mFuelType)
  mType = HSST_REACTOR;
}

HSReactorInstance::~HSReactorInstance()
{
}

std::string
HSReactorInstance::GetSystemStatus()
{
  std::stringstream stream;

  stream << HSSystemInstance::GetSystemStatus();

  std::stringstream fuelEff;
  fuelEff << (GetFuelEfficiency() * 3600000.00) << " l/kWh";
  char fuelTime[64];
  double totalFuel = 0;
  foreach(HSSystemInstance*, tankSys, mShip->FindSystemsByType(HSST_FUELTANK)) {
    HSFuelTankInstance *tank = static_cast<HSFuelTankInstance*>(tankSys);
    if (tank->GetFuelType() == GetFuelType()) {
      totalFuel += tank->GetCurrentFuel();
    }
  }
  
  sprintf(fuelTime, "%.2f hrs", (totalFuel / (GetFuelEfficiency() * mCurrentOutput)) / 3600.00);
  char tbuf[256];
  sprintf(tbuf, "%s|   %sMaximum Output: %s%-8s           %sCurrent Output:%s %-8s      %s|\n",
    ANSI_BLUE, ANSI_YELLOW, ANSI_NORMAL, HSReactor::PowerString(GetMaxOutput()).c_str(), 
    ANSI_YELLOW, ANSI_NORMAL, HSReactor::PowerString((int)GetCurrentOutput()).c_str(), ANSI_BLUE);
  stream << tbuf;
  sprintf(tbuf, "|     %sCharge Speed: %s%-10s         %sDesired Output:%s %-8s      %s|\n",
    ANSI_YELLOW, ANSI_NORMAL, HSReactor::PowerString((int)GetChargeSpeed()).append("/s").c_str(), 
    ANSI_YELLOW, ANSI_NORMAL, HSReactor::PowerString(GetDesiredOutput()).c_str(), ANSI_BLUE);
  stream << tbuf;
  sprintf(tbuf, "|  %sFuel Efficiency: %s%-15s %sCurrent fuel-time:%s %-12s  %s|\n",
    ANSI_YELLOW, ANSI_NORMAL, fuelEff.str().c_str(), 
    ANSI_YELLOW, ANSI_NORMAL, fuelTime, ANSI_BLUE);
  stream << tbuf;

  return stream.str();
}

void
HSReactorInstance::Cycle()
{
  if (mCurrentOutput > GetMaxOutput() * mCondition) {
    // Reactor is overloaded. Handle damage opportunity.
    double overload = (double)mCurrentOutput / ((double)GetMaxOutput() * mCondition) - 1.0;
    double chanceOfDamage = (overload * (1.0 / GetTolerance())) / 100;
    double random = (double)rand() / (double)RAND_MAX;
    if (random < chanceOfDamage) {
      double amount = ((double)rand() / (double)RAND_MAX) * chanceOfDamage * 2.0;
      mCondition -= amount;
      if (mCondition < 0) {
        mCondition = 0;
      }
      char tbuf[256];
      sprintf(tbuf, "%s has been damaged by system stress, condition now at %.0f%%.", GetName().c_str(), mCondition * 100.0);
      mShip->NotifyConsolesFormatted("DAMAGE", tbuf);
    }
  }
  if (GetFuelEfficiency() > 0) {
    std::vector<HSSystemInstance*> fuelTanks = 
      mShip->FindSystemsByType(HSST_FUELTANK);
    std::vector<HSFuelTankInstance*> myTanks;
    double fuelToConsume = mCurrentOutput * GetFuelEfficiency();
    foreach(HSSystemInstance*, tankSys, fuelTanks) {
      HSFuelTankInstance *tank = static_cast<HSFuelTankInstance*>(tankSys);
      if (tank->GetFuelType() == GetFuelType()) {
        if (fuelToConsume < tank->GetCurrentFuel()) {
          tank->SetCurrentFuel(tank->GetCurrentFuel() - fuelToConsume);
          fuelToConsume = 0;
          break;
        }
        fuelToConsume -= tank->GetCurrentFuel();
        tank->SetCurrentFuel(0);
      }
    }
    if (fuelToConsume > 0) {
      mShip->NotifyConsolesFormatted("Power", "Reactor shutting down - insufficient fuel");
      mCurrentOutput = 0;
      mDesiredOutput = 0;
      return;
    }
  }
  if (!mCurrentOutput && !mDesiredOutput) {
    return;
  }
  if (mDesiredOutput > GetMaxOutput() * (1.0 + GetMaxOverload()) * mCondition) {
    mDesiredOutput = (int)(GetMaxOutput() * (1.0 + GetMaxOverload()) * mCondition);
  }
  if (mCurrentOutput == mDesiredOutput) {
    return;
  }

  if (mDesiredOutput < mCurrentOutput) {
    mCurrentOutput = mDesiredOutput;
    char tbuf[128];
    sprintf(tbuf, "%s>%s %s output now at %s", ANSI_GREEN, ANSI_NORMAL,
      GetName().c_str(), HSReactor::PowerString(mDesiredOutput).c_str());
    mShip->NotifyConsoles(tbuf);
    return;
  }

  if (mDesiredOutput > mCurrentOutput) {
    double oldOutput = mCurrentOutput;
    mCurrentOutput += GetChargeSpeed();
    if (mCurrentOutput >= mDesiredOutput) {
      mCurrentOutput = mDesiredOutput;
      char tbuf[128];
      sprintf(tbuf, "%s>%s %s output now at %s", ANSI_GREEN, ANSI_NORMAL,
        GetName().c_str(), HSReactor::PowerString(mDesiredOutput).c_str());
      mShip->NotifyConsoles(tbuf);     
      return;
    }
    for (int i = NOTIFICATIONSTEPS; i > 0; i--) {
      if (oldOutput < (double)GetMaxOutput() * ((1.0f / (double)NOTIFICATIONSTEPS) * (double)i) &&
        mCurrentOutput >= (double)GetMaxOutput() * ((1.0f / (double)NOTIFICATIONSTEPS) * (double)i)) {
          char tbuf[128];
          sprintf(tbuf, "%s>%s %s output now up to %d%%", ANSI_GREEN, ANSI_NORMAL,
            GetName().c_str(), (int)((double)i * (100.0f / (double)NOTIFICATIONSTEPS)));
          mShip->NotifyConsoles(tbuf);
          return;
      }
    }
  }
}
