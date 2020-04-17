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
#include "HSSystem.h"

class HSConsole;

/**
 * \ingroup HS_SYSTEMS
 * \brief This class contains the repair system.
 */
class HSRepairSystem :
  public HSSystem
{
  ATTRIBUTE(RepairCapacity, double)
public:
  HSRepairSystem();
  ~HSRepairSystem();
};

/**
 * \ingroup HS_SYSTEMS
 * \brief This class contains an instance of the dimensional drive system.
 */
class HSRepairSystemInstance :
  public HSSystemInstance
{
  INHERITED_SYSTEM(HSRepairSystem*)
  ATTRIBUTE_INHERIT_ADJUSTED(RepairCapacity, double)
  // Target ship for the repair
  ATTRIBUTE(TargetShipIDs, std::vector<int>)
  // The SystemIDs for the target system
  ATTRIBUTE(TargetSystemIDs, std::vector<int>)
  // The Target System count in the ship's system list.
  // together with the systemID this should uniquely identify
  // the system.
  ATTRIBUTE(TargetSystemCounts, std::vector<int>)
public:
  HSRepairSystemInstance(void);
  ~HSRepairSystemInstance(void);

  void AssignToSystem(HSConsole *aConsole, std::string aSystem);
  void AssignToSystemOn(HSConsole *aConsole, std::string aShip, std::string aSystem);
  void UnassignFromSystem(HSConsole *aConsole, std::string aSystem);
  void UnassignFromSystemOn(HSConsole *aConsole, std::string aShip, std::string aSystem);
  void GiveReport(HSConsole *aConsole);
  void Cycle();
};
