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

#ifndef _GROUPER_SLOT_ATTRIBUTE_
#define _GROUPER_SLOT_ATTRIBUTE_

#include <U2Lang/Attribute.h>
#include <U2Lang/GrouperOutSlot.h>

namespace U2 {

class U2LANG_EXPORT GrouperSlotAttribute : public Attribute {
public:
    GrouperSlotAttribute(const Descriptor &d, const DataTypePtr type, bool required = false, const QVariant &defaultValue = QVariant());
    ~GrouperSlotAttribute();

    virtual Attribute *clone();
    virtual AttributeGroup getGroup();

    QList<GrouperOutSlot> &getOutSlots();
    const QList<GrouperOutSlot> &getOutSlots() const;
    void addOutSlot(const GrouperOutSlot &outSlot);

private:
    QList<GrouperOutSlot> outSlots;
};

} // U2

#endif // _GROUPER_SLOT_ATTRIBUTE_
