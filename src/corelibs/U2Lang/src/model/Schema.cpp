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

#include <memory>

#include <U2Lang/ActorModel.h>
#include <U2Lang/IntegralBusType.h>
#include <U2Lang/WorkflowEnv.h>

#include "Schema.h"

namespace U2 {
namespace Workflow {

/**************************
 * Schema
 **************************/
Schema::Schema() : deepCopy(false), graph(NULL) {
}

Schema::~Schema() {
    reset();
}

Schema::Schema( const Schema & other ) {
    *this = other;
}

Schema & Schema::operator =( const Schema & other ) {
    procs = other.procs;
    flows = other.flows;
    iterations = other.iterations;
    domain = other.domain;
    graph = (NULL == other.graph) ? NULL : new ActorBindingsGraph(*(other.graph));
    deepCopy = false;
    portAliases = other.portAliases;
    includedTypeName = other.includedTypeName;
    return *this;
}

void Schema::applyConfiguration(const Iteration& cfg, QMap<ActorId, ActorId> remap) {
    foreach(Actor* a, procs) {
        ActorId id = remap.key(a->getId());
        if (cfg.cfg.contains(id)) {
            a->setParameters(cfg.cfg.value(id));
        }
    }
}

void Schema::reset() {
    if (deepCopy) {
        qDeleteAll(flows);
        flows.clear();
        qDeleteAll(procs);
        procs.clear();
        delete graph;
        graph = NULL;
    }
}

Actor* Schema::actorById( ActorId id) {
    foreach(Actor* a, procs) {
        if (a->getId() == id) {
            return a;
        }
    }
    return NULL;
}

QList<Actor*> Schema::actorsByOwnerId(ActorId id) {
    QList<Actor*> res;
    foreach (Actor *proc, procs) {
        if (proc->getOwner() == id) {
            res.append(proc);
        }
    }
    return res;
}

int Schema::iterationById(int id) {
    for(int i = 0; i < iterations.size(); i++) {
        if (iterations.at(i).id == id) {
            return i;
        }
    }
    return -1;
}

QString Schema::getDomain() const {
    return domain;
}

void Schema::setDomain(const QString & d) {
    domain = d;
}

const QList<Iteration> & Schema::getIterations() const {
    return iterations;
}

QList<Iteration> & Schema::getIterations() {
    return iterations;
}

void Schema::setActorBindingsGraph(const ActorBindingsGraph &newGraph) {
    graph = new ActorBindingsGraph(newGraph);
}
const ActorBindingsGraph *Schema::getActorBindingsGraph() const {
    return graph;
}
ActorBindingsGraph *Schema::getActorBindingsGraph() {
    return graph;
}

const QList<Actor*> & Schema::getProcesses() const {
    return procs;
}

void Schema::addProcess(Actor * a) {
    assert(a != NULL);
    procs.append(a);
}

const QList<Link*> & Schema::getFlows() const {
    return flows;
}

void Schema::addFlow(Link* l) {
    assert(l != NULL);
    flows.append(l);
}

void Schema::setDeepCopyFlag(bool flag) {
    deepCopy = flag;
}

bool Schema::hasParamAliases() const {
    foreach(Actor * actor, procs) {
        if(actor->hasParamAliases()) {
            return true;
        }
    }
    return false;
}

bool Schema::hasAliasHelp() const {
    foreach(Actor * a, procs) {
        if(a->hasAliasHelp()) {
            return true;
        }
    }
    return false;
}

bool Schema::hasPortAliases() const {
    return !portAliases.isEmpty();
}

const QList<PortAlias> &Schema::getPortAliases() const {
    return portAliases;
}

bool Schema::addPortAlias(const PortAlias &newAlias) {
    foreach (const PortAlias &alias, portAliases) {
        if (alias.getAlias() == newAlias.getAlias()) {
            return false;
        }
        if (alias.getSourcePort() == newAlias.getSourcePort()) {
            return false;
        }
    }

    portAliases.append(newAlias);
    return true;
}

void Schema::setPortAliases(const QList<PortAlias> &aliases) {
    portAliases = aliases;
}

QString Schema::getTypeName() const {
    return includedTypeName;
}

void Schema::setTypeName(const QString &typeName) {
    this->includedTypeName = typeName;
}

using namespace std;

bool Schema::recursiveExpand(QList<QString> &schemaIds) {
    QMap<Actor*, Schema*> subSchemas;

    // Expand all processes
    foreach (Actor *proc, procs) {
        ActorPrototype *proto = proc->getProto();
        if (!proto->isSchemaFlagSet()) {
            continue;
        }

        if (schemaIds.contains(proto->getId())) {
            return false;
        }

        Schema *schema = WorkflowEnv::getSchemaActorsRegistry()->getSchema(proto->getId());
        if (NULL == schema) {
            return false;
        }

        QList<QString> newIdList(schemaIds);
        newIdList.append(proto->getId());
        bool res = schema->recursiveExpand(newIdList);
        if (!res) {
            return false;
        }

        subSchemas.insert(proc, schema);
    }

    // Everything is all right after expanding. So replace expanded processes
    foreach (Actor *proc, procs) {
        if (!proc->getProto()->isSchemaFlagSet()) {
            continue;
        }
        Schema *schema = subSchemas.value(proc);

        // set owner
        foreach (Actor *subProc, schema->getProcesses()) {
            subProc->setOwner(proc->getId());
        }

        // replace parameters
        foreach (Actor *subProc, schema->getProcesses()) {
            if (subProc->hasParamAliases()) {
                setAliasedAttributes(proc, subProc);
            }
        }

        // replace ports and slots
        foreach (const PortAlias &subPortAlias, schema->getPortAliases()) {
            if (subPortAlias.isInput()) {
                replaceInLinksAndSlots(proc, subPortAlias);
            } else {
                replaceOutLinks(proc, subPortAlias);
                replaceOutSlots(proc, subPortAlias);
            }

            if (this->hasPortAliases()) {
                replacePortAliases(subPortAlias);
            }
        }   

        flows.append(schema->getFlows());
        graph->getBindings().unite(schema->getActorBindingsGraph()->getBindings());

        // replace procs
        procs.removeOne(proc);
        procs.append(schema->getProcesses());
    }

    return true;
}

bool Schema::expand() {
    QList<QString> emptyList;
    return this->recursiveExpand(emptyList);
}

void Schema::setAliasedAttributes(Actor *proc, Actor *subProc) {
    QMap<QString, QString> newParamAliases;

    foreach (QString subAttrId, subProc->getParamAliases().keys()) {
        QString alias = subProc->getParamAliases().value(subAttrId);

        QVariant value = proc->getParameter(alias)->getAttributePureValue();
        subProc->getParameter(subAttrId)->setAttributeValue(value);
        AttributeScript script = proc->getParameter(alias)->getAttributeScript();
        subProc->getParameter(subAttrId)->getAttributeScript() = script;

        if (proc->getParamAliases().keys().contains(alias)) {
            newParamAliases.insert(subAttrId, proc->getParamAliases().value(alias));
        }
    }
    subProc->getParamAliases() = newParamAliases;
}

void Schema::replaceInLinksAndSlots(Actor *proc, const PortAlias &portAlias) {
    Port *port = proc->getPort(portAlias.getAlias());
    Actor *subProc = portAlias.getSourcePort()->owner();
    Port *subPort = subProc->getPort(portAlias.getSourcePort()->getId());

    foreach (Link *link, this->getFlows()) {
        if (link->destination() == port) {
            // replace ports link
            link->disconnect();
            link->connect(link->source(), subPort);
            graph->removeBinding(link->source(), port);
            graph->addBinding(link->source(), subPort);

            // replace slots links
            Attribute *a = port->getParameter(IntegralBusPort::BUS_MAP_ATTR_ID);
            QStrStrMap busMap = a->getAttributeValueWithoutScript<QStrStrMap>();
            QStrStrMap subBusMap;
            foreach (const SlotAlias &slotAlias, portAlias.getSlotAliases()) {
                subBusMap[slotAlias.getSourceSlotId()] = busMap[slotAlias.getAlias()];
            }
            subPort->getParameter(IntegralBusPort::BUS_MAP_ATTR_ID)->setAttributeValue(qVariantFromValue(subBusMap));
        }
    }
}

void Schema::replaceOutLinks(Actor *origProc, const PortAlias &portAlias) {
    Port *port = origProc->getPort(portAlias.getAlias());
    Actor *subProc = portAlias.getSourcePort()->owner();
    Port *subPort = subProc->getPort(portAlias.getSourcePort()->getId());

    foreach (Link *link, this->getFlows()) {
        if (link->source() == port) {
            // replace only ports link
            link->disconnect();
            link->connect(subPort, link->destination());
            graph->removeBinding(port, link->destination());
            graph->addBinding(subPort, link->destination());
        }
    }
}

void Schema::replaceOutSlots(Actor *origProc, const PortAlias &portAlias) {
    // replace slots links
    foreach (Actor *proc, procs) {
        foreach (Port *p, proc->getInputPorts()) {
            Attribute *a = p->getParameter(IntegralBusPort::BUS_MAP_ATTR_ID);
            QStrStrMap busMap = a->getAttributeValueWithoutScript<QStrStrMap>();
            QStrStrMap newMap;

            QMapIterator<QString, QString> it(busMap);
            while (it.hasNext()) {
                it.next();
                // replace ids at slots' values
                QString value = it.value();
                foreach (const SlotAlias &slotAlias, portAlias.getSlotAliases()) {
                    QString origSlotId = slotAlias.getAlias();
                    QString subSlotId = slotAlias.getSourceSlotId();

                    QString slotString = origProc->getId() + ":" + origSlotId;
                    int idPos = value.indexOf(slotString);
                    while (idPos >= 0) {
                        Actor *subProc = slotAlias.getSourcePort()->owner();
                        value.remove(idPos, slotString.length());
                        value.insert(idPos, subProc->getId()+":"+subSlotId);
                        idPos = value.indexOf(slotString);
                    }
                }
                newMap.insert(it.key(), value);
            }

            a->setAttributeValue(qVariantFromValue(newMap));
        }
    }
}

void Schema::replacePortAliases(const PortAlias &subPortAlias) {
    // replace port aliases
    QList<PortAlias> newPortAliases;
    foreach (PortAlias origPortAlias, this->portAliases) {
        if (origPortAlias.getSourcePort()->getId() == subPortAlias.getAlias()) {
            origPortAlias.setNewSourcePort(subPortAlias.getSourcePort());
        }

        // replace slot aliases
        QList<SlotAlias> newSlotAliases;
        foreach (const SlotAlias &origSlotAlias, origPortAlias.getSlotAliases()) {
            if (origSlotAlias.getSourcePort()->getId() == subPortAlias.getAlias()) {
                foreach (const SlotAlias &subSlotAlias, subPortAlias.getSlotAliases()) {
                    if (subSlotAlias.getAlias() == origSlotAlias.getSourceSlotId()) {
                        SlotAlias newSlotAlias(subSlotAlias.getSourcePort(), subSlotAlias.getSourceSlotId(), origSlotAlias.getAlias());
                        newSlotAliases.append(newSlotAlias);
                        break;
                    }
                }
            } else {
                newSlotAliases.append(origSlotAlias);
            }
        }
        origPortAlias.setNewSlotAliases(newSlotAliases);
        newPortAliases.append(origPortAlias);
    }
    this->portAliases = newPortAliases;
}

/**************************
 * Iteration
 **************************/
Iteration::Iteration() : id(nextId()) {
}

Iteration::Iteration(const QString& n) : name(n), id(nextId()) {
}

Iteration::Iteration(const Iteration & it) : name(it.name), id(it.id), cfg(it.cfg) {
}

int Iteration::nextId() {
    static int id = 0;
    return id++;
}

void Iteration::remap(QMap<ActorId, ActorId> map) {
    CfgMap newCfg;
    QMapIterator< ActorId, QVariantMap> it(cfg);
    while (it.hasNext())
    {
        it.next();
        newCfg.insert(map.value(it.key()), it.value());
    }
    cfg = newCfg;
}

bool Iteration::isEmpty() const {
    return cfg.isEmpty();
}

const QMap<ActorId, QVariantMap> &Iteration::getConfig() const {
    return cfg;
}

QMap<ActorId, QVariantMap> &Iteration::getConfig() {
    return cfg;
}

QVariantMap Iteration::getParameters(const ActorId& id) const  {
    return cfg.value(id);
}

/**************************
 * Metadata
 **************************/
Metadata::Metadata() { 
    reset(); 
}

void Metadata::reset() {
    name = QString();
    comment = QString();
    url = QString();
}

/**************************
 * ActorBindingGraph
 **************************/
bool ActorBindingsGraph::validateGraph(QString &) {
    return true;
}

bool ActorBindingsGraph::addBinding(Port *source, Port *dest) {
    QList<Port*> ports;
    if (bindings.contains(source)) {
        ports = bindings.value(source);
        if (ports.contains(dest)) {
            return false;
        }
    }
    ports.append(dest);
    bindings.insert(source, ports);
    return true;
}

bool ActorBindingsGraph::contains(Port *source, Port *dest) {
    if (bindings.contains(source)) {
        QList<Port*> &ports = bindings[source];
        return ports.contains(dest);
    }
    return false;
}

void ActorBindingsGraph::removeBinding(Port *source, Port *dest) {
    if (bindings.contains(source)) {
        QList<Port*> &ports = bindings[source];
        ports.removeOne(dest);
    }
}

const QMap<Port*, QList<Port*> > &ActorBindingsGraph::getBindings() const {
    return bindings;
}

QMap<Port*, QList<Port*> > &ActorBindingsGraph::getBindings() {
    return bindings;
}

QMap<int, QList<Actor*> > ActorBindingsGraph::getTopologicalSortedGraph(QList<Actor*> actors) const {
    QMap<Actor*, QList<Port*> > graph;
    foreach (Port *source, bindings.keys()) {
        if (graph.contains(source->owner())) {
            graph[source->owner()].append(bindings.value(source));
        } else {
            graph.insert(source->owner(), bindings.value(source));
        }
    }
    QMap<int, QList<Actor*> > result;

    int vertexLabel = 0;
    while (!graph.isEmpty()) {
        QList<Actor*> endVertexes;
        {
            foreach (Actor *a, actors) {
                if (!graph.keys().contains(a)) { // so, there is no arcs from this actor
                    endVertexes.append(a);
                }
            }
        }
        result.insert(vertexLabel, endVertexes);

        foreach (Actor *a, graph.keys()) {
            QList<Port*> ports = graph.value(a);
            foreach (Port *p, ports) {
                if (endVertexes.contains(p->owner())) {
                    ports.removeOne(p);
                }
            }
            if (ports.isEmpty()) {
                graph.remove(a);
            } else {
                graph.insert(a, ports);
            }
        }

        foreach (Actor *a, endVertexes) {
            actors.removeOne(a);
        }
        vertexLabel++;
    }
    result.insert(vertexLabel, actors);

    return result;
}

}//Workflow namespace

}//GB2namespace
