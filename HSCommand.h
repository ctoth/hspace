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
#ifndef __RSCOMMAND_H__
#define __RSCOMMAND_H__

#include "HSIface.h"

/**
 * Man a console
 *
 * @param aObject DBRef of the caller
 * @param aConsole Name for the console
 */
void HSCommandMan(DBRef aObject, std::string aConsole);

/**
 * Unman a console
 *
 * @param aObject DBRef of the caller
 */
void HSCommandUnman(DBRef aObject);

/**
 * Board a ship
 *
 * @param aObject DBRef of the caller
 * @param aShip Name of the ship
 */
void HSCommandBoard(DBRef aObject, std::string aShip);
/**
 * Disembark a ship
 *
 * @param aObject DBRef of the caller
 */
void HSCommandDisembark(DBRef aObject);

/**
 * General @Space command
 *
 * @param aObject DBRef of the caller
 * @param aSwitch Switch supplied to the @space command
 * @param aArgument Argument supplied to the @space command
 */
void HSCommandSpace(DBRef aObject, std::string aSwitch, std::string aArgument);
void HSCommandNewUniverse(DBRef aObject, std::string aArgument);
void HSCommandDelUniverse(DBRef aObject, std::string aArgument);
void HSCommandNewHullClass(DBRef aObject, std::string aArgument);
void HSCommandDelHullClass(DBRef aObject, std::string aArgument);
void HSCommandNewSystem(DBRef aObject, std::string aArgument);
void HSCommandDelSystem(DBRef aObject, std::string aArgument);
void HSCommandNewCommodity(DBRef aObject, std::string aArgument);
void HSCommandDelCommodity(DBRef aObject, std::string aArgument);
void HSCommandNewWarehouse(DBRef aObject, std::string aArgument);
void HSCommandDelWarehouse(DBRef aObject, std::string aArgument);
void HSCommandAddObject(DBRef aObject, std::string aArgument);
void HSCommandDelObject(DBRef aObject, std::string aArgument);
void HSCommandPlaceSystem(DBRef aObject, std::string aArgument);
void HSCommandRemoveSystem(DBRef aObject, std::string aArgument);
void HSCommandSetShipSystem(DBRef aObject, std::string aArgument);
void HSCommandAddConsole(DBRef aObject, std::string aArgument);
void HSCommandDelConsole(DBRef aObject, std::string aArgument);
void HSCommandAddLandingLoc(DBRef aObject, std::string aArgument);
void HSCommandRemoveLandingLoc(DBRef aObject, std::string aArgument);
void HSCommandAddDockingHatch(DBRef aObject, std::string aArgument);
void HSCommandRemoveDockingHatch(DBRef aObject, std::string aArgument);
void HSCommandAddTerritory(DBRef aObject, std::string aArgument);
void HSCommandDelTerritory(DBRef aObject, std::string aArgument);
void HSCommandList(DBRef aObject, std::string aArgument);
void HSCommandDumpObject(DBRef aObject, std::string aType, std::string aArgument);
void HSCommandSetObject(DBRef aObject, std::string aType, std::string aArgument);
void HSCommandAddSRoom(DBRef aObject, std::string aArgument);
void HSCommandDelSRoom(DBRef aObject, std::string aArgument);
void HSCommandClone(DBRef aObject, std::string aArgument);

#endif /* __RSCOMMAND_H__ */
