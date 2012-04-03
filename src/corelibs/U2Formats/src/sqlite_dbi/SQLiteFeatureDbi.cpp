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

#include "SQLiteFeatureDbi.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2SafePoints.h>

#include <memory>

namespace U2 {

SQLiteFeatureDbi::SQLiteFeatureDbi(SQLiteDbi* dbi) : U2FeatureDbi(dbi), SQLiteChildDBICommon(dbi) {
}

void SQLiteFeatureDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }
    SQLiteQuery("CREATE TABLE Feature (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, parent INTEGER, name TEXT, sequence INTEGER NOT NULL, "
        " rversion INTEGER NOT NULL DEFAULT 0, lversion INTEGER NOT NULL DEFAULT 0,"
        " strand INTEGER NOT NULL DEFAULT 0, start INTEGER NOT NULL DEFAULT 0, len INTEGER NOT NULL DEFAULT 0, "
        " FOREIGN KEY(sequence) REFERENCES Object(id))", db, os).execute();

    SQLiteQuery("CREATE TABLE FeatureKey (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, feature INTEGER NOT NULL, "
        " name TEXT NOT NULL, value TEXT NOT NULL, "
        " FOREIGN KEY(feature) REFERENCES Feature(id))", db, os).execute();

    //id: feature id
    SQLiteQuery("CREATE VIRTUAL TABLE FeatureLocationRTreeIndex USING rtree_i32(id, start, end)", db, os).execute();

    // TODO: create indexes
}

#define FDBI_FIELDS QString("f.id, f.parent, f.name, f.sequence, f.rversion, f.lversion, f.strand, f.start, f.len ")
class SqlFeatureRSLoader : public SqlRSLoader<U2Feature> {
public:
    U2Feature load(SQLiteQuery* q) {
        return loadStatic(q);
    }

    static U2Feature loadStatic(SQLiteQuery* q) {
        U2Feature res;
        //parent, name, sequence, rversion, lversion, strand, start, len
        res.id = q->getDataId(0, U2Type::Feature);
        res.parentFeatureId = q->getDataId(1, U2Type::Feature);
        res.name = q->getString(2);
        res.sequenceId = q->getDataId(3, U2Type::Sequence);
        res.treeVersion = q->getInt32(4);
        res.version = q->getInt32(5);
        res.location.strand = U2Strand(U2Strand::Direction(q->getInt32(6)));
        res.location.region.startPos = q->getInt64(7);
        res.location.region.length= q->getInt64(8);
        return res;
    }
};

U2Feature SQLiteFeatureDbi::getFeature(const U2DataId& featureId, U2OpStatus& os) {
    U2Feature res;
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, res);

    SQLiteQuery q("SELECT " + FDBI_FIELDS + " FROM Feature AS f WHERE id = ?1", db, os);
    q.bindDataId(1, featureId);
    q.execute();
    CHECK_OP(os, res);

    res = SqlFeatureRSLoader::loadStatic(&q);
    return res;
}

static void add (QString& buf, const QString& str, const QString& op, int& n) {
    if (!buf.isEmpty()) {
        buf += " AND ";
    }
    buf += str;
    if (!op.isEmpty()) {
        n++;
        buf += op + "?" + QString::number(n);
    }
}

static QString toSqlCompareOp(ComparisonOp op) {
    QString res;
    switch(op) {
        case ComparisonOp_EQ  : res = "=";  break;
        case ComparisonOp_NEQ : res = "!="; break;
        case ComparisonOp_GT  : res = ">";  break;
        case ComparisonOp_GET : res = ">="; break;
        case ComparisonOp_LT  : res = "<";  break;
        case ComparisonOp_LET : res = "<="; break;
        default: res = "?"; break;
    }
    return res;
}

static QString toSqlOrderOp(OrderOp op) {
    QString res;
    switch(op) {
        case OrderOp_Asc: res = "ASC";  break;
        case OrderOp_Desc : res = "DESC"; break;
        default: break;
    }
    return res;
}


SQLiteQuery* SQLiteFeatureDbi::createFeatureQuery(const QString& selectPart, const FeatureQuery& fq, bool useOrder, U2OpStatus& os) {
    QString wherePart;
    int n = 0;
    bool useParent = !fq.parentFeatureId.isEmpty();
    if (useParent) {
        DBI_TYPE_CHECK(fq.parentFeatureId, U2Type::Feature, os, NULL);
        add(wherePart, "f.parent", "=", n);
    } else if (fq.topLevelOnly) {
        add(wherePart, "f.parent", "=", n);
    }

    bool useFeatureName = !fq.featureName.isEmpty();
    if(useFeatureName){
        add(wherePart, "f.name", "=", n);
    }

    bool useSequence = !fq.sequenceId.isEmpty();
    if (useSequence) {
        DBI_TYPE_CHECK(fq.parentFeatureId, U2Type::Sequence, os, NULL);
        add(wherePart, "f.sequenceId", "=", n);
    }

    bool useKeyName = !fq.keyName.isEmpty();
    if (useKeyName) {
        add(wherePart, "fk.name", "=" , n);
    }

    bool useKeyValue = !fq.keyValue.isEmpty();
    if (useKeyValue) {
        add(wherePart, "fk.value", toSqlCompareOp(fq.keyValueCompareOp) , n);
    }

    bool useRegion = fq.intersectRegion.length > 0;
    if (useRegion) {
        add(wherePart, QString("(fr.start < ?%2 AND fr.end > ?%1) AND fr.id = f.id ").arg(n + 1).arg(n + 2), "", n);
        n += 2;
    }

    bool useKeyTable = useKeyName || useKeyValue;
    if (useKeyValue) {
        wherePart += " AND fk.feature = f.id ";
    }

    if (useOrder) {
        if(fq.featureNameOrderOp != OrderOp_None){
            wherePart += " ORDER BY f.name " + toSqlOrderOp(fq.featureNameOrderOp);
        }else if (fq.keyNameOrderOp != OrderOp_None) {
            wherePart += " ORDER BY fk.name " + toSqlOrderOp(fq.keyNameOrderOp);
        } else if (fq.keyValueOrderOp != OrderOp_None) {
            wherePart += " ORDER BY fk.value";
        } else if (fq.startPosOrderOp != OrderOp_None && useRegion) {
            wherePart += " ORDER BY fr.start";
        }
    }

    QString tablesPart = "Feature AS f";
    if (useKeyTable) {
        tablesPart+=", FeatureKey AS fk";
    }
    if (useRegion) {
        tablesPart+=", FeatureLocationRTreeIndex AS fr";
    }

    QString fullQuery = selectPart + " FROM " + tablesPart;
    if(!wherePart.isEmpty()) {
        fullQuery += " WHERE " + wherePart;
    }
    SQLiteQuery* q = new SQLiteQuery(fullQuery, db, os);
    int m = 0;
    
    if (useParent) {
        q->bindDataId(++m, fq.parentFeatureId);
    } else if (fq.topLevelOnly) {
        q->bindDataId(++m, U2DataId());
    }
    if(useFeatureName){
        q->bindString(++m, fq.featureName);
    }
    if (useSequence) {
        q->bindDataId(++m, fq.sequenceId);
    }
    if (useKeyName) {
        q->bindString(++m, fq.keyName);
    }
    if (useKeyValue) {
        q->bindString(++m, fq.keyValue);
    }
    if (useRegion) {
        q->bindInt64(++m, fq.intersectRegion.startPos);
        q->bindInt64(++m, fq.intersectRegion.endPos());
    }
    return q;

}

qint64 SQLiteFeatureDbi::countFeatures(const FeatureQuery& fq, U2OpStatus& os) {
    std::auto_ptr<SQLiteQuery> q(createFeatureQuery("SELECT COUNT(*)", fq, false, os));
    CHECK_OP(os, -1);
    return q->selectInt64();
}


U2DbiIterator<U2Feature>* SQLiteFeatureDbi::getFeatures(const FeatureQuery& fq, U2OpStatus& os) {
    SQLiteQuery* q = createFeatureQuery("SELECT " + FDBI_FIELDS, fq, true, os);
    CHECK_OP_EXT(os, delete q, NULL);
    return new SqlRSIterator<U2Feature>(q, new SqlFeatureRSLoader(), NULL, U2Feature(), os);
}

QList<U2FeatureKey> SQLiteFeatureDbi::getFeatureKeys(const U2DataId& featureId, U2OpStatus& os) {
    SQLiteQuery q("SELECT name, value FROM FeatureKey WHERE feature = ?1", db, os);
    q.bindDataId(1, featureId);
    CHECK_OP(os, QList<U2FeatureKey>());
    QList<U2FeatureKey> result;
    while(q.step()) {
        U2FeatureKey key;
        key.name = q.getString(0);
        key.value = q.getString(1);
        result.append(key);
    }
    return result;
}

static void addKeyCommon(SQLiteQuery& qk, const U2DataId& featureId, const U2FeatureKey& key) {
    qk.reset();
    qk.bindDataId(1, featureId);
    qk.bindString(2, key.name);
    qk.bindString(3, key.value);
    qk.insert();
}

void SQLiteFeatureDbi::createFeature(U2Feature& feature, const QList<U2FeatureKey>& keys, U2OpStatus& os) {
    SQLiteTransaction t(db, os);

    SQLiteQuery qf("INSERT INTO Feature(parent, name, sequence, strand, start, len) VALUES(?1, ?2, ?3, ?4, ?5, ?6)" , db, os);
    SQLiteQuery qk("INSERT INTO FeatureKey(feature, name, value) VALUES(?1, ?2, ?3)"  , db, os);
    SQLiteQuery qr("INSERT INTO FeatureLocationRTreeIndex(id, start, end) VALUES(?1, ?2, ?3)"  , db, os);

    qf.bindDataId(1, feature.parentFeatureId);
    qf.bindString(2, feature.name);
    qf.bindDataId(3, feature.sequenceId);
    qf.bindInt32(4, feature.location.strand.getDirectionValue());
    qf.bindInt64(5, feature.location.region.startPos);
    qf.bindInt64(6, feature.location.region.length);
    feature.id = qf.insert(U2Type::Feature);
    CHECK_OP(os, );

    foreach (const U2FeatureKey& key, keys) {
        addKeyCommon(qk, feature.id, key);
        CHECK_OP(os, );
    }

    qr.bindDataId(1, feature.id);
    qr.bindInt64(2, feature.location.region.startPos);
    qr.bindInt64(3, feature.location.region.endPos());
    qr.execute();
}

void SQLiteFeatureDbi::addKey(const U2DataId& featureId, const U2FeatureKey& key, U2OpStatus& os) {
    SQLiteQuery qk("INSERT INTO FeatureKey(feature, key, value) VALUES(?1, ?2, ?3)"  , db, os);
    addKeyCommon(qk, featureId, key);
}

void SQLiteFeatureDbi::removeAllKeys(const U2DataId& featureId, const QString& keyName, U2OpStatus& os) {
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );

    SQLiteQuery q("DELETE FROM FeatureKey WHERE feature = ?1 AND name = ?2" , db, os);
    q.bindDataId(1, featureId);
    q.bindString(2, keyName);
    q.execute();
}

void SQLiteFeatureDbi::updateKeyValue(const U2DataId& featureId, const U2FeatureKey& key, U2OpStatus& os){
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );

    SQLiteQuery q("UPDATE FeatureKey SET value = ?3 WHERE feature = ?1 AND key = ?2" , db, os);
    q.bindDataId(1, featureId);
    q.bindString(2, key.name);
    q.bindString(3, key.value);
    q.execute();
}

void SQLiteFeatureDbi::removeAllKeys(const U2DataId& featureId, const U2FeatureKey& key, U2OpStatus& os) {
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );

    SQLiteQuery q("DELETE FROM FeatureKey WHERE feature = ?1 AND name = ?2 AND value = ?3" , db, os);
    q.bindDataId(1, featureId);
    q.bindString(2, key.name);
    q.bindString(3, key.value);
    q.execute();
}

void SQLiteFeatureDbi::updateLocation(const U2DataId& featureId, const U2FeatureLocation& location, U2OpStatus& os) {
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );

    SQLiteTransaction t(db, os);

    SQLiteQuery qf("UPDATE Feature SET strand = ?1, start = ?2, len = ?3 WHERE id = ?4" , db, os);
    qf.bindInt32(1, location.strand.getDirectionValue());
    qf.bindInt64(2, location.region.startPos);
    qf.bindInt64(3, location.region.length);
    qf.bindDataId(4, featureId);
    qf.execute();
    CHECK_OP(os, );

    SQLiteQuery qr("UPDATE FeatureLocationRTreeIndex SET start = ?1, end = ?2 WHERE id = ?3" , db, os);
    qr.bindInt64(1, location.region.startPos);
    qr.bindInt64(2, location.region.endPos());
    qr.bindDataId(3, featureId);
    qr.execute();
}

void SQLiteFeatureDbi::updateName(const U2DataId& featureId, const QString& newName, U2OpStatus& os){
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );

    SQLiteTransaction t(db, os);

    SQLiteQuery qf("UPDATE Feature SET name = ?1 WHERE id = ?2" , db, os);
    qf.bindDataId(1, featureId);
    qf.bindString(2, newName);
    qf.execute();
    CHECK_OP(os, );
}

void SQLiteFeatureDbi::removeFeature(const U2DataId& featureId, U2OpStatus& os) {
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );

    SQLiteTransaction t(db, os);

    SQLiteQuery qk("DELETE FROM FeatureKey WHERE feature = ?1" , db, os);
    qk.bindDataId(1, featureId);
    qk.execute();
    CHECK_OP(os, );

    SQLiteQuery qr("DELETE FROM FeatureLocationRTreeIndex WHERE id = ?1" , db, os);
    qr.bindDataId(1, featureId);
    qr.execute();
    CHECK_OP(os, );

    SQLiteQuery qf("DELETE FROM Feature WHERE id = ?1" , db, os);
    qf.bindDataId(1, featureId);
    qf.execute();
}


} //namespace
