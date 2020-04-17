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
#include "HSObject.h"
#include "HSPlanet.h"
#include "HSShip.h"
#include "HSTools.h"
#include "HSIface.h"
#include "HSLandingLocation.h"
#include "HSConf.h"
#include "HSDB.h"
#include "HSUniverse.h"
#include "HSDockingHatch.h"
#include "HSJumpGate.h"
#include "HSResource.h"
#include "HSCargoPod.h"
#include "HSNebula.h"
#include "HSAsteroids.h"

HSObject::HSObject(void)
  : mID(0)
  , mType(0)
  , mUniverse(0)
  , mSize(0)
{
  ADD_ATTRIBUTE_INTERNAL("ID", AT_INTEGER, mID)
  ADD_ATTRIBUTE_INTERNAL("TYPE", AT_INTEGER, mType)
  ADD_ATTRIBUTE("NAME", AT_STRING, mName)
  ADD_ATTRIBUTE("UNIVERSE", AT_INTEGER, mUniverse)
  ADD_ATTRIBUTE("LOCATION", AT_VECTOR, mLocation)
  ADD_ATTRIBUTE("VELOCITY", AT_VECTOR, mVelocity)
  ADD_ATTRIBUTE("SIZE", AT_INTEGER, mSize)
}

HSObject::~HSObject(void)
{
  foreach(HSLandingLocation*, loc, mLandingLocations) {
    delete loc;
  }
  foreach(HSDockingHatch*, hatch, mDockingHatches) {
    delete hatch;
  }
}

void
HSObject::Cycle()
{
  if (!mVelocity.zero()) {
    if (mType == HSOT_SHIP) {
      HSShip *ship = static_cast<HSShip*>(this);
      if (ship->HasDDEngaged()) {
        mLocation += mVelocity * sHSConf.DDMultiplier;
      } else {
        mLocation += mVelocity;
      }
    } else {
      mLocation += mVelocity;
    }
    foreach(HSDockingHatch*, hatch, mDockingHatches) {
      if (hatch->GetLinkedTo() == DBRefNothing) {
        continue;
      }
      // Expensive lookup, might want to cache.
      HSDockingHatch *connHatch = sHSDB.FindDockingHatch(hatch->GetLinkedTo());
      if (!connHatch) {
        HSLog() << "Database inconsistency, invalid linked to location.";
        continue;
      }
      HSObject *obj = sHSDB.FindObject(connHatch->GetObjectID());
      if (!obj) {
        HSLog() << "Database inconsistency, invalid owner for hatch.";
        continue;
      }
      HSVector3D distVec = GetLocation() - obj->GetLocation();
      if (distVec.length() > sHSConf.DockingDistance) {
        hatch->Disconnect();
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////////
// Static
std::vector<HSObjectType>
HSObject::GetTypes()
{
  std::vector<HSObjectType> types;
  for (int i = 0; i != HSOT_END; i++) {
    types.push_back((HSObjectType)i);
  }
  return types;
}

std::string
HSObject::GetTableForType(HSObjectType aType)
{
  switch (aType) {
    case HSOT_GENERIC:
      return "GENERIC";
    case HSOT_PLANET:
      return "PLANETS";
    case HSOT_SHIP:
      return "SHIPS";
    case HSOT_JUMPGATE:
      return "JUMPGATES";
    case HSOT_RESOURCE:
      return "RESOURCES";
    case HSOT_CARGOPOD:
      return "CARGOPODS";
    case HSOT_NEBULA:
      return "NEBULAE";
    case HSOT_ASTEROIDS:
      return "ASTEROIDS";
    default:
      return "";
  }
}

std::string
HSObject::GetNameForType(HSObjectType aType)
{
  switch (aType) {
    case HSOT_GENERIC:
      return "Generic Object";
    case HSOT_PLANET:
      return "Planet";
    case HSOT_SHIP:
      return "Ship";
    case HSOT_JUMPGATE:
      return "Jump Gate";
    case HSOT_RESOURCE:
      return "Resource";
    case HSOT_CARGOPOD:
      return "Cargo Pod";
    case HSOT_NEBULA:
      return "Nebula";
    case HSOT_ASTEROIDS:
      return "Asteroids";
    default:
      return "";
  }
}

HSObject*
HSObject::CreateObjectForType(HSObjectType aType)
{
  switch (aType) {
    case HSOT_GENERIC:
      return new HSObject();
    case HSOT_PLANET:
      return new HSPlanet();
    case HSOT_SHIP:
      return new HSShip();
    case HSOT_JUMPGATE:
      return new HSJumpGate();
    case HSOT_RESOURCE:
      return new HSResource();
    case HSOT_CARGOPOD:
      return new HSCargoPod();
    case HSOT_NEBULA:
      return new HSNebula();
    case HSOT_ASTEROIDS:
      return new HSAsteroids();
    default:
      return NULL;
  }
}


std::string
HSObject::DistanceString(double aDistance)
{
  std::string unit = " km";
  double displayed = aDistance;

  if (displayed >= 30856775807000ULL) {
    displayed /= 30856775807000ULL;
    unit = " PC";
  } else if (displayed >= 149597871000ULL) {
    displayed /= 149597871000ULL;
    unit = "kAU";
  } else if (displayed >= 149597871) {
    displayed /= 149597871;
    unit = " AU";
  } else if (displayed >= 1000000) {
    unit = " Gm";
    displayed /= 1000000;
  } else if (displayed >= 1000) {
    unit = " Mm";
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

void
HSObject::AddLandingLocation(HSLandingLocation *aLocation)
{
  mLandingLocations.push_back(aLocation);
}

bool
HSObject::RemoveLandingLocationByID(int aID)
{
  if (mLandingLocations.empty()) {
    return false;
  }
  for(std::vector<HSLandingLocation*>::iterator iter = mLandingLocations.begin();
    iter != mLandingLocations.end();iter++) {
      if ((*iter)->GetID() == aID) {
        delete (*iter);
        mLandingLocations.erase(iter);
        return true;
      }
  }
  return false;
}

HSLandingLocation*
HSObject::FindLandingLocationByID(int aID)
{
  if (mLandingLocations.empty()) {
    return false;
  }
  foreach(HSLandingLocation*, landingLoc, mLandingLocations) {
    if (landingLoc->GetID() == aID) {
      return landingLoc;
    }
  }
  return NULL;
}

std::vector<HSLandingLocation*>
HSObject::GetLandingLocations()
{
  return mLandingLocations;
}

void
HSObject::AddDockingHatch(HSDockingHatch *aHatch)
{
  mDockingHatches.push_back(aHatch);
}

HSDockingHatch*
HSObject::FindDockingHatchByID(int aID)
{
  if (mDockingHatches.empty()) {
    return false;
  }
  foreach(HSDockingHatch*, dockingHatch, mDockingHatches) {
    if (dockingHatch->GetID() == aID) {
      return dockingHatch;
    }
  }
  return NULL;
}

bool
HSObject::RemoveDockingHatchByID(int aID)
{
  if (mDockingHatches.empty()) {
    return false;
  }
  for(std::vector<HSDockingHatch*>::iterator iter = mDockingHatches.begin();
    iter != mDockingHatches.end();iter++) {
      if ((*iter)->GetID() == aID) {
        delete (*iter);
        mDockingHatches.erase(iter);
        return true;
      }
  }
  return false;
}

std::vector<HSDockingHatch*>
HSObject::GetDockingHatches()
{
  return mDockingHatches;
}

std::vector<Attribute>
HSObject::GetExtraAttributes()
{
  std::vector<Attribute> retval;
  mLandingLocDBRefs.clear();
  foreach(HSLandingLocation*, landingLoc, mLandingLocations) {
    mLandingLocDBRefs.push_back(landingLoc->GetID());
  }
  Attribute attr = { "LANDINGLOCATIONS", AT_INTLIST, true, &mLandingLocDBRefs, NULL };
  retval.push_back(attr);

  mDockingHatchDBRefs.clear();
  foreach(HSDockingHatch*, dockingHatch, mDockingHatches) {
    mLandingLocDBRefs.push_back(dockingHatch->GetID());
  }
  Attribute attr2 = { "DOCKINGHATCHES", AT_INTLIST, true, &mDockingHatchDBRefs, NULL };
  retval.push_back(attr2);

  return retval;
}

void
HSObject::AttributeChanged(std::string aName)
{
  if (aName == "NAME") {
    if (mID >= 0) {
      HSIFSetName(mID, mName.c_str());
    }
  } else if (aName == "UNIVERSE") {
    HSUniverse *oldUniverse = 0;
    foreach(HSUniverse*, universe, sHSDB.GetUniverses()) {
      if(universe->FindObject(this)) {
        oldUniverse = universe;
        break;
      }
    }
    if (!oldUniverse) {
      // Bail out.
      return;
    }
    HSUniverse *newUniverse = sHSDB.FindUniverse(mUniverse);
    if (!newUniverse) {
      mUniverse = oldUniverse->GetID();
      newUniverse = oldUniverse;
    }
    if (mID >= 0) {
      HSIFTeleportObject(mID, mUniverse);
    }
    oldUniverse->RemoveObject(this);
    newUniverse->AddObject(this);
  }
}
