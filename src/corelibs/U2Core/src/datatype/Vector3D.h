#ifndef _U2_VECTOR_MATH_H_
#define _U2_VECTOR_MATH_H_

#include <QtGlobal>
#include <math.h>
#include <U2Core/global.h>

namespace U2
{

class U2CORE_EXPORT Vector3D {
public:
    double x, y, z;

    explicit Vector3D(double xi = 0.0, double yi = 0.0, double zi = 0.0);
    Vector3D(const Vector3D& v);
    Vector3D& operator = (const Vector3D& v);
    bool operator == (const Vector3D& other);
    bool operator != (const Vector3D& other);
    double& operator [] (unsigned int i);
    double operator [] (unsigned int i) const;
    Vector3D& operator += (const Vector3D& v);
    Vector3D& operator -= (const Vector3D& v);
    Vector3D& operator *= (double f);
    Vector3D& operator /= (double f);
    void set(double xs, double ys, double zs);
    double length(void) const;
    double lengthSquared(void) const;
    void normalize(void);
};

U2CORE_EXPORT Vector3D operator - (const Vector3D& a);
U2CORE_EXPORT Vector3D operator + (const Vector3D& a, const Vector3D& b);
U2CORE_EXPORT Vector3D operator - (const Vector3D& a, const Vector3D& b);
U2CORE_EXPORT double vector_dot(const Vector3D& a, const Vector3D& b);
U2CORE_EXPORT Vector3D vector_cross(const Vector3D& a, const Vector3D& b);
U2CORE_EXPORT Vector3D operator * (const Vector3D& v, double f);
U2CORE_EXPORT Vector3D operator * (double f, const Vector3D& v);
U2CORE_EXPORT Vector3D operator / (const Vector3D& v, double f);




}//namespace

#endif //_U2_VECTOR_MATH_H_
