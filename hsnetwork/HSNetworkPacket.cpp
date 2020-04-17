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
#include "HSNetworkPacket.h"
#include "HSNetworkPacketMessage.h"
#include "HSNetworkPacketLogin.h"
#include "HSNetworkPacketShipInfo.h"
#include "HSNetworkPacketNavStat.h"
#include "HSNetworkPacketEngStat.h"
#include "HSNetworkPacketContact.h"
#include "HSVector3D.h"
#include "HSLog.h"

HSNetworkPacket::HSNetworkPacket(void)
  : mContentFlag(0xFFFFFFFF)
  , mIsFlagged(false)
{
}

HSNetworkPacket::~HSNetworkPacket(void)
{
}

HSByteArray
HSNetworkPacket::ByteStream()
{
  HSByteArray payload;
  unsigned int size = sizeof(unsigned int) * 2;
  if (mIsFlagged) {
    size += 4;
  }
  unsigned int flag = 0xffffffff;
  for (std::deque<HSDataElement>::const_iterator iter = mElements.begin();
    iter != mElements.end(); iter++) {
      switch (iter->type) {
        case HSDT_INTEGER:
          {
            payload += HSByteArray(iter->ptr, sizeof(int));
            size += 4;
          }
          break;
        case HSDT_DOUBLE:
          {
            payload += HSByteArray(iter->ptr, sizeof(double));
            size += sizeof(double);
          }
          break;
        case HSDT_VECTOR:
          {
            payload += HSByteArray(iter->ptr, sizeof(HSVector3D));
            size += sizeof(HSVector3D);
            break;
          }
        case HSDT_STRING:
          {
            std::string *str = (std::string*)iter->ptr;
            unsigned int sizeStr = (unsigned int)str->length();
            payload += HSByteArray(&sizeStr, sizeof(unsigned int));
            payload += HSByteArray((void*)str->c_str(), (int)sizeStr);
            size += sizeof(unsigned int) + sizeStr;
          }
          break;
        case HSDT_BOOLEAN:
          {
            payload += HSByteArray(iter->ptr, sizeof(bool));
            size += sizeof(bool);
          }       
          break;
      }
  }
  HSByteArray retval(&size, sizeof(unsigned int));
  retval += HSByteArray(&mType, sizeof(int));
  if (mIsFlagged) {
    retval += HSByteArray(&flag, sizeof(unsigned int));
  }
  retval += payload;
  return retval;
}

HSByteArray
HSNetworkPacket::ByteStream(HSNetworkPacket *aLatestPacket)
{
  HSByteArray payload;

  if (!mIsFlagged || !aLatestPacket) {
    return ByteStream();
  }

  unsigned int size = sizeof(unsigned int) * 3;
  unsigned int flag = 0x0;
  int element = 0;
  std::deque<HSDataElement>::const_iterator iterOld = aLatestPacket->mElements.begin();
  for (std::deque<HSDataElement>::const_iterator iter = mElements.begin();
    iter != mElements.end(); iter++, iterOld++, element++) {
      switch (iter->type) {
        case HSDT_INTEGER:
          {
            if ((*((int*)iter->ptr) != *((int*)iterOld->ptr)) || iter->alwaysSend) {
              flag |= 1 << element;
              payload += HSByteArray(iter->ptr, sizeof(int));
              size += 4;
            }
          }
          break;
        case HSDT_DOUBLE:
          {
            if ((*((double*)iter->ptr) != *((double*)iterOld->ptr)) || iter->alwaysSend) {
              flag |= 1 << element;
              payload += HSByteArray(iter->ptr, sizeof(double));
              size += sizeof(double);
            }
          }
          break;
        case HSDT_VECTOR:
          {
            if ((*((HSVector3D*)iter->ptr) != *((HSVector3D*)iterOld->ptr)) || iter->alwaysSend) {
              flag |= 1 << element;
              payload += HSByteArray(iter->ptr, sizeof(HSVector3D));
              size += sizeof(HSVector3D);
            }
            break;
          }
        case HSDT_STRING:
          {
            if ((*((std::string*)iter->ptr) != *((std::string*)iterOld->ptr)) || iter->alwaysSend) {
              flag |= 1 << element;
              std::string *str = (std::string*)iter->ptr;
              unsigned int sizeStr = (unsigned int)str->length();
              payload += HSByteArray(&sizeStr, sizeof(unsigned int));
              payload += HSByteArray((void*)str->c_str(), (int)sizeStr);
              size += sizeof(unsigned int) + sizeStr;
            }
          }
          break;
        case HSDT_BOOLEAN:
          {
            if ((*((bool*)iter->ptr) != *((bool*)iterOld->ptr)) || iter->alwaysSend) {
              flag |= 1 << element;
              payload += HSByteArray(iter->ptr, sizeof(bool));
              size += sizeof(bool);
            }
          }
          break;
      }
  }
  if (!flag) {
    return HSByteArray();
  }
  HSByteArray retval(&size, sizeof(unsigned int));
  retval += HSByteArray(&mType, sizeof(int));
  retval += HSByteArray(&flag, sizeof(unsigned int));
  retval += payload;
  return retval;
}

bool
HSNetworkPacket::RetrievePacket(const HSByteArray &aData, HSNetworkPacket *aOldPacket)
{
  size_t pos = 8;
  const char *data = aData.constData();
  mContentFlag = 0xFFFFFFFF;
  unsigned int element = 0;
  if (mIsFlagged) {
    if (aData.size() - pos < sizeof(unsigned int)) {
      return false;
    }
    memcpy(&mContentFlag, data + pos, sizeof(int));
    pos += sizeof(unsigned int);
  }
  for (std::deque<HSDataElement>::const_iterator iter = mElements.begin();
    iter != mElements.end(); iter++, element++) {
      if ((1 << element) & mContentFlag) {
        switch (iter->type) {
          case HSDT_INTEGER:
            {
              if (aData.size() - pos < sizeof(int)) {
                return false;
              }
              memcpy(iter->ptr, data + pos, sizeof(int));
              pos += sizeof(int);
            }
            break;
          case HSDT_DOUBLE:
            {
              if (aData.size() - pos < sizeof(double)) {
                return false;
              }
              memcpy(iter->ptr, data + pos, sizeof(double));
              pos += sizeof(double);
            }
            break;
          case HSDT_VECTOR:
            {
              if (aData.size() - pos < sizeof(HSVector3D)) {
                return false;
              }
              memcpy(iter->ptr, data + pos, sizeof(HSVector3D));
              pos += sizeof(HSVector3D);
            }
            break;
          case HSDT_STRING:
            {
              std::string newStr;
              unsigned int strSize;
              if (aData.size() - pos < sizeof(unsigned int)) {
                return false;
              }
              memcpy(&strSize, data + pos, sizeof(unsigned int));
              pos += sizeof(unsigned int);
              if (aData.size() - pos < strSize) {
                return false;
              }
              newStr.resize(strSize);
              memcpy((void*)newStr.data(), data + pos, strSize);
              pos += strSize;
              *(std::string*)iter->ptr = newStr;
            }
            break;
          case HSDT_BOOLEAN:
            {
              if (aData.size() - pos < sizeof(bool)) {
                return false;
              }
              memcpy(iter->ptr, data + pos, sizeof(bool));
              pos += sizeof(bool);
            }
            break;            
        }
      }
  }
  return true;
}

HSNetworkPacket*
HSNetworkPacket::ReadPacket(const HSByteArray &aData,
                            size_t *aSizeRead)
{
  unsigned int size;
  HSPacketType type;
  *aSizeRead = 0;
  if (aData.size() < 4) {
    return NULL;
  }
  const char *data = aData.constData();
  memcpy(&size, data, sizeof(unsigned int));
  if (aData.size() < size) {
    return NULL;
  }
  memcpy(&type, data + 4, 4);
  HSNetworkPacket *newPacket = NULL;
  switch (type) {
    case HSPT_NONE:
      newPacket = new HSNetworkPacket();
      break;
    case HSPT_MESSAGE:
      newPacket = new HSNetworkPacketMessage();
      break;
    case HSPT_LOGIN:
      newPacket = new HSNetworkPacketLogin();
      break;
    case HSPT_SHIPINFO:
      newPacket = new HSNetworkPacketShipInfo();
      break;
    case HSPT_NAVSTAT:
      newPacket = new HSNetworkPacketNavStat();
      break;
    case HSPT_ENGSTAT:
      newPacket = new HSNetworkPacketEngStat();
      break;
    case HSPT_CONTACT:
      newPacket = new HSNetworkPacketContact();
      break;
  }

  if (!newPacket) {
    HSLog() << "Unknown packet type received.";
    return NULL;
  }

  if (newPacket->RetrievePacket(aData)) {
    *aSizeRead = size;
    return newPacket;
  } else {
    *aSizeRead = size;
    delete newPacket;
    return NULL;
  }
}
