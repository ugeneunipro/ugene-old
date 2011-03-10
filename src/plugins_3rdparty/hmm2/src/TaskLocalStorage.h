#ifndef _TASK_LOCAL_CONTEXT_
#define _TASK_LOCAL_CONTEXT_

#include <hmmer2/funcs.h>

#include <QtCore/QThreadStorage>
#include <QtCore/QHash>
#include <QtCore/QMutex>

namespace U2 {

// this struct is stored in TLS
struct ContextIdContainer {
    ContextIdContainer(qint64 id) : contextId(id){}
    qint64 contextId;
};


class TaskLocalData {
public:
    static HMMERTaskLocalData* current();

    // initializes HMMContext for current thread
    static HMMERTaskLocalData* createHMMContext(qint64 contextId, bool bindThreadToContext);

    static void freeHMMContext(qint64 contextId);



    // binds to existing HMMContext 
    static void bindToHMMContext(qint64 contextId);

    static qint64 detachFromHMMContext();


private:
    static QHash<qint64, struct HMMERTaskLocalData*> data;
    static QThreadStorage<ContextIdContainer*> tls;
    static QMutex mutex;
};
} //namespace

#endif
