#ifndef _U2_CLUSTALW_WORKER_H_
#define _U2_CLUSTALW_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "ClustalWSupportTask.h"

namespace U2 {

namespace LocalWorkflow {

class ClustalWPrompter : public PrompterBase<ClustalWPrompter> {
    Q_OBJECT
public:
    ClustalWPrompter(Actor* p = 0);
protected:
    QString composeRichDoc();
};

class ClustalWWorker : public BaseWorker {
    Q_OBJECT
public:
    ClustalWWorker(Actor* a);
    
    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup();
    
private slots:
    void sl_taskFinished();

protected:
    CommunicationChannel *input, *output;
    QString resultName,transId;
    ClustalWSupportTaskSettings cfg;
    
}; 

class ClustalWWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    ClustalWWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new ClustalWWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif
