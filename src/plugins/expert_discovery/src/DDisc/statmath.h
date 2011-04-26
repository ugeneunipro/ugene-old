//****************************************************************************
//	File name:			StatMath.h
//	Title:				Библиотека компонентов DiscoveryLib
//	Version:			1.0
//	Author:				Шипилов Тимур Игоревич
//	Creation Date:		26.02.2005 г.
//
//	Description: Некоторые формулы из математической статистики
//
//****************************************************************************


#pragma once

#include <math.h>

#define CORELATION_UNDEFINED -2

#ifndef min
#define min(a,b) (a<b)?a:b
#endif

namespace DDisc {
	inline double factln(int ifak) {
		const double faktor[] = { 0.0, 0.0, 0.692, 1.791, 3.177, 4.787,
			6.579, 8.525, 10.604, 12.802, 15.104 };
		if (ifak <= 10) {
			return faktor[ifak];
		} else {
			return ifak * log((double)ifak) + 0.5 * (1.8374 + log((double)ifak))
				- ifak + 1. / (12. * ifak) - 1. / (360. * ifak * ifak);
		}
	}

	inline double fisher(int a, int b, int c, int d) {
		double result = 0;

		int n = a + c + b + d;
		if (n == 0) {
			return 1; // NO DATA
		}
		double constant = factln(a + b) + factln(c + d) + factln(a + c)
			+ factln(b + d) - factln(n);

		int m00 = a;
		int m01 = b;
		int m10 = c;
		int m11 = d;
		if (c * b > a * d) {
			m00 = c;
			m01 = d;
			m10 = a;
			m11 = b;
		}
		int lowestNum = min(m01, m10);
		for (int i1 = 0; i1 <= lowestNum; i1++) {
			double variable = -factln(m11) - factln(m01) - factln(m10)
				- factln(m00);
			result += exp(constant + variable);

			m11 = m11 + 1;
			m01 = m01 - 1;
			m10 = m10 - 1;
			m00 = m00 + 1;
		}
		return result;
	}

	inline double corelation(int a, int b, int c, int d) {
		double dSqrt = sqrt((double)(b + d) * (a + c) * (c + d) * (a + b));
		if (dSqrt != 0)
			return (a * d - c * b) / dSqrt;
		else
			return CORELATION_UNDEFINED;
	}

	inline double ul(int a, int b, int c, int d) {
		if (b == 0 || c == 0) {
			return 1;
		} else if (a == 0 || d == 0) {
			return -1;
		}
		double Q = (a * d - b * c) / (double) (a * d + b * c);
		double D = 0.5 * (1 - Q * Q)
			* sqrt((double)(1.0 / a) + (1.0 / b) + (1.0 / c) + (1.0 / d));
		double Q005 = Q - 1.96 * D;
		return Q005;
	}
}