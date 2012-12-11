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

#include "SQLiteMsaDbi.h"
#include "SQLiteObjectDbi.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

SQLiteMsaDbi::SQLiteMsaDbi(SQLiteDbi* dbi) : U2MsaDbi(dbi), SQLiteChildDBICommon(dbi) {
}

void SQLiteMsaDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }

    // MSA object
    SQLiteQuery("CREATE TABLE Msa (object INTEGER UNIQUE, length INTEGER NOT NULL, alphabet TEXT NOT NULL,  sequenceCount INTEGER NOT NULL,"
        " FOREIGN KEY(object) REFERENCES Object(id) )", db, os).execute();

     // MSA object row
     //   msa      - msa object id
     //   rowId    - id of the row in the msa
     //   sequence - sequence object id
     //   pos      - positional number of a row in the msa (initially, equals 'id', but can be changed, e.g. in GUI by moving rows)
     //   gstart   - offset of the first element in the sequence
     //   gend     - offset of the last element in the sequence (non-inclusive)
    SQLiteQuery("CREATE TABLE MsaRow (msa INTEGER NOT NULL, rowId INTEGER NOT NULL, sequence INTEGER NOT NULL,"
        " pos INTEGER NOT NULL, gstart INTEGER NOT NULL, gend INTEGER NOT NULL, "
        " FOREIGN KEY(msa) REFERENCES Msa(object), "
        " FOREIGN KEY(sequence) REFERENCES Sequence(object) )", db, os).execute();

     // Gap info for a MSA row: 
     //   msa       - msa object id
     //   rowId     - id of the row in the msa
     //   gapStart  - start of the gap, the coordinate is relative to the gstart coordinate of the row
     //   gapEnd    - end of the gap, the coordinate is relative to the gstart coordinate of the row (non-inclusive)
     // Note! there is invariant: gend - gstart (of the row) == gapEnd - gapStart
    SQLiteQuery("CREATE TABLE MsaRowGap (msa INTEGER NOT NULL, rowId INTEGER NOT NULL, gapStart INTEGER NOT NULL, gapEnd INTEGER NOT NULL, "
        "FOREIGN KEY(rowId) REFERENCES MsaRow(rowId) )", db, os).execute();
}

void SQLiteMsaDbi::createMsaObject(U2Msa& msa, const QString& folder, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    dbi->getSQLiteObjectDbi()->createObject(msa, folder, SQLiteDbiObjectRank_TopLevel, os);
    CHECK_OP(os, );

    static const QString queryString("INSERT INTO Msa(object, length, alphabet, sequenceCount) VALUES(?1, ?2, ?3, ?4)");
    SQLiteQuery *q = t.getPreparedQuery(queryString, db, os);
    CHECK_OP(os, );

    q->bindDataId(1, msa.id);
    q->bindInt64(2, msa.length);
    q->bindString(3, msa.alphabet.id);
    q->bindInt64(4, msa.numOfRows);
    q->insert();
}

void SQLiteMsaDbi::updateMsaObject(U2Msa& msa, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    static const QString queryString("UPDATE Msa SET length = ?1, alphabet = ?2, sequenceCount = ?3 WHERE object = ?4");
    SQLiteQuery *q = t.getPreparedQuery(queryString, db, os);
    CHECK_OP(os, );

    q->bindInt64(1, msa.length);
    q->bindString(2, msa.alphabet.id);
    q->bindInt64(3, msa.numOfRows);
    q->bindDataId(4, msa.id);
    q->execute();

    SAFE_POINT_OP(os, );

    dbi->getSQLiteObjectDbi()->updateObject(msa, os);
}

void SQLiteMsaDbi::createMsaRow(const U2DataId& msaId, qint64 posInMsa, U2MsaRow& msaRow, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    static const QString queryString("INSERT INTO MsaRow(msa, rowId, sequence, pos, gstart, gend)"
        " VALUES(?1, ?2, ?3, ?4, ?5, ?6)");
    SQLiteQuery *q = t.getPreparedQuery(queryString, db, os);
    CHECK_OP(os, );

    q->bindDataId(1, msaId);
    q->bindInt64(2, posInMsa);
    q->bindDataId(3, msaRow.sequenceId);
    q->bindInt64(4, posInMsa);
    q->bindInt64(5, msaRow.gstart);
    q->bindInt64(6, msaRow.gend);
    q->insert();
}

void SQLiteMsaDbi::createMsaRowGap(const U2DataId& msaId, qint64 msaRowId, const U2MsaGap& msaGap, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    static const QString queryString("INSERT INTO MsaRowGap(msa, rowId, gapStart, gapEnd)"
        " VALUES(?1, ?2, ?3, ?4)");
    SQLiteQuery *q = t.getPreparedQuery(queryString, db, os);
    CHECK_OP(os, );

    q->bindDataId(1, msaId);
    q->bindInt64(2, msaRowId);
    q->bindInt64(3, msaGap.offset);
    q->bindInt64(4, msaGap.offset + msaGap.gap);
    q->insert();
}

void SQLiteMsaDbi::addRow(const U2DataId& msaId, qint64 posInMsa, U2MsaRow& row, U2OpStatus& os) {
    createMsaRow(msaId, posInMsa, row, os);
    CHECK_OP(os, );

    foreach (U2MsaGap gap, row.gaps) {
        createMsaRowGap(msaId, row.rowId, gap, os);
        CHECK_OP(os, );
    }

    dbi->getSQLiteObjectDbi()->ensureParent(msaId, row.sequenceId, os);
}

void SQLiteMsaDbi::addRows(U2Msa& msa, QList<U2MsaRow>& rows, U2OpStatus& os) {
    for (int i = 0; i < rows.count(); ++i) {
        addRow(msa.id, i, rows[i], os);
        CHECK_OP(os, );
    }
    msa.version++;
}

void SQLiteMsaDbi::removeRecordFromMsaRow(const U2DataId& msaId, qint64 rowId, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    static const QString queryString("DELETE FROM MsaRow WHERE msa = ?1 AND rowId = ?2");
    SQLiteQuery *q = t.getPreparedQuery(queryString, db, os);
    CHECK_OP(os, );

    q->bindDataId(1, msaId);
    q->bindInt64(2, rowId);
    q->update(1);
}

void SQLiteMsaDbi::removeRecordsFromMsaRowGap(const U2DataId& msaId, qint64 rowId, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    static const QString queryString("DELETE FROM MsaRowGap WHERE msa = ?1 AND rowId = ?2");
    SQLiteQuery *q = t.getPreparedQuery(queryString, db, os);
    CHECK_OP(os, );

    q->bindDataId(1, msaId);
    q->bindInt64(2, rowId);
    q->update();
}

void SQLiteMsaDbi::removeRow(const U2DataId& msaId, const U2MsaRow& row, U2OpStatus& os) {
    removeRecordsFromMsaRowGap(msaId, row.rowId, os);
    removeRecordFromMsaRow(msaId, row.rowId, os);
    dbi->getSQLiteObjectDbi()->removeParent(msaId, row.sequenceId, true, os);
}

void SQLiteMsaDbi::removeRows(U2Msa& msa, const QList<U2MsaRow>& rows, U2OpStatus& os) {
    for (int i = 0; i < rows.count(); ++i) {
        removeRow(msa.id, rows[i], os);
        CHECK_OP(os, );
    }
    msa.version++;
}

void SQLiteMsaDbi::removeAllRows(const U2DataId& msaId, U2OpStatus& os) {
    QList<U2MsaRow> rows = getRows(msaId, os);
    for (int i = 0; i < rows.count(); ++i) {
        removeRow(msaId, rows[i], os);
        CHECK_OP(os, );
    }
}

U2Msa SQLiteMsaDbi::getMsaObject(const U2DataId& msaId, U2OpStatus& os) {
    U2Msa res;
    dbi->getSQLiteObjectDbi()->getObject(res, msaId, os);

    SAFE_POINT_OP(os, res);

    SQLiteQuery q("SELECT Msa.length, Msa.alphabet, Msa.sequenceCount FROM Msa WHERE Msa.object = ?1", db, os);
    q.bindDataId(1, msaId);
    if (q.step())  {
        res.length = q.getInt64(0);
        res.alphabet = q.getString(1);
        res.numOfRows = q.getInt64(2);
        q.ensureDone();
    } else if (!os.hasError()) {
        os.setError(SQLiteL10N::tr("Msa object not found!"));
    }
    return res;
}


qint64 SQLiteMsaDbi::getSequencesCount(const U2DataId& msaId, U2OpStatus& os) {
    SQLiteQuery q("SELECT COUNT(*) FROM MsaRow WHERE msa = ?1", db, os);
    q.bindDataId(1, msaId);
    return q.selectInt64();
}

QList<U2MsaRow> SQLiteMsaDbi::getRows(const U2DataId& msaId, U2OpStatus& os) {
    QList<U2MsaRow> res;
    SQLiteQuery q("SELECT rowId, sequence, gstart, gend FROM MsaRow WHERE msa = ?1 ORDER BY pos", db, os);
    q.bindDataId(1, msaId);

    SQLiteQuery gapQ("SELECT gapStart, gapEnd FROM MsaRowGap WHERE msa = ?1 AND rowId = ?2 ORDER BY gapStart", db, os);
    while (q.step()) {
        U2MsaRow row;
        row.rowId = q.getInt64(0);
        row.sequenceId = q.getDataId(1, U2Type::Sequence);
        row.gstart = q.getInt64(2);
        row.gend = q.getInt64(3);

        gapQ.reset();
        gapQ.bindDataId(1, msaId);
        gapQ.bindInt64(2, row.rowId);
        while (gapQ.step()) {
            U2MsaGap gap;
            gap.offset = gapQ.getInt64(0);
            gap.gap = gapQ.getInt64(1) - gap.offset;
            row.gaps.append(gap);
        }

        SAFE_POINT_OP(os, res);
        res.append(row);
    }
    return res;
}

qint32 SQLiteMsaDbi::countSequencesAt(const U2DataId& msaId, qint64 coord, U2OpStatus& os) {
    SQLiteQuery q("SELECT COUNT(*) FROM MsaRow WHERE msa = ?1 AND gstart <= ?2 AND gend > ?2", db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, coord);
    return (qint32)q.selectInt64();
}

QList<U2DataId> SQLiteMsaDbi::getSequencesAt(const U2DataId& msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os) {
    SQLiteQuery q("SELECT sequenceId FROM MsaRow WHERE msa = ?1 AND gstart <= ?2 AND gend > ?2", offset, count, db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, coord);
    return q.selectDataIds(U2Type::Sequence);
}

qint32 SQLiteMsaDbi::countSequencesAt(const U2DataId& msaId, const U2Region& r, U2OpStatus& os) {
    SQLiteQuery q("SELECT COUNT(*) FROM MsaRow WHERE msa = ?1 AND ((gstart <= ?2 AND gend > ?2) OR (gstart <= ?3 AND gend > ?3)", db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, r.startPos);
    q.bindInt64(3, r.endPos());
    return (qint32)q.selectInt64();
}

QList<U2DataId> SQLiteMsaDbi::getSequencesAt(const U2DataId& msaId, const U2Region& r, qint32 offset, qint32 count, U2OpStatus& os) {
    SQLiteQuery q("SELECT sequenceId FROM MsaRow WHERE msa = ?1 AND ((gstart <= ?2 AND gend > ?2) OR (gstart <= ?3 AND gend > ?3)", 
        offset, count, db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, r.startPos);
    q.bindInt64(3, r.endPos());
    return q.selectDataIds(U2Type::Sequence);
}

qint32 SQLiteMsaDbi::countSequencesWithoutGapAt(const U2DataId& msaId, qint64 coord, U2OpStatus& os) {
    SQLiteQuery q("SELECT COUNT(*) FROM MsaRowGap WHERE msa = ?1 AND gstart <= ?2 AND gend > ?2", db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, coord);
    return (qint32)q.selectInt64();
}
    
QList<U2DataId> SQLiteMsaDbi::getSequencesWithoutGapAt(const U2DataId& msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os) {
    SQLiteQuery q("SELECT sequenceId FROM MsaRowGap WHERE msa = ?1 AND gstart <= ?2 AND gend > ?2", offset, count, db, os);
    q.bindDataId(1, msaId);
    q.bindInt64(2, coord);
    return q.selectDataIds(U2Type::Sequence);
}

} //namespace
