#ifndef __IMPORT_ANNOTATIONS_WORKER_H_
#define __IMPORT_ANNOTATIONS_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Core/AnnotationData.h>

namespace U2 {
namespace LocalWorkflow {

class ImportAnnotationsWorker : public BaseWorker {
    Q_OBJECT
public:
    ImportAnnotationsWorker(Actor * p) : BaseWorker(p) {}
    
    virtual void init();
    virtual bool isReady();
    virtual Task * tick();
    virtual bool isDone();
    virtual void cleanup();
    
private slots:
    void sl_docsLoaded(Task *);
    
private:
    IntegralBus * inPort;
    IntegralBus * outPort;
    QMap<Task*, QList<SharedAnnotationData> > annsMap;
    
}; // ImportAnnotationsWorker

class ImportAnnotationsWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    
    ImportAnnotationsWorkerFactory() : DomainFactory(ACTOR_ID) {}
    static void init();
    virtual Worker * createWorker(Actor* a);
    
}; // ImportAnnotationsWorkerFactory

} // LocalWorkflow
} // U2

#endif // __IMPORT_ANNOTATIONS_WORKER_H_
