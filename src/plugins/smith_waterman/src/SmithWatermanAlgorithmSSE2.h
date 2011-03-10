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
