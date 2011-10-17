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

#ifndef _U2_WORKFLOW_MNGR_H_
#define _U2_WORKFLOW_MNGR_H_

#include <U2Lang/ActorModel.h>
#include <U2Lang/DbiDataStorage.h>
#include <U2Lang/WorkflowTransport.h>
#include <U2Lang/Schema.h>

#include <U2Core/Task.h>
#include <U2Core/IdRegistry.h>

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <assert.h>

namespace U2 {

namespace Workflow {

/**
 * Contains a common data for whole workflow running process
 */
class U2LANG_EXPORT WorkflowContext {
public:
    WorkflowContext();
    ~WorkflowContext();

    bool init();
    DbiDataStorage *getDataStorage();

private:
    DbiDataStorage *storage;
};

/**
 * Worker represents actor at runtime
 * takes input data from and puts output data to CommunicationChannels
 * 
 * base class
 */
class U2LANG_EXPORT Worker {
public:
    Worker() : context(NULL) {}
    virtual ~Worker() {}
    
    // initialize input and output ports
    virtual void init() = 0;
    // if data in the ports is ready
    virtual bool isReady() = 0;
    // get data from actor and return task
    // if you want your worker support scripting: 
    // use BaseWorker::getMessageAndSetupScriptValues to take data from port
    virtual Task* tick() = 0;
    // nothing to do
    virtual bool isDone() = 0;
    // opened files, etc...
    virtual void cleanup() = 0;

    void setContext(WorkflowContext *newContext) {context = newContext;}

protected:
    WorkflowContext *context;

}; // Worker

/**
* runtime state of worker
*/
enum WorkerState {
    WorkerWaiting, 
    WorkerReady, 
    WorkerRunning, 
    WorkerDone
}; // WorkerState

/**
 * Worker for whole schema
 * can decide if workers can run in parallel or any other analysis
 */
class U2LANG_EXPORT Scheduler : public Worker {
public:
    virtual WorkerState getWorkerState(ActorId) = 0;
    
}; // Scheduler

/**
 * represents controller between actor and runtime realizations of actor (Worker)
 * is a container of such controllers (inherits IdRegistry<DomainFactory>)
 * represents set of tasks of some computational domain
 *
 * says how to execute your worker (Scheduler and CommunicationChannel)
 * Usage: create execution domain factory (e.g. LocalDomainFactory) and register there your tasks factories
 */
class U2LANG_EXPORT DomainFactory : public IdRegistry<DomainFactory>, public Descriptor {
public:
    DomainFactory(const Descriptor& d) : Descriptor(d) {}
    DomainFactory(const QString& id) : Descriptor(id) {}
    virtual ~DomainFactory(){}
    
    // computational tasks domain
    virtual Worker* createWorker(Actor*) = 0;
    
    // execution domain
    virtual CommunicationChannel* createConnection(Link*) {return NULL;}
    virtual Scheduler* createScheduler(Schema*) {return NULL;}
    virtual void destroy(Scheduler*, Schema*) {}
    
}; // DomainFactory

/**
 * standard registry of factories
 * Usage: register here execution domain factories (see usage of DomainFactory)
 */
class U2LANG_EXPORT DomainFactoryRegistry : public IdRegistry<DomainFactory> {
}; // DomainFactoryRegistry

}//Workflow namespace

}//GB2 namespace


#endif
