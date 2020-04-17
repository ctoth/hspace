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
#include "HSDB.h"
#include "HSTools.h"
#include "HSUniverse.h"
#include "HSHullClass.h"
#include "HSObject.h"
#include "HSSystem.h"
#include "HSShip.h"
#include "HSConsole.h"
#include "HSLandingLocation.h"
#include "HSCommunications.h"
#include "HSConf.h"
#include "HSDockingHatch.h"
#include "HSCommodity.h"
#include "HSWarehouse.h"
#include "HSTerritory.h"
#include "HSResource.h"
#include "HSCargoBay.h"
#include "HSHarvester.h"

#define MAX_TEMP_OBJECTS -32767

// System
#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "string.h"

// STL
#include <string>
#include <sstream>
#include <algorithm>
#include <deque>

using namespace std;

extern "C" {
#include "sqlite3.h"
}

HSDB sHSDB;

//////////////////////////////////////////////////////////////////////////////////////////
// HSDB
HSDB::HSDB(void)
  : mInitialized(false)
  , mDB(0)
  , mLastCycleTime(0)
{
}

HSDB::~HSDB(void)
{
  foreach(HSUniverse*, universe, mUniverses) {
    delete universe;
  }
  foreach(HSHullClass*, hullClass, mHullClasses) {
    delete hullClass;
  }
  foreach(HSCommodity*, commodity, mCommodities) {
    delete commodity;
  }
  foreach(HSSystem*, system, mSystems) {
    delete system;
  }
  foreach(HSWarehouse*, warehouse, mWarehouses) {
    delete warehouse;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Public
bool
HSDB::Initialize()
{
  if (sqlite3_initialize() != SQLITE_OK) {
    return false;
  }
  mInitialized = true;

  return true;  
}

void
HSDB::Shutdown()
{
  if (!mInitialized) {
    return;
  }
  sqlite3_shutdown();
  mInitialized = false;
}

bool
HSDB::LoadDatabase()
{
  if (sqlite3_open("data/hspacedb.s3db", &mDB) != SQLITE_OK) {
    HSLog() << "Failed to open database file for loading.";
    return false;
  }
  HSLog() << "Loading database...";
  sqlite3_stmt *statement;
  const char *tail;
  int result;

  result = sqlite3_prepare_v2(mDB, "SELECT * FROM UNIVERSES", -1, &statement, &tail);
  if (result == SQLITE_OK) {
    while (sqlite3_step(statement) == SQLITE_ROW) {
      HSUniverse *newUniverse = new HSUniverse();
      RetrieveObjectFromDB(statement, newUniverse);
      mUniverses.push_back(newUniverse);
    }
  }
  sqlite3_finalize(statement);

  result = sqlite3_prepare_v2(mDB, "SELECT * FROM HULLCLASSES", -1, &statement, &tail);
  if (result == SQLITE_OK) {
    while (sqlite3_step(statement) == SQLITE_ROW) {
      HSHullClass *newHullClass = new HSHullClass();
      RetrieveObjectFromDB(statement, newHullClass);
      mHullClasses.push_back(newHullClass);
    }
  }
  sqlite3_finalize(statement);

  result = sqlite3_prepare_v2(mDB, "SELECT * FROM COMMODITIES", -1, &statement, &tail);
  if (result == SQLITE_OK) {
    while (sqlite3_step(statement) == SQLITE_ROW) {
      HSCommodity *newCommodity = new HSCommodity();
      RetrieveObjectFromDB(statement, newCommodity);
      mCommodities.push_back(newCommodity);
    }
  }
  sqlite3_finalize(statement);

  result = sqlite3_prepare_v2(mDB, "SELECT * FROM WAREHOUSES", -1, &statement, &tail);
  if (result == SQLITE_OK) {
    while (sqlite3_step(statement) == SQLITE_ROW) {
      HSWarehouse *newWarehouse = new HSWarehouse();
      RetrieveObjectFromDB(statement, newWarehouse);
      mWarehouses.push_back(newWarehouse);
    }
  }
  sqlite3_finalize(statement);

  result = sqlite3_prepare_v2(mDB, "SELECT * FROM TERRITORIES", -1, &statement, &tail);
  if (result == SQLITE_OK) {
    while (sqlite3_step(statement) == SQLITE_ROW) {
      HSTerritory *newTerritory = new HSTerritory();
      RetrieveObjectFromDB(statement, newTerritory);
      HSUniverse *univ = FindUniverse(newTerritory->GetUniverse());
      if (univ) {
        univ->AddTerritory(newTerritory);
      } else {
        HSLog() << "Territory (" << newTerritory << ") found with invalid universe while loading database.";
      }
    }
  }
  sqlite3_finalize(statement);

  // Iterate through all types and load from their designated tables.
  foreach(HSObjectType, type, HSObject::GetTypes()) {
    result = sqlite3_prepare_v2(mDB, 
      string("SELECT * FROM OBJECTS").append(HSObject::GetTableForType(type)).c_str(), 
      -1, &statement, &tail);
    if (result == SQLITE_OK) {
      while (sqlite3_step(statement) == SQLITE_ROW) {
        HSObject *newObject = HSObject::CreateObjectForType(type);
        RetrieveObjectFromDB(statement, newObject);
        HSUniverse *univ = FindUniverse(newObject->GetUniverse());
        if (univ) {
          univ->AddObject(newObject);
        } else {
          HSLog() << "Object (" << newObject << ") found with invalid universe while loading database.";
        }
        if (newObject->GetID() >= 0) {
          if (newObject->GetType() == HSOT_SHIP) {
            HSShip *newShip = static_cast<HSShip*>(newObject);
            if (newShip->GetLandedLoc() == -1) {
              HSIFTeleportObject(newObject->GetID(), newObject->GetUniverse());
            }
          } else {
            HSIFTeleportObject(newObject->GetID(), newObject->GetUniverse());
          }
        }
      }
    }
    sqlite3_finalize(statement);
  }

  // Push all docked ships to their dock objects lists.
  foreach(HSUniverse*, universe, GetUniverses()) {
    foreach(HSObject*, obj, universe->GetObjects()) {
      if (obj->GetType() == HSOT_SHIP) {
        HSShip *ship = 
          static_cast<HSShip*>(obj);
        if (ship->GetLandedObj() != -1) {
          HSObject *dstObj = FindObject(ship->GetLandedObj());
          if (dstObj) {
            dstObj->mLandedShips.push_back(ship);
          }
        }
      }
    }
  }

  // Iterate through all types and load from their designated tables.
  foreach(HSSystemType, type, HSSystem::GetTypes()) {
    result = sqlite3_prepare_v2(mDB, 
      string("SELECT * FROM SYSTEMS").append(HSSystem::GetTableForType(type)).c_str(), 
      -1, &statement, &tail);
    if (result == SQLITE_OK) {
      while (sqlite3_step(statement) == SQLITE_ROW) {
        HSSystem *newSystem = HSSystem::CreateSystemForType(type);
        RetrieveObjectFromDB(statement, newSystem);
        mSystems.push_back(newSystem);
      }
    }
    sqlite3_finalize(statement);
  }

  foreach(HSSystemType, type, HSSystem::GetTypes()) {
    result = sqlite3_prepare_v2(mDB, 
      string("SELECT * FROM SYSTEMINSTANCES").append(HSSystem::GetTableForType(type)).c_str(), 
      -1, &statement, &tail);
    if (result == SQLITE_OK) {
      while (sqlite3_step(statement) == SQLITE_ROW) {
        HSSystemInstance *newSystem = HSSystemInstance::CreateSystemInstanceForType(type);
        RetrieveObjectFromDB(statement, newSystem);
        HSObject *obj = FindObject(newSystem->GetShipID());
        if (obj) {
          if(obj->GetType() != HSOT_SHIP) {
            delete newSystem;
            continue;
          }
          HSSystem *system = sHSDB.FindSystem(newSystem->GetSystemID());
          if (!system) {
            // Eep! This system doesn't even exist!
            delete newSystem;
            continue;
          }
          HSShip *owningShip = static_cast<HSShip*>(obj);
          newSystem->SetSystem(system);
          newSystem->SetShip(owningShip);
          owningShip->mSystems.push_back(newSystem);
        } else {
          HSLog() << "System (" << newSystem << ") found with invalid ship ID.";
        }
      }
    }
    sqlite3_finalize(statement);
  }
  result = sqlite3_prepare_v2(mDB, "SELECT * FROM CONSOLES", -1, &statement, &tail);
  if (result == SQLITE_OK) {
    while (sqlite3_step(statement) == SQLITE_ROW) {
      HSConsole *newConsole = new HSConsole();
      RetrieveObjectFromDB(statement, newConsole);
      HSObject *obj = FindObject(newConsole->GetShipID());
      if (obj) {
        if(obj->GetType() != HSOT_SHIP) {
          delete newConsole;
          continue;
        }
        HSShip *owningShip = static_cast<HSShip*>(obj);
        owningShip->AddConsole(newConsole);
        newConsole->SetShip(owningShip);
        HSIFSetObjectLock(newConsole->GetID(), HSLock_Use, newConsole->GetID());
      } else {
        HSLog() << "Console (" << newConsole << ") found with invalid ship ID.";
      }
    }
  }
  sqlite3_finalize(statement);
  result = sqlite3_prepare_v2(mDB, "SELECT * FROM LANDINGLOCATIONS", -1, &statement, &tail);
  if (result == SQLITE_OK) {
    while (sqlite3_step(statement) == SQLITE_ROW) {
      HSLandingLocation *newLandingLoc = new HSLandingLocation();
      RetrieveObjectFromDB(statement, newLandingLoc);
      HSObject *obj = FindObject(newLandingLoc->GetObjectID());
      if (obj) {
        obj->AddLandingLocation(newLandingLoc);
      } else {
        HSLog() << "Landing location (" << newLandingLoc << ") found with invalid object ID.";
      }
    }
  }
  sqlite3_finalize(statement);
  result = sqlite3_prepare_v2(mDB, "SELECT * FROM DOCKINGHATCHES", -1, &statement, &tail);
  if (result == SQLITE_OK) {
    while (sqlite3_step(statement) == SQLITE_ROW) {
      HSDockingHatch *newDockingHatch = new HSDockingHatch();
      RetrieveObjectFromDB(statement, newDockingHatch);
        HSObject *obj = FindObject(newDockingHatch->GetObjectID());
      if (obj) {
        obj->AddDockingHatch(newDockingHatch);
      } else {
        HSLog() << "Docking hatch (" << newDockingHatch << " found with invalid object ID.";
      }
    }
  }
  sqlite3_finalize(statement);

  result = sqlite3_prepare_v2(mDB, "SELECT * FROM CONFIGURATION", -1, &statement, &tail);
  if (result == SQLITE_OK) {
    if (sqlite3_step(statement) == SQLITE_ROW) {
      RetrieveObjectFromDB(statement, &sHSConf);
    }
  }

  sqlite3_finalize(statement);

  sqlite3_close(mDB);
  mDB = NULL;
  HSLog() << "Database load completed.";
  return true;
}

bool
HSDB::DumpDatabase()
{
  if (sqlite3_open("data/hspacedb.s3db", &mDB) != SQLITE_OK) {
    HSLog() << "Failed to open database for dumping.";
    return false;
  }
  HSLog() << "Dumping database...";

  std::vector<HSAttributed*> universes;
  foreach(HSUniverse*, universe, mUniverses) {
    universes.push_back(universe);
  }
  if (!InsertListIntoDB("universes", universes)) {
    return false;
  }

  std::vector<HSAttributed*> hullClasses;
  foreach(HSHullClass*, hullClass, mHullClasses) {
    hullClasses.push_back(hullClass);
  }  
  if (!InsertListIntoDB("hullclasses", hullClasses)) {
    return false;
  }

  std::vector<HSAttributed*> commodities;
  foreach(HSCommodity*, commodity, mCommodities) {
    commodities.push_back(commodity);
  }
  if (!InsertListIntoDB("commodities", commodities)) {
    return false;
  }

  std::vector<HSAttributed*> warehouses;
  foreach(HSWarehouse*, warehouse, mWarehouses) {
    warehouses.push_back(warehouse);
  }
  if (!InsertListIntoDB("warehouses", warehouses)) {
    return false;
  }

  std::vector<HSAttributed*> objectsToStore[sObjectTypeCount];
  std::vector<HSAttributed*> territoriesToStore;

  foreach(HSUniverse*, universe, mUniverses) {
    std::vector<HSObject*> objs = universe->GetObjects();
    foreach(HSObject*, obj, objs) {
      objectsToStore[obj->GetType()].push_back(obj);
    }
    foreach(HSTerritory*, territory, universe->GetTerritories()) {
      territoriesToStore.push_back(territory);
    }
  }
  if (!InsertListIntoDB("TERRITORIES", territoriesToStore)) {
    return false;
  }

  for (int i = 0; i < sObjectTypeCount; i++) {
    if (!InsertListIntoDB(
      string("OBJECTS").append(HSObject::GetTableForType((HSObjectType)i)),
      objectsToStore[i])) {
        return false;
    }
  }

  std::vector<HSAttributed*> toBeStored[sSystemTypeCount];
  foreach(HSSystem*, system, mSystems) {
    toBeStored[system->GetType()].push_back(system);
  }

  for (int i = 0; i < sSystemTypeCount; i++) {
    if (!InsertListIntoDB(
      string("SYSTEMS").append(HSSystem::GetTableForType((HSSystemType)i)),
      toBeStored[i])) {
        return false;
    }
    toBeStored[i].clear();
  }

  std::vector<HSAttributed*> consolesToStore;
  std::vector<HSAttributed*> landingLocsToStore;
  std::vector<HSAttributed*> dockingHatchesToStore;

  foreach(HSUniverse*, universe, mUniverses) {
    std::vector<HSObject*> objs = universe->GetObjects();
    foreach(HSObject*, obj, objs) {
      foreach(HSLandingLocation*, landingLoc, obj->GetLandingLocations()) {
        landingLocsToStore.push_back(landingLoc);
      }
      foreach(HSDockingHatch*, dockingHatch, obj->GetDockingHatches()) {
        dockingHatchesToStore.push_back(dockingHatch);
      }
      if (obj->GetType() == HSOT_SHIP) {
        HSShip *ship = (HSShip*)obj;
        foreach(HSSystemInstance*, sys, ship->mSystems) {
          toBeStored[sys->GetType()].push_back(sys);
        }
        foreach(HSConsole*, console, ship->mConsoles) {
          consolesToStore.push_back(console);
        }
      }
    }
  }
  for (int i = 0; i < sSystemTypeCount; i++) {
    if (!InsertListIntoDB(
      string("SYSTEMINSTANCES").append(HSSystem::GetTableForType((HSSystemType)i)),
      toBeStored[i])) {
        return false;
    }
  }
  if (!InsertListIntoDB("CONSOLES", consolesToStore)) {
      return false;
  }
  if (!InsertListIntoDB("LANDINGLOCATIONS", landingLocsToStore)) {
      return false;
  }
  if (!InsertListIntoDB("DOCKINGHATCHES", dockingHatchesToStore)) {
      return false;
  }
  
  std::vector<HSAttributed*> conf;
  conf.push_back(&sHSConf);
  if (!InsertListIntoDB("CONFIGURATION", conf)) {
    return false;
  }
  sqlite3_close(mDB);
  mDB = NULL;
  HSLog() << "Dumping databases complete.";
  return true;
}

void
HSDB::Cycle()
{
#ifdef WIN32
  LARGE_INTEGER cycleStart;
  ::QueryPerformanceCounter(&cycleStart);
#else
  double firstms;
  struct timeval tv;
  gettimeofday(&tv, (struct timezone *) NULL);
  firstms = (tv.tv_sec * 1000000) + tv.tv_usec;
#endif
  foreach(HSUniverse*, universe, mUniverses) {
    universe->Cycle();
  }
#ifdef WIN32
  LARGE_INTEGER cycleEnd;
  ::QueryPerformanceCounter(&cycleEnd);
  LARGE_INTEGER cycleTime;
  cycleTime.QuadPart = cycleEnd.QuadPart - cycleStart.QuadPart;
  LARGE_INTEGER frequency;
  ::QueryPerformanceFrequency(&frequency);
  mLastCycleTime = (double)cycleTime.QuadPart / (double)frequency.QuadPart;
#else
  gettimeofday(&tv, (struct timezone *) NULL);
  mLastCycleTime = (double)((tv.tv_sec * 1000000) + tv.tv_usec - firstms) * 0.000001;
#endif
}

void
HSDB::AddUniverse(HSUniverse *aUniverse)
{
  if (!aUniverse) {
    return;
  }
  mUniverses.push_back(aUniverse);
}

HSUniverse*
HSDB::FindUniverse(int aID)
{
  foreach(HSUniverse*, universe, mUniverses) {
    if (universe->mID == aID) {
      return universe;
    }
  }
  return 0;
}

void
HSDB::DelUniverse(HSUniverse *aUniverse)
{
  if (aUniverse) {
    for (std::vector<HSUniverse*>::iterator iter = mUniverses.begin(); iter != mUniverses.end(); iter++) {
      if (*iter == aUniverse) {
        mUniverses.erase(iter);
        delete aUniverse;
        return;
      }
    }
  }
}

int
HSDB::AddHullClass(HSHullClass *aHullClass)
{
  if (!aHullClass) {
    return -1;
  }

  std::deque<int> usedIds;

  foreach(HSHullClass*, hullClass, mHullClasses) {
    usedIds.push_back(hullClass->GetID());
  }

  int i = 0;
  sort(usedIds.begin(), usedIds.end());
  for(std::deque<int>::iterator iterate = usedIds.begin(); iterate != usedIds.end(); iterate++) {
    if (*iterate != i++) {
      --i;
      break;
    }
  }
  mHullClasses.push_back(aHullClass);
  aHullClass->SetID(i);
  return i;
}

HSHullClass*
HSDB::FindHullClass(int aID)
{
  foreach(HSHullClass*, hullClass, mHullClasses) {
    if (hullClass->GetID() == aID) {
      return hullClass;
    }
  }
  return 0;
}

void
HSDB::DelHullClass(HSHullClass *aHullClass)
{
  if (aHullClass) {
    for (std::vector<HSHullClass*>::iterator iter = mHullClasses.begin(); iter != mHullClasses.end(); iter++) {
      if (*iter == aHullClass) {
        mHullClasses.erase(iter);
        delete aHullClass;
        return;
      }
    }
  }
}

int
HSDB::AddSystem(HSSystem *aSystem)
{
  if (!aSystem) {
    return -1;
  }

  std::deque<int> usedIds;

  foreach(HSSystem*, system, mSystems) {
    usedIds.push_back(system->GetID());
  }

  int i = 0;
  sort(usedIds.begin(), usedIds.end());
  for(std::deque<int>::iterator iterate = usedIds.begin(); iterate != usedIds.end(); iterate++) {
    if (*iterate != i++) {
      --i;
      break;
    }
  }
  mSystems.push_back(aSystem);
  aSystem->SetID(i);
  return i;
}

HSSystem*
HSDB::FindSystem(int aID)
{
  foreach(HSSystem*, system, mSystems) {
    if (system->GetID() == aID) {
      return system;
    }
  }
  return 0;
}

void
HSDB::DelSystem(HSSystem *aSystem)
{
  if (aSystem) {
    // First delete all instances of this sytem.
    foreach(HSUniverse*, universe, mUniverses) {
      foreach(HSObject*, object, universe->GetObjects()) {
        if (object->GetType() == HSOT_SHIP) {
          HSShip *ship = static_cast<HSShip*>(object);
          foreach(HSSystemInstance*, system, ship->GetSystems()) {
            if (system->GetSystem() == aSystem) {
              ship->RemoveSystem(system);
            }
          }
        }
      }
    }
    for (std::vector<HSSystem*>::iterator iter = mSystems.begin(); iter != mSystems.end(); iter++) {
      if (*iter == aSystem) {
        mSystems.erase(iter);
        delete aSystem;
        return;
      }
    }
  }
}

HSObject*
HSDB::FindObject(int aID)
{
  HSObject* obj;
  foreach(HSUniverse*, universe, mUniverses) {
    if ((obj = universe->FindObject(aID))) {
      return obj;
    }
  }
  return 0;
}

int
HSDB::FindObjectID()
{
  std::deque<int> usedIds;

  foreach(HSUniverse*, univ, mUniverses) {
    foreach(HSObject*, object, univ->GetObjects()) {
      usedIds.push_back(object->GetID());
    }
  }

  int i = 0;
  int next = -1;
  sort(usedIds.begin(), usedIds.end());
  if (*usedIds.begin() >= 0) {
    i = 0;
  } else {
    i = *usedIds.begin();
    next = i - 1;
  }
  for(std::deque<int>::iterator iterate = usedIds.begin(); iterate != usedIds.end(); iterate++) {
    if (*iterate >= 0) {
      break;
    }
    if (i++ != *iterate) {
      return --i;
    }
  }

  return next;
}
  
HSConsole*
HSDB::FindConsole(int aID)
{
  foreach(HSUniverse*, universe, mUniverses) {
    foreach(HSObject*, object, universe->GetObjects()) {
      if (object->GetType() == HSOT_SHIP) {
        HSShip *ship = static_cast<HSShip*>(object);
        HSConsole *console = ship->FindConsoleByID(aID);
        if (console) {
          return console;
        }
      }
    }
  }
  return 0;
}

HSConsole*
HSDB::FindConsoleMannedBy(DBRef aObject)
{
  foreach(HSUniverse*, universe, mUniverses) {
    foreach(HSObject*, object, universe->GetObjects()) {
      if (object->GetType() == HSOT_SHIP) {
        HSShip *ship = static_cast<HSShip*>(object);
        foreach(HSConsole*, console, ship->mConsoles) {
          if (HSIFGetObjectLock(console->GetID(), HSLock_Use) == aObject) {
            return console;
          }
        }
      }
    }
  }
  return 0;
}

HSLandingLocation*
HSDB::FindLandingLocation(int aID)
{
  foreach(HSUniverse*, universe, mUniverses) {
    foreach(HSObject*, object, universe->GetObjects()) {
      HSLandingLocation *landingLoc = object->FindLandingLocationByID(aID);
      if (landingLoc) {
        return landingLoc;
      }
    }
  }
  return 0;
}

HSDockingHatch*
HSDB::FindDockingHatch(int aID)
{
  foreach(HSUniverse*, universe, mUniverses) {
    foreach(HSObject*, object, universe->GetObjects()) {
      HSDockingHatch *dockingHatch = object->FindDockingHatchByID(aID);
      if (dockingHatch) {
        return dockingHatch;
      }
    }
  }
  return 0;
}

HSTerritory*
HSDB::FindTerritory(int aID)
{
  foreach(HSUniverse*, universe, mUniverses) {
    HSTerritory *territory = universe->FindTerritory(aID);
    if (territory) {
      return territory;
    }
  }
  return 0;
}

std::vector<HSUniverse*>
HSDB::GetUniverses()
{
  return mUniverses;
}

std::vector<HSHullClass*>
HSDB::GetHullClasses()
{
  return mHullClasses;
}

std::vector<HSSystem*>
HSDB::GetSystems()
{
  return mSystems;
}

double
HSDB::GetLastCycleTime()
{
  return mLastCycleTime;
}

int
HSDB::AddCommodity(HSCommodity *aCommodity)
{
  if (!aCommodity) {
    return -1;
  }

  std::deque<int> usedIds;

  foreach(HSCommodity*, commodity, mCommodities) {
    usedIds.push_back(commodity->GetID());
  }

  int i = 0;
  sort(usedIds.begin(), usedIds.end());
  for(std::deque<int>::iterator iterate = usedIds.begin(); iterate != usedIds.end(); iterate++) {
    if (*iterate != i++) {
      --i;
      break;
    }
  }
  mCommodities.push_back(aCommodity);
  aCommodity->SetID(i);
  return i;
}

HSCommodity*
HSDB::FindCommodity(int aID)
{
  foreach(HSCommodity*, commodity, mCommodities) {
    if (commodity->GetID() == aID) {
      return commodity;
    }
  }
  return 0;
}

void
HSDB::DelCommodity(HSCommodity *aCommodity)
{
  if (aCommodity) {
    for (std::vector<HSCommodity*>::iterator iter = mCommodities.begin(); iter != mCommodities.end(); iter++) {
      if (*iter == aCommodity) {
        mCommodities.erase(iter);
        // Find -ALL- locations in the universe holding cargo and make sure
        // this isn't referenced in any cargo item anymore.
        foreach(HSUniverse*, univ, mUniverses) {
          foreach(HSObject*, obj, univ->GetObjects()) {
            if (obj->GetType() == HSOT_RESOURCE) {
              static_cast<HSResource*>(obj)->SetCargoForCommodity(aCommodity->GetID(), 0.0);
            } else if (obj->GetType() == HSOT_SHIP) {
              foreach (HSSystemInstance*, sys, static_cast<HSShip*>(obj)->GetSystems()) {
                if (sys->GetType() == HSST_CARGOBAY) {
                  static_cast<HSCargoBayInstance*>(sys)->SetCargoForCommodity(aCommodity->GetID(), 0.0);
                } else if (sys->GetType() == HSST_HARVESTER) {
                  HSHarvesterInstance *harvester = static_cast<HSHarvesterInstance*>(sys);
                  if (harvester->GetCommodObj() == aCommodity) {
                    harvester->CancelHarvesting();
                  }
                }
              }
            }
          }
        }
        foreach (HSWarehouse*, warehouse, mWarehouses) {
          warehouse->SetCargoForCommodity(aCommodity->GetID(), 0.0);
        }
        delete aCommodity;
        return;
      }
    }
  }
}

std::vector<HSCommodity*>
HSDB::GetCommodities()
{
  return mCommodities;
}

int
HSDB::AddWarehouse(HSWarehouse *aWarehouse)
{
  if (!aWarehouse) {
    return -1;
  }

  std::deque<int> usedIds;

  foreach(HSWarehouse*, warehouse, mWarehouses) {
    usedIds.push_back(warehouse->GetID());
  }

  int i = 0;
  sort(usedIds.begin(), usedIds.end());
  for(std::deque<int>::iterator iterate = usedIds.begin(); iterate != usedIds.end(); iterate++) {
    if (*iterate != i++) {
      --i;
      break;
    }
  }
  mWarehouses.push_back(aWarehouse);
  aWarehouse->SetID(i);
  return i;
}

HSWarehouse*
HSDB::FindWarehouse(int aID)
{
  foreach(HSWarehouse*, warehouse, mWarehouses) {
    if (warehouse->GetID() == aID) {
      return warehouse;
    }
  }
  return 0;
}

void
HSDB::DelWarehouse(HSWarehouse *aWarehouse)
{
  if (aWarehouse) {
    for (std::vector<HSWarehouse*>::iterator iter = mWarehouses.begin(); iter != mWarehouses.end(); iter++) {
      if (*iter == aWarehouse) {
        mWarehouses.erase(iter);
        delete aWarehouse;
        return;
      }
    }
  }
}

std::vector<HSWarehouse*>
HSDB::GetWarehouses()
{
  return mWarehouses;
}

void
HSDB::ObjectDestroyed(DBRef aObject)
{
  HSObject *obj = FindObject(aObject);
  if (obj) {
    HSUniverse *univ = FindUniverse(obj->GetUniverse());
    if (univ) {
      univ->RemoveObject(obj);
    }
    delete obj;
  }
  HSLandingLocation *landingLoc = FindLandingLocation(aObject);
  if (landingLoc) {
    HSObject *obj = FindObject(landingLoc->GetObjectID());
    if (obj) {
      obj->RemoveLandingLocationByID(landingLoc->GetID());
    }
  }
  HSConsole *console = FindConsole(aObject);
  if (console) {
    HSShip *ship = console->GetShip();
    if (ship) {
      ship->RemoveConsoleByID(console->GetID());
    }
  }
  HSUniverse *universe = FindUniverse(aObject);
  DelUniverse(universe);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Private
bool
HSDB::InsertListIntoDB(string aTable, vector<HSAttributed*> aList)
{
  sqlite3_stmt *statement;
  const char *tail;
  int result;
  stringstream dropquery;
  dropquery << "DROP TABLE " << aTable;
  result = sqlite3_prepare_v2(mDB, dropquery.str().c_str(), -1, &statement, &tail);
  if (result == SQLITE_OK) {
    sqlite3_step(statement);
    sqlite3_finalize(statement);
  }
  if (aList.empty()) {
    // Nothing to do.
    return true;
  }

  HSAttributed *objectModel = *aList.begin();

  stringstream query;

  query << "CREATE TABLE [" << aTable << "] (";

  vector<Attribute> attrs = objectModel->mAttributeList;

  int i = 0;

  foreach(Attribute, attr, attrs) {
    if (i++) {
      query << ",";
    }
    switch (attr.Type) {
      case AT_INTEGER:
        query << "[" << attr.Name << "] INTEGER";
        break;
      case AT_STRING:
        query << "[" << attr.Name << "] VARCHAR(256)";
        break;
      case AT_DOUBLE:
        query << "[" << attr.Name << "] FLOAT";
        break;
      case AT_BOOLEAN:
        query << "[" << attr.Name << "] BOOLEAN";
        break;
      case AT_VECTOR:
        query << "[" << attr.Name << "X] FLOAT, [" << attr.Name << "Y] FLOAT, [" << attr.Name << "Z] FLOAT";
        break;
      case AT_INTLIST:
      case AT_COMMCHANNELS:
      case AT_CARGOLIST:
        query << "[" << attr.Name << "] TEXT";
        break;
    }
    query << " NULL";
  }
  query << ")";


  result = sqlite3_prepare_v2(mDB, query.str().c_str(), -1, &statement, &tail);
  if (result != SQLITE_OK) {
    HSLog() << "Failed to create database schema for " << objectModel;
    return false;
  }
  sqlite3_step(statement);
  result = sqlite3_finalize(statement);
  if (result != SQLITE_OK) {
    HSLog() << "Failed to create database schema for " << objectModel;
    return false;
  }

  char tbuf[512];

  foreach(HSAttributed*, obj, aList) {
    i = 0;
    stringstream insertquery;
    insertquery << "INSERT INTO " << aTable << " VALUES (";
    for(vector<Attribute>::iterator attiter = obj->mAttributeList.begin(); attiter != obj->mAttributeList.end(); attiter++) {
      Attribute attr = *attiter;
      if (i++) {
        insertquery << ",";
      }
      if (attr.IsSet && !*attr.IsSet) {
        if (attr.Type != AT_VECTOR) {
          insertquery << "NULL";
        } else {
          insertquery << "NULL, NULL, NULL";
        }
      } else {
        switch (attr.Type) {
          case AT_INTEGER:
            insertquery << *(int*)attr.Ptr;
            break;
          case AT_STRING:
            {
              string *toInsert = (string*)attr.Ptr;
              if (toInsert->empty()) {
                insertquery << "'" << *toInsert << "'";
              } else {
                char *str = sqlite3_mprintf("%q", toInsert->c_str());
                insertquery << "'" << str << "'";
                sqlite3_free(str);
              }
              break;
            }
          case AT_DOUBLE:
            sprintf(tbuf, "%.12f", *(double*)attr.Ptr);
            insertquery << tbuf;
            break;
          case AT_BOOLEAN:
            insertquery << *(bool*)attr.Ptr;
            break;
          case AT_VECTOR:
            sprintf(tbuf, "%f,%f,%f", (*(HSVector3D*)attr.Ptr).mX, (*(HSVector3D*)attr.Ptr).mY, (*(HSVector3D*)attr.Ptr).mZ);
            insertquery <<  tbuf;
            break;
          case AT_INTLIST:
            {
              insertquery << "'";
              foreach(int, num, *(std::vector<int>*)attr.Ptr) {
                insertquery << num << " ";
              }
              insertquery << "'";
            }
            break;
          case AT_COMMCHANNELS:
            {
              insertquery << "'";
              foreach(HSCommChannel, channel, *(std::vector<HSCommChannel>*)attr.Ptr) {
                insertquery << channel.Frequency << " " << channel.Encryption << " ";
              }
              insertquery << "'";
            }
            break;
          case AT_CARGOLIST:
            {
              insertquery << "'";
              char tbuf[256];
              foreach(HSCargoItem, item, *(std::vector<HSCargoItem>*)attr.Ptr) {
                sprintf(tbuf, "%d %f ", item.commod->GetID(), item.amount);
                insertquery << tbuf;
              }
              insertquery << "'";
            }
            break;
        }
      }
    }
    insertquery << ")";
    result = sqlite3_prepare_v2(mDB, insertquery.str().c_str(), -1, &statement, &tail);
    if (result != SQLITE_OK) {
      HSLog() << "Failed to dump object " << obj;
      return false;
    }
    sqlite3_step(statement);
    result = sqlite3_finalize(statement);
    if (result != SQLITE_OK) {
      HSLog() << "Failed to dump object " << obj;
      return false;
    }
  }

  return true;
}

bool
HSDB::RetrieveObjectFromDB(sqlite3_stmt *aStatement, HSAttributed *aObject)
{
  AttributeType type;
  string name;
  for (int i = 0; i < sqlite3_column_count(aStatement); i++) {
    name = sqlite3_column_name(aStatement, i);
    if (sqlite3_column_type(aStatement, i) == SQLITE_NULL) {
      continue;
    }
    if (aObject->GetAttributeType(name, &type)) {
      switch(type) {
        case AT_INTEGER:
        case AT_BOOLEAN:
          aObject->SetAttribute(name, sqlite3_column_int(aStatement, i), true);
          break;
        case AT_STRING:
          aObject->SetAttribute(name, std::string((const char*)sqlite3_column_text(aStatement, i)), true);
          break;
        case AT_DOUBLE:
          aObject->SetAttribute(name, sqlite3_column_double(aStatement, i), true);
          break;
        case AT_INTLIST:
          {
            std::string strlist = (const char*)sqlite3_column_text(aStatement, i);
            std::vector<int> list;
            int pos = (int)strlist.find(" ");
            while (pos != -1) {
              list.push_back(atoi(strlist.substr(0, pos).c_str()));
              strlist = strlist.substr(pos + 1);
              pos = (int)strlist.find(" ");
            }
            aObject->SetAttribute(name, list, true);
          }
          break;
        case AT_COMMCHANNELS:
          {
            std::string strlist = (const char*)sqlite3_column_text(aStatement, i);
            std::vector<HSCommChannel> list;
            int pos = (int)strlist.find(" ");
            while (pos != -1) {
              HSCommChannel newChannel;
              newChannel.Frequency = atof(strlist.substr(0, pos).c_str());
              strlist = strlist.substr(pos + 1);
              pos = (int)strlist.find(" ");
              if (pos == -1) {
                HSLog() << "Corrupted comm channel entry in " << aObject;
                return false;
              }
              newChannel.Encryption = strlist.substr(0, pos);
              strlist = strlist.substr(pos + 1);
              list.push_back(newChannel);
              pos = (int)strlist.find(" ");
            }
            aObject->SetAttribute(name, list, true);
          }
        case AT_CARGOLIST:
          {
            std::string strlist = (const char*)sqlite3_column_text(aStatement, i);
            std::vector<HSCargoItem> list;
            int pos = (int)strlist.find(" ");
            while (pos != -1) {
              HSCargoItem newCargoItem;
              HSCommodity *commod = FindCommodity(atoi(strlist.substr(0, pos).c_str()));
              newCargoItem.commod = commod;
              strlist = strlist.substr(pos + 1);
              pos = (int)strlist.find(" ");
              if (pos == -1) {
                HSLog() << "Corrupted cargo entry in " << aObject;
                return false;
              }
              newCargoItem.amount = atof(strlist.substr(0, pos).c_str());
              strlist = strlist.substr(pos + 1);
              list.push_back(newCargoItem);
              pos = (int)strlist.find(" ");
            }
            aObject->SetAttribute(name, list, true);
          }
        default:
          break;
      }
    } else if (aObject->GetAttributeType(name.substr(0, name.length() - 1), &type)) {
      if (type == AT_VECTOR) {
        aObject->SetAttribute(name.substr(0, name.length() - 1), HSVector3D(sqlite3_column_double(aStatement, i), sqlite3_column_double(aStatement, i + 1), sqlite3_column_double(aStatement, i + 2)), true);
        i += 2;
      }
    }
  }
  return true;
}
