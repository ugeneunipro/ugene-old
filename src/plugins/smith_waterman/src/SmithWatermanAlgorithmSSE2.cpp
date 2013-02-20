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

#ifdef SW2_BUILD_WITH_SSE2

#include "SmithWatermanAlgorithmSSE2.h"

#include <emmintrin.h>
#include <iostream>

using namespace std;

namespace U2 {
    
    
//     static bool PRINT_DIRECTIONS = false;
//     static bool PRINT_SCORE = false;
//     static bool PRINT_JUST_SCORE = false;
//     static bool PRINT_E = false;
//     static bool PRINT_F = true;
//     static bool PRINT_F_SHIFT = false;
//     static bool PRINT_H = false;

    quint64 SmithWatermanAlgorithmSSE2::estimateNeededRamAmount(const SMatrix& sm, QByteArray const & _patternSeq,
                                                                QByteArray const & _searchSeq, const qint32 gapOpen,
                                                                const qint32 gapExtension, const quint32 minScore,
                                                                const quint32 maxScore, const SmithWatermanSettings::SWResultView resultView) {
    const double b_to_mb_factor = 1048576.0;

    const quint64 queryLength = _patternSeq.length();
    const quint64 searchLength = _searchSeq.length();

    const quint32 iter = (queryLength + nElementsInVec - 1) / nElementsInVec;
    const int alphaSize = sm.getAlphabet()->getNumAlphabetChars();
    
    const quint64 queryProfSize = 'Z' * alphaSize * iter * sizeof(__m128i);

    const quint64 patternLengthDivisibleByN = queryLength + 1 + nElementsInVec - (queryLength + 1) % nElementsInVec;
    
    const quint64 tempSize = nElementsInVec * sizeof(ScoreType);
    const quint64 eArraySize = (nElementsInVec + patternLengthDivisibleByN) * sizeof(ScoreType);
    const quint64 fArraySize = (nElementsInVec + patternLengthDivisibleByN) * sizeof(ScoreType);
    const quint64 maxArraySize = (nElementsInVec + patternLengthDivisibleByN) * sizeof(ScoreType);

    qint32 maxGapPenalty = (gapOpen > gapExtension) ? gapOpen : gapExtension;
    assert(0 > maxGapPenalty);

    quint64 matrixLength = queryLength - (maxScore - minScore) / maxGapPenalty + 1;    
    if (searchLength + 1 < matrixLength) {
        matrixLength = searchLength + 1;
    }

    const quint64 matrixLengthDivisibleByN = matrixLength + nElementsInVec - matrixLength % nElementsInVec;
    
    const quint64 matrixPtrsSize = sizeof(ScoreType *) * matrixLengthDivisibleByN;
    const quint64 matrixSize = sizeof(ScoreType) * matrixLengthDivisibleByN * patternLengthDivisibleByN;

    quint64 directionArraySize = 0;
    if(SmithWatermanSettings::MULTIPLE_ALIGNMENT == resultView) {
        directionArraySize = matrixLength * (queryLength + 2) * sizeof(char);
    } else if (SmithWatermanSettings::ANNOTATIONS == resultView) {
        directionArraySize = 2 * sizeof(int) * (patternLengthDivisibleByN + 1);
    } else {
        assert(0);
    }

    const quint64 MatrixCalculationRequiredSpace = matrixPtrsSize + matrixSize + tempSize + eArraySize + fArraySize +
                                maxArraySize + queryProfSize + directionArraySize +
                                (sizeof(int) + sizeof(char)) * (patternLengthDivisibleByN + 1);

    return MatrixCalculationRequiredSpace / b_to_mb_factor;
}

void SmithWatermanAlgorithmSSE2::launch(const SMatrix& _substitutionMatrix, QByteArray const & _patternSeq,
    QByteArray const & _searchSeq, int _gapOpen, int _gapExtension, int _minScore, SmithWatermanSettings::SWResultView _resultView) {
    setValues(_substitutionMatrix, _patternSeq, _searchSeq, _gapOpen, _gapExtension, _minScore, _resultView);
    int maxScor = 0;
    if (isValidParams() && calculateMatrixLength()) {
        maxScor = calculateMatrixSSE2(patternSeq.length(),(unsigned char *)searchSeq.data(), searchSeq.length(), (-1)*(gapOpen + gapExtension), (-1)*(gapExtension));

        if (minScore <= maxScor) {
            calculateMatrix();
        }            
    } else {
        //No result
        // See validationParams();
        //Perhaps reason: minScore > maxScore 
        //  or gap_open >= 0 or gap_extension >= 0
    }
}

//Calculating dynamic matrix
//and save results
void SmithWatermanAlgorithmSSE2::calculateMatrix() {
    char ch;
    //ScoreType substValue = 0; //warning: unused variable substValue
    ScoreType max = 0;

    int matrixLengthDivisibleByN = matrixLength;
    int patternLengthDivisibleByN = patternSeq.length() + 1;

    matrixLengthDivisibleByN += nElementsInVec - matrixLengthDivisibleByN % nElementsInVec;
    patternLengthDivisibleByN += nElementsInVec - patternLengthDivisibleByN % nElementsInVec;

    ScoreType * temp = (ScoreType*)_mm_malloc(nElementsInVec*sizeof(ScoreType),16);
    for (int pp = 0; pp < nElementsInVec; pp++) temp[pp] = 0;

    __m128i zero = _mm_set1_epi32(0);
    __m128i tmp = _mm_set1_epi32(0);

    //Load 0 all elements of vector zero
    zero = _mm_insert_epi16 (zero, 0, 0);
    zero = _mm_shufflelo_epi16 (zero, 0);
    zero = _mm_shuffle_epi32 (zero, 0);

    __m128i substValVec = zero;        


    //    int f1, f2, e1, e2;
    __m128i f1 = zero, f2 = zero, f3 = zero;//, e1 = zero, e2 = zero, e3 = zero; //warning: unused variables e1, e2, e3
    __m128i gapOpenVec = _mm_set1_epi32(0);
    __m128i gapExtensionVec  = _mm_set1_epi32(0);
    __m128i maxVec  = _mm_set1_epi32(0);

    // Load gap opening penalty to all elements of a vector
    gapOpenVec = _mm_insert_epi16 (gapOpenVec, gapOpen, 0);
    gapOpenVec = _mm_shufflelo_epi16 (gapOpenVec, 0);
    gapOpenVec = _mm_shuffle_epi32 (gapOpenVec, 0);

    // Load gap extension penalty to all elements of a vector
    gapExtensionVec = _mm_insert_epi16 (gapExtensionVec, gapExtension, 0);
    gapExtensionVec = _mm_shufflelo_epi16 (gapExtensionVec, 0);
    gapExtensionVec = _mm_shuffle_epi32 (gapExtensionVec, 0);        

    ScoreType * EArray = (ScoreType*)_mm_malloc((nElementsInVec + patternLengthDivisibleByN)*sizeof(ScoreType),16);
    ScoreType * FArray = (ScoreType*)_mm_malloc((nElementsInVec + patternLengthDivisibleByN)*sizeof(ScoreType),16);
    ScoreType * maxArray = (ScoreType*)_mm_malloc((nElementsInVec + patternLengthDivisibleByN)*sizeof(ScoreType),16);
    
    for (int tt = 0; tt < patternLengthDivisibleByN; tt++) {            
        FArray[tt] = 0;
        EArray[tt] = 0;
    }

    ScoreType ** matrix = new ScoreType*[matrixLengthDivisibleByN];    
    matrix[0] = (ScoreType*)_mm_malloc(matrixLengthDivisibleByN*patternLengthDivisibleByN*sizeof(ScoreType), 16);
    for (int i = 1; i < matrixLengthDivisibleByN; i++) {
        matrix[i] = matrix[i-1] + patternLengthDivisibleByN;
    }
    
    __m128i * pvScore;

    int segSize = (patternSeq.length() + nElementsInVec - 1) / nElementsInVec; //count iter

    int ALPHA_SIZE = substitutionMatrix.getAlphabet()->getNumAlphabetChars();

    __m128i* pvQueryProf = (__m128i*)_mm_malloc
        ('Z'*ALPHA_SIZE*segSize*sizeof(__m128i), 16);    

    ScoreType * queryProfile = (ScoreType *) pvQueryProf;
    
    
    QByteArray alphaChars = substitutionMatrix.getAlphabet()->getAlphabetChars();
    for (int i = 0; i <ALPHA_SIZE ; i++) {        
        
        char ch = alphaChars.at(i);
        
        int posI = ch * segSize * nElementsInVec;

        for (int j = 0; j < segSize; j++) {
            for (int k = j; k < segSize*nElementsInVec; k+=segSize) {                        
                if (k >= patternSeq.length()) 
                    queryProfile[posI + k] = 0;
                else {
                    queryProfile[posI + k] = 
                        (ScoreType)substitutionMatrix.getScore(patternSeq.at(k), ch);                    
                }
            }                    
        }
    }

//////////////////////////////////////////////////////////////////////////

    for (int tt = 0; tt < matrixLengthDivisibleByN; tt++) 
        for (int ww = 0; ww < patternLengthDivisibleByN; ww++) matrix[tt][ww] = 0;


    //initialization dynamic matrices        
    QVector<char> rowChars;
    QVector<int> rowInts;
    for (int tt = 0; tt < patternLengthDivisibleByN + 1; tt++) {    
        rowInts.append(0);
    }

    if(SmithWatermanSettings::MULTIPLE_ALIGNMENT == resultView) {
        for (int tt = 0; tt < patternSeq.length() + 2; tt++) {        
            rowChars.append(STOP);
        }
        for (int tt = 0; tt < matrixLength; tt++) {
            directionMatrix.append(rowChars);
        }
    }

    ScoreType ee1 = 0, ee2 = 0;


    PairAlignSequences p;

    QVector<QVector<int> > directionSearchSeq;
    if(SmithWatermanSettings::ANNOTATIONS == resultView) {
        directionSearchSeq.push_back(rowInts);
        directionSearchSeq.push_back(rowInts);
    }

    int even = 0;
    int odd = 0;

    //Start dynamic programming
    for (int i = 1; i < searchSeq.length() + 1; i++) {

        ch = searchSeq.at(i - 1);

        pvScore = pvQueryProf + ch * segSize;
        
        if(SmithWatermanSettings::ANNOTATIONS == resultView) {
            even = i % 2;
            odd = (i + 1) % 2;

            directionSearchSeq[odd][0] = i - 1;

            p.score = 0;
        }
        for (int j = 0; j < patternSeq.length(); j += nElementsInVec) {                            
//TODO: use __m128i*, not array
            f1 = _mm_load_si128((__m128i*) ((ScoreType *) &FArray[j]));                        
            f1 = _mm_add_epi16(f1, gapExtensionVec);        

            tmp = _mm_load_si128((__m128i*) ((ScoreType *) &matrix[getRow(i-1)][j]));
            f2 = _mm_add_epi16(tmp, gapOpenVec);                    

            f3 = _mm_max_epi16(f1, f2);                        

            _mm_store_si128((__m128i*) &FArray[j], f3);            

            substValVec = _mm_load_si128( pvScore++ );


            tmp = _mm_slli_si128(tmp, 2);

            tmp = _mm_insert_epi16 (tmp, matrix[getRow(i-1)][j-1], 0);

            tmp = _mm_add_epi16(tmp, substValVec);

            _mm_store_si128((__m128i*) &temp[0], tmp);

            maxVec = _mm_max_epi16(tmp, f3);
            
            _mm_store_si128((__m128i*) &maxArray[0], maxVec);


//Use non vector operations for calculate maxScore and backtrace
            for (int pp = j; pp < j + nElementsInVec && pp < patternSeq.length(); pp++) {
                int eIndex = pp + 1;
                ee1 = EArray[eIndex - 1] + gapExtension;                
                ee2 = matrix[getRow(i)][pp - 1] + gapOpen;

                if (ee1 > ee2) EArray[eIndex] = ee1;
                else EArray[eIndex] = ee2;
                
                if (maxArray[pp - j] > EArray[eIndex]) max = maxArray[pp - j];
                else max = EArray[eIndex];

                if (max < 0) max = 0;

                matrix[getRow(i)][pp] = max;
                int col = pp + 1;

                if(SmithWatermanSettings::MULTIPLE_ALIGNMENT == resultView) {
                    //Save direction from we come here for back trace
                    if (max == 0) {
                        directionMatrix[getRow(i)][col] = STOP;
                    } else if (max == EArray[eIndex]) {
                        directionMatrix[getRow(i)][col] = LEFT;
                    } else if (max == FArray[pp]) {
                        directionMatrix[getRow(i)][col] = UP;
                    } else if (max == temp[pp - j]) {
                        directionMatrix[getRow(i)][col] = DIAG;
                    }

                    //If value meet the conditions then start backtrace()
                    if (max >= minScore) {
                        backtrace(i, col, max);
                    }
                } else if(SmithWatermanSettings::ANNOTATIONS == resultView) {
                    if (max == 0) {
                        directionSearchSeq[even][col] = i;                
                    }
                    else if (max == EArray[eIndex]) {
                        directionSearchSeq[even][col] = directionSearchSeq[even][col - 1];
                    }
                    else if (max == FArray[pp]) {
                        directionSearchSeq[even][col] = directionSearchSeq[odd][col];
                    }
                    else if (max == temp[pp - j]) {                    
                        directionSearchSeq[even][col] = directionSearchSeq[odd][col - 1];                
                    }
                    if (max >= p.score) {
                        p.score = max;
                        p.refSubseqInterval.startPos = directionSearchSeq[even][col];
                        p.refSubseqInterval.length = i - p.refSubseqInterval.startPos;
                    }
                }
            }
        }    
        if (SmithWatermanSettings::ANNOTATIONS == resultView && p.score >= minScore) {
            pairAlignmentStrings.append(p);
        }
//        Print matrix
//                  for(int k = 0; k < patternSeq.length() + 1 ; k++) cout <<matrix[getRow(i)][k] <<" ";
//                  cout <<endl;            
//                 for(int k = 1; k < patternSeq.length() + 1; k++) cout <<directionMatrix[getRow(i)][k] <<" ";
//                 cout <<endl;        

    }
//Free memory
    _mm_free(EArray);
    _mm_free(FArray);
    _mm_free(temp);    
    _mm_free(maxArray);
    _mm_free(pvQueryProf);
    
    //TODO: it is correct?    
    _mm_free(matrix[0]);
    delete matrix;
}

int SmithWatermanAlgorithmSSE2::calculateMatrixSSE2(unsigned queryLength, unsigned char *dbSeq, unsigned dbLength, unsigned short gapOpenOrig, unsigned short gapExtend) {

    unsigned iter = (queryLength + nElementsInVec - 1) / nElementsInVec;

    int ALPHA_SIZE = substitutionMatrix.getAlphabet()->getNumAlphabetChars();

    __m128i* pvQueryProf = (__m128i*)_mm_malloc
        ('Z'*ALPHA_SIZE*iter*sizeof(__m128i), 16);    

    int weight = 0;
    unsigned short * queryProfile = (unsigned short *) pvQueryProf;
    int segSize = (queryLength + 7) / 8; //iter
    int nCount = segSize * 8;
    char curChar = ' ';

    QByteArray alphaChars = substitutionMatrix.getAlphabet()->getAlphabetChars();
    for (int i = 0; i < ALPHA_SIZE; i++) {
        curChar = alphaChars.at(i);        
        int h = 0;
        for (int j = 0; j < segSize; j++) {
            unsigned int k = j;
            for (unsigned int kk = 0; kk < 8; kk++) {
                if (k >= queryLength) {
                    weight = 0;
                } else {
                    weight = substitutionMatrix.getScore(curChar, patternSeq.at(k));
                }
                queryProfile[curChar * nCount + h] = (unsigned short) weight;
                k += segSize;
                h++;
            }
        }
    }


    __m128i *pvHLoad = (__m128i*)_mm_malloc(iter*sizeof(__m128i), 16);    
    __m128i *pvHStore = (__m128i*)_mm_malloc(iter*sizeof(__m128i), 16);    
    __m128i *pvE = (__m128i*)_mm_malloc(iter*sizeof(__m128i), 16);


    unsigned i = 0, j = 0;
    int     score = 0;

    int cmp = 0;    

    unsigned short gapOpenFarrar = gapOpenOrig - gapExtend;

    __m128i *pv = 0;

    __m128i vE = _mm_set1_epi32(0), vF = _mm_set1_epi32(0), vH = _mm_set1_epi32(0);

    __m128i vMaxScore = _mm_set1_epi32(0);
    __m128i vGapOpen = _mm_set1_epi32(0);
    __m128i vGapExtend = _mm_set1_epi32(0);

    __m128i vMin = _mm_set1_epi32(0);
    __m128i vMinimums = _mm_set1_epi32(0);
    __m128i vTemp = _mm_set1_epi32(0);

    __m128i *pvScore = 0;


    /* Load gap opening penalty to all elements of a constant */
    vGapOpen = _mm_insert_epi16 (vGapOpen, gapOpenFarrar, 0);
    vGapOpen = _mm_shufflelo_epi16 (vGapOpen, 0);
    vGapOpen = _mm_shuffle_epi32 (vGapOpen, 0);

    /* Load gap extension penalty to all elements of a constant */
    vGapExtend = _mm_insert_epi16 (vGapExtend, gapExtend, 0);
    vGapExtend = _mm_shufflelo_epi16 (vGapExtend, 0);
    vGapExtend = _mm_shuffle_epi32 (vGapExtend, 0);

    /*  load vMaxScore with the zeros.  since we are using signed */
    /*  math, we will bias the maxscore to -32768 so we have the */
    /*  full range of the short. */
    vMaxScore = _mm_cmpeq_epi16 (vMaxScore, vMaxScore);
    vMaxScore = _mm_slli_epi16 (vMaxScore, 15);    

    vMinimums = _mm_shuffle_epi32 (vMaxScore, 0);
    Q_UNUSED(vMinimums);

    vMin = _mm_shuffle_epi32 (vMaxScore, 0);
    vMin = _mm_srli_si128 (vMin, 14);

    /* Zero out the storage vector */
    for (i = 0; i < iter; ++i) {
        _mm_store_si128 (pvE + i, vMaxScore);
        _mm_store_si128 (pvHStore + i, vMaxScore);
    }

    PairAlignSequences p;
    U2Region sReg;    

    for (i = 0; i < dbLength; ++i) {    


        /* fetch first data asap. */
        pvScore = pvQueryProf + dbSeq[i] * iter;

        /* zero out F. */
        vF = _mm_cmpeq_epi16 (vF, vF);
        vF = _mm_slli_epi16 (vF, 15);

        /* load the next h value */
        vH = _mm_load_si128 (pvHStore + iter - 1);
        vH = _mm_slli_si128 (vH, 2);
        vH = _mm_or_si128 (vH, vMin);

        pv = pvHLoad;
        pvHLoad = pvHStore;
        pvHStore = pv;

        for (j = 0; j < iter; ++j) {

            /* load values of vF and vH from previous row (one unit up) */            
            vE = _mm_load_si128 (pvE + j);

            /* add score to vH */
            vH = _mm_adds_epi16 (vH, *pvScore++);
            
            /* Update highest score encountered this far */
            vMaxScore = _mm_max_epi16 (vMaxScore, vH);

            /* get max from vH, vE and vF */
            vH = _mm_max_epi16 (vH, vE);
            vH = _mm_max_epi16 (vH, vF);

            /* save vH values */
            _mm_store_si128 (pvHStore + j, vH);

            /* update vE value */
            vH = _mm_subs_epi16 (vH, vGapOpen);
            vE = _mm_subs_epi16 (vE, vGapExtend);
            vE = _mm_max_epi16 (vE, vH);            

            /* update vF value */
            vF = _mm_subs_epi16 (vF, vGapExtend);
            vF = _mm_max_epi16 (vF, vH);

            /* save vE values */
            _mm_store_si128 (pvE + j, vE);

            /* load the next h value */
            vH = _mm_load_si128 (pvHLoad + j);                
            
        }        

        /* reset pointers to the start of the saved data */
        j = 0;
        vH = _mm_load_si128 (pvHStore + j);        

        /*  the computed vF value is for the given column.  since */
        /*  we are at the end, we need to shift the vF value over */
        /*  to the next column. */
        
        vF = _mm_slli_si128 (vF, 2);
        vF = _mm_or_si128 (vF, vMin);
        vTemp = _mm_subs_epi16 (vH, vGapOpen);

        vTemp = _mm_cmpgt_epi16 (vF, vTemp);
        
        cmp  = _mm_movemask_epi8 (vTemp);
        
        while (cmp != 0x0000) 
            //for (unsigned cnt=0; cnt<iter; ++cnt)
        {    
            vE = _mm_load_si128 (pvE + j);

            vH = _mm_max_epi16 (vH, vF);

            /* save vH values */
            _mm_store_si128 (pvHStore + j, vH);
            
            /*  update vE incase the new vH value would change it */
            vH = _mm_subs_epi16 (vH, vGapOpen);
            vE = _mm_max_epi16 (vE, vH);            
            

            /* update vF value */
            vF = _mm_subs_epi16 (vF, vGapExtend);        
            
            
            j++;
            if (j >= iter) {
                j = 0;
                vF = _mm_slli_si128 (vF, 2);
                vF = _mm_or_si128 (vF, vMin);
            }            

            vH = _mm_load_si128 (pvHStore + j);                        

            vTemp = _mm_subs_epi16 (vH, vGapOpen);
            vTemp = _mm_cmpgt_epi16 (vF, vTemp);
            cmp  = _mm_movemask_epi8 (vTemp);
        }
    }

    /* find largest score in the vMaxScore vector */
    vTemp = _mm_srli_si128 (vMaxScore, 8);
    vMaxScore = _mm_max_epi16 (vMaxScore, vTemp);
    vTemp = _mm_srli_si128 (vMaxScore, 4);
    vMaxScore = _mm_max_epi16 (vMaxScore, vTemp);
    vTemp = _mm_srli_si128 (vMaxScore, 2);
    vMaxScore = _mm_max_epi16 (vMaxScore, vTemp);

    /* store in temporary variable */
    score = (short) _mm_extract_epi16 (vMaxScore, 0);

    _mm_free(pvHLoad);
    _mm_free(pvHStore);
    _mm_free(pvE);
    _mm_free(pvQueryProf);

    /* return largest score */
    return score + 32768;
}


    
inline void SmithWatermanAlgorithmSSE2::printVector(__m128i &toprint, int add) {

    ScoreType * tmpArray = (ScoreType*)_mm_malloc(nElementsInVec*sizeof(ScoreType),16);

    _mm_store_si128((__m128i*) &tmpArray[0], toprint);
    cout <<"printVector" <<endl;
    for (int i = 0; i < nElementsInVec; i++) cout <<tmpArray[i] + add <<" ";

    cout <<endl;
}



} //namespase
#endif //SW2_BUILD_WITH_SSE2
