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
#include "HSIface.h"
#include "HSCommand.h"
#include "HSFunctions.h"
#include "HSDB.h"
#include "HSConsole.h"
#include "HSVersion.h"
#include "HSTools.h"

#ifdef HS_SERVER
#include "HSServer.h"
#endif

extern "C" {

#include "config.h"
#include "conf.h"
#include "externs.h"
#include "parse.h"
#include "command.h"

void HSInitialize()
{
  sHSDB.Initialize();
  sHSDB.LoadDatabase();
  HSLog() << "Initializing HSpace " << GetVersionString();
#ifdef HS_SERVER
  sServer = new HSServer();
  sServer->Start();
#endif
}

void HSShutdown()
{
#ifdef HS_SERVER
  sServer->Stop();
  delete sServer;
#endif
  sHSDB.Shutdown();
}

void HSDumpDatabase()
{
  sHSDB.DumpDatabase();
}

void HSCycle()
{
  sHSDB.Cycle();
#ifdef HS_SERVER
  sServer->UpdateClients();
#endif
}

void HSObjectDestroyed(dbref aObject)
{
  sHSDB.ObjectDestroyed(aObject);
}

FUNCTION(hsf_info)
{
  HSIFNotify(executor, "This is the first Testing Incarnation of HSpace.");
}

FUNCTION(hsf_console_cmd)
{
  HSConsole *console = sHSDB.FindConsole(executor);
  if (!console) {
    HSIFNotify(executor, "You are not a valid console.");
    return;
  }

  if (!nargs) {
    HSIFNotify(executor, "You atleast need to specify a command.");
    return;
  }

  std::string *strArgs = new std::string[nargs - 1];

  for (int i = 1; i < nargs; i++) {
    strArgs[i - 1] = args[i];
  }

  console->ExecuteCommand(args[0], strArgs, nargs - 1);
}

FUNCTION(hsf_set)
{
  safe_str(HSFunctionSet(executor, args[0], args[1], args[2], args[3]).c_str(), buff, bp);
}

FUNCTION(hsf_get)
{
  safe_str(HSFunctionGet(executor, args[0], args[1], args[2]).c_str(), buff, bp);
}

FUNCTION(hsf_shipsys_set)
{
  if (nargs == 3) {
    safe_str(HSFunctionShipSysSet(executor, args[0], args[1], args[2], NULL).c_str(), buff, bp);
  } else if (nargs == 4) {
    std::string val = args[3];
    safe_str(HSFunctionShipSysSet(executor, args[0], args[1], args[2], &val).c_str(), buff, bp);
  }
}

FUNCTION(hsf_shipsys_get)
{
  safe_str(HSFunctionShipSysGet(executor, args[0], args[1], args[2]).c_str(), buff, bp);
}

FUNCTION(hsf_clone)
{
  safe_str(HSFunctionClone(executor, args[0]).c_str(), buff, bp);
}

FUNCTION(hsf_systems)
{
  if (nargs == 1) {
    safe_str(HSFunctionSystems(executor, args[0], std::string()).c_str(), buff, bp);
  } else if (nargs == 2) {
    safe_str(HSFunctionSystems(executor, args[0], args[1]).c_str(), buff, bp);
  }
}

FUNCTION(hsf_list)
{
  if (nargs == 1) {
    safe_str(HSFunctionList(executor, args[0], NULL).c_str(), buff, bp);
  } else if (nargs == 2) {
    std::string val = args[1];
    safe_str(HSFunctionList(executor, args[0], &val).c_str(), buff, bp);
  }
}

FUNCTION(hsf_placesystem)
{
  safe_str(HSFunctionPlaceSystem(
    executor, args[0], args[1], atoi(args[2]) ? true : false).c_str(), buff, bp);
}

FUNCTION(hsf_removesystem)
{
  safe_str(HSFunctionRemoveSystem(executor, args[0], args[1]).c_str(), buff, bp);
}

FUNCTION(hsf_cargo_transfer)
{
  safe_str(HSFunctionTransferCargo(
    executor, args[0], args[1], args[2], args[3]).c_str(), buff, bp);
}

FUNCTION(hsf_cargo_get)
{
  safe_str(HSFunctionGetCargo(executor, args[0], args[1]).c_str(), buff, bp);
}

FUNCTION(hsf_cargo_set)
{
  safe_str(HSFunctionSetCargo(executor, args[0], args[1], args[2]).c_str(), buff, bp);
}

FUNCTION(hsf_is_object)
{
  safe_str(HSFunctionIsObject(executor, args[0], args[1]).c_str(), buff, bp);
}

FUNCTION(hsf_new)
{
  if (nargs == 1) {
    safe_str(HSFunctionNew(executor, args[0], std::string(), std::string()).c_str(), buff, bp);
  } else if (nargs == 2) {
    safe_str(HSFunctionNew(executor, args[0], args[1], std::string()).c_str(), buff, bp);
  } else if (nargs == 3) {
    safe_str(HSFunctionNew(executor, args[0], args[1], args[2]).c_str(), buff, bp);
  }
}

FUNCTION(hsf_srep)
{
  if (nargs == 1) {
    safe_str(HSFunctionSRep(executor, args[0], std::string()).c_str(), buff, bp);
  } else if (nargs == 2) {
    safe_str(HSFunctionSRep(executor, args[0], args[1]).c_str(), buff, bp);
  }
}

COMMAND(hsc_space)
{
  std::string sSwitches;
  std::string sArgs;
  if (switches) {
    sSwitches = switches;
  }
  if (args_raw) {
    sArgs = args_raw;
  }
  HSCommandSpace(executor, sSwitches, sArgs);
}

COMMAND(hsc_man)
{
  std::string sArgs;
  if (args_raw) {
    sArgs = args_raw;
  }
  HSCommandMan(executor, sArgs);
}

COMMAND(hsc_unman)
{
  HSCommandUnman(executor);
}

COMMAND(hsc_board)
{
  std::string sArgs;
  if (args_raw) {
    sArgs = args_raw;
  }
  HSCommandBoard(executor, sArgs);
}

COMMAND(hsc_disembark)
{
  HSCommandDisembark(executor);
}

}
