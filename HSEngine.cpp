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
#include "HSEngine.h"
#include "HSTools.h"
#include "HSShip.h"
#include "HSFuelTank.h"

#include <sstream>

HSEngine::HSEngine(void)
  : mThrust(0)
  , mMaxSpeed(0)
  , mFuelEfficiency(0)
{
  ADD_ATTRIBUTE("THRUST", AT_INTEGER, mThrust)
  ADD_ATTRIBUTE("MAXSPEED", AT_INTEGER, mMaxSpeed)
  ADD_ATTRIBUTE("FUELEFFICIENCY", AT_DOUBLE, mFuelEfficiency)
  ADD_ATTRIBUTE("FUELTYPE", AT_STRING, mFuelType)
  mType = HSST_ENGINE;
}

HSEngine::~HSEngine(void)
{
}

HSEngineInstance::HSEngineInstance(void)
{
  ADD_ATTRIBUTE_INHERIT("THRUST", AT_INTEGER, mThrust)
  ADD_ATTRIBUTE_INHERIT("MAXSPEED", AT_INTEGER, mMaxSpeed)
  ADD_ATTRIBUTE_INHERIT("FUELEFFICIENCY", AT_DOUBLE, mFuelEfficiency)
  ADD_ATTRIBUTE_INHERIT("FUELTYPE", AT_STRING, mFuelType)
  mType = HSST_ENGINE;
}

HSEngineInstance::~HSEngineInstance(void)
{
}

bool
HSEngineInstance::ConsumeFuel()
{
  if (!GetCurrentPower()) {
    return false;
  }
  if (GetFuelEfficiency() > 0) {
    std::vector<HSSystemInstance*> fuelTanks = 
      mShip->FindSystemsByType(HSST_FUELTANK);
    std::vector<HSFuelTankInstance*> myTanks;
    double fuelToConsume = (double)GetThrust(true) * GetFuelEfficiency();
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
      char tbuf[256];
      sprintf(tbuf, "%s shutting down - insufficient fuel", GetName().c_str());
      mShip->NotifyConsolesFormatted("Engines", tbuf);
      mCurrentPower = 0;
      return false;
    }
  }
  return true;
}

std::string
HSEngineInstance::GetSystemStatus()
{
  std::stringstream stream;

  stream << HSSystemInstance::GetSystemStatus();

  std::stringstream fuelEff;
  fuelEff << (GetFuelEfficiency() * 1000.00) << " l/kN";
  char fuelTime[64];
  double totalFuel = 0;
  foreach(HSSystemInstance*, tankSys, mShip->FindSystemsByType(HSST_FUELTANK)) {
    HSFuelTankInstance *tank = static_cast<HSFuelTankInstance*>(tankSys);
    if (tank->GetFuelType() == GetFuelType()) {
      totalFuel += tank->GetCurrentFuel();
    }
  }
  
  sprintf(fuelTime, "%.2f hrs", (totalFuel / (GetFuelEfficiency() * GetThrust(true))) / 3600.00);
  char thrust[64];
  sprintf(thrust, "%d N", GetThrust(true));
  char tbuf[256];
  sprintf(tbuf, "%s|    %sEngine Thrust: %s%-8s                %sMax Speed:%s %-12s  %s|\n",
    ANSI_BLUE, ANSI_YELLOW, ANSI_NORMAL, thrust, 
    ANSI_YELLOW, ANSI_NORMAL, HSObject::DistanceString(GetMaxSpeed(true)).append("/s").c_str(), ANSI_BLUE);
  stream << tbuf;
  sprintf(tbuf, "|  %sFuel Efficiency: %s%-15s %sCurrent fuel-time:%s %-12s  %s|\n",
    ANSI_YELLOW, ANSI_NORMAL, fuelEff.str().c_str(), 
    ANSI_YELLOW, ANSI_NORMAL, fuelTime, ANSI_BLUE);
  stream << tbuf;

  return stream.str();
}
