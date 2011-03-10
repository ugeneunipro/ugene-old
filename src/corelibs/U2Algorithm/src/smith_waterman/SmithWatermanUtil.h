#ifndef _U2_SMITH_WATERMAN_UTIL_H_
#define _U2_SMITH_WATERMAN_UTIL_H_

#include "SmithWatermanResult.h"


namespace U2 {

class SMatrix;

class U2ALGORITHM_EXPORT SmithWatermanUtil {
public:
    static float calcScoreItSelf(const SMatrix& m,
                                 const char* pattern, 
                                 int patternLen);

    static int calcWindowLen(bool transl,
                             int sequenceLen,
                             int patternLen,
                             float scoreItSelf,
                             float thresholdScore,
                             float scoreGapOpen,
                             float scoreGapExtd);

    static float calcScoreThreshold(float percent, 
                                    float scoreItSelf);

    static int calcOverlapSize(bool transl,
                               int windowLen,
                               float scoreItSelf,
                               float scoreGapOpen, 
                               float scoreGapExtd);

    static bool removeIdenticalResults(QList<SmithWatermanResult>* lst);
};

} //namespace

#endif
