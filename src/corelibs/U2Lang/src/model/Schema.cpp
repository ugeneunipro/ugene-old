#include "Schema.h"
#include "ActorModel.h"

namespace U2 {
namespace Workflow {

/**************************
 * Schema
 **************************/
Schema::Schema() : deepCopy(false) {
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
    deepCopy = false;
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



}//Workflow namespace

}//GB2namespace
