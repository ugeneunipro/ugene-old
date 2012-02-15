/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifdef SW2_BUILD_WITH_SSE2

#ifndef _SMITHWATERMANALGORITHM_SSE2_H
#define _SMITHWATERMANALGORITHM_SSE2_H

#include "SmithWatermanAlgorithm.h"

#include <emmintrin.h>

namespace U2 {

class SmithWatermanAlgorithmSSE2 : public SmithWatermanAlgorithm {
public:
    typedef qint16 ScoreType;

    virtual void launch(const SMatrix& substitutionMatrix, QByteArray const & _patternSeq, QByteArray const & _searchSeq, int _gapOpen, int _gapExtension, int _minScore);
    


private:
    static const int nElementsInVec = 8;
    void printVector(__m128i &toprint, int add);
    void calculateMatrix();
    int calculateMatrixSSE2(unsigned queryLength, unsigned char *dbSeq, unsigned dbLength, unsigned short gapOpenOrig, unsigned short gapExtend);

    ScoreType ** matrix;    

    ScoreType maximumScore;
    
};


} // namespace

#endif
#endif //SW2_BUILD_WITH_SSE2
