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
#include "HSTools.h"

extern "C" {
// Included since we use some Penn code.
#include "copyrite.h"
#include "config.h"

#include "conf.h"
#include "externs.h"
#include "dbdefs.h"
#include "mushdb.h"
#include "flags.h"
#include "attrib.h"
#include "lock.h"
#include "game.h"
#include "match.h"
#include "parse.h"
  
extern struct db_stat_info current_state;

// Declare this function since we use it to implement HSIFEncrypt/Decrypt.
static char *crypt_code(char *code, char *text, int type);

}

void 
HSIFNotify(DBRef aObject, std::string aMessage)
{
  if (!GoodObject(aObject)) {
    return;
  }
  raw_notify(aObject, aMessage.c_str());
}

DBRef
HSIFLocateNeighbor(DBRef aLooker, std::string aObject, ObjectType aType)
{
  dbref item;
  int matchtype;
  switch (aType) {
    case OT_NOTYPE:
      matchtype = NOTYPE;
      break;
    case OT_PLAYER:
      matchtype = TYPE_PLAYER;
      break;
    case OT_THING:
      matchtype = TYPE_THING;
      break;
    case OT_ROOM:
      matchtype = TYPE_ROOM;
      break;
    case OT_EXIT:
      matchtype = TYPE_EXIT;
      break;
  }
  item = match_result(aLooker, aObject.c_str(), matchtype, MAT_NEARBY);

  if (GoodObject(item)) {
    return item;
  }

  return DBRefNothing;
}

DBRef
HSIFLocateObject(DBRef aLooker, std::string aObject, ObjectType aType)
{
  int matchtype;
  switch (aType) {
    case OT_NOTYPE:
      matchtype = NOTYPE;
      break;
    case OT_PLAYER:
      matchtype = TYPE_PLAYER;
      break;
    case OT_THING:
      matchtype = TYPE_THING;
      break;
    case OT_ROOM:
      matchtype = TYPE_ROOM;
      break;
    case OT_EXIT:
      matchtype = TYPE_EXIT;
      break;
  }

  dbref item;

  item = match_result(aLooker, aObject.c_str(), matchtype, MAT_OBJECTS);

  if (!GoodObject(item)) {
    return -1;
  }

  if (!(Type(item) & matchtype)) {
    return -1;
  }

  return item;
}

DBRef
HSIFGetObjectLock(DBRef aObject, HSLockType aLockType)
{
  lock_type lt;
  switch (aLockType) {
    case HSLock_Basic:
      lt = Basic_Lock;
      break;
    case HSLock_Use:
      lt = Use_Lock;
      break;
    case HSLock_Zone:
      lt = Zone_Lock;
      break;
    case HSLock_Board:
      // We use the interact lock for this.
      lt = Interact_Lock;
      break;
  }
  if (!GoodObject(aObject)) {
    return DBRefNothing;
  }
  boolexp boolExp = getlock(aObject, lt);

  if (boolExp == NULL) {
    return DBRefNothing;
  } else {
    return atoi(unparse_boolexp(aObject, boolExp, UB_DBREF) + 1); //strtodbref();
  }
}

bool
HSIFSetObjectLock(DBRef aObject, HSLockType aLockType, DBRef aLockTo)
{
  lock_type lt;
  switch (aLockType) {
    case HSLock_Basic:
      lt = Basic_Lock;
      break;
    case HSLock_Use:
      lt = Use_Lock;
      break;
    case HSLock_Zone:
      lt = Zone_Lock;
      break;
    case HSLock_Board:
      // We use the interact lock for this.
      lt = Interact_Lock;
      break;
  }
  if (aObject == DBRefNothing) {
    return false;
  }
  char tbuf[256];

  sprintf(tbuf, "#%d", aLockTo);

  if (add_lock(GOD, aObject, lt, parse_boolexp(aLockTo, tbuf, lt), -1)) {
    return true;
  } else {
    return false;
  }
}

DBRef
HSIFGetObjectLocation(DBRef aObject)
{
  if (!GoodObject(aObject)) {
    return DBRefNothing;
  }
  return Location(aObject);
}

DBRef
HSIFGetObjectHome(DBRef aObject)
{
  if (!GoodObject(aObject)) {
    return DBRefNothing;
  }
  return Home(aObject);
}

bool
HSIFSetExitDestination(DBRef aObject, DBRef aDestination)
{
  if (!GoodObject(aObject) || !IsExit(aObject)) {
    return false;
  }
  if (aDestination == DBRefNothing) {
    Destination(aObject) = NOTHING;
    return true;
  }
  if (!GoodObject(aDestination) || !IsRoom(aDestination)) {
    return false;
  }
  Destination(aObject) = aDestination;
  return true;
}

void
HSIFTeleportObject(DBRef aObject, DBRef aLocation)
{
  if (!GoodObject(aObject) || IsGarbage(aObject)
    || !GoodObject(aLocation) || IsGarbage(aLocation)) {
      return;
  }
  safe_tel(aObject, aLocation, true, GOD, "MOVE");
}

void
HSIFSetName(DBRef aObject, std::string aName)
{
  if (!GoodObject(aObject)) {
    return;
  }
  set_name(aObject, aName.c_str());
}

void
HSIFNotifyContentsExcept(DBRef aObject, std::string aMsg, DBRef aExcept)
{
  if (!GoodObject(aObject)) {
    return;
  }

  if (aExcept == DBRefNothing) {
    aExcept = NOTHING;
  }
  notify_except(GOD, Contents(aObject), aExcept, aMsg.c_str(), 0);
}

bool
HSIFValidObject(DBRef aObject)
{
  return GoodObject(aObject) && !IsGarbage(aObject);
}

std::string
HSIFGetName(DBRef aObject)
{
  if (!GoodObject(aObject)) {
    return "";
  }
  return Name(aObject);
}

std::string
HSIFEncrypt(std::string aString, std::string aKey)
{
  if (aString.empty()) {
    return "";
  } else if (aKey.empty()) {
    return aString;
  }
  char *key = new char[aKey.size()];
  char *string = new char[aString.size()];
  memcpy(key, aKey.c_str(), aKey.size());
  memcpy(string, aString.c_str(), aString.size());
  std::string retval = crypt_code(key, string, 0);
  delete [] key;
  delete [] string;
  return retval;
}

std::string
HSIFDecrypt(std::string aString, std::string aKey)
{
  if (aString.empty()) {
    return "";
  } else if (aKey.empty()) {
    return aString;
  }
  char *key = new char[aKey.size()];
  char *string = new char[aString.size()];
  memcpy(key, aKey.c_str(), aKey.size());
  memcpy(string, aString.c_str(), aString.size());
  std::string retval = crypt_code(key, string, 1);
  delete [] key;
  delete [] string;
  return retval;
}

// Retrieved from PennMUSH file funcrypt.c, all credits for this function go to PennMUSH.
#ifdef WIN32
#define strcpy strcpy_s
#endif

static char *
crunch_code(char *code)
{
  char *in;
  char *out;
  static char output[BUFFER_LEN];

  out = output;
  in = code;
  while (*in) {
    while (*in == ESC_CHAR) {
      while (*in && *in != 'm')
        in++;
      in++;                     /* skip 'm' */
    }
    if ((*in >= 32) && (*in <= 126)) {
      *out++ = *in;
    }
    in++;
  }
  *out = '\0';
  return output;
}
static char *
crypt_code(char *code, char *text, int type)
{
  static char textbuff[BUFFER_LEN];
  char codebuff[BUFFER_LEN];
  int start = 32;
  int end = 126;
  int mod = end - start + 1;
  char *p, *q, *r;

  if (!text && !*text)
    return (char *) "";
  strcpy(codebuff, crunch_code(code));
  if (!code || !*code || !codebuff || !*codebuff)
    return text;
  textbuff[0] = '\0';

  p = text;
  q = codebuff;
  r = textbuff;
  /* Encryption: Simply go through each character of the text, get its ascii
   * value, subtract start, add the ascii value (less start) of the
   * code, mod the result, add start. Continue  */
  while (*p) {
    if ((*p < start) || (*p > end)) {
      p++;
      continue;
    }
    if (type)
      *r++ = (((*p++ - start) + (*q++ - start)) % mod) + start;
    else
      *r++ = (((*p++ - *q++) + 2 * mod) % mod) + start;
    if (!*q)
      q = codebuff;
  }
  *r = '\0';
  return textbuff;
}

DBRef
CloneObject(DBRef aThing)
{
  DBRef clone;

  clone = new_object();

  memcpy(REFDB(clone), REFDB(aThing), sizeof(object));

  // Clear name pointer, and give clone its own.
  Name(clone) = NULL;
  set_name(clone, Name(aThing));

  Locks(clone) = NULL;

  atr_cpy(clone, aThing);
  clone_locks(GOD, aThing, clone);
  Zone(clone) = Zone(aThing);
  Parent(clone) = Parent(aThing);
  Flags(clone) = clone_flag_bitmask("FLAG", Flags(aThing));
  local_data_clone(clone, aThing, 1);
  Powers(clone) = clone_flag_bitmask("POWER", Powers(aThing));

  /**
   * We give the clone the same modification time that its
   * other clone has, but update the creation time 
   */
  CreTime(clone) = mudtime;

  Contents(clone) = Location(clone) = Next(clone) = NOTHING;

  return clone;
}

DBRef
HSIFCloneThing(DBRef aThing)
{
  if (!GoodObject(aThing) || !IsThing(aThing) || IsGarbage(aThing)) {
    return DBRefNothing;
  }
  DBRef clone = CloneObject(aThing);
  current_state.things++;
  local_data_clone(clone, aThing, 1);
  moveto(clone, Location(aThing), GOD, "HSCLONE");

  return clone;
}

DBRef
HSIFCloneRoom(DBRef aRoom, std::map<DBRef, DBRef> *aClonedObjects)
{
  if (aClonedObjects->count(aRoom) > 0) {
    // This room is already in the cloned room map. Return the DBRef
    // of the existing clone.
    return (*aClonedObjects)[aRoom];
  }
  if (!GoodObject(aRoom) || !IsRoom(aRoom) || IsGarbage(aRoom)) {
    // We're not cloning this as a room!
    return DBRefNothing;
  }
  DBRef clone = CloneObject(aRoom);

  (*aClonedObjects)[aRoom] = clone;
  if(aClonedObjects->count(Zone(aRoom))) {
    Zone(clone) = (*aClonedObjects)[Zone(aRoom)];
  }
  current_state.rooms++;

  Exits(clone) = NOTHING;

  DBRef object;
  DOLIST(object, Contents(aRoom)) {
    if (IsPlayer(object)) {
      continue;
    }
    DBRef objectClone = HSIFCloneThing(object);
    if (objectClone != NOTHING) {
      moveto(objectClone, clone, GOD, "HSCLONE");
    }
    (*aClonedObjects)[object] = objectClone;
    if(aClonedObjects->count(Zone(object))) {
      Zone(objectClone) = (*aClonedObjects)[Zone(object)];
    }
  }

  DBRef exit;
  DOLIST(exit, Exits(aRoom)) {
    DBRef exitClone = CloneObject(exit);
    Source(exitClone) = clone;
    Destination(exitClone) = HSIFCloneRoom(Destination(exit), aClonedObjects);
    PUSH(exitClone, Exits(clone));
    (*aClonedObjects)[exit] = exitClone;
    current_state.exits++;
    if(aClonedObjects->count(Zone(exit))) {
      Zone(exitClone) = (*aClonedObjects)[Zone(exit)];
    }
  }
  return clone;
}

std::vector<DBRef>
HSIFGetContents(DBRef aRoom)
{
  std::vector<DBRef> retval;

  if (!GoodObject(aRoom) || IsGarbage(aRoom)) {
    return retval;
  }

  DBRef object;
  DOLIST(object, Contents(aRoom)) {
    retval.push_back(object);
  }
  return retval;
}

bool
HSIFPassesLock(DBRef aObject, DBRef aTarget, HSLockType aLockType)
{
  lock_type lt;
  switch (aLockType) {
    case HSLock_Basic:
      lt = Basic_Lock;
      break;
    case HSLock_Use:
      lt = Use_Lock;
      break;
    case HSLock_Zone:
      lt = Zone_Lock;
      break;
    case HSLock_Board:
      // We use the interact lock for this.
      lt = Interact_Lock;
      break;
  }
  if (!GoodObject(aObject) || IsGarbage(aObject)
    || !GoodObject(aTarget) || IsGarbage(aTarget)) {
    return DBRefNothing;
  }
  int retval = eval_lock(aObject, aTarget, lt);

  return retval ? true : false;
}

DBRef
HSIFGetDestination(DBRef aExit)
{
  if (!GoodObject(aExit) || !IsExit(aExit)) {
    return DBRefNothing;
  }
  return Destination(aExit);
}

void
HSIFLook(DBRef aPlayer, DBRef aObject)
{
  if (!GoodObject(aObject) || IsGarbage(aObject) ||
    !GoodObject(aPlayer)) {
    return;
  }
  look_room(aPlayer, aObject, LOOK_NORMAL, NULL);
  return;
}

bool
HSIFCallAttrib(DBRef aObject,
               DBRef aEnactor,
               std::string aAttribute,
               std::vector<std::string> aArgs,
               std::string *aReturn)
{
  if (!GoodObject(aObject) || IsGarbage(aObject) ||
    !GoodObject(aEnactor) || IsGarbage(aEnactor)) {
      return false;
  }

  bool success = false;
  char *retVal = new char[BUFFER_LEN];

  PE_REGS *pe_regs;
  pe_regs = pe_regs_create(PE_REGS_ARG, "hs_callattrib");
  size_t numargs = aArgs.size() > 10 ? 10 : aArgs.size();

  for (unsigned int i = 0; i < numargs; i++) {
    pe_regs_setenv_nocopy(pe_regs, i, aArgs[i].c_str());
  }

  if (call_attrib(aObject, aAttribute.c_str(), retVal, aEnactor, NULL, pe_regs)) {
    success = true;
  }

  pe_regs_free(pe_regs);

  if (aReturn) {
    *aReturn = retVal;
  }
  delete [] retVal;
  
  return success;
}

DBRef
HSIFLogin(std::string aUser, std::string aPassword)
{
  DBRef player = lookup_player(aUser.c_str());
  if (player == NOTHING) {
    return DBRefNothing;
  }
  if (!password_check(player, aPassword.c_str())) {
    return DBRefNothing;
  }
  return player;
}

bool
HSIFIsWizard(DBRef aObject)
{
	if (GoodObject(aObject) &&
		!IsGarbage(aObject) &&
		Wizard(aObject)) {
		return true;
	}
	return false;
}

void
HSIFDestroy(DBRef aObject)
{
  if (!GoodObject(aObject) || IsGarbage(aObject)) {
    return;
  }
  char tbuf[64];
  sprintf(tbuf, "#%d", aObject);

  do_destroy(GOD, tbuf, 1, NULL);
}
