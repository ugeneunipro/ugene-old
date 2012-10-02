#ifndef _SPB_GENERATE_URL_WORKER_H_
#define _SPB_GENERATE_URL_WORKER_H_

#include <U2Core/U2OpStatus.h>

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

using namespace U2;
using namespace U2::LocalWorkflow;

namespace SPB {

class GenerateUrlWorker : public BaseWorker {
    Q_OBJECT
public:
    GenerateUrlWorker(Actor *a);

    virtual void init();
    virtual Task * tick();
    virtual void cleanup();

private:
    IntegralBus *inPort;
    IntegralBus *outPort;

private:
    QString getInUrl(U2OpStatus &os);
    QString generateUrl(const QString &inUrl);
    void sendUrl(const QString &url);
}; // GenerateUrlWorker

class GenerateUrlWorkerFactory : public DomainFactory {
public:
    GenerateUrlWorkerFactory() : DomainFactory(ACTOR_ID) {}

    static void init();
    virtual Worker * createWorker(Actor *a);

private:
    static const QString ACTOR_ID;
}; // GenerateUrlWorkerFactory

class GenerateUrlPrompter : public PrompterBase<GenerateUrlPrompter> {
public:
    GenerateUrlPrompter(Actor *a = 0)
        : PrompterBase<GenerateUrlPrompter>(a) {}

protected:
    QString composeRichDoc();
}; // GenerateUrlPrompter

} // SPB

#endif // _SPB_GENERATE_URL_WORKER_H_
