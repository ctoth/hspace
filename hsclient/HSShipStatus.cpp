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
#include "HSShipStatus.h"
#include "HSNetworkPacketContact.h"
#include "HSTools.h"

HSShipStatus::HSShipStatus(HSClientWindow *aClientWindow)
  : mClientWindow(aClientWindow)
  , mManned(false)
  , mConnected(false)
  , mDesiredSpeed(0)
  , mHullMax(0)
  , mHullCurrent(0)
  , mTurnRate(0.0)
  , mAcceleration(0.0)
  , mRadarRange(100)
{
}

HSShipStatus::~HSShipStatus(void)
{
}

void
HSShipStatus::HandleContactPacket(HSNetworkPacketContact *aPacket)
{
  if (aPacket->mContentFlag & 0x40) {
    if (aPacket->mRemoved) {
      for (std::vector<HSSensorContact>::iterator iter = mContacts.begin();
        iter != mContacts.end(); iter++) {
          if (iter->contactID == aPacket->mContactID) {
            mContacts.erase(iter);
            return;
          }
      }
    }
  }

  bool found = false;
  for (size_t i = 0; i < mContacts.size(); i++) {
    if (mContacts[i].contactID == aPacket->mContactID) {
      found = true;
      if (aPacket->mContentFlag & 0x01) {
        mContacts[i].objectID = aPacket->mObjectID;
      }
      if (aPacket->mContentFlag & 0x04) {
        mContacts[i].position = aPacket->mPosition;
      }
      if (aPacket->mContentFlag & 0x08) {
        mContacts[i].velocity = aPacket->mVelocity;
      }
      if (aPacket->mContentFlag & 0x10) {
        mContacts[i].objectType = aPacket->mObjectType;
      }
      if (aPacket->mContentFlag & 0x20) {
        mContacts[i].name = aPacket->mName;
      }
    }
  }

  if (!found) {
    HSSensorContact contact;

    if (aPacket->mContentFlag & 0x01) {
      contact.objectID = aPacket->mObjectID;
    }
    if (aPacket->mContentFlag & 0x02) {
      contact.contactID = aPacket->mContactID;
    } else {
      // This won't work.
      return;
    }
    if (aPacket->mContentFlag & 0x04) {
      contact.position = aPacket->mPosition;
    }
    if (aPacket->mContentFlag & 0x08) {
      contact.velocity = aPacket->mVelocity;
    }
    if (aPacket->mContentFlag & 0x10) {
      contact.objectType = aPacket->mObjectType;
    }
    if (aPacket->mContentFlag & 0x20) {
      contact.name = aPacket->mName;
    }
    mContacts.push_back(contact);    
  }
}
