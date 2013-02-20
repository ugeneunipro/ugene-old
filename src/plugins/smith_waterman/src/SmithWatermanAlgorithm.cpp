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

#include "SmithWatermanAlgorithm.h"
#include <U2Core/U2Region.h>
#include <U2Core/Timer.h>
#include <U2Core/Log.h>

namespace U2 {

const char SmithWatermanAlgorithm::STOP = 's';
const char SmithWatermanAlgorithm::UP = 'u';
const char SmithWatermanAlgorithm::LEFT = 'l';
const char SmithWatermanAlgorithm::DIAG = 'd';

SmithWatermanAlgorithm::SmithWatermanAlgorithm() {    
//default values

    gapOpen = 0;
    gapExtension = 0;
    minScore = 0;
    matrixLength = 0;
}

quint64 SmithWatermanAlgorithm::estimateNeededRamAmount(const qint32 gapOpen, const qint32 gapExtension,
                                                        const quint32 minScore, const quint32 maxScore,
                                                        QByteArray const & patternSeq, QByteArray const & searchSeq,
                                                        const SmithWatermanSettings::SWResultView resultView) {
    const double b_to_mb_factor = 1048576.0;

    const quint64 queryLength = patternSeq.length();
    const quint64 searchLength = searchSeq.length();

    const qint32 maxGapPenalty = (gapOpen > gapExtension) ? gapOpen : gapExtension;
    assert(0 > maxGapPenalty);

    quint64 matrixLength = queryLength - (maxScore - minScore) / maxGapPenalty + 1;    

    if (searchLength + 1 < matrixLength) {
        matrixLength = searchLength + 1;
    }
    
    const quint64 maxPairSequencesSize = (searchLength + 1) * sizeof(PairAlignSequences);
    const quint64 matrixSize = matrixLength * (queryLength + 2) * sizeof(int);
    const quint64 EMatrixSize = (queryLength + 2) * sizeof(int);
    const quint64 FMatrixSize = (queryLength + 2) * sizeof(int);

    quint64 directionArraySize = 0;
    
    if(SmithWatermanSettings::ANNOTATIONS == resultView) {
       directionArraySize = 2 * (queryLength + 2) * sizeof(int);
    } else if(SmithWatermanSettings::MULTIPLE_ALIGNMENT == resultView) {
        directionArraySize = matrixLength * (queryLength + 2) * sizeof(char);
    } else {
        assert(0);
    }
    
    const quint64 memToAllocInBytes = (queryLength + 2) * ((sizeof(int) * 3)  + sizeof(char)) +
        maxPairSequencesSize + matrixSize + EMatrixSize + FMatrixSize + directionArraySize;

    return memToAllocInBytes / b_to_mb_factor;
}

//notes: Gap < 0
bool SmithWatermanAlgorithm::calculateMatrixLength() {    
    int maxScore = 0;
    int max;    
    int substValue = 0;    

    QByteArray alphaChars = substitutionMatrix.getAlphabet()->getAlphabetChars();
    for (int i = 0; i < patternSeq.length(); i++) {
        max = 0;        
        int nCharsInAlphabet = alphaChars.size();
        for (int j = 0; j < nCharsInAlphabet; j++) {
            //TODO: cache pattern seq raw pointer and alphaChars row pointer out of the loop
            char c1 = patternSeq.at(i);
            char c2 = alphaChars.at(j);
            substValue = (int) substitutionMatrix.getScore(c1, c2);
            max = qMax(max, substValue);
        }
        maxScore += max;
    }        

    if (minScore > maxScore) return 0;

    int gap = gapOpen;
    if (gapOpen < gapExtension) gap = gapExtension;


    if (gap < 0) matrixLength = patternSeq.length() + (maxScore - minScore)/gap * (-1) + 1;    
    if (searchSeq.length() + 1 < matrixLength) matrixLength = searchSeq.length() + 1;

    return 1;
}

void SmithWatermanAlgorithm::setValues(const SMatrix& _substitutionMatrix, 
                                       QByteArray const & _patternSeq, QByteArray const & _searchSeq, 
                                       int _gapOpen, int _gapExtension, int _minScore, SmithWatermanSettings::SWResultView _resultView) {

    substitutionMatrix = _substitutionMatrix;
    patternSeq = _patternSeq;
    searchSeq = _searchSeq;
    gapOpen = _gapOpen;
    gapExtension = _gapExtension;
    minScore = _minScore;    
    resultView = _resultView;
}

void SmithWatermanAlgorithm::launch(const SMatrix& _substitutionMatrix, 
                                    QByteArray const & _patternSeq, QByteArray const & _searchSeq, 
                                    int _gapOpen, int _gapExtension, int _minScore, SmithWatermanSettings::SWResultView _resultView) 
{    
    //set values
    setValues(_substitutionMatrix, _patternSeq, _searchSeq, _gapOpen, _gapExtension, _minScore, _resultView);

//launch algorithm

    if (isValidParams() && calculateMatrixLength()) {            
        calculateMatrix();        
    }    
    else {        
        //No result
        // See validationParams();
        //Perhaps reason: minScore > maxScore 
        //  or gap_open >= 0 or gap_extension >= 0
    }
//    clock_t eTime = clock();

}


bool SmithWatermanAlgorithm::isValidParams() {
    if (searchSeq.length() <= 0) return false;
    if (patternSeq.length() <= 0) return false;
    if (searchSeq.length() < patternSeq.length()) return false;
    if (gapOpen >= 0 || gapExtension >= 0) return false;    
//    if (substitutionMatrix.alphabet.isEmpty() || substitutionMatrix.alphabet.isNull()) return false;

    return true;
}

//Get results
//countResults - count of results will be return
QList<PairAlignSequences> SmithWatermanAlgorithm::getResults() {
    return pairAlignmentStrings;
}

void SmithWatermanAlgorithm::sortByScore( QList<PairAlignSequences> & res) {
    QList<PairAlignSequences> buf;
    QVector<int> pos;    
    QVector<KeyOfPairAlignSeq> sortedScores;

    for (int i = 0; i < res.size(); i++){ 
        for (int j = i + 1; j < res.size(); j++) {

            if (res.at(i).score < res.at(j).score) {
                KeyOfPairAlignSeq::exchange(res[i],res[j]);
            }
            else if (res.at(i).score == res.at(j).score 
                && res.at(i).refSubseqInterval.startPos > res.at(j).refSubseqInterval.startPos) {
                    KeyOfPairAlignSeq::exchange(res[i], res[j]);
            }
            else if (res.at(i).score == res.at(j).score 
                && res.at(i).refSubseqInterval.startPos == res.at(j).refSubseqInterval.startPos
                && res.at(i).refSubseqInterval.length > res.at(j).refSubseqInterval.length) {
                    KeyOfPairAlignSeq::exchange(res[i], res[j]);
            }
        }
    }
}


//Calculating dynamic matrix
//and save results
void SmithWatermanAlgorithm::calculateMatrix() {
    int subst, max, pos;
    int i, j, n, e1, f1, f2, f3;
    unsigned int src_n = searchSeq.length(), pat_n = patternSeq.length();
    unsigned char *src = (unsigned char*)searchSeq.data(), *pat = (unsigned char*)patternSeq.data();

    n = pat_n * 3;
    int *buf, *matrix = (int*)malloc(n * sizeof(int) + pat_n * 0x80);
    char *score_i, *score = (char*)(matrix + n);
    memset(matrix, 0, n * sizeof(int));

    foreach(char ch, substitutionMatrix.getAlphabet()->getAlphabetChars()){
        score_i = score + ch * pat_n;
        j = 0; do score_i[j] = substitutionMatrix.getScore(ch, pat[j]); while(++j < pat_n);
    }

    PairAlignSequences p;

    i = 1; do {
        buf = matrix;
        score_i = score + src[i - 1] * pat_n;
        e1 = 0; p.score = 0;
        f3 = i - 1; subst = 0;
        j = pat_n; do {
            f1 = buf[2] + gapExtension;
            f2 = buf[0] + gapOpen;
            f1 = f1 > f2 ? f1 : f2;
            subst += *score_i++;
            buf[2] = f1;

            max = subst; n = f3; \
                if(max >= p.score) { p.score = max; pos = n; } \
                    if(max <= 0) { max = 0; n = i; }
                    if(max < f1) { max = f1; n = buf[1]; }
                    if(max < e1) { max = e1; n = buf[1 - 3]; }
                    subst = buf[0]; f3 = buf[1];
                    buf[0] = max; buf[1] = n;

                    e1 += gapExtension;
                    max += gapOpen;
                    e1 = e1 > max ? e1 : max;
                    buf += 3;
        } while(--j);

        if(p.score >= minScore) {
            p.refSubseqInterval.startPos = pos;
            p.refSubseqInterval.length = i - pos;
            pairAlignmentStrings.append(p);
        }
    } while(++i <= src_n);

    free(matrix);
}


//Trace back in matrix
//Get similar sequences
void SmithWatermanAlgorithm::backtrace(int row, int col, int score) {            
    QByteArray pairAlign;
    int rBegin = row, cBegin = col;

    //algorithm back trace
    while (directionMatrix[getRow(row)][col] != STOP) {
        if (DIAG == directionMatrix[getRow(row)][col]) {
            // substitution                
            pairAlign.append(PairAlignSequences::DIAG);
            row--; col--;
        } else if (LEFT == directionMatrix[getRow(row)][col]) {
            // insertion                                
            pairAlign.append(PairAlignSequences::LEFT);
            col--;          
        } else if (UP == directionMatrix[getRow(row)][col]) {
            // must be a deletion                                                            
            pairAlign.append(PairAlignSequences::UP);
            row--;
        }                                                                            

    }                
    //Save sequences
    U2Region sReg(row, rBegin - row);
    U2Region pReg(col, cBegin - col);
        
    PairAlignSequences p;
    p.setValues(score, sReg, pReg, pairAlign);
    pairAlignmentStrings.append(p);
}

//GetRow with shift
int SmithWatermanAlgorithm::getRow(int row) const {
    return row % (matrixLength - 1) + 1;    
}


//Print matrix
void SmithWatermanAlgorithm::printMatrix() const {
    /*cout <<"-------------" <<endl;
    for (int i = 0; i < matrixLength; i++) {
        for (int j = 0; j < matrixLength; j++) cout <<matrix[i][j] <<" ";
        cout <<endl;
    }*/
}
//Get maximum value
int SmithWatermanAlgorithm::maximum (int var1, int var2, int var3, int var4) {
    int maxValue = var1;

    if (var2 > maxValue) maxValue = var2;
    if (var3 > maxValue) maxValue = var3;
    if (var4 > maxValue) maxValue = var4;

    return maxValue;
}

}//namespace
