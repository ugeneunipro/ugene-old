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
    storedResults = 0; 
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
    
    if (minScore > maxScore) {                        
        //Error: perhaps minScore > maxScore
        algoLog.trace(QString("SW Error min score > max score. Min score: %1, max score: %2").arg(minScore).arg(maxScore));
        return false;
    }    
    
    int gap = gapOpen;
    if (gapOpen < gapExtension) gap = gapExtension;

    
    if (gap < 0) matrixLength = patternSeq.length() + (maxScore - minScore)/gap * (-1) + 1;    

    if (searchSeq.length() + 1 < matrixLength) matrixLength = searchSeq.length() + 1;

    return true;    

}

void SmithWatermanAlgorithm::setValues(const SMatrix& _substitutionMatrix, 
                                       QByteArray const & _patternSeq, QByteArray const & _searchSeq, 
                                       int _gapOpen, int _gapExtension, int _minScore) {

    substitutionMatrix = _substitutionMatrix;
    patternSeq = _patternSeq;
    searchSeq = _searchSeq;
    gapOpen = _gapOpen;
    gapExtension = _gapExtension;
    minScore = _minScore;    

}

void SmithWatermanAlgorithm::launch(const SMatrix& _substitutionMatrix, 
                                    QByteArray const & _patternSeq, QByteArray const & _searchSeq, 
                                    int _gapOpen, int _gapExtension, int _minScore) 
{    
    //set values
    setValues(_substitutionMatrix, _patternSeq, _searchSeq, _gapOpen, _gapExtension, _minScore);

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
    algoLog.trace("RUN sortByScore");
    QList<PairAlignSequences> buf;
    QVector<int> pos;    
    QVector<KeyOfPairAlignSeq> sortedScores;

//     for (int i = 0; i < res.size(); i++) {
//         pos.append(i);        
//         sortedScores.append(
//             KeyOfPairAlignSeq(res.at(i).score, 
//             res.at(i).intervalSeq1));
//     }

    for (int i = 0; i < res.size(); i++) 
        for (int j = i + 1; j < res.size(); j++) {


//             if (sortedScores.at(i).score < sortedScores.at(j).score)
//                 KeyOfPairAlignSeq::exchange(sortedScores[i], pos[i], sortedScores[j] , pos[j]);
// 
//             else if (sortedScores.at(i).score == sortedScores.at(j).score 
//                 && sortedScores.at(i).intervalSeq1.startPos > sortedScores.at(j).intervalSeq1.startPos)                                 KeyOfPairAlignSeq::exchange(sortedScores[i], pos[i], sortedScores[j] , pos[j]);
// 
//             else if (sortedScores.at(i).score == sortedScores.at(j).score 
//                 && sortedScores.at(i).intervalSeq1.startPos == sortedScores.at(j).intervalSeq1.startPos
//                 && sortedScores.at(i).intervalSeq1.len > sortedScores.at(j).intervalSeq1.len) 
//                 KeyOfPairAlignSeq::exchange(sortedScores[i], pos[i], sortedScores[j] , pos[j]);
// 
//             else if (sortedScores.at(i).score == sortedScores.at(j).score 
//                 && sortedScores.at(i).intervalSeq1.startPos == sortedScores.at(j).intervalSeq1.startPos
//                 && sortedScores.at(i).intervalSeq1.len == sortedScores.at(j).intervalSeq1.len
//                 && sortedScores.at(i).intervalSeq2.startPos > sortedScores.at(j).intervalSeq2.startPos) 
//                 KeyOfPairAlignSeq::exchange(sortedScores[i], pos[i], sortedScores[j] , pos[j]);                    

            if (res.at(i).score < res.at(j).score) {
                KeyOfPairAlignSeq::exchange(res[i],res[j]);
            }
            else if (res.at(i).score == res.at(j).score 
                && res.at(i).intervalSeq1.startPos > res.at(j).intervalSeq1.startPos) {
                    KeyOfPairAlignSeq::exchange(res[i], res[j]);
            }
            else if (res.at(i).score == res.at(j).score 
                && res.at(i).intervalSeq1.startPos == res.at(j).intervalSeq1.startPos
                && res.at(i).intervalSeq1.length > res.at(j).intervalSeq1.length) {
                    KeyOfPairAlignSeq::exchange(res[i], res[j]);
            }
        }

//         for (int i = 0; i < res.size(); i++) {        
//             buf.append(res.at(pos.at(i)));
//         }
//        res = buf;


        algoLog.trace("FINISH sortByScore");
}


//Calculating dynamic matrix
//and save results
void SmithWatermanAlgorithm::calculateMatrix() {
    QString str;        
    char ch;
    int substValue = 0, max = 0;
    int f1, f2, e1, e2;

    //initialization dynamic matrices
    QVector<int> rowInts;
    QVector<char> rowChars;
    for (int tt = 0; tt < patternSeq.length() + 2; tt++) {        
        rowInts.append(0);
        rowChars.append(STOP);
        EMatrix.append(0);
        FMatrix.append(0);
    }    

    for (int tt = 0; tt < matrixLength; tt++) {            
        matrix.append(rowInts);
        directionMatrix.append(rowChars);            
    }

    PairAlignSequences p;
    QByteArray pairAlign;
    U2Region sReg;
    U2Region pReg;

    QVector<QVector<int> > directionSearchSeq;    
    directionSearchSeq.push_back(rowInts);
    directionSearchSeq.push_back(rowInts);

    int even = 0;
    int odd = 0;
    for (int i = 1; i < searchSeq.length() + 1; i++) {
        ch = searchSeq.at(i - 1);
        even = i % 2;
        odd = (i + 1) % 2;

        directionSearchSeq[odd][0] = i - 1;
        p.score = 0;

        for (int j = 1; j < patternSeq.length()+1; j++) {                

            f1 = FMatrix[j] + gapExtension;                
            f2 = matrix[getRow(i-1)][j] + gapOpen;

            if (f1 > f2) FMatrix[j] = f1;
            else FMatrix[j] = f2;

            e1 = EMatrix[j - 1] + gapExtension;                
            e2 = matrix[getRow(i)][j-1] + gapOpen;

            if (e1 > e2) EMatrix[j] = e1;
            else EMatrix[j] = e2;

            substValue = matrix[getRow(i-1)][j-1] 
                + substitutionMatrix.getScore(ch, patternSeq.at(j-1));

            //Get max value and store them
            max = maximum(EMatrix[j], FMatrix[j], substValue);
            matrix[getRow(i)][j] = max;

            //Save direction from we come here for back trace
//             if (max == 0) directionMatrix[getRow(i)][j] = STOP; 
//             else if (max == EMatrix[j]) directionMatrix[getRow(i)][j] = LEFT;
//             else if (max == FMatrix[j]) directionMatrix[getRow(i)][j] = UP;
//             else if (max == substValue) directionMatrix[getRow(i)][j] = DIAG;
/*
            //If value meet the conditions then start backtrace()

            if ( max >= minScore ) backtrace(i, j, max);                
            */
            
            if (max == 0) {
                directionSearchSeq[even][j] = i;                
            }
            else if (max == EMatrix[j]) {
                directionSearchSeq[even][j] = directionSearchSeq[even][j - 1];                
            }
            else if (max == FMatrix[j]) {
                directionSearchSeq[even][j] = directionSearchSeq[odd][j];                                
            }
            else if (max == substValue) {
                directionSearchSeq[even][j] = directionSearchSeq[odd][j - 1];                
            }
            if (max >= p.score) {
                p.intervalSeq1.startPos = directionSearchSeq[even][j];
                p.intervalSeq1.length = i - directionSearchSeq[even][j];
                p.score = max;
            }
        }
        if (p.score >= minScore) {
            pairAlignmentStrings.append(p);
        }

//         for (int qq = 0; qq < directionSearchSeq[0].size(); qq++) {
//             cout <<directionSearchSeq[even][qq] <<" ";
//         }
//         cout <<endl;

        //Print matrix
//         QString str = "";
//         for(int k = 1; k < patternSeq.length() + 1; k++) str += QString::number(directionSearchSeq[even][k]) + " ";
//        for(int k = 1; k < patternSeq.length() + 1; k++) str += QString::number(FMatrix[k]) + " ";
//        for(int k = 1; k < patternSeq.length() + 1; k++) str += QString::number(matrix[getRow(i)][k]) + " ";
//       log.info(str);

    }        
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
                // skip to the next iteration
                continue;
            }

            if (LEFT == directionMatrix[getRow(row)][col]) {
                // insertion                                
                pairAlign.append(PairAlignSequences::LEFT);

                col--;
                // skip to the next iteration
                continue;            
            }

            if (UP == directionMatrix[getRow(row)][col]) {
                // must be a deletion                                                            
                pairAlign.append(PairAlignSequences::UP);

                row--;
                // skip to the next iteration
                continue;
            }                                                                            

        }                
        //Save sequences
        U2Region sReg(row, rBegin - row);
        U2Region pReg(col, cBegin - col);
        


//             PairAlignSequences p;
//             p.setValues(score, pairAlign, sReg, pReg);
//             pairAlignmentStrings.append(p);
//             storedResults++;

        PairAlignSequences p;
        p.setValues(score, sReg);
        pairAlignmentStrings.append(p);
        storedResults++;


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
