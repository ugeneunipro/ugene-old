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

#include "WorkflowTransport.h"

namespace U2 {

namespace Workflow {

Message::Message(DataTypePtr _t, const QVariant& d) : id(nextid()), t(_t), data(d) {
}

int Message::nextid() {
    static QAtomicInt id(0);
    return id.fetchAndAddRelaxed(1); //memory model??
}

int Message::getId() const {
    return id;
}

DataTypePtr Message::getType() const {
    return t;
}

QVariant Message::getData() const {
    return data;
}

bool Message::isEmpty() const {
    if (t->isMap()) {
        return data.toMap().isEmpty();
    } else {
        return data.isNull();
    }
}

Message Message::getEmptyMapMessage() {
    static const QVariantMap emptyData;
    static const QMap<Descriptor, DataTypePtr> emptyTypeMap;
    static DataTypePtr emptyType(new MapDataType(Descriptor(), emptyTypeMap));
    
    return Message(emptyType, emptyData);
}

} //Workflow namespace

} //GB2 namespace
