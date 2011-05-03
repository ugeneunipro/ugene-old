#ifndef _REVERSE_COMPLEMENT_WORKER_H_
#define _REVERSE_COMPLEMENT_WORKER_H_


#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

namespace U2 {
namespace LocalWorkflow {


class RCWorkerPrompter: public PrompterBase<RCWorkerPrompter> {
    Q_OBJECT
public:
    RCWorkerPrompter(Actor* p = 0) : PrompterBase<RCWorkerPrompter>(p) {}
protected:
    QString composeRichDoc();
};


class RCWorker: public BaseWorker {
    Q_OBJECT
public:
    RCWorker(Actor* a): BaseWorker(a), input(NULL), output(NULL) {}

    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup() {}

    /*private slots:
        void sl_taskFinished(Task*);*/

protected:
    CommunicationChannel *input, *output;
};

class RCWorkerFactory: public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    RCWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new RCWorker(a);}
};



}
}

#endif