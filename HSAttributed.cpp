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
#include "HSAttributed.h"
#include "HSTools.h"
#include "HSCommunications.h"
#include "HSCommodity.h"
#include "HSDB.h"

// STL
#include <sstream>

size_t HSAttributed::sMemoryUsage = 0;

using namespace std;

HSAttributed::HSAttributed(void)
{
}

HSAttributed::~HSAttributed(void)
{
}

bool
HSAttributed::SetAttribute(string aName, int aValue, bool aInternal)
{
  Attribute attr;
  foreach(Attribute, attrs, mAttributeList) {
    if (attrs.Name == aName) {
      attr = attrs;
      break;
    }
  }
  if (attr.Name.empty() || (attr.Internal && !aInternal)) {
    return false;
  }

  switch (attr.Type) {
    case AT_INTEGER:
      *((int*)attr.Ptr) = aValue;
      break;
    case AT_BOOLEAN:
      *((bool*)attr.Ptr) = (aValue == 0) ? false : true;
      break;
    default:
      return false;
  }

  if (attr.IsSet && !*attr.IsSet) {
    *attr.IsSet = true;
  }
  AttributeChanged(aName);
  return true;
}

bool
HSAttributed::SetAttribute(string aName, string aValue, bool aInternal)
{
  Attribute attr;
  foreach(Attribute, attrs, mAttributeList) {
    if (attrs.Name == aName) {
      attr = attrs;
      break;
    }
  }
  if (attr.Name.empty() || attr.Type != AT_STRING || (attr.Internal && !aInternal)) {
    return false;
  }
  *((std::string*)attr.Ptr) = aValue;

  if (attr.IsSet && !*attr.IsSet) {
    *attr.IsSet = true;
  }
  AttributeChanged(aName);
  return true;
}

bool
HSAttributed::SetAttribute(string aName, double aValue, bool aInternal)
{
  Attribute attr;
  foreach(Attribute, attrs, mAttributeList) {
    if (attrs.Name == aName) {
      attr = attrs;
      break;
    }
  }
  if (attr.Name.empty() || attr.Type != AT_DOUBLE || (attr.Internal && !aInternal)) {
    return false;
  }
  *((double*)attr.Ptr) = aValue;

  if (attr.IsSet && !*attr.IsSet) {
    *attr.IsSet = true;
  }
  AttributeChanged(aName);
  return true;
}

bool
HSAttributed::SetAttribute(string aName, HSVector3D aValue, bool aInternal)
{  
  Attribute attr;
  foreach(Attribute, attrs, mAttributeList) {
    if (attrs.Name == aName) {
      attr = attrs;
      break;
    }
  }
  if (attr.Name.empty() || attr.Type != AT_VECTOR || (attr.Internal && !aInternal)) {
    return false;
  }
  *((HSVector3D*)attr.Ptr) = aValue;

  if (attr.IsSet && !*attr.IsSet) {
    *attr.IsSet = true;
  }
  AttributeChanged(aName);
  return true;
}

bool
HSAttributed::SetAttribute(string aName, std::vector<int> aValue, bool aInternal)
{  
  Attribute attr;
  foreach(Attribute, attrs, mAttributeList) {
    if (attrs.Name == aName) {
      attr = attrs;
      break;
    }
  }
  if (attr.Name.empty() || attr.Type != AT_INTLIST || (attr.Internal && !aInternal)) {
    return false;
  }
  *((std::vector<int>*)attr.Ptr) = aValue;

  if (attr.IsSet && !*attr.IsSet) {
    *attr.IsSet = true;
  }
  AttributeChanged(aName);
  return true;
}

bool
HSAttributed::SetAttribute(string aName, std::vector<HSCommChannel> aValue, bool aInternal)
{  
  Attribute attr;
  foreach(Attribute, attrs, mAttributeList) {
    if (attrs.Name == aName) {
      attr = attrs;
      break;
    }
  }
  if (attr.Name.empty() || attr.Type != AT_COMMCHANNELS || (attr.Internal && !aInternal)) {
    return false;
  }
  *((std::vector<HSCommChannel>*)attr.Ptr) = aValue;

  if (attr.IsSet && !*attr.IsSet) {
    *attr.IsSet = true;
  }
  AttributeChanged(aName);
  return true;
}

bool
HSAttributed::SetAttribute(string aName, std::vector<HSCargoItem> aValue, bool aInternal)
{  
  Attribute attr;
  foreach(Attribute, attrs, mAttributeList) {
    if (attrs.Name == aName) {
      attr = attrs;
      break;
    }
  }
  if (attr.Name.empty() || attr.Type != AT_CARGOLIST || (attr.Internal && !aInternal)) {
    return false;
  }
  *((std::vector<HSCargoItem>*)attr.Ptr) = aValue;

  if (attr.IsSet && !*attr.IsSet) {
    *attr.IsSet = true;
  }
  AttributeChanged(aName);
  return true;
}

bool
HSAttributed::GetAttribute(std::string aName, int* aValue)
{
  Attribute attr;
  ABORT_IF_FALSE(FindAttribute(aName, &attr));

  if (attr.IsSet && !*attr.IsSet) {
    return false;
  }

  switch (attr.Type) {
    case AT_INTEGER:
      *aValue = *((int*)attr.Ptr);
      break;
    case AT_BOOLEAN:
      *aValue = *((bool*)attr.Ptr);
      break;
    default:
      return false;
  }

  return true;
}

bool
HSAttributed::GetAttribute(std::string aName, string* aValue)
{
  Attribute attr;
  ABORT_IF_FALSE(FindAttribute(aName, &attr));

  if (attr.Type != AT_STRING || (attr.IsSet && !*attr.IsSet)) {
    return false;
  }
  *aValue = *((std::string*)attr.Ptr);

  return true;
}

bool
HSAttributed::GetAttribute(std::string aName, double* aValue)
{
  Attribute attr;
  ABORT_IF_FALSE(FindAttribute(aName, &attr));

  if (attr.Type != AT_DOUBLE || (attr.IsSet && !*attr.IsSet)) {
    return false;
  }
  *aValue = *((double*)attr.Ptr);

  return true;
}

bool
HSAttributed::GetAttribute(std::string aName, HSVector3D* aValue)
{
  Attribute attr;
  ABORT_IF_FALSE(FindAttribute(aName, &attr));

  if (attr.Type != AT_DOUBLE || (attr.IsSet && !*attr.IsSet)) {
    return false;
  }
  *aValue = *((HSVector3D*)attr.Ptr);

  return true;
}

bool
HSAttributed::GetAttribute(std::string aName, std::vector<HSCargoItem> *aValue)
{
  Attribute attr;
  ABORT_IF_FALSE(FindAttribute(aName, &attr));

  if (attr.Type != AT_CARGOLIST || (attr.IsSet && !*attr.IsSet)) {
    return false;
  }
  *aValue = *((std::vector<HSCargoItem>*)attr.Ptr);

  return true;
}

bool
HSAttributed::SetAttributeFromString(std::string aName, std::string aValue, bool aInternal)
{
  Attribute attr;
  attr.Name.clear();
  foreach(Attribute, attrs, mAttributeList) {
    if (!strncasecmp(aName.c_str(), attrs.Name.c_str())) {
      attr = attrs;
      break;
    }
  }
  if (attr.Name.empty()) {
    return false;
  }

  if (strncasecmp(aName.c_str(), attr.Name.c_str()) || (attr.Internal && !aInternal)) {
    return false;
  }

  switch (attr.Type) {
    case AT_INTEGER:
      *((int*)attr.Ptr) = atoi(aValue.c_str());
      break;
    case AT_BOOLEAN:
      if (aValue == "true" || aValue == "1") {
        *((bool*)attr.Ptr) = true;
      } else if (aValue == "false" || aValue == "0") {
        *((bool*)attr.Ptr) = false;
      } else {
        return false;
      }
      break;
    case AT_DOUBLE:
      *((double*)attr.Ptr) = atof(aValue.c_str());
      break;
    case AT_STRING:
      *((std::string*)attr.Ptr) = aValue;
      break;
    case AT_VECTOR:
      {
        string x = aValue.substr(0,aValue.find(","));
        string yz = aValue.substr(aValue.find(",") + 1);
        string y = yz.substr(0,yz.find(","));
        string z = yz.substr(yz.find(",") + 1);
        *((HSVector3D*)attr.Ptr) = HSVector3D(atof(x.c_str()), atof(y.c_str()), atof(z.c_str()));
      }
      break;
    case AT_CARGOLIST:
      {
        std::vector<HSCargoItem> cargo;
        while (aValue.length() > 0) {
          HSCargoItem newItem;
          string commodStr = aValue.substr(0, aValue.find(":"));
          HSCommodity *commod = sHSDB.FindCommodity(atoi(commodStr.c_str()));
          if (!commod) {
            return false;
          }
          newItem.commod = commod;
          string rest = aValue.substr(aValue.find(":") + 1);
          if (rest.find(" ") != (size_t)-1) {
            std::string amountStr = rest.substr(0, rest.find(" "));
            aValue = rest.substr(rest.find(" ") + 1);
            newItem.amount = atof(amountStr.c_str());
          } else {
            newItem.amount = atof(rest.c_str());
            aValue.clear();
          }
          cargo.push_back(newItem);
        }
        *((std::vector<HSCargoItem>*)attr.Ptr) = cargo;
      }
      break;
    case AT_INTLIST:
      {
        std::vector<int> nums;
        while (aValue.length() > 0) {
          if (aValue.find(" ") != (size_t)-1) {
            string numStr = aValue.substr(0, aValue.find(" "));
            aValue = aValue.substr(aValue.find(" ") + 1);
            nums.push_back(atoi(numStr.c_str()));
          } else {
            nums.push_back(atoi(aValue.c_str()));
            aValue.clear();
          }
        }
        *((std::vector<int>*)attr.Ptr) = nums;
      }
      break;
    default:
      return false;
  }
  if (attr.IsSet && !*attr.IsSet) {
    *attr.IsSet = true;
  }
  AttributeChanged(attr.Name);
  return true;
}

bool
HSAttributed::WipeAttribute(std::string aName, bool aInternal)
{
  Attribute attr;
  foreach(Attribute, attrs, mAttributeList) {
    if (!strncasecmp(aName.c_str(), attrs.Name.c_str())) {
      attr = attrs;
      break;
    }
  }
  if (strncasecmp(aName.c_str(), attr.Name.c_str()) || 
    (attr.Internal && !aInternal) ||
    !attr.IsSet) {
    return false;
  }
  *attr.IsSet = false;
  return true;
}

bool
HSAttributed::GetAttributeFromString(std::string aName, std::string *aValue)
{
  Attribute attr;
  bool found = false;

  foreach(Attribute, attrs, mAttributeList) {
    if (!strncasecmp(attrs.Name.c_str(), aName.c_str())) {
      attr = attrs;
      if (!attr.IsSet || *attr.IsSet) {
        found = true;
      }
    }
  }

  if (!found) {
    // Not found in standard attributes, search extra.
    foreach(Attribute, attrs, GetExtraAttributes()) {
      if (!strncasecmp(attrs.Name.c_str(), aName.c_str())) {
        attr = attrs;
        found = true;
      }
    }
  }

  if (!found) {
    return false;
  }

  char tbuf[128];
  switch (attr.Type) {
    case AT_INTEGER:
      sprintf(tbuf, "%d", *((int*)attr.Ptr));
      *aValue = std::string(tbuf);
      break;
    case AT_DOUBLE:
      sprintf(tbuf, "%.12f", *((double*)attr.Ptr));
      *aValue = std::string(tbuf);
      break;
    case AT_BOOLEAN:
      *aValue =  *((bool*)attr.Ptr) ? "true" : "false";
      break;
    case AT_STRING:
      *aValue = *((std::string*)attr.Ptr);
      break;
    case AT_VECTOR:
      *aValue = *((HSVector3D*)attr.Ptr);
      break;
    case AT_CARGOLIST:
      {
        stringstream retval;
        foreach(HSCargoItem, item, *(std::vector<HSCargoItem>*)attr.Ptr) {
          if (retval.str().size()) {
            retval << ' ';
          }
          retval << item.commod->GetID() << ':' << item.amount;
        }
        *aValue = retval.str();
      }
      break;
    case AT_INTLIST:
      {
        stringstream retval;
        foreach(int, num, *(std::vector<int>*)attr.Ptr) {
          if (retval.str().size()) {
            retval << ' ';
          }
          retval << num;
        }
        *aValue = retval.str();
      }
      break;
    default:
      return false;
  }
  return true;
}

bool
HSAttributed::GetAttributeType(std::string aName, AttributeType *aType)
{
  Attribute attr;
  ABORT_IF_FALSE(FindAttribute(aName, &attr, false));

  *aType = attr.Type;
  return true;
}

void
HSAttributed::Clone(HSAttributed *aTarget)
{
  foreach(Attribute, attr, mAttributeList) {
    Attribute target;
    foreach(Attribute, targetAttr, aTarget->mAttributeList) {
      if (attr.Name == targetAttr.Name) {
        target = targetAttr;
        break;
      }
    }
    if (attr.Name != target.Name) {
      continue;
    }
    if (attr.Type != target.Type) {
      continue;
    }
    switch (attr.Type) {
      case AT_INTEGER:
        *((int*)target.Ptr) = *((int*)attr.Ptr);
        break;
      case AT_DOUBLE:
        *((double*)target.Ptr) = *((double*)attr.Ptr);
        break;
      case AT_BOOLEAN:
        *((bool*)target.Ptr) = *((bool*)attr.Ptr);
        break;
      case AT_STRING:
        *((std::string*)target.Ptr) = *((std::string*)attr.Ptr);
        break;
      case AT_VECTOR:
        *((HSVector3D*)target.Ptr) = *((HSVector3D*)attr.Ptr);
        break;
      case AT_INTLIST:
        *((std::vector<int>*)target.Ptr) = *((std::vector<int>*)attr.Ptr);
        break;
      default:
        continue;
    }
    if (attr.IsSet && target.IsSet) {
      *target.IsSet = *attr.IsSet;
    }
    aTarget->AttributeChanged(target.Name);
  }
}

std::vector<Attribute>
HSAttributed::GetAttributeList()
{
  return mAttributeList;
}

void*
HSAttributed::operator new(size_t aSize)
{
  sMemoryUsage += aSize;
  char *p = (char*)malloc(aSize + sizeof(size_t));
  memcpy(p, &aSize, sizeof(size_t));
  return p + sizeof(size_t);
}

void
HSAttributed::operator delete(void *aPointer)
{
  size_t size;
  char *p = (char*)aPointer - sizeof(size_t);
  memcpy(&size, p, sizeof(size_t));
  sMemoryUsage -= size;
  free(p);
}

bool
HSAttributed::FindAttribute(std::string aName, Attribute *aAttribute, bool aOnlySet)
{
  foreach(Attribute, attrs, mAttributeList) {
    if (attrs.Name == aName) {
      if (aOnlySet && attrs.IsSet && !*attrs.IsSet) {
        return false;
      }
      *aAttribute = attrs;
      return true;
    }
  }

  // Not found in standard attributes, search extra.
  foreach(Attribute, attrs, GetExtraAttributes()) {
    if (attrs.Name == aName) {
      *aAttribute = attrs;
      return true;
    }
  }

  return false;
}