#ifndef _U2_GENOME_ALIGNER_WORKER_H_
#define _U2_GENOME_ALIGNER_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "GenomeAlignerTask.h"

namespace U2 {

namespace LocalWorkflow {

class GenomeAlignerPrompter : public PrompterBase<GenomeAlignerPrompter> {
    Q_OBJECT
public:
    GenomeAlignerPrompter(Actor* p = 0) : PrompterBase<GenomeAlignerPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class GenomeAlignerWorker : public BaseWorker {
    Q_OBJECT
public:
    GenomeAlignerWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {}
    virtual void init() ;
    virtual bool isReady();
    virtual Task* tick() ;
    virtual bool isDone() ;
    virtual void cleanup() {}
private slots:
    void sl_taskFinished();

protected:
    CommunicationChannel *input, *output;
    QString resultName,transId;
    DnaAssemblyToRefTaskSettings settings;
}; 

class GenomeAlignerWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    GenomeAlignerWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new GenomeAlignerWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif // GENOME_ALIGNER_WORKER
