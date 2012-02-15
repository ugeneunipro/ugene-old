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

#include "IntegralBusType.h"
#include "IntegralBusModel.h"

#include <U2Core/Log.h>

namespace U2 {
namespace Workflow {

IntegralBusType::IntegralBusType(const Descriptor& d, const QMap<Descriptor, DataTypePtr>& m) : MapDataType(d, m) {
}

Descriptor IntegralBusType::assignSlotDesc(const Descriptor& d, const Port* p) {
    QString id = QString("%1:%2").arg(p->owner()->getId()).arg(d.getId());
    QString name = U2::Workflow::IntegralBusPort::tr("%1 (by %2)").arg(d.getDisplayName()).arg(p->owner()->getLabel());
    QString doc = d.getDocumentation();
    return Descriptor(id, name, doc);
}

ActorId IntegralBusType::parseSlotDesc(const QString& id) {
    QStringList lst = id.split(":");
    QString sid = lst.first();
    ActorId aid = str2aid(sid);
    return aid;
}

QString IntegralBusType::parseAttributeIdFromSlotDesc(const QString & str) {
    QStringList lst = str.split(":");
    if( lst.size() == 2 ) {
        return lst.at(1);
    } else {
        return QString();
    }
}

void IntegralBusType::remap(QStrStrMap& busMap, const QMap<ActorId, ActorId>& m) {
    foreach(QString key, busMap.uniqueKeys()) {
        QStringList newValList;
        foreach(QString val, busMap.value(key).split(";")) {
            QStringList lst = val.split(":");
            QString sid = lst.first();
            ActorId id = str2aid(sid);
            coreLog.trace("trying remap key="+key+" sid="+sid);
            if (m.contains(id)) {
                QString newSid = QString("%1").arg(m.value(id));
                lst.replace(0, newSid);
                QString newVal = lst.join(":");
                coreLog.trace("remapping old="+val+" to new="+newVal);        
                val = newVal;
            }
            newValList.append(val);
        }
        busMap.insert(key, newValList.join(";"));
    }
}

void IntegralBusType::addInputs(const Port* p) {
    if (p->isInput()) {
        foreach(Port* peer, p->getLinks().uniqueKeys()) {
            DataTypePtr pt = peer->getType();
            if (qobject_cast<IntegralBusPort*>(peer)) {
                assert(pt->isMap());
                map.unite(pt->getDatatypesMap());
            } else {
                addOutput(pt, peer);
            }
        }
    }
}

void IntegralBusType::addOutput(DataTypePtr t, const Port* producer) {

    if (t->isMap()) {
        foreach(Descriptor d, t->getAllDescriptors()) {
            map[assignSlotDesc(d, producer)] = t->getDatatypeByDescriptor(d);
        }
    } else {
        map[assignSlotDesc(*producer, producer)] = t;
    }

}

}//Workflow namespace
}//GB2namespace
