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

#include <string>
#include <vector>

#include "HSVector3D.h"

/**
 * Types of attributes.
 */
enum AttributeType
{
  /** String */
  AT_STRING = 0,
  /** Boolean */
  AT_BOOLEAN,
  /** 32-bit Integer */
  AT_INTEGER,
  /** Double precision floating point */
  AT_DOUBLE,
  /** HSVector3D */
  AT_VECTOR,
  /** std::vector<int> */
  AT_INTLIST,
  /** List of communication channels */
  AT_COMMCHANNELS,
  /** List of cargo items */
  AT_CARGOLIST
};

struct HSCommChannel;
struct HSCargoItem;

/**
 * \todo Need to look if we should store pointers seperately and create attribute vectors
 * statically per class, as this is currently adding a significant memory overhead per class.
 * (atleast 16 bytes)
 */
struct Attribute
{
  /** Name of the attribute */
  std::string Name;
  /** Type of the attribute */
  AttributeType Type;
  /** Can this attribute be modded by admins */
  bool Internal;
  /** Pointer to the attribute */
  void *Ptr;
  /** NULL if this attribute is always set. Otherwise indicates if the attribute is set */
  bool *IsSet;
};

#define ADD_ATTRIBUTE(name, type, var) \
{ \
  Attribute attr = { name, type, false, &var, 0 }; \
  mAttributeList.push_back(attr); \
}

#define ADD_ATTRIBUTE_INTERNAL(name, type, var) \
{ \
  Attribute attr = { name, type, true, &var, 0 }; \
  mAttributeList.push_back(attr); \
}

#define ATTRIBUTE(name, type) \
  public: \
  inline type Get##name () { return m##name; } \
  void inline Set##name (type aValue) { m##name = aValue; } \
  protected: \
  type m##name;

class HSDB;

/**
 * \ingroup HS_CORE
 * \brief This is the base class for all 'attributed' objects,
 *        attributed objects can be easily accessed by MU* admins
 *        and stored to/loaded from the database. Several macro's
 *        are provided for easy use.
 */
class HSAttributed
{
public:
  /**
   * Used to set the value of an attribute.
   *
   * @param aName Name of the attribute
   * @param aValue Value of the attribute
   * @param aInternal Is this an internal setattribute call
   * @return False if attribute of the right type was not found
   */
  bool SetAttribute(std::string aName, int aValue, bool aInternal = false);

  /**
   * Used to set the value of an attribute.
   *
   * @param aName Name of the attribute
   * @param aValue Value of the attribute
   * @param aInternal Is this an internal setattribute call
   * @return False if attribute of the right type was not found
   */
  bool SetAttribute(std::string aName, std::string aValue, bool aInternal = false);

  /**
   * Used to set the value of an attribute.
   *
   * @param aName Name of the attribute
   * @param aValue Value of the attribute
   * @param aInternal Is this an internal setattribute call
   * @return False if attribute of the right type was not found
   */
  bool SetAttribute(std::string aName, double aValue, bool aInternal = false);

  /**
   * Used to set the value of an attribute.
   *
   * @param aName Name of the attribute
   * @param aValue Value of the attribute
   * @param aInternal Is this an internal setattribute call
   * @return False if attribute of the right type was not found
   */
  bool SetAttribute(std::string aName, HSVector3D aValue, bool aInternal = false);

  /**
   * Used to set the value of an attribute.
   *
   * @param aName Name of the attribute
   * @param aValue Value of the attribute
   * @param aInternal Is this an internal setattribute call
   * @return False if attribute of the right type was not found
   */
  bool SetAttribute(std::string aName, std::vector<int> aValue, bool aInternal = false);

  /**
   * Used to set the value of an attribute.
   *
   * @param aName Name of the attribute
   * @param aValue Value of the attribute
   * @param aInternal Is this an internal setattribute call
   * @return False if attribute of the right type was not found
   */
  bool SetAttribute(std::string aName, std::vector<HSCommChannel> aValue, bool aInternal = false);

  /**
   * Used to set the value of an attribute.
   *
   * @param aName Name of the attribute
   * @param aValue Value of the attribute
   * @param aInternal Is this an internal setattribute call
   * @return False if attribute of the right type was not found
   */
  bool SetAttribute(std::string aName, std::vector<HSCargoItem> aValue, bool aInternal = false);

  /**
   * Used to get the value of an attribute.
   *
   * @param aName Name of the attribute
   * @param aValue Value of the attribute
   * @return False if attribute of the right type was not found
   */
  bool GetAttribute(std::string aName, int* aValue);

  /**
   * Used to get the value of an attribute.
   *
   * @param aName Name of the attribute
   * @param aValue Value of the attribute
   * @return False if attribute of the right type was not found
   */
  bool GetAttribute(std::string aName, std::string* aValue);

  /**
   * Used to get the value of an attribute.
   *
   * @param aName Name of the attribute
   * @param aValue Value of the attribute
   * @return False if attribute of the right type was not found
   */
  bool GetAttribute(std::string aName, double* aValue);

  /**
   * Used to get the value of an attribute.
   *
   * @param aName Name of the attribute
   * @param aValue Value of the attribute
   * @return False if attribute of the right type was not found
   */
  bool GetAttribute(std::string aName, HSVector3D *aValue);

  /**
   * Used to get the value of an attribute.
   *
   * @param aName Name of the attribute
   * @param aValue Value of the attribute
   * @return False if attribute of the right type was not found
   */
  bool GetAttribute(std::string aName, std::vector<HSCargoItem> *aValue);

  /**
   * Set attribute from string
   *
   * @param aName Name of the attribute
   * @param aValue of the attribute
   * @return False if attribute was not found or conversion failed
   */
  bool SetAttributeFromString(std::string aName, std::string aValue, bool aInternal = false);

  /**
   * Get attribute as a string
   *
   * @param aName Name of the attribute
   * @param aValue Pointer to an std::string to contain the value
   * @return False if attribute was not found
   */
  bool GetAttributeFromString(std::string aName, std::string *aValue);

  /**
   * Wipe the value of an attribute (unset it), this does not remove it.
   * \note This function is only valid for attributes which can be unset.
   *
   * @param aName Name of the sttribute
   * @param aInternal True if an internal request
   * @return True if succesful.
   */
  bool WipeAttribute(std::string aName, bool aInternal);

  /**
   * Get the type for an attribute
   *
   * @param aName Name of the attribute
   * @param aType Pointer to a variable to hold the type of the object
   * @return False if attribute was not found
   */
  bool GetAttributeType(std::string aName, AttributeType *aType);

  /**
   * Show additional info on this object. This is for override purposes.
   *
   * @return Additional information
   */
  virtual std::string GetAdditionalInfo() { return std::string(); }

  /**
   * Clone the value of all attributes on this object to the target
   * object.
   *
   * @param aTarget Target object.
   */
  void Clone(HSAttributed *aTarget);

  std::vector<Attribute> GetAttributeList();

  /**
   * Get attributes that may be stored on an object but that are not saved to the
   * database in a normal fashion.
   *
   * @return Extra attributes
   */
  virtual std::vector<Attribute> GetExtraAttributes() { return std::vector<Attribute>(); }

  // Total memory usage of all HSAttributed classes.
  static size_t sMemoryUsage;

  // New operator overload to track creation of attributed objects.
  void *operator new(size_t aSize);

  // Delete operator to balance attributed object memory tracking.
  void operator delete(void *aPointer);

protected:
  friend class HSDB;

  /**
   * Protected constructor instantiating the object.
   */
  HSAttributed(void);
  virtual ~HSAttributed(void);

  /**
   * Called when an attribute with a certain name is changed,
   * can be overidden by children to act upon attribute changes.
   *
   * @param aName Name of the attribute that changed
   */
  virtual void AttributeChanged(std::string aName) {}

  /** List of attributes on this instance of a HSAttributed class */
  std::vector<Attribute> mAttributeList;

private:
  
  bool FindAttribute(std::string aName, Attribute *aAttribute, bool aOnlySet = true);
};
