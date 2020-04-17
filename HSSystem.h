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
#pragma once
#include "HSAttributed.h"
#include <string>

/**
 * Enumeration of available system types
 */
enum HSSystemType {
  HSST_GENERIC = 0,
  HSST_REACTOR,
  HSST_COMPUTER,
  HSST_SENSOR,
  HSST_ENGINE,
  HSST_THRUSTERS,
  HSST_COMMUNICATIONS,
  HSST_SHIELD,
  HSST_BEAMWEAPON,
  HSST_FUELTANK,
  HSST_AUTOPILOT,
  HSST_CARGOBAY,
  HSST_HARVESTER,
  HSST_DIMENSIONALDRIVE,
  HSST_REPAIRSYSTEM,
  HSST_END
};

class HSShip;

#define ATTRIBUTE_INHERIT(name, type) \
  public: \
  void Set##name (type *aValue) { if (!aValue) { m##name##IsSet = false; return; } m##name##IsSet = true; m##name = *aValue; } \
  type Get##name () { if (!m##name##IsSet) { return mSystem->Get##name(); } return m##name; } \
  protected: \
  bool m##name##IsSet; \
  type m##name;

#define ATTRIBUTE_INHERIT_ADJUSTED(name, type) \
  public: \
  void Set##name (type *aValue) { if (!aValue) { m##name##IsSet = false; return; } m##name##IsSet = true; m##name = *aValue; } \
  type Get##name (bool aAdjusted) { \
  if (!aAdjusted || !GetOptimalPower()) { if (!m##name##IsSet) { return mSystem->Get##name(); } return m##name; \
  } else { if (!m##name##IsSet) { return (type)(((double)mSystem->Get##name()) * \
  ((double)mCurrentPower / (double)GetOptimalPower()) * mCondition); } \
  return (type)(((double)m##name) * ((double)mCurrentPower / (double)GetOptimalPower())); } } \
  private: \
  bool m##name##IsSet; \
  type m##name;

#define ADD_ATTRIBUTE_INHERIT(name, type, var) \
  var##IsSet = false; \
{ \
  Attribute attr = { name, type, false, &var, &var##IsSet }; \
  mAttributeList.push_back(attr); \
}

#define INHERITED_SYSTEM(type) \
  public: \
  void SetSystem(HSSystem *aValue) { mSystem = (type)aValue; HSSystemInstance::SetSystem(aValue); } \
  type GetSystem() { return mSystem; } \
  protected: \
  type mSystem;

const int sSystemTypeCount = HSST_END;

/**
 * \ingroup HS_SYSTEMS
 * \brief This is the base class for all systems
 */
class HSSystem :
  public HSAttributed
{
  /** ID of the system, unique integer identifier */
  ATTRIBUTE(ID, int)
  /** Type ID of the system (as in HSSystemType) */
  ATTRIBUTE(Type, int)
  /** Name of the system */
  ATTRIBUTE(Name, std::string)
  /** Optimal power, at which no stress occurs and system functions at best */
  ATTRIBUTE(OptimalPower, int)
  /** Size of the system, number of space units it takes on the ship */
  ATTRIBUTE(Size, int)
  /** Mass of the system */
  ATTRIBUTE(Mass, int)
  /** 
   * Overload tolerance of the system. The chance damage occurs to the system
   * is equal to Overload (i.e. 1.1 for 110%) in percentages times 1 divided
   * by this number. i.e. 2.0 makes the system half as likely to be damaged.
   */
  ATTRIBUTE(Tolerance, double)
  /** 
   * Maximum the system can be overloaded 
   * (i.e. 1.0 means 200% optimal power, 0.5 150%, etc.) 
   */
  ATTRIBUTE(MaxOverload, double)
public:
  /** Construct a system */
  HSSystem(void);
  virtual ~HSSystem(void);

  /**
   * Get a vector of all available HSSystemTypes.
   *
   * @return All available types.
   */
  static std::vector<HSSystemType> GetTypes();

  /**
   * Get the table name used for storage of a specific type
   * of system.
   *
   * @param aType Type to lookup the table for.
   * @return Name of the storage suffix
   */
  static std::string GetTableForType(HSSystemType aType);

  /**
   * Get the descriptive name of a type of system.
   *
   * @param aType Type to lookup the name for.
   * @return Type name
   */
  static std::string GetNameForType(HSSystemType aType);

  /**
   * Create an instance of a child class of HSSystem for a
   * specific type.
   *
   * @param aType Type of child class to create.
   * @return Instantiated object of the right type
   */
  static HSSystem* CreateSystemForType(HSSystemType aType);

  /**
   * Return a string for the passed mass
   *
   * @param aMass Mass in kg
   * @return String for mass
   */
  static std::string MassString(int aMass);

  /**
   * Check if more systems are allowed on this ship
   *
   * @return True if more systems are allowed
   */
  bool MoreAllowed();

  /**
   * Check if this system consumes a mountpoint
   *
   * @return True if it consumes a mountpoint
   */
  bool TakesMount();
protected:
  /** True if a ship is allowed to have more than one of this */
  bool mMoreAllowed;
  /** True if this system consumes a mount point */
  bool mTakesMount;
};

/**
 * \ingroup HS_SYSTEMINSTANCES
 * \brief Parent class for all system instances, that is the representation
 * of a HSSystem on individual ships.
 */
class HSSystemInstance :
  public HSAttributed
{
  /** ID of the ship this instance belongs to */
  ATTRIBUTE(ShipID, int)
  /** Type of the system */
  ATTRIBUTE(Type, int)
  /** ID of the HSSystem this is a child of */
  ATTRIBUTE(SystemID, int)
  /** Current power allocated to the system */
  ATTRIBUTE(CurrentPower, int)
  /** Condition the system is in <0.0, 1.0> */
  ATTRIBUTE(Condition, double)
  ATTRIBUTE_INHERIT(Name, std::string)
  ATTRIBUTE_INHERIT(OptimalPower, int)
  ATTRIBUTE_INHERIT(MaxOverload, double)
  /** Size of the system, number of space units it takes on the ship */
  ATTRIBUTE_INHERIT(Size, int)
  /** Mass of the system */
  ATTRIBUTE_INHERIT(Mass, int)
  /** Overload tolerance */
  ATTRIBUTE_INHERIT(Tolerance, double)
  /** Just stored for optimization, not settable. */
  ATTRIBUTE(Ship, HSShip*)
public:
  HSSystemInstance(void);
  virtual ~HSSystemInstance(void);

  /**
   * Overloadable function, this is called every cycle and
   * can be reimplemented on child system instances.
   */
  virtual void Cycle(void);

  /**
   * Returns a new HS<system>Instance for a HSSystemType
   *
   * @param aType Type of system
   * @return Instance of the corresponding HS<system>Instance
   */
  static HSSystemInstance* CreateSystemInstanceForType(HSSystemType aType);

  /**
   * Set a pointer to our parent HSSystem
   *
   * @param aValue System
   */
  virtual inline void SetSystem(HSSystem *aValue) { mSystem = aValue; }

  /**
   * Get the status of the system, can be reimplemented for specific systems
   * to give additional or different status information.
   */
  virtual std::string GetSystemStatus();

  /**
   * Get the pointer to the HSSystem that we are an instance of
   *
   * @return System
   */
  inline HSSystem *GetSystem() { return mSystem; }
protected:
  /** Pointer to HSSystem that is we are an instance of */
  HSSystem *mSystem;
};
