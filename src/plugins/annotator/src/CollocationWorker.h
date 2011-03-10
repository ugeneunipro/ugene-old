#ifndef _U2_ANNOTATOR_WORKER_H_
#define _U2_ANNOTATOR_WORKER_H_

#include <QtCore/QSet>

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "CollocationsSearchAlgorithm.h"

namespace U2 {

namespace LocalWorkflow {

class CollocationPrompter;
typedef PrompterBase<CollocationPrompter> CollocationPrompterBase;

class CollocationPrompter : public CollocationPrompterBase {
    Q_OBJECT
public:
    CollocationPrompter(Actor* p = 0) : CollocationPrompterBase(p) {}
    virtual ~CollocationPrompter() {}
protected:
    QString composeRichDoc();
};

class CollocationWorker : public BaseWorker {
    Q_OBJECT
public:
    CollocationWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {}
    virtual ~CollocationWorker() {}
    virtual void init() ;
    virtual bool isReady();
    virtual Task* tick() ;
    virtual bool isDone() ;
    virtual void cleanup() {}
private slots:
    void sl_taskFinished();

protected:
    CommunicationChannel *input, *output;
    QString resultName;
    QSet<QString> names;
    CollocationsAlgorithmSettings cfg;
}; 

class CollocationWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    CollocationWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual ~CollocationWorkerFactory() {}
    virtual Worker* createWorker(Actor* a) {return new CollocationWorker(a);}
};

}//Workflow namespace
}//U2 namespace

#endif
