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

#include "HSVector3D.h"
#include "HSAttributed.h"
#include "HSGlobal.h"
#include <vector>

enum HSObjectType
{
  HSOT_GENERIC = 0,
  HSOT_PLANET,
  HSOT_SHIP,
  HSOT_JUMPGATE,
  HSOT_RESOURCE,
  HSOT_CARGOPOD,
  HSOT_NEBULA,
  HSOT_ASTEROIDS,
  HSOT_END
};

const int sObjectTypeCount = HSOT_END;

class HSLandingLocation;
class HSWeaponInstance;
class HSDockingHatch;
class HSDB;
class HSShip;

/**
 * \ingroup HS_OBJECTS
 * \brief This is the base class for all space objects.
 */
class HSObject
  : public HSAttributed
{
  /** 
   * ID of the object, corresponds to MUSH object dbref for persistent objects
   * negative for temporary objects.
   */
  ATTRIBUTE(ID, int)
  /** Type of the object (see HSObjectType) */
  ATTRIBUTE(Type, int)
  /** Name of the object */
  ATTRIBUTE(Name, std::string)
  /** Universe the object is in, corresponds to universe ID */
  ATTRIBUTE(Universe, int)
  /** Location of the object */
  ATTRIBUTE(Location, HSVector3D)
  /** Motion vector of the object */
  ATTRIBUTE(Velocity, HSVector3D)
  /** Size of the object */
  ATTRIBUTE(Size, int)
public:
  HSObject(void);
  virtual ~HSObject(void);

  virtual void Cycle();
  virtual bool IsInSpace() { return true; }

  static std::vector<HSObjectType> GetTypes();
  static std::string GetTableForType(HSObjectType aType);
  static std::string GetNameForType(HSObjectType aType);
  static HSObject* CreateObjectForType(HSObjectType aType);

  static std::string DistanceString(double aDistance);

  /** 
   * Add a landing location to this object.
   * \warning This takes ownership of the object
   *
   * @param aLocation Location to add
   */
  void AddLandingLocation(HSLandingLocation *aLocation);
  
  /**
   * Remove a landing location from this object.
   * \warning This deletes the associated object
   *
   * @param aID ID of the location to remove
   * @return True if the location was succesfully found and removed
   */
  bool RemoveLandingLocationByID(int aID);

  /**
   * Find a landing location by its ID
   *
   * @param aID ID of the location
   * @return NULL if not found
   */
  HSLandingLocation *FindLandingLocationByID(int aID);

  /**
   * Get all landing locations on this object
   *
   * @return A list of landing locations
   */
  std::vector<HSLandingLocation*> GetLandingLocations();

  /** 
   * Add a docking hatch to this object.
   * \warning This takes ownership of the object
   *
   * @param aLocation Location to add
   */
  void AddDockingHatch(HSDockingHatch *aLocation);

  /**
   * Find a docking hatch by its ID
   *
   * @param aID ID of the atch
   * @return NULL if not found
   */
  HSDockingHatch *FindDockingHatchByID(int aID);
  
  /**
   * Remove a docking hatch from this object.
   * \warning This deletes the associated object
   *
   * @param aID ID of the hatch to remove
   * @return True if the location was succesfully found and removed
   */
  bool RemoveDockingHatchByID(int aID);

  /**
   * Get all docking hatchs on this object
   *
   * @return A list of docking hatchs
   */
  std::vector<HSDockingHatch*> GetDockingHatches();
  
  /**
   * Handles incoming damage for thisobject.
   *
   * @param aDamage Amount of damage delt.
   * @param aDirection Direction the damage comes from.
   * @return Result of the damage.
   */
  virtual HSDamageResult HandleDamage(double aDamage, HSVector3D aDirection) { return HSDR_HULL; }

  /**
   * Handles weapon impact.
   *
   * @param aDamage Amount of damage done.
   * @param aWeapon Weapon that is doing the damage.
   */
  virtual void HandleWeaponImpact(double aDamage, HSWeaponInstance *aWeapon) {}

  /**
   * Handles a weapon miss.
   *
   * @param A weapon Weapon that missed.
   */
  virtual void HandleWeaponMiss(HSWeaponInstance *aWeapon) {}

  /**
   * Send a pre-formatted message to all consoles.
   *
   * @param aSource Source of the message
   * @param aMsg Message to send
   */
  virtual void NotifyConsolesFormatted(std::string aSource, std::string aMsg) {}

  /** Ships landed here, stored as optimization */
  std::vector<HSShip*> mLandedShips;

  std::vector<Attribute> GetExtraAttributes();

protected:
  virtual void AttributeChanged(std::string aName);

private:
  std::vector<HSLandingLocation*> mLandingLocations;
  std::vector<HSDockingHatch*> mDockingHatches;

  // These are for returning the dbrefs from GetExtraAttributes.
  std::vector<int> mLandingLocDBRefs;
  std::vector<int> mDockingHatchDBRefs;
};
