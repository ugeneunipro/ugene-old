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

#include <U2Core/U2AttributeUtils.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2OpStatusUtils.h>

namespace U2 {


void U2AttributeUtils::init(U2Attribute &attr, U2Object & obj, const QString& name)
{
    attr.objectId = obj.id;
    attr.version = obj.version;
    attr.name = name;
}

void U2AttributeUtils::removeAttribute(U2AttributeDbi* adbi, const U2DataId& attrId, U2OpStatus& os) {
    QList<U2DataId> ids;
    ids << attrId;
    adbi->removeAttributes(ids, os);
}

// Finders: generic functionality

template <class AttrType>
static AttrType getAttribute(U2AttributeDbi* adbi, const U2DataId& attrId, U2OpStatus& os);


template <class AttrType>
static AttrType findGenericAttribute(U2AttributeDbi* adbi, const U2DataId& objectId, const QString& attrName, U2DataType type, U2OpStatus& os) {
    QList<U2DataId> attributeIds = adbi->getObjectAttributes(objectId, attrName, os);
    if (attributeIds.isEmpty() || os.hasError()) {
        return AttrType();
    }
    U2Dbi* dbi = adbi->getRootDbi();
    foreach(const U2DataId& id, attributeIds) {
        if (dbi->getEntityTypeById(id) == type) {
            return getAttribute<AttrType>(adbi, id, os);
        }
    }
    return AttrType();
}

// Implementations

U2IntegerAttribute U2AttributeUtils::findIntegerAttribute(U2AttributeDbi* adbi, const U2DataId& objectId, const QString& attrName, U2OpStatus& os) {
    return findGenericAttribute<U2IntegerAttribute>(adbi, objectId, attrName, U2Type::AttributeInteger, os);
}


U2RealAttribute U2AttributeUtils::findRealAttribute(U2AttributeDbi* adbi, const U2DataId& objectId, const QString& attrName, U2OpStatus& os) {
    return findGenericAttribute<U2RealAttribute>(adbi, objectId, attrName, U2Type::AttributeReal, os);
}


U2ByteArrayAttribute U2AttributeUtils::findByteArrayAttribute(U2AttributeDbi* adbi, const U2DataId& objectId, const QString& attrName, U2OpStatus& os) {
    return findGenericAttribute<U2ByteArrayAttribute>(adbi, objectId, attrName, U2Type::AttributeByteArray, os);
}


U2StringAttribute U2AttributeUtils::findStringAttribute(U2AttributeDbi* adbi, const U2DataId& objectId, const QString& attrName, U2OpStatus& os) {
    return findGenericAttribute<U2StringAttribute>(adbi, objectId, attrName, U2Type::AttributeString, os);
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
