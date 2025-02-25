/*   ColDet - C++ 3D Collision Detection Library
 *   Copyright (C) 2000   Amir Geva
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 *
 * Any comments, questions and bug reports send to:
 *   photon@photoneffect.com
 *
 * Or visit the home page: http://photoneffect.com/coldet/
 */
#include "math3d.h"

#include <algorithm>

namespace Claudette {
namespace Internal {

const Vector3D Vector3D::Zero(0.0f,0.0f,0.0f);
const Matrix3D Matrix3D::Identity(1.0f,0.0f,0.0f,0.0f,
                                  0.0f,1.0f,0.0f,0.0f,
                                  0.0f,0.0f,1.0f,0.0f,
                                  0.0f,0.0f,0.0f,1.0f);


inline float
MINOR(const Matrix3D& m, const int r0, const int r1, const int r2, const int c0, const int c1, const int c2)
{
    return m(r0,c0) * (m(r1,c1) * m(r2,c2) - m(r2,c1) * m(r1,c2)) -
            m(r0,c1) * (m(r1,c0) * m(r2,c2) - m(r2,c0) * m(r1,c2)) +
            m(r0,c2) * (m(r1,c0) * m(r2,c1) - m(r2,c0) * m(r1,c1));
}


Matrix3D
Matrix3D::Adjoint() const
{
    return Matrix3D( MINOR(*this, 1, 2, 3, 1, 2, 3),
                     -MINOR(*this, 0, 2, 3, 1, 2, 3),
                     MINOR(*this, 0, 1, 3, 1, 2, 3),
                     -MINOR(*this, 0, 1, 2, 1, 2, 3),

                     -MINOR(*this, 1, 2, 3, 0, 2, 3),
                     MINOR(*this, 0, 2, 3, 0, 2, 3),
                     -MINOR(*this, 0, 1, 3, 0, 2, 3),
                     MINOR(*this, 0, 1, 2, 0, 2, 3),

                     MINOR(*this, 1, 2, 3, 0, 1, 3),
                     -MINOR(*this, 0, 2, 3, 0, 1, 3),
                     MINOR(*this, 0, 1, 3, 0, 1, 3),
                     -MINOR(*this, 0, 1, 2, 0, 1, 3),

                     -MINOR(*this, 1, 2, 3, 0, 1, 2),
                     MINOR(*this, 0, 2, 3, 0, 1, 2),
                     -MINOR(*this, 0, 1, 3, 0, 1, 2),
                     MINOR(*this, 0, 1, 2, 0, 1, 2));
}


float
Matrix3D::Determinant() const
{
    return m[0][0] * MINOR(*this, 1, 2, 3, 1, 2, 3) -
            m[0][1] * MINOR(*this, 1, 2, 3, 0, 2, 3) +
            m[0][2] * MINOR(*this, 1, 2, 3, 0, 1, 3) -
            m[0][3] * MINOR(*this, 1, 2, 3, 0, 1, 2);
}

Matrix3D 
Matrix3D::Inverse() const
{
    return (1.0f / Determinant()) * Adjoint();
}



Vector3D *Vector3D::asPointer(float *floatPtr)
{
    return (Vector3D*)floatPtr;
}

const Vector3D *Vector3D::asConstPointer(const float *floatPtr)
{
    return (const Vector3D*)floatPtr;
}

Vector3D &Vector3D::asRef(float *floatPtr)
{
    return *(asPointer(floatPtr));
}

const Vector3D &Vector3D::asConstRef(const float *floatPtr)
{
    return *(asConstPointer(floatPtr));
}

void Vector3D::fillFloatVec(float vec[], float value)
{
    std::fill(vec, vec + 3, value);
}

void Vector3D::setFloatVec(float vec[], float x, float y, float z)
{
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
}

} // namespace Internal
} // namespace Claudette
