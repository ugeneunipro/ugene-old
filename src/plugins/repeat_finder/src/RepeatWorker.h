#ifndef _U2_REPEAT_WORKER_H_
#define _U2_REPEAT_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "FindRepeatsTask.h"

namespace U2 {

namespace LocalWorkflow {

class RepeatPrompter;
typedef PrompterBase<RepeatPrompter> RepeatPrompterBase;

class RepeatPrompter : public RepeatPrompterBase {
    Q_OBJECT
public:
    RepeatPrompter(Actor* p = 0) : RepeatPrompterBase(p) {}
protected:
    QString composeRichDoc();
};

class RepeatWorker : public BaseWorker {
    Q_OBJECT
public:
    RepeatWorker(Actor* a);
    
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
    FindRepeatsTaskSettings cfg;
    
}; 

class RepeatWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    RepeatWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new RepeatWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif
