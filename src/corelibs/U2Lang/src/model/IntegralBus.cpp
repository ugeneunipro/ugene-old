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

#include "IntegralBus.h"
#include "IntegralBusType.h"

#include <U2Core/Log.h>

#include <U2Lang/WorkflowUtils.h>

#include <limits.h>

namespace U2 {
namespace Workflow {

static QMap<QString, QStringList> getListMappings(const QStrStrMap& bm, const Port* p) {
    assert(p->isInput());    
    DataTypePtr dt = p->getType();
    QMap<QString, QStringList> res;
    if (dt->isList()) {
        if (bm.contains(p->getId())) {
            res.insert(p->getId(), bm.value(p->getId()).split(";"));
        }
    } else if (dt->isMap()) {
        foreach(Descriptor d, dt->getAllDescriptors()) {
            if (dt->getDatatypeByDescriptor(d)->isList() && bm.contains(d.getId())) {
                res.insert(d.getId(), bm.value(d.getId()).split(";"));
            }
        }
    }
    return res;
}


IntegralBus::IntegralBus(Port* p)
: busType(p->getType()), complement(NULL), portId(p->getId()), takenMsgs(0), workflowContext(NULL) {
    actorId = p->owner()->getId();
    QString name = p->owner()->getLabel() + "[" + p->owner()->getId()+"]";
    if (p->isInput()) {
        Attribute* a = p->getParameter(IntegralBusPort::BUS_MAP_ATTR_ID);
        if(a == NULL) {
            assert(false);
            return;
        }
        
        busMap = a->getAttributeValueWithoutScript<QStrStrMap>();
        if (busMap.isEmpty()) {
            ActorPrototype *proto = p->owner()->getProto();
            assert(proto->isAllowsEmptyPorts());
            Q_UNUSED(proto);
        }
        QMapIterator<QString, QString> it(busMap);
        while (it.hasNext()) {
            it.next();
            coreLog.trace(QString("%1 - input bus map key=%2 val=%3").arg(name).arg(it.key()).arg(it.value()));
        }
        listMap = getListMappings(busMap, p);

    } else { // p is output
        IntegralBusPort* bp = qobject_cast<IntegralBusPort*>(p);
        DataTypePtr t = bp ? bp->getOwnType() : p->getType();
        if (t->isMap()) {
            foreach(Descriptor d, t->getAllDescriptors()) {
                QString key = d.getId();
                QString val = IntegralBusType::assignSlotDesc(d, p).getId();
                busMap.insert(key, val);
            }
        } else {
            QString key = p->getId();
            QString val = IntegralBusType::assignSlotDesc(*p, p).getId();
            busMap.insert(key, val);
        }
        QMapIterator<QString, QString> it(busMap);
        while (it.hasNext()) {
            it.next();
            coreLog.trace(QString("%1 - output bus map key=%2 val=%3").arg(name).arg(it.key()).arg(it.value()));
        }
    }
}

bool IntegralBus::addCommunication(const QString& id, CommunicationChannel* ch) {
    outerChannels.insertMulti(id, ch); 
    return true;
}

CommunicationChannel * IntegralBus::getCommunication(const QString& id) {
    return outerChannels.value(id);
}

Message IntegralBus::get() {
    QVariantMap result;
    context.clear();
    foreach (CommunicationChannel* ch, outerChannels) {
        Message m = ch->get();
        assert(m.getData().type() == QVariant::Map);
        QVariantMap imap = m.getData().toMap();
        context.unite(imap);
        foreach(QString ikey, imap.uniqueKeys()) {
            QVariant ival = imap.value(ikey);
            foreach(QString rkey, busMap.keys(ikey)) {
                coreLog.trace("reducing bus from key="+ikey+" to="+rkey);
                result[rkey] = ival;
            }
            QMapIterator<QString,QStringList> lit(listMap);
            while (lit.hasNext())
            {
                lit.next();
                QString rkey = lit.key();
                assert(!lit.value().isEmpty());
                if (lit.value().contains(ikey)) {
                    QVariantList vl = result[rkey].toList();
                    if (m.getType()->getDatatypeByDescriptor(ikey)->isList()) {
                        vl += ival.toList();
                        coreLog.trace("reducing bus key="+ikey+" to list of "+rkey);
                    } else {
                        vl.append(ival);
                        coreLog.trace("reducing bus key="+ikey+" to list element of "+rkey);
                    }
                    result[rkey] = vl;
                }
            }
        }
    }
    //assert(busType->isMap() || result.size() == 1);
    if (!printSlots.isEmpty()) {
        foreach (const QString &key, result.keys()) {
            if (printSlots.contains(key)) {
                QString slotString = actorId + "." + portId + "." + key;
                WorkflowUtils::print(slotString, result.value(key), workflowContext);
            }
        }
    }
    QVariant data;
    if (busType->isMap()) {
        data.setValue(result);
    } else if (result.size() == 1) {
        data = result.values().at(0);
    }
    if (complement) {
        complement->setContext(context);
    }
    
    takenMsgs++;
    return Message(busType, data);
}

Message IntegralBus::look() const {
    QVariantMap result;
    foreach(CommunicationChannel* channel, outerChannels) {
        assert(channel != NULL);
        Message message = channel->look();
        assert(message.getData().type() == QVariant::Map);
        result.unite(message.getData().toMap());
    }
    return Message(busType, result);
}

Message IntegralBus::composeMessage(const Message& m) {
    QVariantMap data(getContext());
    if (m.getData().type() == QVariant::Map) {
        QMapIterator<QString, QVariant> it(m.getData().toMap());
        while (it.hasNext()) {
            it.next();
            QString key = busMap.value(it.key());
            coreLog.trace("putting key="+key+" remapped from="+it.key());
            data.insert(key, it.value());
        }
    } else {
        assert(busMap.size() == 1);
        data.insert(busMap.values().first(), m.getData());
    }
    return Message(busType, data);
}

void IntegralBus::put(const Message& m) {
    Message busMessage = composeMessage(m);
    foreach(CommunicationChannel* ch, outerChannels) {
        ch->put(busMessage);
    }
    if ( !printSlots.isEmpty() && (m.getData().type() == QVariant::Map) ) {
        QVariantMap map = m.getData().toMap();
        foreach (const QString &key, map.keys()) {
            if (printSlots.contains(key)) {
                QString slotString = actorId + "." + portId + "." + key;
                WorkflowUtils::print(slotString, map.value(key), workflowContext);
            }
        }
    }
}

void IntegralBus::putWithoutContext(const Message& m) {
    foreach(CommunicationChannel* ch, outerChannels) {
        ch->put(m);
    }
}

int IntegralBus::hasMessage() const {
    if (outerChannels.isEmpty()) {
        return 0;
    }
    int num = INT_MAX;
    foreach(CommunicationChannel* ch, outerChannels) {
        num = qMin(num, ch->hasMessage());
    }
    return num;
}

int IntegralBus::takenMessages() const {
    return takenMsgs;
}

int IntegralBus::hasRoom(const DataType*) const {
    if (outerChannels.isEmpty()) {
        return 0;
    }
    int num = INT_MAX;
    foreach(CommunicationChannel* ch, outerChannels) {
        num = qMin(num, ch->hasRoom());
    }
    return num;
}

bool IntegralBus::isEnded() const {
    foreach(CommunicationChannel* ch, outerChannels) {
        if (ch->isEnded()) {
#ifdef _DEBUG
            foreach(CommunicationChannel* dbg, outerChannels) {
                assert(dbg->isEnded());
            }
#endif
            return true;
        }
    }
    return false;
}

void IntegralBus::setEnded() {
    foreach(CommunicationChannel* ch, outerChannels) {
        ch->setEnded();
    }
}

void IntegralBus::setPrintSlots(bool in, const QList<QString> &printSlots) {
        this->printSlots = printSlots;
}

void IntegralBus::setContext(WorkflowContext *context) {
    workflowContext = context;
}

}//namespace Workflow
}//namespace U2
