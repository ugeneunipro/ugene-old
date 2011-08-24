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
#include <U2Core/U2OpStatusUtils.h>

namespace U2 {

// Generic functionality

template <class AttrType>
static AttrType getAttribute(U2AttributeDbi* adbi, const U2DataId& attrId, U2OpStatus& os);

template <class AttrType, class ValueType>
static ValueType findGenericAttribute(U2AttributeDbi* adbi, const U2Object& obj, const QString& attrName, U2DataType type, const ValueType& defaultValue, U2OpStatus& os) {
    QList<U2DataId> attributeIds = adbi->getObjectAttributes(obj.id, attrName, os);
    if (attributeIds.isEmpty() || os.hasError()) {
        return defaultValue;
    }
    U2Dbi* dbi = adbi->getRootDbi();
    foreach(const U2DataId& id, attributeIds) {
        if (dbi->getEntityTypeById(id) == type) {
            AttrType attr = getAttribute<AttrType>(adbi, id, os);
            if(os.hasError()) {
                LOG_OP(os);
                continue;
            }
            if(attr.version == obj.version) {
                return attr.value;
            }
        }
    }
    return defaultValue;
}

// Implementations

qint64 U2AttributeUtils::findIntegerAttribute(U2AttributeDbi* adbi, const U2Object& obj, const QString& attrName, qint64 defaultVal, U2OpStatus& os) {
    return findGenericAttribute<U2IntegerAttribute, qint64>(adbi, obj, attrName, U2Type::AttributeInteger, defaultVal, os);
}


double U2AttributeUtils::findRealAttribute(U2AttributeDbi* adbi, const U2Object& obj, const QString& attrName, double defaultVal, U2OpStatus& os) {
    return findGenericAttribute<U2RealAttribute, double>(adbi, obj, attrName, U2Type::AttributeReal, defaultVal, os);
}


QByteArray U2AttributeUtils::findByteArrayAttribute(U2AttributeDbi* adbi, const U2Object& obj, const QString& attrName, const QByteArray& defaultVal, U2OpStatus& os) {
    return findGenericAttribute<U2ByteArrayAttribute, QByteArray>(adbi, obj, attrName, U2Type::AttributeByteArray, defaultVal, os);
}


QString U2AttributeUtils::findStringAttribute(U2AttributeDbi* adbi, const U2Object& obj, const QString& attrName, const QString& defaultVal, U2OpStatus& os) {
    return findGenericAttribute<U2StringAttribute, QString>(adbi, obj, attrName, U2Type::AttributeString, defaultVal, os);
}

// Getting attribute template specializations

template <>
U2IntegerAttribute getAttribute<U2IntegerAttribute>(U2AttributeDbi* adbi, const U2DataId& attrId, U2OpStatus &os) {
    return adbi->getIntegerAttribute(attrId, os);
}

template <>
U2RealAttribute getAttribute<U2RealAttribute>(U2AttributeDbi* adbi, const U2DataId& attrId, U2OpStatus &os) {
    return adbi->getRealAttribute(attrId, os);
}

template <>
U2ByteArrayAttribute getAttribute<U2ByteArrayAttribute>(U2AttributeDbi* adbi, const U2DataId& attrId, U2OpStatus &os) {
    return adbi->getByteArrayAttribute(attrId, os);
}

template <>
U2StringAttribute getAttribute<U2StringAttribute>(U2AttributeDbi* adbi, const U2DataId& attrId, U2OpStatus &os) {
    return adbi->getStringAttribute(attrId, os);
}


} //namespace
