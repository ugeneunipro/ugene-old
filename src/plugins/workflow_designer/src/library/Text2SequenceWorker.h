#ifndef __TEXT_2_SEQUENCE_WORKER_H_
#define __TEXT_2_SEQUENCE_WORKER_H_

#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/LocalDomain.h>

namespace U2 {
namespace LocalWorkflow {

class Text2SequencePrompter : public PrompterBase<Text2SequencePrompter> {
    Q_OBJECT
public:
    Text2SequencePrompter(Actor * p = NULL) : PrompterBase<Text2SequencePrompter>(p) {}

protected:
    QString composeRichDoc();
    
}; // Text2SequencePrompter

class Text2SequenceWorker : public BaseWorker {
    Q_OBJECT
public:
    static QMap<QString, QString> cuteAlIdNames;
    
public:
    Text2SequenceWorker(Actor * p) : BaseWorker(p), txtPort(NULL), outSeqPort(NULL), tickedNum(0) {}

    virtual void init();
    virtual bool isReady();
    virtual Task * tick();
    virtual bool isDone();
    virtual void cleanup();

private:
    static QMap<QString, QString> initCuteAlNames();
    
private:
    IntegralBus * txtPort;
    IntegralBus * outSeqPort;
    int tickedNum;
    
}; // Text2SequenceWorker

class Text2SequenceWorkerFactory : public DomainFactory {
public:
    static const QString ACTOR_ID;
    
    Text2SequenceWorkerFactory() : DomainFactory(ACTOR_ID) {}
    static void init();
    virtual Worker * createWorker(Actor* a);
    
}; // Text2SequenceWorkerFactory

} // LocalWorkflow
} // U2

#endif // __TEXT_2_SEQUENCE_WORKER_H_
