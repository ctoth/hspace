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
#include "HSCommunications.h"
#include "HSConsole.h"
#include "HSDB.h"
#include "HSShip.h"
#include "HSUniverse.h"
#include "HSComputer.h"
#include "HSTools.h"
#include "HSIface.h"
#include <sstream>

HSCommunications::HSCommunications(void)
  : mRange(0)
  , mNumChannels(0)
{
  mType = HSST_COMMUNICATIONS;
  ADD_ATTRIBUTE("RANGE", AT_INTEGER, mRange)
  ADD_ATTRIBUTE("NUMCHANNELS", AT_INTEGER, mNumChannels)
}

HSCommunications::~HSCommunications(void)
{
}

HSCommunicationsInstance::HSCommunicationsInstance(void)
  : mFrequency(100.00)
{
  mType = HSST_COMMUNICATIONS;
  ADD_ATTRIBUTE("FREQUENCY", AT_DOUBLE, mFrequency)
  ADD_ATTRIBUTE_INHERIT("RANGE", AT_INTEGER, mRange)
  ADD_ATTRIBUTE_INHERIT("NUMCHANNELS", AT_INTEGER, mNumChannels)
  ADD_ATTRIBUTE_INTERNAL("CHANNELS", AT_COMMCHANNELS, mChannels)
}

HSCommunicationsInstance::~HSCommunicationsInstance(void)
{
}

void
HSCommunicationsInstance::AddChannel(HSConsole *aConsole, double aFrequency, std::string aEncryption)
{
  if (aFrequency < 100.00 || aFrequency > 10000.00) {
    aConsole->NotifyUserSimple("Cannot tune in on that frequency.");
    return;
  }
  if (mChannels.size() >= (unsigned int)GetNumChannels()) {
    aConsole->NotifyUserSimple("Cannot tune into any more channels.");
    return;
  }
  if (aEncryption.find(" ") != std::string::npos) {
    aConsole->NotifyUserSimple("Encryption key cannot contain spaces.");
    return;
  }
  for(std::vector<HSCommChannel>::iterator iter = mChannels.begin();
    iter != mChannels.end(); iter++) {
      if (COMPARE_DOUBLE((*iter).Frequency, aFrequency, 0.05)) {
        mChannels.erase(iter);
        break;
      }
  }
  
  HSCommChannel newChannel;
  newChannel.Frequency = aFrequency;
  newChannel.Encryption = aEncryption;
  mChannels.push_back(newChannel);
  aConsole->NotifyUserSimple("Succesfully tuned into channel.");
}

void
HSCommunicationsInstance::RemoveChannel(HSConsole *aConsole, double aFrequency)
{
  for(std::vector<HSCommChannel>::iterator iter = mChannels.begin();
    iter != mChannels.end(); iter++) {
      if (COMPARE_DOUBLE((*iter).Frequency, aFrequency, 0.05)) {
        mChannels.erase(iter);
        aConsole->NotifyUserSimple("Channel removed.");
        return;
      }
  }
  aConsole->NotifyUserSimple("Channel not found.");
}

void
HSCommunicationsInstance::SendMessageChannel(HSConsole *aConsole, std::string aMessage)
{
  HSUniverse *myUniv = sHSDB.FindUniverse(mShip->GetUniverse());

  if (!myUniv) {
    // Woah! This shouldn't happen.
    aConsole->NotifyUser("<HSpace Error> Database inconsistency found! Please notify an administrator.");
    return;
  }
  std::string encryption;
  foreach(HSCommChannel, channel, mChannels) {
    if (channel.Frequency == mFrequency) {
      encryption = channel.Encryption;
      break;
    }
  }
  std::string message = HSIFEncrypt(aMessage, encryption);

  foreach(HSObject*, object, myUniv->GetObjects()) {
    ASSERT(object);
    if (object->GetType() == HSOT_SHIP) {
      // Don't broadcast to ourselves.
      if (object->GetID() == mShip->GetID()) {
        continue;
      }
      HSVector3D diffVector = object->GetLocation() - mShip->GetLocation();
      if (diffVector.length() > GetRange(true)) {
        continue;
      }
      HSShip *ship = static_cast<HSShip*>(object);
      std::vector<HSSystemInstance*> systems = ship->FindSystemsByType(HSST_COMMUNICATIONS);
      if (!systems.size()) {
        continue;
      }
      HSCommunicationsInstance *commSystem = 
        static_cast<HSCommunicationsInstance*>(systems[0]);
      commSystem->ReceiveMessageChannel(mShip, mFrequency, message);
    }
  }
  mShip->NotifyConsolesFormatted("Communications", 
    std::string("Message broadcasted: ").append(aMessage));
}

void
HSCommunicationsInstance::SetFrequency(HSConsole *aConsole, double aFrequency)
{
  if (aFrequency < 100.00 || aFrequency > 10000.00) {
    aConsole->NotifyUserSimple("Cannot broadcast on that frequency.");
    return;
  }
  mFrequency = aFrequency;
  char tbuf[256];
  sprintf(tbuf, "Broadcasting frequency set to %.2f MHz", aFrequency);
  mShip->NotifyConsolesFormatted("Communications", tbuf);
}

void
HSCommunicationsInstance::ReceiveMessageChannel(HSObject *aSource, double aFrequency, std::string aMessage)
{
  if (!GetCurrentPower()) {
    return;
  }
  bool found = false;
  std::string encKey;
  foreach(HSCommChannel, channel, mChannels) {
    if (COMPARE_DOUBLE(channel.Frequency, aFrequency, 0.005)) {
      found = true;
      encKey = channel.Encryption;
      break;
    }
  }
  if (!found) {
    return;
  }
  std::string decryptedMsg = HSIFDecrypt(aMessage, encKey);
  char tbuf[256];
  sprintf(tbuf, "Unknown (%.2f MHz)", aFrequency);
  std::string source = tbuf;
  HSSensorContact contact;
  
  HSComputerInstance *computer = mShip->FindComputer();
  if (computer) {
    if (computer->FindSensorContactByObjectID(aSource->GetID(), &contact)) {
      char tbuf[256];
      if (contact.Detail >= 50) {
        sprintf(tbuf, "%d - %s (%.2f MHz)", contact.ID, aSource->GetName().c_str(), aFrequency);
      } else {
        sprintf(tbuf, "Contact %d (%.2f MHz)", contact.ID, aFrequency);
      }
      source = tbuf;
    }
  }
  mShip->NotifyConsolesFormatted("Communications", source.append(": ").append(decryptedMsg));
}

void
HSCommunicationsInstance::SendMessage(HSConsole *aConsole, int aDestination, std::string aMessage)
{
  HSComputerInstance *computer = mShip->FindComputer();
  if (!computer) {
    aConsole->NotifyUserSimple("Sensor contact not found.");
    return;
  }
  HSSensorContact contact;

  if (!computer->FindSensorContact(aDestination, &contact)) {
    aConsole->NotifyUserSimple("Sensor contact not found.");
    return;
  }
  ASSERT(contact.Object);

  HSVector3D diffVector = contact.Object->GetLocation() - mShip->GetLocation();
  if (diffVector.length() > GetRange(true)) {
    aConsole->NotifyUserSimple("Sensor contact outside of communications range.");
    return;
  }

  if (contact.Object->GetType() == HSOT_SHIP) {
    HSShip *ship = static_cast<HSShip*>(contact.Object);

    std::vector<HSSystemInstance*> systems = ship->FindSystemsByType(HSST_COMMUNICATIONS);
    if (systems.size()) {
      HSCommunicationsInstance *commSystem = 
        static_cast<HSCommunicationsInstance*>(systems[0]);
      commSystem->ReceiveMessage(mShip, aMessage);
    }
  }
  char tbuf[256];
  if (contact.Detail > 50) {
    sprintf(tbuf, "Message sent to %d - %s: ", contact.ID, contact.Object->GetName().c_str());
  } else {
    sprintf(tbuf, "Message sent to contact %d: ", contact.ID);
  }
  mShip->NotifyConsolesFormatted("Communications", std::string(tbuf).append(aMessage));
}


void
HSCommunicationsInstance::ReceiveMessage(HSObject *aSource, std::string aMessage)
{
  if (!GetCurrentPower()) {
    return;
  }
  std::string source = "Unknown";
  HSSensorContact contact;
  
  HSComputerInstance *computer = mShip->FindComputer();
  if (computer) {
    if (computer->FindSensorContactByObjectID(aSource->GetID(), &contact)) {
      char tbuf[256];
      if (contact.Detail >= 50) {
        sprintf(tbuf, "%d - %s", contact.ID, aSource->GetName().c_str());
      } else {
        sprintf(tbuf, "Contact %d", contact.ID);
      }
      source = tbuf;
    }
  }
  mShip->NotifyConsolesFormatted("Communications", source.append(" (Direct): ").append(aMessage));
}

std::string
HSCommunicationsInstance::GetSystemStatus()
{
  std::stringstream stream;

  stream << HSSystemInstance::GetSystemStatus();

  char tbuf[256];
  sprintf(tbuf, "%s|   %sBroadcast Freq: %s%7.2f MHz          %sSignal Range:%s %-8s      %s|\n",
    ANSI_BLUE, ANSI_YELLOW, ANSI_NORMAL, mFrequency, ANSI_YELLOW, ANSI_NORMAL, 
    HSObject::DistanceString(GetRange(true)).c_str(), ANSI_BLUE);
  stream << tbuf;
  sprintf(tbuf, "%s|   %s[%sFrequency%s] [%sEncrypted%s] %s                                         |\n",
    ANSI_BLUE, ANSI_YELLOW, ANSI_NORMAL, ANSI_YELLOW, ANSI_NORMAL,
    ANSI_YELLOW, ANSI_BLUE);
  stream << tbuf;
  foreach(HSCommChannel, channel, mChannels) {
    sprintf(tbuf, "|   %s%7.2f MHz     %-3s                                              %s|\n",
      ANSI_NORMAL, channel.Frequency, channel.Encryption.empty() ? "No" : "Yes", ANSI_BLUE);
    stream << tbuf;
  }

  return stream.str();
}
