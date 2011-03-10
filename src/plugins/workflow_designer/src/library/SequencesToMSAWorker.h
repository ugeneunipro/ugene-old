#ifndef _U2_SEQUENCES_TO_MSA_WORKER_H_
#define _U2_SEQUENCES_TO_MSA_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include <U2Core/MAlignment.h>
#include <U2Core/DNASequence.h>

namespace U2 {
namespace LocalWorkflow {

class SequencesToMSAPromter : public PrompterBase<SequencesToMSAPromter> {
    Q_OBJECT
public:
    SequencesToMSAPromter( Actor * p = 0 ) : PrompterBase<SequencesToMSAPromter>(p) {};
protected:
    QString composeRichDoc();
};

class SequencesToMSAWorker : public BaseWorker {
    Q_OBJECT
public:
    SequencesToMSAWorker( Actor * p ) : BaseWorker(p), inPort(NULL), outPort(NULL) {};

    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup();
private:
    IntegralBus* inPort;
    IntegralBus* outPort;
private slots:
    void sl_onTaskFinished(Task* t);
private:
    QList<DNASequence> data;
};

class SequencesToMSAWorkerFactory : public DomainFactory {
public:
    const static QString ACTOR_ID;
    SequencesToMSAWorkerFactory() : DomainFactory(ACTOR_ID) {};
    static void init();
    virtual Worker* createWorker( Actor * a ) { return new SequencesToMSAWorker(a); }
};

class MSAFromSequencesTask : public Task {
    Q_OBJECT
public:
    MSAFromSequencesTask(const QList<DNASequence>& sequences)
        : Task(tr("MSAFromSequencesTask"), TaskFlag_None), sequences_(sequences), ma(NULL) {}
    void run();
    MAlignment getResult() const { return ma; }
private:
    QList<DNASequence> sequences_;
    MAlignment ma;
};

} //LocalWorkflow namespace
} //U2 namespace

#endif
