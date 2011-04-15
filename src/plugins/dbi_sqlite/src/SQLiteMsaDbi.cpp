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

#include "SQLiteMsaDbi.h"

#include <U2Core/U2SqlHelpers.h>

namespace U2 {

SQLiteMsaRDbi::SQLiteMsaRDbi(SQLiteDbi* dbi) : U2SimpleMsaDbi(dbi), SQLiteChildDBICommon(dbi) {
}

void SQLiteMsaRDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }
    // msa object
    SQLiteQuery(" CREATE TABLE Msa (object INTEGER, length INTEGER NOT NULL, alphabet TEXT NOT NULL, sequenceCount INTEGER NOT NULL, "
        " FOREIGN KEY(object) REFERENCES Object(id) )", db, os).execute();

    // msa object row
    // msa      - msa object id
    // sequence - sequence object id
    // pos      - positional number of row in msa
    // gstart   - offset of the first non-gap sequence element. Same as gstart of the first MsaRowGap
    // gend     - offset of the last non-gap sequence element. Same as gend of the last MsaRowGap
    SQLiteQuery("CREATE TABLE MsaRow (msa INTEGER, sequence INTEGER, pos INTEGER NOT NULL, gstart INTEGER NOT NULL, gend INTEGER NOT NULL, "
        " FOREIGN KEY(msa) REFERENCES Msa(object), "
        " FOREIGN KEY(sequence) REFERENCES Msa(object) )", db, os).execute();

    // gap info for msa row: 
    //  gstart  - global (in msa) start of non-gap region
    //  gend    - global (in msa) end of non-gap region
    //  sstart  - local (in sequence) start of non-gap region
    //  send    - local (in sequence) end of non-gap region
    // Note! there is invariant: gend - gstart == send - sstart
    SQLiteQuery("CREATE TABLE MsaRowGap (msa INTEGER, sequence INTEGER, gstart INTEGER NOT NULL, gend INTEGER NOT NULL, "
        " sstart INTEGER NOT NULL, send INTEGER NOT NULL, "
        " FOREIGN KEY(msa) REFERENCES MsaRow(msa), "
        " FOREIGN KEY(sequence) REFERENCES MsaRow(sequence) )", db, os).execute();
}

U2Msa SQLiteMsaRDbi::getMsaObject(const U2DataId& msaId, U2OpStatus& os) {
    U2Msa res(msaId, dbi->getDbiId(), 0);
    SQLiteQuery q("SELECT Msa.alphabet, Object.version FROM Msa, Object WHERE Object.id = ?1 AND Msa.object = Object.id", db, os);
    q.bindDataId(1, msaId);
    if (q.step())  {
        res.alphabet = q.getString(1);
        res.version = q.getInt64(2);
        q.ensureDone();
    } 
    return res;
}


qint64 SQLiteMsaRDbi::getSequencesCount(const U2DataId& msaId, U2OpStatus& os) {
    SQLiteQuery q("SELECT COUNT(*) FROM MsaRow WHERE msa = ?1", db, os);
    q.bindDataId(1, msaId);
    return q.selectInt64();
}

QList<U2MsaRow> SQLiteMsaRDbi::getRows(const U2DataId& msaId, qint32 firstRow, qint32 numRows, U2OpStatus& os) {
    QList<U2MsaRow> res;
    SQLiteQuery q("SELECT sequence FROM MsaRow WHERE msa = ?1 AND pos >= ?2 AND pos < ?3", db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, firstRow);
    q.bindInt64(3, firstRow + numRows);

    SQLiteQuery gapQ("SELECT sstart, send, FROM MsaRowGap WHERE msa = ?1 AND sequence = ?2 ORDER BY sstart", db, os);
    while (q.step()) {
        U2MsaRow row;
        row.sequenceId = q.getDataId(0, U2Type::Sequence);
        gapQ.reset();
        gapQ.bindDataId(1, msaId);
        gapQ.bindDataId(2, row.sequenceId);
        U2MsaGap prev;
        while (gapQ.step()) {
            U2MsaGap gap;
            gap.offset = gapQ.getInt64(0);
            gap.gap = gapQ.getInt64(1) - gap.offset;
            row.gaps.append(gap);
            assert(prev.offset + prev.gap < gap.offset); //gaps are ordered correctly and do not overlap
            prev = gap;
        }
        if (os.hasError()) {
            break;
        }
        res.append(row);
    }
    return res;
}

qint32 SQLiteMsaRDbi::countSequencesAt(const U2DataId& msaId, qint64 coord, U2OpStatus& os) {
    SQLiteQuery q("SELECT COUNT(*) FROM MsaRow WHERE msa = ?1 AND gstart <= ?2 AND gend > ?2", db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, coord);
    return (qint32)q.selectInt64();
}

QList<U2DataId> SQLiteMsaRDbi::getSequencesAt(const U2DataId& msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os) {
    SQLiteQuery q("SELECT sequenceId FROM MsaRow WHERE msa = ?1 AND gstart <= ?2 AND gend > ?2", offset, count, db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, coord);
    return q.selectDataIds(U2Type::Sequence);
}

qint32 SQLiteMsaRDbi::countSequencesAt(const U2DataId& msaId, const U2Region& r, U2OpStatus& os) {
    SQLiteQuery q("SELECT COUNT(*) FROM MsaRow WHERE msa = ?1 AND ((gstart <= ?2 AND gend > ?2) OR (gstart <= ?3 AND gend > ?3)", db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, r.startPos);
    q.bindInt64(3, r.endPos());
    return (qint32)q.selectInt64();
}

QList<U2DataId> SQLiteMsaRDbi::getSequencesAt(const U2DataId& msaId, const U2Region& r, qint32 offset, qint32 count, U2OpStatus& os) {
    SQLiteQuery q("SELECT sequenceId FROM MsaRow WHERE msa = ?1 AND ((gstart <= ?2 AND gend > ?2) OR (gstart <= ?3 AND gend > ?3)", 
        offset, count, db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, r.startPos);
    q.bindInt64(3, r.endPos());
    return q.selectDataIds(U2Type::Sequence);
}

qint32 SQLiteMsaRDbi::countSequencesWithoutGapAt(const U2DataId& msaId, qint64 coord, U2OpStatus& os) {
    SQLiteQuery q("SELECT COUNT(*) FROM MsaRowGap WHERE msa = ?1 AND gstart <= ?2 AND gend > ?2", db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, coord);
    return (qint32)q.selectInt64();
}
    
QList<U2DataId> SQLiteMsaRDbi::getSequencesWithoutGapAt(const U2DataId& msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os) {
    SQLiteQuery q("SELECT sequenceId FROM MsaRowGap WHERE msa = ?1 AND gstart <= ?2 AND gend > ?2", offset, count, db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, coord);
    return q.selectDataIds(U2Type::Sequence);
}

} //namespace
