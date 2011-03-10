#ifndef __SEQUENCE_SPLIT_WORKER_H__
#define __SEQUENCE_SPLIT_WORKER_H__

#include <U2Core/AnnotationData.h>
#include <U2Core/DNASequence.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/ExtractAnnotatedRegionTask.h>

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

namespace U2 {
namespace LocalWorkflow {

class SequenceSplitPromter : public PrompterBase<SequenceSplitPromter> {
    Q_OBJECT
public:
    SequenceSplitPromter( Actor * p = 0 ) : PrompterBase<SequenceSplitPromter>(p) {};
protected:
    QString composeRichDoc();
};

class SequenceSplitWorker : public BaseWorker {
    Q_OBJECT
public:
    SequenceSplitWorker( Actor * p ) : BaseWorker(p, /*auto transit*/false), 
        seqPort(NULL), outPort(NULL), useAcceptedOrFiltered(false) {};

    virtual void init();
    virtual bool isReady();
    virtual Task * tick();
    virtual bool isDone();
    virtual void cleanup();

protected:
    IntegralBus * seqPort;
    IntegralBus * outPort;

private:
    QList<Task *> ssTasks;
    QList<SharedAnnotationData> inputAnns;

    QStringList acceptedNames;
    QStringList filteredNames;
    bool useAcceptedOrFiltered;
    ExtractAnnotatedRegionTaskSettings cfg;
private slots:
    void sl_onTaskFinished( Task * t );
};

class SequenceSplitWorkerFactory : public DomainFactory {
public:
    const static QString ACTOR;
    SequenceSplitWorkerFactory() : DomainFactory( ACTOR ) {};
    static void init();
    virtual Worker * createWorker( Actor * a );
};

} //ns LocalWorkflow
} //ns U2

#endif
