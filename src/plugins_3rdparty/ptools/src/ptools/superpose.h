// $Id: superpose.h 644 2008-12-22 15:05:30Z asaladin $
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

#ifndef _SUPERPOSE_H_
#define _SUPERPOSE_H_

#include <iostream>

#include "screw.h"

#include "rigidbody.h"


namespace PTools
{

typedef double Mat33[3][3];
typedef double Mat44[4][4];


void Rotate(Rigidbody& rigid, Mat33 mat);

void Mat44toMat33trans(Mat44 mat44, Mat33 mat33, Coord3D& trans);
Screw MatTrans2screw(const Matrix& mat); // transforme t(r(X)) en un vissage d'axe de rotation colineaire au vecteur translation.

Superpose_t superpose(const Rigidbody& ref, const Rigidbody& mob, int verbosity=0);

}

#endif




