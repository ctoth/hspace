/**
 *
 * Hemlock Space 5 (HSpace 5)
 * Copyright (c) 2009, Bas Schouten
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
#include "HSSphere.h"
#include "HSVertexBuffer.h"
#include "HSRenderer.h"

#include <math.h>

struct triangle
{
  HSVector3D point1;
  HSVector3D point2;
  HSVector3D point3;
};

const double t = (1+sqrt((float)5)) / 2;
const double tau = t / sqrt(1 + t*t);
const double one = 1 / sqrt(1 + t*t);

HSVector3D icosahedr_vecs[] =
{ 
  HSVector3D(tau, one, 0.0),
  HSVector3D(-tau, one, 0.0),
  HSVector3D(-tau, -one, 0.0),
  HSVector3D(tau, -one, 0.0),
  HSVector3D(one, 0.0, tau),
  HSVector3D(one, 0.0, -tau),
  HSVector3D(-one, 0.0, -tau),
  HSVector3D(-one, 0.0, tau),
  HSVector3D(0.0, tau, one),
  HSVector3D(0.0, -tau, one),
  HSVector3D(0.0, -tau, -one),
  HSVector3D(0.0, tau, -one)
};

triangle icosahedron_triangles[] =
{
  { icosahedr_vecs[4], icosahedr_vecs[8], icosahedr_vecs[7] },
  { icosahedr_vecs[4], icosahedr_vecs[7], icosahedr_vecs[9] },
  { icosahedr_vecs[5], icosahedr_vecs[6], icosahedr_vecs[11] },
  { icosahedr_vecs[5], icosahedr_vecs[10], icosahedr_vecs[6] },
  { icosahedr_vecs[0], icosahedr_vecs[4], icosahedr_vecs[3] },
  { icosahedr_vecs[0], icosahedr_vecs[3], icosahedr_vecs[5] },
  { icosahedr_vecs[2], icosahedr_vecs[7], icosahedr_vecs[1] },
  { icosahedr_vecs[2], icosahedr_vecs[1], icosahedr_vecs[6] },
  { icosahedr_vecs[8], icosahedr_vecs[0], icosahedr_vecs[11] },
  { icosahedr_vecs[8], icosahedr_vecs[11], icosahedr_vecs[1] },
  { icosahedr_vecs[9], icosahedr_vecs[10], icosahedr_vecs[3] },
  { icosahedr_vecs[9], icosahedr_vecs[2], icosahedr_vecs[10] },
  { icosahedr_vecs[8], icosahedr_vecs[4], icosahedr_vecs[0] },
  { icosahedr_vecs[11], icosahedr_vecs[0], icosahedr_vecs[5] },
  { icosahedr_vecs[4], icosahedr_vecs[9], icosahedr_vecs[3] },
  { icosahedr_vecs[5], icosahedr_vecs[3], icosahedr_vecs[10] },
  { icosahedr_vecs[7], icosahedr_vecs[8], icosahedr_vecs[1] },
  { icosahedr_vecs[6], icosahedr_vecs[1], icosahedr_vecs[11] },
  { icosahedr_vecs[7], icosahedr_vecs[2], icosahedr_vecs[9] },
  { icosahedr_vecs[6], icosahedr_vecs[10], icosahedr_vecs[2] }
};

HSSphere::HSSphere(HSRenderer *aRenderer)
  : mVB(NULL)
  , mRenderer(aRenderer)
  , mRadius(0.0)
{
}

HSSphere::~HSSphere(void)
{
}

void
HSSphere::SetRadius(double aRadius)
{
  mRadius = aRadius;
  UpdateVB();
}

void
HSSphere::SetColor(const HSColor &aColor)
{
  mColor = aColor;
  UpdateVB();
}

void
HSSphere::UpdateVB()
{
  delete mVB;
  unsigned int numTriangles = sizeof(icosahedron_triangles) / sizeof(triangle);
  triangle *triangles = new triangle[numTriangles];
  memcpy(triangles, icosahedron_triangles, sizeof(icosahedron_triangles));

  // TODO: Intelligently decide on detail level.
  IncreaseDetail(&triangles, &numTriangles);
  IncreaseDetail(&triangles, &numTriangles);
  IncreaseDetail(&triangles, &numTriangles);

  for (unsigned int i = 0; i < numTriangles; i++) {
    PushVertices(triangles + i);
  }
  mVB = mRenderer->CreateVertexBuffer(HSVT_COLORED, numTriangles * 3);

  if (!mVB) {
    return;
  }
  HSVertexColored *vertices = 
    static_cast<HSVertexColored*>(mVB->Lock());
  for (unsigned int i = 0; i < numTriangles; i++) {
    vertices[i * 3].v = (triangles[i].point1).RenderVector();
    vertices[i * 3].color = mColor.ARGB();
    vertices[i * 3 + 1].v = (triangles[i].point2).RenderVector();
    vertices[i * 3 + 1].color = mColor.ARGB();
    vertices[i * 3 + 2].v = (triangles[i].point3).RenderVector();
    vertices[i * 3 + 2].color = mColor.ARGB();
  }
  mVB->Unlock();
  delete [] triangles;
  mTriangles = numTriangles;
}

void
HSSphere::Draw()
{
  mRenderer->SetVertexFormat(HSVT_COLORED);
  mRenderer->SetStreamSource(mVB);
  mRenderer->DrawPrimitive(HSPRIM_TRIANGLELIST, 0, mTriangles);
}

void
HSSphere::PushVertices(triangle *aTriangle)
{
  aTriangle->point1 *= mRadius / aTriangle->point1.length();  
  aTriangle->point2 *= mRadius / aTriangle->point2.length();  
  aTriangle->point3 *= mRadius / aTriangle->point3.length();  
}

void
HSSphere::TesselateTriangle(const triangle &aTriangle, triangle *aResult)
{
  HSVector3D newVertice1 = aTriangle.point1 + (aTriangle.point2 - aTriangle.point1) * 0.5;
  HSVector3D newVertice2 = aTriangle.point2 + (aTriangle.point3 - aTriangle.point2) * 0.5;
  HSVector3D newVertice3 = aTriangle.point3 + (aTriangle.point1 - aTriangle.point3) * 0.5;

  aResult[0].point1 = aTriangle.point1;
  aResult[0].point2 = newVertice1;
  aResult[0].point3 = newVertice3;
  aResult[1].point1 = newVertice3;
  aResult[1].point2 = newVertice2;
  aResult[1].point3 = aTriangle.point3;
  aResult[2].point1 = newVertice3;
  aResult[2].point2 = newVertice1;
  aResult[2].point3 = newVertice2;
  aResult[3].point1 = newVertice1;
  aResult[3].point2 = aTriangle.point2;
  aResult[3].point3 = newVertice2;
}

void
HSSphere::IncreaseDetail(triangle **aTriangles, unsigned int *aTriangleCount)
{
  triangle *newTriangles = new triangle[*aTriangleCount * 4];
  for (unsigned int i = 0; i < *aTriangleCount; i++) {
    PushVertices(*aTriangles + i);
    TesselateTriangle((*aTriangles)[i], newTriangles + i * 4);
  }
  delete [] *aTriangles;
  *aTriangles = newTriangles;
  *aTriangleCount *= 4;
}