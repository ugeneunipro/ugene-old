#ifndef _U2_GENERATE_DNA_WORKER_H_
#define _U2_GENERATE_DNA_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>


namespace U2 {
namespace LocalWorkflow {

class GenerateDNAPrompter : public PrompterBase<GenerateDNAPrompter> {
    Q_OBJECT
public:
    GenerateDNAPrompter(Actor* p = 0) : PrompterBase<GenerateDNAPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class GenerateDNAWorker : public BaseWorker {
    Q_OBJECT
public:
    GenerateDNAWorker(Actor* a) : BaseWorker(a), ch(NULL), done(false) {}

    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup() {}

private slots:
    void sl_taskFinished(Task*);

private:
    CommunicationChannel* ch;
    bool done;
};

class GenerateDNAWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    GenerateDNAWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) { return new GenerateDNAWorker(a); }
};

} // LocalWorkflow namespace
} // U2 namespace

#endif
