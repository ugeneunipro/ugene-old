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

#include <U2Core/DatatypeSerializeUtils.h>
#include <U2Core/RawDataUdrSchema.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "GObjectTypes.h"

#include "DNAChromatogramObject.h"

namespace U2 {

DNAChromatogramObject * DNAChromatogramObject::createInstance(const DNAChromatogram &chroma, const QString &objectName, const U2DbiRef &dbiRef, U2OpStatus &os, const QVariantMap &hintsMap) {
    U2RawData object(dbiRef);
    object.url = objectName;
    object.serializer = DNAChromatogramSerializer::ID;

    RawDataUdrSchema::createObject(dbiRef, object, os);
    CHECK_OP(os, NULL);

    U2EntityRef entRef(dbiRef, object.id);
    QByteArray data = DNAChromatogramSerializer::serialize(chroma);
    RawDataUdrSchema::writeContent(data, entRef, os);
    CHECK_OP(os, NULL);

    return new DNAChromatogramObject(objectName, entRef, hintsMap);
}

DNAChromatogramObject::DNAChromatogramObject(const QString &objectName, const U2EntityRef &chromaRef, const QVariantMap &hintsMap)
: GObject(GObjectTypes::CHROMATOGRAM, objectName, hintsMap), cached(false)
{
    entityRef = chromaRef;
}

const DNAChromatogram & DNAChromatogramObject::getChromatogram() const {
    QMutexLocker lock(&mutex);
    if (!cached) {
        U2OpStatus2Log os;
        QString serializer = RawDataUdrSchema::getObject(entityRef, os).serializer;
        CHECK_OP(os, cache);
        SAFE_POINT(DNAChromatogramSerializer::ID == serializer, "Unknown serializer id", cache);
        QByteArray data = RawDataUdrSchema::readAllContent(entityRef, os);
        CHECK_OP(os, cache);
        cache = DNAChromatogramSerializer::deserialize(data, os);
        cached = true;
    }

    return cache;
}

GObject * DNAChromatogramObject::clone(const U2DbiRef &dstRef, U2OpStatus &os) const {
    U2RawData dstObject = RawDataUdrSchema::cloneObject(entityRef, dstRef, os);
    CHECK_OP(os, NULL);

    U2EntityRef dstEntRef(dstRef, dstObject.id);
    DNAChromatogramObject *dst = new DNAChromatogramObject(getGObjectName(), dstEntRef, getGHintsMap());
    dst->setIndexInfo(getIndexInfo());
    return dst;
}

}//namespace


