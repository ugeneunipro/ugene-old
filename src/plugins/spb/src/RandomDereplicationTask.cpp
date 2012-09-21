#include <U2Core/Task.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "ComparingAlgorithm.h"

#include "RandomDereplicationTask.h"

namespace SPB {
static const int initProgressStep = 10;
RandomDereplicationTask::RandomDereplicationTask(const DereplicationData &data)
: DereplicationTask("Random dereplication", data), initialSeqCount(0), initTask(NULL),
comparer(NULL), sequences(NULL)
{
    iterationIdx = 0;
    nextInitProgress = initProgressStep;
}

RandomDereplicationTask::~RandomDereplicationTask() {
    delete comparer;
    delete sequences;
}

void RandomDereplicationTask::prepare() {
    initTask = new InitialSubTask(data);
    addSubTask(initTask);
}

QList<Task*> RandomDereplicationTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> tasks;
    if (initTask == subTask) {
        comparer = initTask->takeComparer();
        sequences = initTask->takeSequences();
    }
    return tasks;
}

void RandomDereplicationTask::run() {
    initialSeqCount = data.getSeqs().size();

    QMap<int, SharedDbiDataHandler> idMap;
    for (int i=0; i<initialSeqCount; i++) {
        idMap[i] = data.getSeqs().at(i);
    }

    while (!sequences->isEmpty()) {
        int leaderIdx = sequences->getRandomSeqNum();
        result << idMap.take(leaderIdx);
        comparer->removeSimilars(leaderIdx);
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
        double p = double(initialSeqCount - sequences->getSeqCount()) / initialSeqCount;
        int progress = int(100 * p);
        taskLog.details(tr("Dereplication progress: %1 %").arg(progress));
        stateInfo.setProgress(progress);

        if (nextInitProgress < progress) {
            nextInitProgress += initProgressStep;
            comparer->initialize();
        }
        return stateInfo.isCanceled();
    }
    return false;
}

/************************************************************************/
/* InitialSubTask */
/************************************************************************/
InitialSubTask::InitialSubTask(DereplicationData &_data)
: Task("Init random dereplication task", TaskFlag_None), data(_data), comparer(NULL), sequences(NULL)
{
    setSubtaskProgressWeight(0.0);
}

InitialSubTask::~InitialSubTask() {
    delete comparer;
    delete sequences;
}

void InitialSubTask::run() {
    sequences = new SequencesStorage(data.getSeqs());
    sequences->initialize(data.getStorage(), stateInfo);
    CHECK_OP(stateInfo, );

    comparer = new FullIndexComparer(data.getAccuracy(), *sequences, data.getComparingAlgoId());
    comparer->initialize();
}

FullIndexComparer * InitialSubTask::takeComparer() {
    FullIndexComparer *ret = comparer;
    comparer = NULL;
    return ret;
}

SequencesStorage * InitialSubTask::takeSequences() {
    SequencesStorage *ret = sequences;
    sequences = NULL;
    return ret;
}

} // SPB
