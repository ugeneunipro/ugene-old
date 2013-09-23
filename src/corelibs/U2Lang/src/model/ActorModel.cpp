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

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Lang/ConfigurationEditor.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/GrouperSlotAttribute.h>
#include <U2Lang/IntegralBusType.h>
#include <U2Lang/WorkflowUtils.h>

#include "support/IntegralBusUtils.h"

#include "ActorModel.h"

namespace U2 {
namespace Workflow {

/**************************
 * Actor
 **************************/
Actor::Actor(const ActorId &actorId, ActorPrototype* proto, AttributeScript * _script)
: id(actorId), proto(proto), doc(NULL), script(_script), condition(new AttributeScript()) {
    if(script == NULL) {
        if(proto->isScriptFlagSet()) {
            script = new AttributeScript();
            script->setScriptText("");   
        }
        else {
            script = NULL;
        }
    }

    if(script != NULL) {
        setupVariablesForPort(script);
        setupVariablesForAttribute(script);
    }
    setupVariablesForPort(condition, true);
    setupVariablesForAttribute(condition);
}

Actor::Actor(const Actor&) : QObject(), Configuration(), Peer() {
    assert(false);
}

Actor::~Actor() {
    qDeleteAll(ports.values()); 
    delete doc;
    delete script;
    delete condition;
}

void Actor::setupVariablesForPort(AttributeScript *_script, bool inputOnly) {
    foreach(const PortDescriptor *descr, proto->getPortDesciptors()) {
        QString prefix;
        if(descr->isInput()) {
            prefix = "in_";
        }
        else if (!inputOnly) {
            prefix = "out_";
        } else {
            continue;
        }

        DataTypePtr dataTypePtr = descr->getType();
        if(dataTypePtr->isMap()) {
            QMap<Descriptor, DataTypePtr> map = dataTypePtr->getDatatypesMap();
            foreach(const Descriptor & d, map.keys()) {
                Descriptor var(prefix + d.getId(), d.getDisplayName(), d.getDocumentation());
                _script->setScriptVar(var, QVariant());
            }
        }
        else  if(dataTypePtr->isList()) {
            foreach(const Descriptor & typeDescr, dataTypePtr->getAllDescriptors()) {
                Descriptor var(prefix + typeDescr.getId(), typeDescr.getDisplayName(), typeDescr.getDocumentation());
                _script->setScriptVar(var, QVariant());
            }
        }
        else {
            QString id = prefix + dataTypePtr->getId();
            QString displayName = dataTypePtr->getDisplayName();
            QString doc = prefix + dataTypePtr->getDocumentation();
            _script->setScriptVar(Descriptor(id,displayName,doc), QVariant());
        }
    }
}

void Actor::setupVariablesForAttribute(AttributeScript *_script) {
    foreach( Attribute * attribute, proto->getAttributes() ) {
        assert(attribute != NULL);
        QString attrVarName = attribute->getDisplayName();
        _script->setScriptVar(Descriptor(attribute->getId().replace(".","_"), attrVarName.replace(".", "_"), attribute->getDocumentation()), QVariant());
    }
}

AttributeScript * Actor::getScript() const {
    return script;
} 

void Actor::setScript(AttributeScript* _script) {
    script->setScriptText(_script->getScriptText());
}

AttributeScript *Actor::getCondition() const {
    return condition;
}

ActorId Actor::getOwner() const {
    return owner;
}

void Actor::setOwner(const ActorId &owner) {
    this->owner = owner;
}

void Actor::updateActorIds(const QMap<ActorId, ActorId> &actorIdsMap) {
    if (actorIdsMap.contains(owner)) {
        owner = actorIdsMap[owner];
    }

    foreach (Attribute *a, this->getAttributes()) {
        a->updateActorIds(actorIdsMap);
    }
}

ActorPrototype * Actor::getProto() const {
    return proto;
}

ActorId Actor::getId() const {
    return id;
}

void Actor::setId(const ActorId &id) {
    this->id = id;
}

QString Actor::getLabel() const {
    if( label.isEmpty() ) {
        return QString("%1 %2").arg(getProto()->getDisplayName()).arg(getId());
    } else {
        return label;
    }
}

void Actor::setLabel(const QString& l) {
    label = l; 
    emit si_labelChanged();
}

Port * Actor::getPort(const QString& id) const {
    return ports.value(id);
}

QList<Port*> Actor::getPorts() const {
    return ports.values();
}

QList<Port*> Actor::getInputPorts() const {
    QList<Port*> l; foreach (Port* p, ports.values()) if (p->isInput()) l<<p; 
    return l;
}

QList<Port*> Actor::getOutputPorts() const {
    QList<Port*> l; foreach (Port* p, ports.values()) if (p->isOutput()) l<<p; 
    return l;
}

void Actor::setParameter(const QString& name, const QVariant& val) {
    Configuration::setParameter(name, val);
    emit si_modified();
}

ActorDocument * Actor::getDescription() const {
    return doc;
}

void Actor::setDescription(ActorDocument* d) {
    assert(d != NULL);
    doc = d;
}

const QMap<QString, QString> & Actor::getParamAliases() const {
    return paramAliases;
}

QMap<QString, QString> & Actor::getParamAliases() {
    return paramAliases;
}

bool Actor::hasParamAliases() const {
    return !paramAliases.isEmpty();
}

const QMap<QString, QString> & Actor::getAliasHelp() const {
    return aliasHelpDescs;
}

QMap<QString, QString> & Actor::getAliasHelp() {
    return aliasHelpDescs;
}

bool Actor::hasAliasHelp() const {
    foreach(const QString & alias, paramAliases.values()) {
        if(aliasHelpDescs.contains(alias)) {
            return true;
        }
    }
    return false;
}

void Actor::remap(const QMap<ActorId, ActorId>& m) {
    foreach(Port* p, ports) {
        p->remap(m);
    }
}

void Actor::update(const QMap<ActorId, ActorId> &actorsMapping) {
    foreach (Port *p, getPorts()) {
        p->updateBindings(actorsMapping);
    }
    if (CoreLibConstants::GROUPER_ID == proto->getId()) {
        updateGrouperSlots(actorsMapping);
    }
}

void Actor::updateGrouperSlots(const QMap<ActorId, ActorId> &actorsMapping) {
    SAFE_POINT(1 == getOutputPorts().size(), "Grouper port error 1", );
    SAFE_POINT(1 == getInputPorts().size(), "Grouper port error 2", );
    Port *outPort = getOutputPorts().first();
    SAFE_POINT(outPort->getOutputType()->isMap(), "Grouper port error 3", );
    QMap<Descriptor, DataTypePtr> outBusMap = outPort->getOutputType()->getDatatypesMap();
    QMap<Descriptor, DataTypePtr> inBusMap;
    {
        Port *inPort = getInputPorts().first();
        inBusMap = WorkflowUtils::getBusType(inPort);
    }

    // update in slot attribute
    {
        Attribute *attr = getParameter(CoreLibConstants::GROUPER_SLOT_ATTR);
        QString groupSlot = attr->getAttributeValueWithoutScript<QString>();
        if (!groupSlot.isEmpty()) {
            groupSlot = GrouperOutSlot::readable2busMap(groupSlot);
            U2OpStatus2Log logOs;
            IntegralBusSlot slot = IntegralBusSlot::fromString(groupSlot, logOs);
            foreach (const ActorId &oldId, actorsMapping.keys()) {
                slot.replaceActorId(oldId, actorsMapping[oldId]);
            }
            groupSlot = slot.toString();
            bool found = false;
            foreach (const Descriptor &d, inBusMap.keys()) {
                if (d.getId() == groupSlot) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                groupSlot = "";
            }
            attr->setAttributeValue(GrouperOutSlot::busMap2readable(groupSlot));
        }
    }
    // update out slots
    {
        GrouperOutSlotAttribute *attr = dynamic_cast<GrouperOutSlotAttribute*>(getParameter(CoreLibConstants::GROUPER_OUT_SLOTS_ATTR));
        QList<GrouperOutSlot> &outSlots = attr->getOutSlots();
        QList<GrouperOutSlot>::iterator i = outSlots.begin();
        while (i != outSlots.end()) {
            QString in = i->getBusMapInSlotId();
            U2OpStatus2Log logOs;
            IntegralBusSlot slot = IntegralBusSlot::fromString(in, logOs);
            foreach (const ActorId &oldId, actorsMapping.keys()) {
                slot.replaceActorId(oldId, actorsMapping[oldId]);
            }
            in = slot.toString();
            i->setBusMapInSlotStr(in);
            bool found = false;
            foreach (const Descriptor &d, inBusMap.keys()) {
                if (d.getId() == in) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                outBusMap.remove(i->getOutSlotId());
                i = outSlots.erase(i);
            } else {
                ++i;
            }
        }
    }

    DataTypePtr newType(new MapDataType(dynamic_cast<Descriptor&>(*(outPort->getType())), outBusMap));
    outPort->setNewType(newType);
}

void Actor::replaceActor(Actor *oldActor, Actor *newActor, const QList<PortMapping> &mappings) {
    foreach (Port *p, getPorts()) {
        p->replaceActor(oldActor, newActor, mappings);
    }
    if (CoreLibConstants::GROUPER_ID == proto->getId()) {
        {
            Attribute *attr = getParameter(CoreLibConstants::GROUPER_SLOT_ATTR);
            QString groupSlot = attr->getAttributeValueWithoutScript<QString>();
            groupSlot = GrouperOutSlot::readable2busMap(groupSlot);
            foreach (const PortMapping &pm, mappings) {
                IntegralBusUtils::remapPathedSlotString(groupSlot, oldActor->getId(), newActor->getId(), pm);
            }
            attr->setAttributeValue(GrouperOutSlot::busMap2readable(groupSlot));
        }

        {
            GrouperOutSlotAttribute *attr = dynamic_cast<GrouperOutSlotAttribute*>(getParameter(CoreLibConstants::GROUPER_OUT_SLOTS_ATTR));
            QList<GrouperOutSlot>::iterator i = attr->getOutSlots().begin();
            for (; i!=attr->getOutSlots().end(); i++) {
                QString in = i->getBusMapInSlotId();
                foreach (const PortMapping &pm, mappings) {
                    IntegralBusUtils::remapPathedSlotString(in, oldActor->getId(), newActor->getId(), pm);
                }
                i->setBusMapInSlotStr(in);
            }
        }
    }
}

void Actor::updateDelegateTags() {
    CHECK(NULL != editor, );
    foreach (Attribute *influencing, getAttributes()) {
        foreach (const AttributeRelation *rel, influencing->getRelations()) {
            PropertyDelegate *dependentDelegate = editor->getDelegate(rel->getRelatedAttrId());
            if (NULL == dependentDelegate) {
                continue;
            }
            rel->updateDelegateTags(influencing->getAttributePureValue(), dependentDelegate->tags());
        }
    }
}

/**************************
 * ActorPrototype
 **************************/
QList<PortDescriptor*> ActorPrototype::getPortDesciptors() const {
    return ports;
}

QList<Attribute*> ActorPrototype::getAttributes() const {
    return attrs;
}

void ActorPrototype::addAttribute( Attribute * a ) {
    assert(a != NULL);
    attrs << a;
}

int ActorPrototype::removeAttribute( Attribute * attr ) {
    assert(attr != NULL);
    return attrs.removeAll( attr );
}

void ActorPrototype::setEditor(ConfigurationEditor* e) {
    assert(e != NULL);
    ed = e;
}

ConfigurationEditor * ActorPrototype::getEditor()const {
    return ed;
}

void ActorPrototype::setValidator(ConfigurationValidator* v) {
    assert(v != NULL);
    val = v;
}

void ActorPrototype::setPrompter(Prompter* p) {
    assert(p != NULL);
    prompter = p;
}

void ActorPrototype::setPortValidator(const QString& id, ConfigurationValidator* v) {
    assert(v != NULL);
    portValidators[id] = v;
}

bool ActorPrototype::isAcceptableDrop(const QMimeData*, QVariantMap* ) const {
    return false;
}

Port* ActorPrototype::createPort(const PortDescriptor& d, Actor* p) {
    return new Port(d, p);
}

Actor* ActorPrototype::createInstance(const ActorId &actorId, AttributeScript *script, const QVariantMap& params) {
    Actor* proc = new Actor(actorId, this, script);
    if (ed) {
        ed->updateDelegates();
    }

    foreach(PortDescriptor* pd, getPortDesciptors()) {
        Port* p = createPort(*pd, proc);
        QString pid = pd->getId();
        if (portValidators.contains(pid)) {
            p->setValidator(portValidators.value(pid));
        }
        proc->ports[pid] = p;
    }
    foreach(Attribute* a, getAttributes()) {
        proc->addParameter(a->getId(), a->clone());
    }
    if (val) {
        proc->setValidator(val);
    }
    if (prompter) {
        proc->setDescription(prompter->createDescription(proc));
    }

    QMapIterator<QString, QVariant> i(params);
    while (i.hasNext()) {
        i.next();
        proc->setParameter(i.key(), i.value());
    }
    if (ed) {
        ActorConfigurationEditor *actorEd = dynamic_cast<ActorConfigurationEditor*>(ed);
        if (NULL != actorEd) {
            ActorConfigurationEditor *editor = dynamic_cast<ActorConfigurationEditor*>(ed->clone());
            editor->setConfiguration(proc);
            proc->setEditor(editor);
        } else {
            proc->setEditor(ed->clone());
        }
    }
    return proc;
}


Attribute * ActorPrototype::getAttribute( const QString & id ) const {
    Attribute * res = NULL;
    foreach( Attribute * a, attrs ) {
        if( a->getId() == id ) {
            res = a;
            break;
        }
    }
    return res;
}

void ActorPrototype::setScriptFlag(bool flag) {
    isScript = flag;
}

void ActorPrototype::setNonStandard(const QString &path) {
    isStandard = false;
    actorFilePath = path;
}

void ActorPrototype::setSchema(const QString &path) {
    isStandard = false;
    isSchema = true;
    actorFilePath = path;
}

void ActorPrototype::addExternalTool(const QString &toolId, const QString &paramId) {
    externalTools[toolId] = paramId;
}

const QStrStrMap & ActorPrototype::getExternalTools() const {
    return externalTools;
}

ActorPrototype::ActorPrototype(const Descriptor& d, 
                               const QList<PortDescriptor*>& ports, 
                               const QList<Attribute*>& attrs)
: VisualDescriptor(d), attrs(attrs), ports(ports), ed(NULL), val(NULL), prompter(NULL),
isScript(false), isStandard(true), isSchema(false), allowsEmptyPorts(false), influenceOnPathFlag(false) {
}

ActorPrototype::~ActorPrototype()
{
    qDeleteAll(attrs);
    qDeleteAll(ports);
    delete ed;
    delete val;
    delete prompter;
    qDeleteAll(portValidators);
}

/**************************
 * ActorDocument
 **************************/
ActorDocument::ActorDocument(Actor* a) : QTextDocument(a), target(a) {
}

//void ActorDocument::update(const QVariantMap& ) {
//}

ActorConfigurationEditor::ActorConfigurationEditor(const ActorConfigurationEditor &other)
: ConfigurationEditor(other)
{
    cfg = other.cfg;
}

/************************************************************************/
/* ActorValidator */
/************************************************************************/
bool ActorValidator::validate(const Configuration *cfg, QStringList &errors) const {
    const Actor *actor = static_cast<const Actor*>(cfg);
    SAFE_POINT(NULL != actor, "NULL actor", false);
    return validate(actor, errors);
}

}//Workflow namespace
}//GB2namespace
