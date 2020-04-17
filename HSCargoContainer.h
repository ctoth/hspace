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
#include "HSCommodity.h"

/**
 * \ingroup HS_CORE
 * \brief This is a general class that can store cargo. It can be inherited
 * by child classes to implement a cargo container.
 */
class HSCargoContainer
{
  /** Cargo contained in this cargo bay */
  ATTRIBUTE(Cargo, std::vector<HSCargoItem>)
public:
  /** Construct cargo container */
  HSCargoContainer(void);
  virtual ~HSCargoContainer(void);

  /**
   * Get the total amount of cargo in the container.
   *
   * @return Total amount of cargo
   */
  int GetTotalCargo();
  
  /**
   * Get te total amount of cargo in the container for
   * a specific commodity.
   *
   * @param aCommodity Commodity to get the cargo for
   * @return Amount of cargo for that commodity
   */
  double GetCargoForCommodity(int aCommodity);

  /**
   * Set the total amount of a commodity in the container.
   *
   * @param aCommodity Commodity to set the amount for
   * @param aAmount Amount set for commodity
   */
  void SetCargoForCommodity(int aCommodity, double aAmount);

  /**
   * Get the amount of space available in the container.
   *
   * @return Amount of space available.
   */
  virtual int GetFreeSpace();

  /**
   * Get the total mass of all cargo in the container.
   *
   * @return Mass of total cargo.
   */
  double GetCargoMass();
};
