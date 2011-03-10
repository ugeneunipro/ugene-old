#ifndef _IMPORT_PHRED_QUALITY_WORKER_H_
#define _IMPORT_PHRED_QUALITY_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "ImportQualityScoresTask.h"

namespace U2 {

class DNASequenceObject;

namespace LocalWorkflow {

class ImportPhredQualityPrompter : public PrompterBase<ImportPhredQualityPrompter> {
    Q_OBJECT
public:
    ImportPhredQualityPrompter(Actor* p = 0) : PrompterBase<ImportPhredQualityPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class ImportPhredQualityWorker : public BaseWorker {
    Q_OBJECT
public:
    ImportPhredQualityWorker(Actor* a);
    
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
    ImportQualityScoresConfig cfg;
    QList<DNASequenceObject*> seqObjs;
}; 

class ImportPhredQualityWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    ImportPhredQualityWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new ImportPhredQualityWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif // _IMPORT_PHRED_QUALITY_WORKER_H_
