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
 * THIS  SOFTWARE IS PROVIDED BY THE HSPACE DEVELOPMENT TEAM AND CONTRIBUTORS
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

// Local
#include "HSSystem.h"
#include "HSReactor.h"
#include "HSComputer.h"
#include "HSSensor.h"
#include "HSEngine.h"
#include "HSThrusters.h"
#include "HSCommunications.h"
#include "HSTools.h"
#include "HSShield.h"
#include "HSBeamWeapon.h"
#include "HSFuelTank.h"
#include "HSAutopilot.h"
#include "HSCargoBay.h"
#include "HSHarvester.h"
#include "HSDimensionalDrive.h"
#include "HSCommodity.h"
#include "HSRepairSystem.h"
#include "HSShip.h"

// STL
#include <sstream>

HSSystem::HSSystem(void)
  : mID(0)
  , mType(0)
  , mOptimalPower(0)
  , mSize(0)
  , mMass(0)
  , mTolerance(1.0)
  , mMaxOverload(0)
  , mMoreAllowed(true)
  , mTakesMount(false)
{
  ADD_ATTRIBUTE_INTERNAL("ID", AT_INTEGER, mID)
  ADD_ATTRIBUTE_INTERNAL("TYPE", AT_INTEGER, mType)
  ADD_ATTRIBUTE("NAME", AT_STRING, mName)
  ADD_ATTRIBUTE("OPTIMALPOWER", AT_INTEGER, mOptimalPower)
  ADD_ATTRIBUTE("SIZE", AT_INTEGER, mSize)
  ADD_ATTRIBUTE("MASS", AT_INTEGER, mMass)
  ADD_ATTRIBUTE("MAXOVERLOAD", AT_DOUBLE, mMaxOverload)
  ADD_ATTRIBUTE("TOLERANCE", AT_DOUBLE, mTolerance)
}

HSSystem::~HSSystem(void)
{
}

std::vector<HSSystemType>
HSSystem::GetTypes()
{
  std::vector<HSSystemType> types;
  for (int i = 0; i != HSST_END; i++) {
    types.push_back((HSSystemType)i);
  }
  return types;
}

std::string
HSSystem::GetTableForType(HSSystemType aType)
{
  switch (aType) {
    case HSST_GENERIC:
      return "GENERIC";
    case HSST_REACTOR:
      return "REACTORS";
    case HSST_COMPUTER:
      return "COMPUTERS";
    case HSST_SENSOR:
      return "SENSORS";
    case HSST_ENGINE:
      return "ENGINES";
    case HSST_THRUSTERS:
      return "THRUSTERS";
    case HSST_COMMUNICATIONS:
      return "COMMUNICATIONS";
    case HSST_SHIELD:
      return "SHIELDS";
    case HSST_BEAMWEAPON:
      return "BEAMWEAPONS";
    case HSST_FUELTANK:
      return "FUELTANKS";
    case HSST_AUTOPILOT:
      return "AUTOPILOTS";
    case HSST_CARGOBAY:
      return "CARGOBAYS";
    case HSST_HARVESTER:
      return "HARVESTERS";
    case HSST_DIMENSIONALDRIVE:
      return "DIMENSIONALDRIVES";
    case HSST_REPAIRSYSTEM:
      return "REPAIRSYSTEMS";
    default:
      return "";
  }
}

std::string
HSSystem::GetNameForType(HSSystemType aType)
{
  switch (aType) {
    case HSST_GENERIC:
      return "Generic System";
    case HSST_REACTOR:
      return "Reactor";
    case HSST_COMPUTER:
      return "Computer";
    case HSST_SENSOR:
      return "Sensor";
    case HSST_ENGINE:
      return "Engine";
    case HSST_THRUSTERS:
      return "Thrusters";
    case HSST_COMMUNICATIONS:
      return "Communications";
    case HSST_SHIELD:
      return "Shield";
    case HSST_BEAMWEAPON:
      return "Beam Weapon";
    case HSST_FUELTANK:
      return "Fuel Tank";
    case HSST_AUTOPILOT:
      return "Autopilot";
    case HSST_CARGOBAY:
      return "Cargo Bay";
    case HSST_HARVESTER:
      return "Harvester";
    case HSST_DIMENSIONALDRIVE:
      return "Dimen. Drive";
    case HSST_REPAIRSYSTEM:
      return "Repair System";
    default:
      return "";
  }
}

HSSystem*
HSSystem::CreateSystemForType(HSSystemType aType)
{
  switch (aType) {
    case HSST_GENERIC:
      return new HSSystem();
    case HSST_REACTOR:
      return new HSReactor();
    case HSST_COMPUTER:
      return new HSComputer();
    case HSST_SENSOR:
      return new HSSensor();
    case HSST_ENGINE:
      return new HSEngine();
    case HSST_THRUSTERS:
      return new HSThrusters();
    case HSST_COMMUNICATIONS:
      return new HSCommunications();
    case HSST_SHIELD:
      return new HSShield();
    case HSST_BEAMWEAPON:
      return new HSBeamWeapon();
    case HSST_FUELTANK:
      return new HSFuelTank();
    case HSST_AUTOPILOT:
      return new HSAutopilot();
    case HSST_CARGOBAY:
      return new HSCargoBay();
    case HSST_HARVESTER:
      return new HSHarvester();
    case HSST_DIMENSIONALDRIVE:
      return new HSDimensionalDrive();
    case HSST_REPAIRSYSTEM:
      return new HSRepairSystem();
    default:
      return NULL;
  }
}

std::string
HSSystem::MassString(int aMass)
{
  std::string unit = " kg";
  double displayed = aMass;

  if (displayed >= 1000000000) {
    displayed /= 1000000000;
    unit = " Mt";
  } else if (displayed >= 1000000) {
    unit = " kt";
    displayed /= 1000000;
  } else if (displayed >= 1000) {
    unit = " t ";
    displayed /= 1000;
  }

  char tbuf[32];
  sprintf(tbuf, "%.3f", displayed);

  std::string retval = tbuf;
  if (retval.length() > 5) {
    retval = retval.substr(0, 5);
  }
  return retval.append(unit);
}

HSSystemInstance::HSSystemInstance()
  : mShipID(0)
  , mType(0)
  , mSystemID(0)
  , mCurrentPower(0)
  , mCondition(1.0f)
  , mOptimalPower(0)
{
  ADD_ATTRIBUTE_INTERNAL("SHIPID", AT_INTEGER, mShipID)
  ADD_ATTRIBUTE("SYSTEMID", AT_INTEGER, mSystemID)
  ADD_ATTRIBUTE("CURRENTPOWER", AT_INTEGER, mCurrentPower)
  ADD_ATTRIBUTE("CONDITION", AT_DOUBLE, mCondition)
  ADD_ATTRIBUTE_INHERIT("NAME", AT_STRING, mName)
  ADD_ATTRIBUTE_INHERIT("OPTIMALPOWER", AT_INTEGER, mOptimalPower)
  ADD_ATTRIBUTE_INHERIT("SIZE", AT_INTEGER, mSize)
  ADD_ATTRIBUTE_INHERIT("MASS", AT_INTEGER, mMass)
  ADD_ATTRIBUTE_INHERIT("TOLERANCE", AT_DOUBLE, mTolerance)
  ADD_ATTRIBUTE_INHERIT("MAXOVERLOAD", AT_DOUBLE, mMaxOverload)
}

HSSystemInstance::~HSSystemInstance()
{

}

HSSystemInstance*
HSSystemInstance::CreateSystemInstanceForType(HSSystemType aType)
{
  switch (aType) {
    case HSST_GENERIC:
      return new HSSystemInstance();
    case HSST_REACTOR:
      return new HSReactorInstance();
    case HSST_COMPUTER:
      return new HSComputerInstance();
    case HSST_SENSOR:
      return new HSSensorInstance();
    case HSST_ENGINE:
      return new HSEngineInstance();
    case HSST_THRUSTERS:
      return new HSThrustersInstance();
    case HSST_COMMUNICATIONS:
      return new HSCommunicationsInstance();
    case HSST_SHIELD:
      return new HSShieldInstance();
    case HSST_BEAMWEAPON:
      return new HSBeamWeaponInstance();
    case HSST_FUELTANK:
      return new HSFuelTankInstance();
    case HSST_AUTOPILOT:
      return new HSAutopilotInstance();
    case HSST_CARGOBAY:
      return new HSCargoBayInstance();
    case HSST_HARVESTER:
      return new HSHarvesterInstance();
    case HSST_DIMENSIONALDRIVE:
      return new HSDimensionalDriveInstance();
    case HSST_REPAIRSYSTEM:
      return new HSRepairSystemInstance();
    default:
      return NULL;
  }
}

std::string
HSSystemInstance::GetSystemStatus()
{
  std::stringstream stream;

  char tbuf[512];
//               ========================================|=======================================
  sprintf(tbuf, "%s|             %sSize: %s%4d m³                      %sMass:%s %-8s      %s|\n",
    ANSI_BLUE, ANSI_YELLOW, ANSI_NORMAL, GetSize(), ANSI_YELLOW, ANSI_NORMAL,
    HSSystem::MassString(GetMass()).c_str(), ANSI_BLUE);
  stream << tbuf;
  sprintf(tbuf, "|    %sOptimal Power: %s%-8s          %sAllocated Power:%s %-8s      %s|\n",
    ANSI_YELLOW, ANSI_NORMAL, HSReactor::PowerString(GetOptimalPower()).c_str(), 
    ANSI_YELLOW, ANSI_NORMAL, HSReactor::PowerString(GetCurrentPower()).c_str(), ANSI_BLUE);
  stream << tbuf;
  sprintf(tbuf, "|        %sCondition: %s%3d%%             %sMaximum Overload:%s %3d%%          %s|%s\n",
    ANSI_YELLOW, ANSI_NORMAL, (int)(GetCondition() * 100.0), ANSI_YELLOW,
    ANSI_NORMAL, (int)(GetMaxOverload() * 100.0), ANSI_BLUE, ANSI_NORMAL);
  stream << tbuf;

  return stream.str();
}

void
HSSystemInstance::Cycle()
{
  if (mCurrentPower > GetOptimalPower()) {
    // System is overloaded. Handle damage opportunity.
    double overload = (double)mCurrentPower / (double)GetOptimalPower() - 1.0;
    double chanceOfDamage = (overload * (1.0 / GetTolerance())) / 100;
    double random = (double)rand() / (double)RAND_MAX;
    if (random < chanceOfDamage) {
      double amount = ((double)rand() / (double)RAND_MAX) * chanceOfDamage * 2.0;
      mCondition -= amount;
      char tbuf[256];
      if (mCondition < 0) {
        mCondition = 0;
        mCurrentPower = 0;
      }
      sprintf(tbuf, "%s has been damaged by system stress, condition now at %.0f%%.", GetName().c_str(), mCondition * 100.0);
      mShip->NotifyConsolesFormatted("DAMAGE", tbuf);
    }
  }
}

bool
HSSystem::MoreAllowed()
{
  return mMoreAllowed;
}

bool
HSSystem::TakesMount()
{
  return mTakesMount;
}
