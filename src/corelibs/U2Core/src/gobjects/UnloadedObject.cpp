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

#include "UnloadedObject.h"

#include <U2Core/GHints.h>

namespace U2 {

UnloadedObject::UnloadedObject(const QString& objectName, const GObjectType& lot, const QVariantMap& hintsMap)
: GObject(GObjectTypes::UNLOADED, objectName, hintsMap)
{
    setLoadedObjectType(lot);
}

UnloadedObject::UnloadedObject(const UnloadedObjectInfo& info) 
: GObject(GObjectTypes::UNLOADED, info.name, info.hints) 
{
    setLoadedObjectType(info.type);
}

GObject* UnloadedObject::clone() const {
    UnloadedObject* cln = new UnloadedObject(getGObjectName(), getLoadedObjectType(), getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    return cln;
}

void UnloadedObject::setLoadedObjectType(const GObjectType& lot) {
    assert(lot!=GObjectTypes::UNLOADED);
    loadedObjectType = lot;
}

UnloadedObjectInfo::UnloadedObjectInfo(GObject* obj) {
    if (obj == NULL) {
        return;
    }
    name = obj->getGObjectName();
    hints = obj->getGHintsMap();
    if (obj->isUnloaded()) {
        UnloadedObject* uo = qobject_cast<UnloadedObject*>(obj);
        type = uo->getLoadedObjectType();
    } else {
        type = obj->getGObjectType();
    }
}

}//namespace
