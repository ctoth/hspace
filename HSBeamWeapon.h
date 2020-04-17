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
#include "HSWeapon.h"

class HSConsole;

/**
 * \ingroup HS_SYSTEMS
 * \brief This class is for beam weapons
 */
class HSBeamWeapon :
  public HSSystem
{
  /** Total power of the weapon (maximum charge) */
  ATTRIBUTE(TotalPower, int)
  /** Speed at which the weapon charges */
  ATTRIBUTE(ChargeSpeed, double)
  /** Minimum charge needed to fire (relative <0.00,1.00>) */
  ATTRIBUTE(MinCharge, double)
  /** Maximum range of the weapon */
  ATTRIBUTE(Range, int)
  /** 
   * Accuracy of the weapon, this is a factor by which the normal
   * chance to hit is modified.
   */
  ATTRIBUTE(Accuracy, double)
  /**
   * Damage falloff of the weapon per km.
   */
  ATTRIBUTE(FallOff, double)
public:
  /**
   * Constructs a beam weapon.
   */
  HSBeamWeapon(void);
  ~HSBeamWeapon(void);
};

/**
 * \ingroup HS_SYSTEMINSTANCES
 * \brief This class is for beam weapon instances.
 */
class HSBeamWeaponInstance :
  public HSWeaponInstance
{
  INHERITED_SYSTEM(HSBeamWeapon*)
  ATTRIBUTE_INHERIT(TotalPower, int)
  ATTRIBUTE_INHERIT_ADJUSTED(ChargeSpeed, double)
  ATTRIBUTE_INHERIT(MinCharge, double)
  ATTRIBUTE_INHERIT(Range, int)
  ATTRIBUTE_INHERIT(Accuracy, double)
  ATTRIBUTE_INHERIT(FallOff, double)
  /** Current charge of the weapon */
  ATTRIBUTE(CurrentCharge, double)
  /** Mount angle in the XY plane in degrees */
  ATTRIBUTE(MountXY, int)
  /** Mount angle in the Z plane in degrees <-90,90> */
  ATTRIBUTE(MountZ, int)
  /** Firing arc of the weapon */
  ATTRIBUTE(Arc, int)

public:
  virtual void Cycle();

  /**
   * Fire the weapon.
   */
  HSFireResult Fire();

  /**
   * Constructs beam weapon instance
   */
  HSBeamWeaponInstance(void);
  ~HSBeamWeaponInstance(void);
};
