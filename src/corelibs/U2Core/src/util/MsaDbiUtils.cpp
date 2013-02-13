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

#include "MsaDbiUtils.h"

#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/DNASequenceUtils.h>
#include <U2Core/MAlignmentExporter.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceDbi.h>


namespace U2 {

/////////////////////////////////////////////////////////////////
// Helper-methods to validate parameters

/** Validates that all 'rowIds' contains in the alignment rows */
bool validateRowIds(const MAlignment& al, const QList<qint64>& rowIds) {
    QList<qint64> alRowIds = al.getRowsIds();
    foreach (qint64 rowId, rowIds) {
        if (!alRowIds.contains(rowId)) {
            coreLog.trace(QString("No row ID '%1' in '%2' alignment!").arg(rowId).arg(al.getName()));
            return false;
        }
    }
    return true;
}

/** Validates 'pos' in an alignment: it must be non-negative and less than or equal to the alignment length */
bool validatePos(const MAlignment& al, qint64 pos) {
    if (pos < 0 || pos > al.getLength()) {
        coreLog.trace(QString("Invalid position '%1' in '%2' alignment!").arg(pos).arg(al.getName()));
        return false;
    }
    return true;
}

/** Validates 'count' of characters - it must be positive */
bool validateCharactersCount(qint64 count) {
    if (count <= 0) {
        coreLog.trace(QString("Invalid value of characters count '%1'!").arg(count));
        return false;
    }
    return true;
}

/** Validates that sequenceId is not empty in the row */
bool validateSequenceId(const U2MsaRow& row) {
    if (row.sequenceId.isEmpty()) {
        coreLog.trace("Empty sequence ID!");
        return false;
    }
    return true;
}

/**
 * Validates that 'posInMsa' is equal to '-1' or
 * located within bounds [0, numRows].
 * In the first case sets 'posInMsa' to 'numRows'.
 */
bool validateAndPreparePosInMsa(const MAlignment& al, qint64& posInMsa) {
    if (-1 == posInMsa) {
        posInMsa = al.getNumRows();
        return true;
    }

    if (posInMsa >= 0 && posInMsa <= al.getNumRows()) {
        return true;
    }

    coreLog.trace(QString("Invalid row position '%1' for alignment '%2'!").arg(posInMsa).arg(al.getName()));
    return false;
}


/////////////////////////////////////////////////////////////////
// Helper-methods for additional calculations

void MsaDbiUtils::calculateGapModelAfterInsert(QList<U2MsaGap>& gapModel, qint64 pos, qint64 count) {    
    SAFE_POINT(pos >= 0, QString("Invalid position '%1'!").arg(pos), );
    SAFE_POINT(count > 0, QString("Invalid characters count '%1'!").arg(count), );
    
    // There are no gaps yet
    if (gapModel.isEmpty()) {
        U2MsaGap gap(pos, count);
        gapModel.append(gap);
        return;
    }
    // There are gaps in the row
    else {
        // Insert gaps to the row beginning
        if (0 == pos) {
            U2MsaGap& firstGap = gapModel[0];
            if (0 == firstGap.offset) {
                firstGap.gap += count;
            }
            else {
                U2MsaGap beginningGap(0, count);
                gapModel.insert(0, beginningGap);
            }

            // Shift other gaps
            if (gapModel.count() > 1) {
                for (int i = 1; i < gapModel.count(); ++i) {
                    qint64 newOffset = gapModel[i].offset + count;
                    gapModel[i].offset = newOffset;
                }
            }
        }
        // Gaps are inserted to the middle of the row
        else {
            // A gap is near
            if (gapInPosition(gapModel, pos) ||
                gapInPosition(gapModel, pos - 1))
            {
                // Find the gaps and append 'count' gaps to it
                // Shift all gaps that further in the row
                for (int i = 0; i < gapModel.count(); ++i) {
                    if (pos >= gapModel[i].offset) {
                        if (pos <= gapModel[i].offset + gapModel[i].gap) {
                            gapModel[i].gap += count;
                        }
                    }
                    else {
                        gapModel[i].offset += count;
                    }
                }
            }
            // Insert between chars
            else {
                bool found = false;

                int indexGreaterGaps = 0;
                for (int i = 0; i < gapModel.count(); ++i) {
                    if (pos > gapModel[i].offset + gapModel[i].gap) {
                        continue;
                    }
                    else {
                        found = true;
                        U2MsaGap newGap(pos, count);
                        gapModel.insert(i, newGap);
                        indexGreaterGaps = i;
                        break;
                    }
                }

                // If found somewhere between existent gaps
                if (found) {
                    // Shift further gaps
                    for (int i = indexGreaterGaps + 1; i < gapModel.count(); ++i) {
                        gapModel[i].offset += count;
                    }
                }
                // This is the last gap
                else {
                    U2MsaGap newGap(pos, count);
                    gapModel.append(newGap);
                }
            }

        }
    }
}

qint64 MsaDbiUtils::calculateLeadingGapColumnsLength(const MAlignment& al) {
    qint64 leadingGapColumnsNum = 0;
    foreach (const MAlignmentRow& row, al.getRows()) {
        // Return, if there are no gaps
        if (row.getGapModel().isEmpty()) {
            return 0;
        }
        
        // Return, if the first gap is not leading
        const U2MsaGap& firstGap = row.getGapModel().first();
        if (firstGap.offset > 0) {
            return 0;
        }

        // Otherwise, compare with other rows
        if (0 == leadingGapColumnsNum) {
            leadingGapColumnsNum = firstGap.gap;
        }
        else {
            leadingGapColumnsNum = qMin(leadingGapColumnsNum, firstGap.gap);
        }
    }

    return leadingGapColumnsNum;
}

qint64 MsaDbiUtils::calculateTrailingGapColumnsLength(const MAlignment& al) {
    qint64 trailingGapColumnsNum = 0;

    qint64 oldMsaLength = al.getLength();
    qint64 newMsaLength = 0;
    foreach (const MAlignmentRow& row, al.getRows()) {
        qint64 lengthWithoutLastGap = row.getRowLengthWithoutTrailing();
        if (0 == newMsaLength) {
            newMsaLength = lengthWithoutLastGap;
        }
        else {
            newMsaLength = qMax(lengthWithoutLastGap, newMsaLength);
        }
    }
    SAFE_POINT(newMsaLength <= oldMsaLength, "Length without trailing gaps can't be greater than the original length!", 0);

    return oldMsaLength - newMsaLength;
}

void MsaDbiUtils::calculateGapModelAfterRemove(QList<U2MsaGap>& gapModel, qint64 pos, qint64 count) {
    QList<U2MsaGap> newGapModel;
    qint64 endRegionPos = pos + count; // non-inclusive
    foreach (U2MsaGap gap, gapModel) 
    {
        qint64 gapEnd = gap.offset + gap.gap;
        if (gapEnd < pos) {
            newGapModel << gap;
        }
        else if (gapEnd <= endRegionPos) {
            if (gap.offset < pos) {
                gap.gap = pos - gap.offset;
                newGapModel << gap;
            }
            // Otherwise just remove the gap (do not write to the new gap model)
        }
        else {
            if (gap.offset < pos) {
                gap.gap -= count;
                SAFE_POINT(gap.gap >= 0, "Non-positive gap length!", );
                newGapModel << gap;
            }
            else if (gap.offset < endRegionPos) {
                gap.gap = gapEnd - endRegionPos;
                gap.offset = pos;
                SAFE_POINT(gap.gap > 0, "Non-positive gap length!", );
                SAFE_POINT(gap.offset >= 0, "Negative gap offset!", );
                newGapModel << gap;
            }
            else {
                // Shift the gap
                gap.offset -= count;
                SAFE_POINT(gap.offset >= 0, "Negative gap offset!", );
                newGapModel << gap;
            }
        }
    }

    gapModel = newGapModel;
}

qint64 MsaDbiUtils::calculateGapsLength(const QList<U2MsaGap>& gapModel) {
    qint64 length = 0;
    foreach (U2MsaGap elt, gapModel) {
        length += elt.gap;
    }
    return length;
}

qint64 MsaDbiUtils::calculateRowLength(const U2MsaRow& row) {
    qint64 seqLength = row.gend - row.gstart;
    qint64 gapsLength = calculateGapsLength(row.gaps);
    return seqLength + gapsLength;
}

void MsaDbiUtils::mergeConsecutiveGaps(QList<U2MsaGap>& gapModel) {
    QList<U2MsaGap> newGapModel;
    if (gapModel.isEmpty()) {
        return;
    }

    newGapModel << gapModel[0];
    int indexInNewGapModel = 0;
    for (int i = 1; i < gapModel.count(); ++i) {
        int previousGapEnd = newGapModel[indexInNewGapModel].offset +
            newGapModel[indexInNewGapModel].gap - 1;
        int currectGapStart = gapModel[i].offset;
        SAFE_POINT(currectGapStart > previousGapEnd,
            "Incorrect gap model during merging consecutive gaps!",);
        if (currectGapStart == previousGapEnd + 1) {
            // Merge gaps
            qint64 newGapLength = newGapModel[indexInNewGapModel].gap + gapModel[i].gap;
            SAFE_POINT(newGapLength > 0, "Non-positive gap length!", )
                newGapModel[indexInNewGapModel].gap = newGapLength;
        }
        else {
            // Add the gap to the list
            newGapModel << gapModel[i];
            indexInNewGapModel++;
        }
    }
    gapModel = newGapModel;
}

void MsaDbiUtils::getStartAndEndSequencePositions(const MAlignmentRow& alRow, qint64 pos, qint64 count, qint64& startPosInSeq, qint64& endPosInSeq) {
    int rowLengthWithoutTrailingGap = alRow.getRowLengthWithoutTrailing();
    SAFE_POINT(pos < rowLengthWithoutTrailingGap, "Incorrect position!", );

    // Remove chars from the sequence
    // Calculate start position in the sequence
    if (MAlignment_GapChar == alRow.charAt(pos)) {
        int i = 1;
        while (MAlignment_GapChar == alRow.charAt(pos + i)) {
            if (alRow.getRowLength() == pos + i) {
                break;
            }
            i++;
        }
        startPosInSeq = alRow.getUngappedPosition(pos + i);
    }
    else {
        startPosInSeq = alRow.getUngappedPosition(pos);
    }

    // Calculate end position in the sequence
    int endRegionPos = pos + count; // non-inclusive

    if (endRegionPos > rowLengthWithoutTrailingGap) {
        endRegionPos = rowLengthWithoutTrailingGap;
    }

    if (endRegionPos == rowLengthWithoutTrailingGap) {
        endPosInSeq = alRow.getUngappedLength();
    }
    else {
        if (MAlignment_GapChar == alRow.charAt(endRegionPos)) {
            int i = 1;
            while (MAlignment_GapChar == alRow.charAt(endRegionPos + i)) {
                if (alRow.getRowLength() == endRegionPos + i) {
                    break;
                }
                i++;
            }
            endPosInSeq = alRow.getUngappedPosition(endRegionPos + i);
        }
        else {
            endPosInSeq = alRow.getUngappedPosition(endRegionPos);
        }
    }
}

void MsaDbiUtils::removeCharsFromRow(MAlignmentRow& alRow, qint64 pos, qint64 count) {
    SAFE_POINT(pos >= 0, "Incorrect position!", );
    SAFE_POINT(count > 0, "Incorrect characters count!", );

    if (pos >= alRow.getRowLength()) {
        return;
    }

    if (pos < alRow.getRowLengthWithoutTrailing()) {
        qint64 startPosInSeq = -1;
        qint64 endPosInSeq = -1;
        getStartAndEndSequencePositions(alRow, pos, count, startPosInSeq, endPosInSeq);

        if ((startPosInSeq < endPosInSeq) && (-1 != startPosInSeq) && (-1 != endPosInSeq)) {
            DNASequence sequence = alRow.getSequence();
            U2OpStatus2Log os;
            DNASequenceUtils::removeChars(sequence, startPosInSeq, endPosInSeq, os);
            SAFE_POINT_OP(os, );
            alRow.setSequence(sequence);
        }
    }

    QList<U2MsaGap> gapModel = alRow.getGapModel();
    calculateGapModelAfterRemove(gapModel, pos, count);
    mergeConsecutiveGaps(gapModel);
    alRow.setGapModel(gapModel);
}

void MsaDbiUtils::cropCharsFromRow(MAlignmentRow& alRow, qint64 pos, qint64 count) {
    SAFE_POINT(pos >= 0, "Incorrect position!", );
    SAFE_POINT(count > 0, "Incorrect characters count!", );

    // Change the sequence
    qint64 initialRowLength = alRow.getRowLength();
    qint64 initialSeqLength = alRow.getUngappedLength();
    DNASequence modifiedSeq = alRow.getSequence();

    if (pos >= alRow.getRowLengthWithoutTrailing()) {
        DNASequenceUtils::makeEmpty(modifiedSeq);
    }
    else {
        qint64 startPosInSeq = -1;
        qint64 endPosInSeq = -1;
        getStartAndEndSequencePositions(alRow, pos, count, startPosInSeq, endPosInSeq);

        // Remove inside a gap
        if ((startPosInSeq <= endPosInSeq) && (-1 != startPosInSeq) && (-1 != endPosInSeq))
        {
            U2OpStatus2Log os;
            if (endPosInSeq < initialSeqLength){
                DNASequenceUtils::removeChars(modifiedSeq, endPosInSeq, initialSeqLength, os);
                SAFE_POINT_OP(os, );
            }

            if (startPosInSeq > 0) {
                DNASequenceUtils::removeChars(modifiedSeq, 0, startPosInSeq, os);
                SAFE_POINT_OP(os, );
            }
        }
    }
    alRow.setSequence(modifiedSeq);

    // Change the gap model
    QList<U2MsaGap> gapModel = alRow.getGapModel();
    if (pos + count < initialRowLength) {
        calculateGapModelAfterRemove(gapModel, pos + count, initialRowLength - pos - count);
    }

    if (pos > 0) {
        calculateGapModelAfterRemove(gapModel, 0, pos);
    }
    alRow.setGapModel(gapModel);
}

/** Returns "true" if there is a gap on position "pos" */
bool MsaDbiUtils::gapInPosition(const QList<U2MsaGap>& gapModel, qint64 pos) {
    foreach (const U2MsaGap& gap, gapModel) {
        if (gap.offset + gap.gap - 1 < pos) {
            continue;
        }
        if (gap.offset > pos) {
            return false;
        }
        return true;
    }
    return false;
}


/////////////////////////////////////////////////////////////////
// MSA DBI Utilities
void MsaDbiUtils::splitBytesToCharsAndGaps(const QByteArray& input, QByteArray& seqBytes, QList<U2MsaGap>& gapModel) {
    bool previousCharIsGap = false;
    int gapsCount = 0;
    int gapsOffset = 0;

    for (int i = 0; i < input.count(); ++i) {
        // A char
        if ((MAlignment_GapChar != input.at(i)))
        {
            if (previousCharIsGap) {
                U2MsaGap gap(gapsOffset, gapsCount);
                gapModel.append(gap);
                gapsCount = 0;
            }
            seqBytes.append(input.at(i));
            previousCharIsGap = false;
        }
        // A gap
        else {
            gapsCount++;
            // A gap before the end of the row
            if (i < input.count() - 1) {
                if (!previousCharIsGap) {
                    gapsOffset = i;
                }
                previousCharIsGap = true;
            }
            // A gap at the end of the row
            else {
                // Correct the offset if there is one gap at the end of the row
                if (1 == gapsCount) {
                    gapsOffset = i;
                }
                SAFE_POINT(gapsOffset >= 0, "Negative gap offset!", );
                SAFE_POINT(gapsCount > 0, "Non-positive gap length!", );
                U2MsaGap gap(gapsOffset, gapsCount);
                gapModel.append(gap);
            }
        }
    }

    SAFE_POINT(-1 == seqBytes.indexOf(MAlignment_GapChar), "Row sequence contains gaps!", );
}

void MsaDbiUtils::updateMsa(const U2EntityRef& msaRef, const MAlignment& al, U2OpStatus& os) {
    // Prepare the connection
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL Msa Dbi!", );

    U2SequenceDbi* seqDbi = con.dbi->getSequenceDbi();
    SAFE_POINT(NULL != seqDbi, "NULL Sequence Dbi!", );

    //// UPDATE MSA OBJECT
    DNAAlphabet* alphabet = al.getAlphabet();
    SAFE_POINT(NULL != alphabet, "The alignment alphabet is NULL!", );

    U2Msa msaObj;
    msaObj.id = msaRef.entityId;
    msaObj.visualName = al.getName();
    msaObj.alphabet.id = alphabet->getId();
    msaObj.length = al.getLength();

    msaDbi->updateMsaName(msaRef.entityId, al.getName(), os);
    CHECK_OP(os, );

    msaDbi->updateMsaAlphabet(msaRef.entityId, alphabet->getId(), os);
    CHECK_OP(os, );

    //// UPDATE ROWS AND SEQUENCES
    // Get rows that are currently stored in the database
    QList<U2MsaRow> currentRows = msaDbi->getRows(msaRef.entityId, os);
    QList<qint64> currentRowIds;
    CHECK_OP(os, );

    QList<qint64> newRowsIds = al.getRowsIds();

    foreach (const U2MsaRow& currentRow, currentRows) {
        currentRowIds.append(currentRow.rowId);

        // Update data for rows with the same row and sequence IDs
        if (newRowsIds.contains(currentRow.rowId)) {
            U2MsaRow newRow = al.getRowByRowId(currentRow.rowId, os);
            CHECK_OP(os, );

            if (newRow.sequenceId == currentRow.sequenceId){
                // Update sequence and row info
                DNASequence sequence = al.getSequenceByRowId(newRow.rowId, os);
                CHECK_OP(os, );

                msaDbi->updateRowName(msaRef.entityId, newRow.rowId, sequence.getName(), os);
                CHECK_OP(os, );

                msaDbi->updateRowContent(msaRef.entityId, newRow.rowId, sequence.seq, newRow.gaps, os);
                CHECK_OP(os, );
            }
            else {
                MsaDbiUtils::removeRow(msaRef, currentRow.rowId, os);
                CHECK_OP(os, );

                MsaDbiUtils::addRow(msaRef, -1, newRow, os);
                CHECK_OP(os, );
            }
        }
        // Remove rows that are no more present in the alignment
        else {
            MsaDbiUtils::removeRow(msaRef, currentRow.rowId, os);
            CHECK_OP(os, );
        }
    }

    // Add rows that are stored in memory, but are not present in the database,
    // remember the rows order
    QList<qint64> rowsOrder;
    for (int i = 0, n = al.getNumRows(); i < n; ++i) {
        const MAlignmentRow& alRow = al.getRow(i);
        U2MsaRow row = alRow.getRowDBInfo();

        if (row.sequenceId.isEmpty() || !currentRowIds.contains(row.rowId)) {
            // Import the sequence
            DNASequence rowSeq = alRow.getSequence();
            U2Sequence sequence = U2Sequence();
            sequence.visualName = rowSeq.getName();
            sequence.circular = rowSeq.circular;
            sequence.length = rowSeq.length();

            DNAAlphabet* alphabet;
            if (NULL == rowSeq.alphabet) {
                alphabet = U2AlphabetUtils::findBestAlphabet(rowSeq.constData(), rowSeq.length());
            }
            SAFE_POINT(NULL != alphabet, "Failed to get alphabet for a sequence!", );
            sequence.alphabet.id = alphabet->getId();

            seqDbi->createSequenceObject(sequence, "", os);
            CHECK_OP(os, );

            QVariantMap hints;
            const QByteArray& seqData = rowSeq.constSequence();
            seqDbi->updateSequenceData(sequence.id, U2_REGION_MAX, seqData, hints, os);
            CHECK_OP(os, );

            // Create the row
            row.rowId = -1; // set the row ID automatically
            row.sequenceId = sequence.id;
            row.gstart = 0;
            row.gend = sequence.length;
            row.gaps = alRow.getGapModel();
            MsaDbiUtils::addRow(msaRef, -1, row, os);
            CHECK_OP(os, );
        }
        rowsOrder.append(row.rowId);
    }

    //// UPDATE ROWS POSITIONS
    msaDbi->setNewRowsOrder(msaRef.entityId, rowsOrder, os);
}

void MsaDbiUtils::updateRowContent(const U2EntityRef& msaRef, qint64 rowId, const QByteArray& seqBytes, const QList<U2MsaGap>& gaps, U2OpStatus& os) {
    // Prepare the connection
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL Msa Dbi!", );

    // Update the data
    msaDbi->updateRowContent(msaRef.entityId, rowId, seqBytes, gaps, os);
}

void MsaDbiUtils::updateRowGapModel(const U2EntityRef& msaRef, qint64 rowId, const QList<U2MsaGap>& gaps, U2OpStatus& os) {
    // Prepare the connection
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL Msa Dbi!", );

    // Update the data
    msaDbi->updateGapModel(msaRef.entityId, rowId, gaps, os);
}

void MsaDbiUtils::updateRowsOrder(const U2EntityRef& msaRef, const QList<qint64>& rowsOrder, U2OpStatus& os) {
    // Prepare the connection
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL Msa Dbi!", );

    // Update the data
    msaDbi->setNewRowsOrder(msaRef.entityId, rowsOrder, os);
}

void MsaDbiUtils::renameMsa(const U2EntityRef& msaRef, const QString& newName, U2OpStatus& os) {
    if (newName.isEmpty()) {
        os.setError(tr("Can't rename an alignment to an empty name!"));
    }

    // Prepare the connection
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL Msa Dbi!",);

    // Update the name
    msaDbi->updateMsaName(msaRef.entityId, newName, os);
}

void MsaDbiUtils::insertGaps(const U2EntityRef& msaRef, const QList<qint64>& rowIds, qint64 pos, qint64 count, U2OpStatus& os) {
    // Prepare the connection
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL Msa Dbi!",);

    // Get the MSA properties
    U2Msa msaObj = msaDbi->getMsaObject(msaRef.entityId, os);
    qint64 alLength = msaObj.length;
    qint64 numRows = msaDbi->getNumOfRows(msaRef.entityId, os);

    // Validate the position
    if (pos < 0 || pos > alLength) {
        coreLog.trace(QString("Invalid position '%1' in '%2' alignment!").arg(pos).arg(msaObj.visualName));
        os.setError(tr("Failed to insert gaps into an alignment!"));
        return;
    }

    // Validate the count of gaps
    if (count <= 0) {
        coreLog.trace(QString("Invalid value of characters count '%1'!").arg(count));
        os.setError(tr("Failed to insert gaps into an alignment!"));
        return;
    }

    // Insert gaps into rows
    QList<U2MsaRow> rows;
    foreach (qint64 rowId, rowIds) {
        U2MsaRow row = msaDbi->getRow(msaRef.entityId, rowId, os);
        CHECK_OP(os, );

        rows.append(row);
    }

    foreach (U2MsaRow row, rows) {
        // Calculate the new gap model
        calculateGapModelAfterInsert(row.gaps, pos, count);

        // Trim trailing gap (if any)
        qint64 seqLength = row.gend - row.gstart;
        for (int i = 0, n = row.gaps.count(); i < n; ++i) {
            const U2MsaGap& gap = row.gaps[i];
            if ((i == n - 1) && (gap.offset >= seqLength)) {
                row.gaps.removeAt(i);
                break;
            }
        }

        // Put the new gap model into the database
        msaDbi->updateGapModel(msaRef.entityId, row.rowId, row.gaps, os);
        CHECK_OP(os, );
    }
}

void MsaDbiUtils::removeRegion(const U2EntityRef& msaRef, const QList<qint64>& rowIds, qint64 pos, qint64 count, U2OpStatus& os) {
    // Get the alignment
    MAlignmentExporter alExporter;
    MAlignment al = alExporter.getAlignment(msaRef.dbiRef, msaRef.entityId, os);

    // Validate the parameters
    bool validPosCount = true;
    if (pos < 0 || count <= 0 || pos + count > al.getLength()) {
        coreLog.trace(QString("Incorrect position '%1' or characters count '%2'!").arg(pos).arg(count));
        validPosCount = false;
    }
    if (!validPosCount ||
        !validateRowIds(al, rowIds))
    {
        os.setError(tr("Failed to remove a region from an alignment!"));
        return;
    }

    // Prepare the connection
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL Msa Dbi!", );

    U2SequenceDbi* sequenceDbi = con.dbi->getSequenceDbi();
    SAFE_POINT(NULL != sequenceDbi, "NULL Sequence Dbi!", );

    // Remove region for each row from the list
    for (int i = 0, n = al.getNumRows(); i < n; ++i) {
        MAlignmentRow row = al.getRow(i);
        qint64 rowId = row.getRowId();
        if (rowIds.contains(rowId)) {
            U2Region regionToReplaceInSeq(0, row.getSequence().length());
            U2DataId sequenceId = row.getRowDBInfo().sequenceId;
            SAFE_POINT(!sequenceId.isEmpty(), "Empty sequence ID!", );

            // Calculate the modified row
            removeCharsFromRow(row, pos, count);

            // Put the new sequence and gap model into the database
            msaDbi->updateGapModel(msaRef.entityId, row.getRowId(), row.getGapModel(), os);
            CHECK_OP(os, );

            QVariantMap hints;
            sequenceDbi->updateSequenceData(sequenceId, regionToReplaceInSeq, row.getSequence().constSequence(), hints, os);
            CHECK_OP(os, );
        }
    }
}

void MsaDbiUtils::crop(const U2EntityRef& msaRef, const QList<qint64> rowIds, qint64 pos, qint64 count, U2OpStatus& os) {
    // Get the alignment
    MAlignmentExporter alExporter;
    MAlignment al = alExporter.getAlignment(msaRef.dbiRef, msaRef.entityId, os);

    // Validate the parameters
    if (!validatePos(al, pos) ||
        !validateCharactersCount(count) ||
        !validateRowIds(al, rowIds))
    {
        os.setError(tr("Failed to crop an alignment!"));
        return;
    }

    // Prepare the connection
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL Msa Dbi!", );

    // Crop or remove each row
    for (int i = 0, n = al.getNumRows(); i < n; ++i) {
        MAlignmentRow row = al.getRow(i);
        qint64 rowId = row.getRowId();
        if (rowIds.contains(rowId)) {
            U2Region regionToReplaceInSeq(0, row.getSequence().length());
            U2DataId sequenceId = row.getRowDBInfo().sequenceId;
            SAFE_POINT(!sequenceId.isEmpty(), "Empty sequence ID!", );

            // Calculate the modified row
            cropCharsFromRow(row, pos, count);

            // Put the new sequence and gap model into the database
            msaDbi->updateRowContent(msaRef.entityId, rowId, row.getSequence().constSequence(), row.getGapModel(), os);
            CHECK_OP(os, );
        }
        else {
            MsaDbiUtils::removeRow(msaRef, i, os);
            CHECK_OP(os, );
        }
    }
}

void MsaDbiUtils::trim(const U2EntityRef& msaRef, U2OpStatus& os) {
    // Get the alignment
    MAlignmentExporter alExporter;
    MAlignment al = alExporter.getAlignment(msaRef.dbiRef, msaRef.entityId, os);

    // Verify if there are leading/trailing gaps to remove
    qint64 leadingGapsToRemove = calculateLeadingGapColumnsLength(al);
    qint64 trailingGapsToRemove = calculateTrailingGapColumnsLength(al);
    if (0 == leadingGapsToRemove && 0 == trailingGapsToRemove) {
        return;
    }
    
    // Prepare the connection
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL Msa Dbi!",);

    // Remove the gaps
    qint64 alLength = 0;
    for (int i = 0, n = al.getNumRows(); i < n; ++i) {
        const MAlignmentRow& row = al.getRow(i);
        QList<U2MsaGap> newGapModel = row.getGapModel();
        if (leadingGapsToRemove > 0) {
            calculateGapModelAfterRemove(newGapModel, 0, leadingGapsToRemove);
        }
        
        if (trailingGapsToRemove > 0) {
            qint64 rowLength = row.getRowLength();
            qint64 rowLengthWithoutTrailing = row.getRowLengthWithoutTrailing();

            if (rowLength > rowLengthWithoutTrailing) {
                qint64 trailingGapsInCurrentRow = rowLength - rowLengthWithoutTrailing;
                trailingGapsToRemove = qMin(trailingGapsToRemove, trailingGapsInCurrentRow);

                calculateGapModelAfterRemove(newGapModel, rowLength - trailingGapsToRemove, trailingGapsToRemove);
            }
        }

        // Put the new gap model into the database
        msaDbi->updateGapModel(msaRef.entityId, row.getRowId(), newGapModel, os);
        CHECK_OP(os, );
    }
}

void MsaDbiUtils::addRow(const U2EntityRef& msaRef, qint64 posInMsa, U2MsaRow& row, U2OpStatus& os) {
    // Get the alignment
    MAlignmentExporter alExporter;
    MAlignment al = alExporter.getAlignment(msaRef.dbiRef, msaRef.entityId, os);

    // Validate the parameters, prepare 'posInMsa'
    if (!validateSequenceId(row) ||
        !validateAndPreparePosInMsa(al, posInMsa))
    {
        os.setError(tr("Failed to add a row to an alignment!"));
        return;
    }

    // Prepare the connection
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL Msa Dbi!",);

    // Add the row
    msaDbi->addRow(msaRef.entityId, posInMsa, row, os);
    CHECK_OP(os, );
}

void MsaDbiUtils::removeRow(const U2EntityRef& msaRef, qint64 rowId, U2OpStatus& os) {
    // Prepare the connection
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL Msa Dbi!", );

    // Remove the row
    msaDbi->removeRow(msaRef.entityId, rowId, os);
}

void MsaDbiUtils::renameRow(const U2EntityRef& msaRef, qint64 rowId, const QString& newName, U2OpStatus& os) {
    if (newName.isEmpty()) {
        os.setError(tr("Can't rename a row to an empty name!"));
    }

    // Prepare the connection
    DbiConnection con(msaRef.dbiRef, os);
    CHECK_OP(os, );

    U2MsaDbi* msaDbi = con.dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "NULL Msa Dbi!",);

    // Update the row name
    msaDbi->updateRowName(msaRef.entityId, rowId, newName, os);
}


} // namespace
