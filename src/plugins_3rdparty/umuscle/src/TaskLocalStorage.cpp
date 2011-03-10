#include "TaskLocalStorage.h"
#include "muscle/muscle.h"
#include "muscle/objscore.h"
#include "muscle/profile.h"
#include "muscle/enumopts.h"
#include "muscle/params.h"
#include "muscle/muscle_context.h"

class MuscleContext *getMuscleContext() {
    return U2::TaskLocalData::current();
}
int getMuscleWorkerID() {
    return U2::TaskLocalData::currentWorkerID();
}

namespace U2 {

QThreadStorage<MuscleContextTLSRef*> TaskLocalData::tls;

class MuscleContext* TaskLocalData::current(){
    MuscleContextTLSRef* ref = tls.localData();
    if (ref!=NULL) {
        assert(ref->ctx!=NULL);
        return ref->ctx;
    }
    assert(0);
    return NULL;
}

unsigned TaskLocalData::currentWorkerID() {
    MuscleContextTLSRef* ref = tls.localData();
    if (ref!=NULL) {
        return ref->workerID;
    }
    assert(0);
    return -1;

}

void TaskLocalData::bindToMuscleTLSContext(MuscleContext *ctx, int workerID) {
    assert(ctx!=NULL);
    assert(!tls.hasLocalData());
    tls.setLocalData(new MuscleContextTLSRef(ctx, workerID));
}

void TaskLocalData::detachMuscleTLSContext() {
    MuscleContextTLSRef* ref = tls.localData();
    assert(ref!=NULL && ref->ctx!=NULL);
    ref->ctx = NULL;
    tls.setLocalData(NULL);
}


}//namespace

