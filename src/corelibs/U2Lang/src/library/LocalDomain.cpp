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

#include "LocalDomain.h"

#include <U2Lang/LastReadyScheduler.h>
#include <U2Lang/Schema.h>
#include <U2Lang/IntegralBusType.h>
#include <U2Lang/WorkflowSettings.h>
#include <U2Core/Log.h>

namespace U2 {
namespace LocalWorkflow {

const QString LocalDomainFactory::ID("domain.local.bio");

/*****************************
 * BaseWorker
 *****************************/
BaseWorker::BaseWorker(Actor* a, bool autoTransitBus) : actor(a) {
    foreach(Port* p, a->getPorts()) {
        if (qobject_cast<IntegralBusPort*>(p)) {
            IntegralBus* bus = new IntegralBus(p);
            ports.insert(p->getId(), bus);
            p->setPeer(bus);
        }
    }
    if (autoTransitBus) {
        foreach(Port* p, a->getInputPorts()) {
            IntegralBus* bus = p->castPeer<IntegralBus>();
            foreach(Port* op, a->getOutputPorts()) {
                if (p->isInput() != op->isInput()) {
                    IntegralBus* ob = op->castPeer<IntegralBus>();
                    ob->addComplement(bus);
                    bus->addComplement(ob);
                }
            }
        }
    }
    a->setPeer(this);
    //failFast = WorkflowSettings::failFast();
}

BaseWorker::~BaseWorker() {
    foreach(Port* p, actor->getPorts()) {
        if (qobject_cast<IntegralBusPort*>(p)) {
            p->setPeer(NULL);
        }
    }
    qDeleteAll(ports.values());
    actor->setPeer(NULL);
}

bool BaseWorker::addCommunication(const QString& id, CommunicationChannel* ch) {
    Q_UNUSED(id);
    Q_UNUSED(ch);
    assert(0);
    return false;
}

CommunicationChannel * BaseWorker::getCommunication(const QString& name) {
    return ports.value(name);
}

ActorId BaseWorker::getActorId() const {
    return actor->getId();
}

Message BaseWorker::getMessageAndSetupScriptValues( CommunicationChannel * channel ) {
    assert(channel != NULL);
    assert(channel->hasMessage());
    bindScriptValues();
    return channel->get();
}

void BaseWorker::bindScriptValues() {
    foreach( IntegralBus * bus, ports.values() ) {
        assert(bus != NULL);
        if(!bus->hasMessage()) { // means that it is bus for output port
            continue;
        }
        
        QVariantMap busData = bus->look().getData().toMap();
        foreach( Attribute * attribute, actor->getParameters().values() ) {
            assert(attribute != NULL);
            foreach(const QString & slotDesc, busData.keys()) {
                ActorId actorId = IntegralBusType::parseSlotDesc(slotDesc);
                QString attrId = IntegralBusType::parseAttributeIdFromSlotDesc(slotDesc);
                QString portId = bus->getPortId();
                IntegralBusPort * busPort = qobject_cast<IntegralBusPort*>(actor->getPort(portId));
                assert(busPort != NULL);
                
                Actor * bindedAttrOwner = busPort->getLinkedActorById(actorId);
                if(bindedAttrOwner == NULL) {
                    continue;
                }
                //attrId.replace(".", "_");
                //attrId.replace("-", "_");
                AttributeScript & attrScript = attribute->getAttributeScript();
                if( !attrScript.getScriptText().isEmpty() ) {
                    //attrScript.setVarValueWithId(attrId, busData.value(slotDesc));
                    attrScript.setScriptVar(attrId, busData.value(slotDesc));
                }
            }
        }
    }
}

/*****************************
 * SimplestSequentialScheduler
 *****************************/
SimplestSequentialScheduler::SimplestSequentialScheduler(Schema* sh) : schema(sh), lastWorker(NULL), lastTask(NULL) {
}

void SimplestSequentialScheduler::init() {
    foreach(Actor* a, schema->getProcesses()) {
        a->castPeer<BaseWorker>()->setContext(context);
        a->castPeer<BaseWorker>()->init();
    }
}

bool SimplestSequentialScheduler::isReady() {
    foreach(Actor* a, schema->getProcesses()) {
        if (a->castPeer<BaseWorker>()->isReady()) {
            return true;
        }
    }
    return false;
}

Task* SimplestSequentialScheduler::tick() {
    foreach(Actor* a, schema->getProcesses()) {
        if (a->castPeer<BaseWorker>()->isReady()) {
            lastWorker = a->castPeer<BaseWorker>();
            return lastTask = lastWorker->tick();
        }
    }
    assert(0);
    return NULL;
}

bool SimplestSequentialScheduler::isDone() {
    foreach(Actor* a, schema->getProcesses()) {
        if (!a->castPeer<BaseWorker>()->isDone()) {
            return false;
        }
    }
    return true;
}

void SimplestSequentialScheduler::cleanup() {
    foreach(Actor* a, schema->getProcesses()) {
        a->castPeer<BaseWorker>()->cleanup();
    }
}

SimplestSequentialScheduler::~SimplestSequentialScheduler() {
}

U2::Workflow::WorkerState SimplestSequentialScheduler::getWorkerState( ActorId id) {
    Actor* a = schema->actorById(id);
    assert(a);
    BaseWorker* w = a->castPeer<BaseWorker>();
    if (lastWorker == w) {
        Task* t = lastTask;
        if (w->isDone() && t && t->isFinished()) {
            return WorkerDone;
        }
        return WorkerRunning;
    }
    if (w->isDone()) {
        return WorkerDone;
    } else if (w->isReady()) {
        return WorkerReady;
    }
    return WorkerWaiting;
}

/*****************************
 * SimpleQueue
 *****************************/
SimpleQueue::SimpleQueue() : ended(false), takenMsgs(0) {
}

Message SimpleQueue::get() {
    assert(hasMessage());
    takenMsgs++;
    return que.dequeue();
}

Message SimpleQueue::look() const {
    assert(hasMessage()); 
    return que.head();
}

void SimpleQueue::put(const Message& m) {
    que.enqueue(m);
}

int SimpleQueue::hasMessage() const {
    return que.size();
}

int SimpleQueue::takenMessages() const {
    return takenMsgs;
}

int SimpleQueue::hasRoom(const DataType* ) const {
    return 1000;
}

bool SimpleQueue::isEnded() const {
    return ended && que.isEmpty();
}

void SimpleQueue::setEnded() {
    ended = true;
}

int SimpleQueue::capacity() const {
    return INT_MAX;
}

void SimpleQueue::setCapacity(int) {
}

/*****************************
 * LocalDomainFactory
 *****************************/
LocalDomainFactory::LocalDomainFactory() : DomainFactory(ID) {
}

static CommunicationSubject* setupBus(Port* p) {
    QString id = p->getId();
    BaseWorker* worker = p->owner()->castPeer<BaseWorker>();
    assert(worker);
    CommunicationSubject* subj = worker;
    IntegralBus* bus = qobject_cast<IntegralBus*>(p->castPeer<QObject>());
    if (bus) {
        assert(subj->getCommunication(id) == dynamic_cast<CommunicationChannel*>(bus));
        subj = bus;
    } else if (subj) {
        assert(0);
        bus = new IntegralBus(p);
        p->setPeer(bus);
        subj->addCommunication(id, bus);
        subj = bus;
        foreach(Port* op, p->owner()->getPorts()) {
            if (p->isInput() != op->isInput()) {
                IntegralBus* ob = qobject_cast<IntegralBus*>(op->castPeer<QObject>());
                if (ob) {
                    ob->addComplement(bus);
                    bus->addComplement(ob);
                }
            }
        }
    }
    return subj;
}

Worker* LocalDomainFactory::createWorker(Actor* a) {
    Worker* w = NULL;
    DomainFactory* f = getById(a->getProto()->getId());
    if (f) {
        w = f->createWorker(a);
#ifdef _DEBUG
        assert(w);
        BaseWorker* bw = dynamic_cast<BaseWorker*>(w);
        assert(qobject_cast<BaseWorker*>(bw));
        assert(bw == a->getPeer());
#endif
    }
    
    return w;    
}

CommunicationChannel* LocalDomainFactory::createConnection(Link* l) {
    SimpleQueue* cc = NULL;
    QString srcId = l->source()->getId();
    QString dstId = l->destination()->getId();
    CommunicationSubject* src = setupBus(l->source());
    CommunicationSubject* dst = setupBus(l->destination());
    if (src && dst) {
        cc = new SimpleQueue();
        src->addCommunication(srcId, cc);
        dst->addCommunication(dstId, cc);
    }
    l->setPeer(cc);
    return cc;
}

Scheduler* LocalDomainFactory::createScheduler(Schema* sh) {
    Scheduler *sc = NULL;
    if (NULL == sh->getActorBindingsGraph()) {
        sc = new SimplestSequentialScheduler(sh);
    } else {
        sc = new LastReadyScheduler(sh);
    }
    return sc;
}

void LocalDomainFactory::destroy( Scheduler* sh, Schema* schema) {
    foreach(Link* l, schema->getFlows()) {
        delete l->castPeer<SimpleQueue>();
        l->setPeer(NULL);
    }

    foreach(Actor* a, schema->getProcesses()) {
        delete a->castPeer<BaseWorker>();
    }

    delete sh;
}

} // Workflow namespace
} // U2 namespace
