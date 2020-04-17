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
#include "HSSystem.h"

/**
 * Communication channel.
 */
struct HSCommChannel {
  /** Frequency of the channel */
  double Frequency;
  /** Encryption of the channel */
  std::string Encryption;
};

class HSObject;

class HSConsole;

/**
 * \ingroup HS_SYSTEMS
 * \brief This is the class for the communications system.
 */
class HSCommunications :
  public HSSystem
{
  /** Range in km of the signal */
  ATTRIBUTE(Range, int)
  /** Max amount of channels we're listening to */
  ATTRIBUTE(NumChannels, int)
public:
  /** Constructs communications system */
  HSCommunications(void);
  ~HSCommunications(void);
};

/**
 * \ingroup HS_SYSTEMINSTANCES
 * \brief This is the class for communication system instances.
 */
class HSCommunicationsInstance :
  public HSSystemInstance
{
  INHERITED_SYSTEM(HSCommunications*)
  /** Frequency we're currently broadcasting on */
  ATTRIBUTE(Frequency, double)
  ATTRIBUTE_INHERIT_ADJUSTED(Range, int)
  ATTRIBUTE_INHERIT(NumChannels, int)
public:
  HSCommunicationsInstance(void);
  ~HSCommunicationsInstance(void);

  /**
   * Add a channel to listen to.
   *
   * @param aConsole Console executing the command
   * @param aFrequency Frequency for the channel
   * @param aEncryption Encryption string for the channel
   */
  void AddChannel(HSConsole *aConsole, double aFrequency, std::string aEncryption);
  
  /**
   * Remove a channel from listening to it
   *
   * @param aConsole Console executing the command
   * @param aFrequency Frequency for channel to remove
   */
  void RemoveChannel(HSConsole *aConsole, double aFrequency);

  /**
   * Receive a message from a channel, called when we're in range of a
   * communications message.
   *
   * @param aSource Object sending the message
   * @param aFrequency Frequency the message is coming in on
   * @param aMessage Encrypted message string
   */
  void ReceiveMessageChannel(HSObject *aSource, double aFrequency, std::string aMessage);

  /**
   * Send message to the currently selected channel
   *
   * @param aConsole Console executing the command
   * @param aMessage Message to send
   */
  void SendMessageChannel(HSConsole *aConsole, std::string aMessage);
  
  /**
   * Set frequency to send messages on
   *
   * @param aConsole Console executing the command
   * @param aFrequency Frequency to send on
   */
  void SetFrequency(HSConsole *aConsole, double aFrequency);

  /**
   * Send a message to a specific contact
   *
   * @param aConsole Console executing the command
   * @param aDestination Destination contact ID
   * @param aMessage Message to send
   */
  void SendMessage(HSConsole *aConsole, int aDestination, std::string aMessage);

  /**
   * Receive a message from a source, called when someone sends us a message
   *
   * @param aSource Source sending the message
   * @param aMessage Message coming in
   */
  void ReceiveMessage(HSObject *aSource, std::string aMessage);

  virtual std::string GetSystemStatus();

private:
  /** Channels we're listening on */
  std::vector<HSCommChannel> mChannels;
};
