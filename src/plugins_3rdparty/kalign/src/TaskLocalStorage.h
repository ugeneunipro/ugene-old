#ifndef _KALIGN_TASK_LOCAL_STORAGE_
#define _KALIGN_TASK_LOCAL_STORAGE_

#include <QtCore/QThreadStorage>

class KalignContext;

namespace U2 {

class KalignContextTLSRef {
public:
    KalignContextTLSRef(KalignContext* _ctx, int _workerID) : ctx(_ctx), workerID(_workerID){}
    KalignContext* ctx;
    int workerID;
};

class TaskLocalData {
public:
    static KalignContext* current();

    static unsigned currentWorkerID();


    static void bindToKalignTLSContext(KalignContext *ctx, int workerID = 0);

    static void detachKalignTLSContext();


private:
    static QThreadStorage<KalignContextTLSRef*> tls;
};
} //namespace

#endif
