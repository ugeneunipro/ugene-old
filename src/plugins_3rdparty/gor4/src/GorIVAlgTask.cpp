#include <QtCore/QFile>

#include <U2Core/Counter.h>
#include <U2View/SecStructPredictUtils.h>
#include "GorIVAlgTask.h"
#include "gor.h"


namespace U2 {

const QString GorIVAlgTask::taskName(tr("GORIV"));
QMutex GorIVAlgTask::runLock;

GorIVAlgTask::GorIVAlgTask(const QByteArray& inputSeq) : SecStructPredictTask(inputSeq)
{
    GCOUNTER( cvar, tvar, "GorIVAlgTask" );
}

void GorIVAlgTask::run() 
{
    output.resize(sequence.size());
    sequence.push_front(' ');


    QFile seqDb(":gor4//datafiles//New_KS.267.seq");
    if (!seqDb.open(QIODevice::ReadOnly)) {
        stateInfo.setError(SecStructPredictTask::tr("seq database not found"));
        return;
    }

    QFile strucDb(":gor4//datafiles//New_KS.267.obs");
    if (!strucDb.open(QIODevice::ReadOnly)) {
        stateInfo.setError(SecStructPredictTask::tr("observed structures database not found"));
        return;
    }

    //TODO: get rid of this limit
    const int MAXRES = 10000;
    if (sequence.size() > MAXRES) {
        stateInfo.setError(SecStructPredictTask::tr("sequence is too long, max seq size is 10000"));
        return;
    }

    //TODO: BUG-0000808
    QMutexLocker runLocker( &runLock );
    runGORIV(seqDb, strucDb, sequence.data(), sequence.size() - 1, output.data());

    results = SecStructPredictUtils::saveAlgorithmResultsAsAnnotations(output, GORIV_ANNOTATION_NAME);

}


} //namespace



