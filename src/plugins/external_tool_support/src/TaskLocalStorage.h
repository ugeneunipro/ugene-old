#ifndef _ETS_TASK_LOCAL_STORAGE_
#define _ETS_TASK_LOCAL_STORAGE_

#include <QtCore/QThreadStorage>

class ETSContext;

namespace U2 {

class ETSContextTLSRef {
public:
    ETSContextTLSRef(ETSContext* _ctx, int _workerID) : ctx(_ctx), workerID(_workerID){}
    ETSContext* ctx;
    int workerID;
};

class TaskLocalData {
public:
    static ETSContext* current();

    static unsigned currentWorkerID();


    static void bindToETSTLSContext(ETSContext *ctx, int workerID = 0);

    static void detachETSTLSContext();


private:
    static QThreadStorage<ETSContextTLSRef*> tls;
};
} //namespace

#endif
