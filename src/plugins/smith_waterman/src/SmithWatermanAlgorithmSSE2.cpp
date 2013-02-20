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
    int i, j, n, e1, k, max, pos;
    __m128i f1, f2, f3, f4, f5;
    unsigned int src_n = searchSeq.length(), pat_n = patternSeq.length();
    unsigned char *src = (unsigned char*)searchSeq.data(), *pat = (unsigned char*)patternSeq.data();
    unsigned int iter = (pat_n + 7) >> 3;

    n = (iter + 1) * 5;
    __m128i *buf, *matrix = (__m128i*)_mm_malloc((n + iter * 0x80) * sizeof(__m128i), 16);
    short *score_i, *score = (short*)(matrix + n);
    memset(matrix, 0, n * sizeof(__m128i));

    foreach(char ch, substitutionMatrix.getAlphabet()->getAlphabetChars()){
        score_i = score + ch * iter * 8;
        j = 0; do *score_i++ = substitutionMatrix.getScore(ch, pat[j]); while(++j < pat_n);
        if(j = -j & 7) do *score_i++ = -0x8000; while(--j);
    }

    __m128i xMax, xPos;
    __m128i xOpen = _mm_insert_epi16(xOpen, gapOpen, 0);
    __m128i xExt = _mm_insert_epi16(xExt, gapExtension, 0);
    xOpen = _mm_shufflelo_epi16(xOpen, 0);
    xExt = _mm_shufflelo_epi16(xExt, 0);
    xOpen = _mm_unpacklo_epi32(xOpen, xOpen);
    xExt = _mm_unpacklo_epi32(xExt, xExt);

    PairAlignSequences p;

#define SWI(x) ((short*)(x))
#define SWLOOP(SWA, SWB) \
    buf = matrix + 5; \
    score_i = score + src[i - 1] * iter * 8; \
    xMax = _mm_xor_si128(xMax, xMax); \
    f4 = _mm_insert_epi16(f4, i, 0); \
    f4 = _mm_shufflelo_epi16(f4, 0); \
    f4 = _mm_unpacklo_epi32(f4, f4); \
    e1 = gapExtension; n = i; \
    SWI(SWB - 5 + 1)[7] = i - 1; \
    j = iter; do { \
    f2 = _mm_slli_si128(_mm_load_si128(SWB), 2); \
    f1 = _mm_slli_si128(_mm_load_si128(SWB + 1), 2); \
    f2 = _mm_insert_epi16(f2, SWI(SWB - 5)[7], 0); \
    f1 = _mm_insert_epi16(f1, SWI(SWB - 5 + 1)[7], 0); /* subst pos */ \
    f2 = _mm_adds_epi16(f2, *((__m128i*)score_i)); score_i += 8; /* subst */ \
    /* f2 f1 */ \
    f3 = _mm_xor_si128(f3, f3); \
    f2 = _mm_max_epi16(f2, f3); \
    f3 = _mm_cmpeq_epi16(f3, f2); \
    f3 = _mm_or_si128(_mm_and_si128(f3, f4), _mm_andnot_si128(f3, f1)); \
    /* f2 f3 */ \
    xMax = _mm_max_epi16(xMax, f2); \
    f1 = _mm_cmpeq_epi16(f2, xMax); \
    xPos = _mm_or_si128(_mm_and_si128(f1, f3), _mm_andnot_si128(f1, xPos)); \
    \
    f1 = _mm_adds_epi16(_mm_load_si128(buf + 4), xExt); \
    f1 = _mm_max_epi16(f1, _mm_adds_epi16(_mm_load_si128(SWB), xOpen)); \
    _mm_store_si128(buf + 4, f1); \
    \
    f1 = _mm_max_epi16(f1, f2); \
    f2 = _mm_cmpeq_epi16(f2, f1); \
    f2 = _mm_or_si128(_mm_and_si128(f2, f3), _mm_andnot_si128(f2, _mm_load_si128(SWB + 1))); \
    \
    _mm_store_si128(SWA, f1); \
    _mm_store_si128(SWA + 1, f2); \
    \
    k = 0; do { \
    if(e1 <= SWI(SWA)[k]) { max = SWI(SWA)[k] + gapOpen; n = SWI(SWA + 1)[k]; \
    e1 += gapExtension; e1 = e1 > max ? e1 : max; } \
          else { SWI(SWA)[k] = e1; SWI(SWA + 1)[k] = n; e1 += gapExtension; } \
    } while(++k < 8); \
    buf += 5; \
    } while(--j); \
    \
    max = SWI(&xMax)[0]; n = 0; \
    k = 1; do { \
    e1 = SWI(&xMax)[k]; \
    if(e1 >= max) { max = e1; n = k; } \
    } while(++k < 8); \
    \
    if(max >= minScore) { \
    n = ((SWI(&xPos)[n] - i - 1) | -0x10000) + i + 1; \
    p.score = max; \
    p.refSubseqInterval.startPos = n; \
    p.refSubseqInterval.length = i - n; \
    pairAlignmentStrings.append(p); \
    }

    i = 1; do {
        SWLOOP(buf, buf + 2);
        if(++i > src_n) break;
        SWLOOP(buf + 2, buf);
    } while(++i <= src_n);

#undef SWLOOP
#undef SWI

    _mm_free(matrix);
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
