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

}//Workflow namespace

}//GB2 namespace

#endif // _U2_WORKFLOW_ITERATION_H_
