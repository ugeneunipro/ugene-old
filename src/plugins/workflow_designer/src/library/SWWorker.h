#ifndef _U2_SW_WORKER_H_
#define _U2_SW_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Algorithm/SmithWatermanSettings.h>
#include <U2Algorithm/SmithWatermanTaskFactory.h>
#include <U2Algorithm/SmithWatermanReportCallback.h>

namespace U2 {

namespace LocalWorkflow {

class SWPrompter : public PrompterBase<SWPrompter> {
    Q_OBJECT
public:
    SWPrompter(Actor* p = 0) : PrompterBase<SWPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class SWAlgoEditor : public ComboBoxDelegate {
    Q_OBJECT
public:
    SWAlgoEditor(ActorPrototype* proto) : ComboBoxDelegate(QVariantMap()), proto(proto) {}
public slots:
    void populate();
private:
    ActorPrototype* proto;
};

class SWWorker : public BaseWorker {
    Q_OBJECT
public:
    SWWorker(Actor* a);
    
    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup();

private slots:
    void sl_taskFinished(Task*);

private:
    CommunicationChannel *input, *output;
    QMap<Task*, SmithWatermanReportCallbackImpl*> callbacks;
    QMap<Task*, QByteArray> patterns;
}; 

class SWWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    SWWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new SWWorker(a);}
};

} // Workflow namespace
} // U2 namespace

#endif
