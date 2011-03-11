#ifndef _U2_GENOME_ALIGNER_WORKER_H_
#define _U2_GENOME_ALIGNER_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "GenomeAlignerTask.h"
#include "GenomeAlignerIO.h"

namespace U2 {
namespace LocalWorkflow {

/************************************************************************/
/* Genome aligner                                                       */
/************************************************************************/
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
    GenomeAlignerWorker(Actor* a);
    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone() {return done;}
    virtual void cleanup();
private slots:
    void sl_taskFinished();

protected:
    CommunicationChannel *reads, *index, *output;
    GenomeAlignerReader *reader;
    GenomeAlignerWriter *writer;
    QString resultName,transId;
    DnaAssemblyToRefTaskSettings settings;
    bool done;
}; 

class GenomeAlignerWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    GenomeAlignerWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new GenomeAlignerWorker(a);}
};

/************************************************************************/
/* Genome aligner index build                                           */
/************************************************************************/
class GenomeAlignerBuildPrompter : public PrompterBase<GenomeAlignerBuildPrompter> {
    Q_OBJECT
public:
    GenomeAlignerBuildPrompter(Actor* p = 0) : PrompterBase<GenomeAlignerBuildPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class GenomeAlignerBuildWorker : public BaseWorker {
    Q_OBJECT
public:
    GenomeAlignerBuildWorker(Actor* a) : BaseWorker(a), output(NULL), done(false) {}
    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup() {}
private slots:
    void sl_taskFinished();

protected:
    CommunicationChannel *output;
    QString resultName, transId;
    GUrl refSeqUrl;
    GUrl indexUrl;
    bool done;
    DnaAssemblyToRefTaskSettings settings;
};

class GenomeAlignerBuildWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    GenomeAlignerBuildWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new GenomeAlignerBuildWorker(a);}
};

/************************************************************************/
/* Genome aligner index read                                            */
/************************************************************************/
class GenomeAlignerIndexReaderPrompter : public PrompterBase<GenomeAlignerIndexReaderPrompter> {
    Q_OBJECT
public:
    GenomeAlignerIndexReaderPrompter(Actor* p = 0) : PrompterBase<GenomeAlignerIndexReaderPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class GenomeAlignerIndexReaderWorker : public BaseWorker {
    Q_OBJECT
public:
    GenomeAlignerIndexReaderWorker(Actor* a) : BaseWorker(a), output(NULL), done(false) {}
    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup() {}
private slots:
    void sl_taskFinished();

protected:
    CommunicationChannel *output;
    QString resultName,transId;
    GUrl indexUrl;
    bool done;
}; 

class GenomeAlignerIndexReaderWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    GenomeAlignerIndexReaderWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new GenomeAlignerIndexReaderWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif // GENOME_ALIGNER_WORKER
