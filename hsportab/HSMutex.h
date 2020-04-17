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
#pragma once

class HSMutexPriv;
class HSCondVar;

/**
 * \ingroup HS_PORTAB
 * \brief This is a portable mutex.
 */
class HSMutex
{
public:
  HSMutex(void);
  ~HSMutex(void);

  /**
   * Lock the mutex, this function will grab ownership
   * of the mutex, it can be called multiple times from the
   * thread owning it, but has to be released an equal amount
   * of times.
   */
  void Lock();

  /**
   * Tries to lock the mutex. It returns true if the mutex is
   * succesfully acquired, and false if it fails to acquire the
   * mutex.
   */
  bool TryLock();

  /**
   * Releases the mutex and will set off any waiting thread.s
   */
  void Release();

private:
  friend class HSCondVar;

  HSMutexPriv *p;
};

/**
 * \ingroup HS_PORTAB
 * Stack based helper for mutex usage.
 */
class HSAutoMutexLock
{
public:
  HSAutoMutexLock(HSMutex &aMutex)
    : mMutex(&aMutex)
  {
    mMutex->Lock();
  }
  ~HSAutoMutexLock()
  {
    mMutex->Release();
  }
private:
  HSMutex *mMutex;
};
