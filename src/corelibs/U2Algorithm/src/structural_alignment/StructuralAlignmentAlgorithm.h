/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_H_
#define _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_H_

#include <U2Core/global.h>
#include <U2Core/Matrix44.h>

namespace U2 {

class BioStruct3D;

class U2ALGORITHM_EXPORT StructuralAlignment {
public:
    double rmsd;
    Matrix44 transform;
};  // class StructuralAlignment

class U2ALGORITHM_EXPORT StructuralAlignmentAlgorithm {
public:
    virtual StructuralAlignment align(const BioStruct3D &ref, const BioStruct3D &alt, int refModel = 0, int altModel = 0) = 0;
};  // class StructuralAlignmentAlgorithm

}   // namespace U2

#endif  // #ifndef _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_H_
