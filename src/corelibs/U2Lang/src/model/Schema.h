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

#ifndef _U2_WORKFLOW_ITERATION_H_
#define _U2_WORKFLOW_ITERATION_H_

#include <U2Lang/Attribute.h>

#include <QtCore/QPair>

typedef QPair<U2::ActorId,QString> IterationCfgKey;
typedef QMap<IterationCfgKey, QVariant> IterationCfg;
typedef QMap<U2::ActorId, QVariantMap> CfgMap; 

Q_DECLARE_METATYPE(IterationCfg)
Q_DECLARE_METATYPE(CfgMap)

namespace U2 {

namespace Workflow {

class Actor;
class Link;
class Iteration;
class Port;
class ActorBindingsGraph;

/**
 * Schema is oriented graph of actors
 * graph is oriented because Link has orientation
 */
class U2LANG_EXPORT Schema {
public:
    Schema();
    virtual ~Schema();
    Schema( const Schema & other );
    Schema & operator=( const Schema & other );
    
    // after schema is copied its actor's id's changed
    // in that case we need to configurate new schema with values from Iteration using actors mapping
    void applyConfiguration(const Iteration&, QMap<ActorId, ActorId>);
    
    Actor* actorById(ActorId);
    int iterationById(int);
    
    QString getDomain() const;
    void setDomain(const QString & d);
    
    const QList<Iteration> & getIterations() const;
    QList<Iteration> & getIterations();

    void setActorBindingsGraph(const ActorBindingsGraph &graph);
    const ActorBindingsGraph *getActorBindingsGraph() const;
    ActorBindingsGraph *getActorBindingsGraph();
    
    const QList<Actor*> & getProcesses() const;
    void addProcess(Actor * a);
    
    const QList<Link*> & getFlows() const;
    void addFlow(Link* l);
    
    void setDeepCopyFlag(bool flag);
    
    void reset();
    
    bool hasParamAliases() const;
    bool hasAliasHelp() const;
    
private:
    // set of actors
    QList<Actor*> procs;
    // set of links between actors
    QList<Link*> flows;
    // list of iterations that user has fulfilled
    QList<Iteration> iterations;
    // name of domain in which we work now
    // default is LocalDomainFactory::ID
    QString domain;
    // true if schema was deeply copied
    // this means that every actor, link and iteration was copied
    // if true -> need to delete all corresponding data
    bool deepCopy;
    ActorBindingsGraph *graph;
    
}; // Schema

/**
 * Iteration is a set of values for schema's attributes
 * 
 * using schema and iteration you can parametrize schema and then run it
 */
class U2LANG_EXPORT Iteration {
public:
    Iteration();
    Iteration(const QString& name);
    Iteration(const Iteration & it);
    
    QVariantMap getParameters(const ActorId& id) const;
    // when actor changes id (if schema was deeply copied)
    // we need to remap iteration's data to new actorId
    void remap(QMap<ActorId, ActorId>);
    
    bool isEmpty() const;
    
private:
    static int nextId();
    
public:
    // each configuration has name
    // default name is 'default'
    QString name;
    //
    int id;
    // for each actor in schema iteration saves QVariantMap of it's attributes values
    // QMap<attributeId, attributeValue>
    // this QVariantMap contains only those attributes that were changed by user
    QMap<ActorId, QVariantMap> cfg;
    
}; // Iteration

/**
 * Schema's metadata
 * saves with schema to file and loads with it
 * 
 * Schema don't aggregate metadata (see WorkflowViewController for usage)
 */
class U2LANG_EXPORT Metadata {
public:
    Metadata();
    
    void reset();
    
public:
    QString name;
    QString url;
    QString comment;
    
}; // Metadata

/**
 * Schema's actors' graph
 * saves with schema to file and loads with it
 */
class U2LANG_EXPORT ActorBindingsGraph {
public:
    ActorBindingsGraph() {}
    virtual ~ActorBindingsGraph() {}

    bool validateGraph(QString &message);
    bool addBinding(Actor *actor, Port *port);
    bool contains(Actor *actor, Port *port);
    const QMap<Actor*, QList<Port*> > getBindings() const;

private:
    QMap<Actor*, QList<Port*> > bindings;
}; // ActorBindingsGraph

}//Workflow namespace

}//GB2 namespace

#endif // _U2_WORKFLOW_ITERATION_H_
