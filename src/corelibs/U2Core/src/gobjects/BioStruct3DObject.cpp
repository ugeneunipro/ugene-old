/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <U2Core/DNASequence.h>
#include <U2Core/DatatypeSerializeUtils.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/RawDataUdrSchema.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "BioStruct3DObject.h"

namespace U2 {

BioStruct3DObject * BioStruct3DObject::createInstance(const BioStruct3D &bioStruct3D, const QString &objectName, const U2DbiRef &dbiRef, U2OpStatus &os, const QVariantMap &hintsMap) {
    U2RawData object(dbiRef);
    object.url = objectName;
    object.serializer = BioStruct3DSerializer::ID;

    RawDataUdrSchema::createObject(dbiRef, object, os);
    CHECK_OP(os, NULL);

    const U2EntityRef entRef(dbiRef, object.id);
    const QByteArray data = BioStruct3DSerializer::serialize(bioStruct3D);
    RawDataUdrSchema::writeContent(data, entRef, os);
    CHECK_OP(os, NULL);

    return new BioStruct3DObject(objectName, entRef, hintsMap);
}

BioStruct3DObject::BioStruct3DObject(const QString &objectName, const U2EntityRef &structRef, const QVariantMap &hintsMap)
: GObject(GObjectTypes::BIOSTRUCTURE_3D, objectName, hintsMap)
{
    entityRef = structRef;
    retrieve();
}

BioStruct3DObject::BioStruct3DObject(const BioStruct3D &bioStruct, const QString &objectName, const U2EntityRef &structRef, const QVariantMap &hintsMap)
: GObject(GObjectTypes::BIOSTRUCTURE_3D, objectName, hintsMap), bioStruct3D(bioStruct3D)
{
    entityRef = structRef;
}

void BioStruct3DObject::retrieve() {
    U2OpStatus2Log os;
    const QString serializer = RawDataUdrSchema::getObject(entityRef, os).serializer;
    CHECK_OP(os, );
    SAFE_POINT(BioStruct3DSerializer::ID == serializer, "Unknown serializer id", );

    const QByteArray data = RawDataUdrSchema::readAllContent(entityRef, os);
    CHECK_OP(os, );

    bioStruct3D = BioStruct3DSerializer::deserialize(data, os);
}

GObject * BioStruct3DObject::clone(const U2DbiRef &dstRef, U2OpStatus &os) const {
    U2RawData dstObject = RawDataUdrSchema::cloneObject(entityRef, dstRef, os);
    CHECK_OP(os, NULL);

    U2EntityRef dstEntRef(dstRef, dstObject.id);
    BioStruct3DObject *dst = new BioStruct3DObject(getGObjectName(), dstEntRef, getGHintsMap());
    dst->setIndexInfo(getIndexInfo());
    return dst;
}

} // U2
