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

#include "SQLiteAttributeDbi.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/Timer.h>

namespace U2 {

SQLiteAttributeDbi::SQLiteAttributeDbi(SQLiteDbi* dbi) : U2AttributeDbi(dbi), SQLiteChildDBICommon(dbi) {
}

void SQLiteAttributeDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }

    // object attribute main table
    // object -> object id this attribute is for
    // child -> optional object id in case if this attribute shows relation between 2 objects
    // otype, ctype -> object and child types
    // oextra, cextra -> object and child db extra
    // version -> object version is attribute is valid for
    // name -> name of the attribute
    SQLiteQuery("CREATE TABLE Attribute (id INTEGER PRIMARY KEY AUTOINCREMENT, type INTEGER NOT NULL, "
        " object INTEGER, child INTEGER, otype INTEGER NOT NULL, ctype INTEGER, oextra BLOB NOT NULL, cextra BLOB, "
        " version INTEGER NOT NULL, name TEXT NOT NULL, "
        " FOREIGN KEY(object) REFERENCES Object(id), FOREIGN KEY(child) REFERENCES Object(id) )" , db, os).execute();

    SQLiteQuery("CREATE INDEX Attribute_name on Attribute(name)" , db, os).execute();
    
    SQLiteQuery("CREATE TABLE IntegerAttribute (attribute INTEGER, value INTEGER NOT NULL, "
        " FOREIGN KEY(attribute) REFERENCES Attribute(id) )" , db, os).execute();

    SQLiteQuery("CREATE TABLE RealAttribute (attribute INTEGER, value REAL NOT NULL, "
        " FOREIGN KEY(attribute) REFERENCES Attribute(id) )" , db, os).execute();

    SQLiteQuery("CREATE TABLE StringAttribute (attribute INTEGER, value TEXT NOT NULL, "
        " FOREIGN KEY(attribute) REFERENCES Attribute(id) )" , db, os).execute();

    SQLiteQuery("CREATE TABLE ByteArrayAttribute (attribute INTEGER, value BLOB NOT NULL, "
        " FOREIGN KEY(attribute) REFERENCES Attribute(id) )" , db, os).execute();

}

/** Returns all attribute names available in the database */
QStringList SQLiteAttributeDbi::getAvailableAttributeNames(U2OpStatus& os) {
    return SQLiteQuery("SELECT DISTINCT name FROM Attribute", db, os).selectStrings();
}

/** Returns all attribute ids for the given object */
QList<U2DataId> SQLiteAttributeDbi::getObjectAttributes(const U2DataId& objectId, U2OpStatus& os) {
    SQLiteQuery q("SELECT id, type, '' FROM Attribute WHERE object = ?1", db, os);
    q.bindDataId(1, objectId);
    return q.selectDataIdsExt();
}

/** Returns all attribute ids for the given object */
QList<U2DataId> SQLiteAttributeDbi::getObjectPairAttributes(const U2DataId& objectId, const U2DataId& childId, U2OpStatus& os) {
    SQLiteQuery q("SELECT id, type, '' FROM Attribute WHERE object = ?1 AND child = ?2", db, os);
    q.bindDataId(1, objectId);
    q.bindDataId(2, childId);
    return q.selectDataIdsExt();
}

/** Loads int64 attribute by id */
U2IntegerAttribute SQLiteAttributeDbi::getIntegerAttribute(const U2DataId& attributeId, U2OpStatus& os) {
    SQLiteQuery q(buildSelectAttributeQuery("IntegerAttribute"), db, os);
    q.bindDataId(1, attributeId);
    U2IntegerAttribute res;
    readAttribute(q, res);
    res.value = q.getInt64(0);
    q.ensureDone();
    return res;
}

/** Loads real64 attribute by id */
U2RealAttribute SQLiteAttributeDbi::getRealAttribute(const U2DataId& attributeId, U2OpStatus& os) {
    SQLiteQuery q(buildSelectAttributeQuery("RealAttribute"), db, os);
    q.bindDataId(1, attributeId);
    U2RealAttribute res;
    readAttribute(q, res);
    res.value = q.getDouble(0);
    q.ensureDone();
    return res;
}

/** Loads String attribute by id */
U2StringAttribute SQLiteAttributeDbi::getStringAttribute(const U2DataId& attributeId, U2OpStatus& os) {
    SQLiteQuery q(buildSelectAttributeQuery("StringAttribute"), db, os);
    q.bindDataId(1, attributeId);
    U2StringAttribute res;
    readAttribute(q, res);
    res.value = q.getString(0);
    q.ensureDone();
    return res;
}

/** Loads byte attribute by id */
U2ByteArrayAttribute SQLiteAttributeDbi::getByteArrayAttribute(const U2DataId& attributeId, U2OpStatus& os) {
    SQLiteQuery q(buildSelectAttributeQuery("ByteArrayAttribute"), db, os);
    q.bindDataId(1, attributeId);
    U2ByteArrayAttribute res;
    readAttribute(q, res);
    res.value = q.getBlob(0);
    q.ensureDone();
    return res;
}


QString SQLiteAttributeDbi::buildSelectAttributeQuery(const QString& attributeTable) {
    return "SELECT t.value, a.id, a.type, '', a.object, a.otype, a.oextra, a.child, a.ctype, a.cextra, a.version, a.name "
            " FROM Attribute AS a, " + attributeTable + " AS t WHERE a.id = ?1 AND t.attribute = a.id";
}

void SQLiteAttributeDbi::readAttribute(SQLiteQuery& q, U2Attribute& attr) {
    if (q.hasError()) {
        return;
    }
    if (!q.step()) {
        if (!q.hasError()) {
            q.setError(SQLiteL10n::tr("Attribute not found!"));
        }
        return;
    }
    attr.id = q.getDataIdExt(1);
    attr.objectId = q.getDataIdExt(4);
    attr.childId = q.getDataIdExt(7);
    attr.version = q.getInt64(10);
    attr.name = q.getString(11);
}


/** Sorts all objects in database according to U2DbiSortConfig provided  */
QList<U2DataId> SQLiteAttributeDbi::sort(const U2DbiSortConfig& sc, qint64 offset, qint64 count, U2OpStatus& os) {
    QList<U2DataId> res;
    os.setError("not implemented");
    return res;
}


static void removeAttribute(SQLiteQuery& q, const U2DataId& id) {
    q.reset();
    q.bindDataId(1, id);
    q.execute();
}
/** 
Removes attribute from database 
Requires U2DbiFeature_WriteAttribute feature support
*/
void SQLiteAttributeDbi::removeAttributes(const QList<U2DataId>& attributeIds, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    SQLiteQuery q("DELETE FROM Attribute WHERE id = ?1", db, os);
    SQLiteQuery qi("DELETE FROM IntegerAttribute WHERE attribute = ?1", db, os);
    SQLiteQuery qr("DELETE FROM RealAttribute WHERE attribute = ?1", db, os);
    SQLiteQuery qs("DELETE FROM StringAttribute WHERE attribute = ?1", db, os);
    SQLiteQuery qb("DELETE FROM ByteArrayAttribute WHERE attribute = ?1", db, os);
    foreach(const U2DataId& id, attributeIds) {
        q.reset();
        q.bindDataId(1, id);
        q.execute();
        U2DataType type = SQLiteUtils::toType(id);
        switch (type) {
            case U2Type::AttributeInteger:
                removeAttribute(qi, id);
                break;
            case U2Type::AttributeReal:
                removeAttribute(qr, id);
                break;
            case U2Type::AttributeString:
                removeAttribute(qs, id);
                break;
            case U2Type::AttributeByteArray:
                removeAttribute(qb, id);
                break;
            default:
                os.setError(SQLiteL10n::tr("Unsupported attribute type: %1").arg(type));
                break;
        }
        if (os.hasError()) {
            break;
        }
    }
}

qint64 SQLiteAttributeDbi::createAttribute(U2Attribute& attr, U2DataType type, U2OpStatus& os) {
    SQLiteQuery q("INSERT INTO Attribute(type, object, child, otype, ctype, oextra, cextra, version, name) "
                " VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9)", db, os);
    
    q.bindType(1, type);
    q.bindDataId(2, attr.objectId);
    q.bindDataId(3, attr.childId);
    q.bindType(4, SQLiteUtils::toType(attr.objectId));
    q.bindType(5, SQLiteUtils::toType(attr.childId));
    q.bindBlob(6, SQLiteUtils::toDbExtra(attr.objectId));
    q.bindBlob(7, SQLiteUtils::toDbExtra(attr.childId));
    q.bindInt64(8, attr.version);
    q.bindString(9, attr.name);
    
    return q.insert();
}


/** 
Creates int64 attribute in database. ObjectId must be already set in attribute and present in the same database 
Requires U2DbiFeature_WriteAttribute feature support
*/    
void SQLiteAttributeDbi::createIntegerAttribute(U2IntegerAttribute& a, U2OpStatus& os) {
    qint64 id = createAttribute(a, U2Type::AttributeInteger, os);
    if (os.hasError()) {
        return;
    }
    a.id = SQLiteUtils::toU2DataId(id, U2Type::AttributeInteger);

    SQLiteQuery q("INSERT INTO IntegerAttribute(attribute, value) VALUES(?1, ?2)", db, os);
    q.bindInt64(1, id);
    q.bindInt64(2, a.value);
    q.execute();
}

/** 
Creates real64 attribute in database. ObjectId must be already set in attribute and present in the same database 
Requires U2DbiFeature_WriteAttribute feature support
*/    
void SQLiteAttributeDbi::createRealAttribute(U2RealAttribute& a, U2OpStatus& os) {
    qint64 id = createAttribute(a, U2Type::AttributeReal, os);
    if (os.hasError()) {
        return;
    }
    a.id = SQLiteUtils::toU2DataId(id, U2Type::AttributeReal);

    SQLiteQuery q("INSERT INTO RealAttribute(attribute, value) VALUES(?1, ?2)", db, os);
    q.bindInt64(1, id);
    q.bindDouble(2, a.value);
    q.execute();
}

/** 
Creates String attribute in database. ObjectId must be already set in attribute and present in the same database 
Requires U2DbiFeature_WriteAttribute feature support
*/    
void SQLiteAttributeDbi::createStringAttribute(U2StringAttribute& a, U2OpStatus& os) {
    qint64 id = createAttribute(a, U2Type::AttributeString, os);
    if (os.hasError()) {
        return;
    }
    a.id = SQLiteUtils::toU2DataId(id, U2Type::AttributeString);

    SQLiteQuery q("INSERT INTO StringAttribute(attribute, value) VALUES(?1, ?2)", db, os);
    q.bindInt64(1, id);
    q.bindString(2, a.value);
    q.execute();
}

/** 
Creates Byte attribute in database. ObjectId must be already set in attribute and present in the same database 
Requires U2DbiFeature_WriteAttribute feature support
*/    
void SQLiteAttributeDbi::createByteArrayAttribute(U2ByteArrayAttribute& a, U2OpStatus& os) {
    qint64 id = createAttribute(a, U2Type::AttributeByteArray, os);
    if (os.hasError()) {
        return;
    }
    a.id = SQLiteUtils::toU2DataId(id, U2Type::AttributeByteArray);

    SQLiteQuery q("INSERT INTO ByteArrayAttribute(attribute, value) VALUES(?1, ?2)", db, os);
    q.bindInt64(1, id);
    q.bindBlob(2, a.value, false);
    q.execute();
}

} //namespace
