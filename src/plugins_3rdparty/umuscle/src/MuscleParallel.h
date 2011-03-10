#ifndef _U2_UMUSCLE_PARALLEL_H_
#define _U2_UMUSCLE_PARALLEL_H_

#include "MuscleTask.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/Task.h>
#include <U2Core/GAutoDeleteList.h>
#include <U2Core/MAlignment.h>

#include <algorithm>
#include <QtCore/QVector>

class MuscleContext;
class MSA;

namespace U2 {
class MusclePrepareTask;
class ProgressiveAlignTask;
class RefineTreeTask;
class RefineTask;


struct MuscleWorkPool;

class MuscleParallelTask: public Task {
    Q_OBJECT
public:
    MuscleParallelTask(const MAlignment& ma, MAlignment& res, const MuscleTaskSettings& config, MuscleContext* ctx);
    ~MuscleParallelTask() {cleanup();}
    void cleanup();
    QList<Task*> onSubTaskFinished(Task* subTask);

private:
    MuscleWorkPool *workpool;

    MusclePrepareTask     *prepareTask;  
    ProgressiveAlignTask  *progAlignTask;
    RefineTreeTask        *refineTreeTask;
    RefineTask            *refineTask;
};

class MusclePrepareTask: public Task {
    Q_OBJECT
public:
    MusclePrepareTask(MuscleWorkPool *wp);
    ~MusclePrepareTask() {cleanup();}
    void run();
    void _run();
    void alignPrepareUnsafe();
    void refinePrepareUnsafe();
    void cleanup();

    QList<Task*> res;
private:
    MuscleWorkPool *workpool;
};

class ProgressiveAlignTask: public Task {
    Q_OBJECT
public:
    ProgressiveAlignTask(MuscleWorkPool *wp);
    ~ProgressiveAlignTask() {cleanup();}
    void run();
    void _run();
    void prepare();
    void cleanup();
private:
    MuscleWorkPool *workpool;
    QTime timer;
};  

class ProgressiveAlignWorker: public Task {
    Q_OBJECT
public:
    ProgressiveAlignWorker(MuscleWorkPool *wp, int workerID);
    void run();
    void _run();
private:
    unsigned uTreeNodeIndex;
    MuscleWorkPool *workpool;
    int workerID;
}; 

class RefineTreeTask: public Task {
    Q_OBJECT
public:
    RefineTreeTask(MuscleWorkPool *workpool);
    void run();
    void _run();
private:
    MuscleWorkPool *workpool;
};

class RefineTask: public Task {
    Q_OBJECT
public:
    RefineTask(MuscleWorkPool *workpool);
    void run();
    void _run();
    bool RefineVertP(MSA *msa, unsigned uIters);
    bool RefineHorizP(MSA *msa, unsigned uIters, bool bLockLeft,
        bool bLockRight);
    void RefineHeightPartsP(bool* ptrbAnyChanges);
private:
    
    MuscleWorkPool *workpool;
};

class RefineWorker: public Task {
    Q_OBJECT
public:
    RefineWorker(MuscleWorkPool *workpool, int workerID);
    void run();
    void _run();
private:
    MuscleWorkPool *workpool;
    int workerID;
    unsigned *Leaves1;
    unsigned *Leaves2;
};

} //namespace

#endif //_U2_UMUSCLE_PARALLEL_H_
