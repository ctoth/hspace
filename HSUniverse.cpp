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
#include "HSUniverse.h"
#include "HSObject.h"
#include "HSTools.h"
#include "HSShip.h"
#include "HSComputer.h"
#include "HSDB.h"
#include "HSTerritory.h"
#include "HSNebula.h"

#include <sstream>

using namespace std;

HSUniverse::HSUniverse()
  : mID(0)
{
  ADD_ATTRIBUTE_INTERNAL("ID", AT_INTEGER, mID)
  ADD_ATTRIBUTE("NAME", AT_STRING, mName)
}

HSUniverse::~HSUniverse(void)
{
  // Place all our objects in the remaining universes.
  // If none exist, destroy all of them.
  std::vector<HSUniverse*> universes = sHSDB.GetUniverses();
  if (!universes.size()) {
    foreach(HSObject*, object, mObjects) {
      delete object;
    }
    foreach(HSTerritory*, territory, mTerritories) {
      delete territory;
    }  
  } else {
    HSUniverse *newUniv = *universes.begin();
    foreach(HSObject*, object, mObjects) {
      newUniv->AddObject(object);
      object->SetUniverse(newUniv->GetID());
    }
    foreach(HSTerritory*, territory, mTerritories) {
      newUniv->AddTerritory(territory);
      territory->SetUniverse(newUniv->GetID());
    }
  }
}

void
HSUniverse::AddObject(HSObject *aObject)
{
  if (!aObject) {
    return;
  }
  mObjects.push_back(aObject);
}

HSObject*
HSUniverse::FindObject(int aID)
{
  foreach(HSObject*, object, mObjects) {
    if (object->GetID() == aID) {
      return object;
    }
  }
  return 0;
}

bool
HSUniverse::FindObject(HSObject *aObject)
{
  foreach(HSObject*, object, mObjects) {
    if (object == aObject) {
      return true;
    }
  }
  return false;
}

bool
HSUniverse::RemoveObject(HSObject *aObject)
{
  if (!mObjects.size()) {
    return false;
  }
  if (aObject) {
    for (std::vector<HSObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); iter++) {
      if (*iter == aObject) {
        mObjects.erase(iter);
        return true;
      }
    }
  }
  return false;
}

std::vector<HSObject*>
HSUniverse::GetObjects()
{
  return mObjects;
}

void
HSUniverse::AddTerritory(HSTerritory *aTerritory)
{
  if (!aTerritory) {
    return;
  }
  mTerritories.push_back(aTerritory);
}

HSTerritory*
HSUniverse::FindTerritory(int aID)
{
  foreach(HSTerritory*, territory, mTerritories) {
    if (territory->GetID() == aID) {
      return territory;
    }
  }
  return 0;
}

bool
HSUniverse::RemoveTerritory(HSTerritory *aTerritory)
{
  if (!mTerritories.size()) {
    return false;
  }
  if (aTerritory) {
    for (std::vector<HSTerritory*>::iterator iter = mTerritories.begin(); iter != mTerritories.end(); iter++) {
      if (*iter == aTerritory) {
        mTerritories.erase(iter);
        return true;
      }
    }
  }
  return false;
}

std::vector<HSTerritory*>
HSUniverse::GetTerritories()
{
  return mTerritories;
}

void
HSUniverse::NotifyWeaponsFire(HSObject *aSource, HSObject *aVictim, HSDamageResult aDamage)
{
  foreach(HSObject*, object, mObjects) {
    if (object->GetType() != HSOT_SHIP) {
      continue;
    }
    if (!object->IsInSpace()) {
      continue;
    }
    if (object == aSource || object == aVictim) {
      continue;
    }
    HSShip *ship = static_cast<HSShip*>(object);
    HSComputerInstance *computer = ship->FindComputer();
    if (!computer || !computer->IsOnline()) {
      continue;
    }

    HSSensorContact source;
    source.Object = 0;
    HSSensorContact victim;
    victim.Object = 0;
    foreach(HSSensorContact, contact, computer->mSensorContacts) {
      if (contact.Object == aSource) {
        source = contact;
      }
      if (contact.Object == aVictim) {
        victim = contact;
      }
    }

    char tbuf[256];
    if (!source.Object && !victim.Object) {
      continue;
    }
    char sourceStr[128];
    char victimStr[128];
    if (source.Object) {
      if (source.Detail > 50) {
        sprintf(sourceStr, "The %s (%d)", source.Object->GetName().c_str(), source.ID);
      } else {
        sprintf(sourceStr, "Contact %d", source.ID);
      }
    }
    if (victim.Object) {
      if (victim.Detail > 50) {
        sprintf(victimStr, "the %s (%d)", victim.Object->GetName().c_str(), victim.ID);
      } else {
        sprintf(victimStr, "contact %d", victim.ID);
      }
    }


    if (source.Object && !victim.Object) {
      sprintf(tbuf, "%s fires on an unknown target", sourceStr);
    } else if (!source.Object && victim.Object) {
      if (aDamage != HSDR_NONE) {
        if (victim.Detail >= 90) {
          if (aDamage == HSDR_HULL) {
            sprintf(tbuf, "An unknown vessel fires and strikes the hull of %s", victimStr);
          } else {
            sprintf(tbuf, "An unknown vessel's weapons fire is stopped by the shields of %s", victimStr);
          }
        } else {
          sprintf(tbuf, "An unknown vessel fires and hits %s", victimStr);
        }
      } else {
        sprintf(tbuf, "An unknown vessel fires and misses %s", victimStr);
      }
    } else {
      if (aDamage != HSDR_NONE) {
        if (victim.Detail >= 90) {
          if (aDamage == HSDR_HULL) {
            sprintf(tbuf, "%s fires and strikes the hull of %s", sourceStr, victimStr);
          } else {
            sprintf(tbuf, "%s its weapons fire is stopped by the shields of %s", sourceStr, victimStr);
          }
        } else {
          sprintf(tbuf, "%s fires and hits %s", sourceStr, victimStr);
        }
      } else {
        sprintf(tbuf, "%s fires and misses %s", sourceStr, victimStr);
      }
    }
    ship->NotifyConsolesFormatted("Combat", tbuf);
  }
}

void
HSUniverse::NotifyObjectPose(HSObject *aSource, std::string aType, std::string aPose)
{
  foreach(HSObject*, object, mObjects) {
    if (object->GetType() != HSOT_SHIP) {
      continue;
    }
    if (!object->IsInSpace()) {
      continue;
    }
    if (object == aSource) {
      continue;
    }
    HSShip *ship = static_cast<HSShip*>(object);
    HSComputerInstance *computer = ship->FindComputer();
    if (!computer || !computer->IsOnline()) {
      continue;
    }

    HSSensorContact source;
    source.Object = 0;
    foreach(HSSensorContact, contact, computer->mSensorContacts) {
      if (contact.Object == aSource) {
        source = contact;
      }
    }

    if (!source.Object) {
      continue;
    }
    char sourceStr[128];
    if (source.Object) {
      if (source.Detail > 50) {
        sprintf(sourceStr, "The %s (%d)", source.Object->GetName().c_str(), source.ID);
      } else {
        sprintf(sourceStr, "Contact %d", source.ID);
      }
    }
    std::stringstream notification;
    notification << sourceStr << aPose;
    ship->NotifyConsolesFormatted(aType, notification.str());
  }
}

void
HSUniverse::Cycle()
{
  std::vector<HSStoredPose> posesToExecute = mPoses;
  mPoses.clear();
  foreach(HSObject*, object, mObjects) {
    object->Cycle();
  }
  foreach(HSStoredPose, pose, posesToExecute) {
    if (pose.nominative) {
      NotifyObjectAffectPose(pose.source, pose.pose, pose.nominative, pose.type);
    } else {
      NotifyObjectPose(pose.source, pose.type, pose.pose);
    }
  }
}

void
HSUniverse::PushPose(const HSStoredPose &aPose)
{
  mPoses.push_back(aPose);
}

void
HSUniverse::NotifyObjectAffectPose(HSObject *aSource, std::string aPose, HSObject *aNominative, std::string aType)
{
  foreach(HSObject*, object, mObjects) {
    if (object->GetType() != HSOT_SHIP) {
      continue;
    }
    if (!object->IsInSpace()) {
      continue;
    }
    if (object == aSource || object == aNominative) {
      continue;
    }
    HSShip *ship = static_cast<HSShip*>(object);
    HSComputerInstance *computer = ship->FindComputer();
    if (!computer || !computer->IsOnline()) {
      continue;
    }

    HSSensorContact source;
    source.Object = 0;
    HSSensorContact nominative;
    nominative.Object = 0;
    foreach(HSSensorContact, contact, computer->mSensorContacts) {
      if (contact.Object == aSource) {
        source = contact;
      }
      if (contact.Object == aNominative) {
        nominative = contact;
      }
    }

    char tbuf[256];
    if (!source.Object && !nominative.Object) {
      continue;
    }
    char sourceStr[128];
    char nominativeStr[128];
    if (source.Object) {
      if (source.Detail > 50) {
        if (source.Object->GetType() == HSOT_SHIP) {
          sprintf(sourceStr, "The %s (%d)", source.Object->GetName().c_str(), source.ID);
        } else {
          sprintf(sourceStr, "%s (%d)", source.Object->GetName().c_str(), source.ID);
        }
      } else {
        sprintf(sourceStr, "Contact %d", source.ID);
      }
    }
    if (nominative.Object) {
      if (nominative.Detail > 50) {
        if (nominative.Object->GetType() == HSOT_SHIP) {
          sprintf(nominativeStr, "the %s (%d)", nominative.Object->GetName().c_str(), nominative.ID);
        } else {
          sprintf(nominativeStr, "%s (%d)", nominative.Object->GetName().c_str(), nominative.ID);
        }
      } else {
        sprintf(nominativeStr, "contact %d", nominative.ID);
      }
    }


    if (source.Object && !nominative.Object) {
      sprintf(tbuf, "%s %s an unknown object", sourceStr, aPose.c_str());
    } else if (!source.Object && nominative.Object) {
      sprintf(tbuf, "An unknown source %s %s", aPose.c_str(), nominativeStr);
    } else {
      sprintf(tbuf, "%s %s %s", sourceStr, aPose.c_str(), nominativeStr);
    }
    ship->NotifyConsolesFormatted(aType, tbuf);
  }
}

double
HSUniverse::GetShieldModifier(const HSVector3D &aLocation)
{
  double shieldModified = 1.0;
  foreach(HSObject*, object, mObjects) {
    HSVector3D diffVector = object->GetLocation() - aLocation;
    if (object->GetType() == HSOT_NEBULA) {
      HSNebula *nebula = static_cast<HSNebula*>(object);
      if (diffVector.length() < nebula->GetRadius()) {
        double falloff = 1.0 - nebula->GetFalloff() * 
            (diffVector.length() / nebula->GetRadius());
        shieldModified *= (1.0 - nebula->GetShieldEffect()) * falloff;
      }
    }
  }
  return shieldModified;
}
