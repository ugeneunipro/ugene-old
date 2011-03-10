#ifndef _U2_MAFFT_WORKER_H_
#define _U2_MAFFT_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "MAFFTSupportTask.h"

namespace U2 {

namespace LocalWorkflow {

class MAFFTPrompter : public PrompterBase<MAFFTPrompter> {
    Q_OBJECT
public:
    MAFFTPrompter(Actor* p = 0);
protected:
    QString composeRichDoc();
};

class MAFFTWorker : public BaseWorker {
    Q_OBJECT
public:
    MAFFTWorker(Actor* a);
    
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
    MAFFTSupportTaskSettings cfg;
    
}; 

class MAFFTWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    MAFFTWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new MAFFTWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif
