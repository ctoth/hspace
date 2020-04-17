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

#include "ctype.h"
#include "string.h"
#include "HSAnsi.h"
#include <string>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#define sprintf sprintf_s
#endif

template <typename T>
class HSForeachContainer {
public:
    inline HSForeachContainer(const T& t) : c(t), brk(0), i(c.begin()), e(c.end()) { }
    const T c;
    int brk;
    typename T::const_iterator i, e;
};

template <class T>
bool from_string(T& t, 
                 const std::string& s, 
                 std::ios_base& (*f)(std::ios_base&) = std::dec)
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
}

#define foreach( type, var, l ) \
  for(HSForeachContainer< std::vector<type> > _container_(l); (_container_.i != _container_.e && (!_container_.brk++)); ++_container_.i) \
    for(type var = *_container_.i; _container_.brk; _container_.brk--)


static inline int strncasecmp (const char *s1, const char *s2)
{
    for (size_t i = 0; s1[i] && s2[i]; i++)
    {
        int d = tolower (s1[i]) - tolower (s2[i]);
        if (d || !s1[i]) return d;
    }
    return 0;
}
/**
 * Displays an arbitrary length horizontal meter.
 *
 * @oaram aScale Total length of the meter
 * @param aMax Maximum amount for thing being metered
 * @param aCurrent The Current amount for the thing being metered
 * @param aStyle Character repeated through the meter for decoration
 * @return String containing the meter.
 */
std::string HSMeter(double aScale, double aMax, double aCurrent, std::string aStyle = ".");

/**
 * Displays a colorized xxx% reading.
 *
 * @param aMax Maximum amount for thing being metered
 * @param aCurrent The Current amount for the thing being metered
 * @return String containing the meter.
 */
std::string HSNumMeter(double aMax, double aCurrent);

/**
 * Displays a time string for the number of seconds.
 *
 * @param aTime Time in seconds.
 * @return String containing the time string
 */
std::string HSTimeString(int aTime);

/**
 * Returns a formatted string for a distance.
 *
 * @param aDistance Distance to format string for.
 * @return Formatted string
 */
std::string HSDistanceString(double aDistance);

/**
 * Returns a formatted string for a mass.
 *
 * @param aMass Mass to format string for.
 * @return Formatted string
 */
std::string HSMassString(int aMass);

#ifdef _DEBUG
#define DEBUG
#endif

#ifdef DEBUG
#define ASSERT(expr) _ASSERT(expr)
#else
#define ASSERT(expr)
#endif

#include "HSLog.h"
#ifdef HS_LOG
#define HSLog() HSLogInst()
#else
#define HSLog() if (true) {} else HSLogInst()
#endif

#define HS_MIN(x,y) ((x)<(y)?(x):(y))
#define HS_MAX(x,y) ((x)>(y)?(x):(y))
#define ABORT_IF_FALSE(x) do { if (!(x)) return false; } while (0)
