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

#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/IntegralBus.h>
#include <U2Lang/IntegralBusModel.h>

#include "IntegralBusUtils.h"

namespace U2 {
namespace Workflow {

IntegralBusUtils::SplitResult IntegralBusUtils::splitCandidates(const QList<Descriptor> &candidates, DataTypePtr toElementDatatype) {
    SplitResult r;
    foreach (const Descriptor &c, candidates) {
        if (BaseTypes::STRING_TYPE() == toElementDatatype) {
            U2OpStatus2Log os;
            IntegralBusSlot slot = IntegralBusSlot::fromString(c.getId(), os);
            if (BaseSlots::URL_SLOT().getId() == slot.getId()
                || BaseSlots::DATASET_SLOT().getId() == slot.getId()) {
                r.otherDescs << c;
                continue;
            }
        }
        r.mainDescs << c;
    }

    return r;
}

void IntegralBusUtils::remapBus(QStrStrMap &busMap, const ActorId &oldId, const ActorId &newId, const PortMapping &mapping) {
    foreach (QString key, busMap.uniqueKeys()) {
        U2OpStatus2Log os;
        QList<IntegralBusSlot> slotList = IntegralBusSlot::listFromString(busMap[key], os);
        QList<IntegralBusSlot> newSlots;
        foreach (const IntegralBusSlot &slot, slotList) {
            IntegralBusSlot newSlot = slot;
            if (slot.actorId() == oldId) {
                U2OpStatusImpl os;
                QString newSlotId = mapping.getDstSlotId(slot.getId(), os);
                if (!os.hasError()) {
                    newSlot = IntegralBusSlot(newSlotId, "", newId);
                }
            }
            newSlots << newSlot;
        }
        busMap[key] = IntegralBusSlot::listToString(newSlots);
    }
}

void IntegralBusUtils::remapPathedSlotString(QString &pathedSlotStr, const ActorId &oldId, const ActorId &newId, const PortMapping &mapping) {
    if (pathedSlotStr.isEmpty()) {
        return;
    }
    QString slotStr;
    QStringList path;
    BusMap::parseSource(pathedSlotStr, slotStr, path);

    U2OpStatus2Log logOs;
    IntegralBusSlot slot = IntegralBusSlot::fromString(slotStr, logOs);
    if (slot.actorId() == oldId) {
        U2OpStatusImpl os;
        QString newSlot = mapping.getDstSlotId(slot.getId(), os);
        if (!os.hasError()) {
            slot = IntegralBusSlot(newSlot, "", newId);
        }
    }
    pathedSlotStr = slot.toString();

    if (!path.isEmpty()) {
        for (QStringList::iterator i=path.begin(); i!=path.end(); i++) {
            if (*i == oldId) {
                *i = newId;
            }
        }
        pathedSlotStr += ">" + path.join(",");
    }
}

} // Workflow
} // U2
