#ifndef _U2_BLASTPLUS_WORKER_H_
#define _U2_BLASTPLUS_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "BlastPlusSupportCommonTask.h"

namespace U2 {

namespace LocalWorkflow {

class BlastPlusPrompter : public PrompterBase<BlastPlusPrompter> {
    Q_OBJECT
public:
    BlastPlusPrompter(Actor* p = 0);
protected:
    QString composeRichDoc();
};

class BlastPlusWorker : public BaseWorker {
    Q_OBJECT
public:
    BlastPlusWorker(Actor* a);
    
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
    BlastTaskSettings   cfg;
    
}; 

class BlastPlusWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    BlastPlusWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new BlastPlusWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif
