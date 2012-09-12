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

    while (!data.getSeqs().isEmpty()) {
        SharedDbiDataHandler leaderId = data.takeRandomSequence();
        result << leaderId;

        QScopedPointer<U2SequenceObject> leader(StorageUtils::getSequenceObject(data.getStorage(), leaderId));
        CHECK_EXT(NULL != leader.data(), setError("NULL sequence object"), );

        QByteArray leaderData = leader->getSequenceData(U2_REGION_MAX, stateInfo);
        CHECK_OP(stateInfo, );

        // TODO: must be parallelized
        foreach (const SharedDbiDataHandler &seqId, data.getSeqs()) {
            QScopedPointer<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(data.getStorage(), seqId));
            CHECK_EXT(NULL != seqObj.data(), setError("NULL sequence object"), );

            QByteArray seqData = seqObj->getSequenceData(U2_REGION_MAX, stateInfo);
            CHECK_OP(stateInfo, );

            double res = algo->compare(leaderData, seqData);
            if (res >= data.getAccuracy()) {
                data.getSeqs().removeOne(seqId);
            }
        }
        if (updateProgress()) {
            return;
        }
    }
}

static const int UPDATE_STEP = 50;
inline bool RandomDereplicationTask::updateProgress() {
    iterationIdx++;
    if (iterationIdx > UPDATE_STEP) {
        iterationIdx = 0;
        double p = double(initialSeqCount - data.getSeqs().size()) / initialSeqCount;
        stateInfo.setProgress(int(100 * p));
        return stateInfo.isCanceled();
    }
    return false;
}

} // SPB
