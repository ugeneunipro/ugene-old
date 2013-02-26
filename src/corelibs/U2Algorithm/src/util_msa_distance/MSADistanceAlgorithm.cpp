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

#include "MSADistanceAlgorithm.h" 

#include <U2Core/DNAAlphabet.h>
#include <U2Core/MAlignment.h>

#include <QtCore/QVector>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
// Factory    

MSADistanceAlgorithmFactory::MSADistanceAlgorithmFactory(const QString& algoId, DistanceAlgorithmFlags _flags, QObject* p)
: QObject(p), algorithmId(algoId), flags(_flags)
{
}

DistanceAlgorithmFlags MSADistanceAlgorithmFactory::getAphabetFlags(const DNAAlphabet* al) {
    if (al->getType() == DNAAlphabet_AMINO) {
        return DistanceAlgorithmFlag_Amino;
    } else if (al->getType() == DNAAlphabet_NUCL) {
        return DistanceAlgorithmFlag_Nucleic;
    } 
    assert(al->getType() ==  DNAAlphabet_RAW);
    return DistanceAlgorithmFlag_Raw;
}

void MSADistanceAlgorithmFactory::setFlag( DistanceAlgorithmFlag flag ){
    flags |= flag; 
}

void MSADistanceAlgorithmFactory::resetFlag( DistanceAlgorithmFlag flag ){
    if(flags.testFlag(flag)){
        flags ^= flag;
    }
}

//////////////////////////////////////////////////////////////////////////
// Algorithm

MSADistanceAlgorithm::MSADistanceAlgorithm(MSADistanceAlgorithmFactory* _factory, const MAlignment& _ma)
: Task(tr("MSA distance algorithm \"%1\" task").arg(_factory->getName()), TaskFlag_None)
, factory(_factory)
, ma(_ma)
, excludeGaps(true)
, isSimilarity(true)
{    
    for (int i = 0; i < ma.getNumRows(); i++) {
        distanceTable.append(QVarLengthArray<int>(ma.getNumRows()));
        qMemSet(distanceTable[i].data(), 0, ma.getNumRows() * sizeof(int));
    }    
}

int MSADistanceAlgorithm::getSimilarity(int row1, int row2) {    
    lock.lock();
    int res = distanceTable[row1][row2];
    lock.unlock();
    return res;    
}
void MSADistanceAlgorithm::fillTable() {
    int nSeq = ma.getNumRows();
    for (int i = 0; i < nSeq; i++) {
        for (int j = i; j < nSeq; j++) {
            int sim = calculateSimilarity(i, j);
            lock.lock();
            distanceTable[i][j] = distanceTable[j][i] = sim;
            lock.unlock();
        }
    }
}


MSADistanceMatrix::MSADistanceMatrix(const MSADistanceAlgorithm *algo, bool _usePercents) 
: distanceTable(algo->distanceTable), percentsTable(algo->distanceTable), usePercents(_usePercents) {
    int i = 0;
    foreach(QString seqName, algo->ma.getRowNames()) {
        namesAndIndexes[seqName] = i++;
    }
    int nSeq = algo->ma.getNumRows();
    for (int i = 0; i < nSeq; i++) {
        for (int j = i; j < nSeq; j++) {
            int len1 = algo->ma.getRow(i).getUngappedLength();
            int len2 = algo->ma.getRow(j).getUngappedLength();
            int minLen = qMin(len1, len2);
            percentsTable[i][j] = percentsTable[j][i] = qRound(distanceTable[i][j] * 100.0 / minLen);
        }
    }
}

int MSADistanceMatrix::getSimilarity(int row1, int row2) {    
    if(usePercents) {
        return percentsTable[row1][row2];
    }
    else {
        return distanceTable[row1][row2];
    }
}

int MSADistanceMatrix::getSimilarity(const QString& firstSeqName, const QString& secondSeqName) {
    if(namesAndIndexes.contains(firstSeqName) && namesAndIndexes.contains(secondSeqName)) {
        return getSimilarity(namesAndIndexes[firstSeqName], namesAndIndexes[secondSeqName]);
    }
    else {
        return -1;
    }
}
void MSADistanceMatrix::sl_onSequenceNameChanged(QString oldSeqName, QString newSeqName) {
    if(namesAndIndexes.contains(oldSeqName)) {
        namesAndIndexes[newSeqName] = namesAndIndexes[oldSeqName];
        namesAndIndexes.remove(oldSeqName);
    }
}

} //namespace
