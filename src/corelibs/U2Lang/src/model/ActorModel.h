/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_WORKFLOW_MODEL_H_
#define _U2_WORKFLOW_MODEL_H_

#include <QtCore/QMimeData>
#include <QtGui/QTextDocument>

#include <U2Lang/Datatype.h>
#include <U2Lang/Attribute.h>
#include <U2Lang/Configuration.h>
#include <U2Lang/ConfigurationEditor.h>
#include <U2Lang/ConfigurationValidator.h>
#include <U2Lang/Peer.h>
#include <U2Lang/Port.h>
#include <U2Lang/PortMapping.h>

namespace U2 {

namespace Workflow {

class Actor;
class ActorDocument;
class Prompter;

/**
 * Actor represents particular semantic template 
 * and can be configured via set of template-specific parameters
 * ActorPrototype is such a template
 */
class U2LANG_EXPORT ActorPrototype : public VisualDescriptor {
public:
    ActorPrototype(const Descriptor& desc, 
        const QList<PortDescriptor*>& ports = QList<PortDescriptor*>(), 
        const QList<Attribute*>& attrs = QList<Attribute*>());
    virtual ~ActorPrototype();
    
    QList<PortDescriptor*> getPortDesciptors() const;
    
    QList<Attribute*> getAttributes() const;
    Attribute * getAttribute( const QString & id ) const;
    void addAttribute( Attribute * a );
    int removeAttribute( Attribute * attr );
    
    void setEditor(ConfigurationEditor* e);
    ConfigurationEditor * getEditor() const;
    
    // validator has setter but no getter
    void setValidator(ConfigurationValidator* v);
    
    // prompter has no getter
    void setPrompter(Prompter* p);
    
    // port validators has no getters
    // all validators will be used as validator in corresponding port
    // see createInstance()
    void setPortValidator(const QString& id, ConfigurationValidator* v);
    
    // for drag'n'drop purposes
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap* = NULL) const;

    virtual Actor* createInstance(const ActorId &actorId, AttributeScript *script = NULL, const QVariantMap& params = QVariantMap());
    
    void setScriptFlag(bool flag = true);
    bool isScriptFlagSet() {return isScript;}

    void setSchema(const QString &path);
    void setNonStandard(const QString &path);
    bool isStandardFlagSet() {return isStandard;}
    bool isSchemaFlagSet() {return isSchema;}
    bool isExternalTool() {return !isStandard && !isSchema && !isScript;}
    QString getFilePath() {return actorFilePath;}

    bool isAllowsEmptyPorts() const {return allowsEmptyPorts;}
    void setAllowsEmptyPorts(bool value) {allowsEmptyPorts = value;}

    bool getInfluenceOnPathFlag() const {return influenceOnPathFlag;}
    void setInfluenceOnPathFlag(bool value) {influenceOnPathFlag = value;}

    void addSlotRelation(const QString &headPortId, const QString headSlot, const QString &depPortId, const QString &depSlot);
    QStrStrMap getSlotRelations() const;
    
protected:
    // create port and sets p as owner of new port
    // caller should add created port to actor's ports see createInstance
    virtual Port* createPort(const PortDescriptor& d, Actor* p);
    
protected:
    // list of attributes
    // can be changed via addAttribute and removeAttribute
    QList<Attribute*> attrs;
    // list of port's
    // real Port's are created via createPort
    QList<PortDescriptor*> ports; 
    // controller for actor's configuration editor
    ConfigurationEditor* ed;
    // makes any non-trivial validations
    ConfigurationValidator* val;
    // some realization of Prompter (e.g. PrompterBaseImpl)
    Prompter* prompter;
    // as if each port is configuration
    // we need port validators
    // QString here - id of corresponding PortDescriptor
    QMap<QString, ConfigurationValidator*> portValidators;
    // actor can be written on a script by user
    // in such case user can define attributes and input, output ports of actor
    bool isScript;
    // actor can be a standard actor or external tool or script actor included from file
    bool isStandard;
    // if actor is non standard then its meta is kept in actorFilePath
    QString actorFilePath;
    // actor can implement some big schema
    bool isSchema;

    bool allowsEmptyPorts;
    bool influenceOnPathFlag;

    // Map< Dependent slotid, Head slotid >
    // slotid is "port.slot"
    QStrStrMap slotRelations;
    
}; // ActorPrototype

/**
 * It is a configuration - it has set of named attributes
 * with set of Ports and links between them
 *
 * represents one of ActorPrototypes
 * Peer is needed for saving Worker that is associated with current actor (see SimplestSequentialScheduler::tick())
 */
class U2LANG_EXPORT Actor: public QObject, public Configuration, public Peer {
    Q_OBJECT
public:
    virtual ~Actor();
    
    ActorPrototype* getProto() const;
    
    // this id is an actor block name at the *.uwl file
    ActorId getId() const;
    void setId(const ActorId &id);
    
    QString getLabel() const;
    void setLabel(const QString& l);
    
    Port* getPort(const QString& id) const;
    QList<Port*> getPorts() const;
    QList<Port*> getInputPorts() const;
    QList<Port*> getOutputPorts() const;
    
    // reimplemented: Configuration::setParameter
    virtual void setParameter(const QString& name, const QVariant& val);
    
    // NULL if description not set by user
    ActorDocument* getDescription() const;
    void setDescription(ActorDocument* d);
    
    const QMap<QString, QString> & getParamAliases() const;
    QMap<QString, QString> & getParamAliases();
    bool hasParamAliases() const;
    
    const QMap<QString, QString> & getAliasHelp() const;
    QMap<QString, QString> & getAliasHelp();
    bool hasAliasHelp() const;
    
    // reimplemented: Configuration::remap
    virtual void remap(const QMap<ActorId, ActorId>&);
    
    // setups variables for script
    void setupVariablesForPort();
    void setupVariablesForAttribute();
    
    AttributeScript *getScript() const;
    void setScript(AttributeScript* _script);

    ActorId getOwner() const;
    void setOwner(const ActorId &owner);

    /**
     * Ids of other actors in the scheme can be changed (after copy-paste, for example).
     * It is needed to update all of ids which are used by this actor
     */
    virtual void updateActorIds(const QMap<ActorId, ActorId> &actorIdsMap);

    void update(const QMap<ActorId, ActorId> &actorsMapping);
    void replaceActor(Actor *oldActor, Actor *newActor, const QList<PortMapping> &mappings);
    
signals:
    void si_labelChanged();
    void si_modified();
    
protected:
    friend class ActorPrototype;
    Actor(const ActorId &actorId, ActorPrototype* proto, AttributeScript *script);
    Actor(const Actor&);
    
protected:
    ActorId id;
    // name of actor
    // can be set by user
    // default is: 'proto->getDisplayName() actorId'
    QString label;
    // semantic template of this actor
    ActorPrototype* proto;
    // ports, binded to their names
    QMap<QString,Port*> ports;
    // description of actor
    // has setter and getter
    ActorDocument* doc;
    // user can set aliases for schema parameters and use them in cmdline or in other schemas through includes
    // ( paramName, alias ) pairs
    QMap<QString, QString> paramAliases;
    // (alias, alias help message) pairs
    QMap<QString, QString> aliasHelpDescs;
    // actor can be written on script
    // this object identifies it's text and variables
    AttributeScript * script;
    // an actor could be a subactor of some another
    ActorId owner;

private:
    void updateGrouperSlots(const QMap<ActorId, ActorId> &actorsMapping);
}; // Actor

/**
 * base class for container of rich document of actor's description
 */
class U2LANG_EXPORT ActorDocument : public QTextDocument {
public:
    ActorDocument(Actor* a);
    virtual ~ActorDocument() {}
    
    // actor's description is changed when attribute value changes
    // e.g. "read sequence from '1.fa'"
    // see realization in PrompterBaseImpl
    virtual void update(const QVariantMap& ) = 0;
    
protected:
    // document's target
    Actor* target;
    
}; // ActorDocument

/**
 * base class for factory of Actor's documents
 */
class U2LANG_EXPORT Prompter {
public:
    virtual ActorDocument* createDescription(Actor*) = 0;
    virtual ~Prompter() {};
    
}; // Prompter

/**
 * base class for Actor's configuration editor
 */
class U2LANG_EXPORT ActorConfigurationEditor : public ConfigurationEditor {
    Q_OBJECT
public:
    ActorConfigurationEditor() : ConfigurationEditor(), cfg(NULL) {}
    ActorConfigurationEditor(const ActorConfigurationEditor &other);
    virtual void setConfiguration(Actor *actor) {cfg = actor;}
    virtual ConfigurationEditor *clone() {return new ActorConfigurationEditor(*this);}

protected:
    Actor *cfg;
}; // ActorConfigurationEditor

}//Workflow namespace

}//GB2 namespace

#endif // _U2_WORKFLOW_MODEL_H_
