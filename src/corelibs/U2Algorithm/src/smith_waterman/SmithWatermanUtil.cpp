#include "SmithWatermanUtil.h"

#include <U2Core/SMatrix.h>

namespace U2 {

float SmithWatermanUtil::calcScoreItSelf(const SMatrix& m, const char* pattern,  int patternLen) {
    float score = 0.0F;
    for (int i = 0; i < patternLen; i++) {
        score += m.getScore(pattern[i], pattern[i]);
    }
    return score;
}

int SmithWatermanUtil::calcWindowLen(bool transl,
                                     int sequenceLen,
                                     int patternLen,
                                     float scoreItSelf,
                                     float thresholdScore,
                                     float scoreGapOpen,
                                     float scoreGapExtd) {
    int tail = (thresholdScore - scoreItSelf + scoreGapOpen) / scoreGapExtd;
    tail = qMax(tail, 0);
    int windowLen = qMin(patternLen + tail, sequenceLen);
    if (transl) {
        windowLen *= 3;
    }
    return windowLen;
}

float SmithWatermanUtil::calcScoreThreshold(float percent, 
                                            float scoreItSelf) {
    if (percent < 0 || 100 < percent) {
        percent = 100;
    }
    float factor = (percent / 100.0F);
    return factor * scoreItSelf;
}

int SmithWatermanUtil::calcOverlapSize(bool transl, int windowLen, float scoreItSelf, float scoreGapOpen, float scoreGapExtd) {
    int dependSize = (scoreGapOpen - scoreItSelf) / scoreGapExtd;
    if (transl) {
        dependSize *= 3;
    }
    return qMax(windowLen, dependSize);
}

//greater score goes first
static bool revScoreComparator(const SmithWatermanResult &s0, const SmithWatermanResult &s1) {
    bool res = false;
    if (s0.score < s1.score) {
        res = false;
    } else if (s0.score == s1.score) {
        const U2Region& r0 = s0.region;
        const U2Region& r1 = s1.region;
        res = r0 < r1;
    } else {
        res = true;
    }
    return res;
}

bool SmithWatermanUtil::removeIdenticalResults(QList<SmithWatermanResult>* lst) {
    QList<SmithWatermanResult>& results = *lst;

    qSort(results.begin(), results.end(), revScoreComparator);
    int i = 0;
    int size = results.size();
    while (i < size) {
        int j = i + 1;
        while (j < size) {
            const SmithWatermanResult& currItem = results[i];
            const SmithWatermanResult& someItem = results[j];
            if (currItem.region == someItem.region && 
                currItem.strand == someItem.strand) {
                results.removeAt(j);
                size--;
                continue;
            }
            j++;
        }
        i++;
    }
    return true;
}

} // namespace
