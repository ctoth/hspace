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

#include "HSObject.h"

/**
 * \ingroup HS_OBJECTS
 * \brief This is the class for the planet object.
 */
class HSNebula :
  public HSObject
{
  /** Radius of the nebula */
  ATTRIBUTE(Radius, int)
  /** 
   * Sensor effect the nebula <0.00,1.00> where 1.00 means sensors
   * are non-functional.
   */
  ATTRIBUTE(SensorEffect, double)
  /**
   * Shield effect of the nebula <0.00,1.00> where 1.00 means shields
   * are completely non-functional.
   */
  ATTRIBUTE(ShieldEffect, double)
  /**
   * Falloff of the nebula towards the edge <0.00,1.00> where 0.00
   * means linear falloff to no effect at the edge.
   */
  ATTRIBUTE(Falloff, double)
public:
  HSNebula(void);
  ~HSNebula(void);
};
