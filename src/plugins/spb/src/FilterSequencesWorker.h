#ifndef _SPB_FILTER_SEQUENCES_WORKER_H_
#define _SPB_FILTER_SEQUENCES_WORKER_H_

#include <U2Core/Task.h>

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

using namespace U2;
using namespace U2::LocalWorkflow;

namespace SPB {

class FullIndexComparer;
class SequencesStorage;

class FilterSequencesWorker : public BaseWorker {
    Q_OBJECT
public:
    FilterSequencesWorker(Actor *a);

    virtual void init();
    virtual bool isReady();
    virtual Task * tick();
    virtual void cleanup();

private slots:
    void sl_taskFinished();

private:
    IntegralBus *inPort1;
    IntegralBus *inPort2;
    IntegralBus *outPort;

    double accuracy;
    QString algoId;
    QList<SharedDbiDataHandler> sequencesToFind;
    SequencesStorage *sequences;
    FullIndexComparer *comparer;

}; // FilterSequencesWorker

class FilterSequencesWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;

    FilterSequencesWorkerFactory() : DomainFactory(ACTOR_ID) {}
    static void init();
    virtual Worker * createWorker(Actor *a);

}; // FilterSequencesWorkerFactory

class FilterSequencesPrompter : public PrompterBase<FilterSequencesPrompter> {
public:
    FilterSequencesPrompter(Actor *a = 0)
        : PrompterBase<FilterSequencesPrompter>(a) {}

protected:
    QString composeRichDoc();
}; // FilterSequencesPrompter

class FilterSequenceTask : public Task {
public:
    FilterSequenceTask(FullIndexComparer *comparer,
        const SharedDbiDataHandler &srcSeq, WorkflowContext *ctx);

    virtual void run();
    virtual void cleanup();
    bool isFiltered() const;

private:
    FullIndexComparer *comparer;
    SharedDbiDataHandler srcSeq;
    WorkflowContext *ctx;
    bool result;
}; // FilterSequenceTask

} // SPB

#endif // _SPB_FILTER_SEQUENCES_WORKER_H_