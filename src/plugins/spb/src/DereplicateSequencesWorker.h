#ifndef _SPB_DEREPLICATE_SEQUENCES_WORKER_H_
#define _SPB_DEREPLICATE_SEQUENCES_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

using namespace U2;
using namespace U2::LocalWorkflow;

namespace SPB {

class DereplicateSequencesWorker : public BaseWorker {
    Q_OBJECT
public:
    DereplicateSequencesWorker(Actor *a);

    virtual void init();
    virtual bool isReady();
    virtual Task * tick();
    virtual void cleanup();

private slots:
    void sl_taskFinished();

private:
    IntegralBus *inPort;
    IntegralBus *outPort;

    QString derepAlgoId;
    QString compAlgoId;
    int accuracy;

    bool taskInRunning;

    QList<SharedDbiDataHandler> sequences;
}; // DereplicateSequencesWorker

class DereplicateSequencesWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;

    DereplicateSequencesWorkerFactory() : DomainFactory(ACTOR_ID) {}
    static void init();
    virtual Worker * createWorker(Actor *a);

}; // DereplicateSequencesWorkerFactory

class DereplicateSequencesPrompter : public PrompterBase<DereplicateSequencesPrompter> {
public:
    DereplicateSequencesPrompter(Actor *a = 0)
        : PrompterBase<DereplicateSequencesPrompter>(a) {}

protected:
    QString composeRichDoc();
}; // DereplicateSequencesPrompter

}

#endif // _SPB_DEREPLICATE_SEQUENCES_WORKER_H_
