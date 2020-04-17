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
#include "HSVector3D.h"
#include "HSTools.h"

#include <sstream>

HSVector3D::operator std::string()
{
  std::stringstream retval;
  char bufX[256];
  char bufY[256];
  char bufZ[256];
  sprintf(bufX, "%f", mX);
  sprintf(bufY, "%f", mY);
  sprintf(bufZ, "%f", mZ);
  retval << "<" << bufX << "," << bufY << "," << bufZ << ">";
  return retval.str();
}

double
HSVector3D::HeadingXY()
{
  double retval = atan2(mY, mX) * 180.00 / M_PI;
  if (retval < 0) {
    retval += 360;
  }
  return retval;
}

double
HSVector3D::HeadingZ()
{
  if (zero()) {
    return 0;
  }
  return asin(mZ / length()) * 180.00 / M_PI;
}

std::string
HSVector3D::HeadingString()
{
  char tbuf[64];
  sprintf(tbuf, "%.f mark %.f", HeadingXY(), HeadingZ());
  return tbuf;
}

HSVector3DRender
HSVector3D::RenderVector() const
{
  HSVector3DRender vec;
  vec._x = (float)mX;
  vec._y = (float)mY;
  vec._z = (float)mZ;
  return vec;
}
