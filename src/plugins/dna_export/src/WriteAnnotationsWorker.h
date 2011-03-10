#ifndef __WRITE_ANNOTATIONS_WORKER_H_
#define __WRITE_ANNOTATIONS_WORKER_H_

#include <U2Core/AnnotationTableObject.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/LocalDomain.h>

namespace U2 {
namespace LocalWorkflow {

class WriteAnnotationsPrompter : public PrompterBase<WriteAnnotationsPrompter> {
    Q_OBJECT
public:
    WriteAnnotationsPrompter(Actor * p = NULL) : PrompterBase<WriteAnnotationsPrompter>(p) {}

protected:
    QString composeRichDoc();

}; // WriteAnnotationsPrompter

class WriteAnnotationsWorker : public BaseWorker {
    Q_OBJECT
public:
    WriteAnnotationsWorker(Actor * p) : BaseWorker(p), annotationsPort(NULL) {}
    ~WriteAnnotationsWorker();
    
    virtual void init();
    virtual bool isReady();
    virtual Task * tick();
    virtual bool isDone();
    virtual void cleanup();
    
private:
    IntegralBus * annotationsPort;
    QList<AnnotationTableObject*> createdAnnotationObjects;
    
}; // WriteAnnotationsWorker

class WriteAnnotationsWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    
    WriteAnnotationsWorkerFactory() : DomainFactory(ACTOR_ID) {}
    static void init();
    virtual Worker * createWorker(Actor* a);
    
}; // WriteAnnotationsWorkerFactory

}
} // U2

#endif // __WRITE_ANNOTATIONS_WORKER_H_
