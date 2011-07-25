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

#include <U2Core/U2AttributeUtils.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2OpStatus.h>

namespace U2 {

    
qint64 U2AttributeUtils::findIntegerAttribute(U2AttributeDbi* adbi, const U2DataId& objectId, const QString& name, qint64 defaultVal, U2OpStatus& os) {
    QList<U2DataId> attributeIds = adbi->getObjectAttributes(objectId, name, os);
    if (attributeIds.isEmpty() || os.hasError()) {
        return defaultVal;
    }
    U2Dbi* dbi = adbi->getRootDbi();
    foreach(const U2DataId& id, attributeIds) {
        if (dbi->getEntityTypeById(id) == U2Type::AttributeInteger) {
            U2IntegerAttribute ia = adbi->getIntegerAttribute(id, os);
            return ia.value;
        }
    }
    return defaultVal;
}


double U2AttributeUtils::findRealAttribute(U2AttributeDbi* adbi, const U2DataId& objectId, const QString& name, double defaultVal, U2OpStatus& os) {
    QList<U2DataId> attributeIds = adbi->getObjectAttributes(objectId, name, os);
    if (attributeIds.isEmpty() || os.hasError()) {
        return defaultVal;
    }
    U2Dbi* dbi = adbi->getRootDbi();
    foreach(const U2DataId& id, attributeIds) {
        if (dbi->getEntityTypeById(id) == U2Type::AttributeReal) {
            U2RealAttribute ra = adbi->getRealAttribute(id, os);
            return ra.value;
        }
    }
    return defaultVal;
}


QByteArray U2AttributeUtils::findByteArrayAttribute(U2AttributeDbi* adbi, const U2DataId& objectId, const QString& name, const QByteArray& defaultVal, U2OpStatus& os) {
    QList<U2DataId> attributeIds = adbi->getObjectAttributes(objectId, name, os);
    if (attributeIds.isEmpty() || os.hasError()) {
        return defaultVal;
    }
    U2Dbi* dbi = adbi->getRootDbi();
    foreach(const U2DataId& id, attributeIds) {
        if (dbi->getEntityTypeById(id) == U2Type::AttributeByteArray) {
            U2ByteArrayAttribute aa = adbi->getByteArrayAttribute(id, os);
            return aa.value;
        }
    }
    return defaultVal;
}


QString U2AttributeUtils::findStringAttribute(U2AttributeDbi* adbi, const U2DataId& objectId, const QString& name, const QString& defaultVal, U2OpStatus& os) {
    QList<U2DataId> attributeIds = adbi->getObjectAttributes(objectId, name, os);
    if (attributeIds.isEmpty() || os.hasError()) {
        return defaultVal;
    }
    U2Dbi* dbi = adbi->getRootDbi();
    foreach(const U2DataId& id, attributeIds) {
        if (dbi->getEntityTypeById(id) == U2Type::AttributeInteger) {
            U2StringAttribute sa = adbi->getStringAttribute(id, os);
            return sa.value;
        }
    }
    return defaultVal;
}

} //namespace
