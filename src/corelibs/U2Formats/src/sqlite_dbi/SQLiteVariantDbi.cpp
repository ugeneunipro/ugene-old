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

#include "SQLiteVariantDbi.h"
#include "SQLiteObjectDbi.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

SQLiteVariantDbi::SQLiteVariantDbi(SQLiteDbi* dbi) : U2VariantDbi(dbi), SQLiteChildDBICommon(dbi) {
}

void SQLiteVariantDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }
    // Variant track object
    SQLiteQuery(" CREATE TABLE VariantTrack (object INTEGER, sequence INTEGER NOT NULL, "
        " FOREIGN KEY(object) REFERENCES Object(id), FOREIGN KEY(sequence) REFERENCES Object(id)  )", db, os).execute();

    // Variant element
    // track - Variant track object id
    // startPos - variation start position
    // endPos - variation end position
    // refData - reference sequence part
    // obsData - observed variation of the reference
    // comment - comment visible for user
    // publicId - identifier visible for user
    SQLiteQuery("CREATE TABLE Variant(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, track INTEGER, startPos INTEGER, endPos INTEGER, "
        " refData BLOB NOT NULL, obsData BLOB NOT NULL, publicId TEXT NOT NULL, "
        " FOREIGN KEY(track) REFERENCES VariantTrack(object) )", db, os).execute();
    
}

U2VariantTrack SQLiteVariantDbi::getVariantTrack(const U2DataId& variantId, U2OpStatus& os) {
    U2VariantTrack res;

    DBI_TYPE_CHECK(variantId, U2Type::VariantTrack, os, res);

    dbi->getSQLiteObjectDbi()->getObject(res, variantId, os);
    CHECK_OP(os, res);

    SQLiteQuery q("SELECT sequence FROM VariantTrack WHERE object = ?1", db, os);
    q.bindDataId(1, variantId);
    if (q.step())  {
        res.sequence = q.getDataId(0, U2Type::Sequence);
        q.ensureDone();
    } 
    return res;
}

void SQLiteVariantDbi::addVariantsToTrack(const U2VariantTrack& track, U2DbiIterator<U2Variant>* it, U2OpStatus& os){
    if (track.sequence.isEmpty()) {
        os.setError(SQLiteL10N::tr("Sequence object is not set!"));
        return;
    }

    //SQLiteTransaction t(db, os);

    SQLiteQuery q2("INSERT INTO Variant(track, startPos, endPos, refData, obsData, publicId) VALUES(?1, ?2, ?3, ?4, ?5, ?6)", db, os);
    while (it->hasNext() && !os.isCoR()) {
        U2Variant var = it->next();
        q2.reset();
        q2.bindDataId(1, track.id);
        q2.bindInt64(2, var.startPos);
        q2.bindInt64(3, var.endPos);
        q2.bindBlob(4, var.refData);
        q2.bindBlob(5, var.obsData);
        q2.bindString(6, var.publicId);
        var.id = q2.insert(U2Type::VariantType);
        SAFE_POINT_OP(os,);
    }

}

void SQLiteVariantDbi::createVariantTrack(U2VariantTrack& track, const QString& folder, U2OpStatus& os){
    if (track.sequence.isEmpty()) {
        os.setError(SQLiteL10N::tr("Sequence object is not set!"));
        return;
    }
    
    //SQLiteTransaction t(db, os);

    dbi->getSQLiteObjectDbi()->createObject(track, folder, SQLiteDbiObjectRank_TopLevel, os);
    SAFE_POINT_OP(os,);

    SQLiteQuery q1("INSERT INTO VariantTrack(object, sequence) VALUES(?1, ?2)", db, os);
    q1.bindDataId(1, track.id);
    q1.bindDataId(2, track.sequence);
    q1.execute();
    SAFE_POINT_OP(os,);
}
    
void SQLiteVariantDbi::updateVariantTrack(const U2VariantTrack& track, U2OpStatus& os) {
    SQLiteQuery q("UPDATE VariantTrack(sequence) SET VALUES(?1) WHERE object = ?2", db, os);
    q.bindString(1, track.sequence);
    q.bindDataId(2, track.id);
    q.execute();
}

class SimpleVariantLoader: public SqlRSLoader<U2Variant> {
public:
    U2Variant load(SQLiteQuery* q) {
        U2Variant res;
        res.id = q->getDataId(0, U2Type::VariantType);
        res.startPos = q->getInt64(1);
        res.endPos =  q->getInt64(2);
        res.refData = q->getBlob(3);
        res.obsData = q->getBlob(4);
        res.publicId = q->getString(5);
        return res;
    }
};

U2DbiIterator<U2Variant>* SQLiteVariantDbi::getVariants(const U2DataId& trackId, const U2Region& region, U2OpStatus& os) {
    if (region == U2_REGION_MAX) {
        SQLiteQuery* q = new SQLiteQuery("SELECT id, startPos, endPos, refData, obsData, publicId FROM Variant WHERE track = ?1", db, os);
        q->bindDataId(1, trackId);
        return new SqlRSIterator<U2Variant>(q, new SimpleVariantLoader(), NULL, U2Variant(), os);
    } 
    SQLiteQuery* q = new SQLiteQuery("SELECT id, startPos, endPos, refData, obsData, publicId FROM Variant \
                                     WHERE track = ?1 AND startPos >= ?2 AND startPos <?3", db, os);
    q->bindDataId(1, trackId);
    q->bindInt64(2, region.startPos);
    q->bindInt64(3, region.endPos());
    return new SqlRSIterator<U2Variant>(q, new SimpleVariantLoader(), NULL, U2Variant(), os);
}

class SimpleVariantTrackLoader : public SqlRSLoader<U2VariantTrack> {
    U2VariantTrack load(SQLiteQuery* q) {
        U2VariantTrack track;
        track.id = q->getDataId(0, U2Type::VariantTrack);
        track.sequence = q->getDataId(1,U2Type::Sequence);
        return track;
    }
};

U2DbiIterator<U2VariantTrack>* SQLiteVariantDbi::getVariantTracks( U2OpStatus& os )
{
    SQLiteQuery* q = new SQLiteQuery("SELECT object, sequence FROM VariantTrack", db, os);
    return new SqlRSIterator<U2VariantTrack>(q, new SimpleVariantTrackLoader(), NULL, U2VariantTrack(), os);    
}
U2DbiIterator<U2VariantTrack>* SQLiteVariantDbi::getVariantTracks(const U2DataId& seqId, U2OpStatus& os){
    SQLiteQuery* q = new SQLiteQuery("SELECT object, sequence FROM VariantTrack WHERE sequence = ?1 ", db, os);

    q->bindDataId(1, seqId);

    return new SqlRSIterator<U2VariantTrack>(q, new SimpleVariantTrackLoader(), NULL, U2VariantTrack(), os);    
}


U2DbiIterator<U2Variant>* SQLiteVariantDbi::getVariantsRange(const U2DataId& track, int offset, int limit, U2OpStatus& os )
{
    SQLiteQuery* q = new SQLiteQuery("SELECT id, startPos, endPos, refData, obsData, publicId FROM Variant \
                                     WHERE track = ?1 LIMIT ?2 OFFSET ?3" , db, os);
    q->bindDataId(1, track);
    q->bindInt64(2, limit);
    q->bindInt64(3, offset);
    return new SqlRSIterator<U2Variant>(q, new SimpleVariantLoader(), NULL, U2Variant(), os);
}

int SQLiteVariantDbi::getVariantCount( const U2DataId& trackId, U2OpStatus& os )
{
    SQLiteQuery q("SELECT COUNT(*) FROM Variant WHERE track = ?1 " , db, os);
    q.bindDataId(1, trackId);
    if (!q.step()) {
        return -1;
    }

    return q.getInt32(0); 
}

void SQLiteVariantDbi::removeTrack(const U2DataId& trackId, U2OpStatus& os){
    SQLiteQuery q1("DELETE FROM Variant WHERE track = ?1", db, os);
    q1.bindDataId(1, trackId);
    q1.execute();
    SAFE_POINT_OP(os,);

    SQLiteQuery q2("DELETE FROM VariantTrack WHERE object = ?1", db, os);
    q2.bindDataId(1, trackId);
    q2.execute();
    SAFE_POINT_OP(os,);
}

} //namespace
