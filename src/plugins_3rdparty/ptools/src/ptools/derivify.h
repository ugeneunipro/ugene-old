// $Id: derivify.h 614 2008-11-25 12:26:40Z asaladin $
/****************************************************************************
 *   Copyright Joaquim R. R. A. Martins,  Peter Sturdza                     *
 *                                                                          *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                          *
 ***************************************************************************/





// NOTE: taken from website http://mdolab.utias.utoronto.ca/resources/complex-step/cpp
//
// please cite:        @article{Martins:2003:CSD, Author = {Joaquim R. R. A. Martins and Peter Sturdza and Juan J. Alonso}, Journal = {{ACM} Transactions on Mathematical Software}, Number = {3}, Pages = {245--262}, Title = {The Complex-Step Derivative Approximation}, Volume = {29}, Year = {2003}}


/***
 *  For automatic differentiation of a C/C++ code:
 *  f'(x) =  imag [ f(surreal(x,1)) ]
 *  Define a double precision class that computes and stores values
 *  and derivatives for each variable and overloads all operators.
 *  Tue Jul 18 22:12:28 PDT 2000
 ***/

#ifndef HDRderivify
#define HDRderivify
#include <iostream>
#include <math.h>

// using namespace std;


// #define AUTO_DIFF   //uncomment this line to use automatic differenciation

#ifndef HDRcomplexify
inline const double & real(const double& r) {
    /***
     *  So the real() statement can be used even with
     *  the double version of the code to be derivified,
     *  and remains compatible with complexified code, too.
     *  Most useful inside printf statements.
     ***/
    return r;
}


inline double & real(double & r) { return r;}



inline double imag(const double&) {
    return 0.;
}
#endif // HDRcomplexify


#ifdef AUTO_DIFF

class surreal {
#define surr_TEENY (1.e-24) /*machine zero compared to nominal value of val*/
    double val, deriv;

public:
    surreal(const double& v=0.0, const double& d=0.0) : val(v), deriv(d) {}
    inline surreal& operator=(const surreal & s) {val = s.val ; deriv = s.deriv; return *this;};
    operator double() const {return val;}
    operator int() const {return int(val);}

    inline friend const double & real(const surreal& z) ;  // born out of
    inline friend const double & imag(const surreal& z) ;  // complex step
    inline friend double & real(surreal& z) ;  // born out of
    inline friend double & imag(surreal& z) ;  // complex step
    // relational operators
    inline friend bool operator==(const surreal&,const surreal&);
    inline friend bool operator==(const surreal&,const double&);
    inline friend bool operator==(const double&,const surreal&);
    inline friend bool operator!=(const surreal&,const surreal&);
    inline friend bool operator!=(const surreal&,const double&);
    inline friend bool operator!=(const double&,const surreal&);
    inline friend bool operator>(const surreal&,const surreal&);
    inline friend bool operator>(const surreal&,const double&);
    inline friend bool operator>(const double&,const surreal&);
    inline friend bool operator<(const surreal&,const surreal&);
    inline friend bool operator<(const surreal&,const double&);
    inline friend bool operator<(const double&,const surreal&);
    inline friend bool operator>=(const surreal&,const surreal&);
    inline friend bool operator>=(const surreal&,const double&);
    inline friend bool operator>=(const double&,const surreal&);
    inline friend bool operator<=(const surreal&,const surreal&);
    inline friend bool operator<=(const surreal&,const double&);
    inline friend bool operator<=(const double&,const surreal&);
    // basic arithmetic
    inline surreal operator+() const;
    inline surreal operator+(const surreal&) const;
    inline surreal operator+(const double&) const;
    inline surreal operator+(const int&) const;
    inline surreal& operator+=(const surreal&);
    inline surreal& operator+=(const double&);
    inline surreal& operator+=(const int&);
    inline friend surreal operator+(const double&, const surreal&);
    inline friend surreal operator+(const int&, const surreal&);
    inline surreal operator-() const;
    inline surreal operator-(const surreal&) const;
    inline surreal operator-(const double&) const;
    inline surreal operator-(const int&) const;
    inline surreal& operator-=(const surreal&);
    inline surreal& operator-=(const double&);
    inline surreal& operator-=(const int&);
    inline friend surreal operator-(const double&, const surreal&);
    inline friend surreal operator-(const int&, const surreal&);
    inline surreal operator*(const surreal&) const;
    inline surreal operator*(const double&) const;
    inline surreal operator*(const int&) const;
    inline surreal& operator*=(const surreal&);
    inline surreal& operator*=(const double&);
    inline surreal& operator*=(const int&);
    inline friend surreal operator*(const double&, const surreal&);
    inline friend surreal operator*(const int&, const surreal&);
    inline surreal operator/(const surreal&) const;
    inline surreal operator/(const double&) const;
    inline surreal operator/(const int&) const;
    inline surreal& operator/=(const surreal&);
    inline surreal& operator/=(const double&);
    inline surreal& operator/=(const int&);
    inline friend surreal operator/(const double&, const surreal&);
    inline friend surreal operator/(const int&, const surreal&);
    // from <math.h>
    // not implemented are ldexp, frexp, modf, and fmod
    inline friend surreal fabs(const surreal&);
    inline friend surreal sin(const surreal&);
    inline friend surreal sinh(const surreal&);
    inline friend surreal asin(const surreal&);
    inline friend surreal cos(const surreal&);
    inline friend surreal cosh(const surreal&);
    inline friend surreal acos(const surreal&);
    inline friend surreal tan(const surreal&);
    inline friend surreal tanh(const surreal&);
    inline friend surreal atan(const surreal&);
    inline friend surreal atan2(const surreal&, const surreal&);
    inline friend surreal log(const surreal&);
    inline friend surreal log10(const surreal&);
    inline friend surreal sqrt(const surreal&);
    inline friend surreal exp(const surreal&);
    inline friend surreal pow(const surreal&, const surreal&);
    inline friend surreal pow(const surreal&, const double&);
    inline friend surreal pow(const surreal&, const int&);
    inline friend surreal pow(const double&, const surreal&);
    inline friend surreal pow(const int&, const surreal&);
    inline friend surreal ceil(const surreal&);
    inline friend surreal floor(const surreal&);
    // input/output
    friend istream& operator>>(istream&, surreal&);
    friend ostream& operator<<(ostream&, const surreal&);
};

inline bool operator==(const surreal& lhs, const surreal& rhs)
{
    return lhs.val == rhs.val;
}

inline bool operator==(const surreal& lhs, const double& rhs)
{
    return lhs.val == rhs;
}

inline bool operator==(const double& lhs, const surreal& rhs)
{
    return lhs == rhs.val;
}

inline bool operator!=(const surreal& lhs, const surreal& rhs)
{
    return lhs.val != rhs.val;
}

inline bool operator!=(const surreal& lhs, const double& rhs)
{
    return lhs.val != rhs;
}

inline bool operator!=(const double& lhs, const surreal& rhs)
{
    return lhs != rhs.val;
}

inline bool operator>(const surreal& lhs, const surreal& rhs)
{
    return lhs.val > rhs.val;
}

inline bool operator>(const surreal& lhs, const double& rhs)
{
    return lhs.val > rhs;
}

inline bool operator>(const double& lhs, const surreal& rhs)
{
    return lhs > rhs.val;
}

inline bool operator<(const surreal& lhs, const surreal& rhs)
{
    return lhs.val < rhs.val;
}

inline bool operator<(const surreal& lhs, const double& rhs)
{
    return lhs.val < rhs;
}

inline bool operator<(const double& lhs, const surreal& rhs)
{
    return lhs < rhs.val;
}

inline bool operator>=(const surreal& lhs, const surreal& rhs)
{
    return lhs.val >= rhs.val;
}

inline bool operator>=(const surreal& lhs, const double& rhs)
{
    return lhs.val >= rhs;
}

inline bool operator>=(const double& lhs, const surreal& rhs)
{
    return lhs >= rhs.val;
}

inline bool operator<=(const surreal& lhs, const surreal& rhs)
{
    return lhs.val <= rhs.val;
}

inline bool operator<=(const surreal& lhs, const double& rhs)
{
    return lhs.val <= rhs;
}

inline bool operator<=(const double& lhs, const surreal& rhs)
{
    return lhs <= rhs.val;
}

inline surreal surreal::operator+() const
{
    return *this;
}

inline surreal surreal::operator+(const surreal& z) const
{
    return surreal(val+z.val,deriv+z.deriv);
}

inline surreal surreal::operator+(const double& r) const
{
    return surreal(val+r,deriv);
}

inline surreal surreal::operator+(const int& i) const
{
    return surreal(val+double(i),deriv);
}

inline surreal& surreal::operator+=(const surreal& z)
{
    val+=z.val;
    deriv+=z.deriv;
    return *this;
}

inline surreal& surreal::operator+=(const double& r)
{
    val+=r;
    return *this;
}

inline surreal& surreal::operator+=(const int& i)
{
    val+=double(i);
    return *this;
}

inline surreal operator+(const double& r, const surreal& z)
{
    return surreal(r+z.val,z.deriv);
}

inline surreal operator+(const int& i, const surreal& z)
{
    return surreal(double(i)+z.val,z.deriv);
}

inline surreal surreal::operator-() const
{
    return surreal(-val,-deriv);
}

inline surreal surreal::operator-(const surreal& z) const
{
    return surreal(val-z.val,deriv-z.deriv);
}

inline surreal surreal::operator-(const double& r) const
{
    return surreal(val-r,deriv);
}

inline surreal surreal::operator-(const int& i) const
{
    return surreal(val-double(i),deriv);
}

inline surreal& surreal::operator-=(const surreal& z)
{
    val-=z.val;
    deriv-=z.deriv;
    return *this;
}

inline surreal& surreal::operator-=(const double& r)
{
    val-=r;
    return *this;
}

inline surreal& surreal::operator-=(const int& i)
{
    val-=double(i);
    return *this;
}

inline surreal operator-(const double& r, const surreal& z)
{
    return surreal(r-z.val,-z.deriv);
}

inline surreal operator-(const int& i, const surreal& z)
{
    return surreal(double(i)-z.val,-z.deriv);
}

inline surreal surreal::operator*(const surreal& z) const
{
    return surreal(val*z.val,val*z.deriv+z.val*deriv);
}

inline surreal surreal::operator*(const double& r) const
{
    return surreal(val*r,deriv*r);
}

inline surreal surreal::operator*(const int& i) const
{
    return surreal(val*double(i),deriv*double(i));
}

inline surreal& surreal::operator*=(const surreal& z)
{
    deriv=val*z.deriv+z.val*deriv;
    val*=z.val;
    return *this;
}

inline surreal& surreal::operator*=(const double& r)
{
    val*=r;
    deriv*=r;
    return *this;
}

inline surreal& surreal::operator*=(const int& i)
{
    val*=double(i);
    deriv*=double(i);
    return *this;
}

inline surreal operator*(const double& r, const surreal& z)
{
    return surreal(r*z.val,r*z.deriv);
}

inline surreal operator*(const int& i, const surreal& z)
{
    return surreal(double(i)*z.val,double(i)*z.deriv);
}

inline surreal surreal::operator/(const surreal& z) const
{
    return surreal(val/z.val,(z.val*deriv-val*z.deriv)/(z.val*z.val));
}

inline surreal surreal::operator/(const double& r) const
{
    return surreal(val/r,deriv/r);
}

inline surreal surreal::operator/(const int& i) const
{
    return surreal(val/double(i),deriv/double(i));
}

inline surreal& surreal::operator/=(const surreal& z)
{
    deriv=(z.val*deriv-val*z.deriv)/(z.val*z.val);
    val/=z.val;
    return *this;
}

inline surreal& surreal::operator/=(const double& r)
{
    val/=r;
    deriv/=r;
    return *this;
}

inline surreal& surreal::operator/=(const int& i)
{
    val/=double(i);
    val/=double(i);
    return *this;
}

inline surreal operator/(const double& r, const surreal& z)
{
    return surreal(r/z.val,-r*z.deriv/(z.val*z.val));
}

inline surreal operator/(const int& i, const surreal& z)
{
    return surreal(double(i)/z.val,-double(i)*z.deriv/(z.val*z.val));
}

inline surreal fabs(const surreal& z)
{
    return (z.val<0.0) ? -z:z;
}

inline surreal sin(const surreal& z)
{
    return surreal(sin(z.val),z.deriv*cos(z.val));
}

inline surreal sinh(const surreal& z)
{
    return surreal(sinh(z.val),z.deriv*cosh(z.val));
}

inline surreal asin(const surreal& z)
{
    // derivative trouble if z.val = +/- 1.0
    return surreal(asin(z.val),z.deriv/sqrt(1.0-z.val*z.val+surr_TEENY));
}

inline surreal cos(const surreal& z)
{
    return surreal(cos(z.val),-z.deriv*sin(z.val));
}

inline surreal cosh(const surreal& z)
{
    return surreal(cosh(z.val),z.deriv*sinh(z.val));
}

inline surreal acos(const surreal& z)
{
    // derivative trouble if z.val = +/- 1.0
    return surreal(acos(z.val),-z.deriv/sqrt(1.0-z.val*z.val+surr_TEENY));
}

inline surreal tan(const surreal& z)
{
    double cosv=cos(z.val);
    return surreal(tan(z.val),z.deriv/(cosv*cosv));
}

inline surreal tanh(const surreal& z)
{
    double coshv=cosh(z.val);
    return surreal(tanh(z.val),z.deriv/(coshv*coshv));
}

inline surreal atan(const surreal& z)
{
    return surreal(atan(z.val),z.deriv/(1.0+z.val*z.val));
}

inline surreal atan2(const surreal& z1, const surreal& z2)
{
    return surreal(atan2(z1.val,z2.val),
                   (z2.val*z1.deriv-z1.val*z2.deriv)/(z1.val*z1.val+z2.val*z2.val));
}

inline surreal log(const surreal& z)
{
    return surreal(log(z.val),z.deriv/z.val);
}

inline surreal log10(const surreal& z)
{
    return surreal(log10(z.val),z.deriv/(z.val*log(10.)));
}

inline surreal sqrt(const surreal& z)
{
    // if z.val = 0, then there is trouble with the derivative.
    // this may work if nominal z.val values are scaled properly.
    double sqrtv=sqrt(z.val);
    return surreal(sqrtv,0.5*z.deriv/(sqrtv+surr_TEENY));
}

inline surreal exp(const surreal& z)
{
    double expv=exp(z.val);
    return surreal(expv,z.deriv*expv);
}

inline surreal pow(const surreal& a, const surreal& b)
{
    // many sticky points were derivative is undefined or infinite
    // badness if 0 <= b.val < 1 and a.val == 0
    double powab=pow(a.val,b.val);
    return surreal(powab,
                   b.val*pow(a.val,b.val-1.)*a.deriv
                   +powab*log(a.val)*b.deriv);
}

inline surreal pow(const surreal& a, const double& b)
{
    return surreal(pow(a.val,b),b*pow(a.val,b-1.)*a.deriv);
}

inline surreal pow(const surreal& a, const int& b)
{
    return surreal(pow(a.val,double(b)),
                   double(b)*pow(a.val,double(b-1))*a.deriv);
}

inline surreal pow(const double& a, const surreal& b)
{
    double powab=pow(a,b.val);
    return surreal(powab,powab*log(a)*b.deriv);
}

inline surreal pow(const int& a, const surreal& b)
{
    double powab=pow(double(a),b);
    return surreal(powab,powab*log(double(a))*b.deriv);
}

inline surreal ceil(const surreal& z)
{
    return surreal(ceil(z.val),0.);
}

inline surreal floor(const surreal& z)
{
    return surreal(floor(z.val),0.);
}



inline istream& operator>>(istream& is, surreal& x)
{
    /***
     *  Straight from Stroustrup's (third edition) complex version.
     *  Much confusion about the ipfx, isfx functions used in
     *  the GNU library and the sentry type that is supposed to
     *  call them indirectly.  Stroustrup implies that this
     *  works fine when streams are tied and other fancy stuff.
     ***/

    double re, im = 0;
    char c = 0;

    is >> c;
    if (c == '(') {
        is >> re >> c;
        if (c == ',') is >> im >> c;
        if (c != ')') is.clear(ios::badbit);
    }
    else {
        is.putback(c);
        is >> re;
    }

    if (is) x = surreal (re, im);
    return is;
}

inline ostream& operator<<(ostream& os, const surreal& z)
{
    return os << '(' << real (z) << ',' << imag (z) << ')';
}



inline const double & real(const surreal& z) {return z.val;}  // born out of
inline const double & imag(const surreal& z) {return z.deriv;}// complex step
inline double & real(surreal& z) {return z.val;}  // born out of
inline double & imag(surreal& z) {return z.deriv;}// complex step


#endif // #ifdef AUTO_DIFF


#undef surr_TEENY
#endif

