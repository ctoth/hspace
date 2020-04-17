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
#include "HSIface.h"
#include <string>

class HSShip;
class HSCommunicationsInstance;
class HSComputerInstance;

#define HS_ENG_MSG 0x01
#define HS_NAV_MSG 0x02
#define HS_COM_MSG 0x04
#define HS_TAC_MSG 0x08
#define HS_ALL_MSGS 0xff

/**
 * \ingroup HS_CORE
 * \brief This class represents a ship console
 */
class HSConsole :
  public HSAttributed
{
  /** ID of this console, corresponds to the MUSH object */
  ATTRIBUTE(ID, int)
  /** ID of the object this console belongs to */
  ATTRIBUTE(ShipID, int)
  /// Added for the future, not using right now, or storing.
  ATTRIBUTE(MessageMask, int)
  /// Private for optimization.
  ATTRIBUTE(Ship, HSShip*)
public:
  HSConsole(void);
  ~HSConsole(void);

  /**
   * Execute a command on this console.
   *
   * @param aCommand Command to execute
   * @param aArgs Array of arguments given to the command
   * @param aCount Number of elements in aArgs
   */
  void ExecuteCommand(std::string aCommand, std::string aArgs[], int aCount);

  /**
   * Send a message to the user of this console
   *
   * @param aMsg Message to send
   */
  void NotifyUser(std::string aMsg);

  /**
   * Sends a message in the simple command message mode.
   *
   * @param aMsg Message to send
   */
  void NotifyUserSimple(std::string aMsg);

private:
  void GiveStatus(std::string aCommand);
  void SetOutput(std::string aSystemName, std::string aOutput);
  void Allocate(std::string aSystemName, std::string aPower);
  void GiveSensorReport();
  void SetSpeed(double aSpeed);
  void SetHeading(int aHeadingXY, int aHeadingZ);
  void CutEngines();
  void Land(int aContact, int aLocation);
  void Launch();
  void Tune(double aFrequency, std::string aEncryptKey);
  void SendMessage(int aContact, std::string aMessage);
  void SetFrequency(double aFrequency);
  void Broadcast(std::string aMessage);
  void SysStat(std::string aSystem);
  void LockWeapons(std::string aWeapon, std::string aTarget);
  void FireWeapons(std::string aWeapon);
  void Scan(std::string aContact);
  void Intercept(std::string aContact);
  void SetDestination(double aX, double aY, double aZ);
  void Autopilot(bool aEngage);
  void Taxi(std::string aExit);
  void Engage();
  void Disengage();
  void Repair(std::string aSystem, std::string aRepairSystem);

  /**
   * Get the ETA to your destination at current speed.
   */
  void ETA();

  /**
   * Jettison the contents of a cargo bay into space.
   *
   * @param aBay The bay that is to be ejected.
   */
  void Jettison(std::string aBay);

  /**
   * Boot the ships computer
   */
  void BootComputer();

  HSComputerInstance *VerifyComputer();

  HSCommunicationsInstance* FindCommSystem();

  DBRef GetUser();
};
