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
#include "HSShip.h"
#include "HSSystem.h"
#include "HSTools.h"
#include "HSConsole.h"
#include "HSReactor.h"
#include "HSEngine.h"
#include "HSDB.h"
#include "HSHullClass.h"
#include "HSThrusters.h"
#include "HSLandingLocation.h"
#include "HSIface.h"
#include "HSComputer.h"
#include "HSWeapon.h"
#include "HSShield.h"
#include "HSUniverse.h"
#include "HSConf.h"
#include "HSCargoBay.h"
#include "HSTerritory.h"
#include "HSDimensionalDrive.h"
#ifdef HS_SERVER
#include "HSServer.h"
#endif

using namespace std;

HSShip::HSShip(void)
  : mHullClass(0)
  , mCurrentHullPoints(0)
  , mHeading(0, 0)
  , mDesiredVelocity(0)
  , mLandedLoc(-1)
  , mLandedObj(-1)
  , mHatchLoc(-1)
  , mOnline(true)
  , mAccelerating(true)
  , mEnginesCut(false)
  , mLandingTimer(0)
{
  ADD_ATTRIBUTE_INTERNAL("HULLCLASS", AT_INTEGER, mHullClass)
  ADD_ATTRIBUTE_INTERNAL("HATCHLOC", AT_INTEGER, mHatchLoc)
  ADD_ATTRIBUTE_INTERNAL("SHIPROOMS", AT_INTLIST, mShipRooms)
  ADD_ATTRIBUTE_INTERNAL("LANDEDOBJ", AT_INTEGER, mLandedObj)
  ADD_ATTRIBUTE("CURRENTHULLPOINTS", AT_INTEGER, mCurrentHullPoints)
  ADD_ATTRIBUTE("HEADING", AT_VECTOR, mHeading)
  ADD_ATTRIBUTE("DESIREDHEADING", AT_VECTOR, mDesiredHeading)
  ADD_ATTRIBUTE("DESIREDVELOCITY", AT_DOUBLE, mDesiredVelocity)
  ADD_ATTRIBUTE("LANDEDLOC", AT_INTEGER, mLandedLoc)
  mType = HSOT_SHIP;
}

HSShip::~HSShip(void)
{
  foreach(HSConsole*, console, mConsoles) {
    delete console;
  }
  foreach(HSSystemInstance*, instance, mSystems) {
    delete instance;
  }
}

void
HSShip::Cycle()
{
  foreach(HSConsole*, console, mConsoles) {
    DBRef user = HSIFGetObjectLock(console->GetID(), HSLock_Use);
    if (user != console->GetID()) {
      if (HSIFGetObjectLocation(user) != HSIFGetObjectLocation(console->GetID())) {
        char tbuf[256];
        sprintf(tbuf, "%s unmans %s.", HSIFGetName(user).c_str(),
          HSIFGetName(console->GetID()).c_str());
        HSIFNotifyContentsExcept(HSIFGetObjectLocation(console->GetID()), tbuf);
        sprintf(tbuf, "You unman %s.", HSIFGetName(console->GetID()).c_str());
        HSIFNotify(user, tbuf);
        HSIFSetObjectLock(console->GetID(), HSLock_Use, console->GetID());
#ifdef HS_SERVER
        sServer->ConsoleUpdate(user);
#endif
      }
    }
  }
  if (!mOnline || !mCurrentHullPoints) {
    HSObject::Cycle();
    return;
  }
  CheckSystemPower();

  HSSystemInstance *computer = NULL;
  foreach(HSSystemInstance*, hssys, mSystems) {
    if (hssys->GetType() == HSST_COMPUTER) {
      // Computer must cycle at the end for all kinds of reasons!
      computer = hssys;
      continue;
    }
    hssys->Cycle();
  }
  if (computer) {
    computer->Cycle();
  }
  if (mLandedLoc == -1) {
    CycleThrusters();
    CycleEngines();
  } else if (mLandingTimer > 0) {
    CycleLanding();
  } else if (mLandingTimer < 0) {
    CycleLaunching();
  }

  // Save our old location for territory boundary checking.
  HSVector3D oldLocation = GetLocation();
  HSObject::Cycle();

  // We now check to see if we entered or left any territories.
  HSUniverse *univ = sHSDB.FindUniverse(GetUniverse());
  if (!univ) {
    HSLog() << "Error, universe not found for ship " << this;
    return;
  }
  foreach(HSTerritory*, territory, univ->GetTerritories()) {
    territory->ShipMoves(this, oldLocation);
  }
}

bool
HSShip::IsInSpace()
{
  return (mLandedLoc == -1) && GetCurrentHullPoints();
}

bool
HSShip::HasDDEngaged()
{
  if (!mOnline) {
    return false;
  }
  std::vector<HSSystemInstance*> dimensionalDrives =
    FindSystemsByType(HSST_DIMENSIONALDRIVE);
  if (dimensionalDrives.empty()) {
    return false;
  }
  return static_cast<HSDimensionalDriveInstance*>(*dimensionalDrives.begin())->GetEngaged();
}

std::vector<HSSystemInstance*>
HSShip::FindSystemsByType(HSSystemType aType)
{
  std::vector<HSSystemInstance*> systems;
  foreach(HSSystemInstance*, system, mSystems) {
    if (system->GetType() == aType) {
      systems.push_back(system);
    }
  }
  return systems;
}

void
HSShip::AddSystem(HSSystemInstance *aSystem)
{
  if (!aSystem) {
    return;
  }
  aSystem->SetShipID(mID);
  mSystems.push_back(aSystem);
}

bool
HSShip::RemoveSystemByName(std::string aName)
{
  for(std::vector<HSSystemInstance*>::iterator iter = mSystems.begin(); iter != mSystems.end(); iter++) {
    HSSystemInstance *inst = *iter;
    if (!strncasecmp(aName.c_str(), inst->GetName().c_str())) {
      mSystems.erase(iter);
      delete inst;
      return true;
    }
  }
  return false;
}

bool
HSShip::RemoveSystem(HSSystemInstance *aSystem)
{
  for(std::vector<HSSystemInstance*>::iterator iter = mSystems.begin(); iter != mSystems.end(); iter++) {
    HSSystemInstance *inst = *iter;
    if (inst == aSystem) {
      mSystems.erase(iter);
      delete inst;
      return true;
    }
  }
  return false;
}

void
HSShip::AddConsole(HSConsole *aConsole)
{
  if (!aConsole) {
    return;
  }
  aConsole->SetShipID(mID);
  mConsoles.push_back(aConsole);
}

bool
HSShip::RemoveConsoleByID(int aID)
{
  for(std::vector<HSConsole*>::iterator iter = mConsoles.begin(); iter != mConsoles.end(); iter++) {
    HSConsole *console = *iter;
    if (console->GetID() == aID) {
      mConsoles.erase(iter);
      delete console;
      return true;
    }
  }
  return false;
}

HSConsole*
HSShip::FindConsoleByID(int aID)
{
  for(std::vector<HSConsole*>::iterator iter = mConsoles.begin(); iter != mConsoles.end(); iter++) {
    HSConsole *console = *iter;
    if (console->GetID() == aID) {
      return console;
    }
  }
  return 0;
}

std::vector<HSSystemInstance*>
HSShip::FindSystemsByName(std::string aName)
{
  std::vector<HSSystemInstance*> retval;
  for(std::vector<HSSystemInstance*>::iterator iter = mSystems.begin(); iter != mSystems.end(); iter++) {
    HSSystemInstance *inst = *iter;
    if (!strncasecmp(aName.c_str(), inst->GetName().c_str())) {
      retval.push_back(inst);
    }
  }
  return retval;
}

HSSystemInstance*
HSShip::FindSystemByName(std::string aName)
{
  std::vector<HSSystemInstance*> retval;
  std::string name = aName.substr(0, aName.find("/"));
  for(std::vector<HSSystemInstance*>::iterator iter = mSystems.begin(); iter != mSystems.end(); iter++) {
    HSSystemInstance *inst = *iter;
    if (!strncasecmp(name.c_str(), inst->GetName().c_str())) {
      retval.push_back(inst);
    }
  }
  if (retval.size() == 0) {
    return 0;
  }
  if (retval.size() == 1) {
    return *retval.begin();
  }
  if ((int)aName.find("/") != -1) {
    int pos = atoi(aName.substr(aName.find("/") + 1).c_str());
    if ((size_t)pos >= retval.size()) {
      return 0;
    }
    return retval[pos];
  }
  return *retval.begin();
}

void
HSShip::NotifyConsoles(std::string aMsg)
{
  foreach(HSConsole*, console, mConsoles) {
    console->NotifyUser(aMsg);
  }
}

void
HSShip::NotifyConsolesFormatted(std::string aSource, std::string aMsg)
{
  char tbuf[512];
  sprintf(tbuf, "%s[%s%s%s]%s %s", ANSI_BLUE, ANSI_YELLOW, aSource.c_str(),
    ANSI_BLUE, ANSI_NORMAL, aMsg.c_str());
  NotifyConsoles(tbuf);
}

void
HSShip::NotifySRooms(std::string aMsg)
{
  foreach(int, room, mShipRooms) {
    HSIFNotifyContentsExcept(room, aMsg);
  }
}

void
HSShip::NotifySurroundingSpace(int aDet, std::string aMsg)
{
  /// \todo Implement me!
}

int
HSShip::GetPowerOutput()
{
  int retval = 0;
  foreach(HSSystemInstance*, system, mSystems) {
    if (system->GetType() == HSST_REACTOR) {
      HSReactorInstance *reactor = (HSReactorInstance*)system;
      retval += (int)reactor->GetCurrentOutput();
    }
  }
  return retval;
}

int
HSShip::GetAvailablePower()
{
  int retval = 0;
  foreach(HSSystemInstance*, system, mSystems) {
    if (system->GetType() == HSST_REACTOR) {
      HSReactorInstance *reactor = (HSReactorInstance*)system;
      retval += (int)reactor->GetCurrentOutput();
    }
    retval -= system->GetCurrentPower();
  }
  return retval;
}

int
HSShip::GetTotalMass()
{
  int retval = mHull->GetMass();
  foreach(HSSystemInstance*, system, mSystems) {
    retval += system->GetMass();
    if (system->GetType() == HSST_CARGOBAY) {
      HSCargoBayInstance *bay = static_cast<HSCargoBayInstance*>(system);
      retval += (int)bay->GetCargoMass();
    }
  }
  foreach(HSShip*, ship, mLandedShips) {
    retval += ship->GetTotalMass();
  }
  return retval;
}

int
HSShip::GetMaxVelocity()
{
  std::vector<HSSystemInstance*> engines = FindSystemsByType(HSST_ENGINE);
  if (engines.empty()) {
    return 0;
  }

  double totalSpeed = 0.0;
  foreach(HSSystemInstance*, engineSys, engines) {
    HSEngineInstance *engine = static_cast<HSEngineInstance*>(engineSys);
    // We use the slowest engine on the ship for the maxium speed.
    // The rest will only contribute thrust on that speed.
    if ((engine->GetMaxSpeed(true) < totalSpeed || totalSpeed == 0.0) && 
      engine->GetCurrentPower() > 0.0) {
        totalSpeed = engine->GetMaxSpeed(true);
    }
  }
  return (int)totalSpeed;
}

std::vector<HSSystemInstance*>
HSShip::GetSystems()
{
  return mSystems;
}

void
HSShip::CycleEngines()
{
  if (!mAccelerating || mEnginesCut || HasDDEngaged()) {
    return;
  }
  if (GetVelocity().zero() && GetDesiredVelocity() == 0) {
    if (GetHeading() == GetDesiredHeading()) {
      NotifyConsolesFormatted("Engines", "Ship now at full stop");
    }
    mAccelerating = false;
    return;
  }
  std::vector<HSSystemInstance*> engines = FindSystemsByType(HSST_ENGINE);
  if (engines.empty()) {
    return;
  }
  double currentSpeed = mVelocity.length();
  if (GetVelocity().normalized() == GetHeading() && 
    COMPARE_DOUBLE(currentSpeed, GetDesiredVelocity(), 0.001) &&
    GetDesiredVelocity() != 0) {
      char tbuf[256];
      sprintf(tbuf, "Ship now travelling %s/s heading %s",
        HSObject::DistanceString(currentSpeed).c_str(),
        GetVelocity().HeadingString().c_str());
      NotifyConsolesFormatted("Engines", tbuf);
      mAccelerating = false;
      return;
  }
  if (GetDesiredVelocity() > GetMaxVelocity()) {
    SetDesiredVelocity(GetMaxVelocity());
  }
  if (sHSConf.RevertThrust > 0.0) {
    if (GetDesiredVelocity() < (-GetMaxVelocity() * sHSConf.RevertThrust)) {
      SetDesiredVelocity(-GetMaxVelocity() * sHSConf.RevertThrust);
    }
  }
  double totalThrust = 0;
  int totalMass = GetTotalMass();

  foreach(HSSystemInstance*, engineSys, engines) {
    HSEngineInstance *engine = static_cast<HSEngineInstance*>(engineSys);

    if (engine->ConsumeFuel()) {
      totalThrust += engine->GetThrust(true);
    }
  }
  if (!totalThrust) {
    NotifyConsolesFormatted("Engines", "No thrust available now drifting");
    mEnginesCut = true;
    mAccelerating = false;
    return;
  }
  if (!totalMass) {
    // Eep, div by zero coming up.
    return;
  }
  double acceleration = totalThrust / (double)totalMass; // a = F/m


  if (GetDesiredVelocity() < 0) {
    if (mVelocity.length() <= -GetDesiredVelocity()) {
      HSVector3D reverseHeading;
      reverseHeading -= GetHeading();
      mVelocity += reverseHeading * acceleration;
    }
    if (mVelocity.length() > -(GetDesiredVelocity() - acceleration)) {
      mVelocity = mVelocity.normalized() * (mVelocity.length() - acceleration);
    } else if (mVelocity.length() > -GetDesiredVelocity()) {
      mVelocity = mVelocity.normalized() * -GetDesiredVelocity();
    }
  } else {
    if (mVelocity.length() <= GetDesiredVelocity()) {
      mVelocity += GetHeading() * acceleration;
    }
    if (mVelocity.length() > (GetDesiredVelocity() + acceleration)) {
      HSVector3D newDirection = mVelocity + (GetHeading() * acceleration);
      mVelocity = newDirection.normalized() * (mVelocity.length() - acceleration);
    } else if (mVelocity.length() > GetDesiredVelocity()) {
      HSVector3D newDirection = mVelocity + (GetHeading() * acceleration);
      mVelocity = newDirection.normalized() * GetDesiredVelocity();
    }
  }
}

void
HSShip::CycleThrusters()
{
  if (GetHeading() == GetDesiredHeading()) {
    return;
  }
  if (HasDDEngaged()) {
    return;
  }
  double angularAccel = 0;
  std::vector<HSSystemInstance*> thrusters = FindSystemsByType(HSST_THRUSTERS);
  foreach(HSSystemInstance*, thrusterSys, thrusters) {
    HSThrustersInstance *thruster = static_cast<HSThrustersInstance*>(thrusterSys);
    angularAccel += thruster->GetAngularVelocity(true);
  }
  if (!GetTotalMass()) {
    // Eep, we weight nothing, get out, and avoid div by 0.
    return;
  }
  angularAccel /= sqrt((double)GetTotalMass());
  double headXY = mHeading.HeadingXY();
  double headZ = mHeading.HeadingZ();
  double destXY = mDesiredHeading.HeadingXY();
  double destZ = mDesiredHeading.HeadingZ();
  if (headXY > destXY) {
    if ((headXY - destXY) < 180.00) {
      headXY -= angularAccel;
      if (headXY < destXY) {
        headXY = destXY;
      }
    } else {
      headXY += angularAccel;
      if (headXY >= 360.00) {
        headXY -= 360.00;
        if (headXY > destXY) {
          headXY = destXY;
        }
      }
    }
  } else if (headXY < destXY) {
    if ((destXY - headXY) < 180.00) {
      headXY += angularAccel;
      if (headXY > destXY) {
        headXY = destXY;
      }
    } else {
      headXY -= angularAccel;
      if (headXY < 0) {
        headXY += 360.00;
        if (headXY < destXY) {
          headXY = destXY;
        }
      }
    }
  }
  if (headZ > destZ) {
    if ((headZ - destZ) < 180.00) {
      headZ -= angularAccel;
      if (headZ < destZ) {
        headZ = destZ;
      }
    } else {
      headZ += angularAccel;
      if (headZ >= 360.00) {
        headZ -= 360.00;
        if (headZ > destZ) {
          headZ = destZ;
        }
      }
    }
  } else if (headZ < destZ) {
    if ((destZ - headZ) < 180.00) {
      headZ += angularAccel;
      if (headZ > destZ) {
        headZ = destZ;
      }
    } else {
      headZ -= angularAccel;
      if (headZ < 0) {
        headZ += 360.00;
        if (headZ < destZ) {
          headZ = destZ;
        }
      }
    }
  }
  mHeading = HSVector3D(headXY, headZ);
}

void
HSShip::CycleLanding()
{
  HSLandingLocation *landingLoc = sHSDB.FindLandingLocation(mLandedLoc);
  if (!landingLoc) {
    return;
  }
  HSObject *obj = sHSDB.FindObject(landingLoc->GetObjectID());
  if (!obj) {
    return;
  }
  if(mLandingTimer == 10) {
    if (obj->GetType() == HSOT_PLANET) {
      NotifySRooms("The ship rumbles gently as it enters the upper atmosphere");
    }
  } else if(mLandingTimer == 5) {
    if (obj->GetType() == HSOT_PLANET) {
      HSIFNotifyContentsExcept(landingLoc->GetID(), "You see a vessel coming in from the sky.");
    } else if (obj->GetType() == HSOT_SHIP) {
      HSIFNotifyContentsExcept(landingLoc->GetID(), "You see a ship entering the hangar bay.");
    }
  } else if (mLandingTimer == 1) {
    char tbuf[256];
    sprintf(tbuf, "The %s gently settles onto the surface", GetName().c_str());
    HSIFNotifyContentsExcept(landingLoc->GetID(), tbuf);
    sprintf(tbuf, "You feel the %s gently settle onto its landing area surface", GetName().c_str());
    NotifySRooms(tbuf);
    sprintf(tbuf, "Now landed on %s", obj->GetName().c_str());
    NotifyConsolesFormatted("Navigation", tbuf);
    HSIFTeleportObject(mID, landingLoc->GetID());
    SetLandedObj(obj->GetID());
    obj->mLandedShips.push_back(this);
  }
  --mLandingTimer;
}

void
HSShip::CycleLaunching()
{
  HSLandingLocation *landingLoc = sHSDB.FindLandingLocation(mLandedLoc);
  if (!landingLoc) {
    return;
  }
  HSObject *obj = sHSDB.FindObject(landingLoc->GetObjectID());
  if (!obj) {
    return;
  }
  if(mLandingTimer == -10) {
    char tbuf[256];
    sprintf(tbuf, "The %s begins to lift off from the surface", GetName().c_str());
    HSIFNotifyContentsExcept(landingLoc->GetID(), tbuf);
    sprintf(tbuf, "You feel the %s gently lift off from the landing area", GetName().c_str());
    NotifySRooms(tbuf);
  } else if(mLandingTimer == -5) {
    char tbuf[256];
    if (obj->GetType() == HSOT_PLANET) {
      sprintf(tbuf, "The %s slowly disappears into the sky", GetName().c_str());
      HSIFNotifyContentsExcept(landingLoc->GetID(), tbuf);
    } else if (obj->GetType() == HSOT_SHIP) {
      sprintf(tbuf, "You see the %s leave the hangar bay", GetName().c_str());
      HSIFNotifyContentsExcept(landingLoc->GetID(), tbuf);
    }
  } else if (mLandingTimer == -1) {
    SetLandedLoc(-1);
    SetLandedObj(-1);
    SetLocation(obj->GetLocation());
    for (std::vector<HSShip*>::iterator iter = obj->mLandedShips.begin();
      iter != obj->mLandedShips.end(); iter++) {
        if (*iter == this) {
          obj->mLandedShips.erase(iter);
          break;
        }
    }
    HSIFTeleportObject(mID, mUniverse);
    NotifyConsolesFormatted("Navigation", "Launch complete ...");
    HSUniverse *univ = sHSDB.FindUniverse(GetUniverse());
    if (univ) {
      HSStoredPose pose;
      pose.type = "Sensors";
      pose.source = this;
      pose.nominative = obj;
      pose.pose = "emerges from the surface of";
      univ->PushPose(pose);
    }
  }
  ++mLandingTimer;
}

void
HSShip::CheckSystemPower()
{
  int totalDesOutput = 0;
  int totalPower = GetPowerOutput();
  int powerUsed = 0;
  foreach(HSSystemInstance*, system, mSystems) {
    powerUsed += system->GetCurrentPower();
    if (system->GetType() == HSST_REACTOR) {
      HSReactorInstance *reactor = static_cast<HSReactorInstance*>(system);
      totalDesOutput += reactor->GetDesiredOutput();
    }
  }

  // Nothing going on here, don't cycle until something happens.
  if (powerUsed == 0 && totalDesOutput == 0) {
    mOnline = false;
    return;
  }

  if (powerUsed <= totalPower) {
    return;
  }
  HSSystemInstance *computer;

  char tbuf[256];
  foreach(HSSystemInstance*, system, mSystems) {
    // Leave computer for last.
    if (system->GetType() != HSST_COMPUTER) {
      if (system->GetCurrentPower() > 0) {
        powerUsed -= system->GetCurrentPower();
        system->SetCurrentPower(0);
        sprintf(tbuf, "Insufficient power supply, %s shutdown",
          system->GetName().c_str());
        NotifyConsolesFormatted("Power Management", tbuf);
        if (powerUsed <= totalPower) {
          return;
        }
      }
    } else {
      computer = system;
    }
  }
  sprintf(tbuf, "Insufficient power supply, %s shutdown",
    computer->GetName().c_str());
  NotifyConsolesFormatted("Power Management", tbuf);
  computer->SetCurrentPower(0);
}

void
HSShip::DestroyShip()
{
  mCurrentHullPoints = 0;
  NotifyConsolesFormatted("The End", "The vessel is destroyed");
  NotifySRooms(sHSConf.DestructionString);
  HSUniverse *myUniv = sHSDB.FindUniverse(mUniverse);
  ASSERT(myUniv);
  myUniv->NotifyObjectPose(this, "Combat", "explodes in a ball of fire!");  
}

HSDamageResult
HSShip::HandleDamage(double aDamage, HSVector3D aDirection)
{
  HSShieldMount shieldArc = HSSM_FORE;

  int direction = (int)aDirection.HeadingXY();

  if (direction >= 315 && direction < 45) {
    shieldArc = HSSM_FORE;
  } else if (direction >= 45 && direction < 135) {
    shieldArc = HSSM_STARBOARD;
  } else if (direction >= 135 && direction < 225) {
    shieldArc = HSSM_AFT;
  } else if (direction >= 225 && direction < 315) {
    shieldArc = HSSM_PORT;
  }

  std::vector<HSShieldInstance*> applicableShields;
  foreach(HSSystemInstance*, system, mSystems) {
    if (system->GetType() == HSST_SHIELD) {
      HSShieldInstance *shield = static_cast<HSShieldInstance*>(system);
      if (shield->GetMountPoint() == shieldArc) {
        applicableShields.push_back(shield);
      }
    }
  }
  double damageLeft = aDamage;
  HSUniverse *univ = sHSDB.FindUniverse(mUniverse);
  if (!univ) {
    HSLog() << this << "Ship with invalid universe set.";
    return HSDR_NONE;
  }
  double shieldModifier = univ->GetShieldModifier(mLocation);

  if (shieldModifier == 0.0) {
    foreach(HSShieldInstance*, shield, applicableShields) {
      shield->SetCurrentAbsorption(0);
    }
  } else {
    damageLeft /= shieldModifier;
    foreach(HSShieldInstance*, shield, applicableShields) {
      if (shield->GetCurrentAbsorption() >= damageLeft) {
        shield->SetCurrentAbsorption(shield->GetCurrentAbsorption() - damageLeft);
        NotifyConsolesFormatted("Combat", "Incoming damage absorbed by shields");
        return HSDR_SHIELD;
      }
      damageLeft -= shield->GetCurrentAbsorption();
      shield->SetCurrentAbsorption(0);
    }
    damageLeft *= shieldModifier;
  }

  mCurrentHullPoints -= (int)damageLeft;

  NotifyConsolesFormatted("Combat", "Hull integrity has been damage");
  if (mCurrentHullPoints <= 0) {
    DestroyShip();
    return HSDR_HULL;
  }
  
  foreach(HSSystemInstance*, system, mSystems) {
    double chance = (double)rand() / RAND_MAX;
    if (chance <= (double)damageLeft / mCurrentHullPoints) {
      char tbuf[256];
      double damage = ((double)rand() / RAND_MAX) * damageLeft;
      double conditionDeterioration = damage / mCurrentHullPoints;
      double newCondition = HS_MAX(system->GetCondition() - conditionDeterioration, 0);
      system->SetCondition(newCondition);
      if (newCondition > 0) {
        sprintf(tbuf, "%s damaged, condition now at %.0f%%",
          system->GetName().c_str(), system->GetCondition() * 100.00);
      } else {
        sprintf(tbuf, "%s damaged, system inoperable",
          system->GetName().c_str());
        system->SetCurrentPower(0);
      }
      NotifyConsolesFormatted("DAMAGE", tbuf);
    }
  }
  return HSDR_HULL;
}

void
HSShip::HandleWeaponImpact(double aDamage, HSWeaponInstance *aWeapon)
{
  NotifySRooms(sHSConf.WeaponsHitString);
  char strWarning[64];
  sprintf(strWarning, "%c%s!WARNING!", BEEP_CHAR, ANSI_RED);
  HSComputerInstance *computer = FindComputer();
  if (computer) {
    HSSensorContact contact;
    char tbuf[256];
    if (!computer->FindSensorContactByObjectID(aWeapon->GetShipID(), &contact)) {
      sprintf(tbuf, "Incoming weapons fire from an unkown origin has hit");
    } else if (contact.Detail > 50) {
      sprintf(tbuf, "Incoming weapons fire from the %s (%d) has hit", 
        contact.Object->GetName().c_str(), contact.ID);
    } else {
      sprintf(tbuf, "Incoming weapons fire from contact %d has hit", 
        contact.ID);
    }
    NotifyConsolesFormatted(strWarning, tbuf);
  }
  HSUniverse *myUniv = sHSDB.FindUniverse(mUniverse);
  ASSERT(myUniv);
  HSDamageResult res = HandleDamage(aDamage, aWeapon->GetShip()->GetLocation() - GetLocation());
  myUniv->NotifyWeaponsFire(aWeapon->GetShip(), this, res);
}

void
HSShip::HandleWeaponMiss(HSWeaponInstance *aWeapon)
{
  HSComputerInstance *computer = FindComputer();
  if (!computer) {
    return;
  }
  HSSensorContact contact;
  char tbuf[256];
  if (!computer->FindSensorContactByObjectID(aWeapon->GetShipID(), &contact)) {
    sprintf(tbuf, "Incoming weapons fire from an unkown origin has missed");
  } else if (contact.Detail > 50) {
    sprintf(tbuf, "Incoming weapons fire from the %s (%d) has missed", 
      contact.Object->GetName().c_str(), contact.ID);
  } else {
    sprintf(tbuf, "Incoming weapons fire from contact %d has missed", 
      contact.ID);
  }
  char strWarning[64];
  sprintf(strWarning, "%c%s!WARNING!", BEEP_CHAR, ANSI_RED);
  NotifyConsolesFormatted(strWarning, tbuf);
}

HSComputerInstance*
HSShip::FindComputer()
{
  std::vector<HSSystemInstance*> systems = FindSystemsByType(HSST_COMPUTER);
  if (systems.empty()) {
    return 0;
  }
  // Take the first computer found.
  return static_cast<HSComputerInstance*>(*systems.begin());
}

HSShip*
HSShip::Clone()
{
  // Start cloning, the ship object first.
  DBRef clone = HSIFCloneThing(GetID());
  if (clone == DBRefNothing) {
    return NULL;
  }

  std::map<DBRef, DBRef> clonedStuff;

  clonedStuff[GetID()] = clone;

  foreach(int, room, GetShipRooms()) {
    HSIFCloneRoom(room, &clonedStuff);
  }

  HSShip *clonedShip = new HSShip();
  HSUniverse *univ = sHSDB.FindUniverse(GetUniverse());
  ASSERT(univ);

  univ->AddObject(clonedShip);
  HSObject::Clone(clonedShip);
  HSHullClass *hullClass = sHSDB.FindHullClass(clonedShip->GetHullClass());
  ASSERT(hullClass);
  SetHull(hullClass);
  clonedShip->SetID(clone);
  if (GetHatchLoc() != DBRefNothing) {
    if (clonedStuff.count(GetHatchLoc()) == 1) {
      clonedShip->SetHatchLoc(clonedStuff[GetHatchLoc()]);
    }
  }
  foreach(HSConsole*, console, mConsoles) {
    if (clonedStuff.count(console->GetID()) == 1) {
      HSConsole *newConsole = new HSConsole();
      newConsole->SetID(clonedStuff[console->GetID()]);
      newConsole->SetShip(clonedShip);
      newConsole->SetShipID(clonedShip->GetID());
      clonedShip->AddConsole(newConsole);
    }
  }
  foreach(HSLandingLocation*, landingLocation, GetLandingLocations()) {
    if (clonedStuff.count(landingLocation->GetID()) == 1) {
      HSLandingLocation *newLandingLoc = new HSLandingLocation();
      newLandingLoc->SetID(clonedStuff[landingLocation->GetID()]);
      newLandingLoc->SetSize(landingLocation->GetSize());
      newLandingLoc->SetObjectID(clonedShip->GetID());
      clonedShip->AddLandingLocation(newLandingLoc);
    }
  }
  std::vector<int> srooms;
  foreach(int, shipRoom, GetShipRooms()) {
    if (clonedStuff.count(shipRoom) == 1) {
      srooms.push_back(clonedStuff[shipRoom]);      
    }
  }
  clonedShip->SetShipRooms(srooms);

  foreach(HSSystemInstance*, system, GetSystems()) {
    HSSystemInstance *clonedSystem = 
      HSSystemInstance::CreateSystemInstanceForType((HSSystemType)system->GetType());
    system->Clone(clonedSystem);
    clonedSystem->SetSystem(system->GetSystem());
    clonedSystem->SetShip(clonedShip);
    clonedShip->AddSystem(clonedSystem);
  }
  
  return clonedShip;
}

std::string
HSShip::GetAdditionalInfo()
{
  string retval;
  if (!mSystems.empty()) {
    retval.append("------------------------------------------------------------------------------\n");
    retval.append("System Name                      System Type\n");
    retval.append("-------------------------------- --------------------\n");
    char tbuf[256];
    foreach(HSSystemInstance*, system, mSystems) {
      sprintf(tbuf, "%-32s %-20s\n", system->GetName().c_str(),
        HSSystem::GetNameForType((HSSystemType)system->GetType()).c_str());
      retval.append(tbuf);
    }
  }
  return retval;
}

std::vector<Attribute>
HSShip::GetExtraAttributes()
{
  std::vector<Attribute> retval;
  mConsoleDBRefs.clear();
  foreach(HSConsole*, console, mConsoles) {
    mConsoleDBRefs.push_back(console->GetID());
  }
  Attribute attr = { "CONSOLES", AT_INTLIST, true, &mConsoleDBRefs, NULL };

  retval.push_back(attr);
  foreach(Attribute, parentAttr, HSObject::GetExtraAttributes()) {
    retval.push_back(parentAttr);
  }
  return retval;
}

void
HSShip::AttributeChanged(std::string aName)
{
  if (aName == "HULLCLASS") {
    HSHullClass *hullClass = sHSDB.FindHullClass(mHullClass);
    mHull = hullClass;
  } else if (aName == "LANDEDLOC") {
    HSIFTeleportObject(mID, mLandedLoc);
  }

  HSObject::AttributeChanged(aName);
}
