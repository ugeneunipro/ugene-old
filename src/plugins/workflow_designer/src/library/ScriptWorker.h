/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_SCRIPT_WORKER_H_
#define _U2_SCRIPT_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

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
    static bool init(QList<DataTypePtr > input, QList<DataTypePtr > output, QList<Attribute*> attrs,
        const QString& name,const QString &description, const QString &actorFilePath);
    virtual Worker* createWorker(Actor* a) {return new ScriptWorker(a);}

    static const QString ACTOR_ID;
};


} // Workflow namespace
} // U2 namespace

#endif
