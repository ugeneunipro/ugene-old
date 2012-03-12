// Windows-specific code, include with:
// #ifdef _MSC_VER
// #include <msvc_compat.h>
// #endif

#ifndef MSVC_COMPAT_H
#define MSVC_COMPAT_H

#include <float.h>
#define isnan _isnan
static inline int isinf(double x) {
    return ! _finite(x);
}
#include <math.h>
//#ifndef lgamma
/* Log gamma function
 * \log{\Gamma(z)}
 * AS245, 2nd algorithm, http://lib.stat.cmu.edu/apstat/245
 */
static double lgamma(double z)
{
	double x = 0;
	x += 0.1659470187408462e-06 / (z+7);
	x += 0.9934937113930748e-05 / (z+6);
	x -= 0.1385710331296526     / (z+5);
	x += 12.50734324009056      / (z+4);
	x -= 176.6150291498386      / (z+3);
	x += 771.3234287757674      / (z+2);
	x -= 1259.139216722289      / (z+1);
	x += 676.5203681218835      / z;
	x += 0.9999999999995183;
	return log(x) - 5.58106146679532777 - z + (z-0.5) * log(z+6.5);
}
//#endif

#endif

