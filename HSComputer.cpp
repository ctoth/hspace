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
#include "HSGlobal.h"
#include "HSIface.h"
#include "HSCargoBay.h"
#include "HSCargoContainer.h"
#include "HSComputer.h"
#include "HSConsole.h"
#include "HSTools.h"
#include "HSSensor.h"
#include "HSShip.h"
#include "HSShield.h"
#include "HSHullClass.h"
#include "HSReactor.h"
#include "HSUniverse.h"
#include "HSEngine.h"
#include "HSFuelTank.h"
#include "HSDB.h"
#include "HSLandingLocation.h"
#include "HSPlanet.h"
#include "HSBeamWeapon.h"
#include "HSAutopilot.h"
#include "HSConf.h"
#include "HSJumpGate.h"
#include "HSThrusters.h"
#include "HSHarvester.h"
#include "HSResource.h"
#include "HSCargoPod.h"
#include "HSDockingHatch.h"
#include "HSNebula.h"

#include <sstream>
#include <algorithm>

using namespace std;

HSComputer::HSComputer(void)
  : mSystemsSupported(0)
{
  ADD_ATTRIBUTE("SYSTEMSSUPPORTED", AT_INTEGER, mSystemsSupported)
  mType = HSST_COMPUTER;
  mMoreAllowed = false;
}

HSComputer::~HSComputer(void)
{
}

HSComputerInstance::HSComputerInstance()
{
  mType = HSST_COMPUTER;
}

HSComputerInstance::~HSComputerInstance()
{
}

void
HSComputerInstance::Cycle()
{
  if (!IsOnline()) {
    // We're offline, we're not doing anything!
    return;
  }

  if (mSensorContacts.empty()) {
    return;
  }
  ///
  /// \todo WOW! This is really ineficient my brain's not working but there
  /// has to be better ways to do this. Optimize sometime.
  for(std::vector<HSSensorContact>::iterator iter = mSensorContacts.begin();
    iter != mSensorContacts.end(); iter++) {
      if (!(*iter).Refreshed) {
        char tbuf[256];
        HSObject *obj = iter->Object;
        sprintf(tbuf, "Contact (%d) has disappeared from sensors", (*iter).ID);
        mShip->NotifyConsolesFormatted("Sensors", tbuf);
        mSensorContacts.erase(iter);
        foreach(HSSystemInstance*, system, mShip->FindSystemsByType(HSST_BEAMWEAPON)) {
          HSBeamWeaponInstance *beam = static_cast<HSBeamWeaponInstance*>(system);
          if (beam->GetTarget() == obj) {
            beam->SetTarget(NULL);
          }
        }
        if (mSensorContacts.empty()) {
          return;
        }
        // Start again from the start
        iter = mSensorContacts.begin();
        continue;
      }
  }
  if (mSensorContacts.empty()) {
    return;
  }
  for(std::vector<HSSensorContact>::iterator iter = mSensorContacts.begin();
    iter != mSensorContacts.end(); iter++) {
      (*iter).Refreshed = false;
  }
}

bool
HSComputerInstance::IsOnline()
{
  if (GetOptimalPower() == GetCurrentPower()) {
    return true;
  } else {
    return false;
  }
}

const std::vector<HSSensorContact>&
HSComputerInstance::GetSensorContacts()
{
  return mSensorContacts;
}

bool
HSComputerInstance::FindSensorContact(int aID, HSSensorContact *aContact)
{
  foreach(HSSensorContact, contact, mSensorContacts) {
    if (contact.ID == aID) {
      *aContact = contact;
      return true;
    }
  }
  return false;
}

bool
HSComputerInstance::FindSensorContactByObjectID(int aID, HSSensorContact *aContact)
{
  foreach(HSSensorContact, contact, mSensorContacts) {
    if (contact.Object->GetID() == aID) {
      *aContact = contact;
      return true;
    }
  }
  return false;
}

void
HSComputerInstance::Land(HSConsole *aConsole, int aContact, int aLocation)
{
  if (mShip->GetLandingTimer() != 0) {
    aConsole->NotifyUserSimple("Ship is currently in landing/launching procedures");
    return;
  }
  HSSensorContact contact;
  if (!FindSensorContact(aContact, &contact)) {
    aConsole->NotifyUserSimple("No such contact found on sensors");
    return;
  }
  
  HSObject *obj = contact.Object;

  ASSERT(obj);

  HSVector3D diffVector = obj->GetLocation() - mShip->GetLocation();
  if (diffVector.length() > 5) {
    aConsole->NotifyUserSimple("You have to be within 5 km to initiate landing");
    return;
  }

  if (mShip->GetVelocity().length() > 0) {
    aConsole->NotifyUserSimple("You cannot land while moving.");
    return;
  }
  
  std::vector<HSLandingLocation*> landingLocs = obj->GetLandingLocations();

  if ((unsigned int)aLocation >= landingLocs.size()) {
    aConsole->NotifyUserSimple("No such landing location available");
    return;
  }

  if (!landingLocs[aLocation]->GetOpen()) {
    aConsole->NotifyUserSimple("That landing location is closed.");
    return;
  }
  if (landingLocs[aLocation]->GetSpaceTaken() + mShip->GetSize() >
    landingLocs[aLocation]->GetSize()) {
      aConsole->NotifyUserSimple("Not enough space to land there.");
      return;
  }

  bool hasEngine = false;
  std::vector<HSSystemInstance*> engines = mShip->FindSystemsByType(HSST_ENGINE);
  foreach(HSSystemInstance*, engine, engines) {
    if (engine->GetCurrentPower() > 0) {
      hasEngine = true;
      break;
    }
  }
  if (!hasEngine) {
    aConsole->NotifyUserSimple("No engine system powered.");
    return;
  }
  bool hasThruster = false;
  std::vector<HSSystemInstance*> thrusters = mShip->FindSystemsByType(HSST_THRUSTERS);
  foreach(HSSystemInstance*, thruster, thrusters) {
    if (thruster->GetCurrentPower() > 0) {
      hasThruster = true;
      break;
    }
  }
  if (!hasThruster) {
    aConsole->NotifyUserSimple("No thrusters powered.");
    return;
  }

  HSLandingLocation *landingLoc = landingLocs[aLocation];

  ASSERT(landingLoc);

  mShip->SetLandedLoc(landingLoc->GetID());
  mShip->SetLandingTimer(20);
  char tbuf[256];
  sprintf(tbuf, "Initiating landing on contact %d (%s)", contact.ID, obj->GetName().c_str());
  mShip->NotifyConsolesFormatted("Navigation", tbuf);
  HSUniverse *univ = sHSDB.FindUniverse(mShip->GetUniverse());
  if (univ) {
    univ->NotifyObjectAffectPose(mShip, "begins its approach to the surface of", obj, "Sensors");
  }
}

void
HSComputerInstance::Launch(HSConsole *aConsole)
{
  if (mShip->GetLandedLoc() == -1) {
    aConsole->NotifyUserSimple("Ship is not currently landed.");
    return;
  }
  if (mShip->GetLandingTimer() != 0) {
    aConsole->NotifyUserSimple("Ship is currently in landing/launching procedures");
    return;
  }
  HSLandingLocation *landingLoc = sHSDB.FindLandingLocation(mShip->GetLandedLoc());
  if (!landingLoc) {
    aConsole->NotifyUserSimple("You cannot take off from here.");
    return;
  }
  bool hasEngine = false;
  std::vector<HSSystemInstance*> engines = mShip->FindSystemsByType(HSST_ENGINE);
  foreach(HSSystemInstance*, engine, engines) {
    if (engine->GetCurrentPower() > 0) {
      hasEngine = true;
      break;
    }
  }
  if (!hasEngine) {
    aConsole->NotifyUserSimple("No engine system powered.");
    return;
  }
  bool hasThruster = false;
  std::vector<HSSystemInstance*> thrusters = mShip->FindSystemsByType(HSST_THRUSTERS);
  foreach(HSSystemInstance*, thruster, thrusters) {
    if (thruster->GetCurrentPower() > 0) {
      hasThruster = true;
      break;
    }
  }
  if (!hasThruster) {
    aConsole->NotifyUserSimple("No thrusters powered.");
    return;
  }
  mShip->SetLandingTimer(-10);
  mShip->NotifyConsolesFormatted("Navigation", "Launching vessel ...");
}
    
void
HSComputerInstance::DisplayNavigationReport(HSConsole *aConsole)
{
  stringstream notification;
  char tbuf[256];
  char tbuf2[64];
  char tbuf3[64];
  char stopDist[64];
  char turnRate[64];
  // Calculate turning rate and stop distance:
  double totalThrust = 0;
  double totalMass = mShip->GetHull()->GetMass();
  double totalAngular = 0;
  foreach(HSSystemInstance*, system, mShip->GetSystems()) {
    totalMass += system->GetMass();
    if (system->GetType() == HSST_ENGINE) {
      HSEngineInstance *engine = static_cast<HSEngineInstance*>(system);
      totalThrust += engine->GetThrust(true);
    }
    if (system->GetType() == HSST_CARGOBAY) {
      HSCargoBayInstance *bay = static_cast<HSCargoBayInstance*>(system);
      totalMass += bay->GetCargoMass();
    }
    if (system->GetType() == HSST_THRUSTERS) {
      HSThrustersInstance *thrusters = static_cast<HSThrustersInstance*>(system);
      totalAngular = thrusters->GetAngularVelocity(true);
    }
  }
  double angularRate;
  if (!totalMass) {
    angularRate = totalAngular;
  } else {
    angularRate = totalAngular / sqrt(totalMass);  
  }
  if (angularRate > 360) {
    angularRate = 360;
  }
  double accel;
  if (!totalMass) {
    accel = totalThrust;
  } else {
    accel = totalThrust / totalMass;
  }
  if (mShip->GetVelocity().length() > 0) {
    double distToStop = pow(mShip->GetVelocity().length() + (accel / 2.0), 2) / (2.0 * accel);
    sprintf(stopDist, "%s", HSObject::DistanceString(distToStop).c_str());
  } else {
    sprintf(stopDist, "N/A");
  }
  sprintf(turnRate, "%d deg/s", (int)angularRate);

  /* Begin the navigation report */
  sprintf(tbuf, "%s.----------------------------------------------------------------------------.\n", ANSI_BLUE);
  notification << tbuf;
  sprintf(tbuf, "|                             %sNavigation Status                              %s|\n", ANSI_NORMAL,
    ANSI_BLUE);
  notification << tbuf;
  sprintf(tbuf, " >------------------------.------------------------.------------------------<\n");
  notification << tbuf;
  sprintf(tbuf, "|         %sHEADING         %s|        %sVELOCITY        %s|        %sTELEMETRY        %s|\n", ANSI_YELLOW,
    ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE);
  notification << tbuf;
  sprintf(tbuf, " >------------------------+------------------------+-------------------------|\n");
  notification << tbuf;
  sprintf(tbuf2, "%s/s", HSObject::DistanceString(mShip->GetDesiredVelocity()).c_str());
  sprintf(tbuf, "| %sDESIRED%s: %s%-14s %s| %sDESIRED%s: %s%-13s %s| %sSTOPDST%s: %s%-14s %s|\n", ANSI_YELLOW, ANSI_BLUE,
    ANSI_GREEN, mShip->GetDesiredHeading().HeadingString().c_str(), ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL, tbuf2,
    ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL, stopDist, ANSI_BLUE); 
  notification << tbuf;
  sprintf(tbuf2, "%s/s", HSObject::DistanceString(mShip->HasDDEngaged() ? mShip->GetVelocity().length() * sHSConf.DDMultiplier : mShip->GetVelocity().length()).c_str());
  sprintf(tbuf, "| %sCURRENT%s: %s%-14s %s| %sCURRENT%s: %s%-13s %s| %sTURN RT%s: %s%-14s %s|\n", ANSI_YELLOW, ANSI_BLUE,
    ANSI_GREEN, mShip->GetHeading().HeadingString().c_str(), ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL, tbuf2, ANSI_BLUE,
    ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL, turnRate, ANSI_BLUE);
  notification << tbuf;
  sprintf(tbuf2, "%s/s", HSObject::DistanceString(mShip->GetMaxVelocity()).c_str());
  sprintf(tbuf3, "%s/s", HSObject::DistanceString(accel).c_str());
  sprintf(tbuf, "|  %sVECTOR%s: %s%-14s %s| %sMAXIMUM%s: %s%-13s %s|   %sACCEL%s: %s%-14s %s|\n", ANSI_YELLOW, ANSI_BLUE,
    ANSI_GREEN, mShip->GetVelocity().HeadingString().c_str(), ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL, tbuf2, ANSI_BLUE,
    ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL, tbuf3, ANSI_BLUE);
  notification << tbuf;
  sprintf(tbuf, "'-------------------------^------------------------^-------------------------'%s\n", ANSI_NORMAL);
  notification << tbuf;
  // Send All to user.
  aConsole->NotifyUser(notification.str());
}

void
HSComputerInstance::DisplayStatus(HSConsole *aConsole)
{
  stringstream notification;
  char tbuf[256];
  char tbuf2[64];
  char tbuf3[64];
  char xbuf[64]; // X Coordinate Buffer since these are going to be tricky to format right.
  char ybuf[64]; // Y Coordinate Buffer
  char zbuf[64]; // Z Coordinate Buffer
  char lpad[64]; // Left Pad for text centering
  size_t len;
  size_t pos;

  // Figure out some stuff we will use later in the display like coordinates and status displays that require
  // more advanced formatting.
  
  sprintf(xbuf, "%.1f", mShip->GetLocation().mX);
  sprintf(ybuf, "%.1f", mShip->GetLocation().mY);
  sprintf(zbuf, "%.1f", mShip->GetLocation().mZ);

  /* Start the real status report */
  sprintf(tbuf, "%s.----------------------------------------------------------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s|  %sShip Status                                   %26s  %s|%s\n",ANSI_BLUE, ANSI_NORMAL, mShip->GetName().c_str(), ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s.\\---------[ %sX %s]----------.---------[ %sY %s]----------.---------[ %sZ %s]----------/.%s\n", ANSI_BLUE, ANSI_YELLOW,
	      ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;

  // Create the Coordinate Line, each coordinate needs to be centered in its own section
  
  // Center the X Coordinate
  len = strlen(xbuf); 
  pos = 12 - len / 2;
  // If length is odd we need to -1 from the position for formatting.
  if ( len % 2 != 0 ) {
	pos = pos -1;
  }
  size_t i; 
  for(i = 0; i < pos; i++) {
    lpad[i] = ' ';
  }
  lpad[i] = '\0';
  sprintf(tbuf2, "%s%s", lpad, xbuf);
  sprintf(tbuf, "%s| %s%-24s%s|", ANSI_BLUE, ANSI_GREEN, tbuf2, ANSI_BLUE);
  notification << tbuf;
  
  // Center the Y Coordinate
  len = strlen(ybuf);
  pos = 12 - len / 2;
  if ( len % 2 != 0 ) {
    pos = pos -1;
  }
  for(i = 0;i < pos; i++) {
    lpad[i] = ' ';
  }
  lpad[i] = '\0';
  sprintf(tbuf2, "%s%s", lpad, ybuf);
  sprintf(tbuf, "%s%-24s%s|", ANSI_GREEN, tbuf2, ANSI_BLUE);
  notification << tbuf;
  
  // Center the Z Coordinate
  len = strlen(zbuf);
  pos = 12 - len / 2;
  if ( len % 2 != 0 ) {
    pos = pos -1;
  }
  for(i = 0;i < pos; i++) {
    lpad[i] = ' ';
  }
  lpad[i] = '\0';
  sprintf(tbuf2, "%s%s", lpad, zbuf);
  sprintf(tbuf, "%s%-24s%s |%s\n", ANSI_GREEN, tbuf2, ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s >------------------------+------------------------+------------------------<%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s|        %sNAVIGATION       %s|        %sVELOCITY        %s|       %sENGINEERING       %s|%s\n", ANSI_BLUE,
    ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf2, "%s/s", HSObject::DistanceString(mShip->HasDDEngaged() ? mShip->GetVelocity().length() * sHSConf.DDMultiplier : mShip->GetVelocity().length()).c_str());
  sprintf(tbuf, "%s| %sHEADING%s: %s%-14s %s| %sCURRENT%s: %s%-13s %s| %sHULL%s: %s %s|%s\n", ANSI_BLUE, ANSI_YELLOW,
    ANSI_BLUE, ANSI_GREEN, mShip->GetHeading().HeadingString().c_str() ,ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_GREEN, tbuf2,
    ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, HSMeter(17, mShip->GetHull()->GetHullPoints(), mShip->GetCurrentHullPoints()).c_str(), 
    ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf2, "%s/s", HSObject::DistanceString(mShip->GetMaxVelocity()).c_str());
  sprintf(tbuf3, "%d", mShip->GetTotalMass());
  sprintf(tbuf, "%s|  %sVECTOR%s: %s%-14s %s| %sMAXIMUM%s: %s%-13s %s| %sMASS%s: %s%-17s %s|%s\n", ANSI_BLUE, ANSI_YELLOW,
    ANSI_BLUE, ANSI_GREEN, mShip->GetVelocity().HeadingString().c_str(), ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_GREEN, 
    tbuf2, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_GREEN, tbuf3, ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;

  // Calculate turning rate and stop distance:
  double totalThrust = 0;
  double totalMass = mShip->GetHull()->GetMass();
  double totalAngular = 0;
  foreach(HSSystemInstance*, system, mShip->GetSystems()) {
    totalMass += system->GetMass();
    if (system->GetType() == HSST_ENGINE) {
      HSEngineInstance *engine = static_cast<HSEngineInstance*>(system);
      totalThrust += engine->GetThrust(true);
    }
    if (system->GetType() == HSST_CARGOBAY) {
      HSCargoBayInstance *bay = static_cast<HSCargoBayInstance*>(system);
      totalMass += bay->GetCargoMass();
    }
    if (system->GetType() == HSST_THRUSTERS) {
      HSThrustersInstance *thrusters = static_cast<HSThrustersInstance*>(system);
      totalAngular = thrusters->GetAngularVelocity(true);
    }
  }
  double angularRate;
  if (!totalMass) {
    angularRate = totalAngular;
  } else {
    angularRate = totalAngular / sqrt(totalMass);  
  }
  if (angularRate > 360) {
    angularRate = 360;
  }
  double accel;
  if (!totalMass) {
    accel = totalThrust;
  } else {
    accel = totalThrust / totalMass;
  }
  if (mShip->GetVelocity().length() > 0) {
    double distToStop = pow(mShip->GetVelocity().length() + (accel / 2.0), 2) / (2.0 * accel);
    sprintf(tbuf3, "%s", HSObject::DistanceString(distToStop).c_str());
  } else {
    sprintf(tbuf3, "N/A");
  }
  sprintf(tbuf2, "%d deg/s", (int)angularRate);
  sprintf(tbuf, "%s| %sTURN RT%s: %s%-14s %s|%sSTOPDIST%s: %s%-13s %s|-------------------------'%s\n", ANSI_BLUE, ANSI_YELLOW,
    ANSI_BLUE, ANSI_GREEN, tbuf2, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_GREEN, 
    tbuf3, ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;

  sprintf(tbuf, "%s'-------------------------^-----------------------/%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;

  std::vector<HSSystemInstance*> sysShields = mShip->FindSystemsByType(HSST_SHIELD);    // Load Shields
  std::vector<HSSystemInstance*> sysTanks = mShip->FindSystemsByType(HSST_FUELTANK);    // Load Fuel Tanks
  std::vector<HSSystemInstance*> sysCargoBay = mShip->FindSystemsByType(HSST_CARGOBAY); // Load Cargo bays
  size_t syscount;
  if (sysShields.size() || sysTanks.size()) {
    std::vector<std::string> fuelout;
    std::vector<std::string> shieldout;

    // Pre-Create the fuel section
    if(!sysTanks.size()) {
      // Leaving fuelout blank.
    } else {
      sprintf(tbuf, "%s.-------------------------------------------------. ", ANSI_BLUE);
      fuelout.push_back(tbuf);
      sprintf(tbuf, "%s|                    %sFUEL STATUS                  %s| ", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE);
      fuelout.push_back(tbuf);
      sprintf(tbuf, "%s >----------------.----------------.-------------<  ", ANSI_BLUE);
      fuelout.push_back(tbuf);
      sprintf(tbuf, "%s|      %sTANK       %s|      %sFUEL      %s|    %s LEVEL    %s| ", ANSI_BLUE, ANSI_YELLOW,
        ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE);
      fuelout.push_back(tbuf);
      sprintf(tbuf, "%s|-----------------+----------------+--------------| ", ANSI_BLUE);
      fuelout.push_back(tbuf);

      foreach(HSSystemInstance*, system, sysTanks){
        HSFuelTankInstance *tank = static_cast<HSFuelTankInstance*>(system);
        sprintf(tbuf, "%s| %s%-15s %s| %s%-14s %s| %s %s| ", ANSI_BLUE, ANSI_WHITE, tank->GetName().substr(0,15).c_str(),
          ANSI_BLUE, ANSI_WHITE, tank->GetFuelType().substr(0,14).c_str(), ANSI_BLUE, 
          HSMeter(12, tank->GetMaxFuel(), tank->GetCurrentFuel()).c_str(), ANSI_BLUE);
        fuelout.push_back(tbuf);
      }
      sprintf(tbuf, "%s`-----------------^----------------^--------------' ", ANSI_BLUE);
      fuelout.push_back(tbuf);
    }

    // Pre-Create the shield section.
    if(!sysShields.size()) {
      // Leaving it blank.
    } else {
      sprintf(tbuf, "%s.------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
      shieldout.push_back(tbuf);
      sprintf(tbuf, "%s|         %sSHIELDS        %s|%s\n", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
      shieldout.push_back(tbuf);
      sprintf(tbuf, "%s >----------------------<%s\n", ANSI_BLUE, ANSI_NORMAL);
      shieldout.push_back(tbuf);
      double foreMax = 0;
      double foreCurrent = 0;
      double aftMax = 0;
      double aftCurrent = 0;
      double portMax = 0;
      double portCurrent = 0;
      double starMax = 0;
      double starCurrent = 0;
      foreach(HSSystemInstance*, system, sysShields) {
        HSShieldInstance *shield = static_cast<HSShieldInstance*>(system);
        switch(shield->GetMountPoint()) {
          case 0:
            foreMax += shield->GetMaxAbsorption();
            foreCurrent += shield->GetCurrentAbsorption();
            break;
          case 1:
            aftMax += shield->GetMaxAbsorption();
            aftCurrent += shield->GetCurrentAbsorption();
            break;
          case 2:
            portMax += shield->GetMaxAbsorption();
            portCurrent += shield->GetCurrentAbsorption();
            break;
          case 3:
            starMax += shield->GetMaxAbsorption();
            starCurrent += shield->GetCurrentAbsorption();
            break;
        }
      }
      if(foreMax > 0) {
        sprintf(tbuf, "%s| %sF%s: %s %s|%s\n", ANSI_BLUE, ANSI_WHITE, ANSI_BLUE,
          HSMeter(19, foreMax, foreCurrent,"_").c_str(),
          ANSI_BLUE, ANSI_NORMAL);
        shieldout.push_back(tbuf);
      }
      if(aftMax > 0) {
        sprintf(tbuf, "%s| %sA%s: %s %s|%s\n", ANSI_BLUE, ANSI_WHITE, ANSI_BLUE,
          HSMeter(19, aftMax, aftCurrent,"_").c_str(),
          ANSI_BLUE, ANSI_NORMAL);
        shieldout.push_back(tbuf);
      }
      if(portMax > 0) {
        sprintf(tbuf, "%s| %sP%s: %s %s|%s\n", ANSI_BLUE, ANSI_WHITE, ANSI_BLUE,
          HSMeter(19, portMax, portCurrent,"_").c_str(),
          ANSI_BLUE, ANSI_NORMAL);
        shieldout.push_back(tbuf);
      }
      if(starMax > 0) {
        sprintf(tbuf, "%s| %sS%s: %s %s|%s\n", ANSI_BLUE, ANSI_WHITE, ANSI_BLUE,
          HSMeter(19, starMax, starCurrent,"_").c_str(),
          ANSI_BLUE, ANSI_NORMAL);
        shieldout.push_back(tbuf);
      }
      sprintf(tbuf, "%s'------------------------'%s\n", ANSI_BLUE, ANSI_NORMAL);
      shieldout.push_back(tbuf);
    }

    // Put it all together
    if( shieldout.size() > fuelout.size()) {
      syscount = shieldout.size();
    } else {
      syscount = fuelout.size();
    }
    for(int i=0; (size_t)i < syscount; i++) {
      if(i >= (int)fuelout.size()) {
        sprintf(tbuf, "                                                    ");
      } else {
        sprintf(tbuf, "%s", fuelout[i].c_str());
      }
      notification << tbuf;

      if(i >= (int)shieldout.size()) {
        sprintf(tbuf, "%s\n", ANSI_NORMAL);
      } else {
        sprintf(tbuf, "%s", shieldout[i].c_str());
      }
      notification << tbuf;
    }
  }

  // Create a litte information about the cargo system.
  if(!sysCargoBay.size()) {
    // If there are no cargo bays we dont need to print anything.
  } else {
    // Get our numbers together for the readout.
    int cargoSpace = 0;
    int cargoFreeSpace = 0;
    double cargoMass = 0.0;

    foreach(HSSystemInstance*, system, sysCargoBay){
      HSCargoBayInstance *bay = static_cast<HSCargoBayInstance*>(system);
      cargoSpace += bay->GetSpace();
      cargoMass += bay->GetCargoMass();
      cargoFreeSpace += bay->GetFreeSpace();
    }
    sprintf(tbuf, "%s.----------------------------------.-----------------------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s| %sCARGO CAPACITY%s: %s %s| %sTOTAL CARGO MASS%s: %s%-21.2f %s|%s\n", ANSI_BLUE, ANSI_YELLOW,
      ANSI_BLUE, HSMeter(16, cargoSpace, (cargoSpace - cargoFreeSpace)).c_str(), ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_WHITE, cargoMass, ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s`----------------------------------^-----------------------------------------'%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
  }


  // Send it all to the user
  aConsole->NotifyUser(notification.str());
}

void
HSComputerInstance::DisplayPowerStatus(HSConsole *aConsole)
{
  stringstream notification;
  char tbuf[256];
  sprintf(tbuf, "%s.------------------------------------------------------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s|%s Ship Power Status %52s %s|%s\n", ANSI_BLUE, ANSI_YELLOW, mShip->GetName().c_str(), ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s >-------------------------------.-------------.-------------.----------<%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s|%s System%s                         |  %sCondition%s  | %sMax/Optimal %s|  %sCurrent  %s|%s\n", ANSI_BLUE, 
    ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s >-------------------------------+-------------+-------------+----------<%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  foreach(HSSystemInstance*, system, mShip->GetSystems()) {
    if (system->GetType() == HSST_REACTOR) {
      HSReactorInstance *reactor = static_cast<HSReactorInstance*>(system);
      sprintf(tbuf, "%s|%s %-30s %s|%s%s|%s   %8s+ %s|%s %8s+ %s|%s\n", ANSI_BLUE, ANSI_NORMAL,
        system->GetName().c_str(), ANSI_BLUE, HSMeter(13, 1.0, system->GetCondition()).c_str(), ANSI_BLUE, ANSI_NORMAL, HSReactor::PowerString(reactor->GetMaxOutput()).c_str(),
        ANSI_BLUE, ANSI_NORMAL, HSReactor::PowerString((int)reactor->GetCurrentOutput()).c_str(), ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    } else if (system->GetOptimalPower() > 0) {
      sprintf(tbuf, "%s|%s %-30s %s|%s%s|%s   %8s- %s|%s %8s- %s|%s\n", ANSI_BLUE, ANSI_NORMAL,
        system->GetName().c_str(), ANSI_BLUE, HSMeter(13, 1.0, system->GetCondition()).c_str(), ANSI_BLUE, ANSI_NORMAL, HSReactor::PowerString((int)system->GetOptimalPower()).c_str(),
        ANSI_BLUE, ANSI_NORMAL, HSReactor::PowerString(system->GetCurrentPower()).c_str(), ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    }
  }
  sprintf(tbuf, "%s >-------------------------------^-------------^-------------+----------<%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s| %sTotal Available%s                                            |%s %9s %s|%s\n", 
    ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL, HSReactor::PowerString(mShip->GetAvailablePower()).c_str(), ANSI_BLUE,
    ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s| %sTotal Supply%s                                               |%s %9s %s|%s\n", 
    ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL, HSReactor::PowerString(mShip->GetPowerOutput()).c_str(), ANSI_BLUE,
    ANSI_NORMAL);
  notification << tbuf;

  sprintf(tbuf, "%s'------------------------------------------------------------^-----------'%s", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;  
  aConsole->NotifyUser(notification.str());
}

bool
ContactCloser(const HSSensorContact &aContact1, const HSSensorContact &aContact2)
{
  HSVector3D vector1 = aContact1.DiffVector;
  HSVector3D vector2 = aContact2.DiffVector;
  HSObject *object1 = aContact1.Object;
  HSObject *object2 = aContact2.Object;

  bool object1Closer = vector1.length() < vector2.length();
  int object1Type = object1->GetType();
  int object2Type = object2->GetType();

  if (object1Type == object2Type) {
    return object1Closer;
  } else if(object2Type == HSOT_PLANET) {
    return false;
  } else if(object2Type == HSOT_SHIP) {
    if(object1Type == HSOT_PLANET) {
      return true;
    } else {
      return false;
    }
  } else {
    if (object1Type == HSOT_SHIP || object1Type == HSOT_PLANET) {
      return true;
    } else {
      return object1Closer;
    }
  }
}

void
HSComputerInstance::DisplaySensorReport(HSConsole *aConsole)
{
  stringstream notification;
  char tbuf[256];
  char tbuf2[64];
  char tbuf3[64];

  HSUniverse *univ = sHSDB.FindUniverse(mShip->GetUniverse());
  if (!univ) {
    // Eeep! We're not in an existing universe, bail out.
    return;
  }
  // Add check to see if our sensors are powered up befor giving this report.
  std::vector<HSSystemInstance*> sysSensors = mShip->FindSystemsByType(HSST_SENSOR);
  bool sensorStatus = 0;
  foreach(HSSystemInstance*, system, sysSensors){
    HSSensorInstance *sensor = static_cast<HSSensorInstance*>(system);
    if (sensor->GetCurrentPower() > 0) {
      sensorStatus = 1;
      break;
    }
  }
  if(!sensorStatus) {
    aConsole->NotifyUserSimple("Sensors are offline!");
    return;
  }
  if(mShip->GetLandedLoc() != -1) {
    aConsole->NotifyUserSimple("Sensors will not operate while landed.");
    return;
  }
  if(mShip->GetLandingTimer() != 0) {
    aConsole->NotifyUserSimple("Sensors will not operate while ship is landing/launching.");
    return;
  }
  sprintf(tbuf, "%s.----------------------------------------------------------------------------.%s\n",
    ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s|%s[ID] Name                 Det Bearing Distance Heading Speed      Type      %s|%s\n",
    ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s >--------------------------------------------------------------------------<%s\n",
    ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  std::sort(mSensorContacts.begin(), mSensorContacts.end(), ContactCloser);
  bool seperatorDisplayed = false;
  foreach(HSSensorContact, contact, mSensorContacts) {
    HSObject *object = contact.Object;
    if (!object) {
      HSLog() << "Weird... NULL object in sensor contacts.";
      // Weeeeeird.
      continue;
    }
    
    char type[64];
    if (object->GetType() == HSOT_SHIP) {
      HSShip *ship = static_cast<HSShip*>(object);
      if (ship->FindSystemsByType(HSST_ENGINE).empty()) {
        sprintf(type, "Base");
      } else {
        sprintf(type, "Ship");
      }
    } else {
      sprintf(type, "%s", HSObject::GetNameForType((HSObjectType)object->GetType()).c_str());
    }
    HSVector3D diffVector = object->GetLocation() - mShip->GetLocation();    
    if (!seperatorDisplayed &&
      object->GetType() != HSOT_PLANET &&
      mSensorContacts.begin()->Object->GetType() == HSOT_PLANET)
    {
      sprintf(tbuf, "%s >--------------------------------------------------------------------------<%s\n", ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
      seperatorDisplayed = true;
    }
    if (object->GetType() == HSOT_SHIP || object->GetType() == HSOT_CARGOPOD) {
      // If the ship is not moving we don't know its heading.
      // As well if the ship is beyond a certain range we dont know its speed or heading.
      if(contact.Detail >= 30 && object->GetVelocity().length() > 0) {
        sprintf(tbuf2, "%3dm%-3d", (int)object->GetVelocity().HeadingXY(), (int)object->GetVelocity().HeadingZ());
        sprintf(tbuf3, "%s", HSObject::DistanceString(object->GetVelocity().length()).append("/s").c_str());
      } else {
        sprintf(tbuf2, " ");
        sprintf(tbuf3, " ");
      }
      
      sprintf(tbuf, "%s|%s%4d %-20s %3d %3dm%-3d %8s %-7s %-10s %-10s%s|%s\n",
        ANSI_BLUE, ANSI_NORMAL, contact.ID, contact.Detail > 50 ? object->GetName().c_str() : 
        type, 
        (int)contact.Detail, (int)diffVector.HeadingXY(), 
        (int)diffVector.HeadingZ(), HSObject::DistanceString(diffVector.length()).c_str(),
        tbuf2, tbuf3,
        type, ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    } else {
      sprintf(tbuf, "%s|%s%4d %-20s %3d %3dm%-3d %8s                    %-10s%s|%s\n",
        ANSI_BLUE, ANSI_NORMAL, contact.ID, contact.Detail > 50 ? object->GetName().c_str() : 
        type, (int)contact.Detail, (int)diffVector.HeadingXY(),
        (int)diffVector.HeadingZ(), HSObject::DistanceString(diffVector.length()).c_str(),
        type, ANSI_BLUE, ANSI_NORMAL);
        notification << tbuf;
    }
  }
  sprintf(tbuf, "%s'----------------------------------------------------------------------------'%s\n",
    ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  aConsole->NotifyUser(notification.str());
}

void
HSComputerInstance::DisplayCargoReport(HSConsole *aConsole)
{
  std::vector<HSCargoBayInstance*> bays;
  foreach(HSSystemInstance*, system, mShip->FindSystemsByType(HSST_CARGOBAY)) {
    bays.push_back(static_cast<HSCargoBayInstance*>(system));
  }
  if (!bays.size()) {
    aConsole->NotifyUserSimple("No cargo bays on this vessel.");
  }
  stringstream notification;
  char tbuf[256];
  sprintf(tbuf, "%s.--------------------------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s|%s Ship Cargo Status %24s %s|%s\n", ANSI_BLUE, ANSI_YELLOW, mShip->GetName().c_str(), ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s >------------------------------------------<%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  int i = 0;
  foreach(HSCargoBayInstance*, bay, bays) {
    if (i++ > 0) {
      sprintf(tbuf, " >-------------------------------^----------<\n");
      notification << tbuf;
    }
    sprintf(tbuf, "%s|%s %-42s %s|\n", ANSI_BLUE, ANSI_NORMAL, bay->GetName().c_str(), ANSI_BLUE);
    notification << tbuf;
    sprintf(tbuf, " >-------------------------------.----------<\n");
    notification << tbuf;
    if(bay->GetFreeSpace() < bay->GetSpace()) {
      foreach(HSCargoItem, cargo, bay->GetCargo()) {
        HSCommodity *commod = cargo.commod;
        if (!commod) {
          continue;
        }
        sprintf(tbuf, "|%s %-30s %s|%s %9.0f %s|\n", ANSI_NORMAL, commod->GetName().c_str(),
          ANSI_BLUE, ANSI_NORMAL, cargo.amount, ANSI_BLUE);
        notification << tbuf;
      }
    } else {
      sprintf(tbuf, "%s| %sNone                           %s|       %s--- %s|\n", ANSI_BLUE, ANSI_RED, ANSI_BLUE, ANSI_RED, ANSI_BLUE);
      notification << tbuf;
    }
    sprintf(tbuf, " >-------------------------------+----------<\n");
    notification << tbuf;
    sprintf(tbuf, "|%s                    Total Cargo %s|%s %9d %s|\n", 
      ANSI_NORMAL, ANSI_BLUE, ANSI_NORMAL, bay->GetTotalCargo(), ANSI_BLUE);
    notification << tbuf;
    sprintf(tbuf, "|%s                    Total Space %s|%s %9d %s|\n",
      ANSI_NORMAL, ANSI_BLUE, ANSI_NORMAL, bay->GetSpace(), ANSI_BLUE);
    notification << tbuf;
    sprintf(tbuf, "|%s                Available Space %s|%s %9d %s|\n",
      ANSI_NORMAL, ANSI_BLUE, ANSI_NORMAL, bay->GetFreeSpace(), ANSI_BLUE);
    notification << tbuf;
  }

  sprintf(tbuf, "%s'--------------------------------^-----------'%s", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;  
  aConsole->NotifyUser(notification.str());
}

void
HSComputerInstance::DisplaySystemStatusReport(HSConsole *aConsole, std::string aSystem)
{
  HSSystemInstance *system = mShip->FindSystemByName(aSystem);

  if (!system) {
    aConsole->NotifyUserSimple("System not found.");
    return;
  }
  stringstream notification;
  char tbuf[256];


  sprintf(tbuf, "%s.--------------------------------------------------------------------.\n",
    ANSI_BLUE);
  notification << tbuf;
  sprintf(tbuf, "|%s %-30s      %30s %s|\n",
    ANSI_CYAN, system->GetName().c_str(), 
    HSSystem::GetNameForType((HSSystemType)system->GetType()).c_str(),
    ANSI_BLUE);
  notification << tbuf;
  sprintf(tbuf, " >------------------------------------------------------------------<%s\n",
    ANSI_NORMAL);
  notification << tbuf;
  notification << system->GetSystemStatus();
  sprintf(tbuf, "%s'--------------------------------------------------------------------'%s",
    ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  aConsole->NotifyUser(notification.str());  
}

void
HSComputerInstance::DisplayScanReport(HSConsole *aConsole, std::string aContact)
{
  std::vector<HSSystemInstance*> sysSensors = mShip->FindSystemsByType(HSST_SENSOR);
  bool sensorStatus = 0;
  foreach(HSSystemInstance*, system, sysSensors){
    HSSensorInstance *sensor = static_cast<HSSensorInstance*>(system);
    if (sensor->GetCurrentPower() > 0) {
      sensorStatus = 1;
      break;
    }
  }
  if(!sensorStatus) {
    aConsole->NotifyUserSimple("Sensors are offline!");
    return;
  }
  if(mShip->GetLandedLoc() != -1) {
    aConsole->NotifyUserSimple("Sensors will not operate while landed.");
    return;
  }
  if(mShip->GetLandingTimer() != 0) {
    aConsole->NotifyUserSimple("Sensors will not operate while ship is landing/launching.");
    return;
  }
  HSSensorContact contact;
  if (!FindSensorContact(atoi(aContact.c_str()), &contact)) {
    aConsole->NotifyUserSimple("Contact not found.");
    return;
  }

  char tbuf[256];
  char tbuf2[128];
  char tbuf3[64];
  bool didCargo = 0;
  bool didDefense = 0;
  stringstream notification;

  HSObject *object = contact.Object;
  HSVector3D diffVector = object->GetLocation() - mShip->GetLocation();
  HSVector3D objectLoc = object->GetLocation();

  // Begin scan report
  sprintf(tbuf, "%s.-------------------------------------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;

  sprintf(tbuf, "%s| %sCONTACT%s: %s%-16d %s| %sX%s:%s%-24.1f %s\\%s\n", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_WHITE, 
    contact.ID, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_GREEN, object->GetLocation().mX, ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s|    %sTYPE%s: %s%-16s %s| %sY%s:%s%-24.1f  %s|%s\n", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_WHITE,
    HSObject::GetNameForType((HSObjectType)object->GetType()).c_str(), ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_GREEN, object->GetLocation().mY, ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s|  %sDETAIL%s: %s %s| %sZ%s:%s%-24.1f  %s|%s\n", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE,
    HSMeter(16, 100, contact.Detail).c_str(), ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_GREEN, object->GetLocation().mZ, ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, " %s>-------------------------------------------------------<%s\n", ANSI_BLUE,
    ANSI_NORMAL);
  notification << tbuf;
  // Check detail for name.
  if( contact.Detail >= 50) {
    sprintf(tbuf2, "%s", object->GetName().c_str());
  } else {
    sprintf(tbuf2, "Unknown");
  }
  if(object->GetType() == HSOT_SHIP) {
    HSShip *ship = static_cast<HSShip*>(contact.Object);
    HSHullClass *hullClass = NULL;
    hullClass = sHSDB.FindHullClass(ship->GetHullClass());
    sprintf(tbuf3, "%s", hullClass->GetName().substr(0,21).c_str());
  } else if(object->GetType() == HSOT_PLANET) {
    HSPlanet *planet = static_cast<HSPlanet*>(contact.Object);
    sprintf(tbuf3, "%s", planet->GetClass().substr(0,21).c_str());
  } else if(object->GetType() == HSOT_JUMPGATE) {
    sprintf(tbuf3, "Jump Gate");
  } else {
    sprintf(tbuf3, "N/A");
  }
  if(strlen(tbuf3) == 0) {
    sprintf(tbuf3, "N/A");
  }

  sprintf(tbuf, "%s|    %sNAME%s: %s%-17s %sCLASS%s: %s%-21s %s|%s\n", ANSI_BLUE, ANSI_YELLOW,
    ANSI_BLUE, ANSI_WHITE, tbuf2, ANSI_YELLOW, ANSI_BLUE, ANSI_WHITE, tbuf3,
    ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;


  // We always know bearing and range, no matter what the detail is.
  sprintf(tbuf, "%s| %sBEARING%s: %s%3d mark %-8d %sRANGE%s: %s%-21s %s|%s\n", ANSI_BLUE, ANSI_YELLOW,
    ANSI_BLUE, ANSI_GREEN, (int)diffVector.HeadingXY(), (int)diffVector.HeadingZ(), ANSI_YELLOW,
    ANSI_BLUE, ANSI_WHITE, HSObject::DistanceString(diffVector.length()).c_str(), ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  if (contact.Object->GetType() == HSOT_NEBULA) {
    HSNebula *nebula = static_cast<HSNebula*>(contact.Object);
    sprintf(tbuf, "%s|  %sRADIUS%s: %s%-46s %s|%s\n", ANSI_BLUE, ANSI_YELLOW,
      ANSI_BLUE, ANSI_NORMAL, HSDistanceString(nebula->GetRadius()).c_str(), ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
  }
  // Test to see if we are close enough for heading and speed if it is a ship.
  if(contact.Object->GetType() == HSOT_SHIP) {
    HSShip *ship = static_cast<HSShip*>(contact.Object);
    if(contact.Detail < 30) {
      // To far away for that much detail.
    } else {
      if(object->GetVelocity().length() > 0 || contact.Detail >=70) {
        sprintf(tbuf2, "%3d mark %-3d", (int)object->GetVelocity().HeadingXY(), (int)object->GetVelocity().HeadingZ());
      } else {
        sprintf(tbuf2, "Unknown");
      }
      sprintf(tbuf3, "%s", HSObject::DistanceString(object->GetVelocity().length()).append("/s").c_str());
      sprintf(tbuf, "%s| %sHEADING%s: %s%-17s %sSPEED%s: %s%-21s %s|%s\n", ANSI_BLUE, ANSI_YELLOW,
        ANSI_BLUE, ANSI_GREEN, tbuf2, ANSI_YELLOW, ANSI_BLUE, ANSI_WHITE, tbuf3, ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    }
    // Do we have enough detail to see shields and hull?
    if(contact.Detail >= 70) {
      didDefense = 1;
      sprintf(tbuf, "%s|    %sHULL%s: %s %sSHIELDS%s: ", ANSI_BLUE, ANSI_YELLOW,
        ANSI_BLUE, HSMeter(15, ship->GetHull()->GetHullPoints(), ship->GetCurrentHullPoints()).c_str(), ANSI_YELLOW, ANSI_BLUE);
      notification << tbuf;
      std::vector<HSSystemInstance*> sysShields = ship->FindSystemsByType(HSST_SHIELD);
      if(!sysShields.size()) {
        sprintf(tbuf, "%sFORE%s:%sNone    %sAFT%s:%sNone %s|\n", ANSI_YELLOW, ANSI_BLUE, ANSI_RED,
          ANSI_YELLOW, ANSI_BLUE, ANSI_RED, ANSI_BLUE);
        notification << tbuf;
        sprintf(tbuf, "%s \\-------------------------------.  %sPORT%s:%sNone   %sSTBD%s:%sNone %s|%s\n",
          ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_RED, ANSI_YELLOW, ANSI_BLUE, ANSI_RED, ANSI_BLUE, ANSI_NORMAL);
        notification << tbuf;
      } else {
        char fore[128];
        char aft[128];
        char port[128];
        char stbd[128];
        double foreMax = 0;
        double foreCurrent = 0;
        double aftMax = 0;
        double aftCurrent = 0;
        double portMax = 0;
        double portCurrent = 0;
        double starMax = 0;
        double starCurrent = 0;
        sprintf(fore, "%sNone", ANSI_RED);
        sprintf(aft, "%sNone", ANSI_RED);
        sprintf(port, "%sNone", ANSI_RED);
        sprintf(stbd, "%sNone", ANSI_RED);

        foreach(HSSystemInstance*, system, sysShields) {
          HSShieldInstance *shield = static_cast<HSShieldInstance*>(system);
          
          switch(shield->GetMountPoint()) {
            case 0:
              foreMax += shield->GetMaxAbsorption();
              foreCurrent += shield->GetCurrentAbsorption();
              break;
            case 1:
              aftMax += shield->GetMaxAbsorption();
              aftCurrent += shield->GetCurrentAbsorption();
              break;
            case 2:
              portMax += shield->GetMaxAbsorption();
              portCurrent += shield->GetCurrentAbsorption();
              break;
            case 3:
              starMax += shield->GetMaxAbsorption();
              starCurrent += shield->GetCurrentAbsorption();
              break;
           
          }
        }
        if(foreMax > 0) {
          sprintf(fore, "%s", HSNumMeter(foreMax, foreCurrent).c_str());
        }
        if(aftMax > 0) {
          sprintf(aft, "%s", HSNumMeter(aftMax, aftCurrent).c_str());
        }
        if(portMax > 0) {
          sprintf(port, "%s", HSNumMeter(portMax, portCurrent).c_str());
        }
        if(starMax > 0) {
          sprintf(stbd, "%s", HSNumMeter(starMax, starCurrent).c_str());
        }

        sprintf(tbuf, "%sFORE%s:%s    %sAFT%s:%s %s|%s\n", ANSI_YELLOW, ANSI_BLUE, fore,
          ANSI_YELLOW, ANSI_BLUE, aft, ANSI_BLUE, ANSI_NORMAL);
        notification << tbuf;
        sprintf(tbuf, "%s \\-------------------------------.  %sPORT%s:%s   %sSTBD%s:%s %s|%s\n",
          ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, port, ANSI_YELLOW, ANSI_BLUE, stbd, ANSI_BLUE, ANSI_NORMAL);
        notification << tbuf;
      }
    }
    // Lets see if we have enough detail to read the cargo holds.
    // Should also add in a method for hiding cargo here.
    if(contact.Detail < 85 && contact.Detail > 70) {
      sprintf(tbuf, "%s                                  \\-----------------------'%s\n", ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    } else if(contact.Detail >= 85) {                                                     
      std::vector<HSCargoBayInstance*> bays;
      foreach(HSSystemInstance*, system, ship->FindSystemsByType(HSST_CARGOBAY)) {
        bays.push_back(static_cast<HSCargoBayInstance*>(system));
      }
      if (!bays.size()) {
        // No bays so skip ahead.
    
      } else {
        didCargo = 1;
        sprintf(tbuf, "%s.------------------------------.  \\-----------------------'%s\n", ANSI_BLUE, ANSI_NORMAL);
        notification << tbuf;
        sprintf(tbuf, "%s|          %sCARGO SCAN           %s\\%s\n", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
        notification << tbuf;
        sprintf(tbuf, "%s|--------------------------------+-----------.%s\n", ANSI_BLUE, ANSI_NORMAL);
        notification << tbuf;
        int i = 0;
        foreach(HSCargoBayInstance*, bay, bays) {
          if (i++ > 0) {
            sprintf(tbuf, " >-------------------------------^----------<\n");
            notification << tbuf;
          }
          sprintf(tbuf, "%s|%s %-42s %s|\n", ANSI_BLUE, ANSI_NORMAL, bay->GetName().c_str(), ANSI_BLUE);
          notification << tbuf;
          sprintf(tbuf, " >-------------------------------.----------<\n");
          notification << tbuf;
          if(bay->GetFreeSpace() < bay->GetSpace()) {
            foreach(HSCargoItem, cargo, bay->GetCargo()) {
              HSCommodity *commod = cargo.commod;
              if (!commod) {
                continue;
              }
              sprintf(tbuf, "|%s %-30s %s|%s %9.0f %s|\n", ANSI_NORMAL, commod->GetName().c_str(),
                ANSI_BLUE, ANSI_NORMAL, cargo.amount, ANSI_BLUE);
              notification << tbuf;
            }
          } else {
            sprintf(tbuf, "%s| %sNone                           %s|       %s--- %s|\n", ANSI_BLUE, ANSI_RED, ANSI_BLUE, ANSI_RED, ANSI_BLUE);
            notification << tbuf;
          }
        }
        sprintf(tbuf, "%s'--------------------------------^-----------'%s\n", ANSI_BLUE, ANSI_NORMAL);
        notification << tbuf;
      }
    } 
  }
  if(object->GetType() == HSOT_CARGOPOD) {
    didCargo = 1;
    sprintf(tbuf, "%s'---------------------------------------------------------'%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s.------------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s|            %sCONTENTS           %s\\%s\n", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s|--------------------------------+-----------.%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    HSCargoPod *pod = static_cast<HSCargoPod*>(contact.Object);
    if(pod->GetCargo().size() > 0) {
      foreach(HSCargoItem, cargo, pod->GetCargo()) {
        HSCommodity *commod = cargo.commod;
        if(!commod) {
          continue;
        } else {
        sprintf(tbuf, "%s|%s %-30s %s|%s %9.0f %s|\n", ANSI_BLUE, ANSI_NORMAL, commod->GetName().c_str(),
          ANSI_BLUE, ANSI_NORMAL, cargo.amount, ANSI_BLUE);
        notification << tbuf;
        }
      }
    } else {
      sprintf(tbuf, "%s| %sNone                           %s|       %s--- %s|\n", ANSI_BLUE, ANSI_RED, ANSI_BLUE, ANSI_RED, ANSI_BLUE);
      notification << tbuf;
    }
    sprintf(tbuf, "%s'--------------------------------^-----------'%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
  } else if(object->GetType() == HSOT_RESOURCE) {
    didCargo = 1;
    sprintf(tbuf, "%s'---------------------------------------------------------'%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s.------------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s|            %sCONTENTS           %s\\%s\n", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s|--------------------------------+-----------.%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    HSResource *resource = static_cast<HSResource*>(contact.Object);
    if(resource->GetCargo().size() > 0) {
      foreach(HSCargoItem, cargo, resource->GetCargo()) {
        HSCommodity *commod = cargo.commod;
        if(!commod) {
          continue;
        } else {
        sprintf(tbuf, "%s|%s %-30s %s|%s %9.0f %s|\n", ANSI_BLUE, ANSI_NORMAL, commod->GetName().c_str(),
          ANSI_BLUE, ANSI_NORMAL, cargo.amount, ANSI_BLUE);
        notification << tbuf;
        }
      }
    } else {
      sprintf(tbuf, "%s| %sNone                           %s|       %s--- %s|\n", ANSI_BLUE, ANSI_RED, ANSI_BLUE, ANSI_RED, ANSI_BLUE);
      notification << tbuf;
    }
    sprintf(tbuf, "%s'--------------------------------^-----------'%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
  }  // Start location section, format based on previous sections.
  std::vector<HSLandingLocation*> landingLocs = object->GetLandingLocations();
  std::vector<HSDockingHatch*> dockingHatches = object->GetDockingHatches();
  if(!landingLocs.size() && !dockingHatches.size()) {
    // No landing locations do the bottom cap.
    if(didDefense && !didCargo) {
      sprintf(tbuf, "%s                                  \\-----------------------'%s\n", ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    } else if(!didDefense && !didCargo) {
      sprintf(tbuf, "%s'---------------------------------------------------------'%s\n", ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    }
  } else if (landingLocs.size() && dockingHatches.size()) {
    if(didDefense && !didCargo) {
      sprintf(tbuf, "%s.------------------------------.  \\-----------------------'%s\n", ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
      sprintf(tbuf, "%s|       %sLANDING LOCATIONS       %s\\%s\n", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    } else if(!didDefense && !didCargo) {
      sprintf(tbuf, "%s'---------------------------------------------------------'%s\n", ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
      sprintf(tbuf, "%s.--------------------------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
      sprintf(tbuf, "%s|             %sLANDING LOCATIONS              %s|%s\n", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW);
      notification << tbuf;
    }
    sprintf(tbuf, "%s|----.---------------------------.-----------|%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s| %sID %s|           %sNAME            %s|   %sSTATUS  %s|%s\n", ANSI_BLUE, ANSI_YELLOW,
      ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s|----+---------------------------+-----------|%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    for(unsigned int i = 0; i < landingLocs.size(); i++) {
      // Get the status of the landing loc.
      if (!landingLocs[i]->GetOpen()) {
        sprintf(tbuf2, "  %s%sCLOSED%s ", ANSI_HILITE, ANSI_RED, ANSI_NORMAL);
      } else if (landingLocs[i]->GetSpaceTaken() + mShip->GetSize() >
        landingLocs[i]->GetSize()) {
        sprintf(tbuf2, "  %s%sFULL%s   ", ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
      } else {
        sprintf(tbuf2, "  %s%sOPEN%s   ", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
      }
      sprintf(tbuf, "%s| %s%2d %s| %s%-25s %s| %s %s|%s\n", ANSI_BLUE, ANSI_GREEN, i, ANSI_BLUE, ANSI_GREEN, HSIFGetName(landingLocs[i]->GetID()).c_str(), 
        ANSI_BLUE, tbuf2, ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    }
    sprintf(tbuf, "%s'----^---------------------------^-----------'%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s.--------------------------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s|               %sDOCKING HATCHES                %s|%s\n", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW);
    notification << tbuf;
    sprintf(tbuf, "%s|----.---------------------------.-----------|%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s| %sID %s|           %sNAME            %s|   %sSTATUS  %s|%s\n", ANSI_BLUE, ANSI_YELLOW,
      ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s|----+---------------------------+-----------|%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    for(unsigned int i = 0; i < dockingHatches.size(); i++) {
      // Get the status of the landing loc.
      if (dockingHatches[i]->GetLinkedTo() != DBRefNothing) {
        sprintf(tbuf2, "  %s%sTAKEN%s  ", ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
      } else if (dockingHatches[i]->GetClamped()) {
        sprintf(tbuf2, " %s%sCLAMPED%s ", ANSI_HILITE, ANSI_RED, ANSI_NORMAL);
      } else {
        sprintf(tbuf2, "  %s%sOPEN%s   ", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
      }
      sprintf(tbuf, "%s| %s%2d %s| %s%-25s %s| %s %s|%s\n", ANSI_BLUE, ANSI_GREEN, i, ANSI_BLUE, ANSI_GREEN, HSIFGetName(dockingHatches[i]->GetID()).c_str(), 
        ANSI_BLUE, tbuf2, ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    }
    sprintf(tbuf, "%s'----^---------------------------^-----------'%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
  } else if (dockingHatches.size()) {
    if(didDefense && !didCargo) {
      sprintf(tbuf, "%s.------------------------------.  \\-----------------------'%s\n", ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
      sprintf(tbuf, "%s|        %sDOCKING HATCHES        %s\\%s\n", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    } else if(!didDefense && !didCargo) {
      sprintf(tbuf, "%s'---------------------------------------------------------'%s\n", ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
      sprintf(tbuf, "%s.--------------------------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
      sprintf(tbuf, "%s|              %sDOCKING HATCHES              %s|%s\n", ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW);
      notification << tbuf;
    }
    sprintf(tbuf, "%s|----.---------------------------.-----------|%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s| %sID %s|           %sNAME            %s|   %sSTATUS  %s|%s\n", ANSI_BLUE, ANSI_YELLOW,
      ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    sprintf(tbuf, "%s|----+---------------------------+-----------|%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
    for(unsigned int i = 0; i < dockingHatches.size(); i++) {
      // Get the status of the landing loc.
      if (dockingHatches[i]->GetLinkedTo() != DBRefNothing) {
        sprintf(tbuf2, "  %s%sTAKEN%s  ", ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
      } else if (dockingHatches[i]->GetClamped()) {
        sprintf(tbuf2, " %s%sCLAMPED%s ", ANSI_HILITE, ANSI_RED, ANSI_NORMAL);
      } else {
        sprintf(tbuf2, "  %s%sOPEN%s   ", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
      }
      sprintf(tbuf, "%s| %s%2d %s| %s%-25s %s| %s %s|%s\n", ANSI_BLUE, ANSI_GREEN, i, ANSI_BLUE, ANSI_GREEN, HSIFGetName(dockingHatches[i]->GetID()).c_str(), 
        ANSI_BLUE, tbuf2, ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    }
    sprintf(tbuf, "%s'----^---------------------------^-----------'%s\n", ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
  }

  aConsole->NotifyUser(notification.str());
}

void
HSComputerInstance::DisplayHatchReport(HSConsole *aConsole)
{
  std::stringstream notification;

  std::vector<HSDockingHatch*> hatches = mShip->GetDockingHatches();

  if (hatches.empty()) {
    aConsole->NotifyUserSimple("No docking hatches present.");
  }
  char tbuf[256];
  sprintf(tbuf, "%s.----------------------------------------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s|%s Hatch Status %43s %s|%s\n", ANSI_BLUE, ANSI_YELLOW, mShip->GetName().c_str(), ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s >------------------------------+-------------------------<%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s| %sDocking Hatch                 %s| %sConnection               %s|%s\n", 
    ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s >------------------------------+-------------------------<%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  int i = 0;
  foreach(HSDockingHatch*, hatch, hatches) {
    char connStr[128];
    char idStr[64];
    if (hatch->GetLinkedTo() == DBRefNothing) {
      sprintf(connStr, "Unconnected");
    } else {
      HSDockingHatch *connHatch = sHSDB.FindDockingHatch(hatch->GetLinkedTo());
      if (connHatch) {
        HSObject *obj = sHSDB.FindObject(connHatch->GetObjectID());
        if (obj) {
          sprintf(connStr, "%s", obj->GetName().c_str());
        }
      }
    }
    sprintf(idStr, "%d", i++);
    sprintf(tbuf, "%s| %s%2s %s%s%s%-25s %s%s | %s%-24s %s|%s\n", 
      ANSI_BLUE, ANSI_YELLOW, idStr, ANSI_NORMAL, ANSI_WHITE, ANSI_HILITE, HSIFGetName(hatch->GetID()).c_str(), ANSI_NORMAL,
      ANSI_BLUE, ANSI_NORMAL, connStr, ANSI_BLUE, ANSI_NORMAL);
    notification << tbuf;
  }
  sprintf(tbuf, "%s'-------------------------------^--------------------------'%s", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;

  aConsole->NotifyUser(notification.str());
}

void
HSComputerInstance::ConnectHatch(HSConsole *aConsole, std::string aHatch, std::string aTarget, std::string aTargetHatch)
{
  std::vector<HSDockingHatch*> hatches = mShip->GetDockingHatches();

  int hatchId = atoi(aHatch.c_str());
  if (hatches.size() <= (size_t)hatchId) {
    aConsole->NotifyUserSimple("No such hatch present.");
    return;
  }

  HSDockingHatch *hatch = hatches[hatchId];

  if (hatch->GetLinkedTo() != DBRefNothing) {
    aConsole->NotifyUserSimple("That hatch is already connected.");
    return;
  }

  if (hatch->GetClamped()) {
    aConsole->NotifyUserSimple("That hatch is clamped.");
    return;
  }

  HSSensorContact contact;
  if (!FindSensorContact(atoi(aTarget.c_str()), &contact)) {
    aConsole->NotifyUserSimple("No such sensor contact.");
    return;
  }

  HSVector3D dist = mShip->GetLocation() - contact.Object->GetLocation();
  if (dist.length() > sHSConf.DockingDistance) {
    aConsole->NotifyUserSimple("That target is too far away to connect a docking hatch.");
    return;
  }

  int targetHatchId = atoi(aTargetHatch.c_str());

  std::vector<HSDockingHatch*> targetHatches = contact.Object->GetDockingHatches();

  if (targetHatches.size() <= (size_t)targetHatchId) {
    aConsole->NotifyUserSimple("No such hatch present on target.");
    return;
  }

  HSDockingHatch *targetHatch = targetHatches[targetHatchId];

  if (targetHatch->GetLinkedTo() != DBRefNothing) {
    aConsole->NotifyUserSimple("Target hatch is already taken.");
    return;
  }

  if (targetHatch->GetClamped()) {
    aConsole->NotifyUserSimple("Target hatch is clamped.");
    return;
  }

  targetHatch->SetLinkedTo(hatch->GetID());
  hatch->SetLinkedTo(targetHatch->GetID());
  HSIFSetExitDestination(targetHatch->GetID(), HSIFGetObjectHome(hatch->GetID()));
  HSIFSetExitDestination(hatch->GetID(), HSIFGetObjectHome(targetHatch->GetID()));
  char tbuf[256];
  sprintf(tbuf, "%s has been connected to the %s.", HSIFGetName(hatch->GetID()).c_str(), contact.Object->GetName().c_str());
  mShip->NotifyConsolesFormatted("Navigation", tbuf);
  sprintf(tbuf, "The %s has connected to our %s.", mShip->GetName().c_str(), HSIFGetName(targetHatch->GetID()).c_str());
  contact.Object->NotifyConsolesFormatted("Navigation", tbuf);
}

void
HSComputerInstance::DisconnectHatch(HSConsole *aConsole, std::string aHatch)
{
  std::vector<HSDockingHatch*> hatches = mShip->GetDockingHatches();

  int hatchId = atoi(aHatch.c_str());
  if (hatches.size() <= (size_t)hatchId) {
    aConsole->NotifyUserSimple("No such hatch present.");
    return;
  }

  HSDockingHatch *hatch = hatches[hatchId];

  if (hatch->GetLinkedTo() == DBRefNothing) {
    aConsole->NotifyUserSimple("That hatch is not connected.");
    return;
  }

  if (hatch->GetClamped()) {
    aConsole->NotifyUserSimple("That hatch is clamped.");
    return;
  }
  hatch->Disconnect();
}

void
HSComputerInstance::LockWeapons(HSConsole *aConsole, std::string aWeapon, std::string aContact)
{
  std::vector<HSWeaponInstance*> weapons;

  if (!strncasecmp(aWeapon.c_str(), "ALL")) {
    foreach(HSSystemInstance*, system, mShip->GetSystems()) {
      if (system->GetType() == HSST_BEAMWEAPON) {
        weapons.push_back(static_cast<HSWeaponInstance*>(system));
      }
    }
  } else {
    if (aWeapon.find("/") != (size_t)-1) {
      HSSystemInstance *system = mShip->FindSystemByName(aWeapon);
      if (system) {
        weapons.push_back(static_cast<HSWeaponInstance*>(system));
      }
    } else {
      std::vector<HSSystemInstance*> systems = mShip->FindSystemsByName(aWeapon);
      foreach(HSSystemInstance*, system, systems) {
        if (system->GetType() == HSST_BEAMWEAPON) {
          weapons.push_back(static_cast<HSWeaponInstance*>(system));
        }
      }
    }
  }
  if (weapons.empty()) {
    aConsole->NotifyUserSimple("Weapon(s) not found.");
    return;
  }

  HSSensorContact contact;
  if (!FindSensorContact(atoi(aContact.c_str()), &contact)) {
    aConsole->NotifyUserSimple("Sensor contact not found.");
    return;
  }

  std::stringstream notification;
  int i = 0;
  foreach(HSWeaponInstance*, weapon, weapons)  {
    weapon->SetTarget(contact.Object);
    if (!i++) {
      notification << weapon->GetName();
    } else {
      notification << ", " << weapon->GetName();
    }
  }
  notification << " locked onto ";

  if (contact.Detail > 50) {
    notification << contact.ID << " - " << contact.Object->GetName();
  } else {
    notification << "contact " << contact.ID;
  }

  mShip->NotifyConsolesFormatted("Weapons", notification.str());
}

void
HSComputerInstance::FireWeapons(HSConsole *aConsole, std::string aWeapon)
{
  std::vector<HSWeaponInstance*> weapons;

  if (!strncasecmp(aWeapon.c_str(), "ALL")) {
    foreach(HSSystemInstance*, system, mShip->GetSystems()) {
      if (system->GetType() == HSST_BEAMWEAPON) {
        weapons.push_back(static_cast<HSWeaponInstance*>(system));
      }
    }
  } else {
    if (aWeapon.find("/") != (size_t)-1) {
      HSSystemInstance *system = mShip->FindSystemByName(aWeapon);
      if (system) {
        weapons.push_back(static_cast<HSWeaponInstance*>(system));
      }
    } else {
      std::vector<HSSystemInstance*> systems = mShip->FindSystemsByName(aWeapon);
      foreach(HSSystemInstance*, system, systems) {
        if (system->GetType() == HSST_BEAMWEAPON) {
          weapons.push_back(static_cast<HSWeaponInstance*>(system));
        }
      }
    }
  }
  if (weapons.empty()) {
    aConsole->NotifyUserSimple("Weapon(s) not found.");
    return;
  }

  HSUniverse *myUniv = sHSDB.FindUniverse(mShip->GetUniverse());
  ASSERT(myUniv);
  std::vector<HSObject*> hitNotifies;
  std::vector<HSObject*> missNotifies;
  char tbuf[256];
  foreach(HSWeaponInstance*, weapon, weapons)  {
    HSFireResult result = weapon->Fire();
    bool contactFound = false;
    HSSensorContact target;
    foreach(HSSensorContact, contact, mSensorContacts) {
      if (contact.Object == weapon->GetTarget()) {
        target = contact;
        contactFound = true;
        break;
      }
    }
    char targetStr[128];
    if (!contactFound) {
      sprintf(targetStr, "unknown target");
    } else {
      if (target.Detail > 50) {
        sprintf(targetStr, "%s (%d)", target.Object->GetName().c_str(), target.ID);
      } else {
        sprintf(targetStr, "contact %d", target.ID);
      }
    }
    if (result == HSFR_HIT) {
      sprintf(tbuf, "%s fires and hits %s", weapon->GetName().c_str(), targetStr);
      mShip->NotifyConsolesFormatted("Weapons", tbuf);
      bool found = false;
      foreach(HSObject*, obj, hitNotifies) {
        if (obj == weapon->GetTarget()) {
          found = true;
        }
      }
      if (!found) {
        hitNotifies.push_back(weapon->GetTarget());
      }
    } else if (result == HSFR_MISS) {
      sprintf(tbuf, "%s fires and misses %s", weapon->GetName().c_str(), targetStr);
      bool found = false;
      foreach(HSObject*, obj, missNotifies) {
        if (obj == weapon->GetTarget()) {
          found = true;
        }
      }
      if (!found) {
        missNotifies.push_back(weapon->GetTarget());
      }
      mShip->NotifyConsolesFormatted("Weapons", tbuf);
    } else if (result == HSFR_NOTLOCKED) {
      sprintf(tbuf, "%s is not locked on a target.", weapon->GetName().c_str());
      aConsole->NotifyUserSimple(tbuf);
    } else if (result == HSFR_RANGE) {
      sprintf(tbuf, "%s does not have %s in range.", weapon->GetName().c_str(), targetStr);
      aConsole->NotifyUserSimple(tbuf);
    } else if (result == HSFR_CHARGE) {
      sprintf(tbuf, "%s is not sufficiently charged to fire.", weapon->GetName().c_str());
      aConsole->NotifyUserSimple(tbuf);
    } else if (result == HSFR_ARC) {
      sprintf(tbuf, "%s does not have %s in its firing arc.", weapon->GetName().c_str(), targetStr);
      aConsole->NotifyUserSimple(tbuf);    
    }    
  }

  foreach(HSObject*, obj, missNotifies) {
    myUniv->NotifyWeaponsFire(mShip, obj, HSDR_NONE);
  }
}

void
HSComputerInstance::Intercept(HSConsole *aConsole, std::string aContact)
{
  HSSensorContact contact;
  if (!FindSensorContact(atoi(aContact.c_str()), &contact)) {
    aConsole->NotifyUserSimple("No such contact found");
    return;
  }
  char tbuf[128];
  if (contact.Detail > 50) {
    sprintf(tbuf, "Intercept course set for %s (%d)", contact.Object->GetName().c_str(), contact.ID);
  } else {
    sprintf(tbuf, "Intercept course set for contact %d", contact.ID);
  }
  HSVector3D targetVector = contact.Object->GetLocation() - mShip->GetLocation();
  mShip->SetDesiredHeading(targetVector.normalized());
  mShip->SetAccelerating(true);
  std::vector<HSSystemInstance*> autopilots = mShip->FindSystemsByType(HSST_AUTOPILOT);
  foreach(HSSystemInstance*, system, autopilots) {
    HSAutopilotInstance *autopilot = static_cast<HSAutopilotInstance*>(system);
    autopilot->SetDestination(contact.Object->GetLocation());
  }
  mShip->NotifyConsolesFormatted("Navigation", tbuf);
}

void
HSComputerInstance::Gate(HSConsole *aConsole, std::string aContact)
{
  HSSensorContact contact;

  if (!FindSensorContact(atoi(aContact.c_str()), &contact)) {
    aConsole->NotifyUserSimple("No such contact found.");
    return;
  }

  if (contact.Object->GetType() != HSOT_JUMPGATE) {
    aConsole->NotifyUserSimple("You cannot gate that contact.");
    return;
  }

  HSVector3D diffVector = contact.Object->GetLocation() - mShip->GetLocation();
  double distance = diffVector.length();

  char tbuf[128];
  if (distance > sHSConf.GateDistance) {
    sprintf(tbuf, "Contact must be within %d km to gate it.", sHSConf.GateDistance);
    aConsole->NotifyUserSimple(tbuf);
    return;
  }

  if (mShip->GetVelocity().length() > 0) {
    aConsole->NotifyUserSimple("You cannot gate an object while moving.");
    return;
  }

  if (contact.Detail > 50) {
    sprintf(tbuf, "Now gating %s (%d)", contact.Object->GetName().c_str(), contact.ID);
    mShip->NotifyConsolesFormatted("Navigation", tbuf);
  } else {
    sprintf(tbuf, "Now gating contact %d", contact.ID);
    mShip->NotifyConsolesFormatted("Navigation", tbuf);
  }

  HSJumpGate *jumpGate = static_cast<HSJumpGate*>(contact.Object);
  mShip->SetLocation(jumpGate->GetDestination());
  HSUniverse *oldUniv = sHSDB.FindUniverse(mShip->GetUniverse());
  ASSERT(oldUniv);
  HSUniverse *univ = sHSDB.FindUniverse(jumpGate->GetDestUniverse());

  oldUniv->NotifyObjectAffectPose(mShip, "gates with", jumpGate, "Sensors");
  HSStoredPose pose;
  pose.source = mShip;
  pose.type = "Sensors";
  pose.pose = sHSConf.GatesIn;
  if (univ) {
    if (univ->GetID() != mShip->GetUniverse()) {
      oldUniv->RemoveObject(mShip);
      univ->AddObject(mShip);
      mShip->SetUniverse(univ->GetID());
    }
    univ->PushPose(pose);
  } else {
    oldUniv->PushPose(pose);
  }
}

void
HSComputerInstance::Harvest(HSConsole *aConsole, std::string aHarvester, std::string aTarget, std::string aCommod)
{
  int com;
  if (!from_string<int>(com, aCommod)) {
    aConsole->NotifyUserSimple("You need to specify a valid material.");
    return;
  }
  HSCommodity *commod = sHSDB.FindCommodity(com);
  if (!commod) {
    aConsole->NotifyUserSimple("You need to specify a valid material.");
    return;
  }

  HSHarvesterInstance *harvester = NULL;
  std::vector<HSSystemInstance*> systems = mShip->FindSystemsByName(aHarvester);

  if (!systems.size()) {
    aConsole->NotifyUserSimple("No harvesters found by that name.");
    return;
  }
  std::vector<HSHarvesterInstance*> harvesters;
  foreach(HSSystemInstance*, sys, systems) {
    if (sys->GetType() ==  HSST_HARVESTER &&
      (sys->GetCurrentPower() > 0 || !sys->GetOptimalPower())) {
      HSHarvesterInstance *harvest = static_cast<HSHarvesterInstance*>(sys);
      foreach(int, commodity, harvest->GetHarvestCommods()) {
        if (commodity == commod->GetID()) {
          harvesters.push_back(harvest);
          break;
        }
      }
    }
  }
  if (!harvesters.size()) {
    aConsole->NotifyUserSimple("No harvesters powered by that name that can mine that commodity.");
    return;
  }

  foreach(HSHarvesterInstance*, harvest, harvesters) {
    if (!harvest->GetTargetObj()) {
      harvester = harvest;
      break;
    }
  }
  if (!harvester) {
    aConsole->NotifyUserSimple("All specified harvesters are already working.");
    return;
  }

  HSSensorContact target;
  if (!FindSensorContact(atoi(aTarget.c_str()), &target)) {
    aConsole->NotifyUserSimple("No such sensor contact found.");
    return;
  }

  if (target.Object->GetType() != HSOT_RESOURCE) {
    aConsole->NotifyUserSimple("You cannot harvest that contact.");
    return;
  }

  HSVector3D diffVector = target.Object->GetLocation() - mShip->GetLocation();
  if (diffVector.length() > sHSConf.HarvestDistance) {
    char tbuf[256];
    sprintf(tbuf, "A resource has to be within %s to harvest from it.",
      HSObject::DistanceString(sHSConf.HarvestDistance).c_str());
    aConsole->NotifyUserSimple(tbuf);
    return;
  }

  HSResource *resource = static_cast<HSResource*>(target.Object);

  if (resource->GetCargoForCommodity(commod->GetID()) == 0.0) {
    aConsole->NotifyUserSimple("You cannot harvest that material from that resource.");
    return;
  }

  harvester->SetCommod(commod->GetID());
  harvester->SetTarget(resource->GetID());
  harvester->SetCommodObj(commod);
  harvester->SetTargetObj(resource);

  char tbuf[256];
  if (target.Detail > 50) {
    sprintf(tbuf, "Initiating harvesting %s from %s (%d)",
      commod->GetName().c_str(), target.Object->GetName().c_str(), target.ID);
  } else {
    sprintf(tbuf, "Initiating harvesting %s from contact %d",
      commod->GetName().c_str(), target.ID);
  }
  mShip->NotifyConsolesFormatted("Harvesting", tbuf);
}

void
HSComputerInstance::AbortHarvest(HSConsole *aConsole, std::string aHarvester)
{
  std::vector<HSSystemInstance*> systems = mShip->FindSystemsByName(aHarvester);
  std::vector<HSHarvesterInstance*> harvesters;
  foreach(HSSystemInstance*, sys, systems) {
    if (sys->GetType() ==  HSST_HARVESTER) {
      harvesters.push_back(static_cast<HSHarvesterInstance*>(sys));
    }
  }
  if (!harvesters.size()) {
    aConsole->NotifyUserSimple("No harvesters available by that name.");
    return;
  }
  char tbuf[256];
  foreach(HSHarvesterInstance*, harvester, harvesters) {
    if (harvester->GetTargetObj()) {
      sprintf(tbuf, "Harvesting by %s aborted", harvester->GetName().c_str());
      mShip->NotifyConsolesFormatted("Harvesting", tbuf);
      harvester->CancelHarvesting();
    } else {
      sprintf(tbuf, "%s not harvesting.", harvester->GetName().c_str());
      aConsole->NotifyUserSimple(tbuf);
    }
  }
}

void
HSComputerInstance::Collect(HSConsole *aConsole, std::string aPod)
{

  HSSensorContact target;
  if (!FindSensorContact(atoi(aPod.c_str()), &target)) {
    aConsole->NotifyUserSimple("No such sensor contact found.");
    return;
  }

  if (target.Object->GetType() != HSOT_CARGOPOD) {
    aConsole->NotifyUserSimple("You cannot collect that contact.");
    return;
  }

  HSVector3D diffVector = target.Object->GetLocation() - mShip->GetLocation();
  if (diffVector.length() > sHSConf.CollectDistance) {
    char tbuf[256];
    sprintf(tbuf, "A pod has to be within %s to collect it.",
      HSObject::DistanceString(sHSConf.CollectDistance).c_str());
    aConsole->NotifyUserSimple(tbuf);
    return;
  }
  HSCargoPod *pod = static_cast<HSCargoPod*>(target.Object);

  double totalCargoSpace = 0.00;

  std::vector<HSCargoBayInstance*> bays;
  foreach(HSSystemInstance*, sys, mShip->FindSystemsByType(HSST_CARGOBAY)) {
    ASSERT(sys->GetType() == HSST_CARGOBAY);
    bays.push_back(static_cast<HSCargoBayInstance*>(sys));
  }

  foreach(HSCargoBayInstance*, bay, bays) {
    totalCargoSpace += bay->GetFreeSpace();
  }

  if (totalCargoSpace < 1.00) {
    aConsole->NotifyUserSimple("No space in any cargo bay to collect that pod.");
    return;
  }

  foreach(HSCargoItem, item, pod->GetCargo()) {
    double toStore = pod->GetCargoForCommodity(item.commod->GetID());
    foreach(HSCargoBayInstance*, bay, bays) {
      if (bay->GetFreeSpace() >= toStore) {
        bay->SetCargoForCommodity(item.commod->GetID(),
          bay->GetCargoForCommodity(item.commod->GetID()) + toStore);
        pod->SetCargoForCommodity(item.commod->GetID(), 0.00);
        toStore = 0;
        break;
      } else {
        toStore -= bay->GetFreeSpace();
        bay->SetCargoForCommodity(item.commod->GetID(),
          bay->GetCargoForCommodity(item.commod->GetID()) + bay->GetFreeSpace());
      }
    }
    if (toStore > 0) {
      pod->SetCargoForCommodity(item.commod->GetID(), toStore);
      break;
    }
  }
  char tbuf[256];
  if (target.Detail > 50) {
    sprintf(tbuf, "Cargo collected from %s (%d)", 
      target.Object->GetName().c_str(), target.ID);
  } else {
    sprintf(tbuf, "Cargo collected from unknown contact %d",
      target.ID);
  }
  mShip->NotifyConsolesFormatted("Cargo", tbuf);
  HSUniverse *univ = sHSDB.FindUniverse(mShip->GetUniverse());
  if (univ) {
    univ->NotifyObjectAffectPose(mShip, "collects cargo from", pod, "Cargo");
  }
  if (!pod->GetTotalCargo()) {
    pod->Destroy();
  }
}
