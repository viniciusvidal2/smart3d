#include "SymetricMatrix.h"


SymetricMatrix::SymetricMatrix(double c)
{
	loopi(0, 10) m[i] = c;
}

SymetricMatrix::SymetricMatrix(double m11, double m12, double m13, double m14,
	double m22, double m23, double m24,
	double m33, double m34,
	double m44)
{
	m[0] = m11;  m[1] = m12;  m[2] = m13;  m[3] = m14;
	m[4] = m22;  m[5] = m23;  m[6] = m24;
	m[7] = m33;  m[8] = m34;
	m[9] = m44;
}

SymetricMatrix::SymetricMatrix(double a, double b, double c, double d)
{
	m[0] = a * a;  m[1] = a * b;  m[2] = a * c;  m[3] = a * d;
	m[4] = b * b;  m[5] = b * c;  m[6] = b * d;
	m[7] = c * c; m[8] = c * d;
	m[9] = d * d;
}

double SymetricMatrix::operator[](int c) const { return m[c]; }

double SymetricMatrix::det(int a11, int a12, int a13,
	int a21, int a22, int a23,
	int a31, int a32, int a33)
{
	double det = m[a11] * m[a22] * m[a33] + m[a13] * m[a21] * m[a32] + m[a12] * m[a23] * m[a31]
		- m[a13] * m[a22] * m[a31] - m[a11] * m[a23] * m[a32] - m[a12] * m[a21] * m[a33];
	return det;
}

const SymetricMatrix SymetricMatrix::operator+(const SymetricMatrix& n) const
{
	return SymetricMatrix(m[0] + n[0], m[1] + n[1], m[2] + n[2], m[3] + n[3],
		m[4] + n[4], m[5] + n[5], m[6] + n[6],
		m[7] + n[7], m[8] + n[8],
		m[9] + n[9]);
}

SymetricMatrix& SymetricMatrix::operator+=(const SymetricMatrix& n)
{
	m[0] += n[0];   m[1] += n[1];   m[2] += n[2];   m[3] += n[3];
	m[4] += n[4];   m[5] += n[5];   m[6] += n[6];   m[7] += n[7];
	m[8] += n[8];   m[9] += n[9];
	return *this;
}
