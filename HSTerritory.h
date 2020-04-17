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
#include "HSAttributed.h"
#include "HSVector3D.h"

class HSShip;
/**
 * \ingroup HS_CORE
 * \brief This is the class to represent territories.
 */
class HSTerritory :
  public HSAttributed
{
  /** ID of the territory, also dbref of the corresponding object */
  ATTRIBUTE(ID, int)
  /** Name of the territory */
  ATTRIBUTE(Name, std::string)
  /** Center of the territory */
  ATTRIBUTE(Center, HSVector3D)
  /** Universe ID the territory is in */
  ATTRIBUTE(Universe, int)
  /** Radius of the territory */
  ATTRIBUTE(Radius, double)

public:
  HSTerritory(void);
  ~HSTerritory(void);

  /**
   * Notifies the territory a ship has just moved. The territory
   * checks if it has entered or left the territory and displays
   * an optional message.
   *
   * @param aShip Ship that has just moved.
   * @param aOldPosition Old position of the ship.
   */
  void ShipMoves(HSShip *aShip, const HSVector3D& aOldPosition);

protected:
  virtual void AttributeChanged(std::string aName);
};
