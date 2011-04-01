// $Id: screw.h 644 2008-12-22 15:05:30Z asaladin $
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


#ifndef SCREW_H
#define SCREW_H

#include <string>
#include <sstream>

#include "coord3d.h"

namespace PTools{

struct Screw
{
    Coord3D unitVector;  ///< unit vector of the translation
    dbl normtranslation; ///< translation norm
    Coord3D point; ///< a point of the rotation axis (to generate the rotation axis)
    dbl angle; ///< angle of rotation

    /// returns a summary of the screw:
    std::string print()
    {
        std::stringstream result;
        result << "rotation axis: " << unitVector.toString() ;
        result << "angle: " << angle << "\n" ;
        result << "translation norm: " <<  Norm(unitVector) << "\n" ;
        result << "axis point: "  <<  point.toString();
        return result.str();
    }
};


} //namespace PTools

#endif //SCREW_H


