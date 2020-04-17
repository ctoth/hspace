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
#include "HSObject.h"
#include "HSSystem.h"
#include "HSIface.h"

class HSSystemInstance;
class HSConsole;
class HSHullClass;
class HSComputerInstance;

/**
 * \ingroup HS_OBJECTS
 * \brief This is the class for the ship object.
 */
class HSShip :
  public HSObject
{
  /** Class of hull this ship has */
  ATTRIBUTE(HullClass, int)
  /** Current hull points describing structural integrity of the ship */
  ATTRIBUTE(CurrentHullPoints, int)
  /** Normalized vector describing where the nose is pointing */
  ATTRIBUTE(Heading, HSVector3D)
  /** Normalized vector describing where we want the nose to point */
  ATTRIBUTE(DesiredHeading, HSVector3D)
  /** Integer describing the desired velocity of the vessel */
  ATTRIBUTE(DesiredVelocity, double)
  /** Integer referring to a landing location that the vessel is landed at */
  ATTRIBUTE(LandedLoc, int)
  /** Integer referring to the actual object that this ship is landed with */
  ATTRIBUTE(LandedObj, int)
  /** Integer referring to a room that the ship is boarded to and from */
  ATTRIBUTE(HatchLoc, int)
  /** Vector of integers for the rooms that are on this ship */
  ATTRIBUTE(ShipRooms, std::vector<int>)
  // Non settable attributes, stored for performance.
  ATTRIBUTE(Online, bool)
  ATTRIBUTE(Hull, HSHullClass*)
  ATTRIBUTE(Accelerating, bool)
  ATTRIBUTE(EnginesCut, bool)
  ATTRIBUTE(LandingTimer, int)
public:
  HSShip(void);
  ~HSShip(void);

  void Cycle();
  bool IsInSpace();

  /**
   * Returns true if the ship has a dimensional drive engaged.
   */
  bool HasDDEngaged();

  /**
   * Returns a list of systems of a certain type.
   *
   * @param aType Type of systems to find
   * @return List of systems
   */
  std::vector<HSSystemInstance*> FindSystemsByType(HSSystemType aType);

  /**
   * Add a system instance to the ship
   *
   * @param aSystem System instance to add
   */
  void AddSystem(HSSystemInstance *aSystem);

  /**
   * Remove a system from the ship by name
   *
   * @param aName Name of the system to remove
   * @return True if succeeded, false if not found
   */
  bool RemoveSystemByName(std::string aName);

  /**
   * Remove a system from the ship
   *
   * @param aName System to remove
   * @return True if succeeded, false if not found
   */
  bool RemoveSystem(HSSystemInstance *aSystem);

  /**
   * Add a console to the ship
   *
   * @param aConsole Console to add to this ship
   */
  void AddConsole(HSConsole *aConsole);

  /**
   * Remove a console from the ship based on its ID (console's DBRef)
   *
   * @param aID ID of console to remove
   * @return True if succesfully removed, false if not found
   */
  bool RemoveConsoleByID(int aID);

  /**
   * Find a console by its ID
   *
   * @param aID ID of the console to look up
   * @return Pointer to a HSConsole object if found, null if not found.
   */
  HSConsole *FindConsoleByID(int aID);

  /**
   * Find systems by their name
   *
   * @param aName Name of the system to look for
   * @return List of HSSystemInstances pointing to systems matching aName.
   */
  std::vector<HSSystemInstance*> FindSystemsByName(std::string aName);

  /**
   * Find a single system by its name
   *
   * @param aName Name of the system to look for, after the name it may contain
   * a '/' in order to specify which system its looking for in case of multiple matches.
   * @return Pointer to a HSSystemInstance
   */
  HSSystemInstance* FindSystemByName(std::string aName);

  /**
   * Send a message to all consoles.
   *
   * @param aMsg Message to send to all consoles
   */
  void NotifyConsoles(std::string aMsg);

  /**
   * Send a pre-formatted message to all consoles.
   *
   * @param aSource Source of the message
   * @param aMsg Message to send
   */
  void NotifyConsolesFormatted(std::string aSource, std::string aMsg);

  /**
   * Send a message to all rooms in the ship
   *
   * @param aMsg Message to send to all rooms
   */
  void NotifySRooms(std::string aMsg);

  /**
   * Sends a notive to surrounding ships in space
   * that have us on sensors.
   *
   * @param aDet Detail from which the message is visible
   * @param aMsg Message to display
   */
  void NotifySurroundingSpace(int aDet, std::string aMsg);

  /**
   * Get the total amount of power output
   *
   * @return Total amount of power
   */
  int GetPowerOutput();

  /**
   * Get the total amount of available power
   *
   * @return Total amount of available power
   */
  int GetAvailablePower();

  /**
   * Get the total mass of the vessel.
   *
   * @return Total mass of the vessel
   */
  int GetTotalMass();

  /**
   * Get the maximum velocity of the vessel
   *
   * @return Maximum velocity
   */
  int GetMaxVelocity();

  /**
   * Get a list of all systems on the ship
   *
   * @return List of all systems
   */
  std::vector<HSSystemInstance*> GetSystems();

  /**
   * Cycles the engines based on the combined thrust
   * and desired speed of the vessel.
   */
  void CycleEngines();

  /**
   * Cycles the thrusters based on the combined angular velocity
   * and desired heading of the vessel.
   */
  void CycleThrusters();

  /**
   * Cycle a landing sequence, give some nice messages to the
   * surrounding area and such.
   */
  void CycleLanding();

  /**
   * Cycle a launching sequence, give some nice messages to the
   * surrounding area and such.
   */
  void CycleLaunching();

  /**
   * Check if systems aren't taking more power than required
   * shutdown systems if needed.
   */
  void CheckSystemPower();

  /**
   * Called when the ship is destroyed.
   */
  void DestroyShip();
  
  /**
   * Handles incoming damage for thisobject.
   *
   * @param aDamage Amount of damage delt.
   * @param aDirection Direction the damage comes from.
   * @return Result of the damage.
   */
  virtual HSDamageResult HandleDamage(double aDamage, HSVector3D aDirection);

  /**
   * Handles weapon impact.
   *
   * @param aDamage Amount of damage done.
   * @param aWeapon Weapon that is doing the damage.
   */
  void HandleWeaponImpact(double aDamage, HSWeaponInstance *aWeapon);

  /**
   * Handles weapon impact.
   *
   * @param aWeapon Weapon that missed.
   */
  void HandleWeaponMiss(HSWeaponInstance *aWeapon);

  /**
   * Finds the computer on this ship.
   *
   * @return Computer instance for this ship.
   */
  HSComputerInstance *FindComputer();

  /**
   * Clones the ship.
   *
   * @return Cloned ship
   */
  HSShip *Clone();

  std::string GetAdditionalInfo();

  std::vector<Attribute> GetExtraAttributes();

protected:
  virtual void AttributeChanged(std::string aName);
private:
  friend class HSDB;
  friend void HSCommandClone(DBRef aObject, std::string aArgument);

  std::vector<HSSystemInstance*> mSystems;
  std::vector<HSConsole*> mConsoles;
  /** Used to store the console dbrefs when needed for extra attributes */
  std::vector<int> mConsoleDBRefs;
};
