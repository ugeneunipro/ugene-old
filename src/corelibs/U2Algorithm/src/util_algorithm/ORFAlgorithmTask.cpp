#include "ORFAlgorithmTask.h"

#include <U2Core/TextUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/Counter.h>

namespace U2 {

ORFFindTask::ORFFindTask(const ORFAlgorithmSettings& s, const QByteArray& seq) 
: Task (tr("ORF find"), TaskFlag_None), config(s), sequence(seq)
{
    GCOUNTER( cvar, tvar, "ORFFindTask" );
    tpm = Progress_Manual;
    assert(config.proteinTT && config.proteinTT->isThree2One());
}

void ORFFindTask::run() {
    ORFFindAlgorithm::find(dynamic_cast<ORFFindResultsListener*>(this),
        config,
        sequence.constData(),
        sequence.size(),
        stateInfo.cancelFlag,
        stateInfo.progress);
}

void ORFFindTask::onResult(const ORFFindResult& r) {
    lock.lock();
    assert(r.region.length % 3 == 0);
    newResults.append(r);
    lock.unlock();
}

QList<ORFFindResult> ORFFindTask::popResults() {
    lock.lock();
    QList<ORFFindResult> res = newResults;
    newResults.clear();
    lock.unlock();
    return res;
}

} //namespace



