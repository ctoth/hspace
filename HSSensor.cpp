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
#include "HSSensor.h"
#include "HSObject.h"
#include "HSUniverse.h"
#include "HSDB.h"
#include "HSShip.h"
#include "HSTools.h"
#include "HSVector3D.h"
#include "HSHullClass.h"
#include "HSShip.h"
#include "HSComputer.h"
#include "HSNebula.h"

HSSensor::HSSensor(void)
  : mStrength(0)
{
  ADD_ATTRIBUTE("STRENGTH", AT_INTEGER, mStrength)
  mType = HSST_SENSOR;
}

HSSensor::~HSSensor(void)
{
}

HSSensorInstance::HSSensorInstance(void)
{
  ADD_ATTRIBUTE_INHERIT("STRENGTH", AT_INTEGER, mStrength)
  mType = HSST_SENSOR;
}

HSSensorInstance::~HSSensorInstance(void)
{
}

void
HSSensorInstance::Cycle()
{
  if (GetCurrentPower() == 0) {
    return;
  }
  HSSystemInstance::Cycle();

  std::vector<HSSystemInstance*> computers = mShip->FindSystemsByType(HSST_COMPUTER);

  if (computers.empty()) {
    // This ship has no computer.
    return;
  }
  HSComputerInstance *computer = 
    static_cast<HSComputerInstance*>(*computers.begin());
  
  if (!computer || !computer->IsOnline()) {
    // No computer on this ship. Sensors can't store data.
    return;
  }

  HSUniverse *myUniv = sHSDB.FindUniverse(mShip->GetUniverse());

  if (!myUniv) {
    return;
  }

  if (mShip->GetLandedLoc() != -1) {
    return;
  }

  double distance;
  double detail;

  // Detect if we have our dimensional drive engaged.
  bool dimDriveEngaged = mShip->HasDDEngaged();

  std::vector<HSNebula*> nebulae;
  // Grab a list of Nebulae possibly affecting sensors in this universe.
  foreach(HSObject*, object, myUniv->GetObjects()) {
    if (object->GetType() == HSOT_NEBULA) {
      nebulae.push_back(static_cast<HSNebula*>(object));
    }
  }

  // Effect nebulae we're in affect our ability to 'see'.
  double nebulaLocalEffect = 1.00;
  foreach(HSNebula*, nebula, nebulae) {
    HSVector3D diffVector = nebula->GetLocation() - mShip->GetLocation();
    if (diffVector.length() < nebula->GetRadius()) {
      double falloff = 1.0 - nebula->GetFalloff() * 
        (diffVector.length() / nebula->GetRadius());
      nebulaLocalEffect *= nebula->GetSensorEffect() * falloff;
    }
  }
  
  // Iterate through objects in the same universe.
  foreach(HSObject*, object, myUniv->GetObjects()) {

    bool targetDimDriveEngaged = false;

    // Only detect objects in space, and don't detect ourselves.
    if (!object->IsInSpace() || object->GetID() == mShip->GetID()) {
      continue;
    }

    // Detect if the object had its dimensional drive engaged.
    if (object->GetType() == HSOT_SHIP) {
      targetDimDriveEngaged = static_cast<HSShip*>(object)->HasDDEngaged();
    }

    // Only see if we're in the same dimension.
    if (targetDimDriveEngaged != dimDriveEngaged) {
      continue;
    }

    HSVector3D diffVector;
    diffVector = object->GetLocation() - mShip->GetLocation();
    distance = diffVector.length();
    if (distance == 0) {
      detail = 100.00;
    } else {
      distance /= 100000.00;
      detail = pow((double)object->GetSize(), 2) * (double)GetStrength(true) * 
        ((double)GetCurrentPower() / (double)GetOptimalPower()) * nebulaLocalEffect;

      // Power output raises a ship's detectibility.
      if (object->GetType() == HSOT_SHIP) {
        double output = static_cast<HSShip*>(object)->GetPowerOutput();
        if (output > 1.0) {
          detail *= output / 1000;
        }
      }
      /// \todo Sensor signal goes down by third order, this is
      /// realistic but might not be good for the gameplay experience
      detail /= distance * distance * distance;

      detail = sqrt(sqrt(detail));

      distance *= 1000000.00;
      if (detail < 1.00) {
        continue;
      }
      // We'd normally see this object, now adjust detail for any nebulae it's in.
      foreach(HSNebula*, nebula, nebulae) {
        HSVector3D diffVector = nebula->GetLocation() - object->GetLocation();
        if (diffVector.length() < nebula->GetRadius()) {
          double falloff = 1.0 - nebula->GetFalloff() * 
            (diffVector.length() / nebula->GetRadius());
          detail *= nebula->GetSensorEffect() * falloff;
        }
      }
      // If we no longer see it now, dump it after all.
      if (detail < 1.00) {
        continue;
      }

      if (detail > 100.00) {
        detail = 100.00;
      }
    }
    bool contactFound = false;
    for (unsigned int i = 0; i < computer->mSensorContacts.size(); i++) {
      HSSensorContact *contact = &computer->mSensorContacts[i];
      if (contact->Object == object) {
        contactFound = true;
        if (contact->Refreshed) {
          if (contact->Detail > detail) {
            break;
          }
          contact->Detail = detail;
          contact->DiffVector = diffVector;
          break;
        } else {
          contact->Refreshed = true;
          contact->Detail = detail;
          contact->DiffVector = diffVector;
          break;
        }
      }
    }
    if (contactFound) {
      continue;
    }
    HSSensorContact newContact;
    newContact.Detail = detail;
    newContact.Refreshed = true;
    newContact.Object = object;
    newContact.DiffVector = diffVector;
    bool idIsUsed = true;
    while (idIsUsed) {
      // Ensure that contact ID is not already in use.
      newContact.ID = rand() % 9000 + 1000;
      idIsUsed = false;
      for (unsigned int i = 0; i < computer->mSensorContacts.size(); i++) {
        if (computer->mSensorContacts[i].ID == newContact.ID) {
          idIsUsed = true;
        }
      }
    }
    computer->mSensorContacts.push_back(newContact);
    char tbuf[256];
    sprintf(tbuf, "New contact (%d) appeared on sensors", newContact.ID);
    mShip->NotifyConsolesFormatted("Sensors", tbuf);
  }
}
