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

#include "HSTools.h"
#include "HSBeamWeapon.h"
#include "HSMatrix3D.h"
#include "HSShip.h"

HSBeamWeapon::HSBeamWeapon(void)
  : mTotalPower(0)
  , mChargeSpeed(0.00)
  , mMinCharge(1.00)
  , mRange(0)
  , mAccuracy(1.00)
  , mFallOff(0.00)
{
  mType = HSST_BEAMWEAPON;
  mTakesMount = true;
  ADD_ATTRIBUTE("TOTALPOWER", AT_INTEGER, mTotalPower)
  ADD_ATTRIBUTE("CHARGESPEED", AT_DOUBLE, mChargeSpeed)
  ADD_ATTRIBUTE("MINCHARGE", AT_DOUBLE, mMinCharge)
  ADD_ATTRIBUTE("RANGE", AT_INTEGER, mRange)
  ADD_ATTRIBUTE("ACCURACY", AT_DOUBLE, mAccuracy)
  ADD_ATTRIBUTE("FALLOFF", AT_DOUBLE, mFallOff)
}

HSBeamWeapon::~HSBeamWeapon(void)
{
}

HSBeamWeaponInstance::HSBeamWeaponInstance(void)
  : mCurrentCharge(0.0)
  , mMountXY(0)
  , mMountZ(0)
  , mArc(0)
{
  mType = HSST_BEAMWEAPON;
  ADD_ATTRIBUTE_INHERIT("TOTALPOWER", AT_INTEGER, mTotalPower)
  ADD_ATTRIBUTE_INHERIT("CHARGESPEED", AT_DOUBLE, mChargeSpeed)
  ADD_ATTRIBUTE_INHERIT("MINCHARGE", AT_DOUBLE, mMinCharge)
  ADD_ATTRIBUTE_INHERIT("RANGE", AT_INTEGER, mRange)
  ADD_ATTRIBUTE_INHERIT("ACCURACY", AT_DOUBLE, mAccuracy)
  ADD_ATTRIBUTE_INHERIT("FALLOFF", AT_DOUBLE, mFallOff)
  ADD_ATTRIBUTE("CURRENTCHARGE", AT_DOUBLE, mCurrentCharge)
  ADD_ATTRIBUTE("MOUNTXY", AT_INTEGER, mMountXY)
  ADD_ATTRIBUTE("MOUNTZ", AT_INTEGER, mMountZ)
  ADD_ATTRIBUTE("ARC", AT_INTEGER, mArc)
}

HSBeamWeaponInstance::~HSBeamWeaponInstance(void)
{
}

void
HSBeamWeaponInstance::Cycle()
{
  HSSystemInstance::Cycle();
  if (!GetCurrentPower()) {
    mCurrentCharge = 0;
    return;
  }

  if (mCurrentCharge >= GetTotalPower()) {
    return;
  }

  mCurrentCharge += GetChargeSpeed(true);

  if (mCurrentCharge > GetTotalPower()) {
    mCurrentCharge = GetTotalPower();
  }
}

HSFireResult
HSBeamWeaponInstance::Fire()
{
  if (!mTarget) {
    return HSFR_NOTLOCKED;
  }
  // Get all relevant vectors. Motion vectors are important for hit-chance
  // calculations.
  HSVector3D ourHeading = mShip->GetHeading();
  HSVector3D ourMotion = mShip->GetVelocity();
  HSVector3D targetMotion = mTarget->GetVelocity();
  HSVector3D firingVector = mTarget->GetLocation() - mShip->GetLocation();
  HSMatrix3D matrix = HSMatrix3D::FromVector(ourHeading);
  HSVector3D weaponVector = matrix * HSVector3D(mMountXY, mMountZ).normalized();

  if (firingVector.length() > GetRange()) {
    return HSFR_RANGE;
  }

  if (mCurrentCharge < (double)GetTotalPower() * GetMinCharge()) {
    return HSFR_CHARGE;
  }

  // Calculate the angle between the weapon heading vector and the firing vector.
  // The result of a crossproduct is A x B = |A| * |B| * Sin(angle) * right hand rule unit vector result
  // |A| and |B| are both known to be one when we normalize them.
  double angleDeviation = asin(HSVectorCrossProduct(firingVector.normalized(), weaponVector).length()) * (double)180 / M_PI;

  HSVector3D sum = firingVector.normalized() + weaponVector;
  if (sum.length() < 1) {
    angleDeviation = 360 - angleDeviation;
  }

  if (angleDeviation >= mArc) {
    return HSFR_ARC;
  }

  // Calculate a factor for effect of source and target movement on the hitdifficulty,
  // Cross products are used since if the motion is colinear to the firing vector it has
  // no influence. Cross product will be zero here. This will also automatically take into
  // account the distance between weapon and target. Weapons with a larger range are inherently
  // considered more accurate.
  double hitDifficultySource = HSVectorCrossProduct(ourMotion, firingVector).length() / (double)GetRange();
  double hitDifficultyTarget = HSVectorCrossProduct(targetMotion, firingVector).length() / (double)GetRange();
  double factor = 3.00 / (hitDifficultySource + hitDifficultyTarget + ((firingVector.length() * 5.0) / (double)GetRange()));
  if (factor < 0.01) {
    factor = 0.01;
  }
  double hitDifficulty = factor * GetAccuracy();
  double chance = (double)rand() / (double)RAND_MAX;
  double damage = mCurrentCharge - (firingVector.length() * GetFallOff());
  mCurrentCharge = 0;
  if (damage <= 0) {
    return HSFR_MISS;
  }

  if (chance < hitDifficulty) {
    GetTarget()->HandleWeaponImpact(damage, this);
    return HSFR_HIT;
  } else {
    GetTarget()->HandleWeaponMiss(this);
    return HSFR_MISS;
  }
}
