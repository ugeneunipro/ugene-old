// $Id: basetypes.h 648 2008-12-23 11:05:27Z asaladin $
/****************************************************************************
 *   Copyright (C) 2006-2008   Adrien Saladin                               *
 *   adrien.saladin@gmail.com                                               *
 *   Copyright (C) 2008   Pierre Poulain                                    *
 *   Copyright (C) 2008   Sebastien Fiorucci                                *
 *   Copyright (C) 2008   Chantal Prevost                                   *
 *   Copyright (C) 2008   Martin Zacharias                                  *
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


#ifndef BASETYPES_H
#define BASETYPES_H


#include <vector>
#include <iostream>

#include <sstream>
// #include <iomanip>   //setw, setprecision

#include <cassert>
#include <cstdio>
//#include <boost/shared_array.hpp> // removed, replaced with QVector
#include <QtCore/QVector>
#include <stdexcept>


//switch to choose between automatic differenciation
//or normal double arithmetic




#include "derivify.h"  //must be included in any cases

#ifdef AUTO_DIFF
typedef surreal dbl;
#else
typedef double dbl;
#endif


typedef unsigned int uint;
typedef std::vector<int> Vint;
typedef std::vector<uint> Vuint;
typedef std::vector<dbl> Vdouble;

#ifndef NDEBUG
#define Debug(func) func
#else
#define Debug(func)
#endif



// power (of int) faster much faster than pow()
// expands pow<6>(a) to a*a*a*a*a*a
template<size_t I,size_t N> struct metapow
{
    static inline dbl Pow( dbl x )
    {
        return x*metapow<I+1,N>::Pow(x);
    }
};

template<size_t N> struct metapow<N,N>
{
    static inline dbl Pow( dbl x ) { return 1; }
};

template<size_t N> dbl pow(dbl x)
{
    return metapow<0,N>::Pow(x);
}











/*! \brief 2-dimensional Object Oriented array
*
*  this class provides a 2-dimensional OO array. Memory is managed automatically.
*  The size of the array cannot be modified after creation.
*  It should be compatible with functions that needs raw pointers (with few modifications of the object).
*  WARNING: the assignemnt like 'Array2D array2=array1;' or 'Array2D array2(array1)' leads to two objects
*  sharing the same memory (this reduce the cost of copying and transmission by value,
*  like for Python objects). To make array2 independant of array1 (deep copy), use: array2.detach() after the
*  copy.
*  It should be reasonably fast for most usages
*/
template <class mytype>
class Array2D
{
public:
    Array2D(){m_rows=0; m_columns=0;};
    Array2D(int row, int columns)
        :   msa_data(),
            m_rows(row),
            m_columns(columns)
    {
        m_size = row*columns;
        //msa_data  = boost::shared_array<mytype>( new mytype[m_size]);
        msa_data.resize(m_size);
    }

    /// index operator (fortran-like syntax). a(0,2) gives raw 1, column 3.
    mytype& operator() (int r, int c)
    {
        assert(r<m_rows);
        assert(c<m_columns);
        return msa_data[r*m_columns+c];

    }

    /// get or set an element of the array
    const mytype& operator() (int r, int c) const
    {
        assert(r<m_rows);
        assert(c<m_columns);
        return msa_data[r*m_columns+c];
    }


    /* // seems to not used anywhere else
    const void * id() {
        return (void *) &msa_data[0];
    }*/

    /* // seems to not used anywhere else
    /// "deep copy" of the array
    void detach()
    {
        mytype * olddata = msa_data.get();
        mytype * newdata = new mytype[m_size];
        memcpy( newdata, olddata, m_size*sizeof(mytype) );
        msa_data=boost::shared_array<mytype>(newdata);
    }*/


    std::string str()
    {
        std::ostringstream oss;
        for (int r=0; r<m_rows; r++)
        {
            for (int c=0; c<m_columns; c++)
            oss << (*this)(r,c) << "  " ;
            oss << "\n";
        }

    return oss.str();
    }

    ///get size of the array
    std::pair<int,int> getDim()
    {
       std::pair<int,int> out;
       out.first = m_rows;
       out.second = m_columns;
       return out;
    }


    // suppressed stdout/stderr output by UGENE
    /*void Print()
    {
       std::cout << str();
    }*/




    bool almostEqual(const Array2D<mytype> & tocompare, dbl difference)
    {
        if (!( this-> m_rows == tocompare.m_rows && this-> m_columns==tocompare.m_columns))
             throw std::invalid_argument("Arrays must have the same size to be compared !\n");

        for (uint i=0; i<m_size; i++)
           if (  fabs(real(msa_data[i] - tocompare.msa_data[i])) > real(difference)) return false;

        return true;
    }

private:
    //boost::shared_array<mytype> msa_data;
    QVector<mytype> msa_data;
    int m_rows;
    int m_columns;
    size_t m_size;

} ;


typedef Array2D<dbl> Matrix;

void MakeIdentity(Matrix & mat);

struct Superpose_t
{
    dbl rmsd;
    Matrix matrix;
};





#endif




