#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "ComparingAlgorithm.h"

#include "RandomDereplicationTask.h"

namespace SPB {

RandomDereplicationTask::RandomDereplicationTask(const DereplicationData &data)
: DereplicationTask("Random dereplication", data), initialSeqCount(0)
{
    iterationIdx = 0;
}

void RandomDereplicationTask::run() {
    qsrand(QDateTime::currentDateTime().toTime_t());
    QScopedPointer<ComparingAlgorithm> algo(
        ComparingAlgorithmFactory::createAlgorithm(data.getComparingAlgoId()));
    if (NULL == algo.data()) {
        setError(tr("Unknown comparing algorithm: %1").arg(data.getComparingAlgoId()));
        return;
    }

    initialSeqCount = data.getSeqs().size();
    QMap<int, SharedDbiDataHandler> idMap;
    QMap<int, DNASequence> seqMap;
    for (int i=0; i<initialSeqCount; i++) {
        SharedDbiDataHandler id = data.getSeqs().at(i);
        idMap[i] = id;
        QScopedPointer<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(data.getStorage(), id));
        CHECK_EXT(NULL != seqObj.data(), setError("NULL sequence object"), );
        seqMap[i] = seqObj->getWholeSequence();
    }

    while (!idMap.isEmpty()) {
        int leaderIdx = idMap.begin().key();
        result << idMap.take(leaderIdx);
        DNASequence leaderSeq = seqMap.take(leaderIdx);

        // TODO: must be parallelized
        foreach (int i, seqMap.keys()) {
            const DNASequence &seq = seqMap[i];

            double res = algo->compare(leaderSeq.seq, seq.seq);
            if (res >= data.getAccuracy()) {
                idMap.remove(i);
                seqMap.remove(i);
            }
        }
        if (updateProgress(idMap.size())) {
            return;
        }
    }
}

static const int UPDATE_STEP = 50;
inline bool RandomDereplicationTask::updateProgress(int seqsRemain) {
    iterationIdx++;
    if (iterationIdx > UPDATE_STEP) {
        iterationIdx = 0;
        double p = double(initialSeqCount - seqsRemain) / initialSeqCount;
        stateInfo.setProgress(int(100 * p));
        return stateInfo.isCanceled();
    }
    return false;
}

} // SPB
