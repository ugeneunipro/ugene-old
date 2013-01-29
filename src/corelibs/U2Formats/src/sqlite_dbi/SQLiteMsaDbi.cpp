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
    SQLiteQuery("CREATE TABLE Msa (object INTEGER UNIQUE, length INTEGER NOT NULL, alphabet TEXT NOT NULL, numOfRows INTEGER NOT NULL,"
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

    static const QString queryString("INSERT INTO Msa(object, length, alphabet, numOfRows) VALUES(?1, ?2, ?3, ?4)");
    SQLiteQuery *q = t.getPreparedQuery(queryString, db, os);
    CHECK_OP(os, );

    q->bindDataId(1, msa.id);
    q->bindInt64(2, msa.length);
    q->bindString(3, msa.alphabet.id);
    q->bindInt64(4, 0); // no rows
    q->insert();
}

void SQLiteMsaDbi::updateMsaObject(U2Msa& msa, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    static const QString queryString("UPDATE Msa SET length = ?1, alphabet = ?2 WHERE object = ?3");
    SQLiteQuery *q = t.getPreparedQuery(queryString, db, os);
    CHECK_OP(os, );

    q->bindInt64(1, msa.length);
    q->bindString(2, msa.alphabet.id);
    q->bindDataId(3, msa.id);
    q->execute();

    SAFE_POINT_OP(os, );

    dbi->getSQLiteObjectDbi()->updateObject(msa, os);
}

void SQLiteMsaDbi::updateMsaLength(const U2DataId& msaId, qint64 length, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    SQLiteQuery q("UPDATE Msa SET length = ?1 WHERE object = ?2", db, os);
    CHECK_OP(os, );

    q.bindInt64(1, length);
    q.bindDataId(2, msaId);
    q.execute();
}

void SQLiteMsaDbi::createMsaRow(const U2DataId& msaId, qint64 posInMsa, U2MsaRow& msaRow, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    static const QString queryString("INSERT INTO MsaRow(msa, rowId, sequence, pos, gstart, gend)"
        " VALUES(?1, ?2, ?3, ?4, ?5, ?6)");
    SQLiteQuery *q = t.getPreparedQuery(queryString, db, os);
    CHECK_OP(os, );

    q->bindDataId(1, msaId);
    q->bindInt64(2, msaRow.rowId);
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

void SQLiteMsaDbi::addMsaRowAndGaps(const U2DataId& msaId, qint64 posInMsa, U2MsaRow& row, U2OpStatus& os) {
    if (-1 == row.rowId) {
        QList<qint64> currentRowIds = getRowsOrder(msaId, os);
        CHECK_OP(os, );

        foreach (qint64 currentRowId, currentRowIds) {
            row.rowId = qMax(row.rowId, currentRowId);
        }

        row.rowId++;
    }
    createMsaRow(msaId, posInMsa, row, os);
    CHECK_OP(os, );

    foreach (const U2MsaGap& gap, row.gaps) {
        createMsaRowGap(msaId, row.rowId, gap, os);
        CHECK_OP(os, );
    }

    dbi->getSQLiteObjectDbi()->ensureParent(msaId, row.sequenceId, os);
    CHECK_OP(os, );
}

void SQLiteMsaDbi::addRow(const U2DataId& msaId, qint64 posInMsa, U2MsaRow& row, U2OpStatus& os) {
    qint64 numOfRows = getNumOfRows(msaId, os);
    CHECK_OP(os, );

    // Append the row to the end, if "-1"
    QList<qint64> rowsOrder;
    if (-1 == posInMsa) {
        posInMsa = numOfRows;
    }
    else {
        rowsOrder = getRowsOrder(msaId, os);
        CHECK_OP(os, );
        SAFE_POINT(rowsOrder.count() == numOfRows, "Incorrect number of rows!", );
    }
    SAFE_POINT(posInMsa >= 0 && posInMsa <= numOfRows, "Incorrect input position!", );

    // Create the row
    addMsaRowAndGaps(msaId, posInMsa, row, os);
    CHECK_OP(os, );

    // Update the number of rows of the MSA
    numOfRows++;
    updateNumOfRows(msaId, numOfRows, os);

    // Re-calculate position, if needed
    if (posInMsa != numOfRows) {
        rowsOrder.insert(posInMsa, row.rowId);
        setNewRowsOrder(msaId, rowsOrder, os);
        CHECK_OP(os, );
    }
}

void SQLiteMsaDbi::addRows(const U2DataId& msaId, QList<U2MsaRow>& rows, U2OpStatus& os) {
    qint64 numOfRows = getNumOfRows(msaId, os);
    CHECK_OP(os, );

    // Add the rows
    qint64 numOfRowsAdded = rows.count();
    for (int i = 0; i < numOfRowsAdded; ++i) {
        addMsaRowAndGaps(msaId, numOfRows + i, rows[i], os);
        CHECK_OP(os, );
    }

    // Update the number of rows
    numOfRows += numOfRowsAdded;
    updateNumOfRows(msaId, numOfRows, os);
}

void SQLiteMsaDbi::updateRow(const U2DataId& msaId, const U2MsaRow& row, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    SQLiteQuery q("UPDATE MsaRow SET sequence = ?1, gstart = ?2, gend = ?3 WHERE msa = ?4 AND rowId = ?5", db, os);
    CHECK_OP(os, );

    q.bindDataId(1, row.sequenceId);
    q.bindInt64(2, row.gstart);
    q.bindInt64(3, row.gend);
    q.bindDataId(4, msaId);
    q.bindInt64(5, row.rowId);
    q.update(1);

    // Update the gaps
    removeRecordsFromMsaRowGap(msaId, row.rowId, os);
    CHECK_OP(os, );

    foreach (const U2MsaGap& gap, row.gaps) {
        createMsaRowGap(msaId, row.rowId, gap, os);
        CHECK_OP(os, );
    }
}

QList<qint64> SQLiteMsaDbi::getRowsOrder(const U2DataId& msaId, U2OpStatus& os) {
    QList<qint64> res;
    SQLiteQuery q("SELECT rowId FROM MsaRow WHERE msa = ?1 ORDER BY pos", db, os);
    q.bindDataId(1, msaId);
    qint64 rowId;
    while (q.step()) {
        rowId = q.getInt64(0);
        res.append(rowId);
    }
    return res;
}

void SQLiteMsaDbi::setNewRowsOrder(const U2DataId& msaId, const QList<qint64>& rowIds, U2OpStatus& os) {
    // Check that row IDs number is correct (if required, can be later removed for efficiency)
    qint64 numOfRows = getNumOfRows(msaId, os);
    CHECK_OP(os, );
    SAFE_POINT(numOfRows == rowIds.count(), "Incorrect number of row IDs!", );

    // Set the new order
    SQLiteTransaction t(db, os);
    SQLiteQuery q("UPDATE MsaRow SET pos = ?1 WHERE msa = ?2 AND rowId = ?3", db, os);
    CHECK_OP(os, );

    for (int i = 0, n = rowIds.count(); i < n; ++i) {
        qint64 rowId = rowIds[i];
        q.reset();
        q.bindInt64(1, i);
        q.bindDataId(2, msaId);
        q.bindInt64(3, rowId);
        q.execute();
    }
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
    qint64 numOfRows = getNumOfRows(msaId, os);
    CHECK_OP(os, );
    SAFE_POINT(numOfRows > 0, "Empty MSA!", );

    // Remove the row
    removeMsaRowAndGaps(msaId, row, os);

    // Update the number of rows
    numOfRows--;
    updateNumOfRows(msaId, numOfRows, os);

    // Re-calculate the rows positions
    recalculateRowsPositions(msaId, os);
}

void SQLiteMsaDbi::removeRows(const U2DataId& msaId, const QList<U2MsaRow>& rows, U2OpStatus& os) {
    qint64 numOfRowsRemoved = rows.count();

    qint64 numOfRows = getNumOfRows(msaId, os);
    CHECK_OP(os, );
    SAFE_POINT(numOfRows >= numOfRowsRemoved , "Incorrect rows to remove!", );

    // Remove the rows
    for (int i = 0; i < rows.count(); ++i) {
        removeMsaRowAndGaps(msaId, rows[i], os);
        CHECK_OP(os, );
    }

    // Update the number of rows
    numOfRows -= numOfRowsRemoved;
    updateNumOfRows(msaId, numOfRows, os);

    // Re-calculate the rows positions
    recalculateRowsPositions(msaId, os);
}

void SQLiteMsaDbi::removeMsaRowAndGaps(const U2DataId& msaId, const U2MsaRow& row, U2OpStatus& os) {
    removeRecordsFromMsaRowGap(msaId, row.rowId, os);
    removeRecordFromMsaRow(msaId, row.rowId, os);
    dbi->getSQLiteObjectDbi()->removeParent(msaId, row.sequenceId, true, os);
}

void SQLiteMsaDbi::removeAllRows(const U2DataId& msaId, U2OpStatus& os) {
    QList<U2MsaRow> rows = getRows(msaId, os);
    CHECK_OP(os, );
    removeRows(msaId, rows, os);
}

U2Msa SQLiteMsaDbi::getMsaObject(const U2DataId& msaId, U2OpStatus& os) {
    U2Msa res;
    dbi->getSQLiteObjectDbi()->getObject(res, msaId, os);

    SAFE_POINT_OP(os, res);

    SQLiteQuery q("SELECT Msa.length, Msa.alphabet FROM Msa WHERE Msa.object = ?1", db, os);
    q.bindDataId(1, msaId);
    if (q.step())  {
        res.length = q.getInt64(0);
        res.alphabet = q.getString(1);
        q.ensureDone();
    } else if (!os.hasError()) {
        os.setError(SQLiteL10N::tr("Msa object not found!"));
    }
    return res;
}

qint64 SQLiteMsaDbi::getNumOfRows(const U2DataId& msaId, U2OpStatus& os) {
    qint64 res = 0;
    SQLiteQuery q("SELECT Msa.numOfRows FROM Msa WHERE Msa.object = ?1", db, os);
    CHECK_OP(os, res);

    q.bindDataId(1, msaId);
    if (q.step())  {
        res = q.getInt64(0);
        q.ensureDone();
    } else if (!os.hasError()) {
        os.setError(SQLiteL10N::tr("Msa object not found!"));
    }
    return res;
}

void SQLiteMsaDbi::recalculateRowsPositions(const U2DataId& msaId, U2OpStatus& os) {
    QList<U2MsaRow> rows = getRows(msaId, os);
    CHECK_OP(os, );

    SQLiteTransaction t(db, os);
    SQLiteQuery q("UPDATE MsaRow SET pos = ?1 WHERE msa = ?2 AND rowId = ?3", db, os);
    CHECK_OP(os, );

    for (int i = 0, n = rows.count(); i < n; ++i) {
        qint64 rowId = rows[i].rowId;
        q.reset();
        q.bindInt64(1, i);
        q.bindDataId(2, msaId);
        q.bindInt64(3, rowId);
        q.execute();
    }
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

U2MsaRow SQLiteMsaDbi::getRow(const U2DataId& msaId, qint64 rowId, U2OpStatus& os) {
    U2MsaRow res;
    SQLiteQuery q("SELECT sequence, gstart, gend FROM MsaRow WHERE msa = ?1 AND rowId = ?2", db, os);
    SAFE_POINT_OP(os, res);

    q.bindDataId(1, msaId);
    q.bindInt64(2, rowId);
    if (q.step()) {
        res.rowId = rowId;
        res.sequenceId = q.getDataId(0, U2Type::Sequence);
        res.gstart = q.getInt64(1);
        res.gend = q.getInt64(2);
        q.ensureDone();
    } else if (!os.hasError()) {
        os.setError(SQLiteL10N::tr("Msa row not found!"));
    }

    SQLiteQuery gapQ("SELECT gapStart, gapEnd FROM MsaRowGap WHERE msa = ?1 AND rowId = ?2 ORDER BY gapStart", db, os);
    SAFE_POINT_OP(os, res);

    gapQ.bindDataId(1, msaId);
    gapQ.bindInt64(2, rowId);
    while (gapQ.step()) {
        U2MsaGap gap;
        gap.offset = gapQ.getInt64(0);
        gap.gap = gapQ.getInt64(1) - gap.offset;
        res.gaps.append(gap);
    }

    return res;
}

void SQLiteMsaDbi::updateNumOfRows(const U2DataId& msaId, qint64 numOfRows, U2OpStatus& os) {
    SQLiteQuery q("UPDATE Msa SET numOfRows = ?1 WHERE object = ?2", db, os);
    SAFE_POINT_OP(os, );

    q.bindInt64(1, numOfRows);
    q.bindDataId(2, msaId);
    q.update(1);
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

void SQLiteMsaDbi::updateGapModel(const U2DataId& msaId, qint64 msaRowId, const QList<U2MsaGap>& gapModel, U2OpStatus& os) {
    // Remove obsolete gaps of the row
    removeRecordsFromMsaRowGap(msaId, msaRowId, os);
    CHECK_OP(os, );

    // Store the new gap model
    foreach (U2MsaGap gap, gapModel) {
        createMsaRowGap(msaId, msaRowId, gap, os);
        CHECK_OP(os, );
    }
}

} //namespace
