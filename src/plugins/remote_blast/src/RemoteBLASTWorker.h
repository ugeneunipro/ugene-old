#ifndef _REMOTE_QUERY_WORKER_
#define _REMOTE_QUERY_WORKER_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include "RemoteBLASTTask.h"

namespace U2 {
namespace LocalWorkflow {

class RemoteBLASTPrompter;
typedef PrompterBase<RemoteBLASTPrompter> RemoteBLASTPrompterBase;

class RemoteBLASTPrompter : public RemoteBLASTPrompterBase {
    Q_OBJECT
public:
    RemoteBLASTPrompter(Actor* p = 0) : RemoteBLASTPrompterBase(p) {}
protected:
    QString composeRichDoc();
};


class RemoteBLASTWorker: public BaseWorker {
    Q_OBJECT
public:
    RemoteBLASTWorker(Actor *a) : BaseWorker(a), input(NULL), output(NULL) {}
    virtual void init();
    virtual bool isReady();
    virtual bool isDone();
    virtual Task* tick();
    virtual void cleanup() {};
private slots:
    void sl_taskFinished();
protected:
    CommunicationChannel *input, *output;
    //QString resultName,transId;
    RemoteBLASTTaskSettings cfg;
};

class RemoteBLASTWorkerFactory:public DomainFactory {
    static const QString ACTOR_ID;
public:
    static void init();
    RemoteBLASTWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new RemoteBLASTWorker(a);}

};

}
}




#endif
