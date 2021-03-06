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
#include "HSDockingHatch.h"
#include "HSIface.h"
#include "HSTools.h"
#include "HSDB.h"
#include "HSObject.h"

// STL
#include <string>

HSDockingHatch::HSDockingHatch(void)
  : mID(DBRefNothing)
  , mObjectID(DBRefNothing)
  , mLinkedTo(DBRefNothing)
  , mClamped(false)
{
  ADD_ATTRIBUTE_INTERNAL("ID", AT_INTEGER, mID)
  ADD_ATTRIBUTE_INTERNAL("OBJECTID", AT_INTEGER, mObjectID)
  ADD_ATTRIBUTE_INTERNAL("LINKEDTO", AT_INTEGER, mLinkedTo)
  ADD_ATTRIBUTE("CLAMPED", AT_BOOLEAN, mClamped)
}

HSDockingHatch::~HSDockingHatch(void)
{
}

void
HSDockingHatch::Disconnect(void)
{
  if (GetLinkedTo() == DBRefNothing) {
    return;
  }

  HSDockingHatch *hatch = sHSDB.FindDockingHatch(GetLinkedTo());
  if (!hatch) {
    HSLog() << "Database inconsistency, non existant linked to hatch.";
    return;
  }
  HSObject *ourObject = sHSDB.FindObject(GetObjectID());
  if (!ourObject) {
    HSLog() << "Database inconsistency, non existant owning object for hatch.";
    return;
  }
  HSObject *targetObject = sHSDB.FindObject(hatch->GetObjectID());
  if (!targetObject) {
    HSLog() << "Database inconsistency, non existant owning object for hatch.";
    return;
  }
  SetLinkedTo(DBRefNothing);
  hatch->SetLinkedTo(DBRefNothing);
  HSIFSetExitDestination(GetID(), DBRefNothing);
  HSIFSetExitDestination(hatch->GetID(), DBRefNothing);
  char tbuf[256];
  sprintf(tbuf, "%s has been disconnected from the %s.", HSIFGetName(GetID()).c_str(), targetObject->GetName().c_str());
  ourObject->NotifyConsolesFormatted("Navigation", tbuf);
  sprintf(tbuf, "The %s has disconnected from our %s.", ourObject->GetName().c_str(), HSIFGetName(hatch->GetID()).c_str());
  targetObject->NotifyConsolesFormatted("Navigation", tbuf);
}
