#ifndef _UMUSCLE_TASK_LOCAL_STORAGE_
#define _UMUSCLE_TASK_LOCAL_STORAGE_

#include <QtCore/QThreadStorage>

class MuscleContext;

namespace U2 {

class MuscleContextTLSRef {
public:
    MuscleContextTLSRef(MuscleContext* _ctx, int _workerID) : ctx(_ctx), workerID(_workerID){}
    MuscleContext* ctx;
    int workerID;
};

class TaskLocalData {
public:
    static MuscleContext* current();

    static unsigned currentWorkerID();


    static void bindToMuscleTLSContext(MuscleContext *ctx, int workerID = 0);

    static void detachMuscleTLSContext();


private:
    static QThreadStorage<MuscleContextTLSRef*> tls;
};
} //namespace

#endif