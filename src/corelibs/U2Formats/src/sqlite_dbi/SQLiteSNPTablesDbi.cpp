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

#include "SQLiteSNPTablesDbi.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2SafePoints.h>

namespace U2{

/*static QString toInnerColumn(int column) {
    QString res;
    switch(column) {
    case 0  : res = "publicId";  break;
    case 1  : res = "startPos"; break;
    case 2  : res = "refData";  break;
    case 3  : res = "obsData"; break;
    case 4  : res = "seqName";  break;
    default : res = "publicId"; break;
    }
    return res;
}*/

/*static QString toSqlOrderOp(bool sortAscending) {
    QString res;

    if(sortAscending){
        res = "ASC";
    }else{
        res = "DESC";
    }

    return res;
}*/

static QString toSortingChrName(const QString& seqName) {
    QString res = seqName;

    //chr1, chr2, ..., chr9
    if(seqName.length() == 4){
        QChar endChar = seqName.at(3);
        if (endChar.isDigit()){
            res = "chr0"+QString(endChar);
        }
    }
    return res;
}

/*static QString toOriginalChrName(const QString& seqName) {
    QString res = seqName;
    QString endChar = seqName.right(1);
    //chr01, chr02, ..., chr09
    if(seqName.length() == 5 && seqName.mid(3,1) == "0"){
        res = "chr"+endChar;
    }
    return res;
}*/

SQLiteSNPTablesDbi::SQLiteSNPTablesDbi(SQLiteDbi* rootDbi): SNPTablesDbi(rootDbi), SQLiteChildDBICommon(rootDbi){}

void SQLiteSNPTablesDbi::initSqlSchema(U2OpStatus& os){
    if (os.hasError()) {
        return;
    }

    // Damage Effect for U2Variant
    // geneName - affected gene
    // effectValue - double value of the effect
    // scores in different dbs
    SQLiteQuery("CREATE TABLE DamageEffect(variant INTEGER, "
        " geneName TEXT, "
        " effectValue DOUBLE,"
        " avSift  DOUBLE, lrt DOUBLE, phylop DOUBLE, pp2 DOUBLE, mt DOUBLE, genomes1000 DOUBLE, segmentalDuplication INTEGER, conserved INTEGER, "
        " gerpConserved DOUBLE, allFreq DOUBLE, hapmap DOUBLE, gerpScore DOUBLE, "
        " inDbSNP INTEGER, discardFilter TEXT, coding INTEGER, frameshift INTEGER, "
        " FOREIGN KEY(variant) REFERENCES Variant(id) )", db, os).execute();

    // Table to connect filter tables with filter names
    // id - FilterTable ID
    // filterName - name of the filter
    SQLiteQuery("CREATE TABLE FilterTableNames (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
        " filterName TEXT NOT NULL )", db, os).execute();


    // Table stores not annotated variations indexes
    SQLiteQuery("CREATE TABLE NotAnnotatedVariations (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
        " variant INTEGER NOT NULL UNIQUE)", db, os).execute();

}
void SQLiteSNPTablesDbi::createDamageEffect( DamageEffect& effect, U2OpStatus& os ){
    if (effect.variant.isEmpty()) {
        os.setError(SQLiteL10N::tr("Variant is not set!"));
        return;
    }

    DBI_TYPE_CHECK(effect.variant, U2Type::VariantType, os,);

    SQLiteTransaction t(db, os);
    static QString queryString("INSERT INTO DamageEffect(variant, geneName, effectValue, avSift, lrt, phylop, pp2, mt, genomes1000, segmentalDuplication, conserved, "
        " gerpConserved, allFreq, hapmap, gerpScore, inDbSNP, discardFilter, coding, frameshift) "
        " VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18, ?19)");

    QSharedPointer<SQLiteQuery> q1 = t.getPreparedQuery(queryString, db, os); 
    q1->bindDataId(1, effect.variant);
    q1->bindString(2, effect.affectedGeneName);
    q1->bindDouble(3, effect.siftEffectValue);
    q1->bindDouble(4, effect.avSift);
    q1->bindDouble(5, effect.ljb_lrt);
    q1->bindDouble(6, effect.ljb_phylop);
    q1->bindDouble(7, effect.ljb_pp2);
    q1->bindDouble(8, effect.ljb_mt);
    q1->bindDouble(9, effect.genomes1000);
    q1->bindInt32(10, effect.segmentalDuplication ? 1 : 0);
    q1->bindInt32(11, effect.conserved ? 1 : 0);
    q1->bindDouble(12, effect.gerpConcerved);
    q1->bindDouble(13, effect.allFreq);
    q1->bindDouble(14, effect.hapmap);
    q1->bindDouble(15, effect.gerpScore);
    q1->bindInt32(16, effect.inDbSNP ? 1 : 0);
    q1->bindString(17, effect.discardFilter);
    q1->bindInt32(18, effect.coding ? 1 : 0);
    q1->bindInt32(19, effect.frameshift ? 1 : 0);
    q1->insert();
    SAFE_POINT_OP(os,); 
}

void SQLiteSNPTablesDbi::createDamageEffectIndex( U2OpStatus& os ){
    SQLiteQuery("CREATE INDEX IF NOT EXISTS VarinatIndex ON DamageEffect(variant)" ,db, os).execute();
}


void SQLiteSNPTablesDbi::removeDamageEffect( const DamageEffect& effect, U2OpStatus& os ){
    SQLiteQuery q("DELETE FROM DamageEffect WHERE variant = ?1 AND geneName = ?2", db, os);
    q.bindDataId(1, effect.variant);
    q.bindString(2, effect.affectedGeneName);
    q.execute();
    SAFE_POINT_OP(os,);
}

void SQLiteSNPTablesDbi::removeAllDamageEffectForVariant(const U2Variant& variant, U2OpStatus& os){
    SQLiteQuery q("DELETE FROM DamageEffect WHERE variant = ?1", db, os);
    q.bindDataId(1, variant.id);
    q.execute();
    SAFE_POINT_OP(os,);
}

class SimpleDamageEffectLoader : public SqlRSLoader<DamageEffect> {
    DamageEffect load(SQLiteQuery* q) {
        DamageEffect de;
        de.variant = q->getDataId(0, U2Type::VariantType);
        de.affectedGeneName = q->getCString(1);
        de.siftEffectValue = q->getDouble(2);

        de.avSift = q->getDouble(3);
        de.ljb_lrt = q->getDouble(4);
        de.ljb_phylop = q->getDouble(5);
        de.ljb_pp2 = q->getDouble(6);
        de.ljb_mt = q->getDouble(7);
        de.genomes1000 = q->getDouble(8);
        de.segmentalDuplication = (q->getInt32(9) == 0) ? false : true;
        de.conserved = (q->getInt32(10) == 0) ? false : true;
        de.gerpConcerved = q->getDouble(11);
        de.allFreq = q->getDouble(12);
        de.hapmap = q->getDouble(13);
        de.gerpScore = q->getDouble(14);
        

        de.inDbSNP = (q->getInt32(15) == 0 ? false : true);

        de.discardFilter = q->getString(16);
        de.coding = (q->getInt32(17) == 0) ? false : true;
        de.frameshift = (q->getInt32(18) == 0) ? false : true;

        return de;
    }
};

U2DbiIterator<DamageEffect>* SQLiteSNPTablesDbi::getDamageEffectsForVariant( const U2DataId& variantId, U2OpStatus& os ){
    SQLiteTransaction t(db, os);

    QSharedPointer<SQLiteQuery> q = t.getPreparedQuery("SELECT variant, geneName, effectValue, avSift, lrt, phylop, pp2, mt, genomes1000, segmentalDuplication, conserved, "
        "gerpConserved, allFreq, hapmap, gerpScore, inDbSNP, discardFilter, coding, frameshift "
        " FROM DamageEffect WHERE variant = ?1", db, os);
    q->bindDataId(1, variantId);

    return new SqlRSIterator<DamageEffect>(q, new SimpleDamageEffectLoader(), NULL, DamageEffect(), os); 
}

class SimpleRegulatoryEffectLoader : public SqlRSLoader<RegulatoryEffect> {
    RegulatoryEffect load(SQLiteQuery* q) {
        RegulatoryEffect re;
        re.variant = q->getDataId(0, U2Type::VariantType);
        re.affectedGeneId = q->getDataId(1, U2Type::Feature);
        re.fromGeneStartPos = q->getInt64(2);

        return re;
    }
};


void SQLiteSNPTablesDbi::removeRegulatoryEffect(const RegulatoryEffect& effect, U2OpStatus& os){
    SQLiteQuery q("DELETE FROM RegulatoryEffect WHERE variant = ?1 AND geneId = ?2", db, os);
    q.bindDataId(1, effect.variant);
    q.bindDataId(2, effect.affectedGeneId);
    q.execute();
    SAFE_POINT_OP(os,);
}

void SQLiteSNPTablesDbi::removeAllRegulatoryEffectForVariant(const U2Variant& variant, U2OpStatus& os){
    SQLiteQuery q("DELETE FROM RegulatoryEffect WHERE variant = ?1", db, os);
    q.bindDataId(1, variant.id);
    q.execute();
    SAFE_POINT_OP(os,);
}

    U2DbiIterator<RegulatoryEffect>* SQLiteSNPTablesDbi::getRegulatoryEffectsForVariant(const U2DataId& variant, U2OpStatus& os){
    SQLiteTransaction t(db, os);

    QSharedPointer<SQLiteQuery> q = t.getPreparedQuery("SELECT variant, geneId, promoterPos "
        " FROM RegulatoryEffect WHERE variant = ?1", db, os);
    q->bindDataId(1, variant);

    return new SqlRSIterator<RegulatoryEffect>(q, new SimpleRegulatoryEffectLoader(), NULL, RegulatoryEffect(), os); 
}

/*
void SQLiteSNPTablesDbi::createFilterTable( FilterTable& table, const QString& filterName, U2OpStatus& os )
{
    if (filterName.isEmpty()) {
        os.setError(SQLiteL10N::tr("Filter name is not set!"));
        return;
    }

    //SQLiteTransaction t(db, os);

    SQLiteQuery q1("INSERT INTO FilterTableNames(filterName) VALUES(?1)", db, os);
    q1.bindBlob(1, filterName.toAscii());
    table.id = q1.insert(U2Type::FilterTableType);
    table.filter = filterName.toAscii();
    SAFE_POINT_OP(os,);

    QString filterTableString = QString("table_%1").arg(U2DbiUtils::toDbiId(table.id));

    // Cache table to store filtered results
    // filterID - FilterTableNames ID
    // track ID - VariantTrack ID
    // variant ID - U2Variant ID
    // startPos - U2Variant.startPos
    // refData  - U2Variant.refData
    // obsData - U2Variant.obsData
    // publicId - U2Variant.publicId
    // seqName - sequence name of VariantTrack
    SQLiteQuery(QString("CREATE TABLE FilterTable_%1 ( "
        " trackID INT NOT NULL, "
        " trackType INT NOT NULL, "
        " variantID INT NOT NULL, "
        " startPos INTEGER, endPos INTEGER, "
        " refData BLOB NOT NULL, obsData BLOB NOT NULL, publicId TEXT NOT NULL, "
        " seqName TEXT)").arg(filterTableString) ,db, os).execute();
}

void SQLiteSNPTablesDbi::removeFilterTable( const FilterTable& table, U2OpStatus& os )
{
    
    QString filterTableString = QString("table_%1").arg(U2DbiUtils::toDbiId(table.id));
    SQLiteQuery q1(QString("DROP TABLE IF EXISTS FilterTable_%1").arg(filterTableString), db, os);
    q1.execute();
    SAFE_POINT_OP(os,);

    SQLiteQuery q2("DELETE FROM FilterTableNames WHERE id = ?1", db, os);
    q2.bindDataId(1, table.id);
    q2.execute();
    SAFE_POINT_OP(os,);
}
*/
void SQLiteSNPTablesDbi::renameFilterTable (const U2DataId& fTable, const QString& newName, U2OpStatus& os)
{
    if (newName.isEmpty()) {
        os.setError(SQLiteL10N::tr("New filter name is not set!"));
        return;
    }

    SQLiteQuery q1("UPDATE FilterTableNames SET filterName=?1 WHERE id=?2", db, os);
    q1.bindBlob(1, newName.toAscii());
    q1.bindDataId(2, fTable);
    q1.update();
    SAFE_POINT_OP(os,);
}

void SQLiteSNPTablesDbi::addVariantsToTable( const U2DataId& fTable, const U2DataId& track, VariantTrackType tType, const QString& visualSeqName, U2DbiIterator<U2Variant>* it, U2OpStatus& os )
{
    QString filterTableString = QString("table_%1").arg(U2DbiUtils::toDbiId(fTable));

    //transform seqName
    QString seqName = toSortingChrName(visualSeqName);


    SQLiteQuery q2(QString("INSERT INTO FilterTable_%1(trackID, trackType, variantID, startPos, endPos, refData, obsData, publicId, seqName) VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9)").arg(filterTableString), db, os);
    while (it->hasNext() && !os.isCoR()) {
        U2Variant var = it->next();
        q2.reset();
        q2.bindDataId(1, track);
        q2.bindInt32(2, (int)tType);
        q2.bindDataId(3, var.id);
        q2.bindInt64(4, var.startPos);
        q2.bindInt64(5, var.endPos);
        q2.bindBlob(6, var.refData);
        q2.bindBlob(7, var.obsData);
        q2.bindString(8, var.publicId);
        q2.bindString(9, seqName);
        q2.execute();
        SAFE_POINT_OP(os,);
    }

    
}
/*
U2::FilterTable SQLiteSNPTablesDbi::getFilterTableByName( const QString& filterName, U2OpStatus& os )
{
    SQLiteQuery q("SELECT id, filterName FROM FilterTableNames WHERE filterName = ?1", db, os);
    q.bindBlob(1, filterName.toAscii() );

    CHECK_OP(os, FilterTable());

    FilterTable ft;
    if (q.step()){
        ft.id = q.getDataId(0, U2Type::FilterTableType);
        ft.filter = q.getBlob(1);
    }
    return ft;
}

class SimpleFilteredVariantLoader: public SqlRSLoader<FilterTableItem> {
public:
    FilterTableItem load(SQLiteQuery* q) {
        FilterTableItem item;
        
        item.variantTrackId = q->getDataId(0, U2Type::VariantTrack);
        item.trackType = (VariantTrackType)(q->getInt32(1));

        U2Variant res;
        res.id = q->getDataId(2, U2Type::VariantType);
        res.startPos = q->getInt64(3);
        res.endPos =  q->getInt64(4);
        res.refData = q->getBlob(5);
        res.obsData = q->getBlob(6);
        res.publicId = q->getString(7);

        //transform seqName
        item.seqName = toOriginalChrName(q->getString(8));

        item.variant = res;

        return item;
    }
};


U2DbiIterator<FilterTableItem>* SQLiteSNPTablesDbi::getVariantsRange( const U2DataId& fTable, VariantTrackType tType, int offset, int limit, int sortColumn, bool sortAscending, U2OpStatus& os ){
    QString filterTableString = QString("table_%1").arg(U2DbiUtils::toDbiId(fTable));
    QString queryS;
    if (TrackType_All == tType){
        queryS = QString("SELECT trackID, trackType, variantID, startPos, endPos, refData, obsData, publicId, seqName FROM FilterTable_%1 \
                ORDER BY ").arg(filterTableString) + toInnerColumn(sortColumn) + "  " + toSqlOrderOp(sortAscending)+ " LIMIT ?1 OFFSET ?2";
    }else{
        queryS = QString("SELECT trackID, trackType, variantID, startPos, endPos, refData, obsData, publicId, seqName FROM FilterTable_%1 \
                WHERE trackType = ?3 ORDER BY ").arg(filterTableString) + toInnerColumn(sortColumn) + "  " + toSqlOrderOp(sortAscending)+ " LIMIT ?1 OFFSET ?2";
    }

    QSharedPointer<SQLiteQuery> q (new SQLiteQuery(queryS , db, os));
    q->bindInt64(1, limit);
    q->bindInt64(2, offset);

    if (tType != TrackType_All){
        q->bindInt32(3, tType);
    }
    
    return new SqlRSIterator<FilterTableItem>(q, new SimpleFilteredVariantLoader(), NULL, FilterTableItem(), os);
}
*/
int SQLiteSNPTablesDbi::getVariantCount( const U2DataId& fTable, VariantTrackType tType, U2OpStatus& os ){
    QString filterTableString = QString("table_%1").arg(U2DbiUtils::toDbiId(fTable));
    QString queryS;

    if (tType == TrackType_All){
        queryS = QString("SELECT COUNT(*) FROM FilterTable_%1 ").arg(filterTableString);
    }else{
        queryS = QString("SELECT COUNT(*) FROM FilterTable_%1 WHERE trackType = ?1").arg(filterTableString);
    }
    SQLiteQuery q(queryS , db, os);
    if (tType != TrackType_All){
        q.bindInt32(1, tType);
    }
    if (!q.step()) {
        return -1;
    }

    return q.getInt32(0); 
}



void SQLiteSNPTablesDbi::markNotAnnotated( const U2DataId& variant, U2OpStatus& os ){
    SQLiteTransaction t(db, os);
    static QString queryString("INSERT INTO NotAnnotatedVariations(variant) VALUES(?1)");

    QSharedPointer<SQLiteQuery> q1 = t.getPreparedQuery(queryString, db, os); 
    q1->bindDataId(1, variant);
    q1->insert();
    SAFE_POINT_OP(os,); 
}

void SQLiteSNPTablesDbi::markAnnotated( const U2DataId& variant, U2OpStatus& os ){
    SQLiteTransaction t(db, os);
    static QString queryString("DELETE FROM NotAnnotatedVariations WHERE variant = ?1");

    QSharedPointer<SQLiteQuery> q1 = t.getPreparedQuery(queryString, db, os); 

    q1->bindDataId(1, variant);
    q1->execute();
    SAFE_POINT_OP(os,);
}

int SQLiteSNPTablesDbi::checkNotAnnotated( const U2DataId& variant, U2OpStatus& os ){
    SQLiteTransaction t(db, os);
    static QString queryString("SELECT COUNT(*) FROM NotAnnotatedVariations WHERE variant = ?1");

    QSharedPointer<SQLiteQuery> q1 = t.getPreparedQuery(queryString, db, os); 

    q1->bindDataId(1, variant);
    if (!q1->step()) {
        return 1;
    }

    return q1->getInt32(0); 
}

void SQLiteSNPTablesDbi::createAnnotationsMarkerIndex( U2OpStatus& os ){
    SQLiteQuery("CREATE INDEX IF NOT EXISTS AnnotatedVariantsIndex ON NotAnnotatedVariations(variant)" ,db, os).execute();
}

void SQLiteSNPTablesDbi::createIndexForFilterTable( const U2DataId& fTable, U2OpStatus& os ){
    QString filterTableString = QString("table_%1").arg(U2DbiUtils::toDbiId(fTable));

    //for sorting by column
    SQLiteQuery(QString("CREATE INDEX IF NOT EXISTS FilterTableid1_%1 ON FilterTable_%1(startPos)").arg(filterTableString) ,db, os).execute();
    os.setProgress(os.getProgress()+1);
    SQLiteQuery(QString("CREATE INDEX IF NOT EXISTS FilterTableid2_%1 ON FilterTable_%1(refData)").arg(filterTableString) ,db, os).execute();
    os.setProgress(os.getProgress()+1);
    SQLiteQuery(QString("CREATE INDEX IF NOT EXISTS FilterTableid3_%1 ON FilterTable_%1(obsData)").arg(filterTableString) ,db, os).execute();
    os.setProgress(os.getProgress()+1);
    SQLiteQuery(QString("CREATE INDEX IF NOT EXISTS FilterTableid4_%1 ON FilterTable_%1(publicId)").arg(filterTableString) ,db, os).execute();
    os.setProgress(os.getProgress()+1);
    SQLiteQuery(QString("CREATE INDEX IF NOT EXISTS FilterTableid5_%1 ON FilterTable_%1(seqName)").arg(filterTableString) ,db, os).execute();
    os.setProgress(os.getProgress()+1);
    SQLiteQuery(QString("CREATE INDEX IF NOT EXISTS FilterTableid6_%1 ON FilterTable_%1(variantID)").arg(filterTableString) ,db, os).execute();

    
}

void SQLiteSNPTablesDbi::updateVariantFilterTable( const U2DataId& fTable, const U2DataId& varId, VariantTrackType tType, U2OpStatus& os ){
    QString filterTableString = QString("table_%1").arg(U2DbiUtils::toDbiId(fTable));
    QString q1 = QString("UPDATE FilterTable_%1 ").arg(filterTableString);

    SQLiteTransaction t(db, os);
    QString queryS = q1 + QString("SET trackType=?2 WHERE variantID=?1");
    QSharedPointer<SQLiteQuery> q = t.getPreparedQuery(queryS, db, os);

    q->bindDataId(1, varId);
    q->bindInt32(2, tType);

    q->execute();
    SAFE_POINT_OP(os,);
}


}//namespace
