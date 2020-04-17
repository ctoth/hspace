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
#ifndef __RSIFACE_H__
#define __RSIFACE_H__

#include <string>
#include <vector>
#include <map>

typedef int DBRef;

/**
 * Type enumeration for objects.
 */
enum ObjectType {
  OT_NOTYPE = 0,
  OT_PLAYER,
  OT_THING,
  OT_ROOM,
  OT_EXIT
};

enum HSLockType {
  HSLock_Basic = 0,
  HSLock_Use,
  HSLock_Zone,
  HSLock_Board
};

const DBRef DBRefNothing = -1;

/**
 * Write something to an object on the MU* we're connected to.
 *
 * @param aObject Object to be notified.
 * @param aMessage Message to be sent.
 */
void HSIFNotify(DBRef aObject, std::string aMessage);

/**
 * Find an object on the MU* we're connected to.
 *
 * @param aLooker Object from who's 'perspective' we search
 * @param aObject String that we're trying to match to an object.
 * @param aType Type restriction to apply when looking.
 * @return DB reference to the MU* object.
 */
DBRef HSIFLocateObject(DBRef aLooker, std::string aObject, ObjectType aType);

/**
 * Find an object in the same location of looker
 *
 * @param aLooker Object from who's perspective we search.
 * @param aObject String of the object we're looking for
 * @return DBRef of the object found, or DBRefNothing
 */
DBRef HSIFLocateNeighbor(DBRef aLooker, std::string aObject, ObjectType aType = OT_THING);

/**
 * Find what an object is locked to.
 *
 * @param aObject Object to retrieve the lock for.
 * @param aLockType Lock type that we want to retrieve.
 * @return DBRef of the object it is locked to, DBRefNothing if nothing.
 */
DBRef HSIFGetObjectLock(DBRef aObject, HSLockType aLockType);

/**
 * Set the lock for an object
 *
 * @param aObject Object to set the lock for
 * @param aLockType Type of lock to set
 * @param aLockTo Who this should be locked to
 * @return True if succeeded, false if failed.
 */
bool HSIFSetObjectLock(DBRef aObject, HSLockType aLockType, DBRef aLockTo);

/**
 * Get the location of an object
 *
 * @param aObject Object to get the location for
 * @return Location of the object
 */
DBRef HSIFGetObjectLocation(DBRef aObject);

/**
 * Get the home of an object
 *
 * @param aObject Object to get the home for
 * @return Home of the object
 */
DBRef HSIFGetObjectHome(DBRef aObject);

/**
 * Set the destination of an exit
 *
 * @param aObject Exit to set the destination for
 * @param aDestination Room to set as destination
 * @return True if succeeded, false if failed.
 */
bool HSIFSetExitDestination(DBRef aObject, DBRef aDestination);

/**
 * Teleport an object to another location.
 *
 * @param aObject Object to teleport
 * @param aLocation Destination
 */
void HSIFTeleportObject(DBRef aObject, DBRef aLocation);

/**4
 * Set the name of an object.
 *
 * @param aObject Object to set the name for
 * @param aName Name to set
 */
void HSIFSetName(DBRef aObject, std::string aName);

/**
 * Notify the contents of an object.
 *
 * @param aObject Object to notify the contents of
 * @param aMsg Message to display
 * @param aException Object not to notify.
 */
void HSIFNotifyContentsExcept(DBRef aObject, std::string aName, DBRef aException = DBRefNothing);

/**
 * Check if something is a valid MU* object.
 *
 * @param aObject Object DBRef
 * @return True if valid, false if not.
 */
bool HSIFValidObject(DBRef aObject);

/**
 * Get the name of an object.
 *
 * @param aObject Object to get the name for
 * @return Name of the object
 */
std::string HSIFGetName(DBRef aObject);

/**
 * Encrypt a string.
 *
 * @param aString String to encrypt.
 * @return Encrypted string
 */
std::string HSIFEncrypt(std::string aString, std::string aKey);

/**
 * Decrypt a string.
 *
 * @param aString String to decrypt.
 * @return Decrypted string
 */
std::string HSIFDecrypt(std::string aString, std::string aKey);

/**
 * Clone a thing.
 *
 * @param aThing DBRef of the thing to clone.
 * @return DBRef of the cloned thing, DBRefNothing if failed.
 */
DBRef HSIFCloneThing(DBRef aThing);

/**
 * Clone a room and all connected rooms, and all contents.
 *
 * @param aRoom DBRef of the room to start
 * @param aClonedRooms A map of original DBRefs and new DBRefs.
 * @return The cloned room
 */
DBRef HSIFCloneRoom(DBRef aRoom, std::map<DBRef, DBRef> *aClonedObjects);

/**
 * Get a list of objects in a room.
 *
 * @param aRoom DBRef of the room
 * @return List of objects in the room
 */
std::vector<DBRef> HSIFGetContents(DBRef aRoom);

/**
 * See if an object passes a target's lock.
 *
 * @param aObject Object trying to pass the lock
 * @param aTarget The object who's lock we're evaluating
 * @param aLockType The type of lock we try to pass
 * @return True if the object passes, false if not
 */
bool HSIFPassesLock(DBRef aObject, DBRef aTarget, HSLockType aLockType);

/**
 * Get the destination of an exit.
 *
 * @param aExit Exit we're getting the destination for
 * @return Destination of the exit
 */
DBRef HSIFGetDestination(DBRef aExit);

/**
 * Let a player look upon something.
 *
 * @param aPlayer Player we want to look
 * @param aObject Object to look at
 */
void HSIFLook(DBRef aPlayer, DBRef aObject);

/**
 * Evaluate an attribute on an object.
 *
 * @param aObject Attribute to call upon.
 * @param aEnactor Enactor for the attribute evaluation.
 * @param aAttribute The attribute to evaluate.
 * @param aArguments Vector of arguments to put in.
 * @param aReturn String to store return value, NULL if discarded.
 * @return True if the attribute was found and succesfully evaluated.
 */
bool HSIFCallAttrib(DBRef aObject,
                    DBRef aEnactor,
                    std::string aAttribute,
                    std::vector<std::string> aArgs,
                    std::string *aReturn);

/**
 * Attempt to verify login of a user. Returns the DBRef of the
 * user, or nothing if unsuccesful.
 *
 * @param aUser User name to log in.
 * @param aPassword User password to log in.
 * @return DBRef of the user logged in, DBRefNothing in case of failure.
 */
DBRef HSIFLogin(std::string aUser,
                std::string aPassword);

/**
 * Check whether an object is Wizard. Returns true if it is.
 *
 * @param aObject Object to check for.
 * @return True if wizard.
 */
bool HSIFIsWizard(DBRef aObject);

/**
 * Destroys an objects.
 *
 * @param aObject Object to destroy.
 */
void HSIFDestroy(DBRef aObject);

#endif /* __RSIFACE_H__ */
