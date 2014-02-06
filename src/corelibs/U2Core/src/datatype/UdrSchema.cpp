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

#include <QtCore/QSet>

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UdrSchemaRegistry.h>

#include "UdrSchema.h"

namespace U2 {

const QByteArray UdrSchema::RECORD_ID_FIELD_NAME("record_id");
const QByteArray UdrSchema::OBJECT_FIELD_NAME("object");
const int UdrSchema::OBJECT_FIELD_NUM = 0;

UdrSchema::FieldDesc::FieldDesc(const QByteArray &name, UdrSchema::DataType dataType, UdrSchema::IndexType indexType, U2DataType objectType)
: name(name), dataType(dataType), indexType(indexType), objectType(objectType)
{

}

const QByteArray UdrSchema::FieldDesc::getName() const {
    return name;
}

UdrSchema::DataType UdrSchema::FieldDesc::getDataType() const {
    return dataType;
}

UdrSchema::IndexType UdrSchema::FieldDesc::getIndexType() const {
    return indexType;
}

U2DataType UdrSchema::FieldDesc::getObjectType() const {
    SAFE_POINT(ID == dataType, "Not ID datatype", objectType);
    return objectType;
}

UdrSchema::UdrSchema(const UdrSchemaId &id)
: id(id), withObjectReference(false)
{

}

UdrSchema::UdrSchema(const UdrSchemaId &id, U2DataType objectType)
: id(id), withObjectReference(true)
{
    U2OpStatusImpl os;
    addField(FieldDesc(OBJECT_FIELD_NAME, ID, INDEXED, objectType), os);
    SAFE_POINT_OP(os, );
}

bool UdrSchema::contains(const QByteArray &name) const {
    CHECK(RECORD_ID_FIELD_NAME != name, true);

    foreach (const FieldDesc &field, fields) {
        if (name == field.getName()) {
            return true;
        }
    }
    return false;
}

void UdrSchema::addField(const FieldDesc &desc, U2OpStatus &os) {
    CHECK_EXT(!contains(desc.getName()), os.setError("Duplicate name"), );
    if (BLOB == desc.getDataType()) {
        CHECK_EXT(NOT_INDEXED == desc.getIndexType(), os.setError("BLOB data can not be indexed"), );
    }
    CHECK_EXT(UdrSchemaRegistry::isCorrectName(desc.getName()), os.setError("Incorrect field name"), );
    fields << desc;
}

void UdrSchema::addMultiIndex(const QList<int> &multiIndex, U2OpStatus &os) {
    CHECK_EXT(multiIndex.toSet().size() == multiIndex.size(), os.setError("Not unique fields"), );
    foreach (int fieldNum, multiIndex) {
        FieldDesc field = getField(fieldNum, os);
        CHECK_OP(os, );
        CHECK_EXT(BLOB != field.getDataType(), os.setError("BLOB data can not be indexed"), );
    }
    CHECK_EXT(!multiIndexes.contains(multiIndex), os.setError("Duplicate multi index"), );
    multiIndexes << multiIndex;
}

const QList< QList<int> > & UdrSchema::getMultiIndexes() const {
    return multiIndexes;
}

const UdrSchemaId & UdrSchema::getId() const {
    return id;
}

int UdrSchema::size() const {
    return fields.size();
}

UdrSchema::FieldDesc UdrSchema::getField(int fieldNum, U2OpStatus &os) const {
    CHECK_EXT(0 <= fieldNum && fieldNum < size(), os.setError("Out of range"), FieldDesc("", INTEGER));
    return fields[fieldNum];
}

bool UdrSchema::hasObjectReference() const {
    return withObjectReference;
}

} // U2
