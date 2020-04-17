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

#define _USE_MATH_DEFINES
#include "math.h"
#include <string>

#define COMPARE_DOUBLE(num1, num2, tolerance) \
  ((num1) > ((num2) - tolerance) && (num1) < ((num2) + tolerance))

struct HSVector3DRender
{
  float _x, _y, _z;
};

/**
 * \ingroup HS_CORE
 * \brief This class is used for vector calculations
 */
class HSVector3D
{
public:
  double mX, mY, mZ;
 
  HSVector3D(void);
  HSVector3D(double aX, double aY, double aZ);
  HSVector3D(double aHeadingXY, double aHeadingZ);

  /**
   * Returns the length of the vector.
   *
   * @return Vector length
   */
  double length() const;

  /**
   * Returns if this is a zero vector
   *
   * @return True if zero vector, false if not
   */
  bool zero() const;

  /**
   * Returns the heading in the XY plane of the vector.
   *
   * @return Heading in the XY plane <0, 360]
   */
  double HeadingXY();

  /**
   * Returns the heading in the Z plane of the vector.
   *
   * @return Heading in the Z plane <-90, +90>
   */
  double HeadingZ();

  /**
   * Returns the heading of this vector in a formatted string
   *
   * @return String representing the heading
   */
  std::string HeadingString();

  HSVector3D& operator+=(const HSVector3D &aVector);
  HSVector3D& operator-=(const HSVector3D &aVector);
  HSVector3D& operator*=(const double &aScalar);
  HSVector3D& operator/=(const double &aScalar);
  bool operator==(const HSVector3D& aVector);
  bool operator!=(const HSVector3D& aVector);

  const HSVector3D operator+(const HSVector3D &aVector) const;
  const HSVector3D operator-(const HSVector3D &aVector) const;
  const HSVector3D operator*(const double &aScalar) const;

  /**
   * Returns a vector containing 3 floats, which can be used
   * by 3d renderers like D3D/OGL.
   *
   * @return Vector struct
   */
  HSVector3DRender RenderVector() const;


  /**
   * Returns the normalized vector.
   */
  HSVector3D normalized() const;

  operator std::string();
};

inline
HSVector3D::HSVector3D()
  : mX(0)
  , mY(0)
  , mZ(0)
{
}

inline
HSVector3D::HSVector3D(double aX, double aY, double aZ)
  : mX(aX)
  , mY(aY)
  , mZ(aZ)
{
}

inline
HSVector3D::HSVector3D(double aHeadingXY, double aHeadingZ)
{
  mX = cos(aHeadingXY / 180.00 * M_PI) * cos(aHeadingZ / 180.00 * M_PI);
  mY = sin(aHeadingXY / 180.00 * M_PI) * cos(aHeadingZ / 180.00 * M_PI);
  mZ = sin(aHeadingZ / 180.00 * M_PI);
}

inline bool
HSVector3D::zero() const
{
  return !mX && !mY && !mZ;
}

inline double
HSVector3D::length() const
{
  return sqrt(mX * mX + mY * mY + mZ * mZ);
}

inline HSVector3D& 
HSVector3D::operator+=(const HSVector3D& aVector)
{
  mX += aVector.mX;
  mY += aVector.mY;
  mZ += aVector.mZ;
  return *this;
}

inline HSVector3D& 
HSVector3D::operator-=(const HSVector3D& aVector)
{
  mX -= aVector.mX;
  mY -= aVector.mY;
  mZ -= aVector.mZ;
  return *this;
}

inline HSVector3D&
HSVector3D::operator*=(const double& aScalar)
{
  mX *= aScalar;
  mY *= aScalar;
  mZ *= aScalar;
  return *this;
}

inline HSVector3D&
HSVector3D::operator/=(const double& aScalar)
{
  mX /= aScalar;
  mY /= aScalar;
  mZ /= aScalar;
  return *this;
}

inline const HSVector3D
HSVector3D::operator+(const HSVector3D &aVector) const
{
  HSVector3D retval = *this;
  retval += aVector;
  return retval;
}

inline const HSVector3D
HSVector3D::operator-(const HSVector3D &aVector) const
{
  HSVector3D retval = *this;
  retval -= aVector;
  return retval;
}

inline const HSVector3D
HSVector3D::operator*(const double &aScalar) const
{
  HSVector3D retval = *this;
  retval *= aScalar;
  return retval;
}

inline HSVector3D
HSVector3D::normalized() const
{
  double len = length();
  if (len == 0) {
    return HSVector3D(1, 0, 0);
  }
  HSVector3D retval = *this;
  retval /= length();
  return retval;
}

inline bool
HSVector3D::operator==(const HSVector3D &aVector)
{
  return COMPARE_DOUBLE(mX, aVector.mX, 0.00001) && 
    COMPARE_DOUBLE(mY, aVector.mY, 0.00001) && 
    COMPARE_DOUBLE(mZ, aVector.mZ, 0.00001);  
}

inline bool
HSVector3D::operator!=(const HSVector3D &aVector)
{
  return !COMPARE_DOUBLE(mX, aVector.mX, 0.00001) ||
    !COMPARE_DOUBLE(mY, aVector.mY, 0.00001) || 
    !COMPARE_DOUBLE(mZ, aVector.mZ, 0.00001);  
}

inline HSVector3D
HSVectorCrossProduct(const HSVector3D &aVector1, const HSVector3D &aVector2)
{
  HSVector3D retval;
  retval.mX = aVector1.mY * aVector2.mZ - aVector1.mZ * aVector2.mY;
  retval.mY = aVector1.mX * aVector2.mZ - aVector1.mZ * aVector2.mX;
  retval.mZ = aVector1.mX * aVector2.mY - aVector1.mY * aVector2.mX;
  return retval;
}
