/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "MSADistanceAlgorithmSimilarity.h"

#include <U2Core/MAlignment.h>

namespace U2 {

MSADistanceAlgorithmFactorySimilarity::MSADistanceAlgorithmFactorySimilarity(QObject* p) 
: MSADistanceAlgorithmFactory(BuiltInDistanceAlgorithms::SIMILARITY_ALGO, DistanceAlgorithmFlags_NuclAmino, p)
{

}


QString MSADistanceAlgorithmFactorySimilarity::getDescription() const {
    return tr("Based on similarity distance between two sequences");
}

QString MSADistanceAlgorithmFactorySimilarity::getName() const {
    return tr("Simple identity");
}


MSADistanceAlgorithm* MSADistanceAlgorithmFactorySimilarity::createAlgorithm(const MAlignment& ma, QObject* ) {
    MSADistanceAlgorithm* res = new MSADistanceAlgorithmSimilarity(this, ma);
    if(flags.testFlag(DistanceAlgorithmFlag_ExcludeGaps)){
        res->setExcludeGaps(true);
    }else{
        res->setExcludeGaps(false);
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////
// Algorithm

void MSADistanceAlgorithmSimilarity::run() {
    int nSeq = ma.getNumRows();
    for (int i = 0; i < nSeq; i++) {
        for (int j = i; j < nSeq; j++) {
            int sim = 0;
            for (int k = 0; k < ma.getLength(); k++) {
                bool similar = (ma.charAt(i, k) == ma.charAt(j, k));

                if(!excludeGaps){
                    if (similar) sim++;
                }else{
                    if (similar && ma.charAt(i, k)!=MAlignment_GapChar) sim++;
                }
                
            }
            lock.lock();
            distanceTable[i][j] = distanceTable[j][i] = sim;
            lock.unlock();
        }
        stateInfo.setProgress(i * 100 / nSeq);
    }
}

} //namespace
