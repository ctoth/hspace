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

#include "HSGlobal.h"
#include "HSAnsi.h"
#include "HSTools.h"

#include <sstream>
#include <math.h>

std::string
HSMeter(double aScale, double aMax, double aCurrent, std::string aStyle)
{
  std::stringstream buf;
  // It all looks like ass with only 5 blocks but lets make this a bottom line.
  if(aScale < 5) {
	  aScale = 5.00;
  }
  // Lets just make sure nothing silly happens.
  if(aCurrent > aMax) {
	  aCurrent = aMax;
  }
  if(aStyle.size() > 1 || !aStyle.size()) {
    aStyle = ".";
  }
  int idx = (int)(aCurrent / aMax * aScale + 1.00); // Find the end of the meter
  int ymark = (int)((aMax * 0.15) / aMax * aScale + 1.00); // Start Yellow BG
  int gmark = (int)((aMax * 0.4) / aMax * aScale + 1.00);  // Start Green BG
  double perc = aCurrent / aMax * 100; // Find the actual percentage of the meter
  char pbuf[8];

  sprintf(pbuf, "%.0f%%", perc);
  int len = (int)strlen(pbuf);
  buf << ANSI_WHITE;
  
  for(int i = 0; i < aScale; i++) {
    // Set the background color if we are still within idx.
    if( i == idx ) {
      buf << ANSI_NORMAL;
      buf << ANSI_HILITE;
    }
    if(i == 0) {
      buf << ANSI_BRED;
      buf << ANSI_HILITE;
      buf << ANSI_RED;
    } 
    if(i == ymark && idx > ymark) {
      buf << ANSI_BYELLOW;
      buf << ANSI_YELLOW;
    }
    if(i == gmark && idx > gmark) {
      buf << ANSI_BGREEN;
      buf << ANSI_GREEN;
    }
    // Add in the xxx% to the end of the meter.
    int backpos = (int)aScale - i;
    if(backpos > len && i >= idx) {
      buf << ' ';
    }
    else if (backpos > len && i < idx) {
      buf << aStyle;
    }
    else {
	  buf << ANSI_HILITE;
      buf << ANSI_WHITE;
      buf << pbuf[len - backpos];
    }
  }
  
  buf << ANSI_NORMAL;
  return buf.str();
}
std::string
HSNumMeter(double aMax, double aCurrent)
{
  std::stringstream buf;
  // Lets just make sure nothing silly happens.
  if(aCurrent > aMax) {
	  aCurrent = aMax;
  }
  double perc = aCurrent / aMax * 100; // Find the Percentage
  char pbuf[8];

  sprintf(pbuf, "%-3.0f%%", perc);
  buf << ANSI_HILITE;
  if(perc > 50) {
    buf << ANSI_GREEN;
  } else if(perc > 25) {
    buf << ANSI_YELLOW;
  } else {
    buf << ANSI_RED;
  }
  buf << pbuf;
  buf << ANSI_NORMAL;
  return buf.str();
}

std::string
HSTimeString(int aTime)
{
  int secs = aTime % 60;
  int minutes = (aTime % 3600) / 60;
  int hours = aTime / 3600;

  char tbufsec[64];
  char tbufmin[64];
  char tbufhour[64];

  if (secs > 9) {
    sprintf(tbufsec, "%d", secs);
  } else {
    sprintf(tbufsec, "0%d", secs);
  }
  if (minutes > 9) {
    sprintf(tbufmin, "%d", minutes);
  } else {
    sprintf(tbufmin, "0%d", minutes);
  }
  if (hours > 9) {
    sprintf(tbufhour, "%d", hours);
  } else {
    sprintf(tbufhour, "0%d", hours);
  }
  char tbuf[64];
  sprintf(tbuf, "%s:%s:%s", tbufhour, tbufmin, tbufsec);
  return tbuf;  
}

std::string
HSDistanceString(double aDistance)
{
  std::string unit = " km";
  double displayed = aDistance;

  if (displayed >= 30856775807000ULL) {
    displayed /= 30856775807000ULL;
    unit = " PC";
  } else if (displayed >= 149597871000ULL) {
    displayed /= 149597871000ULL;
    unit = "kAU";
  } else if (displayed >= 149597871) {
    displayed /= 149597871;
    unit = " AU";
  } else if (displayed >= 1000000) {
    unit = " Gm";
    displayed /= 1000000;
  } else if (displayed >= 1000) {
    unit = " Mm";
    displayed /= 1000;
  }

  char tbuf[512];
  sprintf(tbuf, "%.3f", displayed);

  std::string retval = tbuf;
  if (retval.length() > 5) {
    retval = retval.substr(0, 5);
  }
  return retval.append(unit);
}

std::string
HSMassString(int aMass)
{
  std::string unit = " kg";
  double displayed = aMass;

  if (displayed >= 1000000000) {
    displayed /= 1000000000;
    unit = " Mt";
  } else if (displayed >= 1000000) {
    unit = " kt";
    displayed /= 1000000;
  } else if (displayed >= 1000) {
    unit = " t ";
    displayed /= 1000;
  }

  char tbuf[512];
  sprintf(tbuf, "%.3f", displayed);

  std::string retval = tbuf;
  if (retval.length() > 5) {
    retval = retval.substr(0, 5);
  }
  return retval.append(unit);
}
