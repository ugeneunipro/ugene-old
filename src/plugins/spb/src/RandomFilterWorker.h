#ifndef _SPB_RANDOM_FILTER_WORKER_H_
#define _SPB_RANDOM_FILTER_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

using namespace U2;
using namespace U2::LocalWorkflow;

namespace SPB {

class RandomFilterWorker : public BaseWorker {
    Q_OBJECT
public:
    RandomFilterWorker(Actor *a);

    virtual void init();
    virtual Task * tick();
    virtual void cleanup();

private:
    IntegralBus *inPort;
    IntegralBus *outPort;

    QMap<QString, QList<SharedDbiDataHandler> > sequences;

private:
    void addSequence(const QVariantMap &data);
    void sendData();
    int getMinSize();
    SharedDbiDataHandler takeRandomSequence(QList<SharedDbiDataHandler> &seqs);
};

class RandomFilterWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;

    RandomFilterWorkerFactory() : DomainFactory(ACTOR_ID) {}
    static void init();
    virtual Worker * createWorker(Actor *a);

}; // RandomFilterWorkerFactory

class RandomFilterPrompter : public PrompterBase<RandomFilterPrompter> {
public:
    RandomFilterPrompter(Actor *a = 0)
        : PrompterBase<RandomFilterPrompter>(a) {}

protected:
    QString composeRichDoc();
}; // RandomFilterPrompter

} // SPB

#endif // _SPB_RANDOM_FILTER_WORKER_H_
