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
#include "HSIface.h"
#include "HSTools.h"
#include "HSUniverse.h"
#include "HSHullClass.h"
#include "HSSystem.h"
#include "HSObject.h"
#include "HSDB.h"
#include "HSShip.h"
#include "HSCargoBay.h"
#include "HSWarehouse.h"
#include "HSConsole.h"
#include "HSComputer.h"

// STL
#include <sstream>

std::string
HSFunctionSet(DBRef aObject,
              std::string aType, 
              std::string aTargetObject, 
              std::string aAttribute, 
              std::string aValue)
{
  HSAttributed *obj = 0;
  if (!strncasecmp(aType.c_str(), "UNIVERSE")) {
   DBRef roomObj = HSIFLocateObject(aObject, aTargetObject, OT_ROOM);

    if (roomObj == -1) {
      return "#-1 INVALID UNIVERSE";
    }

    HSUniverse *uni;
    if (!(uni = sHSDB.FindUniverse(roomObj))) {
      return "#-1 INVALID UNIVERSE";
    }
    obj = uni;       
  } else if (!strncasecmp(aType.c_str(), "HULLCLASS")) {
    obj = sHSDB.FindHullClass(atoi(aTargetObject.c_str()));
    if (!obj) {
      return "#-1 INVALID HULLCLASS";
    }
  } else if (!strncasecmp(aType.c_str(), "WAREHOUSE")) {
    obj = sHSDB.FindWarehouse(atoi(aTargetObject.c_str()));
    if (!obj) {
      return "#-1 INVALID WAREHOUSE";
    }  
  } else if (!strncasecmp(aType.c_str(), "COMMODITY")) {
    obj = sHSDB.FindCommodity(atoi(aTargetObject.c_str()));
    if (!obj) {
      return "#-1 INVALID COMMODITY";
    }  
  } else if (!strncasecmp(aType.c_str(), "SYSTEM")) {
    obj =  sHSDB.FindSystem(atoi(aTargetObject.c_str()));
    if (!obj) {
      return "#-1 INVALID SYSTEM";
    }
  } else if (!strncasecmp(aType.c_str(), "OBJECT")) {
   DBRef thing = HSIFLocateObject(aObject, aTargetObject, OT_THING);

    if (thing == -1) {
      return "#-1 INVALID OBJECT";
    }

    HSObject *spaceObj = sHSDB.FindObject(thing);
    if (!spaceObj) {
      return "#-1 INVALID OBJECT";
    }
    obj = spaceObj;
    // Special case for the hatch location attribute.
    if (!strncasecmp(aAttribute.c_str(), "HATCHLOC")) {
      if (spaceObj->GetType() != HSOT_SHIP) {
        return "#-5 MUST BE SHIP";
      }
      DBRef hatchLoc = HSIFLocateObject(aObject, aValue, OT_ROOM);
      if (hatchLoc == DBRefNothing) {
        return "#-6 INVALID HATCHLOC ROOM";
      }
      obj->SetAttribute("HATCHLOC", hatchLoc, true);
      return "ATTRIBUTE SET";
    }
  } else {
    return "#-2 INVALID TYPE";
  }
  if (!obj) {
    return "#-3 OBJECT NOT FOUND";
  }
  if (obj->SetAttributeFromString(aAttribute, aValue)) {
    return "ATTRIBUTE SET";
  } else {
    return "#-4 FAILED TO SET ATTRIBUTE";
  }
}

std::string
HSFunctionGet(DBRef aObject,
              std::string aType,
              std::string aTargetObject,
              std::string aAttribute)
{
  HSAttributed *obj = 0;
  if (!strncasecmp(aType.c_str(), "UNIVERSE")) {
   DBRef roomObj = HSIFLocateObject(aObject, aTargetObject, OT_ROOM);

    if (roomObj == -1) {
      return "#-1 INVALID UNIVERSE";
    }

    HSUniverse *uni;
    if (!(uni = sHSDB.FindUniverse(roomObj))) {
      return "#-1 INVALID UNIVERSE";
    }
    obj = uni;       
  } else if (!strncasecmp(aType.c_str(), "HULLCLASS")) {
    obj = sHSDB.FindHullClass(atoi(aTargetObject.c_str()));
    if (!obj) {
      return "#-1 INVALID HULLCLASS";
    }
  } else if (!strncasecmp(aType.c_str(), "SYSTEM")) {
    obj =  sHSDB.FindSystem(atoi(aTargetObject.c_str()));
    if (!obj) {
      return "#-1 INVALID SYSTEM";
    }
  } else if (!strncasecmp(aType.c_str(), "COMMODITY")) {
    obj =  sHSDB.FindCommodity(atoi(aTargetObject.c_str()));
    if (!obj) {
      return "#-1 INVALID COMMODITY";
    }  
  } else if (!strncasecmp(aType.c_str(), "OBJECT")) {
   DBRef thing = HSIFLocateObject(aObject, aTargetObject, OT_THING);

    if (thing == -1) {
      return "#-1 INVALID OBJECT";
    }

    HSObject *spaceObj = sHSDB.FindObject(thing);
    if (!spaceObj) {
      return "#-1 INVALID OBJECT";
    }
    obj = spaceObj;
  } else if (!strncasecmp(aType.c_str(), "CONSOLE")) {
   DBRef thing = HSIFLocateObject(aObject, aTargetObject, OT_THING);

    if (thing == -1) {
      return "#-1 INVALID OBJECT";
    }

    HSConsole *consoleObj = sHSDB.FindConsole(thing);
    if (!consoleObj) {
      return "#-1 INVALID OBJECT";
    }
    obj = consoleObj;
  } else {
    return "#-3 INVALID TYPE";
  }
  if (!obj) {
    return "#-4 OBJECT NOT FOUND";
  }
  std::string retval;
  if (obj->GetAttributeFromString(aAttribute, &retval)) {
    return retval;
  } else {
    return "#-5 ATTRIBUTE NOT FOUND";
  }
}

std::string
HSFunctionShipSysSet(DBRef aObject,
                     std::string aShip,
                     std::string aSystem,
                     std::string aAttribute,
                     std::string *aValue)
{
  DBRef thing = HSIFLocateObject(aObject, aShip, OT_THING);

  if (thing == -1) {
    return "#-1 INVALID SHIP";
  }

  HSObject *spaceObj = sHSDB.FindObject(thing);
  if (!spaceObj || spaceObj->GetType() != HSOT_SHIP) {
    return "#-1 INVALID SHIP";
  }
  HSShip *ship = static_cast<HSShip*>(spaceObj);

  HSSystemInstance* system = ship->FindSystemByName(aSystem);

  if (!system) {
    return "#-2 SYSTEM NOT FOUND";
  }

  if (aValue) {
    if (system->SetAttributeFromString(aAttribute, *aValue)) {
      return "ATTRIBUTE SET";
    } else {
      return "#-3 ATTRIBUTE NOT FOUND";
    }
  } else {
    if (system->WipeAttribute(aAttribute, false)) {
      return "ATTRIBUTE SET";
    } else {
      return "#-4 ATTRIBUTE NOT FOUND OR NOT WIPEABLE";
    }
  }
}

std::string
HSFunctionShipSysGet(DBRef aObject,
                     std::string aShip,
                     std::string aSystem,
                     std::string aAttribute)
{
  DBRef thing = HSIFLocateObject(aObject, aShip, OT_THING);

  if (thing == -1) {
    return "#-1 INVALID SHIP";
  }

  HSObject *spaceObj = sHSDB.FindObject(thing);
  if (!spaceObj || spaceObj->GetType() != HSOT_SHIP) {
    return "#-1 INVALID SHIP";
  }
  HSShip *ship = static_cast<HSShip*>(spaceObj);

  HSSystemInstance* system = ship->FindSystemByName(aSystem);

  if (!system) {
    return "#-2 SYSTEM NOT FOUND";
  }
  std::string retval;
  if (system->GetAttributeFromString(aAttribute, &retval)) {
    return retval;
  } else {
    if (system->GetSystem()->GetAttributeFromString(aAttribute, &retval)) {
      return retval;
    } else {
      return "#-3 ATTRIBUTE NOT FOUND";
    }
  }
}

std::string
HSFunctionClone(DBRef aCaller, std::string aShip)
{
  DBRef shipRef = HSIFLocateObject(aCaller, aShip, OT_THING);

  if (shipRef == DBRefNothing) {
    return "#-1 SHIP NOT FOUND";
  }

  HSObject *obj = sHSDB.FindObject(shipRef);

  if (!obj || obj->GetType() != HSOT_SHIP) {
    return "#-1 SHIP NOT FOUND";
  }

  HSShip *ship = static_cast<HSShip*>(obj);

  HSShip *clonedShip = ship->Clone();

  char tbuf[256];
  sprintf(tbuf, "#%d", clonedShip->GetID());
  return tbuf;
}

std::string
HSFunctionSystems(DBRef aCaller, std::string aShip, std::string aType)
{
  std::stringstream systems;

  DBRef shipRef = HSIFLocateObject(aCaller, aShip, OT_THING);

  if (shipRef == DBRefNothing) {
    return "#-1 SHIP NOT FOUND";
  }

  HSObject *obj = sHSDB.FindObject(shipRef);

  if (!obj || obj->GetType() != HSOT_SHIP) {
    return "#-1 SHIP NOT FOUND";
  }

  int typeNum = -1;
  if (!aType.empty()) {
    if (from_string<int>(typeNum, aType)) {
      if (typeNum < 0 || typeNum >= sSystemTypeCount) {
        return "#-2 INVALID TYPE_SPECIFIED";
      }
      aType = typeNum;
    }
  }

  HSShip *ship = static_cast<HSShip*>(obj);

  std::vector<std::string> names;
  int i = 0;
  
  std::vector<HSSystemInstance*> systemList;
  if (typeNum == -1) {
    systemList = ship->GetSystems();
  } else {
    systemList = ship->FindSystemsByType((HSSystemType)typeNum);
  }

  foreach(HSSystemInstance*, system, systemList) {
    if (i) {
      systems << "|";
    }
    int existCount = 0;
    foreach(std::string, name, names) {
      if (name == system->GetName()) {
        existCount++;
      }
    }
    names.push_back(system->GetName());
    // If there's multiple systems with this name, append our
    // /<instance #> so a hs_shipsys_get will distinguish betweent hem.
    if (existCount > 0) {
      systems << system->GetName() << "/" << existCount;
    } else {
      systems << system->GetName();
    }
    i++;
  }
  return systems.str();
}

std::string
HSFunctionList(DBRef aCaller, std::string aType, std::string *aMore)
{
  std::stringstream list;
  
  if (!strncasecmp(aType.c_str(), "UNIVERSES")) {
    foreach(HSUniverse*, universe, sHSDB.GetUniverses()) {
      if (list.str().size()) {
        list << ' ';
      }
      list << '#' << universe->GetID();
    }
  } else if (!strncasecmp(aType.c_str(), "HULLCLASSES")) {
    foreach(HSHullClass*, hullClass, sHSDB.GetHullClasses()) {
      if (list.str().size()) {
        list << ' ';
      }
      list << hullClass->GetID();
    }
  } else if (!strncasecmp(aType.c_str(), "SYSTEMS")) {
    foreach(HSSystem*, system, sHSDB.GetSystems()) {
      if (list.str().size()) {
        list << ' ';
      }
      list << system->GetID();
    }
  } else if (!strncasecmp(aType.c_str(), "COMMODITIES")) {
    foreach(HSCommodity*, commod, sHSDB.GetCommodities()) {
      if (list.str().size()) {
        list << ' ';
      }
      list << commod->GetID();
    }
  } else if (!strncasecmp(aType.c_str(), "OBJECTS")) {
    if (!aMore) {
      return "#-2 NO VALID UNIVERSE SPECIFIED";
    }
    HSUniverse *universe = sHSDB.FindUniverse(atoi((*aMore).c_str()));
    if (!universe) {
      return "#-2 NO VALID UNIVERSE SPECIFIED";
    }
    foreach(HSObject*, object, universe->GetObjects()) {
      if (list.str().size()) {
        list << ' ';
      }
      list << '#' << object->GetID();
    }
  }
  return list.str();
}

std::string
HSFunctionPlaceSystem(DBRef aCaller,
                      std::string aShip,
                      std::string aSystem,
                      bool aCheckSpace)
{
  DBRef shipRef = HSIFLocateObject(aCaller, aShip, OT_THING);

  if (shipRef == DBRefNothing) {
    return "#-1 SHIP NOT FOUND";
  }

  HSObject *obj = sHSDB.FindObject(shipRef);

  if (!obj || obj->GetType() != HSOT_SHIP) {
    return "#-1 SHIP NOT FOUND";
  }

  HSShip *ship = static_cast<HSShip*>(obj);

  HSSystem *sys = sHSDB.FindSystem(atoi(aSystem.c_str()));

  if (!sys) {
    return "#-2 SYSTEM NOT FOUND";
  }

  if (!sys->MoreAllowed()) {
    std::vector<HSSystemInstance*> systems = 
      ship->FindSystemsByType((HSSystemType)sys->GetType());
    if (systems.size()) {
      return "#-3 ONLY ONE SUCH SYSTEM ALLOWED";
    }
  }

  if (aCheckSpace) {
    int totalSpace = ship->GetHull()->GetSpace();
    int spaceTaken = 0;
    int mountsTaken = 0;
    foreach(HSSystemInstance*, system, ship->GetSystems()) {
      spaceTaken += system->GetSize();
      if (system->GetSystem()->TakesMount()) {
        ++mountsTaken;
      }
    }
    if ((spaceTaken + sys->GetSize()) > totalSpace) {
      return "#-4 NO SPACE FOR THAT SYSTEM";
    }
    if (sys->TakesMount()) {
      if (mountsTaken >= ship->GetHull()->GetMounts()) {
        return "#-5 NO MOUNT AVAILABLE FOR THAT SYSTEM";
      }
    }
  }
  HSSystemInstance *instance = 
    HSSystemInstance::CreateSystemInstanceForType((HSSystemType)sys->GetType());
  instance->SetSystemID(sys->GetID());
  instance->SetSystem(sys);
  ship->AddSystem(instance);
  instance->SetShip(ship);

  return "System placed";
}

std::string
HSFunctionRemoveSystem(DBRef aCaller,
                       std::string aShip,
                       std::string aSystem)
{
  DBRef shipRef = HSIFLocateObject(aCaller, aShip, OT_THING);

  if (shipRef == DBRefNothing) {
    return "#-1 SHIP NOT FOUND";
  }

  HSObject *obj = sHSDB.FindObject(shipRef);

  if (!obj || obj->GetType() != HSOT_SHIP) {
    return "#-1 SHIP NOT FOUND";
  }

  HSShip *ship = static_cast<HSShip*>(obj);

  HSSystemInstance *system = ship->FindSystemByName(aSystem);
  if (!system) {
    return "#-2 SYSTEM NOT FOUND";
  }
  ship->RemoveSystemByName(aSystem);
  return "System removed";
}

HSCargoContainer*
FindContainer(DBRef aCaller, std::string aContainer)
{
  DBRef objRef;

  std::string subname;
  if (aContainer.find("/") != (unsigned int)-1) {
    objRef = HSIFLocateObject(aCaller, aContainer.substr(0, aContainer.find("/")), OT_THING);
    subname = aContainer.substr(aContainer.find("/") + 1);
  } else {
    objRef = HSIFLocateObject(aCaller, aContainer, OT_THING);
  }

  if (objRef == DBRefNothing) {
    return sHSDB.FindWarehouse(atoi(aContainer.c_str()));
  } else {
    HSObject *obj = sHSDB.FindObject(objRef);
    if (obj && obj->GetType() == HSOT_SHIP) {
      HSShip *ship = static_cast<HSShip*>(obj);
      if (subname.empty()) {
        std::vector<HSSystemInstance*> bays = 
          ship->FindSystemsByType(HSST_CARGOBAY);
        if (!bays.size()) {
          return NULL;
        }
        return static_cast<HSCargoContainer*>(
          static_cast<HSCargoBayInstance*>(*bays.begin()));
      } else {
        HSSystemInstance *bay = ship->FindSystemByName(subname);
        if (!bay || bay->GetType() != HSST_CARGOBAY) {
          return NULL;
        }
        return static_cast<HSCargoContainer*>(
          static_cast<HSCargoBayInstance*>(bay));
      }
    }
  }
  return NULL;
}

std::string
HSFunctionTransferCargo(DBRef aCaller,
                        std::string aSource,
                        std::string aDestination,
                        std::string aCommodity,
                        std::string aAmount)
{
  HSCargoContainer *source = FindContainer(aCaller, aSource);
  HSCargoContainer *dest = FindContainer(aCaller, aDestination);
  if (!source) {
    return "#-2 SOURCE NOT FOUND";
  }
  if (!dest) {
    return "#-3 DESTINATION NOT FOUND";
  }

  HSCommodity *commod = sHSDB.FindCommodity(atoi(aCommodity.c_str()));
  if (!commod) {
    return "#-4 COMMODITY NOT FOUND";
  }
  int amount = atoi(aAmount.c_str());

  if(amount < 0) {
    return "#-7 TRANSFER AMOUNT MUST BE POSITIVE";
  }

  if (source->GetCargoForCommodity(commod->GetID()) < amount) {
    return "#-5 INSUFFICIENT IN SOURCE";
  }
  if (dest->GetFreeSpace() < amount) {
    return "#-6 INSUFFICIENT ROOM IN DEST";
  }
  dest->SetCargoForCommodity(commod->GetID(), 
    dest->GetCargoForCommodity(commod->GetID()) + amount);
  source->SetCargoForCommodity(commod->GetID(),
    source->GetCargoForCommodity(commod->GetID()) - amount);
  return "Cargo transferred";
}

std::string
HSFunctionSetCargo(DBRef aCaller,
                   std::string aContainer,
                   std::string aCommodity,
                   std::string aAmount)
{
  HSCargoContainer *container = FindContainer(aCaller, aContainer);

  if (!container) {
    return "#-2 CONTAINER NOT FOUND";
  }
  HSCommodity *commod = sHSDB.FindCommodity(atoi(aCommodity.c_str()));
  if (!commod) {
    return "#-3 COMMODITY NOT FOUND";
  }
  container->SetCargoForCommodity(commod->GetID(), atoi(aAmount.c_str()));
  return "Cargo set";
}

std::string
HSFunctionGetCargo(DBRef aCaller,
                   std::string aContainer,
                   std::string aCommodity)
{
  HSCargoContainer *container = FindContainer(aCaller, aContainer);

  if (!container) {
    return "#-2 CONTAINER NOT FOUND";
  }
  HSCommodity *commod = sHSDB.FindCommodity(atoi(aCommodity.c_str()));
  if (!commod) {
    return "#-3 COMMODITY NOT FOUND";
  }
  char tbuf[64];
  sprintf(tbuf, "%d", (int)container->GetCargoForCommodity(commod->GetID()));
  return tbuf;
}

std::string
HSFunctionIsObject(DBRef aCaller,
                   std::string aObject,
                   std::string aType)
{
  DBRef objRef = HSIFLocateObject(aCaller, aObject, OT_THING);

  if (objRef == DBRefNothing) {
    return "#-1 OBJECT NOT FOUND";
  }

  if (!strncasecmp(aType.c_str(), "OBJECT")) {
    if (sHSDB.FindObject(objRef)) {
      return "1";
    } else {
      return "0";
    }
  } else if (!strncasecmp(aType.c_str(), "CONSOLE")) {
    if (sHSDB.FindConsole(objRef)) {
      return "1";
    } else {
      return "0";
    }
  } else {
    return "#-2 INVALID TYPE";
  }
}

std::string
HSFunctionNew(DBRef aCaller, std::string aType, std::string aArgument, std::string aClass)
{
  if (!strncasecmp(aType.c_str(), "WAREHOUSE")) {
    HSWarehouse *warehouse = new HSWarehouse();

    if (!aArgument.empty()) {
      warehouse->SetName(aArgument);
    } else {
      warehouse->SetName("Unnamed Warehouse");
    }
    int id = sHSDB.AddWarehouse(warehouse);

    if (id < 0) {
      return "#-2 FAILED";
    } else {
      char tbuf[16];
      sprintf(tbuf, "%d", id);
      return tbuf;
    }
  } else if (!strncasecmp(aType.c_str(), "COMMODITY")) {
    HSCommodity *commodity = new HSCommodity();

    if (!aArgument.empty()) {
      commodity->SetName(aArgument);
    } else {
      commodity->SetName("Unnamed Commodity");
    }
    int id = sHSDB.AddCommodity(commodity);

    if (id < 0) {
      return "#-2 FAILED";
    } else {
      char tbuf[16];
      sprintf(tbuf, "%d", id);
      return tbuf;      
    }
  } else if (!strncasecmp(aType.c_str(), "SYSTEM")) {
    if (aArgument.empty()) {
      return "#-3 NO SYSTEM TYPE SPECIFIED";
    }
    HSSystem *system = HSSystem::CreateSystemForType((HSSystemType)atoi(aArgument.c_str()));

    system->SetName(std::string("Unnamed ")
      .append(HSSystem::GetNameForType((HSSystemType)system->GetType())));
    int id = sHSDB.AddSystem(system);

    if (id < 0) {
      return "#-2 FAILED";
    } else {
      char tbuf[16];
      sprintf(tbuf, "%d", id);
      return tbuf;
    }
  } else {
    return "#-1 INVALID TYPE";
  }
}

std::string
HSFunctionSRep(DBRef aCaller, std::string aShip, std::string aType)
{
  std::stringstream notification;

  DBRef objRef = HSIFLocateObject(aCaller, aShip, OT_THING);

  if (objRef == DBRefNothing) {
    return "#-1 OBJECT NOT FOUND";
  }

  HSObject *object = sHSDB.FindObject(objRef);

  if (!object || object->GetType() != HSOT_SHIP) {
    return "#-2 NOT A SHIP";
  }

  HSShip *ship = static_cast<HSShip*>(object);

  HSComputerInstance *computer = ship->FindComputer();

  if (!computer) {
    return "#-3 NOT EQUIPPED WITH COMPUTER";
  }
  int typeNum = -1;
  if (!aType.empty()) {
    if (from_string<int>(typeNum, aType)) {
      if (typeNum < 0 || typeNum >= sObjectTypeCount) {
        return "#-4 INVALID TYPE_SPECIFIED";
      }
      aType = typeNum;
    }
  }

  foreach(HSSensorContact, contact, computer->GetSensorContacts()) {
    ASSERT(contact.Object);
    if (typeNum >= 0) {
      if (contact.Object->GetType() != typeNum) {
        continue;
      }
    }
    if (!notification.str().empty()) {
      notification << "|";
    }
    notification << contact.Object->GetType() << ":#" << contact.Object->GetID() << ":" << contact.ID << ":" <<
      contact.Detail << ":" << contact.DiffVector.length() << ":" << contact.DiffVector.HeadingString();

    if (contact.Object->GetType() == HSOT_SHIP) {
      HSShip *contactShip = static_cast<HSShip*>(contact.Object);
      notification << ":" << contactShip->GetHeading().HeadingString();
    }
  }
  return notification.str();
}
