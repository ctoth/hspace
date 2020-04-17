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
#include "HSAttributed.h"
#include "HSGlobal.h"
#include <string>

class HSObject;
class HSTerritory;

/**
 * \ingroup HS_CORE
 * \brief This class is used to store ship poses to be executed
 * after the full universe has been cycled and all contacts have been
 * updated.
 */
struct HSStoredPose
{
  HSStoredPose()
    : nominative(NULL)
  {
  }
  HSObject *source;
  /** Null if not a pose with a nominative */
  HSObject *nominative;
  std::string type;
  std::string pose;
};

/**
 * \ingroup HS_CORE
 * \brief This is the class that describes a universe
 */
class HSUniverse :
  public HSAttributed
{
  /** Name of the universe */
  ATTRIBUTE(Name, std::string)
  /** ID of the universe */
  ATTRIBUTE(ID, int)
public:
  HSUniverse(void);
  ~HSUniverse(void);

  /**
   * Add an object to the object vector.
   * @warning This function takes ownership of the object.
   *
   * @param aObject The object containing the universe.
   */
  void AddObject(HSObject *aObject);

  /**
   * Finds an object based on the ID.
   *
   * @param aID ID of the object
   * @return Object with matching ID, NULL if not found.
   */
  HSObject* FindObject(int aID);

  /**
   * Finds an object based on the pointers
   *
   * @param aObject Pointer to the object
   * @return True if found
   */
  bool FindObject(HSObject *aObject);

  /**
   * Remove an object from the object vector.
   * @warning This function will NOT free the object, it hands ownership to
   *          the caller.
   *
   * @param aObject The object to be deleted.
   * @return True if the object was found.
   */
  bool RemoveObject(HSObject *aObject);

  /**
   * Returns a vector of all objects in this universe.
   *
   * @return List of all objects.
   */
  std::vector<HSObject*> GetObjects();

  /**
   * Add an territory to the territory vector.
   * @warning This function takes ownership of the territory.
   *
   * @param aTerritory The territory containing the universe.
   */
  void AddTerritory(HSTerritory *aTerritory);

  /**
   * Finds an territory based on the ID.
   *
   * @param aID ID of the territory
   * @return Territory with matching ID, NULL if not found.
   */
  HSTerritory* FindTerritory(int aID);

  /**
   * Remove an territory from the territory vector.
   * @warning This function will NOT free the territory, it hands ownership to
   *          the caller.
   *
   * @param aTerritory The territory to be deleted.
   * @return True if the territory was found.
   */
  bool RemoveTerritory(HSTerritory *aTerritory);

  /**
   * Returns a vector of all territorys in this universe.
   *
   * @return List of all territorys.
   */
  std::vector<HSTerritory*> GetTerritories();

  /**
   * Notifies all ships in space about weapons fire.
   *
   * @param aSource The object firing.
   * @param aVictim The object being fired upon.
   * @param aDamage Type of damage done.
   */
  void NotifyWeaponsFire(HSObject *aSource, HSObject *aVictim, HSDamageResult aDamage);

  /**
   * Posts a 'pose' for a certain object to surrounding space.
   * For example NotifyObjectPose(this, "explodes in a big fireball!") would
   * display something like 'The SS George (1382) explodes in a big fireball.'
   * Or 'Contact 1382 explodes in a big fireball.'
   *
   * @param aSource The object doing the pose.
   * @param aType The 'type' of event, as in HSShip::NotifyConsolesFormatted
   * @param aPose The pose being done.
   */
  void NotifyObjectPose(HSObject *aSource, std::string aType, std::string aPose);

  /**
   * Cycles the universe and all vessels within
   */
  void Cycle();

  /**
   * Pushes a pose to execute after next cycle, that is when all sensor contacts
   * have been updated.
   *
   * @param aPose Stored pose object to push onto the pose stack.
   */
  void PushPose(const HSStoredPose &aPose);

  /**
   * Posts a 'pose' for two objects, the pose always starts with the source
   * and ends with the victim. So for example: 
   * NotifyObjectAffectPose(this, "disappears in a bright flash as it gates", gate);
   * will display as "The SS George (1382) diseappears in a bright flash as it gated
   * the Earth Gate (3828)".
   *
   * @param aSource The object doing the pose.
   * @param aPose The pose to do
   * @param aNominative The object the pose affects
   * @param aType The type of pose as in NotifyConsolesFormatted.
   */
  void NotifyObjectAffectPose(HSObject *aSource, std::string aPose, HSObject *aNominative, std::string aType);

  /**
   * Get the shield modifier for a certain location in space. This is the effect that
   * location of the universe has on the functioning of shields.
   *
   * @param aLocation Location for which to determine shield effect.
   * @return Shield modifier.
   */
  double GetShieldModifier(const HSVector3D &aLocation);

private:
  friend class HSDB;

  std::vector<HSObject*> mObjects;
  std::vector<HSTerritory*> mTerritories;
  
  /** Poses stored for execution after the cycle */
  std::vector<HSStoredPose> mPoses;
};
