#ifndef _U2_CDSEARCH_WORKER_H_
#define _U2_CDSEARCH_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include <U2Algorithm/CDSearchTaskFactory.h>


namespace U2 {
namespace LocalWorkflow {

class CDSearchPrompter;
typedef PrompterBase<CDSearchPrompter> CDSearchPrompterBase;

class CDSearchPrompter : public CDSearchPrompterBase {
    Q_OBJECT
public:
    CDSearchPrompter(Actor* p = 0) : CDSearchPrompterBase(p) {}
protected:
    QString composeRichDoc();
};

class CDSearchWorker: public BaseWorker {
    Q_OBJECT
public:
    CDSearchWorker(Actor *a) : BaseWorker(a), input(NULL), output(NULL), cds(NULL) {}
    virtual void init();
    virtual bool isReady();
    virtual bool isDone();
    virtual Task* tick();
    virtual void cleanup() {};
private slots:
    void sl_taskFinished(Task*);
protected:
    CommunicationChannel *input, *output;
    CDSearchSettings settings;
    CDSearchResultListener* cds;
};

class CDSearchWorkerFactory : public DomainFactory {
    static const QString ACTOR_ID;
public:
    static void init();
    CDSearchWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new CDSearchWorker(a);}

};

} // Workflow
} // U2

#endif
