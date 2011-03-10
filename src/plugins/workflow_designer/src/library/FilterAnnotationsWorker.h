#ifndef _U2_FILTER_ANNOTATIONS_WORKER_H_
#define _U2_FILTER_ANNOTATIONS_WORKER_H_

#include <U2Core/AnnotationData.h>

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>


namespace U2 {
namespace LocalWorkflow {

class FilterAnnotationsPrompter : public PrompterBase<FilterAnnotationsPrompter> {
    Q_OBJECT
public:
    FilterAnnotationsPrompter(Actor* p = 0) : PrompterBase<FilterAnnotationsPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class FilterAnnotationsWorker : public BaseWorker {
    Q_OBJECT
public:
    FilterAnnotationsWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {};

    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup();
private slots:
    void sl_taskFinished();
private:
    CommunicationChannel *input, *output;
    QList<SharedAnnotationData> inputAnns;
}; 

class FilterAnnotationsWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    static void init();
    FilterAnnotationsWorkerFactory() : DomainFactory(ACTOR_ID) {}
    virtual Worker* createWorker(Actor* a) { return new FilterAnnotationsWorker(a); }
};

class FilterAnnotationsTask : public Task {
    Q_OBJECT
public:
    FilterAnnotationsTask(QList<SharedAnnotationData>& annotations, const QStringList& names, bool accept)
        : Task(tr("Filter annotations task"), TaskFlag_None), annotations_(annotations), names_(names), accept_(accept) {}
    void run();
private:
    QList<SharedAnnotationData>& annotations_;
    QStringList names_;
    bool accept_;
};

} // LocalWorkflow namespace
} // U2 namespace

#endif
