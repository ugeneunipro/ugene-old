#ifndef _U2_ORF_WORKER_H_
#define _U2_ORF_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Algorithm/ORFFinder.h>

namespace U2 {

namespace LocalWorkflow {

class ORFPrompter;
typedef PrompterBase<ORFPrompter> ORFPrompterBase;

class ORFPrompter : public ORFPrompterBase {
    Q_OBJECT
public:
    ORFPrompter(Actor* p = 0) : ORFPrompterBase(p) {}
protected:
    QString composeRichDoc();
};

class ORFWorker : public BaseWorker {
    Q_OBJECT
public:
    ORFWorker(Actor* a);
    
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
    ORFAlgorithmSettings cfg;
    
}; 

class ORFWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    ORFWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new ORFWorker(a);}
};

}// Workflow namespace
}// U2 namespace

#endif
