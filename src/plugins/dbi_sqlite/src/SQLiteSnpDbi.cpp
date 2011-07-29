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

#include "SQLiteSnpDbi.h"
#include "SQLiteObjectDbi.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

SQLiteSnpDbi::SQLiteSnpDbi(SQLiteDbi* dbi) : U2SnpDbi(dbi), SQLiteChildDBICommon(dbi) {
}

void SQLiteSnpDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }
    // snp track object
    SQLiteQuery(" CREATE TABLE SnpTrack (object INTEGER, sequence INTEGER NOT NULL, "
        " FOREIGN KEY(object) REFERENCES Object(id), FOREIGN KEY(sequence) REFERENCES Object(id)  )", db, os).execute();

    // snp element
    // track - snp track object id
    // pos   - mutation position
    // oldBase - original base
    // newBase - resulted base
    SQLiteQuery("CREATE TABLE Snp(track INTEGER, pos INTEGER, oldBase INTEGER NOT NULL, newBase INTEGER NOT NULL, gend INTEGER NOT NULL, "
        " FOREIGN KEY(track) REFERENCES SnpTrack(object)", db, os).execute();
    
}

U2SnpTrack SQLiteSnpDbi::getSnpTrack(const U2DataId& snpId, U2OpStatus& os) {
    U2SnpTrack res(snpId, dbi->getDbiId(), 0);
    SQLiteQuery q("SELECT SnpTrack.sequence, Object.version FROM SnpTrack, Object WHERE Object.id = ?1 AND SnpTrack.object = Object.id", db, os);
    q.bindDataId(1, snpId);
    if (q.step())  {
        res.sequence = q.getDataId(1, U2Type::Sequence);
        res.version = q.getInt64(2);
        q.ensureDone();
    } 
    return res;
}

void SQLiteSnpDbi::createSnpTrack(U2SnpTrack& track, U2DbiIterator<U2Snp>* it, const QString& folder, U2OpStatus& os) {
    if (track.sequence.isEmpty()) {
        os.setError(SQLiteL10N::tr("Sequence object is not set!"));
        return;
    }
    
    SQLiteTransaction t(db, os);

    track.id = SQLiteObjectDbi::createObject(U2Type::SnpTrack, folder, track.visualName, SQLiteDbiObjectRank_TopLevel, db, os);
    SAFE_POINT_OP(os,);

    SQLiteQuery q1("INSERT INTO SnpTrack(object, sequence) VALUES(?1, ?2)", db, os);
    q1.bindDataId(1, track.id);
    q1.bindDataId(2, track.sequence);
    q1.execute();
    SAFE_POINT_OP(os,);

    SQLiteQuery q2("INSERT INTO Snp(track, pos, oldBase, newBase) VALUES(?1, ?2, ?3, ?4)", db, os);
    while (it->hasNext() && !os.isCoR()) {
        U2Snp snp = it->next();
        q2.reset();
        q2.bindDataId(1, track.id);
        q2.bindInt64(2, snp.pos);
        q2.bindInt32(3, snp.oldBase);
        q2.bindInt32(3, snp.newBase);
        q2.execute();
    }
}

void SQLiteSnpDbi::updateSnpTrack(const U2SnpTrack& track, U2OpStatus& os) {
    SQLiteQuery q("UPDATE SnpTrack(sequence) SET VALUES(?1) WHERE object = ?2", db, os);
    q.bindString(1, track.sequence);
    q.bindDataId(2, track.id);
    q.execute();
}

class SimpleSnpLoader: public SqlRSLoader<U2Snp> {
public:
    U2Snp load(SQLiteQuery* q) {
        U2Snp res;
        res.pos = q->getInt64(0);
        res.oldBase = (char)q->getInt32(1);
        res.newBase = (char)q->getInt32(2);
        return res;
    }
};

U2DbiIterator<U2Snp>* SQLiteSnpDbi::getSnps(const U2DataId& trackId, const U2Region& region, U2OpStatus& os) {
    SQLiteQuery* q = new SQLiteQuery("SELECT pos, oldBase, newBase FROM Snp WHERE track = ?1", db, os);
    q->bindDataId(1, trackId);
    return new SqlRSIterator<U2Snp>(q, new SimpleSnpLoader(), NULL, U2Snp(), os);
}

} //namespace
