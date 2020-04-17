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
#pragma once
#include "HSSystem.h"

class HSConsole;
class HSObject;

/**
 * Structure for contacts on our sensors
 */
struct HSSensorContact
{
  /** This contact's HSObject */
  HSObject *Object;
  /** Detail we have on the contact */
  double Detail;
  /** ID of this contact */
  int ID;
  /** Was this contact still detected last cycle */
  bool Refreshed;
  /** Difference vector for this contact */
  HSVector3D DiffVector;
};

/**
 * \ingroup HS_SYSTEMS
 * \brief This class is for the computer systems
 */
class HSComputer :
  public HSSystem
{
  /** Amount of systems this computer can support */
  ATTRIBUTE(SystemsSupported, int);
public:
  /** Construct computer */
  HSComputer(void);
  ~HSComputer(void);
};

/**
 * \ingroup HS_SYSTEMINSTANCES
 * \brief This is the class for computer system instances.
 */
class HSComputerInstance :
  public HSSystemInstance
{
  INHERITED_SYSTEM(HSComputer*)
public:
  /** Construct computer system instance */
  HSComputerInstance(void);
  ~HSComputerInstance(void);

  void Cycle();

  /**
   * Checks whether the computer system is online.
   *
   * @return True if online, false if not
   */
  bool IsOnline();

  /**
   * Returns the list of sensor contacts for this computer.
   *
   * @return List of sensor contacts
   */
  const std::vector<HSSensorContact>& GetSensorContacts();

  /**
   * Finds a sensor contact by its ID
   *
   * @param aID Sensor ID of the contact
   * @param aContact Pointer to a HSSensorContact structure
   * @return True if the sensor contact was found
   */
  bool FindSensorContact(int aID, HSSensorContact *aContact);

  /**
   * Finds a sensor contact by its object ID
   *
   * @param aID Object ID of the contact
   * @param aContact Pointer to a HSSensorContact structure
   * @return True if the sensor contact was found
   */
  bool FindSensorContactByObjectID(int aID, HSSensorContact *aContact);

  /**
   * Instructs the ship computer to land the ship
   *
   * @param aConsole Console the order is executed from
   * @param aContact Contact ID to land on
   * @param aLocation Landing location to land
   */
  void Land(HSConsole *aConsole, int aContact, int aLocation);

  /**
   * Instructs the ship computer to launch the ship
   *
   * @param aConsole Console giving the instruction
   */
  void Launch(HSConsole *aConsole);

  /**
   * Display ship navigation report to a console.
   *
   * @param aConsole Console executing the nav report request.
   */
  void DisplayNavigationReport(HSConsole *aConsole);

  /**
   * Display ship computer status to a console.
   *
   * @param aConsole Console executing the status request.
   */
  void DisplayStatus(HSConsole *aConsole);

  /**
   * Display ship computer power status to a console.
   *
   * @param aConsole Console executing the status request.
   */
  void DisplayPowerStatus(HSConsole *aConsole);

  /**
   * Displays a sensor report for the ship to a console.
   *
   * @param aConsole Console executing the sensor report
   */
  void DisplaySensorReport(HSConsole *aConsole);

  /**
   * Displays a report for the ships docking hatches.
   *
   * @param aConsole Console executing the hatch report
   */
  void DisplayHatchReport(HSConsole *aConsole);

  /**
   * Connects a docking hatch to another docking hatch.
   *
   * @param aConsole Console executing the command
   * @param aHatch Hatch to connect
   * @param aTarget Target contact
   * @param aTargetHatch Target hatch to connect to
   */
  void ConnectHatch(HSConsole *aConsole, std::string aHatch, std::string aTarget, std::string aTargetHatch);

  /**
   * Disconnects a docking hatch.
   *
   * @param aConsole Console executing the command
   * @param aHatch Hatch to disconnect
   */
  void DisconnectHatch(HSConsole *aConsole, std::string aHatch);

  /**
   * Displays a cargo bay report for the ship to a console.
   *
   * @param aConsole Console executing the cargo bay report
   */
  void DisplayCargoReport(HSConsole *aConsole);

  /**
   * Displays the status report for a certain system.
   *
   * @param aConsole Console executing the status report
   * @param aSystem System the report is requested for
   */
  void DisplaySystemStatusReport(HSConsole *aConsole, std::string aSystem);

  /**
   * Displays the scan report for a certain object.
   *
   * @param aConsole Console executing the scan report
   * @param aContact Contact to scan
   */
  void DisplayScanReport(HSConsole *aConsole, std::string aContact);

  /**
   * Lock weapons of the ship on a target.
   *
   * @param aConsole Console executing the weapons lock
   * @param aWeapon Weapon to lock, 'all' for all weapons.
   * @param aContact Sensor contact to lock onto
   */
  void LockWeapons(HSConsole *aConsole, std::string aWeapon, std::string aContact);

  /**
   * Fire weapons of the ship on their locket targets.
   *
   * @param aConsole Console executing the weapons fire
   * @param aWeapon Weapons to fire
   */
  void FireWeapons(HSConsole *aConsole, std::string aWeapon);

  /**
   * Set an intercept course to a contact, if an autopilot
   * is available this will also set the contact's coordinates
   * as the autopilot destination.
   *
   * @param aConsole Console executing the command
   * @param aContact Contact to set intercept course for
   */
  void Intercept(HSConsole *aConsole, std::string aContact);

  /**
   * Gates a certain contact, this is currently only possible to
   * use with jumpgates.
   *
   * @param aConsole Console executing the command
   * @param aContact Contact to gate
   */
  void Gate(HSConsole *aConsole, std::string aContact);

  /**
   * Harvest a commodity using a certain harvester device.
   *
   * @param aConsole Console executing the command
   * @param aHarvester Harvester to use
   * @param aTarget Target contact to harvest from
   * @param aCommod Commodity to harvest from the resource
   */
  void Harvest(HSConsole *aConsole, std::string aHarvester, std::string aTarget, std::string aCommod);

  /**
   * Abort harvesting with a certain harvester device.
   *
   * @param aConsole Console executing the command
   * @param aHarvester Harvester to abort
   */
  void AbortHarvest(HSConsole *aConsole, std::string aHarvester);

  /**
   * Collect a cargo pod.
   *
   * @param aConsole Console executing the command.
   * @param aPod Cargo pod contact.
   */
  void Collect(HSConsole *aConsole, std::string aPod);

private:
  friend class HSSensorInstance;
  friend class HSUniverse;

  /** List of sensor contacts currently on sensors */
  std::vector<HSSensorContact> mSensorContacts;
};
