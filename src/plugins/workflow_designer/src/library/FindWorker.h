#ifndef _U2_FIND_WORKER_H_
#define _U2_FIND_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Algorithm/FindAlgorithm.h>
#include <U2Algorithm/FindAlgorithmTask.h>

namespace U2 {

namespace LocalWorkflow {

class FindPrompter : public PrompterBase<FindPrompter> {
    Q_OBJECT
public:
    FindPrompter(Actor* p = 0) : PrompterBase<FindPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class FindWorker : public BaseWorker {
    Q_OBJECT
public:
    FindWorker(Actor* a);
    
    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup();
    
private slots:
    void sl_taskFinished(Task*);

protected:
    CommunicationChannel *input, *output;
    QString resultName;
    QMap<Task*, QByteArray> patterns;
}; 

class FindWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    FindWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) {return new FindWorker(a);}
};

class FindAllRegionsTask : public Task { //FIXME this is temporary solution until FindAlgorithmTask moved to SequenceWalker
    Q_OBJECT
public:
    FindAllRegionsTask(const FindAlgorithmTaskSettings& s, const QList<SharedAnnotationData>&);
    virtual void prepare();
    QList<FindAlgorithmResult> getResult();
private:
    FindAlgorithmTaskSettings cfg; 
    QList<SharedAnnotationData> regions;
};

} // Workflow namespace
} // U2 namespace

#endif
