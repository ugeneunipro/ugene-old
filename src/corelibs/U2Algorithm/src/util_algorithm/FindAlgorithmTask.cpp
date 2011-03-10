#include "FindAlgorithmTask.h"

#include <U2Core/TextUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/Counter.h>

namespace U2 {

FindAlgorithmTask::FindAlgorithmTask(const FindAlgorithmTaskSettings& s) 
: Task (tr("Find in sequence task"), TaskFlag_None), config(s), currentPos(0)
{
    GCOUNTER(cvar, tvar, "FindAlgorithmTask");
    tpm = Progress_Manual;
    complementRun = false;
    currentPos = s.searchRegion.startPos;
    assert(config.strand == FindAlgorithmStrand_Direct || config.complementTT!=NULL);
}

void FindAlgorithmTask::run() {
    FindAlgorithm::find(dynamic_cast<FindAlgorithmResultsListener*>(this),
        config.proteinTT,
        config.complementTT,
        config.strand,
        config.insDelAlg,
        config.sequence.constData(),
        config.sequence.size(),
        config.searchRegion,
        config.pattern.constData(),
        config.pattern.length(),
        config.singleShot,
        config.maxErr,
        stateInfo.cancelFlag,
        stateInfo.progress,
        currentPos);
}

void FindAlgorithmTask::onResult(const FindAlgorithmResult& r) {
    lock.lock();
    newResults.append(r);
    lock.unlock();
}

QList<FindAlgorithmResult> FindAlgorithmTask::popResults() {
    lock.lock();
    QList<FindAlgorithmResult> res = newResults;
    newResults.clear();
    lock.unlock();
    return res;
}

} //namespace



