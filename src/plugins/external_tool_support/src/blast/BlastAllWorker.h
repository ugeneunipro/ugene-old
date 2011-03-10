#ifndef _U2_BLASTALL_WORKER_H_
#define _U2_BLASTALL_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "BlastAllSupportTask.h"
#include "utils/BlastTaskSettings.h"

namespace U2 {

namespace LocalWorkflow {

class BlastAllPrompter : public PrompterBase<BlastAllPrompter> {
    Q_OBJECT
public:
    BlastAllPrompter(Actor* p = 0);
protected:
    QString composeRichDoc();
};

class BlastAllWorker : public BaseWorker {
    Q_OBJECT
public:
    BlastAllWorker(Actor* a);
    
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
    BlastTaskSettings cfg;
    
}; 

class BlastAllWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    BlastAllWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new BlastAllWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif
