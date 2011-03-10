#ifndef _U2_SCRIPT_WORKER_H_
#define _U2_SCRIPT_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include "WorkflowScriptFunctions.h"

namespace U2 {

namespace LocalWorkflow {

class ScriptWorkerTask: public Task {
    Q_OBJECT
public:
    ScriptWorkerTask(QScriptEngine *_engine, AttributeScript*  _script ):Task(tr("Script worker task"),TaskFlag_None), isList(false),engine(_engine),script(_script) {}
    void run();
    QVariant getResult() const {return result;}
    QScriptEngine *getEngine() {return engine;}
    bool isList;

private:
    QVariant result;
    QScriptEngine *engine;
    AttributeScript *script;
};

class ScriptPromter : public PrompterBase<ScriptPromter> {
    Q_OBJECT
public:
    ScriptPromter( Actor * p = 0 ) : PrompterBase<ScriptPromter>(p) {};
protected:
    QString composeRichDoc();
};

class ScriptWorker: public BaseWorker {
    Q_OBJECT
public:
    ScriptWorker(Actor *a): BaseWorker(a), input(NULL), output(NULL) {script = a->getScript();}
    virtual void init();
    virtual bool isReady();
    virtual bool isDone();
    virtual Task* tick();
    virtual void cleanup() {};

private slots:
    void sl_taskFinished();

private:
    void bindAttributeVariables();
    void bindPortVariables();
    //QVariant scriptResult;

    CommunicationChannel *input, *output;
    QScriptEngine engine;
    AttributeScript *script;  
};

class ScriptWorkerFactory: public DomainFactory {
    
public:
    ScriptWorkerFactory(QString name) : DomainFactory(name) {}
    static bool init(QList<DataTypePtr > input, QList<DataTypePtr > output, QList<Attribute*> attrs, const QString& name,const QString &description);
    virtual Worker* createWorker(Actor* a) {return new ScriptWorker(a);}

    static const QString ACTOR_ID;
};


} // Workflow namespace
} // U2 namespace

#endif
