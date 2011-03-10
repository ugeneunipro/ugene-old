#ifndef _U2_SITECON_WORKER_H_
#define _U2_SITECON_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "SiteconAlgorithm.h"
#include "SiteconSearchTask.h"

namespace U2 {

namespace LocalWorkflow {

class SiteconBuildPrompter : public PrompterBase<SiteconBuildPrompter> {
    Q_OBJECT
public:
    SiteconBuildPrompter(Actor* p = 0) : PrompterBase<SiteconBuildPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class SiteconSearchPrompter : public PrompterBase<SiteconSearchPrompter> {
    Q_OBJECT
public:
    SiteconSearchPrompter(Actor* p = 0) : PrompterBase<SiteconSearchPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class SiteconBuildWorker : public BaseWorker {
    Q_OBJECT
public:
    static const QString ACTOR_ID;
    static void registerProto();

    SiteconBuildWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {}
    virtual void init() ;
    virtual bool isReady();
    virtual Task* tick() ;
    virtual bool isDone() ;
    virtual void cleanup() {}
private slots:
    void sl_taskFinished();

protected:
    CommunicationChannel *input, *output;
    SiteconBuildSettings cfg;
    DataTypePtr mtype;
}; 

class SiteconSearchWorker : public BaseWorker {
    Q_OBJECT
public:
    static const QString ACTOR_ID;
    static void registerProto();

    SiteconSearchWorker(Actor* a) : BaseWorker(a, false), //FIXME??
        modelPort(NULL), dataPort(NULL), output(NULL), strand(0) {}
    virtual void init() ;
    virtual bool isReady();
    virtual Task* tick() ;
    virtual bool isDone() ;
    virtual void cleanup() {}
    private slots:
        void sl_taskFinished(Task*);

protected:
    IntegralBus *modelPort, *dataPort, *output;
    QString resultName;
    QList<SiteconModel> models;
    int strand;
    SiteconSearchCfg cfg;
};

class SiteconWorkerFactory : public DomainFactory {
public:
    static const Descriptor SITECON_CATEGORY();
    static const QString SITECON_MODEL_TYPE_ID;
    static const Descriptor SITECON_SLOT;
    static DataTypePtr const SITECON_MODEL_TYPE();
    static void init();
    SiteconWorkerFactory(const Descriptor& d) : DomainFactory(d) {}
    virtual Worker* createWorker(Actor* a);
};

} // Workflow namespace
} // U2 namespace

#endif
