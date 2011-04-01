// $Id: coordsarray.h 614 2008-11-25 12:26:40Z asaladin $
/****************************************************************************
 *   Copyright (C) 2008   Adrien Saladin                                    *
 *   adrien.saladin@gmail.com                                               *
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

#ifndef _COORDS_ARRAY_H_
#define _COORDS_ARRAY_H_

#include <vector>
#include<stdexcept>


#include "coord3d.h"

namespace PTools{


typedef std::vector<Coord3D> VCoords;


inline void matrix44xVect(const dbl mat[ 4 ][ 4 ], const Coord3D& vect, Coord3D& out )
{
    out.x = vect.x * mat[ 0 ][ 0 ] + vect.y * mat[ 0 ][ 1 ] + vect.z * mat[ 0 ][ 2 ] + mat[ 0 ][ 3 ] ;
    out.y = vect.x * mat[ 1 ][ 0 ] + vect.y * mat[ 1 ][ 1 ] + vect.z * mat[ 1 ][ 2 ] + mat[ 1 ][ 3 ] ;
    out.z = vect.x * mat[ 2 ][ 0 ] + vect.y * mat[ 2 ][ 1 ] + vect.z * mat[ 2 ][ 2 ] + mat[ 2 ][ 3 ] ;
}




class CoordsArray
{



private:  //private data

    /* don't forget the constructors if you add some private data ! */
    VCoords _refcoords;
    mutable VCoords _movedcoords;
    dbl mat44[4][4]; // 4x4 matrix

    mutable bool _uptodate ;

    mutable void (CoordsArray::*_getcoords)(const uint i, Coord3D& co) const; //C++ member function pointer. Points to a CoordArray function that takes a const uint and a Coord3D& and returns void.




private: //private methods



    void _modified() { _uptodate = false; _getcoords = & CoordsArray::_safegetcoords;  }; // call this function when _movedcoords needs an update before getting real coordinates

    void _safegetcoords(const uint i, Coord3D& co) const {

        assert(_refcoords.size() == _movedcoords.size());

        for (uint j=0; j<_refcoords.size(); j++)
        {
            matrix44xVect(mat44, _refcoords[j], _movedcoords[j]);
        }

        _uptodate = true;
        //modify the function pointer _getcoords to call the "unsafe" method next time (faster)
        _getcoords = & CoordsArray::unsafeGetCoords;
        (*this.* _getcoords)(i, co); //return the correct function

    };



public:


    CoordsArray(); //constructor
    CoordsArray(const CoordsArray & ca); //copy constructor

    /// get the cached coordinates. You must ensure that update() has been called first !
    void inline unsafeGetCoords(const uint i, Coord3D& co) const { co = _movedcoords[i];};

    void AddCoord(const Coord3D& co) {_refcoords.push_back(co); _movedcoords.push_back(co);  _modified();  };
    uint Size() const {return _refcoords.size();};


    void GetCoords(const uint i, Coord3D& co)  const throw(std::out_of_range) ;

    void SetCoords(const uint k, const Coord3D& co);

    /// Translate the whole object
    void Translate(const Coord3D& tr);
     /// Euler Rotation
    void AttractEulerRotate(dbl phi, dbl ssi, dbl rot);

    void ResetMatrix();

    // unused by UGENE
    //std::string PrintMatrix() const;

    ///return the rotation/translation matrix
    Matrix GetMatrix() const;



protected:

    void MatrixMultiply(const dbl mat[4][4]);


};




} //namespace PTools




#endif // _COORDS_ARRAY_H_







