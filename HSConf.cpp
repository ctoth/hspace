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
#include "HSConf.h"

HSConf sHSConf;

HSConf::HSConf(void)
  : AfterlifeRoom(0)
  , DestructionString("The last thing you see is a bright flash as the ship you're on is destroyed.")
  , WeaponsHitString("The ship rocks violently as it is hit by weapons fire.")
  , GateDistance(2)
  , HarvestDistance(2)
  , CollectDistance(2)
  , GatesIn(" appears in a bright flash as it gates into the area")
  , RevertThrust(0.0)
  , DDChargeNotifySteps(5)
  , DDEngaged("You feel the ship shake lightly as it shifts into an alternate dimension.")
  , DDDisengaged("You feel the ship shake lightly as it drops out of an alternate dimension.")
  , DDMultiplier(20)
  , DockingDistance(1)
{
  ADD_ATTRIBUTE("AFTERLIFEROOM", AT_INTEGER, AfterlifeRoom)
  ADD_ATTRIBUTE("DESTRUCTIONSTRING", AT_STRING, DestructionString)
  ADD_ATTRIBUTE("WEAPONSHITSTRING", AT_STRING, WeaponsHitString)
  ADD_ATTRIBUTE("GATEDISTANCE", AT_INTEGER, GateDistance)
  ADD_ATTRIBUTE("HARVESTDISTANCE", AT_INTEGER, HarvestDistance)
  ADD_ATTRIBUTE("COLLECTDISTANCE", AT_INTEGER, CollectDistance)
  ADD_ATTRIBUTE("GATESIN", AT_STRING, GatesIn)
  ADD_ATTRIBUTE("REVERTTHRUST", AT_DOUBLE, RevertThrust)
  ADD_ATTRIBUTE("DDCHARGENOTIFYSTEPS", AT_INTEGER, DDChargeNotifySteps)
  ADD_ATTRIBUTE("DDENGAGED", AT_STRING, DDEngaged)
  ADD_ATTRIBUTE("DDDISENGAGED", AT_STRING, DDDisengaged)
  ADD_ATTRIBUTE("DDMULTIPLIER", AT_INTEGER, DDMultiplier)
  ADD_ATTRIBUTE("DOCKINGDISTANCE", AT_INTEGER, DockingDistance)
}

HSConf::~HSConf(void)
{
}
