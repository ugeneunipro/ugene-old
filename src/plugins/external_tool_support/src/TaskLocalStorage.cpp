#include "TaskLocalStorage.h"
#include <assert.h>

class ETSContext *getETSContext() {
    return U2::TaskLocalData::current();
}
int getETSWorkerID() {
    return U2::TaskLocalData::currentWorkerID();
}

namespace U2 {

QThreadStorage<ETSContextTLSRef*> TaskLocalData::tls;

class ETSContext* TaskLocalData::current(){
    ETSContextTLSRef* ref = tls.localData();
    if (ref!=NULL) {
        assert(ref->ctx!=NULL);
        return ref->ctx;
    }
    assert(0);
    return NULL;
}

unsigned TaskLocalData::currentWorkerID() {
    ETSContextTLSRef* ref = tls.localData();
    if (ref!=NULL) {
        return ref->workerID;
    }
    assert(0);
    return -1;

}

void TaskLocalData::bindToETSTLSContext(ETSContext *ctx, int workerID) {
    assert(ctx!=NULL);
    assert(!tls.hasLocalData());
    tls.setLocalData(new ETSContextTLSRef(ctx, workerID));
}

void TaskLocalData::detachETSTLSContext() {
    ETSContextTLSRef* ref = tls.localData();
    assert(ref!=NULL && ref->ctx!=NULL);
    ref->ctx = NULL;
    tls.setLocalData(NULL);
}


}//namespace

