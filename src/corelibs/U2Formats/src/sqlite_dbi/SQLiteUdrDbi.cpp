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

#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UdrSchemaRegistry.h>

#include "SQLiteBlobInputStream.h"
#include "SQLiteBlobOutputStream.h"

#include "SQLiteUdrDbi.h"

namespace U2 {

static const QString TABLE_PREFIX = "UdrSchema_";

SQLiteUdrDbi::SQLiteUdrDbi(SQLiteDbi *dbi)
: UdrDbi(dbi), SQLiteChildDBICommon(dbi)
{

}

UdrRecordId SQLiteUdrDbi::addRecord(const UdrSchemaId &schemaId, const QList<UdrValue> &data, U2OpStatus &os) {
    UdrRecordId result("", "");
    const UdrSchema *schema = udrSchema(schemaId, os);
    CHECK_OP(os, result);
    CHECK_EXT(data.size() == schema->size(), os.setError("Size mismatch"), result);

    SQLiteQuery q(insertDef(schema, os), db, os);
    CHECK_OP(os, result);

    bindData(data, schema, q, os);
    CHECK_OP(os, result);

    U2DataId recordId = q.insert(U2Type::UdrRecord);
    return UdrRecordId(schemaId, recordId);
}

UdrRecord SQLiteUdrDbi::getRecord(const UdrRecordId &recordId, U2OpStatus &os) {
    UdrRecord result(recordId, QList<UdrValue>(), os);
    const UdrSchema *schema = udrSchema(recordId.schemaId, os);
    CHECK_OP(os, result);

    SQLiteQuery q(selectDef(schema, os), db, os);
    CHECK_OP(os, result);

    q.bindDataId(1, recordId.recordId);

    bool ok = q.step();
    CHECK_EXT(ok, os.setError("Unknown record id"), result);

    QList<UdrValue> data;
    retreiveData(data, schema, q, os);
    CHECK_OP(os, result);
    q.ensureDone();
    CHECK_OP(os, result);

    return UdrRecord(recordId, data, os);
}

void SQLiteUdrDbi::removeRecord(const UdrRecordId &recordId, U2OpStatus &os) {
    SQLiteQuery q("DELETE FROM " + tableName(recordId.schemaId) + " WHERE " + UdrSchema::RECORD_ID_FIELD_NAME + " = ?1", db, os);
    q.bindDataId(1, recordId.recordId);
    q.execute();
}

InputStream * SQLiteUdrDbi::createInputStream(const UdrRecordId &recordId, int fieldNum, U2OpStatus &os) {
    UdrSchema::FieldDesc field = getBlobField(recordId.schemaId, fieldNum, os);
    CHECK_OP(os, NULL);

    return new SQLiteBlobInputStream(db, tableName(recordId.schemaId).toAscii(), field.getName(), recordId.recordId, os);
}

OutputStream * SQLiteUdrDbi::createOutputStream(const UdrRecordId &recordId, int fieldNum, qint64 size, U2OpStatus &os) {
    CHECK_EXT(size >= 0, os.setError("Negative stream size"), NULL);
    CHECK_EXT(size <= INT_MAX, os.setError("Too big stream size"), NULL);
    UdrSchema::FieldDesc field = getBlobField(recordId.schemaId, fieldNum, os);
    CHECK_OP(os, NULL);

    return new SQLiteBlobOutputStream(db, tableName(recordId.schemaId).toAscii(), field.getName(), recordId.recordId, (int)size, os);
}

/************************************************************************/
/* SQL initialization */
/************************************************************************/
void SQLiteUdrDbi::initSqlSchema(U2OpStatus &os) {
    UdrSchemaRegistry *udrRegistry = AppContext::getUdrSchemaRegistry();
    SAFE_POINT_EXT(NULL != udrRegistry, os.setError("NULL UDR registry"), );

    foreach (const UdrSchemaId &id, udrRegistry->getRegisteredSchemas()) {
        const UdrSchema *schema = udrSchema(id, os);
        CHECK_OP(os, );
        initSchema(schema, os);
        CHECK_OP(os, );
    }
}

void SQLiteUdrDbi::initSchema(const UdrSchema *schema, U2OpStatus &os) {
    CHECK_EXT(NULL != schema, os.setError("NULL schema"), );
    createTable(schema, os);
    CHECK_OP(os, );

    foreach (const QStringList index, indexes(schema, os)) {
        createIndex(schema->getId(), index, os);
        CHECK_OP(os, );
    }
}

void SQLiteUdrDbi::createTable(const UdrSchema *schema, U2OpStatus &os) {
    CHECK_EXT(schema->size() > 0, os.setError("Empty schema"), );

    QString query = tableStartDef(schema->getId());
    CHECK_OP(os, );
    for (int i=0; i<schema->size(); i++) {
        UdrSchema::FieldDesc field = schema->getField(i, os);
        CHECK_OP(os, );
        query += ", " + fieldDef(field);
        CHECK_OP(os, );
    }
    query += ")";

    SQLiteQuery(query, db, os).execute();
}

void SQLiteUdrDbi::createIndex(const UdrSchemaId &schemaId, const QStringList &fields, U2OpStatus &os) {
    QString query = "CREATE INDEX "
        + tableName(schemaId) + "_" + fields.join("_") + " "
        + "on " + tableName(schemaId)
        + "("
        + fields.join(", ")
        + ")";

    SQLiteQuery(query, db, os).execute();
}

/************************************************************************/
/* Utilities */
/************************************************************************/
const UdrSchema * SQLiteUdrDbi::udrSchema(const UdrSchemaId &schemaId, U2OpStatus &os) {
    UdrSchemaRegistry *udrRegistry = AppContext::getUdrSchemaRegistry();
    SAFE_POINT_EXT(NULL != udrRegistry, os.setError("NULL UDR registry"), NULL);

    const UdrSchema *schema = udrRegistry->getSchemaById(schemaId);
    SAFE_POINT_EXT(NULL != schema, os.setError("NULL UDR schema"), NULL);
    return schema;
}

QString SQLiteUdrDbi::tableName(const UdrSchemaId &schemaId) {
    return TABLE_PREFIX + schemaId;
}

QString SQLiteUdrDbi::insertDef(const UdrSchema *schema, U2OpStatus &os) {
    QStringList nums;
    for (int i=0; i<schema->size(); i++) {
        nums << QString("?%1").arg(i+1);
    }

    return "INSERT INTO " + tableName(schema->getId())
        + "(" + fieldNames(schema, os).join(", ") + ") "
        + "VALUES(" + nums.join(", ") + ")";
}

QString SQLiteUdrDbi::selectDef(const UdrSchema *schema, U2OpStatus &os) {
    QList<int> directFields = notBinary(schema, os);
    CHECK_OP(os, "");

    return "SELECT " + UdrSchema::RECORD_ID_FIELD_NAME + ", " + fieldNames(schema, os, directFields).join(", ")
        + " FROM " + tableName(schema->getId())
        + " WHERE " + UdrSchema::RECORD_ID_FIELD_NAME + " = ?1";
}

QString SQLiteUdrDbi::tableStartDef(const UdrSchemaId &schemaId) {
    return "CREATE TABLE " + tableName(schemaId) + " (" + 
        UdrSchema::RECORD_ID_FIELD_NAME + " INTEGER PRIMARY KEY AUTOINCREMENT";
}

QString SQLiteUdrDbi::fieldDef(const UdrSchema::FieldDesc &field) {
    QString def = field.getName() + " ";
    switch (field.getDataType()) {
        case UdrSchema::INTEGER:
            def += "INTEGER";
            break;
        case UdrSchema::DOUBLE:
            def += "REAL";
            break;
        case UdrSchema::STRING:
            def += "TEXT";
            break;
        case UdrSchema::BLOB:
            def += "BLOB";
            break;
    }
    return def;
}

QStringList SQLiteUdrDbi::fieldNames(const UdrSchema *schema, U2OpStatus &os, const QList<int> &nums) {
    QStringList result;
    QList<int> target = nums;
    if (nums.isEmpty()) {
        for (int i=0; i<schema->size(); i++) {
            target << i;
        }
    }
    foreach (int fieldNum, target) {
        UdrSchema::FieldDesc field = schema->getField(fieldNum, os);
        CHECK_OP(os, result);
        result << field.getName();
    }
    return result;
}

QList< QStringList > SQLiteUdrDbi::indexes(const UdrSchema *schema, U2OpStatus &os) {
    QList< QStringList > result;

    // single column indexes
    for (int i=0; i<schema->size(); i++) {
        UdrSchema::FieldDesc field = schema->getField(i, os);
        CHECK_OP(os, result);
        if (UdrSchema::INDEXED == field.getIndexType()) {
            QStringList index;
            index << field.getName();
            result << index;
        }
    }

    // multi column indexes
    foreach (const QList<int> &multiIndex, schema->getMultiIndexes()) {
        result << fieldNames(schema, os, multiIndex);
        CHECK_OP(os, result);
    }

    return result;
}

void SQLiteUdrDbi::bindData(const QList<UdrValue> &data, const UdrSchema *schema, SQLiteQuery &q, U2OpStatus &os) {
    for (int i=0; i<data.size(); i++) {
        const UdrValue &value = data[i];
        UdrSchema::FieldDesc field = schema->getField(i, os);
        CHECK_OP(os, );

        switch (field.getDataType()) {
            case UdrSchema::INTEGER:
                q.bindInt64(i+1, value.getInt(os));
                break;
            case UdrSchema::DOUBLE:
                q.bindDouble(i+1, value.getDouble(os));
                break;
            case UdrSchema::STRING:
                q.bindString(i+1, value.getString(os));
                break;
            case UdrSchema::BLOB:
                q.bindBlob(i+1, "");
                break;
        }
        CHECK_OP(os, );
    }
}

void SQLiteUdrDbi::retreiveData(QList<UdrValue> &data, const UdrSchema *schema, SQLiteQuery &q, U2OpStatus &os) {
    QList<int> fields = notBinary(schema, os);
    CHECK_OP(os, );

    for (int i=0; i<schema->size(); i++) {
        UdrSchema::FieldDesc field = schema->getField(i, os);
        CHECK_OP(os, );
        int colNum = -1;
        if (UdrSchema::BLOB != field.getDataType()) {
            colNum = fields.lastIndexOf(i) + 1;
        }
        switch (field.getDataType()) {
            case UdrSchema::INTEGER:
                data << UdrValue(q.getInt64(colNum));
                break;
            case UdrSchema::DOUBLE:
                data << UdrValue(q.getDouble(colNum));
                break;
            case UdrSchema::STRING:
                data << UdrValue(q.getString(colNum));
                break;
            case UdrSchema::BLOB:
                data << UdrValue();
                break;
        }
        CHECK_OP(os, );
    }
}

QList<int> SQLiteUdrDbi::notBinary(const UdrSchema *schema, U2OpStatus &os) {
    QList<int> result;
    for (int i=0; i<schema->size(); i++) {
        UdrSchema::FieldDesc field = schema->getField(i, os);
        CHECK_OP(os, result);
        if (UdrSchema::BLOB != field.getDataType()) {
            result << i;
        }
    }
    return result;
}

UdrSchema::FieldDesc SQLiteUdrDbi::getBlobField(const UdrSchemaId &schemaId, int fieldNum, U2OpStatus &os) {
    const UdrSchema *schema = udrSchema(schemaId, os);
    CHECK_OP(os, UdrSchema::FieldDesc("", UdrSchema::INTEGER));
    UdrSchema::FieldDesc field = schema->getField(fieldNum, os);
    CHECK_OP(os, field);

    if (UdrSchema::BLOB != field.getDataType()) {
        os.setError("Only BLOB fields can be used");
    }
    return field;
}

} // U2
