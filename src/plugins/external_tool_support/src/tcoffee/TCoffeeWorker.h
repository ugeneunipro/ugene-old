#ifndef _U2_TCOFFEE_WORKER_H_
#define _U2_TCOFFEE_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "TCoffeeSupportTask.h"

namespace U2 {

namespace LocalWorkflow {

class TCoffeePrompter : public PrompterBase<TCoffeePrompter> {
    Q_OBJECT
public:
    TCoffeePrompter(Actor* p = 0);
protected:
    QString composeRichDoc();
};

class TCoffeeWorker : public BaseWorker {
    Q_OBJECT
public:
    TCoffeeWorker(Actor* a);
    
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
    TCoffeeSupportTaskSettings cfg;
    
}; 

class TCoffeeWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    TCoffeeWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new TCoffeeWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif
