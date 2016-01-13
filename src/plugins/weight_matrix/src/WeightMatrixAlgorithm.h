/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_WEIGHT_MATRIX_ALGORITHM_H_
#define _U2_WEIGHT_MATRIX_ALGORITHM_H_

#include <U2Algorithm/PWMConversionAlgorithm.h>

#include <U2Core/DNATranslation.h>

namespace U2 {

enum MatrixBuldTarget {
    FREQUENCY_MATRIX,
    WEIGHT_MATRIX
};

enum MatrixBuildType {
    PM_MONONUCLEOTIDE,
    PM_DINUCLEOTIDE
};

class PMBuildSettings {
public:
    PMBuildSettings(): type(PM_MONONUCLEOTIDE), target(FREQUENCY_MATRIX) {

    }
    QString                     algo;
    MatrixBuildType             type;
    MatrixBuldTarget            target;
};

class WeightMatrixAlgorithm : public QObject {
    Q_OBJECT
public:
    static float getScore(const char* seq, int len, const PWMatrix& m, DNATranslation* complMap);
};

} //namespace

#endif
