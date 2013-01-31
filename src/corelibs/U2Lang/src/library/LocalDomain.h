/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_WORKFLOW_LOCAL_DOMAIN_H_
#define _U2_WORKFLOW_LOCAL_DOMAIN_H_

#include <U2Lang/IntegralBus.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowTransport.h>
#include <U2Lang/WorkflowManager.h>

#include <QtCore/QQueue>
#include <limits.h>

namespace U2 {

namespace LocalWorkflow {

using namespace Workflow;

/**
 * currently, all wd workers inherits this class
 * 
 * base class for workers in integral bus model
 */
class U2LANG_EXPORT BaseWorker : public QObject, public Worker, public CommunicationSubject {
    Q_OBJECT
public:
    BaseWorker(Actor* a, bool autoTransitBus = true);
    virtual ~BaseWorker();
    
    virtual ActorId getActorId() const;

    virtual void setDone();

    // reimplemented from Worker
    virtual bool isDone();
    virtual bool isReady();
    
    // reimplemented from CommunicationSubject
    virtual bool addCommunication(const QString& name, CommunicationChannel* _ch);
    virtual CommunicationChannel* getCommunication(const QString& name);
    virtual QStringList getOutputFiles();
    
    // if you want your worker support scripts -> you should call this function to get Messages from channels
    // call this when channel has message
    // after calling: set all needed values for running your worker
    // called from 'tick' and then setup worker params
    virtual Message getMessageAndSetupScriptValues( CommunicationChannel * channel );

    QMap<QString, IntegralBus*> &getPorts() {return ports;}
    Actor * getActor() const {return actor;}

    /** Returns the value of a parameter with paramId */
    template<class T>
    T getValue(const QString &paramId) const;

private:
    // bind values from input ports to script vars. 
    // This function is called before 'get' data from channel -> to set up parameters for scripting
    void bindScriptValues();
    bool processDone;
    
protected:
    Actor* actor;
    // integral buses of actor's ports
    QMap<QString, IntegralBus*> ports;
    // workflow settings: worker task should fail on first error
    //bool failFast;
    
}; // BaseWorker

/**
 * simple realization of scheduler
 * worker tasks run sequentially, not in parallel
 *
 * bind to BaseWorker class as actor's workers
 */
class U2LANG_EXPORT SimplestSequentialScheduler : public Scheduler {
public:
    SimplestSequentialScheduler(Schema* sh);
    virtual ~SimplestSequentialScheduler();
    
    // reimplemented from Worker
    virtual void init();
    virtual bool isReady();
    virtual Task* tick();
    virtual bool isDone();
    virtual void cleanup();
    
    virtual WorkerState getWorkerState(ActorId);
    
private:
    Schema* schema;
    BaseWorker* lastWorker;
    Task* lastTask;

    WorkerState getWorkerState(Actor* a);
    
}; // SimplestSequentialScheduler

/**
 * simple realization of Communnication channel
 */
class U2LANG_EXPORT SimpleQueue : public CommunicationChannel {
public:
    SimpleQueue();
    virtual ~SimpleQueue(){}
    
    // reimplemented from CommunicationChannel
    virtual Message get();
    virtual Message look() const;
    virtual void put(const Message& m);
    virtual int hasMessage() const;
    virtual int takenMessages()const;
    virtual int hasRoom(const DataType* ) const;
    virtual bool isEnded() const;
    virtual void setEnded();
    // capacity is INT_MAX
    virtual int capacity() const;
    // does nothing
    virtual void setCapacity(int);
    
protected:
    // first in, first out
    QQueue<Message> que;
    // 'end' flag
    bool ended;
    // 
    int takenMsgs;
    
}; // SimpleQueue

/**
 * runtime domain for SimplestSequentialScheduler and SimpleQueue
 * 
 * currently, container for all DomainFactories of computational tasks
 */
class U2LANG_EXPORT LocalDomainFactory : public DomainFactory {
public:
    static const QString ID;

public:
    LocalDomainFactory();
    virtual ~LocalDomainFactory(){}
    
    virtual Worker* createWorker(Actor*);
    virtual CommunicationChannel* createConnection(Link*);
    virtual Scheduler* createScheduler(Schema*);
    virtual void destroy(Scheduler*, Schema*);
    
}; // LocalDomainFactory

/************************************************************************/
/* Template definitions */
/************************************************************************/
template<class T>
T BaseWorker::getValue(const QString &paramId) const {
    Attribute *attr = actor->getParameter(paramId);
    if (NULL == attr) {
        return T();
    }
    return attr->getAttributeValue<T>(context);
}

}//Workflow namespace

}//GB2 namespace

#endif // _U2_WORKFLOW_LOCAL_DOMAIN_H_
