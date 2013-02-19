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

#include "SQLiteMsaDbi.h"
#include "SQLiteObjectDbi.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

const QByteArray SQLiteMsaDbi::CURRENT_MOD_DETAILS_VERSION = QByteArray::number(0) + QByteArray("&");

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
     //   length   - sequence and gaps length (trailing gap are not taken into account)
    SQLiteQuery("CREATE TABLE MsaRow (msa INTEGER NOT NULL, rowId INTEGER NOT NULL, sequence INTEGER NOT NULL,"
        " pos INTEGER NOT NULL, gstart INTEGER NOT NULL, gend INTEGER NOT NULL, length INTEGER NOT NULL,"
        " FOREIGN KEY(msa) REFERENCES Msa(object), "
        " FOREIGN KEY(sequence) REFERENCES Sequence(object) )", db, os).execute();
    SQLiteQuery("CREATE INDEX MsaRow_msa_rowId ON MsaRow(msa, rowId)", db, os).execute();
    SQLiteQuery("CREATE INDEX MsaRow_length ON MsaRow(length)", db, os).execute();

     // Gap info for a MSA row: 
     //   msa       - msa object id
     //   rowId     - id of the row in the msa
     //   gapStart  - start of the gap, the coordinate is relative to the gstart coordinate of the row
     //   gapEnd    - end of the gap, the coordinate is relative to the gstart coordinate of the row (non-inclusive)
     // Note! there is invariant: gend - gstart (of the row) == gapEnd - gapStart
    SQLiteQuery("CREATE TABLE MsaRowGap (msa INTEGER NOT NULL, rowId INTEGER NOT NULL, gapStart INTEGER NOT NULL, gapEnd INTEGER NOT NULL, "
        "FOREIGN KEY(rowId) REFERENCES MsaRow(rowId) )", db, os).execute();
    SQLiteQuery("CREATE INDEX MsaRowGap_msa_rowId ON MsaRowGap(msa, rowId)", db, os).execute();
}

U2DataId SQLiteMsaDbi::createMsaObject(const QString& folder, const QString& name, const U2AlphabetId& alphabet, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    U2Msa msa;
    msa.visualName = name;
    msa.alphabet = alphabet;

    // Create the object
    dbi->getSQLiteObjectDbi()->createObject(msa, folder, SQLiteDbiObjectRank_TopLevel, os);
    CHECK_OP(os, U2DataId());

    // Create a record in the Msa table
    SQLiteQuery q("INSERT INTO Msa(object, length, alphabet, numOfRows) VALUES(?1, ?2, ?3, ?4)", db, os);
    CHECK_OP(os, U2DataId());

    q.bindDataId(1, msa.id);
    q.bindInt64(2, 0); // length = 0
    q.bindString(3, msa.alphabet.id);
    q.bindInt64(4, 0); // no rows
    q.insert();

    return msa.id;
}

void SQLiteMsaDbi::updateMsaName(const U2DataId& msaId, const QString& name, U2OpStatus& os) {
    ModTrackAction updateAction(dbi, msaId);
    updateAction.prepareTracking(os);
    CHECK_OP(os, );

    // Update the name
    U2Object msaObj;
    dbi->getSQLiteObjectDbi()->getObject(msaObj, msaId, os);
    CHECK_OP(os, );

    QString oldName = msaObj.visualName;
    msaObj.visualName = name;
    dbi->getSQLiteObjectDbi()->updateObject(msaObj, os); // increments the version also
    CHECK_OP(os, );

    // Track the modification
    QByteArray modDetails = dbi->getSQLiteObjectDbi()->getModDetailsForUpdateObjectName(oldName, name);
    updateAction.saveTrack(U2ModType::objUpdatedName, modDetails, os);
    CHECK_OP(os, );
}

void SQLiteMsaDbi::updateMsaAlphabet(const U2DataId& msaId, const U2AlphabetId& alphabet, U2OpStatus& os) {
    ModTrackAction updateAction(dbi, msaId);
    U2TrackModType trackMod = updateAction.prepareTracking(os);
    CHECK_OP(os, );

    // Get modDetails, if required
    QByteArray modDetails;
    if (TrackOnUpdate == trackMod) {
        U2Msa msaObj = getMsaObject(msaId, os);
        CHECK_OP(os, );
        U2AlphabetId oldAlphabet = msaObj.alphabet;

        modDetails = CURRENT_MOD_DETAILS_VERSION +
            oldAlphabet.id.toLatin1() + "&" +
            alphabet.id.toLatin1();

    }

    // Update the alphabet
    SQLiteQuery q("UPDATE Msa SET alphabet = ?1 WHERE object = ?2", db, os);
    CHECK_OP(os, );

    q.bindString(1, alphabet.id);
    q.bindDataId(2, msaId);
    q.update(1);

    // Increment the alignment version
    SQLiteObjectDbi::incrementVersion(msaId, db, os);
    CHECK_OP(os, );

    // Track the modification, if required
    updateAction.saveTrack(U2ModType::msaUpdatedAlphabet, modDetails, os);
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
    // Calculate the row length
    qint64 rowLength = calculateRowLength(msaRow.gend - msaRow.gstart, msaRow.gaps);

    // Insert the data
    SQLiteQuery q("INSERT INTO MsaRow(msa, rowId, sequence, pos, gstart, gend, length)"
        " VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7)", db, os);
    CHECK_OP(os, );

    q.bindDataId(1, msaId);
    q.bindInt64(2, msaRow.rowId);
    q.bindDataId(3, msaRow.sequenceId);
    q.bindInt64(4, posInMsa);
    q.bindInt64(5, msaRow.gstart);
    q.bindInt64(6, msaRow.gend);
    q.bindInt64(7, rowLength);
    q.insert();
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
    U2TrackModType trackMod = dbi->getObjectDbi()->getTrackModType(msaId, os);
    CHECK_OP(os, );

    // Remember version for the case when modifications tracking is required
    qint64 version = -1; // Use only for modification tracking!
    if (TrackOnUpdate == trackMod) {
        version = dbi->getObjectDbi()->getObjectVersion(msaId, os);
        CHECK_OP(os, );
    }

    // Append the row to the end, if "-1"
    qint64 numOfRows = getNumOfRows(msaId, os);
    CHECK_OP(os, );

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

    // Update the alignment length
    row.length = calculateRowLength(row.gend - row.gstart, row.gaps);
    qint64 msaLength = getMsaLength(msaId, os);
    if (row.length > msaLength) {
        updateMsaLength(msaId, row.length, os);
    }

    // Re-calculate position, if needed
    if (posInMsa != numOfRows) {
        rowsOrder.insert(posInMsa, row.rowId);
        setNewRowsOrder(msaId, rowsOrder, os);
        CHECK_OP(os, );
    }

    // Update the number of rows of the MSA
    numOfRows++;
    updateNumOfRows(msaId, numOfRows, os);

    // Increment the alignment version
    SQLiteObjectDbi::incrementVersion(msaId, db, os);
    CHECK_OP(os, );

    // Track the modification
    if (TrackOnUpdate == trackMod) {
        U2ModStep modStep;
        modStep.objectId = msaId;
        modStep.version = version;
        modStep.modType = U2ModType::msaAddedRow;
        modStep.details = CURRENT_MOD_DETAILS_VERSION +
            QByteArray("rowId=") + QByteArray::number(row.rowId);
        dbi->getModDbi()->createModStep(modStep, os);
    }
}

void SQLiteMsaDbi::addRows(const U2DataId& msaId, QList<U2MsaRow>& rows, U2OpStatus& os) {
    U2TrackModType trackMod = dbi->getObjectDbi()->getTrackModType(msaId, os);
    CHECK_OP(os, );

    // Remember version for the case when modifications tracking is required
    qint64 version = -1; // Use only for modification tracking!
    if (TrackOnUpdate == trackMod) {
        version = dbi->getObjectDbi()->getObjectVersion(msaId, os);
        CHECK_OP(os, );
    }

    // Add the rows
    qint64 numOfRows = getNumOfRows(msaId, os);
    CHECK_OP(os, );

    qint64 numOfRowsAdded = rows.count();
    for (int i = 0; i < numOfRowsAdded; ++i) {
        addMsaRowAndGaps(msaId, -1, rows[i], os);
        CHECK_OP(os, );
    }

    // Update the alignment length
    qint64 maxRowLength = 0;
    foreach (const U2MsaRow& row, rows) {
        qint64 rowLength = calculateRowLength(row.gend - row.gstart, row.gaps);
        maxRowLength = qMax(rowLength, maxRowLength);
    }
    
    qint64 msaLength = getMsaLength(msaId, os);
    if (maxRowLength > msaLength) {
        updateMsaLength(msaId, maxRowLength, os);
    }

    // Update the number of rows
    numOfRows += numOfRowsAdded;
    updateNumOfRows(msaId, numOfRows, os);

    // Increment the alignment version
    SQLiteObjectDbi::incrementVersion(msaId, db, os);
    CHECK_OP(os, );

    // Track the modification
    if (TrackOnUpdate == trackMod) {
        // Get the list of rows IDs
        QByteArray rowsIds;
        for (int i = 0, n = rows.count(); i < n; ++i) {
            qint64 rowId = rows[i].rowId;
            rowsIds += QByteArray::number(rowId);
            if (i != n - 1) {
                rowsIds += "&";
            }
        }

        // Save the data
        U2ModStep modStep;
        modStep.objectId = msaId;
        modStep.version = version;
        modStep.modType = U2ModType::msaAddedRows;
        modStep.details = CURRENT_MOD_DETAILS_VERSION + rowsIds;
        dbi->getModDbi()->createModStep(modStep, os);
    }
}

void SQLiteMsaDbi::updateRowName(const U2DataId& msaId, qint64 rowId, const QString& newName, U2OpStatus& os) {
    U2DataId sequenceId = getSequenceIdByRowId(msaId, rowId, os);
    CHECK_OP(os, );

    U2Sequence seqObject = dbi->getSequenceDbi()->getSequenceObject(sequenceId, os);
    CHECK_OP(os, );

    if (newName != seqObject.visualName) {
        U2TrackModType trackMod = dbi->getObjectDbi()->getTrackModType(msaId, os);
        CHECK_OP(os, );

        // Remember version for the case when modifications tracking is required
        qint64 version = -1; // Use only for modification tracking!
        QString oldRowName = seqObject.visualName;
        if (TrackOnUpdate == trackMod) {
            version = dbi->getObjectDbi()->getObjectVersion(msaId, os);
            CHECK_OP(os, );
        }

        // Update the data
        seqObject.visualName = newName;
        dbi->getSequenceDbi()->updateSequenceObject(seqObject, os);

        // Increment the alignment version
        SQLiteObjectDbi::incrementVersion(msaId, db, os);
        CHECK_OP(os, );

        //// Track the modification
        //if (TrackOnUpdate == trackMod) {
        //    dbi->getSQLiteObjectDbi()->addModStepUpdateObjectName(sequenceId, version, oldRowName, os);
        //}
    }
}

void SQLiteMsaDbi::updateRowContent(const U2DataId& msaId, qint64 rowId, const QByteArray& seqBytes, const QList<U2MsaGap>& gaps, U2OpStatus& os) {
    U2TrackModType trackMod = dbi->getObjectDbi()->getTrackModType(msaId, os);
    CHECK_OP(os, );

    SQLiteTransaction t(db, os);

    // Get the row object
    U2MsaRow row = getRow(msaId, rowId, os);
    CHECK_OP(os, );

    // Remember version for the case when modifications tracking is required
    qint64 version = -1; // Use only for modification tracking!
    QByteArray modDetails;
    if (TrackOnUpdate == trackMod) {
        version = dbi->getObjectDbi()->getObjectVersion(msaId, os);
        CHECK_OP(os, );

        QByteArray oldSeqBytes = dbi->getSequenceDbi()->getSequenceData(row.sequenceId, U2_REGION_MAX, os);
        CHECK_OP(os, );

        QByteArray gapsInfo;
        for (int i = 0, n = row.gaps.count(); i < n; ++i) {
            const U2MsaGap& gap = row.gaps[i];
            gapsInfo += "offset=";
            gapsInfo += QByteArray::number(gap.offset);
            gapsInfo += "&gap=";
            gapsInfo += QByteArray::number(gap.gap);

            if (i > 0 && i < n - 1) {
                gapsInfo += "&";
            }
        }

        modDetails += QByteArray("rowId=");
        modDetails += QByteArray::number(rowId);
        modDetails += QByteArray("&oldSeqBytes=");
        modDetails += oldSeqBytes;
        modDetails += QByteArray("&oldGaps=\"");
        modDetails += gapsInfo;
        modDetails += "\"";
    }

    // Update the sequence data
    QVariantMap hints;
    dbi->getSequenceDbi()->updateSequenceData(row.sequenceId, U2_REGION_MAX, seqBytes, hints, os);

    // Update the row
    qint64 seqLength = seqBytes.length();
    row.gstart = 0;
    row.gend = seqLength;
    row.length = calculateRowLength(seqLength, gaps);

    updateRecordFromMsaRow(msaId, row, os);
    CHECK_OP(os,  );

    // Update the gaps
    removeRecordsFromMsaRowGap(msaId, rowId, os);
    CHECK_OP(os, );

    foreach (const U2MsaGap& gap, gaps) {
        createMsaRowGap(msaId, rowId, gap, os);
        CHECK_OP(os, );
    }

    // Update the row length (without trailing gaps)
    qint64 rowSequenceLength = getRowSequenceLength(msaId, rowId, os);
    CHECK_OP(os, );

    qint64 newRowLength = calculateRowLength(rowSequenceLength, gaps);
    updateRowLength(msaId, rowId, newRowLength, os);
    CHECK_OP(os, );

    // Re-calculate the alignment length
    recalculateMsaLength(msaId, os);

    // Increment the alignment version
    SQLiteObjectDbi::incrementVersion(msaId, db, os);
    CHECK_OP(os, );

    // Track the modification
    if (TrackOnUpdate == trackMod) {
        U2ModStep modStep;
        modStep.objectId = msaId;
        modStep.version = version;
        modStep.modType = U2ModType::msaUpdatedRowContent;
        modStep.details = CURRENT_MOD_DETAILS_VERSION + modDetails;
        dbi->getModDbi()->createModStep(modStep, os);
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
    U2TrackModType trackMod = dbi->getObjectDbi()->getTrackModType(msaId, os);
    CHECK_OP(os, );

    // Remember version for the case when modifications tracking is required
    qint64 version = -1; // Use only for modification tracking!
    QByteArray oldRowsOrderDetails("oldRowsOrder=");
    if (TrackOnUpdate == trackMod) {
        version = dbi->getObjectDbi()->getObjectVersion(msaId, os);
        CHECK_OP(os, );

        QList<qint64> rowsOrder = getRowsOrder(msaId, os);
        CHECK_OP(os, );

        for (int i = 0, n = rowsOrder.count(); i < n; ++i) {
            oldRowsOrderDetails += QByteArray::number(rowsOrder[i]);

            if (i != n - 1) {
                oldRowsOrderDetails += "&";
            }
        }
    }

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

    // Increment the alignment version
    SQLiteObjectDbi::incrementVersion(msaId, db, os);
    CHECK_OP(os, );

    // Track the modification
    if (TrackOnUpdate == trackMod) {
        U2ModStep modStep;
        modStep.objectId = msaId;
        modStep.version = version;
        modStep.modType = U2ModType::msaSetNewRowsOrder;
        modStep.details = CURRENT_MOD_DETAILS_VERSION + oldRowsOrderDetails;
        dbi->getModDbi()->createModStep(modStep, os);
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

void SQLiteMsaDbi::removeRow(const U2DataId& msaId, qint64 rowId, U2OpStatus& os) {
    U2TrackModType trackMod = dbi->getObjectDbi()->getTrackModType(msaId, os);
    CHECK_OP(os, );

    bool removeSequence = true; // the row sequence is removed if it has no other parents

    // Remember version for the case when modifications tracking is required
    qint64 version = -1; // Use only for modification tracking!
    U2MsaRow removedRow;
    if (TrackOnUpdate == trackMod) {
        version = dbi->getObjectDbi()->getObjectVersion(msaId, os);
        CHECK_OP(os, );

        removedRow = getRow(msaId, rowId, os);
        CHECK_OP(os, );

        removeSequence = false; // the sequence is not removed as user can undo row removing
    }

    // Get and verify the number of rows
    qint64 numOfRows = getNumOfRows(msaId, os);
    CHECK_OP(os, );
    SAFE_POINT(numOfRows > 0, "Empty MSA!", );

    // Remove the row
    removeMsaRowAndGaps(msaId, rowId, removeSequence, os);

    // Update the number of rows
    numOfRows--;
    updateNumOfRows(msaId, numOfRows, os);

    // Re-calculate the rows positions
    recalculateRowsPositions(msaId, os);

    // Increment the alignment version
    SQLiteObjectDbi::incrementVersion(msaId, db, os);
    CHECK_OP(os, );

    // Track the modification
    if (TrackOnUpdate == trackMod) {
        U2ModStep modStep;
        modStep.objectId = msaId;
        modStep.version = version;
        modStep.modType = U2ModType::msaRemovedRow;
        modStep.details = CURRENT_MOD_DETAILS_VERSION + getRemovedRowDetails(removedRow);

        dbi->getModDbi()->createModStep(modStep, os);
    }
}

void SQLiteMsaDbi::removeRows(const U2DataId& msaId, const QList<qint64>& rowIds, U2OpStatus& os) {
    U2TrackModType trackMod = dbi->getObjectDbi()->getTrackModType(msaId, os);
    CHECK_OP(os, );

    qint64 numOfRowsToRemove = rowIds.count();

    bool removeSequence = true; // the row sequence is removed if it has no other parents

    // Remember version for the case when modifications tracking is required
    qint64 version = -1; // Use only for modification tracking!
    QByteArray removedRowsDetails;
    
    if (TrackOnUpdate == trackMod) {
        version = dbi->getObjectDbi()->getObjectVersion(msaId, os);
        CHECK_OP(os, );

        removeSequence = false; // the sequence is not removed as user can undo row removing

        // Get details about removed rows
        for (int i = 0, rowsNum = rowIds.count(); i < rowsNum; ++i) {
            qint64 rowId = rowIds[i];
            U2MsaRow removedRow = getRow(msaId, rowId, os);
            CHECK_OP(os, );

            removedRowsDetails += getRemovedRowDetails(removedRow);

            if (i != rowsNum - 1) {
                removedRowsDetails += "&";
            }
        }
    }

    // Get and verify the number of rows
    qint64 numOfRows = getNumOfRows(msaId, os);
    CHECK_OP(os, );
    SAFE_POINT(numOfRows >= numOfRowsToRemove , "Incorrect rows to remove!", );

    // Remove the rows
    for (int i = 0; i < rowIds.count(); ++i) {
        removeMsaRowAndGaps(msaId, rowIds[i], removeSequence, os);
        CHECK_OP(os, );
    }

    // Update the number of rows
    numOfRows -= numOfRowsToRemove;
    updateNumOfRows(msaId, numOfRows, os);

    // Re-calculate the rows positions
    recalculateRowsPositions(msaId, os);

    // Increment the alignment version
    SQLiteObjectDbi::incrementVersion(msaId, db, os);
    CHECK_OP(os, );

    // Track the modification
    if (TrackOnUpdate == trackMod) {
        U2ModStep modStep;
        modStep.objectId = msaId;
        modStep.version = version;
        modStep.modType = U2ModType::msaRemovedRows;
        modStep.details = CURRENT_MOD_DETAILS_VERSION + removedRowsDetails;

        dbi->getModDbi()->createModStep(modStep, os);
    }
}

void SQLiteMsaDbi::removeMsaRowAndGaps(const U2DataId& msaId, qint64 rowId, bool removeSequence, U2OpStatus& os) {    
    U2DataId sequenceId = getSequenceIdByRowId(msaId, rowId, os);
    CHECK_OP(os, );

    removeRecordsFromMsaRowGap(msaId, rowId, os);
    removeRecordFromMsaRow(msaId, rowId, os);

    dbi->getSQLiteObjectDbi()->removeParent(msaId, sequenceId, removeSequence, os);
}

void SQLiteMsaDbi::removeAllRows(const U2DataId& msaId, U2OpStatus& os) {
    QList<U2MsaRow> rows = getRows(msaId, os);
    CHECK_OP(os, );

    // Remove the rows
    for (int i = 0; i < rows.count(); ++i) {
        removeMsaRowAndGaps(msaId, rows[i].rowId, true, os);
        CHECK_OP(os, );
    }

    // Set number of rows and length to 0
    updateMsaLength(msaId, 0, os);
    CHECK_OP(os, );

    updateNumOfRows(msaId, 0, os);
    CHECK_OP(os, );

    // Increment the alignment version
    SQLiteObjectDbi::incrementVersion(msaId, db, os);
    CHECK_OP(os, );
}

U2Msa SQLiteMsaDbi::getMsaObject(const U2DataId& msaId, U2OpStatus& os) {
    U2Msa res;
    dbi->getSQLiteObjectDbi()->getObject(res, msaId, os);

    SAFE_POINT_OP(os, res);

    SQLiteQuery q("SELECT length, alphabet FROM Msa WHERE object = ?1", db, os);
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
    SQLiteQuery q("SELECT numOfRows FROM Msa WHERE object = ?1", db, os);
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
    SQLiteQuery q("SELECT rowId, sequence, gstart, gend, length FROM MsaRow WHERE msa = ?1 ORDER BY pos", db, os);
    q.bindDataId(1, msaId);

    SQLiteQuery gapQ("SELECT gapStart, gapEnd FROM MsaRowGap WHERE msa = ?1 AND rowId = ?2 ORDER BY gapStart", db, os);
    while (q.step()) {
        U2MsaRow row;
        row.rowId = q.getInt64(0);
        row.sequenceId = q.getDataId(1, U2Type::Sequence);
        row.gstart = q.getInt64(2);
        row.gend = q.getInt64(3);
        row.length = q.getInt64(4);

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
    SQLiteQuery q("SELECT sequence, gstart, gend, length FROM MsaRow WHERE msa = ?1 AND rowId = ?2", db, os);
    SAFE_POINT_OP(os, res);

    q.bindDataId(1, msaId);
    q.bindInt64(2, rowId);
    if (q.step()) {
        res.rowId = rowId;
        res.sequenceId = q.getDataId(0, U2Type::Sequence);
        res.gstart = q.getInt64(1);
        res.gend = q.getInt64(2);
        res.length = q.getInt64(3);
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

void SQLiteMsaDbi::updateGapModel(const U2DataId& msaId, qint64 msaRowId, const QList<U2MsaGap>& gapModel, U2OpStatus& os) {
    U2TrackModType trackMod = dbi->getObjectDbi()->getTrackModType(msaId, os);
    CHECK_OP(os, );

    // Remember version for the case when modifications tracking is required
    qint64 version = -1; // Use only for modification tracking!
    QByteArray removedGapsDetails = "&oldGaps=";

    if (TrackOnUpdate == trackMod) {
        version = dbi->getObjectDbi()->getObjectVersion(msaId, os);
        CHECK_OP(os, );

        U2MsaRow oldRow = getRow(msaId, msaRowId, os);
        CHECK_OP(os, );

        QList<U2MsaGap> oldGaps = oldRow.gaps;

        // Get details about removed rows
        removedGapsDetails += "\"";
        for (int i = 0, gapsNum = oldGaps.count(); i < gapsNum; ++i) {
            const U2MsaGap& gap = oldGaps[i];

            removedGapsDetails += "offset=";
            removedGapsDetails += QByteArray::number(gap.offset);
            removedGapsDetails += "&gap=";
            removedGapsDetails += QByteArray::number(gap.gap);

            if (i != gapsNum - 1) {
                removedGapsDetails += "&";
            }
        }
        removedGapsDetails += "\"";
    }

    // Remove obsolete gaps of the row
    removeRecordsFromMsaRowGap(msaId, msaRowId, os);
    CHECK_OP(os, );

    // Store the new gap model
    foreach (const U2MsaGap& gap, gapModel) {
        createMsaRowGap(msaId, msaRowId, gap, os);
        CHECK_OP(os, );
    }

    // Update the row length (without trailing gaps)
    qint64 rowSequenceLength = getRowSequenceLength(msaId, msaRowId, os);
    CHECK_OP(os, );

    qint64 newRowLength = calculateRowLength(rowSequenceLength, gapModel);
    updateRowLength(msaId, msaRowId, newRowLength, os);
    CHECK_OP(os, );

    // Re-calculate the alignment length
    recalculateMsaLength(msaId, os);

    // Increment the alignment version
    SQLiteObjectDbi::incrementVersion(msaId, db, os);
    CHECK_OP(os, );

    // Track the modification
    if (TrackOnUpdate == trackMod) {
        U2ModStep modStep;
        modStep.objectId = msaId;
        modStep.version = version;
        modStep.modType = U2ModType::msaUpdatedGapModel;
        modStep.details = CURRENT_MOD_DETAILS_VERSION +
            QByteArray("rowId=") + QByteArray::number(msaRowId) +
            removedGapsDetails;

        dbi->getModDbi()->createModStep(modStep, os);
    }
}

qint64 SQLiteMsaDbi::getMsaLength(const U2DataId& msaId, U2OpStatus& os) {
    qint64 res = 0;
    SQLiteQuery q("SELECT length FROM Msa WHERE object = ?1", db, os);
    CHECK_OP(os, res);

    q.bindDataId(1, msaId);
    if (q.step()) {
        res = q.getInt64(0);
        q.ensureDone();
    }
    else if (!os.hasError()) {
        os.setError(SQLiteL10N::tr("Msa object not found!"));
    }

    return res;
}

void SQLiteMsaDbi::recalculateMsaLength(const U2DataId& msaId, U2OpStatus& os) {
    qint64 msaLength = 0;

    // Get maximum row length
    SQLiteQuery q("SELECT MAX(length) FROM MsaRow WHERE msa = ?1", db, os);
    CHECK_OP(os, );

    q.bindDataId(1, msaId);
    if (q.step()) {
        msaLength = q.getInt64(0);
        q.ensureDone();
    }

    // Update the MSA length
    updateMsaLength(msaId, msaLength, os);
}

qint64 SQLiteMsaDbi::calculateRowLength(qint64 seqLength, const QList<U2MsaGap>& gaps) {
    qint64 res = seqLength;
    foreach (const U2MsaGap& gap, gaps) {
        if (gap.offset < res) { // ignore trailing gaps
            res += gap.gap;
        }
    }
    return res;
}

qint64 SQLiteMsaDbi::getRowSequenceLength(const U2DataId& msaId, qint64 rowId, U2OpStatus& os) {
    qint64 res = 0;
    SQLiteQuery q("SELECT gstart, gend FROM MsaRow WHERE msa = ?1 AND rowId = ?2", db, os);
    CHECK_OP(os, res);

    q.bindDataId(1, msaId);
    q.bindInt64(2, rowId);
    if (q.step()) {
        qint64 startInSeq = q.getInt64(0);
        qint64 endInSeq = q.getInt64(1);
        res = endInSeq - startInSeq;
        q.ensureDone();
    }
    else if (!os.hasError()) {
        os.setError(SQLiteL10N::tr("Msa row not found!"));
    }

    return res;
}

void SQLiteMsaDbi::updateRowLength(const U2DataId& msaId, qint64 rowId, qint64 newLength, U2OpStatus& os) {
    SQLiteQuery q("UPDATE MsaRow SET length = ?1 WHERE msa = ?2 AND rowId = ?3", db, os);
    CHECK_OP(os, );

    q.bindInt64(1, newLength);
    q.bindDataId(2, msaId);
    q.bindInt64(3, rowId);
    q.update(1);
}

U2DataId SQLiteMsaDbi::getSequenceIdByRowId(const U2DataId& msaId, qint64 rowId, U2OpStatus& os) {
    U2DataId res;
    SQLiteQuery q("SELECT sequence FROM MsaRow WHERE msa = ?1 AND rowId = ?2", db, os);
    CHECK_OP(os, res);

    q.bindDataId(1, msaId);
    q.bindInt64(2, rowId);
    if (q.step()) {
        res = q.getDataId(0, U2Type::Sequence);
        q.ensureDone();
    }
    else if (!os.hasError()) {
        os.setError(SQLiteL10N::tr("Msa row not found!"));
    }

    return res;
}

QByteArray SQLiteMsaDbi::getRemovedRowDetails(const U2MsaRow& row) {
    QByteArray res;

    // Info about gaps
    QByteArray gapsInfo;
    for (int i = 0, n = row.gaps.count(); i < n; ++i) {
        const U2MsaGap& gap = row.gaps[i];
        gapsInfo += "offset=";
        gapsInfo += QByteArray::number(gap.offset);
        gapsInfo += "&gap=";
        gapsInfo += QByteArray::number(gap.gap);

        if (i > 0 && i < n - 1) {
            gapsInfo += "&";
        }
    }

    res = QByteArray("rowId=") + QByteArray::number(row.rowId) +
        QByteArray("&sequenceId=") + row.sequenceId.toHex() +
        QByteArray("&gstart=") + QByteArray::number(row.gstart) +
        QByteArray("&gend=") + QByteArray::number(row.gend) +
        QByteArray("&gaps=\"") + gapsInfo + QByteArray("\"") +
        QByteArray("&length=") + QByteArray::number(row.length);

    return res;
}

void SQLiteMsaDbi::updateRecordFromMsaRow(const U2DataId& msaId, const U2MsaRow& row, U2OpStatus& os) {
    SQLiteQuery q("UPDATE MsaRow SET sequence = ?1, gstart = ?2, gend = ?3 WHERE msa = ?4 AND rowId = ?5", db, os);
    CHECK_OP(os, );

    q.bindDataId(1, row.sequenceId);
    q.bindInt64(2, row.gstart);
    q.bindInt64(3, row.gend);
    q.bindDataId(4, msaId);
    q.bindInt64(5, row.rowId);
    q.update(1);
}

void SQLiteMsaDbi::undo(const U2DataId& msaId, qint64 modType, const QByteArray& modDetails, U2OpStatus& os) {
    switch (modType) {
        case U2ModType::msaUpdatedAlphabet:
            undoUpdateMsaAlphabet(msaId, modDetails, os);
            break;
        case U2ModType::msaAddedRows:
            undoAddRows(msaId, modDetails, os);
            break;
        case U2ModType::msaAddedRow:
            undoAddRow(msaId, modDetails, os);
            break;
        case U2ModType::msaRemovedRows:
            undoRemoveRows(msaId, modDetails, os);
            break;
        case U2ModType::msaRemovedRow:
            undoRemoveRow(msaId, modDetails, os);
            break;
        case U2ModType::msaUpdatedRowContent:
            undoUpdateRowContent(msaId, modDetails, os);
            break;
        case U2ModType::msaUpdatedGapModel:
            undoUpdateGapModel(msaId, modDetails, os);
            break;
        case U2ModType::msaSetNewRowsOrder:
            undoSetNewRowsOrder(msaId, modDetails, os);
            break;
        default:
            os.setError(QString("Unexpected modification type '%1'!").arg(QString::number(modType)));
            return;
    }
}

void SQLiteMsaDbi::redo(const U2DataId& msaId, qint64 modType, const QByteArray& modDetails, U2OpStatus& os) {
    switch(modType) {
        case U2ModType::msaUpdatedAlphabet:
            redoUpdateMsaAlphabet(msaId, modDetails, os);
            break;
        case U2ModType::msaAddedRows:
            redoAddRows(msaId, modDetails, os);
            break;
        case U2ModType::msaAddedRow:
            redoAddRow(msaId, modDetails, os);
            break;
        case U2ModType::msaRemovedRows:
            redoRemoveRows(msaId, modDetails, os);
            break;
        case U2ModType::msaRemovedRow:
            redoRemoveRow(msaId, modDetails, os);
            break;
        case U2ModType::msaUpdatedRowContent:
            redoUpdateRowContent(msaId, modDetails, os);
            break;
        case U2ModType::msaUpdatedGapModel:
            redoUpdateGapModel(msaId, modDetails, os);
            break;
        case U2ModType::msaSetNewRowsOrder:
            redoSetNewRowsOrder(msaId, modDetails, os);
            break;
        default:
            os.setError(QString("Unexpected modification type '%1'!").arg(QString::number(modType)));
            return;
    }
}

bool SQLiteMsaDbi::parseUpdateMsaAlphabetDetails(const QByteArray& modDetails, U2AlphabetId& oldAlphabet, U2AlphabetId& newAlphabet) {
    QList<QByteArray> modDetailsParts = modDetails.split('&');
    SAFE_POINT(3 == modDetailsParts.count(), QString("Invalid modDetails '%1'!").arg(QString(modDetails)), false);
    SAFE_POINT(QByteArray("0") == modDetailsParts[0], QString("Invalid modDetails version '%1'").arg(QString(modDetailsParts[0])), false);

    oldAlphabet = QString(modDetailsParts[1]);
    newAlphabet = QString(modDetailsParts[2]);
    
    if (!oldAlphabet.isValid() || !newAlphabet.isValid()) {
        return false;
    }

    return true;
}

void SQLiteMsaDbi::undoUpdateMsaAlphabet(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    U2AlphabetId oldAlphabet;
    U2AlphabetId newAlphabet;
    bool ok = parseUpdateMsaAlphabetDetails(modDetails, oldAlphabet, newAlphabet);
    if (!ok) {
        os.setError("An error occurred during updating an alignment alphabet!");
        return;
    }

    // Update the value
    SQLiteQuery q("UPDATE Msa SET alphabet = ?1 WHERE object = ?2", db, os);
    CHECK_OP(os, );

    q.bindString(1, oldAlphabet.id);
    q.bindDataId(2, msaId);
    q.update(1);
}

void SQLiteMsaDbi::redoUpdateMsaAlphabet(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    U2AlphabetId oldAlphabet;
    U2AlphabetId newAlphabet;
    bool ok = parseUpdateMsaAlphabetDetails(modDetails, oldAlphabet, newAlphabet);
    if (!ok) {
        os.setError("An error occurred during updating an alignment alphabet!");
        return;
    }

    // Redo the updating
    updateMsaAlphabet(msaId, newAlphabet, os);
}

void SQLiteMsaDbi::undoAddRows(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::redoAddRows(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::undoAddRow(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::redoAddRow(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::undoRemoveRows(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::redoRemoveRows(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::undoRemoveRow(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::redoRemoveRow(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::undoUpdateRowContent(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::redoUpdateRowContent(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::undoUpdateGapModel(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::redoUpdateGapModel(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::undoSetNewRowsOrder(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

void SQLiteMsaDbi::redoSetNewRowsOrder(const U2DataId& msaId, const QByteArray& modDetails, U2OpStatus& os) {
    // TODO
}

} //namespace
