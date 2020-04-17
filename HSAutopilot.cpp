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
#include "HSAutopilot.h"
#include "HSEngine.h"
#include "HSShip.h"
#include "HSHullClass.h"
#include "HSTools.h"
#include "HSConf.h"
#include "HSCargoBay.h"
#include "HSDimensionalDrive.h"

HSAutopilot::HSAutopilot(void)
  : mMaxSize(0)
{
  mType = HSST_AUTOPILOT;
  ADD_ATTRIBUTE("MAXSIZE", AT_INTEGER, mMaxSize)
}

HSAutopilot::~HSAutopilot(void)
{
}

HSAutopilotInstance::HSAutopilotInstance(void)
  : mEngaged(false)
  , mChangeHeadingLastCycle(false)
  , mChangeSpeedLastCycle(false)
{
  mType = HSST_AUTOPILOT;
  ADD_ATTRIBUTE("DESTINATION", AT_VECTOR, mDestination)
  ADD_ATTRIBUTE("ENGAGED", AT_BOOLEAN, mEngaged)
}

HSAutopilotInstance::~HSAutopilotInstance(void)
{
}

void
HSAutopilotInstance::Cycle()
{
  if (!mEngaged) {
    return;
  }
  if (GetCurrentPower() < GetOptimalPower()) {
    mShip->NotifyConsolesFormatted("Autopilot", "Insufficient power - disengaging");
    mEngaged = false;
    return;
  }

  HSVector3D travellingVector = GetDestination() - mShip->GetLocation();

  double travellingDistance = travellingVector.length();
  if (travellingDistance < 1.00) {
    mShip->SetDesiredVelocity(0);
    mShip->NotifyConsolesFormatted("Autopilot", "Destination reached - disengaging");
    mEngaged = false;
    return;
  }

  double totalThrust = 0;
  double totalMass = mShip->GetHull()->GetMass();
  double totalMaxSpeed = 0;
  foreach(HSSystemInstance*, system, mShip->GetSystems()) {
    totalMass += system->GetMass();
    if (system->GetType() == HSST_ENGINE) {
      HSEngineInstance *engine = static_cast<HSEngineInstance*>(system);
      totalThrust += engine->GetThrust(true);
      totalMaxSpeed += engine->GetMaxSpeed(true);
    }
    if (system->GetType() == HSST_CARGOBAY) {
      HSCargoBayInstance *bay = static_cast<HSCargoBayInstance*>(system);
      totalMass += bay->GetCargoMass();
    }
  }

  double acceleration = totalThrust / totalMass;
  
  double currentSpeed = mShip->GetVelocity().length();

  if (!(mShip->GetDesiredHeading() == travellingVector.normalized())) {
    double headingXY = travellingVector.HeadingXY();
    double headingZ = travellingVector.HeadingZ();
    mShip->SetDesiredHeading(HSVector3D(headingXY, headingZ));
    if (!mChangeHeadingLastCycle) {
      char tbuf[128];
      sprintf(tbuf, "Changing heading to %.2f mark %.2f", headingXY, headingZ);
      mChangeHeadingLastCycle = true;
      mShip->NotifyConsolesFormatted("Autopilot", tbuf);
    }
  } else {
    mChangeHeadingLastCycle = false;
  }
  double timeToStop = currentSpeed / acceleration + 1.00;

  double desiredVelocity = mShip->GetDesiredVelocity();
  if (mShip->HasDDEngaged()) {
    if (travellingDistance < (mShip->GetVelocity().length() * (double)sHSConf.DDMultiplier +
      timeToStop * desiredVelocity)) {
      std::vector<HSSystemInstance*> dimensionalDrives =
        mShip->FindSystemsByType(HSST_DIMENSIONALDRIVE);
      ASSERT(!dimensionalDrives.empty()); // This would indicate inconsistent state. HasDDEngaged() cannot be true in this case.
      HSDimensionalDriveInstance *ddrive =
        static_cast<HSDimensionalDriveInstance*>(*dimensionalDrives.begin());
      ddrive->Disengage();
      char tbuf[256];
      sprintf(tbuf, "Approaching target - disengaging %s", ddrive->GetName().c_str());
      mShip->NotifyConsolesFormatted("Autopilot", tbuf);
    }
  } 
  if (travellingDistance < (timeToStop * desiredVelocity)) {
    double newSpeed = travellingDistance / timeToStop;
    if (newSpeed > totalMaxSpeed) {
      newSpeed = totalMaxSpeed;
    }
    mShip->SetDesiredVelocity(newSpeed);
    mShip->SetAccelerating(true);
    if (!mChangeSpeedLastCycle && currentSpeed > newSpeed) {
      mChangeSpeedLastCycle = true;
      mShip->NotifyConsolesFormatted("Autopilot", "Approaching target - decelerating");
    }
  } else {
    mChangeSpeedLastCycle = false;
  }
}
