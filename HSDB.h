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
#pragma once

struct sqlite3;
struct sqlite3_stmt;

// Forward declarations
class HSUniverse;
class HSHullClass;
class HSAttributed;
class HSObject;
class HSSystem;
class HSConsole;
class HSLandingLocation;
class HSDockingHatch;
class HSCommodity;
class HSWarehouse;
class HSTerritory;

// STL includes
#include <vector>
#include <string>

// Local
#include "HSIface.h"

struct HSTypeTable {
  int Type;
  std::string TableName;
};

/**
 * \ingroup HS_CORE
 * \brief This class holds the HSpace database. A static instance
 *        is used for all storage.
 */
class HSDB
{
public:
  HSDB(void);
  ~HSDB(void);

  /**
   * Initialize the database system.
   *
   * @return Boolean indicating success or failure.
   */
  bool Initialize();

  /**
   * Shutdown the database system.
   */
  void Shutdown();

  /**
   * Load the database into memory.
   *
   * @return Boolean indicating success or failure.
   */
  bool LoadDatabase();

  /**
   * Dump the database to disk
   *
   * @return Boolean indicating succes of failure.
   */
  bool DumpDatabase();

  /**
   * Cycles all universes and system instances,
   * the driving force in the HSpace realm.
   */
  void Cycle();

  /**
   * Add a Universe to the Universe database.
   * @warning This function takes ownership of the object.
   *
   * @param aUniverse The object containing the universe.
   */
  void AddUniverse(HSUniverse *aUniverse);

  /**
   * Finds a Universe based on the ID.
   *
   * @param aID ID of the universe
   * @return Universe with matching ID, NULL if not found.
   */
  HSUniverse* FindUniverse(int aID);

  /**
   * Deleted a Universe from the Universe database.
   * @warning This function will free the universe, the memory will be deallocated.
   *
   * @param aUniverse The universe to be deleted.
   */
  void DelUniverse(HSUniverse *aUniverse);

  /**
   * Adds a hullclass to the database, it calculated the ID and
   * sets it on the object.
   * @warning This function takes ownership of the object.
   *
   * @param Hullclass to add.
   * @return ID assigned to it
   */
  int AddHullClass(HSHullClass *aHullClass);

  /**
   * Finds a hull class based on the ID.
   *
   * @param aID ID of the hull class
   * @return Hull class with matching ID, NULL if not found.
   */
  HSHullClass* FindHullClass(int aID);

  /**
   * Deleted a HullClass from the HullClass database.
   * @warning The memory for the object will be deallocated.
   *
   * @param aHullClass The hullclass to be deleted.
   */
  void DelHullClass(HSHullClass *aHullClass);

  /**
   * Adds a system to the database, it calculated the ID and
   * sets it on the object.
   * @warning This function takes ownership of the object.
   *
   * @param System to add.
   * @return ID assigned to it
   */
  int AddSystem(HSSystem *aSystem);

  /**
   * Finds a system based on the ID.
   *
   * @param aID ID of the system
   * @return System with matching ID, NULL if not found.
   */
  HSSystem* FindSystem(int aID);

  /**
   * Delete a system from the system database.
   * @warning The memory for the object will be deallocated.
   *
   * @param aSystem The system to be deleted.
   */
  void DelSystem(HSSystem *aSystem);

  /**
   * Finds an object through all universes.
   *
   * @param aID The ID of the object.
   */
  HSObject* FindObject(int aID);

  /**
   * Finds an unused ID for non-persistent objects.
   *
   * @return Available ID, always negative. 0 if none found.
   */
  int FindObjectID();

  /**
   * Finds a console through all ships and universes.
   *
   * @param aID The ID of the console.
   */
  HSConsole* FindConsole(int aID);

  /**
   * Finds a console manned by an object.
   *
   * @param aObject Object that needs to be manning a console.
   * @return Console being manned, or null if not found.
   */
  HSConsole *FindConsoleMannedBy(DBRef aUser);

  /**
   * Finds a landing location through all objects and universes.
   *
   * @param aID The ID of the landing location.
   */
  HSLandingLocation* FindLandingLocation(int aID);

  /**
   * Finds a docking hatch through all objects and universes.
   *
   * @param aID The ID of the landing location.
   */
  HSDockingHatch* FindDockingHatch(int aID);

  /**
   * Finds a territory though all universe.
   *
   * @param aID The ID of the territory.
   */
  HSTerritory* FindTerritory(int aID);

  /**
   * Returns a vector of universes.
   *
   * @return List of all universes.
   */
  std::vector<HSUniverse*> GetUniverses();
  
  /**
   * Returns a vector of hull classes.
   *
   * @return List of all hull classes.
   */
  std::vector<HSHullClass*> GetHullClasses();
  
  /**
   * Returns a vector of systems.
   *
   * @return List of all systems.
   */
  std::vector<HSSystem*> GetSystems();

  /**
   * Returns the time the last cycle took in seconds
   *
   * @return Cycle time in seconds
   */
  double GetLastCycleTime();

  /**
   * Adds a commodity to the database, it calculated the ID and
   * sets it on the object.
   * @warning This function takes ownership of the object.
   *
   * @param Commodity to add.
   * @return ID assigned to it
   */
  int AddCommodity(HSCommodity *aCommodity);

  /**
   * Finds a commodity based on the ID.
   *
   * @param aID ID of the commodity
   * @return Commodity class with matching ID, NULL if not found.
   */
  HSCommodity* FindCommodity(int aID);

  /**
   * Deleted a commodity from the commodity database.
   * @warning The memory for the object will be deallocated.
   *
   * @param aCommodity The commodity to be deleted.
   */
  void DelCommodity(HSCommodity *aCommodity);
  
  /**
   * Returns a vector of hull classes.
   *
   * @return List of all hull classes.
   */
  std::vector<HSCommodity*> GetCommodities();

  /**
   * Adds a warehouse to the database, it calculated the ID and
   * sets it on the object.
   * @warning This function takes ownership of the object.
   *
   * @param Warehouse to add.
   * @return ID assigned to it
   */
  int AddWarehouse(HSWarehouse *aWarehouse);

  /**
   * Finds a warehouse based on the ID.
   *
   * @param aID ID of the warehouse
   * @return Warehouse class with matching ID, NULL if not found.
   */
  HSWarehouse* FindWarehouse(int aID);

  /**
   * Deleted a warehouse from the warehouse database.
   * @warning The memory for the object will be deallocated.
   *
   * @param aWarehouse The warehouse to be deleted.
   */
  void DelWarehouse(HSWarehouse *aWarehouse);
  
  /**
   * Returns a vector of hull classes.
   *
   * @return List of all hull classes.
   */
  std::vector<HSWarehouse*> GetWarehouses();

  /**
   * Indicates an object on the MUSH has been destroyed.
   * We need to clean up any related space objects.
   *
   * @param aObject Object that was destroyed.
   */
  void ObjectDestroyed(DBRef aObject);

private:
  /**
   * Insert a bunch of attributed objects into a table.
   *
   * @param aTable Table name to use for dumping
   * @param aList List of objects to insert
   * @return Bool indicating success or failure.
   */
  bool InsertListIntoDB(std::string aTable, std::vector<HSAttributed*> aList);

  /**
   * Get attributes from an object from the DB.
   *
   * @param aTable Table name to use for retrieval
   * @param aObject Object to set the attributes on
   * @return Bool indicating success or failure
   */
  bool RetrieveObjectFromDB(sqlite3_stmt *aTable, HSAttributed *aObject);

  bool mInitialized;

  sqlite3 *mDB;

  double mLastCycleTime;

  std::vector<HSUniverse*> mUniverses;
  std::vector<HSHullClass*> mHullClasses;
  std::vector<HSSystem*> mSystems;
  std::vector<HSCommodity*> mCommodities;
  std::vector<HSWarehouse*> mWarehouses;
};

extern HSDB sHSDB;
