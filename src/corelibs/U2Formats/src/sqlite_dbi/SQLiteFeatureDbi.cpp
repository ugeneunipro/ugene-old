/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <QtCore/QQueue>

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2SafePoints.h>

#include "SQLiteObjectDbi.h"
#include "SQLiteFeatureDbi.h"

namespace U2 {

SQLiteFeatureDbi::SQLiteFeatureDbi(SQLiteDbi* dbi) : U2FeatureDbi(dbi), SQLiteChildDBICommon(dbi) {
}

void SQLiteFeatureDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }
    // annotation table object
    SQLiteQuery( "CREATE TABLE AnnotationTable (object INTEGER UNIQUE, rootId INTEGER NOT NULL DEFAULT 0, "
        "FOREIGN KEY(object) REFERENCES Object(id), "
        "FOREIGN KEY(rootId) REFERENCES Feature(id) )", db, os ).execute( );

    //nameHash is used for better indexing
    SQLiteQuery("CREATE TABLE Feature (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, parent INTEGER, nameHash INTEGER, name TEXT, sequence INTEGER NOT NULL, "
        " strand INTEGER NOT NULL DEFAULT 0, start INTEGER NOT NULL DEFAULT 0, len INTEGER NOT NULL DEFAULT 0, "
        " FOREIGN KEY(sequence) REFERENCES Object(id))", db, os).execute();

    SQLiteQuery("CREATE TABLE FeatureKey (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, feature INTEGER NOT NULL, "
        " name TEXT NOT NULL, value TEXT NOT NULL, "
        " FOREIGN KEY(feature) REFERENCES Feature(id))", db, os).execute();

    //Feature index
    SQLiteQuery("CREATE VIRTUAL TABLE FeatureLocationRTreeIndex USING rtree_i32(id, start, end)", db, os).execute();

    SQLiteQuery("CREATE INDEX IF NOT EXISTS FeatureParentIndex ON Feature(parent, sequence)" ,db, os).execute();
    SQLiteQuery("CREATE INDEX IF NOT EXISTS FeatureSequenceNameIndex ON Feature(sequence, name)" ,db, os).execute();
    SQLiteQuery("CREATE INDEX IF NOT EXISTS FeatureSequenceNameIndex ON Feature(start)" ,db, os).execute();

    //FeatureKey index
    SQLiteQuery("CREATE INDEX IF NOT EXISTS FeatureKeyIndex ON FeatureKey(feature)" ,db, os).execute();
}

#define FDBI_FIELDS QString("f.id, f.parent, f.name, f.sequence, f.strand, f.start, f.len, f.nameHash ")
class SqlFeatureRSLoader : public SqlRSLoader<U2Feature> {
public:
    U2Feature load(SQLiteQuery* q) {
        return loadStatic(q);
    }

    static U2Feature loadStatic(SQLiteQuery* q) {
        U2Feature res;
        //parent, name, sequence, strand, start, len
        res.id = q->getDataId(0, U2Type::Feature);
        res.parentFeatureId = q->getDataId(1, U2Type::Feature);
        res.name = q->getString(2);
        res.sequenceId = q->getDataId(3, U2Type::Sequence);
        res.location.strand = U2Strand(U2Strand::Direction(q->getInt32(4)));
        res.location.region.startPos = q->getInt64(5);
        res.location.region.length= q->getInt64(6);
        return res;
    }
};

class SqlFeatureFilter: public SqlRSFilter<U2Feature>{
public:
    SqlFeatureFilter(const QString& name, const U2DataId& _seqId){
        nameToFilter = name;
        seqId = _seqId;
    }
    bool filter(const U2Feature& f){
        if ((nameToFilter.isEmpty() || f.name == nameToFilter) && (seqId.isEmpty() || seqId == f.sequenceId)){
            return true;
        }
        return false;
    }

private:
    QString nameToFilter;
    U2DataId seqId;

};

void SQLiteFeatureDbi::createAnnotationTableObject( U2AnnotationTable &table,
    const QString &folder, U2OpStatus &os )
{
    dbi->getSQLiteObjectDbi( )->createObject( table, folder, SQLiteDbiObjectRank_TopLevel, os );
    CHECK_OP( os, );

    static const QString queryString( "INSERT INTO AnnotationTable (object, rootId) VALUES(?1, ?2)" );
    SQLiteQuery q( queryString, db, os );
    CHECK_OP( os, );
    q.bindDataId( 1, table.id );
    q.bindDataId( 2, table.rootFeature );
    q.insert( );
    SAFE_POINT( !os.hasError( ), "Error on Feature DB insertion!", );
}

U2AnnotationTable SQLiteFeatureDbi::getAnnotationTableObject( const U2DataId &tableId,
    U2OpStatus &os )
{
    U2AnnotationTable result;

    DBI_TYPE_CHECK( tableId, U2Type::AnnotationTable, os, result );
    dbi->getSQLiteObjectDbi( )->getObject( result, tableId, os );
    CHECK_OP( os, result );

    SQLiteQuery q( "SELECT rootId FROM AnnotationTable WHERE object = ?1", db, os );
    q.bindDataId( 1, tableId );
    if ( q.step( ) ) {
        result.rootFeature = q.getDataId( 0, U2Type::Feature );
        q.ensureDone( );
    } else if ( !os.hasError( ) ) {
        os.setError( SQLiteL10N::tr( "Annotation table object not found." ) );
    }
    return result;
}

void SQLiteFeatureDbi::renameAnnotationTableObject( const U2DataId &tableId,
    const QString &name, U2OpStatus &os )
{
    U2Object tableObj;
    dbi->getSQLiteObjectDbi( )->getObject( tableObj, tableId, os );
    CHECK_OP( os, );

    SQLiteObjectDbiUtils::renameObject( dbi, tableObj, name, os );
}

U2Feature SQLiteFeatureDbi::getFeature(const U2DataId& featureId, U2OpStatus& os) {
    U2Feature res;
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, res);

    const QString queryString("SELECT " + FDBI_FIELDS + " FROM Feature AS f WHERE id = ?1");
    SQLiteQuery q(queryString, db, os);
    q.bindDataId(1, featureId);
    q.execute();
    CHECK_OP(os, res);

    res = SqlFeatureRSLoader::loadStatic(&q);
    return res;
}

static void add(QString& buf, const QString& str, const QString& op, int& n) {
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

static QString toSqlOrderOpFromCompareOp(ComparisonOp op) {
    QString res;
    switch(op) {
        case ComparisonOp_EQ  : break;
        case ComparisonOp_NEQ : break;
        case ComparisonOp_GT  : res = "ASC";  break;
        case ComparisonOp_GET : res = "ASC"; break;
        case ComparisonOp_LT  : res = "DESC";  break;
        case ComparisonOp_LET : res = "DESC"; break;
        default: break;
    }
    return res;
}

QSharedPointer<SQLiteQuery> SQLiteFeatureDbi::createFeatureQuery( const QString &selectPart,
    const FeatureQuery &fq, bool useOrder, U2OpStatus &os, SQLiteTransaction *trans )
{
    QString wherePart;
    int n = 0;

    bool useParent = !fq.parentFeatureId.isEmpty( );
    if ( useParent ) {
        DBI_TYPE_CHECK( fq.parentFeatureId, U2Type::Feature, os, QSharedPointer<SQLiteQuery>( ) );
        add( wherePart, "f.parent", "=", n );
    } else if ( fq.topLevelOnly ) {
        add( wherePart, "f.parent", "=", n );
    }

    bool useName = !fq.featureName.isEmpty( );
    if ( useName ) {
        add( wherePart, "f.nameHash", "=", n );
    }

    bool useSequence = !fq.sequenceId.isEmpty( );
    if ( useSequence ) {
        DBI_TYPE_CHECK( fq.sequenceId, U2Type::Sequence, os, QSharedPointer<SQLiteQuery>( ) );
        add( wherePart, "f.sequence", "=", n );
    }

    bool useRegion = ( 0 < fq.intersectRegion.length );
    bool oneClosestFeature = ( ComparisonOp_Invalid != fq.closestFeature );
    if ( useRegion ) {
        if ( !oneClosestFeature ) { //check if intersects 
            if ( 1 != fq.intersectRegion.length ) {
                add( wherePart, QString( "fr.id = f.id AND fr.start < ?%2 AND fr.end > ?%1" )
                    .arg( n + 1 ).arg( n + 2 ), "", n );
            } else {
                add( wherePart, QString( "f.start < ?%2 AND f.start + f.len > ?%1" )
                    .arg( n + 1 ).arg( n + 2 ), "", n );
            }
            n += 2;
        } else { //check if close
            add( wherePart, QString( "fr.id = f.id AND fr.start %1 ?%2" )
                .arg( toSqlCompareOp( fq.closestFeature ) ).arg( n + 1 ), "", n );
            n++;
        }
    }

    bool useStrand = ( Strand_Both != fq.strandQuery );
    if ( useStrand ) {
        add( wherePart, "f.strand", "=", n );
    }

    bool useKeyName = !fq.keyName.isEmpty( );
    if ( useKeyName ) {
        add( wherePart, "fk.name", "=" , n );
    }

    bool useKeyValue = !fq.keyValue.isEmpty( );
    if ( useKeyValue ) {
        add( wherePart, "fk.value", toSqlCompareOp( fq.keyValueCompareOp ), n );
    }

    bool useKeyTable = ( useKeyName || useKeyValue );
    if ( useKeyTable ) {
        wherePart += " AND fk.feature = f.id ";
    }

    if ( useOrder ) {
        if ( OrderOp_None != fq.keyNameOrderOp ) {
            wherePart += " ORDER BY fk.name " + toSqlOrderOp( fq.keyNameOrderOp );
        } else if ( OrderOp_None != fq.keyValueOrderOp ) {
            wherePart += " ORDER BY fk.value";
        } else if ( OrderOp_None != fq.startPosOrderOp && useRegion ) {
            if ( !oneClosestFeature ) {
                wherePart += " ORDER BY fr.start ";
            } else {
                wherePart += QString( " ORDER BY fr.start %1 " )
                    .arg( toSqlOrderOpFromCompareOp( fq.closestFeature ) );
            }
        }
    }

    QString tablesPart = "Feature AS f";
    if ( useKeyTable ) {
        tablesPart += ", FeatureKey AS fk";
    }
    if ( useRegion && ( 1 != fq.intersectRegion.length || oneClosestFeature ) ) {
        tablesPart += ", FeatureLocationRTreeIndex AS fr";
    }

    QString fullQuery = selectPart + " FROM " + tablesPart;
    if ( !wherePart.isEmpty( ) ) {
        fullQuery += " WHERE " + wherePart;
    }
    if ( oneClosestFeature ) {
        fullQuery += " LIMIT 1 ";
    }

    QSharedPointer<SQLiteQuery> q;
    if ( NULL == trans ) {
        q = QSharedPointer<SQLiteQuery>( new SQLiteQuery( fullQuery, db, os ) );
    } else {
        q = QSharedPointer<SQLiteQuery>( trans->getPreparedQuery( fullQuery, db, os ) );
        CHECK_OP( os, QSharedPointer<SQLiteQuery>( ) );
    }
    int m = 0;

    if ( useParent ) {
        q->bindDataId( ++m, fq.parentFeatureId );
    } else if ( fq.topLevelOnly ) {
        q->bindDataId( ++m, U2DataId( ) );
    }
    if ( useName ) {
        q->bindInt32( ++m, qHash( fq.featureName ) );
    }
    if ( useSequence ) {
        q->bindDataId( ++m, fq.sequenceId );
    }
    if ( useRegion ) {
        if ( !oneClosestFeature ) {
            q->bindInt64( ++m, fq.intersectRegion.startPos );
            q->bindInt64( ++m, fq.intersectRegion.endPos( ) );
        } else {
            q->bindInt64( ++m, fq.intersectRegion.startPos );
        }
    }
    if ( useStrand ) {
        U2Strand::Direction direction = U2Strand::Direct;
        if( Strand_Direct == fq.strandQuery ) {
            direction = U2Strand::Direct;
        } else if ( Strand_Compl == fq.strandQuery ) {
            direction = U2Strand::Complementary;
        }
        q->bindInt32( ++m, direction );
    }

    if ( useKeyName ) {
        q->bindString( ++m, fq.keyName );
    }
    if ( useKeyValue ) {
        q->bindString( ++m, fq.keyValue );
    }

    return q;
}

qint64 SQLiteFeatureDbi::countFeatures( const FeatureQuery &fq, U2OpStatus &os ) {
    QSharedPointer<SQLiteQuery> q( createFeatureQuery( "SELECT COUNT(*)", fq, false, os ) );
    CHECK_OP( os, -1 );
    return q->selectInt64( );
}

U2DbiIterator<U2Feature>* SQLiteFeatureDbi::getFeatures(const FeatureQuery& fq, U2OpStatus& os) {
    QSharedPointer<SQLiteQuery> q = createFeatureQuery("SELECT " + FDBI_FIELDS, fq, true, os);
    CHECK_OP(os, NULL);
    return new SqlRSIterator<U2Feature>(q, new SqlFeatureRSLoader(), NULL, U2Feature(), os);
}

QList<U2FeatureKey> SQLiteFeatureDbi::getFeatureKeys(const U2DataId& featureId, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    static const QString queryString("SELECT name, value FROM FeatureKey WHERE feature = ?1");
    SQLiteQuery q(queryString, db, os);

    q.bindDataId(1, featureId);
    CHECK_OP(os, QList<U2FeatureKey>());
    QList<U2FeatureKey> result;
    while(q.step()) {
        U2FeatureKey key;
        key.name = q.getCString(0);
        key.value = q.getCString(1);
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

    static const QString queryStringf("INSERT INTO Feature(parent, name, sequence, strand, start, len, nameHash) VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7)");
    QSharedPointer<SQLiteQuery>qf = t.getPreparedQuery(queryStringf, db, os);

    static const QString queryStringk("INSERT INTO FeatureKey(feature, name, value) VALUES(?1, ?2, ?3)");
    SQLiteQuery qk(queryStringk, db, os);

    static const QString queryStringr("INSERT INTO FeatureLocationRTreeIndex(id, start, end) VALUES(?1, ?2, ?3)");
    QSharedPointer<SQLiteQuery> qr = t.getPreparedQuery(queryStringr, db, os);

    CHECK_OP(os, );
    qf->bindDataId(1, feature.parentFeatureId);
    qf->bindString(2, feature.name);
    qf->bindDataId(3, feature.sequenceId);
    qf->bindInt32(4, feature.location.strand.getDirectionValue());
    qf->bindInt64(5, feature.location.region.startPos);
    qf->bindInt64(6, feature.location.region.length);
    qf->bindInt32(7, qHash(feature.name));
    feature.id = qf->insert(U2Type::Feature);
    CHECK_OP(os, );

    foreach (const U2FeatureKey& key, keys) {
        addKeyCommon(qk, feature.id, key);
        CHECK_OP(os, );
    }

    qr->bindDataId(1, feature.id);
    qr->bindInt64(2, feature.location.region.startPos);
    qr->bindInt64(3, feature.location.region.endPos());
    qr->execute();
}

void SQLiteFeatureDbi::addKey(const U2DataId& featureId, const U2FeatureKey& key, U2OpStatus& os) {
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );

    SQLiteQuery qk("INSERT INTO FeatureKey(feature, name, value) VALUES(?1, ?2, ?3)"  , db, os);
    addKeyCommon(qk, featureId, key);
}

void SQLiteFeatureDbi::removeAllKeys(const U2DataId& featureId, const QString& keyName, U2OpStatus& os) {
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );

    SQLiteQuery q("DELETE FROM FeatureKey WHERE feature = ?1 AND name = ?2" , db, os);
    q.bindDataId(1, featureId);
    q.bindString(2, keyName);
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

void SQLiteFeatureDbi::updateName(const U2DataId& featureId, const QString& newName, U2OpStatus& os){
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );

    SQLiteQuery qf("UPDATE Feature SET name = ?3, nameHash = ?2 WHERE id = ?1" , db, os);
    qf.bindDataId(1, featureId);
    qf.bindInt32(2, qHash(newName));
    qf.bindString(3, newName);
    qf.execute();
}

void SQLiteFeatureDbi::updateParentId(const U2DataId& featureId, const U2DataId& parentId, U2OpStatus& os){
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );
    DBI_TYPE_CHECK(parentId, U2Type::Feature, os, );

    SQLiteQuery qf("UPDATE Feature SET parent = ?1 WHERE id = ?2" , db, os);
    qf.bindDataId(1, parentId);
    qf.bindDataId(2, featureId);
    qf.execute();
}

void SQLiteFeatureDbi::updateSequenceId( const U2DataId &featureId, const U2DataId &seqId,
    U2OpStatus &os )
{
    DBI_TYPE_CHECK( featureId, U2Type::Feature, os, );
    DBI_TYPE_CHECK( seqId, U2Type::Sequence, os, );

    SQLiteQuery qf("UPDATE Feature SET sequence = ?1 WHERE id = ?2" , db, os);
    qf.bindDataId( 1, seqId );
    qf.bindDataId( 2, featureId );
    qf.execute( );
}

void SQLiteFeatureDbi::updateKeyValue(const U2DataId& featureId, const U2FeatureKey& key, U2OpStatus& os){
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );

    SQLiteQuery q("UPDATE FeatureKey SET value = ?3 WHERE feature = ?1 AND name = ?2" , db, os);
    q.bindDataId(1, featureId);
    q.bindString(2, key.name);
    q.bindString(3, key.value);
    q.execute();
}

bool SQLiteFeatureDbi::getKeyValue( const U2DataId &featureId, U2FeatureKey &key, U2OpStatus &os ) {
    DBI_TYPE_CHECK( featureId, U2Type::Feature, os, false );

    static const QString queryString( "SELECT value FROM FeatureKey WHERE feature = ?1 AND name = ?2" );
    SQLiteQuery q( queryString, db, os );
    CHECK_OP( os, false );

    q.bindDataId( 1, featureId );
    q.bindString( 2, key.name );

    if ( q.step( ) ) {
        key.value = q.getCString( 0 );
        return true;
    } else {
        return false;
    }
}

void SQLiteFeatureDbi::updateLocation(const U2DataId& featureId, const U2FeatureLocation& location, U2OpStatus& os) {
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );

    SQLiteTransaction t( db, os );

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

void SQLiteFeatureDbi::removeFeature(const U2DataId& featureId, U2OpStatus& os) {
    DBI_TYPE_CHECK(featureId, U2Type::Feature, os, );

    SQLiteTransaction t( db, os );

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

U2DbiIterator<U2Feature>* SQLiteFeatureDbi::getFeaturesByRegion( const U2Region& reg,
    const U2DataId& parentId, const QString& featureName, const U2DataId& seqId, U2OpStatus& os,
    bool contains )
{
    SQLiteTransaction t( db, os );

    const QString queryStringk = "SELECT " + FDBI_FIELDS + " FROM Feature AS f INNER JOIN "
        "FeatureLocationRTreeIndex AS fr ON f.id = fr.id AND "
        + ( parentId.isEmpty( ) ? "" : "f.parent = ?3 AND " )
        + ( contains ? "fr.start >= ?1 AND fr.end <= ?2" : "fr.start < ?2 AND fr.end > ?1" );
    QSharedPointer<SQLiteQuery> q = t.getPreparedQuery( queryStringk, db, os );

    q->bindInt64( 1, reg.startPos );
    q->bindInt64( 2, reg.endPos( ) );
    if ( !parentId.isEmpty( ) ) {
        q->bindDataId( 3, parentId );
    }

    CHECK_OP( os, NULL );
    return new SqlRSIterator<U2Feature>( q, new SqlFeatureRSLoader( ),
        new SqlFeatureFilter( featureName, seqId ), U2Feature( ), os );
}

U2DbiIterator<U2Feature> * SQLiteFeatureDbi::getFeaturesBySequence( const QString &featureName,
    const U2DataId &seqId, U2OpStatus &os )
{
    SQLiteTransaction t( db, os );
    static const QString queryStringk( "SELECT " + FDBI_FIELDS + " FROM Feature AS f "
        "WHERE f.sequence = ?1 and f.name = ?2 ORDER BY f.start" );
    QSharedPointer<SQLiteQuery> q =  t.getPreparedQuery( queryStringk, db, os );

    q->bindDataId( 1, seqId );
    q->bindString( 2, featureName );
    CHECK_OP( os, NULL );
    return new SqlRSIterator<U2Feature>( q, new SqlFeatureRSLoader( ), NULL, U2Feature( ), os );
}

U2DbiIterator<U2Feature> * SQLiteFeatureDbi::getSubFeatures( const U2DataId &parentId,
    const QString &featureName, const U2DataId &seqId, U2OpStatus &os )
{
    SQLiteTransaction t( db, os );
    static const QString queryStringk( "SELECT " + FDBI_FIELDS + " FROM Feature AS f "
        "WHERE f.parent = ?1" );
    QSharedPointer<SQLiteQuery> q =  t.getPreparedQuery( queryStringk, db, os );

    q->bindDataId( 1, parentId );
    CHECK_OP( os, NULL );
    return new SqlRSIterator<U2Feature>( q, new SqlFeatureRSLoader( ),
        new SqlFeatureFilter( featureName, seqId ), U2Feature( ), os );
}

} //namespace
