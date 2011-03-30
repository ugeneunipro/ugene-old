#ifndef _STATISTIC_WORKERS_H_
#define _STATISTIC_WORKERS_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

namespace U2 {
namespace LocalWorkflow {

class DNAStatWorker:public BaseWorker {
    Q_OBJECT
public:
    DNAStatWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {}
    
    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup() {};

private:
    CommunicationChannel *input, *output;
};

class DNAStatWorkerFactory: public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    DNAStatWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) { return new DNAStatWorker(a); }
};

class DNAStatWorkerPrompter: public PrompterBase<DNAStatWorkerPrompter> {
    Q_OBJECT
public:
    DNAStatWorkerPrompter(Actor* p = 0) : PrompterBase<DNAStatWorkerPrompter>(p) {}
protected:
    QString composeRichDoc();
};


}//LocalWorkflow
}//U2

#endif