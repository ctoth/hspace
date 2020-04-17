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

class HSConf :
  public HSAttributed
{
public:
  HSConf(void);
  ~HSConf(void);

  int AfterlifeRoom;
  std::string DestructionString;
  std::string WeaponsHitString;
  /** Distance from which it's possible to gate a jump gate */
  int GateDistance;
  /** Distance from which it's possible to harvest a resource */
  int HarvestDistance;
  /** Distance from which it's possible to collect a cargo pod */
  int CollectDistance;
  /** String emitted to the universe when a ship comes jumping in */
  std::string GatesIn;
  /** How much of the forward velocity can be applied backwards */
  double RevertThrust;
  /** Steps in which dimensional drive charging is notified */
  int DDChargeNotifySteps;
  /** Message displayed to ship rooms when engaging dimensional drive */
  std::string DDEngaged;
  /** Message displayed to ship rooms when dimensional drive disengages */
  std::string DDDisengaged;
  /** Speed up given my dimensional drive */
  int DDMultiplier;
  /** Distance from which a docking hatch can be connected */
  int DockingDistance;

};

extern HSConf sHSConf;
