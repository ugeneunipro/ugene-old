#ifndef _SPB_RANDOM_DEREPLICATION_TASK_H_
#define _SPB_RANDOM_DEREPLICATION_TASK_H_

#include "DereplicationTask.h"
#include "FullIndexComparer.h"

namespace SPB {

class InitialSubTask;

class RandomDereplicationTask : public DereplicationTask {
public:
    RandomDereplicationTask(const DereplicationData &data);
    virtual ~RandomDereplicationTask();

    virtual void prepare();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    virtual void run();

private:
    int iterationIdx;
    int nextInitProgress;
    int initialSeqCount;
    InitialSubTask *initTask;
    FullIndexComparer *comparer;
    SequencesStorage *sequences;

private:
    bool updateProgress();
};

class InitialSubTask : public Task {
public:
    InitialSubTask(DereplicationData &data);
    virtual ~InitialSubTask();

    virtual void run();
    FullIndexComparer * takeComparer();
    SequencesStorage * takeSequences();

private:
    DereplicationData &data;
    FullIndexComparer *comparer;
    SequencesStorage *sequences;
};

} // SPB

#endif // _SPB_RANDOM_DEREPLICATION_TASK_H_
