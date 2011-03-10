#ifndef _U2_QD_WORKER_H_
#define _U2_QD_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>


namespace U2 {

class QDScheme;
class AnnotationTableObject;

namespace LocalWorkflow {

class QDPrompter;
typedef PrompterBase<QDPrompter> QDPrompterBase;

class QDPrompter : public QDPrompterBase {
    Q_OBJECT
public:
    QDPrompter(Actor* p = 0) : QDPrompterBase(p) {}
protected:
    QString composeRichDoc();
};

class QDWorker : public BaseWorker {
    Q_OBJECT
public:
    QDWorker(Actor* a);

    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup();
private slots:
    void sl_taskFinished(Task*);
protected:
    CommunicationChannel *input, *output;
    QDScheme* scheme;
};

class QDWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    QDWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) { return new QDWorker(a); }
};

}//Workflow namespace
}//U2 namespace

#endif
