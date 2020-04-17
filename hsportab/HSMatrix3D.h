#pragma once

#include "HSVector3D.h"
#include <math.h>

/**
 * \brief Basic class used for 4x4 matrices for 3d affine transformations.
 * \ingroup HS_PORTAB
 */
class HSMatrix3D
{
public:
  /**
   * Create matrix.
   */
  HSMatrix3D(void);
  ~HSMatrix3D(void);

  /** Matrix contant */
  union {
    struct {
      float _11, _12, _13, _14;
      float _21, _22, _23, _24;
      float _31, _32, _33, _34;
      float _41, _42, _43, _44;
    };
    float m[4][4];
  };

  /**
   * Returns the result of this matrix multiplied by a vector.
   *
   * @param aVector Vector to multiply with
   * @return Resultant vector
   */
  HSVector3D operator*(const HSVector3D &aVector);

  /**
   * Creates a rotational matrix that rotates by a unit vector.
   *
   * @param aVector Unit vector to rotate by
   * @return Matrix that can be used to rotate another vector
   * by this unit vector.
   */
  static HSMatrix3D FromVector(const HSVector3D &aVector);

  HSMatrix3D operator*(const HSMatrix3D &aMatrix);

  static HSMatrix3D Identity();

  static HSMatrix3D Translate(float aX, float aY, float aZ);

  static HSMatrix3D Projection(float aWidth, float aHeight, float aZNear, float aZFar);

  static HSMatrix3D ProjectionFOV(float aFOV, float aAspect, float aZNear, float aZFar);
};

inline
HSMatrix3D::HSMatrix3D(void)
{
  memset(m, 0, 16 * sizeof(float));
}

inline
HSMatrix3D::~HSMatrix3D(void)
{
}

inline HSMatrix3D
HSMatrix3D::operator*(const HSMatrix3D &aMatrix)
{
  HSMatrix3D matrix;

  matrix._11 = _11 * aMatrix._11 + _12 * aMatrix._21 + _13 * aMatrix._31 + _14 * aMatrix._41;
  matrix._21 = _21 * aMatrix._11 + _22 * aMatrix._21 + _23 * aMatrix._31 + _24 * aMatrix._41;
  matrix._31 = _31 * aMatrix._11 + _32 * aMatrix._21 + _33 * aMatrix._31 + _34 * aMatrix._41;
  matrix._41 = _41 * aMatrix._11 + _42 * aMatrix._21 + _43 * aMatrix._31 + _44 * aMatrix._41;
  matrix._12 = _11 * aMatrix._12 + _12 * aMatrix._22 + _13 * aMatrix._32 + _14 * aMatrix._42;
  matrix._22 = _21 * aMatrix._12 + _22 * aMatrix._22 + _23 * aMatrix._32 + _24 * aMatrix._42;
  matrix._32 = _31 * aMatrix._12 + _32 * aMatrix._22 + _33 * aMatrix._32 + _34 * aMatrix._42;
  matrix._42 = _41 * aMatrix._12 + _42 * aMatrix._22 + _43 * aMatrix._32 + _44 * aMatrix._42;
  matrix._13 = _11 * aMatrix._13 + _12 * aMatrix._23 + _13 * aMatrix._33 + _14 * aMatrix._43;
  matrix._23 = _21 * aMatrix._13 + _22 * aMatrix._23 + _23 * aMatrix._33 + _24 * aMatrix._43;
  matrix._33 = _31 * aMatrix._13 + _32 * aMatrix._23 + _33 * aMatrix._33 + _34 * aMatrix._43;
  matrix._43 = _41 * aMatrix._13 + _42 * aMatrix._23 + _43 * aMatrix._33 + _44 * aMatrix._43;
  matrix._14 = _11 * aMatrix._14 + _12 * aMatrix._24 + _13 * aMatrix._34 + _14 * aMatrix._44;
  matrix._24 = _21 * aMatrix._14 + _22 * aMatrix._24 + _23 * aMatrix._34 + _24 * aMatrix._44;
  matrix._34 = _31 * aMatrix._14 + _32 * aMatrix._24 + _33 * aMatrix._34 + _34 * aMatrix._44;
  matrix._44 = _41 * aMatrix._14 + _42 * aMatrix._24 + _43 * aMatrix._34 + _44 * aMatrix._44;

  return matrix;
}

inline HSVector3D
HSMatrix3D::operator*(const HSVector3D &aVector)
{
  HSVector3D vector;

  vector.mX = aVector.mX * _11 + aVector.mY * _21 + aVector.mZ * _31 + _41;
  vector.mY = aVector.mX * _12 + aVector.mY * _22 + aVector.mZ * _32 + _42;
  vector.mZ = aVector.mX * _13 + aVector.mY * _23 + aVector.mZ * _33 + _43;
  return vector;
}

inline HSMatrix3D
HSMatrix3D::FromVector(const HSVector3D &aVector)
{
  HSMatrix3D matrix;
  HSVector3D unitVec;
  unitVec.mX = 0;
  unitVec.mY = -aVector.mZ;
  unitVec.mZ = aVector.mY;
  double s = sqrt(pow(unitVec.mZ, 2) + pow(unitVec.mY, 2));
  double c = sqrt(1 - pow(unitVec.mZ, 2) - pow(unitVec.mY, 2));
  if (aVector.mX < 0) {
    c = -c;
  }
  unitVec = unitVec.normalized();

  matrix._11 = (float)c;
  matrix._12 = (float)(-unitVec.mZ * s);
  matrix._13 = (float)(unitVec.mY * s);
  matrix._21 = (float)(unitVec.mZ * s);
  matrix._22 = (float)(pow(unitVec.mY, 2) + c - pow(unitVec.mY, 2) * c);
  matrix._23 = (float)(unitVec.mY * unitVec.mZ - unitVec.mY * unitVec.mZ * c);
  matrix._31 = (float)(-unitVec.mY * s);
  matrix._32 = (float)(unitVec.mY * unitVec.mZ - unitVec.mY * unitVec.mZ * c);
  matrix._33 = (float)(pow(unitVec.mZ, 2) + c - pow(unitVec.mZ, 2) * c);
  matrix._44 = 1;
  return matrix;
}

inline HSMatrix3D
HSMatrix3D::Identity()
{
  HSMatrix3D matrix;
  memset(matrix.m, 0, sizeof(float) * 16);
  matrix._11 = matrix._22 = matrix._33 = matrix._44 = 1;
  return matrix;
}

inline HSMatrix3D
HSMatrix3D::Projection(float aWidth, float aHeight, float aZNear, float aZFar)
{
  HSMatrix3D matrix;
  memset(matrix.m, 0, sizeof(float) * 16);

  matrix._11 = (2 * aZNear) / aWidth;
  matrix._22 = (2 * aZNear) / aHeight;
  matrix._33 = aZFar / (aZFar - aZNear);
  matrix._34 = 1;
  matrix._43 = - matrix._33 * aZNear;
  return matrix;
}

inline HSMatrix3D
HSMatrix3D::ProjectionFOV(float aFOV, float aAspect, float aZNear, float aZFar)
{
  HSMatrix3D matrix;
  memset(matrix.m, 0, sizeof(float) * 16);

  matrix._11 = 1 / tan(aFOV / 2);
  matrix._22 = 1 / tan((aFOV / aAspect) / 2);
  matrix._33 = aZFar / (aZFar - aZNear);
  matrix._34 = 1;
  matrix._43 = - matrix._33 * aZNear;
  return matrix;
}

inline HSMatrix3D
HSMatrix3D::Translate(float aX, float aY, float aZ)
{
  HSMatrix3D matrix;
  memset(matrix.m, 0, sizeof(float) * 16);

  matrix._11 = 1;
  matrix._22 = 1;
  matrix._33 = 1;
  matrix._41 = aX;
  matrix._42 = aY;
  matrix._43 = aZ;
  matrix._44 = 1;
  return matrix;
}
