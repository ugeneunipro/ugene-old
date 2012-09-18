#ifndef _SPB_RANDOM_DEREPLICATION_TASK_H_
#define _SPB_RANDOM_DEREPLICATION_TASK_H_

#include "DereplicationTask.h"

namespace SPB {

class RandomDereplicationTask : public DereplicationTask {
public:
    RandomDereplicationTask(const DereplicationData &data);

    virtual void run();

private:
    int iterationIdx;
    int initialSeqCount;

private:
    bool updateProgress(int seqsRemain);
};

} // SPB

#endif // _SPB_RANDOM_DEREPLICATION_TASK_H_
