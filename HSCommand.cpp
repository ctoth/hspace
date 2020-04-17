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
// Local
#include "HSCommand.h"
#include "HSVersion.h"
#include "HSIface.h"
#include "HSDB.h"
#include "HSUniverse.h"
#include "HSHullClass.h"
#include "HSTools.h"
#include "HSObject.h"
#include "HSShip.h"
#include "HSSystem.h"
#include "HSConsole.h"
#include "HSLandingLocation.h"
#include "HSDockingHatch.h"
#include "HSConf.h"
#include "HSCommodity.h"
#include "HSWarehouse.h"
#include "HSTerritory.h"
#include "sqlite3.h"

#ifdef HS_SERVER
#include "HSServer.h"
#endif

// Global
#include <string.h>

// STL
#include <map>

using namespace std;

void
HSCommandMan(DBRef aObject, std::string aConsole)
{
  DBRef consoleObj = HSIFLocateNeighbor(aObject, aConsole);

  if (consoleObj == DBRefNothing) {
    HSIFNotify(aObject, "No such console found.");
    return;
  }

  HSConsole *consoleManned = sHSDB.FindConsoleMannedBy(aObject);

  if (consoleManned) {
    HSIFNotify(aObject, "You are already manning a console.");
    return;
  }

  HSConsole *console = sHSDB.FindConsole(consoleObj);

  if (!console) {
    HSIFNotify(aObject, "No such console found.");
    return;
  }

  if (HSIFGetObjectLock(console->GetID(), HSLock_Use) != console->GetID()) {
    HSIFNotify(aObject, "That console is already manned.");
    return;
  }

  char tbuf[256];
  sprintf(tbuf, "%s mans %s.", HSIFGetName(aObject).c_str(),
    HSIFGetName(console->GetID()).c_str());
  HSIFNotifyContentsExcept(HSIFGetObjectLocation(console->GetID()),
    tbuf, aObject);
  sprintf(tbuf, "You man %s.", HSIFGetName(console->GetID()).c_str());
  HSIFNotify(aObject, tbuf);
  HSIFSetObjectLock(console->GetID(), HSLock_Use, aObject);
#ifdef HS_SERVER
  sServer->ConsoleUpdate(aObject);
#endif
}

void
HSCommandUnman(DBRef aObject)
{
  HSConsole *console = sHSDB.FindConsoleMannedBy(aObject);

  if (!console) {
    HSIFNotify(aObject, "You are not manning a console.");
    return;
  }
  char tbuf[256];
  sprintf(tbuf, "%s unmans %s.", HSIFGetName(aObject).c_str(),
    HSIFGetName(console->GetID()).c_str());
  HSIFNotifyContentsExcept(HSIFGetObjectLocation(console->GetID()),
    tbuf, aObject);
  sprintf(tbuf, "You unman %s.", HSIFGetName(console->GetID()).c_str());
  HSIFNotify(aObject, tbuf);
  HSIFSetObjectLock(console->GetID(), HSLock_Use, console->GetID());
#ifdef HS_SERVER
  sServer->ConsoleUpdate(aObject);
#endif
}

void
HSCommandBoard(DBRef aObject, std::string aShip)
{
  DBRef shipObj = HSIFLocateNeighbor(aObject, aShip);

  if (shipObj == DBRefNothing) {
    HSIFNotify(aObject, "No such vessel found.");
    return;
  }

  HSObject *object = sHSDB.FindObject(shipObj);

  if (!object || object->GetType() != HSOT_SHIP) {
    HSIFNotify(aObject, "No such vessel found.");
    return;
  }

  HSShip *ship = static_cast<HSShip*>(object);

  if (ship->GetHatchLoc() == -1) {
    HSIFNotify(aObject, "That vessel cannot be boarded.");
    return;
  }

  if (!HSIFPassesLock(aObject, ship->GetID(), HSLock_Board)) {
    HSIFNotify(aObject, "You are not allowed to board that vessel.");
    return;
  }

  char tbuf[256];
  sprintf(tbuf, "%s boards the %s.", HSIFGetName(aObject).c_str(),
    ship->GetName().c_str());
  HSIFNotifyContentsExcept(HSIFGetObjectLocation(ship->GetID()),
    tbuf, aObject);
  sprintf(tbuf, "You board the %s.", ship->GetName().c_str());
  HSIFNotify(aObject, tbuf);
  sprintf(tbuf, "%s boards the vessel from outside.", HSIFGetName(aObject).c_str());
  HSIFNotifyContentsExcept(ship->GetHatchLoc(), tbuf);
  HSIFTeleportObject(aObject, ship->GetHatchLoc());
}

void
HSCommandDisembark(DBRef aObject)
{
  HSShip *ship = NULL;
  int loc = HSIFGetObjectLocation(aObject);
  foreach(HSUniverse*, universe, sHSDB.GetUniverses()) {
    foreach(HSObject*, object, universe->GetObjects()) {
      if (object->GetType() == HSOT_SHIP) {
        HSShip *tmpShip = static_cast<HSShip*>(object);
        if (loc == tmpShip->GetHatchLoc()) {
          ship = tmpShip;
          break;
        }
      }
    }
    if (ship) {
      break;
    }
  }
  if (!ship) {
    HSIFNotify(aObject, "You cannot disembark from here.");
    return;
  }

  if (ship->GetLandedLoc() == -1 || ship->GetLandingTimer() != 0) {
    HSIFNotify(aObject, "You cannot disembark while the vessel is in flight.");
    return;
  }

  char tbuf[256];
  HSIFNotify(aObject, "You disembark the vessel.");
  sprintf(tbuf, "%s disembarks the vessel.", HSIFGetName(aObject).c_str());
  HSIFNotifyContentsExcept(ship->GetHatchLoc(), tbuf, aObject);
  sprintf(tbuf, "%s disembarks the %s.", HSIFGetName(aObject).c_str(),
    ship->GetName().c_str());
  HSIFNotifyContentsExcept(ship->GetLandedLoc(), tbuf);
  HSIFTeleportObject(aObject, ship->GetLandedLoc());
}

void 
HSCommandSpace(DBRef aObject, string aSwitch, string aArgument)
{
  stringstream reply;

  if (aSwitch.empty() || !HSIFIsWizard(aObject)) {
    reply << "HSpace Version " << GetVersionString() << "\n";
    reply << "Last cycle took: " << sHSDB.GetLastCycleTime() << " seconds\n";
    reply << "HSpace Objects using: " << HSAttributed::sMemoryUsage / 1000 << " kbytes\n";
    std::vector<HSUniverse*> universes = sHSDB.GetUniverses();
    reply << universes.size() << " universes\n";
    size_t objects = 0;
    foreach(HSUniverse*, universe, universes) {
      objects += universe->GetObjects().size();
    }
    reply << objects << " objects\n";
    reply << sHSDB.GetSystems().size() << " systems\n";
    reply << sHSDB.GetHullClasses().size() << " hullclasses\n";
    reply << sHSDB.GetCommodities().size() << " commodities\n";
    reply << sHSDB.GetWarehouses().size() << " warehouses\n";
    HSIFNotify(aObject, reply.str());
  } else if (aSwitch == "NEWUNIVERSE") {
    HSCommandNewUniverse(aObject, aArgument);
  } else if (aSwitch == "DELUNIVERSE") {
    HSCommandDelUniverse(aObject, aArgument);
  } else if (aSwitch == "NEWHULLCLASS") {
    HSCommandNewHullClass(aObject, aArgument);
  } else if (aSwitch == "DELHULLCLASS") {
    HSCommandDelHullClass(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "NEWCOMMODITY")) {
    HSCommandNewCommodity(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "DELCOMMODITY")) {
    HSCommandDelCommodity(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "NEWWAREHOUSE")) {
    HSCommandNewWarehouse(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "DELWAREHOUSE")) {
    HSCommandDelWarehouse(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "NEWSYSTEM")) {
    HSCommandNewSystem(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "DELSYSTEM")) {
    HSCommandDelSystem(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "ADDOBJECT")) {
    HSCommandAddObject(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "DELOBJECT")) {
    HSCommandDelObject(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "PLACESYSTEM")) {
    HSCommandPlaceSystem(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "REMOVESYSTEM")) {
    HSCommandRemoveSystem(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "SHIPSYSSET")) {
    HSCommandSetShipSystem(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "ADDCONSOLE")) {
    HSCommandAddConsole(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "DELCONSOLE")) {
    HSCommandDelConsole(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "ADDLANDINGLOC")) {
    HSCommandAddLandingLoc(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "REMOVELANDINGLOC")) {
    HSCommandRemoveLandingLoc(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "ADDDOCKINGHATCH")) {
    HSCommandAddDockingHatch(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "REMOVEDOCKINGHATCH")) {
    HSCommandRemoveDockingHatch(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "ADDTERRITORY")) {
    HSCommandAddTerritory(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "DELTERRITORY")) {
    HSCommandDelTerritory(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "ADDSROOM")) {
    HSCommandAddSRoom(aObject, aArgument);
  } else if (!strncasecmp(aSwitch.c_str(), "DELSROOM")) {
    HSCommandDelSRoom(aObject, aArgument);
  } else if (aSwitch.substr(0, 4) == "DUMP") {
    HSCommandDumpObject(aObject, aSwitch.substr(4), aArgument);
  } else if (aSwitch.substr(0, 3) == "SET") {
    HSCommandSetObject(aObject, aSwitch.substr(3), aArgument);
  } else if (aSwitch == "LIST") {
    HSCommandList(aObject, aArgument);
  } else if (aSwitch == "CLONE") {
    HSCommandClone(aObject, aArgument);
  } else {
    HSIFNotify(aObject, "Unknown switch.");
  }
}

void
HSCommandNewUniverse(DBRef aObject, string aArgument)
{
  string room = aArgument.substr(0,aArgument.find("="));
  string name = aArgument.substr(aArgument.find("=") + 1);

  DBRef roomObj = HSIFLocateObject(aObject, room, OT_ROOM);

  if (roomObj == -1) {
    HSIFNotify(aObject, "Invalid universe room specified.");
    return;
  }

  if (sHSDB.FindUniverse(roomObj)) {
    HSIFNotify(aObject, "That room is already a universe.");
    return;
  }

  HSUniverse *newUniverse = new HSUniverse();
  newUniverse->SetID(roomObj);
  newUniverse->SetName(name);

  sHSDB.AddUniverse(newUniverse);
  HSIFNotify(aObject, "Universe succesfully created! Behold!");
}

void
HSCommandDelUniverse(DBRef aObject, string aArgument)
{
  DBRef roomObj = HSIFLocateObject(aObject, aArgument, OT_ROOM);

  if (roomObj == -1) {
    HSIFNotify(aObject, "Invalid universe specified.");
    return;
  }

  HSUniverse *uni;
  if (!(uni = sHSDB.FindUniverse(roomObj))) {
    HSIFNotify(aObject, "That is not a valid universe.");
    return;
  }

  sHSDB.DelUniverse(uni);
  HSIFNotify(aObject, "Universe succesfully destroyed!");
}

void
HSCommandNewHullClass(DBRef aObject, string aArgument)
{
  HSHullClass *hullClass = new HSHullClass();

  if (!aArgument.empty()) {
    hullClass->SetName(aArgument);
  } else {
    hullClass->SetName("Unnamed Hullclass");
  }
  int id = sHSDB.AddHullClass(hullClass);

  if (id < 0) {
    HSIFNotify(aObject, "Failed to create class.");
  } else {
    stringstream notification;
    notification << "Succesfully created new class (" << id << ")";
    HSIFNotify(aObject, notification.str());
  }
}

void
HSCommandDelHullClass(DBRef aObject, string aArgument)
{
  HSHullClass *hullClass;
  if (!(hullClass = sHSDB.FindHullClass(atoi(aArgument.c_str())))) {
    HSIFNotify(aObject, "That is not a valid hull class.");
    return;
  }

  foreach(HSUniverse*, universe, sHSDB.GetUniverses()) {
    foreach(HSObject*, object, universe->GetObjects()) {
      if (object->GetType() == HSOT_SHIP) {
        HSShip *ship = static_cast<HSShip*>(object);
        if (ship->GetHullClass() == atoi(aArgument.c_str())) {
          HSIFNotify(aObject, "That hullclass is still in use, it can not be deleted.");
          return;
        }
      }
    }
  }
  sHSDB.DelHullClass(hullClass);
  HSIFNotify(aObject, "Hullclass succesfully deleted.");
}

void
HSCommandNewSystem(DBRef aObject, string aArgument)
{
  HSSystem *system = HSSystem::CreateSystemForType((HSSystemType)atoi(aArgument.c_str()));
  if (!system) {
    HSIFNotify(aObject, "No such system type.");
    return;
  }

  system->SetName(string("Unnamed ")
    .append(HSSystem::GetNameForType((HSSystemType)system->GetType())));
  int id = sHSDB.AddSystem(system);

  if (id < 0) {
    HSIFNotify(aObject, "Failed to create system.");
  } else {
    stringstream notification;
    notification << "Succesfully created new " 
      << HSSystem::GetNameForType((HSSystemType)system->GetType()) 
      << " system (" << id << ")";
    HSIFNotify(aObject, notification.str());
  }
}

void
HSCommandDelSystem(DBRef aObject, string aArgument)
{
  HSSystem *system;
  if (!(system = sHSDB.FindSystem(atoi(aArgument.c_str())))) {
    HSIFNotify(aObject, "That is not a valid system.");
    return;
  }
  sHSDB.DelSystem(system);
  HSIFNotify(aObject, "System succesfully deleted.");
}

void
HSCommandNewCommodity(DBRef aObject, string aArgument)
{
  HSCommodity *commodity = new HSCommodity();

  if (!aArgument.empty()) {
    commodity->SetName(aArgument);
  } else {
    commodity->SetName("Unnamed Commodity");
  }
  int id = sHSDB.AddCommodity(commodity);

  if (id < 0) {
    HSIFNotify(aObject, "Failed to create commodity.");
  } else {
    stringstream notification;
    notification << "Succesfully created new commodity (" << id << ")";
    HSIFNotify(aObject, notification.str());
  }
}

void
HSCommandDelCommodity(DBRef aObject, string aArgument)
{
  HSCommodity *commodity;
  if (!(commodity = sHSDB.FindCommodity(atoi(aArgument.c_str())))) {
    HSIFNotify(aObject, "That is not a valid commodity.");
    return;
  }

  sHSDB.DelCommodity(commodity);
  HSIFNotify(aObject, "Commodity succesfully deleted.");
}

void
HSCommandNewWarehouse(DBRef aObject, string aArgument)
{
  HSWarehouse *warehouse = new HSWarehouse();

  if (!aArgument.empty()) {
    warehouse->SetName(aArgument);
  } else {
    warehouse->SetName("Unnamed Warehouse");
  }
  int id = sHSDB.AddWarehouse(warehouse);

  if (id < 0) {
    HSIFNotify(aObject, "Failed to create warehouse.");
  } else {
    stringstream notification;
    notification << "Succesfully created new warehouse (" << id << ")";
    HSIFNotify(aObject, notification.str());
  }
}

void
HSCommandDelWarehouse(DBRef aObject, string aArgument)
{
  HSWarehouse *warehouse;
  if (!(warehouse = sHSDB.FindWarehouse(atoi(aArgument.c_str())))) {
    HSIFNotify(aObject, "That is not a valid warehouse.");
    return;
  }

  sHSDB.DelWarehouse(warehouse);
  HSIFNotify(aObject, "Warehouse succesfully deleted.");
}

void
HSCommandAddObject(DBRef aObject, string aArgument)
{
  string thing = aArgument.substr(0,aArgument.find("="));
  string typearg = aArgument.substr(aArgument.find("=") + 1);
  string type = typearg.substr(0,aArgument.find("/"));

  DBRef obj = HSIFLocateObject(aObject, thing, OT_THING);

  if (obj == -1) {
    HSIFNotify(aObject, "Invalid object specified.");
    return;
  }

  if (sHSDB.FindObject(obj)) {
    HSIFNotify(aObject, "That object is already a space object.");
    return;
  }

  if (!sHSDB.GetUniverses().size()) {
    HSIFNotify(aObject, "No valid universes available to place that in.");
    return;
  }

  HSHullClass *hullClass = NULL;
  if (atoi(type.c_str()) == HSOT_SHIP) {
    size_t seperatorPosition = typearg.find("/");
    if (seperatorPosition != (size_t)-1) {
      hullClass = sHSDB.FindHullClass(atoi(typearg.substr(seperatorPosition + 1).c_str()));
    }
    if (!hullClass) {
      HSIFNotify(aObject, "A valid hullclass needs to be specified for ships");
      return;
    }
  }

  HSObject *newObject;
  newObject = HSObject::CreateObjectForType((HSObjectType)atoi(type.c_str()));
  if (!newObject) {
      HSIFNotify(aObject, "Invalid object type specified");
      return;
  }

  if (hullClass) {
    newObject->SetAttribute("HULLCLASS", hullClass->GetID(), true);
    newObject->SetAttribute("CURRENTHULLPOINTS", hullClass->GetHullPoints());
    newObject->SetAttribute("SIZE", hullClass->GetSize());
  }

  newObject->SetID(obj);
  newObject->SetName(string("New ").append(
    HSObject::GetNameForType((HSObjectType)newObject->GetType())));

  // Grab first universe available and dump the object there
  HSUniverse *universe = *sHSDB.GetUniverses().begin();
  newObject->SetUniverse(universe->GetID());
  universe->AddObject(newObject);

  HSIFNotify(aObject, "Object succesfully created!");
}


void
HSCommandDelObject(DBRef aObject, string aArgument)
{
  string thing = aArgument;

  DBRef obj = HSIFLocateObject(aObject, thing, OT_THING);

  if (obj == -1) {
    HSIFNotify(aObject, "Invalid object specified.");
    return;
  }

  HSObject *object = sHSDB.FindObject(obj);
  if (!object) {
    HSIFNotify(aObject, "That is not a valid space object.");
    return;
  }

  // Grab first universe available and dump the object there
  HSUniverse *universe = sHSDB.FindUniverse(object->GetUniverse());
  ASSERT(universe);
  while(universe->RemoveObject(object)) {}
  delete object;
  HSIFNotify(aObject, "Object succesfully destroyed!");
}

void
HSCommandPlaceSystem(DBRef aObject, string aArgument)
{
  string strShip = aArgument.substr(0, aArgument.find("="));
  string strSystem = aArgument.substr(aArgument.find("=") + 1);

  DBRef obj = HSIFLocateObject(aObject, strShip, OT_THING);

  HSObject *object = sHSDB.FindObject(obj);
  if (!object || object->GetType() != HSOT_SHIP) {
    HSIFNotify(aObject, "Invalid ship.");
    return;
  }
  HSShip *ship = static_cast<HSShip*>(object);
  HSSystem *system = sHSDB.FindSystem(atoi(strSystem.c_str())); 

  if (!system) {
    HSIFNotify(aObject, "Invalid system.");
    return;
  }

  HSSystemInstance *instance = 
    HSSystemInstance::CreateSystemInstanceForType((HSSystemType)system->GetType());
  instance->SetSystemID(system->GetID());
  instance->SetSystem(system);
  ship->AddSystem(instance);
  instance->SetShip(ship);
  HSIFNotify(aObject, "System succesfully placed on ship.");
}

void
HSCommandRemoveSystem(DBRef aObject, string aArgument)
{
  string strShip = aArgument.substr(0, aArgument.find("="));
  string strSystem = aArgument.substr(aArgument.find("=") + 1);

  DBRef obj = HSIFLocateObject(aObject, strShip, OT_THING);

  HSObject *object = sHSDB.FindObject(obj);
  if (!object || object->GetType() != HSOT_SHIP) {
    HSIFNotify(aObject, "Invalid ship.");
    return;
  }
  HSShip *ship = static_cast<HSShip*>(object);
  if (ship->RemoveSystemByName(strSystem)) {
    HSIFNotify(aObject, "System succesfully removed from ship.");
  } else {
    HSIFNotify(aObject, "System not found.");
  }
}

void
HSCommandSetShipSystem(DBRef aObject, std::string aArgument)
{
  string strShip = aArgument.substr(0, aArgument.find("/"));
  string strRest = aArgument.substr(aArgument.find("/") + 1);
  string strSystem = strRest.substr(0, strRest.find("="));
  strRest = strRest.substr(strRest.find("=") + 1);
  string strAttr = strRest.substr(0, strRest.find("/"));
  string strValue = strRest.substr(strRest.find("/") + 1);

  DBRef obj = HSIFLocateObject(aObject, strShip, OT_THING);

  HSObject *object = sHSDB.FindObject(obj);
  if (!object || object->GetType() != HSOT_SHIP) {
    HSIFNotify(aObject, "Invalid ship.");
    return;
  }
  HSShip *ship = static_cast<HSShip*>(object);
  HSSystemInstance *sys = ship->FindSystemByName(strSystem);

  if (!sys) {
    HSIFNotify(aObject, "System not found.");
    return;
  }

  if (sys->SetAttributeFromString(strAttr, strValue)) {
    HSIFNotify(aObject, "Attribute set.");
  } else {
    HSIFNotify(aObject, "Failed to set attribute.");
  }
}

void
HSCommandAddConsole(DBRef aObject, string aArgument)
{
  string strShip = aArgument.substr(0, aArgument.find("="));
  string strConsole = aArgument.substr(aArgument.find("=") + 1);

  DBRef objShip = HSIFLocateObject(aObject, strShip, OT_THING);

  HSObject *object = sHSDB.FindObject(objShip);
  if (!object || object->GetType() != HSOT_SHIP) {
    HSIFNotify(aObject, "Invalid ship.");
    return;
  }
  HSShip *ship = static_cast<HSShip*>(object);
  
  DBRef objConsole = HSIFLocateObject(aObject, strConsole, OT_THING);

  if (objConsole == -1) {
    HSIFNotify(aObject, "Cannot find console object.");
    return;
  }

  if (sHSDB.FindObject(objConsole) || sHSDB.FindConsole(objConsole)) {
    HSIFNotify(aObject, "That object is already a HSpace object or console.");
    return;
  }

  HSConsole *newConsole = new HSConsole();
  newConsole->SetShipID(ship->GetID());
  newConsole->SetID(objConsole);
  ship->AddConsole(newConsole);
  newConsole->SetShip(ship);
  HSIFSetObjectLock(newConsole->GetID(), HSLock_Use, newConsole->GetID());
  HSIFNotify(aObject, "Console added.");
}

void
HSCommandDelConsole(DBRef aObject, string aArgument)
{
  DBRef objConsole = HSIFLocateObject(aObject, aArgument, OT_THING);

  if (objConsole == -1) {
    HSIFNotify(aObject, "Cannot find console object.");
    return;
  }

  HSConsole *console = sHSDB.FindConsole(objConsole);
  if (!console) {
    HSIFNotify(aObject, "That object is not a console.");
    return;
  }

  console->GetShip()->RemoveConsoleByID(console->GetID());
  HSIFNotify(aObject, "Console deleted.");
}

void
HSCommandAddLandingLoc(DBRef aObject, string aArgument)
{
  string strObject = aArgument.substr(0, aArgument.find("="));
  string strLandingLoc = aArgument.substr(aArgument.find("=") + 1);

  if (strLandingLoc.empty() || strObject.empty()) {
    HSIFNotify(aObject, "Must specify a valid ship and landing location");
    return;
  }

  DBRef obj = HSIFLocateObject(aObject, strObject, OT_THING);

  HSObject *object = sHSDB.FindObject(obj);
  if (!object) {
    HSIFNotify(aObject, "Invalid object.");
    return;
  }
  
  DBRef objLandingLoc = HSIFLocateObject(aObject, strLandingLoc, OT_ROOM);

  if (objLandingLoc == DBRefNothing) {
    HSIFNotify(aObject, "Cannot find landing location room.");
    return;
  }

  if (sHSDB.FindLandingLocation(objLandingLoc)) {
    HSIFNotify(aObject, "That object is already a HSpace landing location.");
    return;
  }

  HSLandingLocation *newLandingLoc = new HSLandingLocation();
  newLandingLoc->SetObjectID(object->GetID());
  newLandingLoc->SetID(objLandingLoc);
  object->AddLandingLocation(newLandingLoc);
  HSIFNotify(aObject, "Landing location added.");
}

void
HSCommandRemoveLandingLoc(DBRef aObject, string aArgument)
{
  DBRef obj = HSIFLocateObject(aObject, aArgument, OT_ROOM);

  if (obj == DBRefNothing) {
    HSIFNotify(aObject, "Cannot find landing location room.");
    return;
  }

  HSLandingLocation *landingLoc = sHSDB.FindLandingLocation(obj);
  if (!landingLoc) {
    HSIFNotify(aObject, "That is not a valid landing location.");
    return;
  }

  HSObject *object = sHSDB.FindObject(landingLoc->GetObjectID());
  if (!object) {
    HSIFNotify(aObject, "WARNING: Failed. Database inconsistency.");
    return;
  }

  if (object->RemoveLandingLocationByID(obj)) {
    HSIFNotify(aObject, "Landing location removed.");
  } else {
    HSIFNotify(aObject, "Failed to remove landing location.");
  }
}

void
HSCommandAddTerritory(DBRef aObject, string aArgument)
{
  DBRef obj = HSIFLocateObject(aObject, aArgument, OT_THING);

  if (obj == -1) {
    HSIFNotify(aObject, "Invalid object specified.");
    return;
  }

  if (sHSDB.FindTerritory(obj)) {
    HSIFNotify(aObject, "That object is already a space territory.");
    return;
  }

  if (!sHSDB.GetUniverses().size()) {
    HSIFNotify(aObject, "No valid universes available to place that in.");
    return;
  }

  HSTerritory *newTerritory;
  newTerritory = new HSTerritory();
  if (!newTerritory) {
      HSIFNotify(aObject, "Invalid territory type specified");
      return;
  }

  newTerritory->SetID(obj);
  newTerritory->SetName("New Territory");

  // Grab first universe available and dump the territory there
  HSUniverse *universe = *sHSDB.GetUniverses().begin();
  newTerritory->SetUniverse(universe->GetID());
  universe->AddTerritory(newTerritory);

  HSIFNotify(aObject, "Territory succesfully created!");
}


void
HSCommandDelTerritory(DBRef aObject, string aArgument)
{
  string thing = aArgument;

  DBRef obj = HSIFLocateObject(aObject, thing, OT_THING);

  if (obj == -1) {
    HSIFNotify(aObject, "Invalid object specified.");
    return;
  }

  HSTerritory *territory = sHSDB.FindTerritory(obj);
  if (!territory) {
    HSIFNotify(aObject, "That is not a valid territory.");
    return;
  }

  HSUniverse *universe = sHSDB.FindUniverse(territory->GetUniverse());
  ASSERT(universe);
  while(universe->RemoveTerritory(territory)) {}
  delete territory;
  HSIFNotify(aObject, "Territory succesfully destroyed!");
}

void
HSCommandAddDockingHatch(DBRef aObject, string aArgument)
{
  string strObject = aArgument.substr(0, aArgument.find("="));
  string strDockingHatch = aArgument.substr(aArgument.find("=") + 1);

  if (strDockingHatch.empty() || strObject.empty()) {
    HSIFNotify(aObject, "Must specify a valid ship and docking hatch.");
    return;
  }

  DBRef obj = HSIFLocateObject(aObject, strObject, OT_THING);

  HSObject *object = sHSDB.FindObject(obj);
  if (!object) {
    HSIFNotify(aObject, "Invalid object.");
    return;
  }
  
  DBRef objDockingHatch = HSIFLocateObject(aObject, strDockingHatch, OT_EXIT);

  if (objDockingHatch == DBRefNothing) {
    HSIFNotify(aObject, "Cannot find docking hatch exit.");
    return;
  }

  if (sHSDB.FindDockingHatch(objDockingHatch)) {
    HSIFNotify(aObject, "That object is already a HSpace docking hatch.");
    return;
  }

  HSDockingHatch *newDockingHatch = new HSDockingHatch();
  newDockingHatch->SetObjectID(object->GetID());
  newDockingHatch->SetID(objDockingHatch);
  object->AddDockingHatch(newDockingHatch);
  HSIFNotify(aObject, "Docking hatch added.");
}

void
HSCommandRemoveDockingHatch(DBRef aObject, string aArgument)
{
  DBRef obj = HSIFLocateObject(aObject, aArgument, OT_EXIT);

  if (obj == DBRefNothing) {
    HSIFNotify(aObject, "Cannot find docking hatch exit.");
    return;
  }

  HSDockingHatch *dockingHatch = sHSDB.FindDockingHatch(obj);
  if (!dockingHatch) {
    HSIFNotify(aObject, "That is not a valid docking hatch.");
    return;
  }

  HSObject *object = sHSDB.FindObject(dockingHatch->GetObjectID());
  if (!object) {
    HSIFNotify(aObject, "WARNING: Failed. Database inconsistency.");
    return;
  }

  if (object->RemoveDockingHatchByID(obj)) {
    HSIFNotify(aObject, "Docking hatch removed.");
  } else {
    HSIFNotify(aObject, "Failed to remove docking hatch.");
  }
}

void
HSCommandList(DBRef aObject, string aArgument)
{
  string obj = aArgument.substr(0, aArgument.find("="));
  if (!strncasecmp(obj.c_str(), "UNIVERSES")) {
    std::vector<HSUniverse*> universes = sHSDB.GetUniverses();
    HSIFNotify(aObject, " [ID]  Universe Name");
    HSIFNotify(aObject, "------ --------------------------------");
    char idbuf[32];
    char tbuf[256];

    foreach(HSUniverse*, universe, universes) {
      sprintf(idbuf, "[%d]", universe->GetID());
      sprintf(tbuf, "%6s %-32s", idbuf, universe->GetName().c_str());
      HSIFNotify(aObject, tbuf);
    }
  } else if (!strncasecmp(obj.c_str(), "HULLCLASSES")) {
    std::vector<HSHullClass*> classes = sHSDB.GetHullClasses();
    HSIFNotify(aObject, " [ID]  Class Name");
    HSIFNotify(aObject, "------ --------------------------------");
    char idbuf[32];
    char tbuf[256];

    foreach(HSHullClass*, hullClass, classes) {
      sprintf(idbuf, "[%d]", hullClass->GetID());
      sprintf(tbuf, "%6s %-32s", idbuf, hullClass->GetName().c_str());
      HSIFNotify(aObject, tbuf);
    }
  } else if (!strncasecmp(obj.c_str(), "COMMODITIES")) {
    HSIFNotify(aObject, " [ID]  Commodity Name");
    HSIFNotify(aObject, "------ --------------------------------");
    char idbuf[32];
    char tbuf[256];

    foreach(HSCommodity*, commodity, sHSDB.GetCommodities()) {
      sprintf(idbuf, "[%d]", commodity->GetID());
      sprintf(tbuf, "%6s %-32s", idbuf, commodity->GetName().c_str());
      HSIFNotify(aObject, tbuf);
    }
  } else if (!strncasecmp(obj.c_str(), "WAREHOUSES")) {
    HSIFNotify(aObject, " [ID]  Warehouse Name");
    HSIFNotify(aObject, "------ --------------------------------");
    char idbuf[32];
    char tbuf[256];

    foreach(HSWarehouse*, warehouse, sHSDB.GetWarehouses()) {
      sprintf(idbuf, "[%d]", warehouse->GetID());
      sprintf(tbuf, "%6s %-32s", idbuf, warehouse->GetName().c_str());
      HSIFNotify(aObject, tbuf);
    }
  } else if (!strncasecmp(obj.c_str(), "SYSTEM")) {
    std::vector<HSSystem*> systems = sHSDB.GetSystems();
    HSIFNotify(aObject, " [ID]  System Name                      System Type");
    HSIFNotify(aObject, "------ -------------------------------- --------------------");
    char idbuf[32];
    char tbuf[256];

    foreach(HSSystem*, system, systems) {
      sprintf(idbuf, "[%d]", system->GetID());
      sprintf(tbuf, "%6s %-32s %-20s", idbuf, system->GetName().c_str(),
        HSSystem::GetNameForType((HSSystemType)system->GetType()).c_str());
      HSIFNotify(aObject, tbuf);
    }
  } else if (!strncasecmp(obj.c_str(), "OBJECTS")) {
    string universe = aArgument.substr(aArgument.find("=") + 1);

    HSUniverse *uni = sHSDB.FindUniverse(atoi(universe.c_str()));
    if (uni) {
      std::vector<HSObject*> objects = uni->GetObjects();
      HSIFNotify(aObject, " [ID]  Object Name                      Object Type");
      HSIFNotify(aObject, "------ -------------------------------- ------------");
      char idbuf[32];
      char tbuf[256];

      foreach(HSObject*, object, objects) {
        sprintf(idbuf, "[%d]", object->GetID());
        sprintf(tbuf, "%6s %-32s %-12s", idbuf, object->GetName().c_str(), 
          HSObject::GetNameForType((HSObjectType)object->GetType()).c_str());
        HSIFNotify(aObject, tbuf);
      }
    } else {
      HSIFNotify(aObject, "Invalid universe specified");
    }
  } else if (!strncasecmp(obj.c_str(), "TERRITORIES")) {
    string universe = aArgument.substr(aArgument.find("=") + 1);

    HSUniverse *uni = sHSDB.FindUniverse(atoi(universe.c_str()));
    if (uni) {
      std::vector<HSTerritory*> territories = uni->GetTerritories();
      HSIFNotify(aObject, " [ID]  Territory Name                  ");
      HSIFNotify(aObject, "------ --------------------------------");
      char idbuf[32];
      char tbuf[256];

      foreach(HSTerritory*, territory, territories) {
        sprintf(idbuf, "[%d]", territory->GetID());
        sprintf(tbuf, "%6s %-32s", idbuf, territory->GetName().c_str());
        HSIFNotify(aObject, tbuf);
      }
    } else {
      HSIFNotify(aObject, "Invalid universe specified");
    }
  } else {
    HSIFNotify(aObject, "Invalid vectoring.");
  }
}

void
HSCommandDumpObject(DBRef aObject, std::string aType, std::string aArgument)
{
  HSAttributed *obj = NULL;
  if (!strncasecmp(aType.c_str(), "UNIVERSE")) {
   DBRef roomObj = HSIFLocateObject(aObject, aArgument, OT_ROOM);

    if (roomObj == -1) {
      HSIFNotify(aObject, "Invalid universe specified.");
      return;
    }

    HSUniverse *uni;
    if (!(uni = sHSDB.FindUniverse(roomObj))) {
      HSIFNotify(aObject, "That is not a valid universe.");
      return;
    }
    obj = uni;       
  } else if (!strncasecmp(aType.c_str(), "OBJECT")) {
    DBRef thing = HSIFLocateObject(aObject, aArgument, OT_THING);

    if (thing == -1) {
      HSIFNotify(aObject, "Invalid object specified.");
      return;
    }

    HSObject *spaceobj = sHSDB.FindObject(thing);
    if (!spaceobj) {
      HSIFNotify(aObject, "That is not a valid space object.");
      return;
    }
    obj = spaceobj;       
  } else if (!strncasecmp(aType.c_str(), "TERRITORY")) {
    DBRef thing = HSIFLocateObject(aObject, aArgument, OT_THING);

    if (thing == -1) {
      HSIFNotify(aObject, "Invalid object specified.");
      return;
    }

    HSTerritory *territory = sHSDB.FindTerritory(thing);
    if (!territory) {
      HSIFNotify(aObject, "That is not a valid territory.");
      return;
    }
    obj = territory;    
  } else if (!strncasecmp(aType.c_str(), "LANDINGLOCATION")) {
    DBRef room = HSIFLocateObject(aObject, aArgument, OT_ROOM);

    if (room == -1) {
      HSIFNotify(aObject, "Invalid landing location specified.");
      return;
    }

    HSLandingLocation *landingRoom = sHSDB.FindLandingLocation(room);
    if (!landingRoom) {
      HSIFNotify(aObject, "That is not a valid landing location.");
      return;
    }
    obj = landingRoom;    
  } else if (!strncasecmp(aType.c_str(), "DOCKINGHATCH")) {
    DBRef exit = HSIFLocateObject(aObject, aArgument, OT_EXIT);

    if (exit == -1) {
      HSIFNotify(aObject, "Invalid docking hatch specified.");
      return;
    }

    HSDockingHatch *dockingHatch = sHSDB.FindDockingHatch(exit);
    if (!dockingHatch) {
      HSIFNotify(aObject, "That is not a valid landing location.");
      return;
    }
    obj = dockingHatch;    
  } else if (!strncasecmp(aType.c_str(), "HULLCLASS")) {
    obj = sHSDB.FindHullClass(atoi(aArgument.c_str()));
    if (!obj) {
      HSIFNotify(aObject, "That is not a valid hull class.");
      return;
    }
  } else if (!strncasecmp(aType.c_str(), "COMMODITY")) {
    obj = sHSDB.FindCommodity(atoi(aArgument.c_str()));
    if (!obj) {
      HSIFNotify(aObject, "That is not a valid commodity.");
      return;
    }
  } else if (!strncasecmp(aType.c_str(), "WAREHOUSE")) {
    obj = sHSDB.FindWarehouse(atoi(aArgument.c_str()));
    if (!obj) {
      HSIFNotify(aObject, "That is not a valid warehouse.");
      return;
    }
  } else if (!strncasecmp(aType.c_str(), "SYSTEM")) {
    obj = sHSDB.FindSystem(atoi(aArgument.c_str()));
    if (!obj) {
      HSIFNotify(aObject, "That is not a valid system");
      return;
    }
  } else if (!strncasecmp(aType.c_str(), "SHIPSYS")) {
    if ((int)aArgument.find("/") == -1) {
      HSIFNotify(aObject, "Must specify ship and system name");
      return;
    }
    std::string strship = aArgument.substr(0, aArgument.find("/"));
    std::string system = aArgument.substr(aArgument.find("/") + 1);
    DBRef thing = HSIFLocateObject(aObject, strship, OT_THING);

    if (thing == -1) {
      HSIFNotify(aObject, "Invalid ship specified.");
      return;
    }
    HSObject *spaceobj = sHSDB.FindObject(thing);
    if (!spaceobj || spaceobj->GetType() != HSOT_SHIP) {
      HSIFNotify(aObject, "Invalid ship specified.");
      return;
    }
    HSShip *ship = static_cast<HSShip*>(spaceobj);
    obj = ship->FindSystemByName(system);
    if (!obj) {
      HSIFNotify(aObject, "System not found.");
      return;
    }
  } else if (!strncasecmp(aType.c_str(), "CONF")) {
    obj = &sHSConf;
  } else {
    HSIFNotify(aObject, "Unknown switch.");
    return;
  }
  HSIFNotify(aObject, "------------------------------------------------------------------------------");
  char tbuf[256];
  std::vector<Attribute> attrs = obj->GetAttributeList();

  string value;
  foreach(Attribute, att, attrs) {
    obj->GetAttributeFromString(att.Name, &value);
    if (!att.IsSet || *(att.IsSet)) {
      sprintf(tbuf, "%20s : %s", att.Name.c_str(), value.c_str());
    } else {
      sprintf(tbuf, "%20s : Not set", att.Name.c_str());
    }
    HSIFNotify(aObject, tbuf);
  }
  foreach(Attribute, att, obj->GetExtraAttributes()) {
    obj->GetAttributeFromString(att.Name, &value);
    if (!att.IsSet || *(att.IsSet)) {
      sprintf(tbuf, "%20s : %s", att.Name.c_str(), value.c_str());
    } else {
      sprintf(tbuf, "%20s : Not set", att.Name.c_str());
    }
    HSIFNotify(aObject, tbuf);
  }
  string additionalInfo = obj->GetAdditionalInfo();
  if (!additionalInfo.empty()) {
    HSIFNotify(aObject, additionalInfo);
  }
  HSIFNotify(aObject, "------------------------------------------------------------------------------");
}

void
HSCommandSetObject(DBRef aObject, std::string aType, std::string aArgument)
{
  HSAttributed *obj = NULL;
  string target = aArgument.substr(0, aArgument.find("/"));
  string rest = aArgument.substr(aArgument.find("/") + 1);
  string attribute = rest.substr(0, rest.find("="));
  string value = rest.substr(rest.find("=") + 1);
  if (!strncasecmp(aType.c_str(), "UNIVERSE")) {
   DBRef roomObj = HSIFLocateObject(aObject, target, OT_ROOM);

    if (roomObj == -1) {
      HSIFNotify(aObject, "Invalid universe specified.");
      return;
    }

    HSUniverse *uni;
    if (!(uni = sHSDB.FindUniverse(roomObj))) {
      HSIFNotify(aObject, "That is not a valid universe.");
      return;
    }
    obj = uni;       
  } else if (!strncasecmp(aType.c_str(), "HULLCLASS")) {
    obj = sHSDB.FindHullClass(atoi(target.c_str()));
    if (!obj) {
      HSIFNotify(aObject, "That is not a valid hull class.");
      return;
    }
  } else if (!strncasecmp(aType.c_str(), "COMMODITY")) {
    obj = sHSDB.FindCommodity(atoi(target.c_str()));
    if (!obj) {
      HSIFNotify(aObject, "That is not a valid commodity.");
      return;
    }
  } else if (!strncasecmp(aType.c_str(), "WAREHOUSE")) {
    obj = sHSDB.FindWarehouse(atoi(target.c_str()));
    if (!obj) {
      HSIFNotify(aObject, "That is not a valid warehouse.");
      return;
    }
  } else if (!strncasecmp(aType.c_str(), "SYSTEM")) {
    obj =  sHSDB.FindSystem(atoi(target.c_str()));
    if (!obj) {
      HSIFNotify(aObject, "That is not a valid system.");
      return;
    }
  } else if (!strncasecmp(aType.c_str(), "OBJECT")) {
   DBRef thing = HSIFLocateObject(aObject, target, OT_THING);

    if (thing == -1) {
      HSIFNotify(aObject, "Invalid object specified.");
      return;
    }

    HSObject *spaceobj = sHSDB.FindObject(thing);
    if (!spaceobj) {
      HSIFNotify(aObject, "That is not a valid space object.");
      return;
    }
    obj = spaceobj;
    // Special case for the hatch location attribute.
    if (!strncasecmp(attribute.c_str(), "HATCHLOC")) {
      DBRef hatchLoc = HSIFLocateObject(aObject, value, OT_ROOM);
      if (hatchLoc == DBRefNothing) {
        HSIFNotify(aObject, "That is not a valid room.");
        return;
      }
      obj->SetAttribute("HATCHLOC", hatchLoc, true);
      HSIFNotify(aObject, "Attribute set.");
      return;
    }
  } else if (!strncasecmp(aType.c_str(), "TERRITORY")) {
   DBRef thing = HSIFLocateObject(aObject, target, OT_THING);

    if (thing == -1) {
      HSIFNotify(aObject, "Invalid object specified.");
      return;
    }

    HSTerritory *territory = sHSDB.FindTerritory(thing);
    if (!territory) {
      HSIFNotify(aObject, "That is not a valid territory.");
      return;
    }
    obj = territory;
  } else if (!strncasecmp(aType.c_str(), "LANDINGLOCATION")) {
   DBRef thing = HSIFLocateObject(aObject, target, OT_ROOM);

    if (thing == -1) {
      HSIFNotify(aObject, "Invalid room specified.");
      return;
    }

    HSLandingLocation *landingRoom = sHSDB.FindLandingLocation(thing);
    if (!landingRoom) {
      HSIFNotify(aObject, "That is not a valid landing location.");
      return;
    }
    obj = landingRoom;
  } else if (!strncasecmp(aType.c_str(), "DOCKINGHATCH")) {
   DBRef thing = HSIFLocateObject(aObject, target, OT_EXIT);

    if (thing == -1) {
      HSIFNotify(aObject, "Invalid exit specified.");
      return;
    }

    HSDockingHatch *dockingHatch = sHSDB.FindDockingHatch(thing);
    if (!dockingHatch) {
      HSIFNotify(aObject, "That is not a valid docking hatch.");
      return;
    }
    obj = dockingHatch;
  } else if (!strncasecmp(aType.c_str(), "CONF")) {
    string opt = aArgument.substr(0, aArgument.find("="));
    string val = aArgument.substr(aArgument.find("=") + 1);
    if (sHSConf.SetAttributeFromString(opt, val)) {
      HSIFNotify(aObject, "Option set.");
    } else {
      HSIFNotify(aObject, "Failed to set option.");
    }
    return;
  } else {
    HSIFNotify(aObject, "Unknown switch.");
    return;
  }

  if (obj->SetAttributeFromString(attribute, value)) {
    HSIFNotify(aObject, "Attribute set.");
  } else {
    HSIFNotify(aObject, "Failed to set attribute.");
  }
}

void
HSCommandAddSRoom(DBRef aObject, std::string aArgument)
{
  if (aArgument.empty()) {
    HSIFNotify(aObject, "Must specify a ship and a room.");
    return;
  }

  string strShip = aArgument.substr(0, aArgument.find("="));
  string strRoom = aArgument.substr(aArgument.find("=") + 1);

  DBRef shipRef = HSIFLocateObject(aObject, strShip, OT_THING);
  DBRef roomRef = HSIFLocateObject(aObject, strRoom, OT_ROOM);

  if (shipRef == DBRefNothing) {
    HSIFNotify(aObject, "Ship not found.");
    return;
  }

  if (roomRef == DBRefNothing) {
    HSIFNotify(aObject, "Room not found.");
    return;
  }

  HSObject *obj = sHSDB.FindObject(shipRef);

  if (!obj || obj->GetType() != HSOT_SHIP) {
    HSIFNotify(aObject, "Ship not found.");
    return;
  }

  HSShip *ship = static_cast<HSShip*>(obj);

  std::vector<int> rooms = ship->GetShipRooms();
  foreach(int, room, rooms) {
    if (room == roomRef) {
      HSIFNotify(aObject, "That is already a shiproom for that ship.");
      return;
    }
  }
  rooms.push_back(roomRef);
  ship->SetShipRooms(rooms);
  HSIFNotify(aObject, "Ship room added.");
}

void
HSCommandDelSRoom(DBRef aObject, std::string aArgument)
{
  if (aArgument.empty()) {
    HSIFNotify(aObject, "Must specify a ship and a room.");
    return;
  }

  string strShip = aArgument.substr(0, aArgument.find("="));
  string strRoom = aArgument.substr(aArgument.find("=") + 1);

  DBRef shipRef = HSIFLocateObject(aObject, strShip, OT_THING);
  DBRef roomRef = HSIFLocateObject(aObject, strRoom, OT_ROOM);

  if (shipRef == DBRefNothing) {
    HSIFNotify(aObject, "Ship not found.");
    return;
  }

  if (roomRef == DBRefNothing) {
    HSIFNotify(aObject, "Room not found.");
    return;
  }

  HSObject *obj = sHSDB.FindObject(shipRef);

  if (!obj || obj->GetType() != HSOT_SHIP) {
    HSIFNotify(aObject, "Ship not found.");
    return;
  }

  HSShip *ship = static_cast<HSShip*>(obj);

  std::vector<int> rooms = ship->GetShipRooms();

  for (std::vector<int>::iterator iter = rooms.begin(); iter != rooms.end(); iter++) {
    if (*iter == roomRef) {
      rooms.erase(iter);
      HSIFNotify(aObject, "Ship room removed.");
      ship->SetShipRooms(rooms);
      return;
    }
  }
  
  HSIFNotify(aObject, "Ship room not found on that ship.");
}

void
HSCommandClone(DBRef aObject, std::string aArgument)
{
  if (aArgument.empty()) {
    HSIFNotify(aObject, "Must specify a ship to clone.");
    return;
  }

  DBRef shipRef = HSIFLocateObject(aObject, aArgument, OT_THING);

  if (shipRef == DBRefNothing) {
    HSIFNotify(aObject, "Ship not found.");
    return;
  }

  HSObject *obj = sHSDB.FindObject(shipRef);

  if (!obj || obj->GetType() != HSOT_SHIP) {
    HSIFNotify(aObject, "Ship not found.");
    return;
  }

  HSShip *ship = static_cast<HSShip*>(obj);

  HSShip *clonedShip = ship->Clone();

  char tbuf[256];
  sprintf(tbuf, "Ship cloned as #%d.", clonedShip->GetID());
  HSIFNotify(aObject, tbuf);
}
