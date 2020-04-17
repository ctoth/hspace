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
#include "HSConsole.h"
#include "HSSystem.h"
#include "HSTools.h"
#include "HSComputer.h"
#include "HSShip.h"
#include "HSReactor.h"
#include "HSCommunications.h"
#include "HSAutopilot.h"
#include "HSDimensionalDrive.h"
#include "HSConf.h"
#include "HSCargoPod.h"
#include "HSCargoBay.h"
#include "HSDB.h"
#include "HSUniverse.h"
#include "HSRepairSystem.h"

#include <sstream>

using namespace std;

HSConsole::HSConsole(void)
  : mID(0)
  , mShipID(0)
  , mMessageMask(HS_ALL_MSGS)
{
  ADD_ATTRIBUTE_INTERNAL("ID", AT_INTEGER, mID)
  ADD_ATTRIBUTE_INTERNAL("SHIPID", AT_INTEGER, mShipID)
}

HSConsole::~HSConsole(void)
{
}

void
HSConsole::ExecuteCommand(std::string aCommand, std::string aArgs[], int aCount)
{
  if (!mShip->GetCurrentHullPoints()) {
    HSIFNotify(GetUser(), "This ship is destroyed.");
    return;
  }
  if (!strncasecmp(aCommand.c_str(), "STATUS")) {
    GiveStatus(aCommand);    
  } else if (!strncasecmp(aCommand.c_str(), "NAVREP")) {
    GiveStatus(aCommand);
  } else if (!strncasecmp(aCommand.c_str(), "SETOUTPUT")) {
    if (aCount < 2) {
      return;
    }
    SetOutput(aArgs[0], aArgs[1]);
  } else if (!strncasecmp(aCommand.c_str(), "ALLOCATE")) {
    if (aCount < 2) {
      return;
    }
    Allocate(aArgs[0], aArgs[1]);
  } else if (!strncasecmp(aCommand.c_str(), "SREP")) {
    GiveSensorReport();  
  } else if (!strncasecmp(aCommand.c_str(), "BOOT")) {
    BootComputer();
  } else if (!strncasecmp(aCommand.c_str(), "POWER")) {
    GiveStatus(aCommand);
  } else if (!strncasecmp(aCommand.c_str(), "HSTAT")) {
    HSComputerInstance *computer = VerifyComputer();
    if (computer) {
      computer->DisplayHatchReport(this);
    }
  } else if (!strncasecmp(aCommand.c_str(), "CONNECT")) {
    if (aCount < 3) {
      return;
    }
    HSComputerInstance *computer = VerifyComputer();
    if (computer) {
      computer->ConnectHatch(this, aArgs[0], aArgs[1], aArgs[2]);
    }
  } else if (!strncasecmp(aCommand.c_str(), "DISCONNECT")) {
    if (aCount < 1) {
      return;
    }
    HSComputerInstance *computer = VerifyComputer();
    if (computer) {
      computer->DisconnectHatch(this, aArgs[0]);
    }
  } else if (!strncasecmp(aCommand.c_str(), "SETSPEED")) {
    if (aCount < 1) {
      return;
    }
    SetSpeed(atof(aArgs[0].c_str()));
  } else if (!strncasecmp(aCommand.c_str(), "CUTENGINES")) {
    CutEngines();
  } else if (!strncasecmp(aCommand.c_str(), "LAND")) {
    if (aCount < 2) {
      return;
    }
    Land(atoi(aArgs[0].c_str()), atoi(aArgs[1].c_str()));
  } else if (!strncasecmp(aCommand.c_str(), "LAUNCH")) {
    Launch();
  } else if (!strncasecmp(aCommand.c_str(), "SETHEADING")) {
    if (aCount < 2) {
      return;
    }
    SetHeading(atoi(aArgs[0].c_str()), atoi(aArgs[1].c_str()));
  } else if (!strncasecmp(aCommand.c_str(), "TUNE")) {
    if (aCount < 1) {
      return;
    }
    if (aCount == 1) {
      Tune(atof(aArgs[0].c_str()), "");
    } else {
      Tune(atof(aArgs[0].c_str()), aArgs[1]);
    }
  } else if (!strncasecmp(aCommand.c_str(), "SENDMESSAGE")) {
    if (aCount < 2) {
      return;
    }
    SendMessage(atoi(aArgs[0].c_str()), aArgs[1]);
  } else if (!strncasecmp(aCommand.c_str(), "SETFREQUENCY")) {
    if (aCount < 1) {
      return;
    }
    SetFrequency(atof(aArgs[0].c_str()));
  } else if (!strncasecmp(aCommand.c_str(), "BROADCAST")) {
    if (aCount < 1) {
      return;
    }
    Broadcast(aArgs[0]);
  } else if (!strncasecmp(aCommand.c_str(), "SYSSTAT")) {
    if (aCount < 1) {
      return;
    }
    SysStat(aArgs[0]);
  } else if (!strncasecmp(aCommand.c_str(), "LOCK")) {
    if (aCount < 2) {
      return;
    }
    LockWeapons(aArgs[0], aArgs[1]);
  } else if (!strncasecmp(aCommand.c_str(), "FIRE")) {
    if (aCount < 1) {
      return;
    }
    FireWeapons(aArgs[0]);
  } else if (!strncasecmp(aCommand.c_str(), "SCAN")) {
    if (aCount < 1) {
      return;
    }
    Scan(aArgs[0]);
  } else if (!strncasecmp(aCommand.c_str(), "SETDESTINATION")) {
    if (aCount < 3) {
      return;
    }
    SetDestination(atof(aArgs[0].c_str()), atof(aArgs[1].c_str()), atof(aArgs[2].c_str()));
  } else if (!strncasecmp(aCommand.c_str(), "INTERCEPT")) {
    if (aCount < 1) {
      return;
    }
    Intercept(aArgs[0]);
  } else if (!strncasecmp(aCommand.c_str(), "AUTOPILOT")) {
    if (aCount < 1) {
      return;
    }
    Autopilot(atoi(aArgs[0].c_str()) ? true : false);
  } else if (!strncasecmp(aCommand.c_str(), "CARGOREP")) {
    HSComputerInstance *computer = VerifyComputer();
    if (computer) {
      computer->DisplayCargoReport(this);
    }
  } else if (!strncasecmp(aCommand.c_str(), "GATE")) {
    if (aCount < 1) {
      return;
    }
    HSComputerInstance *computer = VerifyComputer();
    if (computer) {
      computer->Gate(this, aArgs[0]);
    }
  } else if (!strncasecmp(aCommand.c_str(), "TAXI")) {
    if (aCount < 1) {
      return;
    }
    Taxi(aArgs[0]);
  } else if (!strncasecmp(aCommand.c_str(), "VIEW")) {
    if (mShip->GetLandedLoc() == DBRefNothing || mShip->GetLandingTimer()) {
      NotifyUserSimple("You are not landed.");
      return;
    }
    NotifyUser("You look outside and see:");
    HSIFLook(GetUser(), mShip->GetLandedLoc());
  } else if (!strncasecmp(aCommand.c_str(), "HARVEST")) {
    if (aCount < 3) {
      return;
    }
    HSComputerInstance *computer = VerifyComputer();
    if (computer) {
      computer->Harvest(this, aArgs[0], aArgs[1], aArgs[2]);
    }
  } else if (!strncasecmp(aCommand.c_str(), "ABORTHARVEST")) {
    if (aCount < 1) {
      return;
    }
    HSComputerInstance *computer = VerifyComputer();
    if (computer) {
      computer->AbortHarvest(this, aArgs[0]);
    }
  } else if (!strncasecmp(aCommand.c_str(), "ETA")) {
    ETA();
  } else if (!strncasecmp(aCommand.c_str(), "JETTISON")) {
    if (aCount < 1) {
      return;
    }
    Jettison(aArgs[0]);
  } else if (!strncasecmp(aCommand.c_str(), "COLLECT")) {
    if (aCount < 1) {
      return;
    }
    HSComputerInstance *computer = VerifyComputer();
    if (computer) {
      computer->Collect(this, aArgs[0]);
    }
  } else if (!strncasecmp(aCommand.c_str(), "ENGAGE")) {
    Engage();
  } else if (!strncasecmp(aCommand.c_str(), "DISENGAGE")) {
    Disengage();
  } else if (!strncasecmp(aCommand.c_str(), "EXTREPAIR")) {
    if (aCount < 3) {
      return;
    }
    HSSystemInstance *system = mShip->FindSystemByName(aArgs[2]);
    if (!system || system->GetType() != HSST_REPAIRSYSTEM) {
      NotifyUserSimple("No such repair system found.");
      return;
    }
    static_cast<HSRepairSystemInstance*>(system)->AssignToSystemOn(this, aArgs[1], aArgs[0]);
  } else if (!strncasecmp(aCommand.c_str(), "EXTSTOPREPAIR")) {
    if (aCount < 3) {
      return;
    }
    HSSystemInstance *system = mShip->FindSystemByName(aArgs[2]);
    if (!system || system->GetType() != HSST_REPAIRSYSTEM) {
      NotifyUserSimple("No such repair system found.");
      return;
    }
    static_cast<HSRepairSystemInstance*>(system)->UnassignFromSystemOn(this, aArgs[1], aArgs[0]);
  } else if (!strncasecmp(aCommand.c_str(), "REPAIR")) {
    if (aCount < 2) {
      return;
    }
    HSSystemInstance *system = mShip->FindSystemByName(aArgs[1]);
    if (!system || system->GetType() != HSST_REPAIRSYSTEM) {
      NotifyUserSimple("No such repair system found.");
      return;
    }
    static_cast<HSRepairSystemInstance*>(system)->AssignToSystem(this, aArgs[0]);
  } else if (!strncasecmp(aCommand.c_str(), "STOPREPAIR")) {
    if (aCount < 2) {
      return;
    }
    HSSystemInstance *system = mShip->FindSystemByName(aArgs[1]);
    if (!system || system->GetType() != HSST_REPAIRSYSTEM) {
      NotifyUserSimple("No such repair system found.");
      return;
    }
    static_cast<HSRepairSystemInstance*>(system)->UnassignFromSystem(this, aArgs[0]);
  } else if (!strncasecmp(aCommand.c_str(), "REPSTAT")) {
    if (aCount < 1) {
      return;
    }
    HSSystemInstance *system = mShip->FindSystemByName(aArgs[0]);
    if (!system || system->GetType() != HSST_REPAIRSYSTEM) {
      NotifyUserSimple("No such repair system found.");
      return;
    }
    static_cast<HSRepairSystemInstance*>(system)->GiveReport(this);
  } else if (!strncasecmp(aCommand.c_str(), "SPOSE")) {
    if (aCount < 1) {
      return;
    }
    HSUniverse *univ = sHSDB.FindUniverse(mShip->GetUniverse());
    if (!univ) {
      HSLog() << mShip << "Invalid universe found.";
      return;
    }
    univ->NotifyObjectPose(mShip, "Sensors", std::string(" ").append(aArgs[0]));
    NotifyUserSimple("Ship pose sent.");
  }
}

void
HSConsole::NotifyUser(std::string aMsg)
{
  DBRef user = GetUser();
  if (user != DBRefNothing) {
    HSIFNotify(user, aMsg);
  }
}

void
HSConsole::NotifyUserSimple(string aMsg)
{
  stringstream msg;
  msg << ANSI_GREEN << "> " << ANSI_NORMAL << aMsg;
  HSIFNotify(GetUser(), msg.str());
}

void
HSConsole::GiveStatus(std::string aCommand)
{
  HSComputerInstance *computer = mShip->FindComputer();

  if (!computer) {
    NotifyUserSimple("NO COMPUTER SYSTEM INSTALLED, LIMITED INPUT POSSIBLE");
    NotifyUserSimple("");
    return;
  }
  stringstream message;
  if (!computer->IsOnline()) {
    message << "MAIN COMPUTER SYSTEMS " << ANSI_HILITE << ANSI_RED << "OFFLINE" << ANSI_NORMAL;
    NotifyUserSimple(message.str());
    NotifyUserSimple("");
    return;
  }

  if (!strncasecmp(aCommand.c_str(), "STATUS")) {
    computer->DisplayStatus(this);
  } else if (!strncasecmp(aCommand.c_str(), "POWER")) {
    computer->DisplayPowerStatus(this);
  } else if (!strncasecmp(aCommand.c_str(), "NAVREP")) {
    computer->DisplayNavigationReport(this);
  }
}

void
HSConsole::SetOutput(std::string aSystemName, std::string aOutput)
{
  /* Potential crash bug if you shutdown the reactor while landing
   * or launching.  In general its a bad idea to allow it anyways. */
  if (mShip->GetLandingTimer() != 0) {
    NotifyUserSimple("You may not adjust output while landing/launching!");
    return;
  }

  if (aSystemName.empty() || aOutput.empty()) {
    NotifyUserSimple("A valid system name and output must be specified.");
    return;
  }

  std::vector<HSSystemInstance*> reactors;
  if (!strncasecmp(aSystemName.c_str(), "ALL")) {
    reactors = mShip->FindSystemsByType(HSST_REACTOR);

    if (!reactors.size()) {
      NotifyUserSimple("No power systems found on this ship.");
      return;
    }
  } else {
    HSSystemInstance *system = mShip->FindSystemByName(aSystemName);

    if (!system || system->GetType() != HSST_REACTOR) {
      NotifyUserSimple("No power system found by that name");
      return;
    }
    reactors.push_back(system);
  }

  foreach(HSSystemInstance*, sys, reactors) {

    HSReactorInstance *reactorInst = static_cast<HSReactorInstance*>(sys);

    string percentile = aOutput.substr(aOutput.length() - 1, aOutput.length() - 1);
    int output;
    if (percentile == "%") {
      int powerPerc = atoi(aOutput.substr(0, aOutput.length() - 1).c_str());
      if (powerPerc <= 0) {
        output = 0;
      } else {
        output = (int)((double)reactorInst->GetMaxOutput() * ((double)powerPerc / 100.00));
      }

    } else {
      output = atoi(aOutput.c_str());
    }
    if (reactorInst->GetMaxOutput() * (1.0 + reactorInst->GetMaxOverload()) * reactorInst->GetCondition() < output) {
      NotifyUserSimple("Power system cannot deliver that much power");
      return;
    }
    reactorInst->SetDesiredOutput(output);

    if (output > 0) {
      // A reactor's output has been set > 0, ship is online for sure.
      mShip->SetOnline(true);
    }

    char tbuf[128];
    sprintf(tbuf, "%s output set to %s", reactorInst->GetName().c_str(), HSReactor::PowerString(output).c_str());
    NotifyUserSimple(tbuf);
  }
}

void
HSConsole::Allocate(std::string aSystemName, std::string aPower)
{
  if (aPower.empty() || aSystemName.empty()) {
    NotifyUserSimple("A valid systemname and amount of power need to be specified.");
    return;
  }

  std::vector<HSSystemInstance*> systems;

  if (!strncasecmp(aSystemName.c_str(), "ALL")) {
    foreach(HSSystemInstance*, sys, mShip->GetSystems()) {
      if (sys->GetOptimalPower() > 0 && sys->GetType() != HSST_COMPUTER) {
        systems.push_back(sys);
      }
    }
  } else {
    if ((int)aSystemName.find("/") == -1) {
      systems = mShip->FindSystemsByName(aSystemName);
    } else {
      HSSystemInstance* sys = mShip->FindSystemByName(aSystemName);
      if (sys) {
        systems.push_back(sys);
      }
    }
  }
  if (!systems.size()) {
    NotifyUserSimple("No such system found.");
    return;
  }

  foreach (HSSystemInstance*, system, systems) {
    if (system->GetCondition() <= 0) {
      NotifyUserSimple("That system is inoperable.");
      continue;
    }
    if (!system || system->GetType() == HSST_REACTOR || system->GetType() == HSST_COMPUTER) {
      NotifyUserSimple("You cannot allocate power to that system");
      continue;
    }

    string percentile = aPower.substr(aPower.length() - 1, aPower.length() - 1);
    int power;
    if (percentile == "%") {
      int powerPerc = atoi(aPower.substr(0, aPower.length() - 1).c_str());
      if (powerPerc <= 0) {
        power = 0;
      } else {
        power = (int)((double)system->GetOptimalPower() * ((double)powerPerc / 100.00));
      }

    } else {
      power = atoi(aPower.c_str());
    }
    if (power <= 0) {
      power = 0;
    }

    if ((mShip->GetAvailablePower() + system->GetCurrentPower()) < power) {
      NotifyUserSimple("Not enough power available");
      return;
    }

    char tbuf[256];
    if (power > (int)((double)system->GetOptimalPower() * (1.0f + system->GetMaxOverload()))) {
      sprintf(tbuf, "Cannot allocate %s of power to %s",
        HSReactor::PowerString(power).c_str(), system->GetName().c_str());
      NotifyUserSimple(tbuf);
      continue;
    }

    system->SetCurrentPower(power);
    sprintf(tbuf, "%s of power allocated to %s",
      HSReactor::PowerString(power).c_str(), system->GetName().c_str());
    mShip->NotifyConsolesFormatted("Power", tbuf);
  }
}

void
HSConsole::BootComputer()
{
  HSComputerInstance *computer = mShip->FindComputer();

  if (computer->IsOnline()) {
    NotifyUserSimple("COMPUTER ALREADY ONLINE");
    return;
  }

  if (mShip->GetAvailablePower() < computer->GetOptimalPower()) {
    NotifyUserSimple("INSUFFICIENT POWER AVAILABLE");
    return;
  }

  computer->SetCurrentPower(computer->GetOptimalPower());
  NotifyUserSimple("COMPUTER SYSTEM BOOTING");
  mShip->NotifySRooms("Around you terminals light up as the ship's main computer powers up");
}

void
HSConsole::GiveSensorReport()
{
  HSComputerInstance *computer = mShip->FindComputer();

  if (!computer) {
    NotifyUserSimple("NO COMPUTER SYSTEM INSTALLED, LIMITED INPUT POSSIBLE");
    NotifyUserSimple("");
    return;
  }
  stringstream message;
  if (!computer->IsOnline()) {
    message << "MAIN COMPUTER SYSTEMS " << ANSI_HILITE << ANSI_RED << "OFFLINE" << ANSI_NORMAL;
    NotifyUserSimple(message.str());
    NotifyUserSimple("");
    return;
  }

  computer->DisplaySensorReport(this);
}

void
HSConsole::SetSpeed(double aSpeed)
{
  HSComputerInstance *computer = mShip->FindComputer();

  if (!computer) {
    NotifyUserSimple("NO COMPUTER SYSTEM INSTALLED, LIMITED INPUT POSSIBLE");
    NotifyUserSimple("");
    return;
  }
  stringstream message;
  if (!computer->IsOnline()) {
    message << "MAIN COMPUTER SYSTEMS " << ANSI_HILITE << ANSI_RED << "OFFLINE" << ANSI_NORMAL;
    NotifyUserSimple(message.str());
    NotifyUserSimple("");
    return;
  }

  char tbuf[128];
  if (aSpeed > mShip->GetMaxVelocity()) {
    sprintf(tbuf, "Maximum velocity for this vessel is %s/s",
      HSObject::DistanceString(mShip->GetMaxVelocity()).c_str());
    NotifyUserSimple(tbuf);
    return;
  }
  if (aSpeed < 0) {
    if (sHSConf.RevertThrust > 0.0) {
      if (aSpeed < (-mShip->GetMaxVelocity() * sHSConf.RevertThrust)) {
        sprintf(tbuf, "Maximum reverse velocity for this vessel is %s/s",
          HSObject::DistanceString(mShip->GetMaxVelocity() * sHSConf.RevertThrust).c_str());
        NotifyUserSimple(tbuf);
        return;
      }
      mShip->SetDesiredVelocity(aSpeed);
      mShip->SetAccelerating(true);
      mShip->SetEnginesCut(false);
      sprintf(tbuf, "Ship set to accelerate to -%s/s",
        HSObject::DistanceString(-aSpeed).c_str());
      mShip->NotifyConsolesFormatted("Engines", tbuf);
    } else {
      NotifyUserSimple("You cannot fly backward.");
      return;
    }
  } else {
    mShip->SetDesiredVelocity(aSpeed);
    mShip->SetAccelerating(true);
    mShip->SetEnginesCut(false);
    sprintf(tbuf, "Ship set to accelerate to %s/s",
      HSObject::DistanceString(aSpeed).c_str());
    mShip->NotifyConsolesFormatted("Engines", tbuf);
  }
}

void
HSConsole::SetHeading(int aHeadingXY, int aHeadingZ)
{
  HSComputerInstance *computer = mShip->FindComputer();

  if (!computer) {
    NotifyUserSimple("NO COMPUTER SYSTEM INSTALLED, LIMITED INPUT POSSIBLE");
    NotifyUserSimple("");
    return;
  }
  stringstream message;
  if (!computer->IsOnline()) {
    message << "MAIN COMPUTER SYSTEMS " << ANSI_HILITE << ANSI_RED << "OFFLINE" << ANSI_NORMAL;
    NotifyUserSimple(message.str());
    NotifyUserSimple("");
    return;
  }

  char tbuf[128];
  if (aHeadingZ > 90 || aHeadingZ < -90) {
    NotifyUserSimple("Invalid heading");
    return;
  }
  mShip->SetAccelerating(true);
  mShip->SetDesiredHeading(HSVector3D(aHeadingXY, aHeadingZ));
  sprintf(tbuf, "Ship rotating to %s",
    mShip->GetDesiredHeading().HeadingString().c_str());
  mShip->NotifyConsolesFormatted("Thrusters", tbuf);
}

void
HSConsole::CutEngines()
{
  HSComputerInstance *computer = mShip->FindComputer();

  if (!computer) {
    NotifyUserSimple("NO COMPUTER SYSTEM INSTALLED, LIMITED INPUT POSSIBLE");
    NotifyUserSimple("");
    return;
  }
  stringstream message;
  if (!computer->IsOnline()) {
    message << "MAIN COMPUTER SYSTEMS " << ANSI_HILITE << ANSI_RED << "OFFLINE" << ANSI_NORMAL;
    NotifyUserSimple(message.str());
    NotifyUserSimple("");
    return;
  }

  mShip->NotifyConsolesFormatted("Engines", "Engine activity stopped");
  mShip->SetEnginesCut(true);
}

void
HSConsole::Land(int aContact, int aLocation)
{
  HSComputerInstance *computer = mShip->FindComputer();

  if (!computer) {
    NotifyUserSimple("NO COMPUTER SYSTEM INSTALLED, LIMITED INPUT POSSIBLE");
    NotifyUserSimple("");
    return;
  }
  stringstream message;
  if (!computer->IsOnline()) {
    message << "MAIN COMPUTER SYSTEMS " << ANSI_HILITE << ANSI_RED << "OFFLINE" << ANSI_NORMAL;
    NotifyUserSimple(message.str());
    NotifyUserSimple("");
    return;
  }

  computer->Land(this, aContact, aLocation);
}

void
HSConsole::Launch()
{
  HSComputerInstance *computer = mShip->FindComputer();

  if (!computer) {
    NotifyUserSimple("NO COMPUTER SYSTEM INSTALLED, LIMITED INPUT POSSIBLE");
    NotifyUserSimple("");
    return;
  }
  stringstream message;
  if (!computer->IsOnline()) {
    message << "MAIN COMPUTER SYSTEMS " << ANSI_HILITE << ANSI_RED << "OFFLINE" << ANSI_NORMAL;
    NotifyUserSimple(message.str());
    NotifyUserSimple("");
    return;
  }

  computer->Launch(this);
}

void
HSConsole::Tune(double aFrequency, std::string aEncryptKey)
{
  HSCommunicationsInstance *commSystem = FindCommSystem();
  if (!commSystem) {
    NotifyUserSimple("No communications system installed.");
    return;
  }
  if (!commSystem->GetCurrentPower()) {
    NotifyUserSimple("Communications system powered down.");
    return;
  }
  commSystem->AddChannel(this, aFrequency, aEncryptKey);
}

void
HSConsole::SetFrequency(double aFrequency)
{
  HSCommunicationsInstance *commSystem = FindCommSystem();
  if (!commSystem) {
    NotifyUserSimple("No communications system installed.");
    return;
  }
  if (!commSystem->GetCurrentPower()) {
    NotifyUserSimple("Communications system powered down.");
    return;
  }
  commSystem->SetFrequency(this, aFrequency);
}

void
HSConsole::SendMessage(int aContact, std::string aMessage)
{
  HSCommunicationsInstance *commSystem = FindCommSystem();
  if (!commSystem) {
    NotifyUserSimple("No communications system installed.");
    return;
  }
  if (!commSystem->GetCurrentPower()) {
    NotifyUserSimple("Communications system powered down.");
    return;
  }
  commSystem->SendMessage(this, aContact, aMessage);
}

void
HSConsole::Broadcast(std::string aMessage)
{
  HSCommunicationsInstance *commSystem = FindCommSystem();
  if (!commSystem) {
    NotifyUserSimple("No communications system installed.");
    return;
  }
  if (!commSystem->GetCurrentPower()) {
    NotifyUserSimple("Communications system powered down.");
    return;
  }

  commSystem->SendMessageChannel(this, aMessage);
}

void
HSConsole::SysStat(std::string aSystem)
{
  HSComputerInstance *computer = mShip->FindComputer();

  if (!computer) {
    NotifyUserSimple("NO COMPUTER SYSTEM INSTALLED, LIMITED INPUT POSSIBLE");
    NotifyUserSimple("");
    return;
  }
  stringstream message;
  if (!computer->IsOnline()) {
    message << "MAIN COMPUTER SYSTEMS " << ANSI_HILITE << ANSI_RED << "OFFLINE" << ANSI_NORMAL;
    NotifyUserSimple(message.str());
    NotifyUserSimple("");
    return;
  }

  computer->DisplaySystemStatusReport(this, aSystem);
}

void
HSConsole::LockWeapons(std::string aWeapon, std::string aTarget)
{
  HSComputerInstance *computer = VerifyComputer();

  if (computer) {
    computer->LockWeapons(this, aWeapon, aTarget);
  }
}

void
HSConsole::FireWeapons(std::string aWeapon)
{
  HSComputerInstance *computer = VerifyComputer();

  if (computer) {
    computer->FireWeapons(this, aWeapon);
  }
}

void
HSConsole::Scan(std::string aContact)
{
  HSComputerInstance *computer = VerifyComputer();

  if (computer) {
    computer->DisplayScanReport(this, aContact);
  }
}

void
HSConsole::SetDestination(double aX, double aY, double aZ)
{
  if (!VerifyComputer()) {
    return;
  }
  std::vector<HSSystemInstance*> autopilots =
    mShip->FindSystemsByType(HSST_AUTOPILOT);

  if (autopilots.size() < (unsigned int)1) {
    NotifyUserSimple("No autopilot installed on this vessel");
    return;
  }
  HSAutopilotInstance *autopilot = 
    static_cast<HSAutopilotInstance*>(*autopilots.begin());
  if (autopilot->GetCurrentPower() < autopilot->GetOptimalPower()) {
    NotifyUserSimple("Autopilot is not powered");
    return;
  }
  autopilot->SetDestination(HSVector3D(aX, aY, aZ));
  char tbuf[128];
  sprintf(tbuf, "Destination set to %f %f %f", aX, aY, aZ);
  mShip->NotifyConsolesFormatted("Autopilot", tbuf);
}

void
HSConsole::ETA()
{
  if (!VerifyComputer()) {
    return;
  }
  std::vector<HSSystemInstance*> autopilots =
    mShip->FindSystemsByType(HSST_AUTOPILOT);

  if (autopilots.size() < (unsigned int)1) {
    NotifyUserSimple("No autopilot installed on this vessel");
    return;
  }
  HSAutopilotInstance *autopilot = 
    static_cast<HSAutopilotInstance*>(*autopilots.begin());
  if (autopilot->GetCurrentPower() < autopilot->GetOptimalPower()) {
    NotifyUserSimple("Autopilot is not powered");
    return;
  }
  HSVector3D diffVector = mShip->GetLocation() - autopilot->GetDestination();

  if (mShip->GetVelocity().zero()) {
    NotifyUserSimple("Ship has no motion.");
  } else {
    double etaTime = diffVector.length() / (mShip->HasDDEngaged() ? 
      mShip->GetVelocity().length() * sHSConf.DDMultiplier : mShip->GetVelocity().length());
    char tbuf[128];
    sprintf(tbuf, "ETA at current speed %s.", HSTimeString((int)etaTime).c_str());
    NotifyUserSimple(tbuf);
  }
}

void
HSConsole::Intercept(std::string aContact)
{
  HSComputerInstance *computer = VerifyComputer();

  if (!computer) {
    return;
  }
  computer->Intercept(this, aContact);
}

void
HSConsole::Autopilot(bool aEngage)
{
  if (!VerifyComputer()) {
    return;
  }
  std::vector<HSSystemInstance*> autopilots =
    mShip->FindSystemsByType(HSST_AUTOPILOT);

  if (autopilots.size() < (unsigned int)1) {
    NotifyUserSimple("No autopilot installed on this vessel");
    return;
  }
  HSAutopilotInstance *autopilot = 
    static_cast<HSAutopilotInstance*>(*autopilots.begin());
  if (autopilot->GetCurrentPower() < autopilot->GetOptimalPower()) {
    NotifyUserSimple("Autopilot is not powered");
    return;
  }

  if (aEngage) {
    if (autopilot->GetEngaged()) {
      NotifyUserSimple("Autopilot is already engaged");
      return;
    }
    mShip->NotifyConsolesFormatted("Autopilot", "Autopilot engaging");
    autopilot->SetEngaged(true);
  } else {
    if (!autopilot->GetEngaged()) {
      NotifyUserSimple("Autopilot is not engaged");
      return;
    }
    mShip->NotifyConsolesFormatted("Autopilot", "Autopilot disengaging");
    autopilot->SetEngaged(false);
  }
}

void
HSConsole::Taxi(std::string aExit)
{
  if (!VerifyComputer()) {
    return;
  }

  if (mShip->GetLandedLoc() == DBRefNothing) {
    NotifyUserSimple("You are not landed.");
    return;
  }

  bool hasThrust = false;
  foreach(HSSystemInstance*, system, mShip->FindSystemsByType(HSST_THRUSTERS)) {
    if (system->GetCurrentPower() > 0) {
      hasThrust = true;
      break;
    }
  }

  if (!hasThrust) {
    NotifyUserSimple("No thrusters powered.");
    return;
  }

  DBRef objRef = HSIFLocateNeighbor(mShip->GetID(), aExit, OT_EXIT);

  if (objRef == DBRefNothing) {
    NotifyUserSimple("No such direction to taxi to.");
    return;
  }

  if (!HSIFPassesLock(mShip->GetID(), objRef, HSLock_Basic) ||
    HSIFGetDestination(objRef) == DBRefNothing) {
    NotifyUserSimple("Cannot taxi in that direction.");
    return;
  }
  char tbuf[128];
  sprintf(tbuf, "Taxiing %s.", HSIFGetName(objRef).substr(0,HSIFGetName(objRef).find(";")).c_str());
  mShip->NotifyConsolesFormatted("Taxi", tbuf);
  HSIFTeleportObject(mShip->GetID(), HSIFGetDestination(objRef));
  mShip->SetLandedLoc(HSIFGetDestination(objRef));
  HSIFLook(GetUser(), HSIFGetDestination(objRef));
}

void
HSConsole::Jettison(std::string aBay)
{
  HSSystemInstance *system = mShip->FindSystemByName(aBay);

  if (!system || system->GetType() != HSST_CARGOBAY) {
    NotifyUserSimple("No such cargo bay found.");
    return;
  }
  HSCargoBayInstance *bay = static_cast<HSCargoBayInstance*>(system);

  if (!bay->GetTotalCargo()) {
    NotifyUserSimple("That bay is empty.");
    return;
  }

  HSUniverse *univ = sHSDB.FindUniverse(mShip->GetUniverse());
  if (!univ) {
    HSLog() << "Ship with invalid universe found " << this;
    NotifyUserSimple("Internal error, notify an administrator and see logs.");
    return;
  }

  univ->NotifyObjectPose(mShip, "Cargo", " jettisons cargo into space.");

  HSCargoPod *pod = new HSCargoPod();
  pod->SetUniverse(mShip->GetUniverse());
  pod->SetLocation(mShip->GetLocation());
  pod->SetVelocity(mShip->GetVelocity());
  pod->SetSize((int)((double)bay->GetTotalCargo() * 1.1));
  std::vector<HSCargoItem> newArray;
  pod->SetCargo(bay->GetCargo());
  pod->SetID(sHSDB.FindObjectID());
  pod->SetName("Cargo Pod");
  univ->AddObject(pod);
  bay->SetCargo(newArray);
  char tbuf[256];
  sprintf(tbuf, "All cargo from %s jettisoned.", bay->GetName().c_str());
  mShip->NotifyConsolesFormatted("Cargo", tbuf);
}

void
HSConsole::Engage()
{
  if (!VerifyComputer()) {
    return;
  }

  std::vector<HSSystemInstance*> ddrives =
    mShip->FindSystemsByType(HSST_DIMENSIONALDRIVE);

  if (ddrives.empty()) {
    NotifyUserSimple("No dimensional drive installed.");
    return;
  }

  HSDimensionalDriveInstance *dd =
    static_cast<HSDimensionalDriveInstance*>(*ddrives.begin());

  if (dd->GetEngaged()) {
    NotifyUserSimple("Dimensional drive already engaged.");
    return;
  }

  if (dd->GetCurrentCharge() < dd->GetRequiredCharge()) {
    NotifyUserSimple("Dimensional drive is not charged to the required level.");
    return;
  }

  dd->Engage();
}

void
HSConsole::Disengage()
{
  if (!VerifyComputer()) {
    return;
  }

  std::vector<HSSystemInstance*> ddrives =
    mShip->FindSystemsByType(HSST_DIMENSIONALDRIVE);

  if (ddrives.empty()) {
    NotifyUserSimple("No dimensional drive installed.");
    return;
  }

  HSDimensionalDriveInstance *dd =
    static_cast<HSDimensionalDriveInstance*>(*ddrives.begin());

  if (!dd->GetEngaged()) {
    NotifyUserSimple("Dimensional drive not engaged.");
    return;
  }

  dd->Disengage();
}

HSComputerInstance*
HSConsole::VerifyComputer()
{
  HSComputerInstance *computer = mShip->FindComputer();

  if (!computer) {
    NotifyUserSimple("NO COMPUTER SYSTEM INSTALLED, LIMITED INPUT POSSIBLE");
    NotifyUserSimple("");
    return 0;
  }
  stringstream message;
  if (!computer->IsOnline()) {
    message << "MAIN COMPUTER SYSTEMS " << ANSI_HILITE << ANSI_RED << "OFFLINE" << ANSI_NORMAL;
    NotifyUserSimple(message.str());
    NotifyUserSimple("");
    return 0;
  }
  return computer;
}

HSCommunicationsInstance*
HSConsole::FindCommSystem()
{
  std::vector<HSSystemInstance*> systems = mShip->FindSystemsByType(HSST_COMMUNICATIONS);

  if (!systems.size()) {
    return 0;
  }
  return static_cast<HSCommunicationsInstance*>(*(systems.begin()));
}

DBRef
HSConsole::GetUser()
{
  return HSIFGetObjectLock(mID, HSLock_Use);
}
