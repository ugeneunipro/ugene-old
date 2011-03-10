#ifndef _U2_KALIGN_WORKER_H_
#define _U2_KALIGN_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "KalignTask.h"

namespace U2 {

namespace LocalWorkflow {

class KalignPrompter : public PrompterBase<KalignPrompter> {
    Q_OBJECT
public:
    KalignPrompter(Actor* p = 0) : PrompterBase<KalignPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class KalignWorker : public BaseWorker {
    Q_OBJECT
public:
    KalignWorker(Actor* a);
    
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
    KalignTaskSettings cfg;
    
}; 

class KalignWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    KalignWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new KalignWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif
