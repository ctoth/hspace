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

#include "string.h"

class HSByteArray
{
public:
  inline HSByteArray();
  inline HSByteArray(void *aData, int aSize);
  inline HSByteArray(const char *aData);
  inline HSByteArray(HSByteArray const& aOriginal);
  inline ~HSByteArray();

  inline HSByteArray &operator=(const HSByteArray &aByteArray);
  inline HSByteArray &operator+=(const HSByteArray &aByteArray);

  inline size_t size() const { return mSize; }

  inline const char *constData() const { return mPtr; }

  inline HSByteArray &Remove(size_t aStart, size_t aCount);
  inline HSByteArray &Clear();
private:
  inline void Release();

  char *mPtr;
  int *mRefs;
  size_t mPtrSize;
  size_t mSize;
};

HSByteArray::HSByteArray()
  : mPtr(0)
  , mPtrSize(0)
  , mSize(0)
{
  mRefs = new int;
  *mRefs = 1;
}

inline
HSByteArray::HSByteArray(void *aData, int aSize)
{
  mRefs = new int;
  *mRefs = 1;
  mPtr = new char[aSize + 1];
  mPtrSize = aSize + 1;
  memcpy(mPtr, aData, aSize);
  mPtr[aSize] = '\0';
  mSize = aSize;
}

inline
HSByteArray::HSByteArray(const char *aData)
{
  mRefs = new int;
  *mRefs = 1;
  mSize = strlen(aData);
  mPtr = new char[mSize + 1];
  mPtrSize = mSize + 1;
  memcpy(mPtr, aData, mSize + 1);
}

inline
HSByteArray::HSByteArray(const HSByteArray &aOriginal)
{
  mRefs = aOriginal.mRefs;
  ++(*mRefs);
  mPtr = aOriginal.mPtr;
  mPtrSize = aOriginal.mPtrSize;
  mSize = aOriginal.mSize;
}

inline
HSByteArray::~HSByteArray()
{
  Release();
}

inline void
HSByteArray::Release()
{
  if (!--(*mRefs)) {
    delete mPtr;
    delete mRefs;
  }
}

inline HSByteArray&
HSByteArray::operator=(const HSByteArray &aByteArray)
{
  if (&aByteArray == this) {
    return *this;
  }
  Release();
  mRefs = aByteArray.mRefs;
  ++(*mRefs);
  mPtr = aByteArray.mPtr;
  mPtrSize = aByteArray.mPtrSize;
  mSize = aByteArray.mSize;
  return *this;
}

inline HSByteArray&
HSByteArray::operator+=(const HSByteArray &aByteArray)
{
  if (aByteArray.mSize == 0) {
    return *this;
  }
  size_t oldSize = mSize;
  mSize = mSize + aByteArray.mSize;
  char *newPtr = new char[mSize + 1];
  if (oldSize) {
    memcpy(newPtr, mPtr, oldSize);
  }
  memcpy(newPtr + oldSize, aByteArray.mPtr, aByteArray.mSize);
  newPtr[mSize] = '\0';
  mPtrSize = mSize + 1;
  if (*mRefs > 1) {
    --(*mRefs);
    mPtr = newPtr;
    mRefs = new int;
    *mRefs = 1;
  } else {
    delete mPtr;
    mPtr = newPtr;
  }
  return *this;
}

inline HSByteArray&
HSByteArray::Remove(size_t aStart, size_t aCount)
{
  if (aCount == mSize) {
    return Clear();
  }
  char *newPtr = new char[mSize - aCount + 1];
  if (aStart > 0) {
    memcpy(newPtr, mPtr, aStart);
  }
  memcpy(newPtr + aStart, mPtr + aStart + aCount, mSize - aStart - aCount);
  mSize = mSize - aCount;
  mPtrSize = mSize + 1;
  newPtr[mSize] = '\0';
  if (*mRefs > 1) {
    --(*mRefs);
    mPtr = newPtr;
    mRefs = new int;
    *mRefs = 1;
  } else {
    delete mPtr;
    mPtr = newPtr;
  }
  return *this;
}

inline HSByteArray&
HSByteArray::Clear()
{
  mSize = 0;
  mPtrSize = 0;
  if (*mRefs > 1) {
    --(*mRefs);
    mRefs = new int;
    *mRefs = 1;
  } else {
    delete mPtr;
  }
  mPtr = NULL;
  return *this;
}
