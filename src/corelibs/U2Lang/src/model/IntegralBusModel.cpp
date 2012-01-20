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

#include <QtCore/QDebug>

#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>

#include "IntegralBusModel.h"
#include "IntegralBusType.h"

/* TRANSLATOR U2::Workflow::IntegralBusPort */
namespace U2 {
namespace Workflow {

/*******************************
 * IntegralBusPort
 *******************************/
static void filterAmbiguousSlots(QList<Descriptor>& keys, const QMap<Descriptor, DataTypePtr>& map, QStrStrMap& result) {
    foreach(DataTypePtr val, map) {
        const QList<Descriptor> lst = map.keys(val);
        if (lst.size() != 1) {
            foreach(Descriptor d, lst) {
                result.insert(d.getId(), "");
                keys.removeOne(d);
            }
        }
    }
}

static Actor* getLinkedActor(ActorId id, Port* output) {
    if (output->owner()->getId() == id) {
        return output->owner();
    }
    foreach(Port* transit, output->owner()->getInputPorts()) {
        foreach(Port* p, transit->getLinks().uniqueKeys()) {
            Actor* a = getLinkedActor(id,p);
            if (a) return a;
        }
    }
    return NULL;
}

static QMap<QString, QStringList> getListMappings(const QStrStrMap& bm, const Port* p) {
    assert(p->isInput());    
    DataTypePtr dt = p->getType();
    QMap<QString, QStringList> res;
    if (dt->isList()) {
        QString val = bm.value(p->getId());
        if (!val.isEmpty()) {
            res.insert(p->getId(), val.split(";"));
        }
    } else if (dt->isMap()) {
        foreach(Descriptor d, dt->getAllDescriptors()) {
            QString val = bm.value(d.getId());
            if (dt->getDatatypeByDescriptor(d)->isList() && !val.isEmpty()) {
                res.insert(d.getId(), val.split(";"));
            }
        }
    }
    return res;
}

const QString IntegralBusPort::BUS_MAP_ATTR_ID = "bus-map";

IntegralBusPort::IntegralBusPort(const PortDescriptor& d, Actor* p) : Port(d,p), recursing(false) {
    addParameter(BUS_MAP_ATTR_ID, new Attribute(Descriptor(BUS_MAP_ATTR_ID), DataTypePtr()));
}

DataTypePtr IntegralBusPort::getType() const {
    return isInput() ? type : getBusType();
}

DataTypePtr IntegralBusPort::getBusType() const {
    if (recursing) {
        return DataTypePtr(new IntegralBusType(Descriptor(), QMap<Descriptor, DataTypePtr>()));
    }
    recursing = true;
    IntegralBusType* t = new IntegralBusType(Descriptor(*this), QMap<Descriptor, DataTypePtr>());
    foreach (Port* p, owner()->getInputPorts()) {
        if ((p->getFlags()&BLIND_INPUT) == 0){
            t->addInputs(p);
        }
    }
    t->addOutput(type, this);
    recursing = false;
    return DataTypePtr(t);
}

Actor* IntegralBusPort::getProducer(const QString& slot) {
    QList<Actor*> l = getProducers(slot);
    if (l.size() == 1) {
        return l.first();
    } else {
        return NULL;
    }
}

QList<Actor*> IntegralBusPort::getProducers(const QString& slot) {
    QList<Actor*> res;
    Attribute* at = getParameter(BUS_MAP_ATTR_ID);
    if(at == NULL) {
        return res;
    }
    QStrStrMap busMap = at->getAttributeValueWithoutScript<QStrStrMap>();
    QString slotValue = busMap.value(slot);
    QStringList vals = slotValue.split(";");
    foreach(QString val, vals) {
        ActorId id = IntegralBusType::parseSlotDesc(val);
        Actor * a = getLinkedActorById(id);
        if(a != NULL) {
            res << a;
        }
    }
    return res;
}

Actor* IntegralBusPort::getLinkedActorById(ActorId id) const {
    QList<Actor*> res;
    foreach(Port* peer, getLinks().uniqueKeys()) {
        Actor* ac = getLinkedActor(id,peer);
        if(ac != NULL) {
            res << ac;
        }
    }

    Actor * ret = NULL;
    if( res.size() == 0 ) {
        ret = NULL;
    } else if( res.size() > 1 ) {
        ret = res.first();
        //assert(false);
    } else {
        ret = res.first();
    }
    return ret;
}

void IntegralBusPort::remap(const QMap<ActorId, ActorId>& m) {
    Attribute* a = getParameter(BUS_MAP_ATTR_ID);
    if (a) {
        QStrStrMap busMap = a->getAttributeValueWithoutScript<QStrStrMap>();
        IntegralBusType::remap(busMap, m);
        setParameter(BUS_MAP_ATTR_ID, qVariantFromValue<QStrStrMap>(busMap));
    }
}

void IntegralBusPort::setBusMapValue(const QString & slotId, const QString & value) {
    if( !isInput() ) {
        return;
    }
    QStrStrMap busMap = getParameter(IntegralBusPort::BUS_MAP_ATTR_ID)->getAttributeValueWithoutScript<QStrStrMap>();
    if(busMap[slotId].isEmpty()) {
        busMap[slotId] = value;
    } else {
        busMap[slotId] = busMap[slotId] + ";" + value;
    }
    setParameter(BUS_MAP_ATTR_ID, qVariantFromValue<QStrStrMap>(busMap));
}

void IntegralBusPort::setupBusMap() {
    if( !isInput() || getWidth() != 1 ) {
        return;
    }
    
    DataTypePtr to = getType();
    assert(to->isMap()); // all port types made as map datatypes
    
    DataTypePtr from = bindings.uniqueKeys().first()->getType();
    QList<Descriptor> keys = to->getAllDescriptors();
    QStrStrMap busMap = getParameter(IntegralBusPort::BUS_MAP_ATTR_ID)->getAttributeValueWithoutScript<QStrStrMap>();
    filterAmbiguousSlots(keys, to->getDatatypesMap(), busMap);
    foreach(const Descriptor & key, keys) {
        // FIXME: hack for not binding 'Location' slot
        if(key == BaseSlots::URL_SLOT()) {
            busMap.insert(key.getId(), "");
            continue;
        }
        
        DataTypePtr elementDatatype = to->getDatatypeByDescriptor(key);
        QStringList candidates = WorkflowUtils::findMatchingTypesAsStringList(from, elementDatatype);
        if (elementDatatype->isList()) {
            candidates += WorkflowUtils::findMatchingTypesAsStringList(from, elementDatatype->getDatatypeByDescriptor());
            QString res = candidates.join(";");
            if(candidates.size() != 1) {
                busMap.insert(key.getId(), "");
            } else {
                busMap.insert(key.getId(), res);
            }
        } else if (candidates.size() == 0) {
            //no unambiguous match, reset
            busMap.insert(key.getId(), "");
        } else {
            // for string type initial value - empty. Because string type is always path to file and not needed for binding
            if( elementDatatype == BaseTypes::STRING_TYPE() ) {
                // FIXME: should use special hints for descriptors?
                foreach(const QString & str, candidates) {
                    if(str.split(":").at(1) == BaseSlots::URL_SLOT().getId()) {
                        candidates.removeAll(str);
                    }
                }
                if(candidates.size() == 1) {
                    busMap.insert(key.getId(), candidates.first());
                } else {
                    busMap.insert(key.getId(), "");
                }
            } else {
                bool fl = false;
                QMap<Port*,Link*> links = this->getLinks();
                Port *port = links.keys().first();
                DataTypePtr ptr = port->getOutputType();
                if(ptr->isMap()) {
                    foreach(const Descriptor & desc, ptr->getAllDescriptors()) {
                        if(key.getId() == desc.getId()) {
                            foreach(const QString &str, candidates) {
                                if(str.split(":").first() == port->owner()->getId()) {
                                    busMap.insert(key.getId(), str);
                                    fl = true;
                                }
                            }
                        }
                    }
                }
                else {
                    if(key.getId() == ptr->getId()) {
                        foreach(const QString &str, candidates) {
                            if(str.split(":").first() == port->owner()->getId()) {
                                busMap.insert(key.getId(), str);
                                fl = true;
                            }
                        }
                    }
                }
                if(!fl) {
                    busMap.insert(key.getId(), candidates.first());
                }
            }
        }
    }
    
    setParameter(BUS_MAP_ATTR_ID, qVariantFromValue<QStrStrMap>(busMap));
}

bool IntegralBusPort::validate(QStringList& l) const {
    bool good = Configuration::validate(l);
    if (isInput() && !validator) {
        good &= ScreenedSlotValidator::validate(QStringList(), this, l);
    }
    return good;
}

/*******************************
* ScreenedSlotValidator
*******************************/
bool ScreenedSlotValidator::validate( const QStringList& screenedSlots, const IntegralBusPort* vport, QStringList& l) 
{
    bool good = true;
    {
        if (vport->getWidth() == 0) {
            l.append(IntegralBusPort::tr("No input data supplied"));
            return false;
        }
        QStrStrMap bm = vport->getParameter(IntegralBusPort::BUS_MAP_ATTR_ID)->getAttributeValueWithoutScript<QStrStrMap>();
        int busWidth = bm.size();
        QMap<QString, QStringList> listMap = getListMappings(bm, vport);
        // iterate over all producers and exclude valid mappings from bus bindings
        foreach(Port* p, vport->getLinks().uniqueKeys()) {
            assert(qobject_cast<IntegralBusPort*>(p));//TBD?
            DataTypePtr t = p->getType();
            assert(t->isMap());
            {
                foreach(Descriptor d, t->getAllDescriptors()) {
                    foreach(QString key, bm.keys(d.getId())) {
                        //log.debug("reducing bus from key="+ikey+" to="+rkey);
                        assert(!key.isEmpty());
                        bm.remove(key);
                    }
                    foreach(QString key, listMap.uniqueKeys()) {
                        QStringList& l = listMap[key];
                        l.removeAll(d.getId());
                        if (l.isEmpty()) {
                            listMap.remove(key);
                            bm.remove(key);
                        }
                    }
                }
            }
        }
        if (busWidth == bm.size()) {
            l.append(IntegralBusPort::tr("No input data supplied"));
            good = false;
        }
        {
            QMapIterator<QString,QString> it(bm);
            while (it.hasNext())
            {
                it.next();
                const QString& slot = it.key();
                QString slotName = vport->getType()->getDatatypeDescriptor(slot).getDisplayName();
                //assert(!slotName.isEmpty());
                if (it.value().isEmpty()) {
                    if (!screenedSlots.contains(slot)) {
                        l.append(IntegralBusPort::tr("Warning, empty input slot: %1").arg(slotName));
                    }
                } else {
                    l.append(IntegralBusPort::tr("Bad slot binding: %1 to %2").arg(slotName).arg(it.value()));
                    good = false;
                }
            }
        }
        {
            QMapIterator<QString,QStringList> it(listMap);
            while (it.hasNext())
            {
                it.next();
                const QString& slot = it.key();
                QString slotName = vport->getType()->getDatatypeDescriptor(slot).getDisplayName();
                assert(!slotName.isEmpty());
                assert(!it.value().isEmpty());
                l.append(IntegralBusPort::tr("Bad slot binding: %1 to %2").arg(slotName).arg(it.value().join(",")));
                good = false;
            }
        }
    }
    return good;
}

bool ScreenedSlotValidator::validate( const Configuration* cfg, QStringList& output ) const {
    return validate(screenedSlots, static_cast<const IntegralBusPort*>(cfg), output);
}


/*******************************
* ScreenedParamValidator
*******************************/
ScreenedParamValidator::ScreenedParamValidator(const QString& id, const QString& port, const QString& slot) 
: id(id), port(port), slot(slot) {}

bool ScreenedParamValidator::validate(const Configuration* cfg, QStringList& output) const {
    QString err = validate(cfg);
    if( !err.isEmpty() ) {
        output.append(err);
        return false;
    }
    return true;
}

QString ScreenedParamValidator::validate(const Configuration * cfg) const {
    Attribute* param = cfg->getParameter(id);
    QVariant val = param->getAttributePureValue();
    const Workflow::Actor* a = dynamic_cast<const Workflow::Actor*>(cfg);
    assert(a);
    
    Workflow::Port* p = a->getPort(port);
    assert(p->isInput());
    
    QVariant busMap = p->getParameter(Workflow::IntegralBusPort::BUS_MAP_ATTR_ID)->getAttributePureValue();
    QString slotVal = busMap.value<QStrStrMap>().value(slot);
    const bool noParam = ( val.isNull() || val.toString().isEmpty() ) && param->getAttributeScript().isEmpty();
    const bool noSlot = slotVal.isNull() || slotVal.isEmpty();
    
    if (noParam && noSlot) {
        QString slotName = p->getType()->getDatatypeDescriptor(slot).getDisplayName(); 
        assert(!slotName.isEmpty());
        return U2::WorkflowUtils::tr("Either parameter '%1' or input slot '%2' must be set")
            .arg(param->getDisplayName()).arg(slotName);//FIXME translator class
    }
    if (noParam == noSlot) {
        QString slotName = p->getType()->getDatatypeDescriptor(slot).getDisplayName();
        assert(!slotName.isEmpty());
        /*return U2::WorkflowUtils::tr("Warning, parameter '%1' overrides bus data slot '%2'")
            .arg(param->getDisplayName()).arg(slotName);//FIXME translator class*/
    }
    return QString();
}

}//Workflow namespace
}//GB2namespace
