/**
 *
 * Hemlock Space 5 (HSpace 5)
 * Copyright (c) 2009, Bas Schouten
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

#include "HSRepairSystem.h"
#include "HSShip.h"
#include "HSConsole.h"
#include "HSTools.h"
#include "HSDB.h"
#include "HSUniverse.h"
#include "HSHullClass.h"

#include <list>
#include <sstream>
#include <map>

#define HULL_ID -10

HSRepairSystem::HSRepairSystem()
  : mRepairCapacity(0)
{
  mType = HSST_REPAIRSYSTEM;
  ADD_ATTRIBUTE("REPAIRCAPACITY", AT_DOUBLE, mRepairCapacity)
}

HSRepairSystem::~HSRepairSystem()
{
}

HSRepairSystemInstance::HSRepairSystemInstance()
{
  mType = HSST_REPAIRSYSTEM;
  ADD_ATTRIBUTE_INHERIT("REPAIRCAPACITY", AT_DOUBLE, mRepairCapacity)
  ADD_ATTRIBUTE_INTERNAL("TARGETSHIPIDS", AT_INTLIST, mTargetShipIDs)
  ADD_ATTRIBUTE_INTERNAL("TARGETSYSTEMIDS", AT_INTLIST, mTargetSystemIDs)
  ADD_ATTRIBUTE_INTERNAL("TARGETSYSTEMCOUNTS", AT_INTLIST, mTargetSystemCounts)
}

HSRepairSystemInstance::~HSRepairSystemInstance()
{
}

void
HSRepairSystemInstance::AssignToSystemOn(HSConsole *aConsole, std::string aShip, std::string aSystem)
{
  HSShip *targetShip = NULL;
  foreach(HSShip*, ship, mShip->mLandedShips) {
    if (!strncasecmp(ship->GetName().c_str(), aShip.c_str())) {
      targetShip = ship;
      break;
    }
  }
  if (!targetShip) {
    aConsole->NotifyUserSimple("No such ship landed with us.");
    return;
  }

  int systemID = HULL_ID;
  int count = 0;
  std::string systemName = "hull";

  if (strncasecmp(aSystem.c_str(), "HULL")) {
    HSSystemInstance *sys = targetShip->FindSystemByName(aSystem);

    if (!sys) {
      aConsole->NotifyUserSimple("No such target system found.");
      return;
    }
    systemID = sys->GetSystemID();
    systemName = sys->GetName();

    foreach(HSSystemInstance*, system, targetShip->GetSystems()) {
      if (system == sys) {
        break;
      }
      if (system->GetSystemID() == sys->GetSystemID()) {
        count++;
      }
    }
  }
  for (unsigned int i = 0; i < mTargetSystemIDs.size(); i++) {
    if (mTargetShipIDs[i] != targetShip->GetID()) {
      continue;
    }
    if (mTargetSystemIDs[i] == systemID) {
      if (mTargetSystemCounts[i] == count) {
        aConsole->NotifyUserSimple("Repair system already assigned to that system.");
        return;
      }
    }
  }
  char tbuf[256];
  sprintf(tbuf, "%s assigned to work on %s of the %s. Now assigned to work on %d system(s).",
    GetName().c_str(), systemName.c_str(), targetShip->GetName().c_str(), (int)mTargetSystemIDs.size() + 1);
  mShip->NotifyConsolesFormatted("Repair Systems", tbuf);
  sprintf(tbuf, "The %s has begun repairs on our %s.",
    mShip->GetName().c_str(), systemName.c_str());
  targetShip->NotifyConsolesFormatted("Repairs", tbuf);
  mTargetSystemIDs.push_back(systemID);
  mTargetSystemCounts.push_back(count);
  mTargetShipIDs.push_back(targetShip->GetID());
}

void
HSRepairSystemInstance::UnassignFromSystemOn(HSConsole *aConsole, std::string aShip, std::string aSystem)
{  
  HSShip *targetShip = NULL;
  foreach(HSShip*, ship, mShip->mLandedShips) {
    if (!strncasecmp(ship->GetName().c_str(), aShip.c_str())) {
      targetShip = ship;
      break;
    }
  }
  if (!targetShip) {
    aConsole->NotifyUserSimple("No such ship landed with us.");
    return;
  }
  std::vector<int> newTargetSystems;
  std::vector<int> newTargetCounts;
  std::vector<int> newTargetShips;

  int count = 0;
  int systemID = HULL_ID;
  std::string systemName = "hull";

  if (strncasecmp(aSystem.c_str(), "HULL")) {
    HSSystemInstance *sys = targetShip->FindSystemByName(aSystem);

    if (!sys) {
      aConsole->NotifyUserSimple("No such system found.");
      return;
    }
    foreach(HSSystemInstance*, system, targetShip->GetSystems()) {
      if (system == sys) {
        break;
      }
      if (system->GetSystemID() == sys->GetSystemID()) {
        count++;
      }
    }
    systemID = sys->GetSystemID();
    systemName = sys->GetName();
  }
  bool found = false;
  for (unsigned int i = 0; i < mTargetSystemIDs.size(); i++) {
    if (mTargetShipIDs[i] == targetShip->GetID() &&
        mTargetSystemIDs[i] == systemID) {
      if (mTargetSystemCounts[i] == count) {
        found = true;
        continue;
      }
    }
    newTargetSystems.push_back(mTargetSystemIDs[i]);
    newTargetShips.push_back(mTargetShipIDs[i]);
    newTargetCounts.push_back(mTargetSystemCounts[i]);
  }

  if (!found) {
    aConsole->NotifyUserSimple("Not working on that system.");
    return;
  }
  char tbuf[256];
  sprintf(tbuf, "%s ceased repairs of %s on %s.", 
    GetName().c_str(), systemName.c_str(), targetShip->GetName().c_str());
  mShip->NotifyConsolesFormatted("Repair Systems", tbuf);
  sprintf(tbuf, "The %s ceased repairs on our %s.",
    mShip->GetName().c_str(), systemName.c_str());
  targetShip->NotifyConsolesFormatted("Repairs", tbuf);
  mTargetSystemIDs = newTargetSystems;
  mTargetSystemCounts = newTargetCounts;
  mTargetShipIDs = newTargetShips;
}

void
HSRepairSystemInstance::AssignToSystem(HSConsole *aConsole, std::string aSystem)
{
  int systemID = HULL_ID;
  int count = 0;
  std::string systemName = "hull";

  if (strncasecmp(aSystem.c_str(), "HULL")) {
    HSSystemInstance *sys = mShip->FindSystemByName(aSystem);

    if (!sys) {
      aConsole->NotifyUserSimple("No such target system found.");
      return;
    }
    foreach(HSSystemInstance*, system, mShip->GetSystems()) {
      if (system == sys) {
        break;
      }
      if (system->GetSystemID() == sys->GetSystemID()) {
        count++;
      }
    }
    systemID = sys->GetSystemID();
    systemName = sys->GetName();
  }
  for (unsigned int i = 0; i < mTargetSystemIDs.size(); i++) {
    if (mTargetShipIDs[i] != 0) {
      continue;
    }
    if (mTargetSystemIDs[i] == systemID) {
      if (mTargetSystemCounts[i] == count) {
        aConsole->NotifyUserSimple("Repair system already assigned to that system.");
        return;
      }
    }
  }

  char tbuf[256];
  sprintf(tbuf, "%s assigned to work on %s. Now assigned to work on %d system(s).",
    GetName().c_str(), systemName.c_str(), (int)mTargetSystemIDs.size() + 1);
  mShip->NotifyConsolesFormatted("Repair Systems", tbuf);
  mTargetSystemIDs.push_back(systemID);
  mTargetSystemCounts.push_back(count);
  mTargetShipIDs.push_back(0);
}

void
HSRepairSystemInstance::UnassignFromSystem(HSConsole *aConsole, std::string aSystem)
{
  std::vector<int> newTargetSystems;
  std::vector<int> newTargetCounts;
  std::vector<int> newTargetShips;

  int count = 0;
  int systemID = HULL_ID;
  std::string systemName = "hull";

  if (strncasecmp(aSystem.c_str(), "HULL")) {
    HSSystemInstance *sys = mShip->FindSystemByName(aSystem);

    if (!sys) {
      aConsole->NotifyUserSimple("No such system found.");
      return;
    }
    foreach(HSSystemInstance*, system, mShip->GetSystems()) {
      if (system == sys) {
        break;
      }
      if (system->GetSystemID() == sys->GetSystemID()) {
        count++;
      }
    }
    systemID = sys->GetSystemID();
    systemName = sys->GetName();
  }
  bool found = false;
  for (unsigned int i = 0; i < mTargetSystemIDs.size(); i++) {
    if (mTargetShipIDs[i] == 0 &&
        mTargetSystemIDs[i] == systemID) {
      if (mTargetSystemCounts[i] == count) {
        found = true;
        continue;
      }
    }
    newTargetSystems.push_back(mTargetSystemIDs[i]);
    newTargetShips.push_back(mTargetShipIDs[i]);
    newTargetCounts.push_back(mTargetSystemCounts[i]);
  }

  if (!found) {
    aConsole->NotifyUserSimple("Not working on that system.");
    return;
  }
  char tbuf[256];
  sprintf(tbuf, "Ceased repairs of %s.", systemName.c_str());
  mShip->NotifyConsolesFormatted("Repair systems", tbuf);
  mTargetSystemIDs = newTargetSystems;
  mTargetSystemCounts = newTargetCounts;
  mTargetShipIDs = newTargetShips;
}

void
HSRepairSystemInstance::GiveReport(HSConsole *aConsole)
{
  std::stringstream notification;
  char tbuf[256];
  sprintf(tbuf, "%s.----------------------------------------------------------------------------.%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s|%s Ship Repair Status %55s %s|%s\n", ANSI_BLUE, ANSI_YELLOW, GetName().c_str(), ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s >-----------------------------.-------------------.-------------.----------<%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s|%s System%s                       |%s        Ship%s       |  %sCondition%s  |  %sRepairs  %s|%s\n", ANSI_BLUE, 
    ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_YELLOW, ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  sprintf(tbuf, "%s >-----------------------------+-------------------+-------------+----------<%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;

  for (unsigned int i = 0; i < mTargetSystemIDs.size(); i++) {
    HSShip *targetShip = mShip;
    foreach(HSShip*, ship, mShip->mLandedShips) {
      if (ship->GetID() == mTargetShipIDs[i]) {
        targetShip = ship;
        break;
      }
    }
    if (!targetShip) {
      continue;
    }
    if (mTargetSystemIDs[i] != HULL_ID) {
      int count = 0;
      HSSystemInstance *foundSystem = 0;
      foreach(HSSystemInstance*, system, targetShip->GetSystems()) {
        if (system->GetSystemID() == mTargetSystemIDs[i]) {
          if (count == mTargetSystemCounts[i]) {
            foundSystem = system;
            break;
          }
          count++;
        }
      }
      if (!foundSystem) {
        continue;
      }
      double repCap = GetRepairCapacity(true) / mTargetSystemIDs.size();
      int timeLeft = (int)(((1.0 - foundSystem->GetCondition()) * foundSystem->GetMass()) / repCap);
      sprintf(tbuf, "%s|%s %-28s %s|%s %-17s %s|%s%s|%s %9s %s|%s\n", ANSI_BLUE, ANSI_NORMAL,
        foundSystem->GetName().c_str(), ANSI_BLUE, ANSI_NORMAL, targetShip->GetName().c_str(), ANSI_BLUE, 
        HSMeter(13, 1.0, foundSystem->GetCondition()).c_str(), ANSI_BLUE, ANSI_NORMAL, 
        HSTimeString(timeLeft).c_str(), ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    } else {
      double repCap = GetRepairCapacity(true) / mTargetSystemIDs.size();
      int timeLeft = (int)(((1.0 - (double)targetShip->GetCurrentHullPoints() / 
        targetShip->GetHull()->GetHullPoints()) * targetShip->GetHull()->GetMass()) / repCap);
      sprintf(tbuf, "%s|%s %-28s %s|%s %-17s %s|%s%s|%s %9s %s|%s\n", ANSI_BLUE, ANSI_NORMAL,
        "Hull", ANSI_BLUE, ANSI_NORMAL, targetShip->GetName().c_str(), ANSI_BLUE, 
        HSMeter(13, 1.0, (double)targetShip->GetCurrentHullPoints() / targetShip->GetHull()->GetHullPoints()).c_str(), 
        ANSI_BLUE, ANSI_NORMAL, HSTimeString(timeLeft).c_str(), ANSI_BLUE, ANSI_NORMAL);
      notification << tbuf;
    }
  }

  sprintf(tbuf, "%s'------------------------------^-------------------^-------------^-----------'%s\n", ANSI_BLUE, ANSI_NORMAL);
  notification << tbuf;
  aConsole->NotifyUser(notification.str());
}

void
HSRepairSystemInstance::Cycle()
{
  std::list<HSSystemInstance*> systemsToWorkOn;
  char tbuf[256];
  
  if (mTargetSystemIDs.size() != mTargetSystemCounts.size() ||
      mTargetSystemIDs.size() != mTargetShipIDs.size()) {
    HSLog() << this << "Invalid state on repair system target systems.";
    return;
  }

  std::vector<int> newTargetSystems;
  std::vector<int> newTargetCounts;
  std::vector<int> newTargetShips;
  for (unsigned int i = 0; i < mTargetSystemIDs.size(); i++) {
    HSShip *targetShip = NULL;
    if (mTargetShipIDs[i] == 0) {
      targetShip = mShip;
    } else {
      foreach(HSShip*, ship, mShip->mLandedShips) {
        if (ship->GetID() == mTargetShipIDs[i]) {
          targetShip = ship;
          break;
        }
      }
      if (!targetShip) {
        HSUniverse *univ = sHSDB.FindUniverse(mShip->GetUniverse());
        if (!univ) {
          // Eek!
          continue;
        }
        HSObject *object = univ->FindObject(mTargetShipIDs[i]);
        if (!object) {
          // Woah!
          continue;
        }
        sprintf(tbuf, "Not able to work on %s.", object->GetName().c_str());
        mShip->NotifyConsolesFormatted("Repair Systems", tbuf);
        continue;
      }
    }
    double repCap = (double)GetRepairCapacity(true) / mTargetSystemIDs.size();
    if (mTargetSystemIDs[i] == HULL_ID) {
      double percFixed = repCap / targetShip->GetHull()->GetMass();
      int newHullPoints = (int)(targetShip->GetCurrentHullPoints() + 
        (double)targetShip->GetHull()->GetHullPoints() * percFixed);

      if (newHullPoints > targetShip->GetHull()->GetHullPoints()) {
        targetShip->SetCurrentHullPoints(targetShip->GetHull()->GetHullPoints());
        sprintf(tbuf, "Finished hull repairs on the %s.", targetShip->GetName().c_str());
        mShip->NotifyConsolesFormatted("Repair system", tbuf);
        if (targetShip != mShip) {
          sprintf(tbuf, "The %s finished repairs on our hull.", mShip->GetName().c_str());
          targetShip->NotifyConsolesFormatted("Repairs", tbuf);
        }
      } else {
        targetShip->SetCurrentHullPoints(newHullPoints);
        newTargetSystems.push_back(HULL_ID);
        newTargetShips.push_back(mTargetShipIDs[i]);
        newTargetCounts.push_back(0);
      }
      continue;
    }
    int count = 0;
    foreach(HSSystemInstance*, system, targetShip->GetSystems()) {
      if (system->GetSystemID() == mTargetSystemIDs[i]) {
        if (count == mTargetSystemCounts[i]) {
          double percFixed = repCap / system->GetMass();
          double newCondition = system->GetCondition() + percFixed;
          if (newCondition >= 1.0) {
            system->SetCondition(1.0);
            sprintf(tbuf, "Finished repairs of %s on the %s.",
              system->GetName().c_str(), targetShip->GetName().c_str());
            mShip->NotifyConsolesFormatted("Repair system", tbuf);
            if (targetShip != mShip) {
              sprintf(tbuf, "The %s finished repairs on our %s.",
                mShip->GetName().c_str(), system->GetName().c_str());
              targetShip->NotifyConsolesFormatted("Repairs", tbuf);
            }
          } else {
            system->SetCondition(newCondition);
            newTargetSystems.push_back(mTargetSystemIDs[i]);
            newTargetShips.push_back(mTargetShipIDs[i]);
            newTargetCounts.push_back(mTargetSystemCounts[i]);
          }
        }
        count++;
      }
    }
  }
  mTargetSystemIDs = newTargetSystems;
  mTargetSystemCounts = newTargetCounts;
  mTargetShipIDs = newTargetShips;
}
