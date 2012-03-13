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

#include "GrouperSlotAttribute.h"

namespace U2 {

GrouperSlotAttribute::GrouperSlotAttribute(const Descriptor &d, const DataTypePtr type, bool required, const QVariant &defaultValue)
: Attribute(d, type, required, defaultValue)
{

}

GrouperSlotAttribute::~GrouperSlotAttribute() {
}

Attribute *GrouperSlotAttribute::clone() {
    return new GrouperSlotAttribute(*this);
}

AttributeGroup GrouperSlotAttribute::getGroup() {
    return GROUPER_SLOT_GROUP;
}

QList<GrouperOutSlot> &GrouperSlotAttribute::getOutSlots() {
    return outSlots;
}

const QList<GrouperOutSlot> &GrouperSlotAttribute::getOutSlots() const {
    return outSlots;
}

void GrouperSlotAttribute::addOutSlot(const GrouperOutSlot &outSlot) {
    outSlots.append(outSlot);
}

} // U2
