#pragma once
#ifdef FastQuadricSimplification_EXPORTS
#define FASTQUADRICSIMPLIFICATION_API __declspec(dllexport)
#else
#define FASTQUADRICSIMPLIFICATION_API __declspec(dllimport)
#endif
#include "VecMath.h"


class FASTQUADRICSIMPLIFICATION_API SymetricMatrix
{
private:
	double m[10];
public:

	// Constructor
	SymetricMatrix(double c = 0);

	SymetricMatrix(double m11, double m12, double m13, double m14,
		double m22, double m23, double m24,
		double m33, double m34,
		double m44);

	// Make plane

	SymetricMatrix(double a, double b, double c, double d);

	double operator[](int c) const;

	// Determinant

	double det(int a11, int a12, int a13,
		int a21, int a22, int a23,
		int a31, int a32, int a33);

	const SymetricMatrix operator+(const SymetricMatrix& n) const;

	SymetricMatrix& operator+=(const SymetricMatrix& n);
};