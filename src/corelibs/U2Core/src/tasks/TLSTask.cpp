#include "TLSTask.h"

namespace U2 {

QThreadStorage<TLSContextRef*> TLSUtils::tls;

/************************************************************************/
/* TaskLocalData                                                        */
/************************************************************************/
TLSContext* TLSUtils::current( QString contextId ) {
    Q_UNUSED(contextId)
    TLSContextRef* ref = tls.localData();
    if (ref!=NULL) {
        assert(ref->ctx!=NULL);
        assert(ref->ctx->id == contextId);
        return ref->ctx;
    }
    assert(0);
    return NULL;
}

void TLSUtils::bindToTLSContext(TLSContext *ctx) {
    assert(ctx!=NULL);
    assert(!tls.hasLocalData());
    tls.setLocalData(new TLSContextRef(ctx));
}

void TLSUtils::detachTLSContext() {
    TLSContextRef* ref = tls.localData();
    assert(ref!=NULL && ref->ctx!=NULL);
    ref->ctx = NULL;
    tls.setLocalData(NULL);
}

/************************************************************************/
/* TLSTask                                                              */
/************************************************************************/

TLSTask::TLSTask( const QString& _name, TaskFlags _flags, bool _deleteContext)
:Task(_name, _flags), taskContext(NULL), deleteContext(_deleteContext)
{
}

void TLSTask::prepare()
{
    taskContext = createContextInstance();
}

void TLSTask::run()
{
    TLSUtils::bindToTLSContext(taskContext);
    try {
        _run();
    } catch(...) {
        stateInfo.setError("_run() throws exception");
    }
    TLSUtils::detachTLSContext();
}

TLSTask::~TLSTask()
{
    if(deleteContext)
        delete taskContext;
    taskContext = NULL;
}
} //namespace U2
