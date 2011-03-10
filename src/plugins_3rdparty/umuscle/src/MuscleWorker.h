#ifndef _U2_MUSCLE_WORKER_H_
#define _U2_MUSCLE_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "MuscleTask.h"

namespace U2 {

namespace LocalWorkflow {

class MusclePrompter : public PrompterBase<MusclePrompter> {
    Q_OBJECT
public:
    MusclePrompter(Actor* p = 0) : PrompterBase<MusclePrompter>(p) {}
protected:
    QString composeRichDoc();
};

class MuscleWorker : public BaseWorker {
    Q_OBJECT
public:
    MuscleWorker(Actor* a);
    
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
    MuscleTaskSettings cfg;
    
}; 

class MuscleWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    MuscleWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new MuscleWorker(a);}
};

}// Workflow namespace
}// U2 namespace

#endif
