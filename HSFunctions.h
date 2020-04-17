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

std::string HSFunctionSet(DBRef aCaller, 
                          std::string aType, 
                          std::string aObject, 
                          std::string aAttribute, 
                          std::string aValue);

std::string HSFunctionGet(DBRef aCaller,
                          std::string aType,
                          std::string aObject,
                          std::string aAttribute);

std::string HSFunctionShipSysSet(DBRef aCaller,
                                 std::string aShip,
                                 std::string aSystem,
                                 std::string aAttribute,
                                 std::string *aValue);

std::string HSFunctionShipSysGet(DBRef aCaller,
                                 std::string aSystem,
                                 std::string aShip,
                                 std::string aAttribute);

std::string HSFunctionClone(DBRef aCaller, std::string aShip);

std::string HSFunctionSystems(DBRef aCaller, std::string aShip, std::string aType);

std::string HSFunctionList(DBRef aCaller, std::string aType, std::string *aMore);

std::string HSFunctionPlaceSystem(DBRef aCaller, 
                                  std::string aShip, 
                                  std::string aSystem,
                                  bool aCheckSpace);

std::string HSFunctionRemoveSystem(DBRef aCaller,
                                   std::string aShip,
                                   std::string aSystem);

std::string HSFunctionTransferCargo(DBRef aCaller,
                                    std::string aSource,
                                    std::string aDestination,
                                    std::string aCommodity,
                                    std::string aAmount);

std::string HSFunctionSetCargo(DBRef aCaller,
                               std::string aContainer,
                               std::string aCommodity,
                               std::string aAmount);

std::string HSFunctionGetCargo(DBRef aCaller,
                               std::string aContainer,
                               std::string aCommodity);

std::string HSFunctionIsObject(DBRef aCaller,
                               std::string aObject,
                               std::string aType);

std::string HSFunctionNew(DBRef aCaller,
                          std::string aType,
                          std::string aArgument,
                          std::string aClass);

std::string HSFunctionSRep(DBRef aCaller,
                           std::string aShip,
                           std::string aType);
