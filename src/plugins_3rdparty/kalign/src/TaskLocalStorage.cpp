#include "TaskLocalStorage.h"
#include <assert.h>

class KalignContext *getKalignContext() {
    return U2::TaskLocalData::current();
}
int getKalignWorkerID() {
    return U2::TaskLocalData::currentWorkerID();
}

namespace U2 {

QThreadStorage<KalignContextTLSRef*> TaskLocalData::tls;

class KalignContext* TaskLocalData::current(){
    KalignContextTLSRef* ref = tls.localData();
    if (ref!=NULL) {
        assert(ref->ctx!=NULL);
        return ref->ctx;
    }
    assert(0);
    return NULL;
}

unsigned TaskLocalData::currentWorkerID() {
    KalignContextTLSRef* ref = tls.localData();
    if (ref!=NULL) {
        return ref->workerID;
    }
    assert(0);
    return -1;

}

void TaskLocalData::bindToKalignTLSContext(KalignContext *ctx, int workerID) {
    assert(ctx!=NULL);
    assert(!tls.hasLocalData());
    tls.setLocalData(new KalignContextTLSRef(ctx, workerID));
}

void TaskLocalData::detachKalignTLSContext() {
    KalignContextTLSRef* ref = tls.localData();
    assert(ref!=NULL && ref->ctx!=NULL);
    ref->ctx = NULL;
    tls.setLocalData(NULL);
}


}//namespace

