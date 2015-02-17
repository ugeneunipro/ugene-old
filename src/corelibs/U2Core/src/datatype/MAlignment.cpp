/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QtCore/QStack>

#include "MAlignment.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceUtils.h>
#include <U2Core/Log.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2OpStatusUtils.h>


namespace U2 {

//////////////////////////////////////////////////////////////////////////
// MAlignmentRow
MAlignmentRow::MAlignmentRow(const U2MsaRow& _rowInDb,
                             const DNASequence& _sequence,
                             const QList<U2MsaGap>& _gaps,
                             MAlignment* _alignment)
    : alignment(_alignment),
      sequence(_sequence),
      gaps(_gaps),
      initialRowInDb(_rowInDb)
{
    SAFE_POINT(alignment != NULL, "Parent MAlignment is NULL", );
    removeTrailingGaps();
}

MAlignmentRow::MAlignmentRow(MAlignment* al)
    : alignment(al),
      sequence(DNASequence()),
      initialRowInDb(U2MsaRow())
{
    initialRowInDb.rowId = invalidRowId();
    removeTrailingGaps();
}

MAlignmentRow::MAlignmentRow(const MAlignmentRow &r, MAlignment* al)
    : alignment(al),
      sequence(r.sequence),
      gaps(r.gaps),
      initialRowInDb(r.initialRowInDb)
{
    SAFE_POINT(alignment != NULL, "Parent MAlignment is NULL", );
}

QByteArray MAlignmentRow::toByteArray(int length, U2OpStatus& os) const {
    if (length < getCoreEnd()) {
        coreLog.trace("Incorrect length was passed to MAlignmentRow::toByteArray!");
        os.setError("Failed to get row data!");
        return QByteArray();
    }

    if (gaps.isEmpty() && sequence.length() == length) {
        return sequence.constSequence();
    }

    QByteArray bytes = joinCharsAndGaps(true, true);

    // Append additional gaps, if necessary
    if (length > bytes.count()) {
        QByteArray gapsBytes;
        gapsBytes.fill(MAlignment_GapChar, length - bytes.count());
        bytes.append(gapsBytes);
    }
    if (length < bytes.count()) {
        // cut extra trailing gaps
        bytes = bytes.left(length);
    }

    return bytes;
}

int MAlignmentRow::getRowLength() const {
    SAFE_POINT(alignment != NULL, "Parent MAlignment is NULL", getRowLengthWithoutTrailing());
    return alignment->getLength();
}

QByteArray MAlignmentRow::getCore() const {
    return joinCharsAndGaps(false, false);
}

QByteArray MAlignmentRow::getData() const {
    return joinCharsAndGaps(true, true);
}

void MAlignmentRow::splitBytesToCharsAndGaps(const QByteArray& input, QByteArray& seqBytes, QList<U2MsaGap>& gapsModel) {
    bool previousCharIsGap = false;
    int gapsCount = 0;
    int gapsOffset = 0;

    for (int i = 0; i < input.count(); ++i) {
        // A char
        if ((MAlignment_GapChar != input.at(i)))
        {
            if (previousCharIsGap) {
                U2MsaGap gap(gapsOffset, gapsCount);
                gapsModel.append(gap);
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
                gapsModel.append(gap);
            }
        }
    }

    SAFE_POINT(-1 == seqBytes.indexOf(MAlignment_GapChar), "Row sequence contains gaps!", );
}

void MAlignmentRow::addOffsetToGapModel(QList<U2MsaGap>& gapModel, int offset) {
    if (0 == offset) {
        return;
    }

    if (!gapModel.isEmpty()) {

        U2MsaGap& firstGap = gapModel[0];
        if (0 == firstGap.offset) {
            firstGap.gap += offset;
        }
        else {
            SAFE_POINT(offset >= 0, "Negative gap offset!", );
            U2MsaGap beginningGap(0, offset);
            gapModel.insert(0, beginningGap);
        }

        // Shift other gaps
        if (gapModel.count() > 1) {
            for (int i = 1; i < gapModel.count(); ++i) {
                qint64 newOffset = gapModel[i].offset + offset;
                SAFE_POINT(newOffset >= 0, "Negative gap offset!", );
                gapModel[i].offset = newOffset;
            }
        }
    }
    else {
        SAFE_POINT(offset >= 0, "Negative gap offset!", );
        U2MsaGap gap(0, offset);
        gapModel.append(gap);
    }
}

QByteArray MAlignmentRow::joinCharsAndGaps(bool keepOffset, bool keepTrailingGaps) const {
    QByteArray bytes = sequence.constSequence();
    int beginningOffset = 0;

    if (gaps.isEmpty()) {
        return bytes;
    }

    for (int i = 0; i < gaps.size(); ++i) {
        QByteArray gapsBytes;
        if (!keepOffset && (0 == gaps[i].offset)) {
            beginningOffset = gaps[i].gap;
            continue;
        }

        gapsBytes.fill(MAlignment_GapChar, gaps[i].gap);
        bytes.insert(gaps[i].offset - beginningOffset, gapsBytes);
    }
    SAFE_POINT(alignment != NULL, "Parent MAlignment is NULL", QByteArray());
    if (keepTrailingGaps && bytes.size() < alignment->getLength()) {
        QByteArray gapsBytes;
        gapsBytes.fill(MAlignment_GapChar, alignment->getLength() - bytes.size());
        bytes.append(gapsBytes);
    }

    return bytes;
}

int MAlignmentRow::getCoreLength() const {
    int coreStart = getCoreStart();
    int coreEnd = getCoreEnd();
    int length = coreEnd - coreStart;
    SAFE_POINT(length >= 0, QString("Internal error in MAlignamentRow:"
        " coreEnd is %1, coreStart is %2!").arg(coreEnd).arg(coreStart), length);
    return length;
}

void MAlignmentRow::append(const MAlignmentRow& anotherRow, int lengthBefore, U2OpStatus& os) {
    int rowLength = getRowLengthWithoutTrailing();

    if (lengthBefore < rowLength/*getRowLengthWithoutTrailing()*/) {
        coreLog.trace(QString("Internal error: incorrect length '%1' were passed to MAlignmentRow::append,"
            "coreEnd is '%2'").arg(lengthBefore).arg(getCoreEnd()));
        os.setError("Failed to append one row to another!");
        return;
    }

    // Gap between rows
    if (lengthBefore > rowLength) {
        gaps.append(U2MsaGap(getRowLengthWithoutTrailing(),
                             lengthBefore - getRowLengthWithoutTrailing()));
    }

    // Merge gaps
    QList<U2MsaGap> anotherRowGaps = anotherRow.getGapModel();
    for (int i = 0; i < anotherRowGaps.count(); ++i) {
        anotherRowGaps[i].offset += lengthBefore;
    }
    gaps.append(anotherRowGaps);
    mergeConsecutiveGaps();

    // Merge sequences
    DNASequenceUtils::append(sequence, anotherRow.sequence, os);
}

U2MsaRow MAlignmentRow::getRowDBInfo() const {
    U2MsaRow row;
    row.rowId = initialRowInDb.rowId;
    row.sequenceId = initialRowInDb.sequenceId;
    row.gstart = 0;
    row.gend = sequence.length();
    row.gaps = gaps;
    row.length = getRowLengthWithoutTrailing();
    return row;
}

void MAlignmentRow::setRowContent(const QByteArray& bytes, int offset, U2OpStatus& /* os */) {
    QByteArray newSequenceBytes;
    QList<U2MsaGap> newGapsModel;

    splitBytesToCharsAndGaps(bytes, newSequenceBytes, newGapsModel);
    DNASequence newSequence(getName(), newSequenceBytes);

    addOffsetToGapModel(newGapsModel, offset);

    sequence = newSequence;
    gaps = newGapsModel;
    removeTrailingGaps();
}

void MAlignmentRow::setGapModel(const QList<U2MsaGap> &newGapModel) {
    gaps = newGapModel;
    removeTrailingGaps();
}

void MAlignmentRow::setSequence(const DNASequence& newSequence) {
    SAFE_POINT(!newSequence.constSequence().contains(MAlignment_GapChar),
        "The sequence must be without gaps!", );
    sequence = newSequence;
}

char MAlignmentRow::charAt(int pos) const {
    return MsaRowUtils::charAt(sequence.seq, gaps, pos);
}

void MAlignmentRow::insertGaps(int pos, int count, U2OpStatus& os) {
    if (count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed to MAlignmentRow::insertGaps,"
            "pos '%1', count '%2'!").arg(pos).arg(count));
        os.setError("Failed to insert gaps into a row!");
        return;
    }

    if (pos < 0 || pos >= getRowLengthWithoutTrailing()) {
        return;
    }

    if (0 == pos) {
        addOffsetToGapModel(gaps, count);
    }
    else {
        // A gap is near
        if (MAlignment_GapChar == charAt(pos) ||
            MAlignment_GapChar == charAt(pos - 1))
        {
            // Find the gaps and append 'count' gaps to it
            // Shift all gaps that further in the row
            for (int i = 0; i < gaps.count(); ++i) {
                if (pos >= gaps[i].offset) {
                    if (pos <= gaps[i].offset + gaps[i].gap) {
                        gaps[i].gap += count;
                    }
                }
                else {
                    gaps[i].offset += count;
                }
            }
        }
        // Insert between chars
        else {
            bool found = false;

            int indexGreaterGaps = 0;
            for (int i = 0; i < gaps.count(); ++i) {
                if (pos > gaps[i].offset + gaps[i].gap) {
                    continue;
                }
                else {
                    found = true;
                    U2MsaGap newGap(pos, count);
                    gaps.insert(i, newGap);
                    indexGreaterGaps = i;
                    break;
                }
            }

            // If found somewhere between existent gaps
            if (found) {
                // Shift further gaps
                for (int i = indexGreaterGaps + 1; i < gaps.count(); ++i) {
                    gaps[i].offset += count;
                }
            }
            // This is the last gap
            else {
                U2MsaGap newGap(pos, count);
                gaps.append(newGap);
                return;
            }
        }
    }
}

void MAlignmentRow::mergeConsecutiveGaps() {
    QList<U2MsaGap> newGapModel;
    if (gaps.isEmpty()) {
        return;
    }

    newGapModel << gaps[0];
    int indexInNewGapModel = 0;
    for (int i = 1; i < gaps.count(); ++i) {
        int previousGapEnd = newGapModel[indexInNewGapModel].offset +
            newGapModel[indexInNewGapModel].gap - 1;
        int currectGapStart = gaps[i].offset;
        SAFE_POINT(currectGapStart > previousGapEnd,
            "Incorrect gap model during merging consecutive gaps!",);
        if (currectGapStart == previousGapEnd + 1) {
            // Merge gaps
            qint64 newGapLength = newGapModel[indexInNewGapModel].gap + gaps[i].gap;
            SAFE_POINT(newGapLength > 0, "Non-positive gap length!", )
            newGapModel[indexInNewGapModel].gap = newGapLength;
        }
        else {
            // Add the gap to the list
            newGapModel << gaps[i];
            indexInNewGapModel++;
        }
    }
    gaps = newGapModel;
}

void MAlignmentRow::removeTrailingGaps() {
    if (gaps.isEmpty()) {
        return;
    }

    // If the last char in the row is gap, remove the last gap
    if (MAlignment_GapChar == charAt(MsaRowUtils::getRowLength(sequence.constData(), gaps) - 1)) {
        gaps.removeLast();
    }
}

void MAlignmentRow::removeChars(int pos, int count, U2OpStatus& os) {
    if (pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed to MAlignmentRow::removeChars,"
            "pos '%1', count '%2'!").arg(pos).arg(count));
        os.setError("Can't remove chars from a row!");
        return;
    }

    if (pos >= getRowLengthWithoutTrailing()) {
        return;
    }

    if (pos < getRowLengthWithoutTrailing()) {
        int startPosInSeq = -1;
        int endPosInSeq = -1;
        getStartAndEndSequencePositions(pos, count, startPosInSeq, endPosInSeq);

        // Remove inside a gap
        if ((startPosInSeq < endPosInSeq) && (-1 != startPosInSeq) && (-1 != endPosInSeq)) {
            DNASequenceUtils::removeChars(sequence, startPosInSeq, endPosInSeq, os);
            CHECK_OP(os, );
        }
    }

    // Remove gaps from the gaps model
    removeGapsFromGapModel(pos, count);

    removeTrailingGaps();
    mergeConsecutiveGaps();
}

bool MAlignmentRow::isRowContentEqual(const MAlignmentRow& row) const {
    if (MatchExactly == DNASequenceUtils::compare(sequence, row.getSequence())) {
        if (sequence.length() == 0) {
            return true;
        }
        else {
            QList<U2MsaGap> firstRowGaps = gaps;
            if  (!firstRowGaps.isEmpty() &&
                (MAlignment_GapChar == charAt(0)))
            {
                firstRowGaps.removeFirst();
            }

            QList<U2MsaGap> secondRowGaps = row.getGapModel();
            if (!secondRowGaps.isEmpty() &&
                (MAlignment_GapChar == row.charAt(0)))
            {
                secondRowGaps.removeFirst();
            }

            if (firstRowGaps == secondRowGaps) {
                return true;
            }
        }
    }

    return false;
}

int MAlignmentRow::getUngappedPosition(int pos) const {
    return MsaRowUtils::getUngappedPosition(sequence.seq, gaps, pos);
}

int MAlignmentRow::getBaseCount(int before) const {
    const int rowLength = MsaRowUtils::getRowLength(sequence.seq, gaps);
    const int trimmedRowPos = before < rowLength ? before : rowLength;
    return MsaRowUtils::getUngappedPosition(sequence.seq, gaps, trimmedRowPos, true);
}

void MAlignmentRow::getStartAndEndSequencePositions(int pos, int count, int& startPosInSeq, int& endPosInSeq) {
    int rowLengthWithoutTrailingGap = getRowLengthWithoutTrailing();
    SAFE_POINT(pos < rowLengthWithoutTrailingGap,
        QString("Incorrect position '%1' in MAlignmentRow::getStartAndEndSequencePosition,"
        " row length without trailing gaps is '%2'!").arg(pos).arg(rowLengthWithoutTrailingGap),);

    // Remove chars from the sequence
    // Calculate start position in the sequence
    if (MAlignment_GapChar == charAt(pos)) {
        int i = 1;
        while (MAlignment_GapChar == charAt(pos + i)) {
            if (getRowLength() == pos + i) {
                break;
            }
            i++;
        }
        startPosInSeq = getUngappedPosition(pos + i);
    }
    else {
        startPosInSeq = getUngappedPosition(pos);
    }

    // Calculate end position in the sequence
    int endRegionPos = pos + count; // non-inclusive

    if (endRegionPos > rowLengthWithoutTrailingGap) {
        endRegionPos = rowLengthWithoutTrailingGap;
    }

    if (endRegionPos == rowLengthWithoutTrailingGap) {
        endPosInSeq = getUngappedLength();
    }
    else {
        if (MAlignment_GapChar == charAt(endRegionPos)) {
            int i = 1;
            while (MAlignment_GapChar == charAt(endRegionPos + i)) {
                if (getRowLength() == endRegionPos + i) {
                    break;
                }
                i++;
            }
            endPosInSeq = getUngappedPosition(endRegionPos + i);
        }
        else {
            endPosInSeq = getUngappedPosition(endRegionPos);
        }
    }
}

void MAlignmentRow::removeGapsFromGapModel(int pos, int count) {
    QList<U2MsaGap> newGapModel;
    int endRegionPos = pos + count; // non-inclusive
    foreach (U2MsaGap gap, gaps)
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

    gaps = newGapModel;
}

void MAlignmentRow::crop(int pos, int count, U2OpStatus& os) {
    if (pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed to MAlignmentRow::crop,"
            "startPos '%1', length '%2', row length '%3'!").arg(pos).arg(count).arg(getRowLength()));
        os.setError("Can't crop a row!");
        return;
    }

    int initialRowLength = getRowLength();
    int initialSeqLength = getUngappedLength();

    if (pos >= getRowLengthWithoutTrailing()) {
        // Clear the row content
        DNASequenceUtils::makeEmpty(sequence);
    }
    else {
        int startPosInSeq = -1;
        int endPosInSeq = -1;
        getStartAndEndSequencePositions(pos, count, startPosInSeq, endPosInSeq);

        // Remove inside a gap
        if ((startPosInSeq <= endPosInSeq) && (-1 != startPosInSeq) && (-1 != endPosInSeq))
        {
            if (endPosInSeq < initialSeqLength){
                DNASequenceUtils::removeChars(sequence, endPosInSeq, getUngappedLength(), os);
                CHECK_OP(os, );
            }

            if (startPosInSeq > 0) {
                DNASequenceUtils::removeChars(sequence, 0, startPosInSeq, os);
                CHECK_OP(os, );
            }
        }
    }

    if (pos + count < initialRowLength) {
        removeGapsFromGapModel(pos + count, initialRowLength - pos - count);
    }

    if (pos > 0) {
        removeGapsFromGapModel(0, pos);
    }
    removeTrailingGaps();
}

MAlignmentRow MAlignmentRow::mid(int pos, int count, U2OpStatus& os) const {
    MAlignmentRow row = *this;
    row.crop(pos, count, os);
    return row;
}

bool MAlignmentRow::operator==(const MAlignmentRow& row) const {
    return isRowContentEqual(row);
}

void MAlignmentRow::toUpperCase() {
    DNASequenceUtils::toUpperCase(sequence);
}

bool gapLessThan(const U2MsaGap& gap1, const U2MsaGap& gap2) {
    return gap1.offset < gap2.offset;
}

void MAlignmentRow::replaceChars(char origChar, char resultChar, U2OpStatus& os) {
    if (MAlignment_GapChar == origChar) {
        coreLog.trace("The original char can't be a gap in MAlignmentRow::replaceChars!");
        os.setError("Failed to replace chars in an alignment row!");
        return;
    }

    if (MAlignment_GapChar == resultChar) {
        // Get indexes of all 'origChar' characters in the row sequence
        QList<int> gapsIndexes;
        for (int i = 0; i < getRowLength(); i++) {
            if (origChar == charAt(i)) {
                gapsIndexes.append(i);
            }
        }

        if (gapsIndexes.isEmpty()) {
            return; // There is nothing to replace
        }

        // Remove all 'origChar' characters from the row sequence
        sequence.seq.replace(origChar, "");

        // Re-calculate the gaps model
        QList<U2MsaGap> newGapsModel = gaps;
        for (int i = 0; i < gapsIndexes.size(); ++i) {
            int index = gapsIndexes[i];
            U2MsaGap gap(index, 1);
            newGapsModel.append(gap);
        }
        qSort(newGapsModel.begin(), newGapsModel.end(), gapLessThan);

        // Replace the gaps model with the new one
        gaps = newGapsModel;
        mergeConsecutiveGaps();
    }
    else {
        // Just replace all occurrences of 'origChar' by 'resultChar'
        sequence.seq.replace(origChar, resultChar);
    }
}


//////////////////////////////////////////////////////////////////////////
// MAlignment

// Helper class to call MAlignment state check
class MAStateCheck {
public:
    MAStateCheck(const MAlignment* _ma) : ma(_ma) {}

    ~MAStateCheck() {
#ifdef _DEBUG
        ma->check();
#endif
    }
    const MAlignment* ma;
};

MAlignment::MAlignment(const QString& _name, const DNAAlphabet* al, const QList<MAlignmentRow>& r)
: alphabet(al), rows(r)
{
    MAStateCheck check(this);

    SAFE_POINT(al==NULL || !_name.isEmpty(), "Incorrect parameters in MAlignment ctor!", );

    setName(_name );
    length = 0;
    for (int i = 0, n = rows.size(); i < n; i++) {
        const MAlignmentRow& r = rows.at(i);
        length = qMax(length, r.getCoreEnd());
    }
}

MAlignment::MAlignment(const MAlignment &m)
    : alphabet(m.alphabet),
      length(m.length),
      info(m.info)
{
    U2OpStatusImpl os;
    for (int i = 0; i < m.rows.size(); i++) {
        MAlignmentRow r = createRow( m.rows.at(i), os);
        addRow(r, m.length, i, os);
        SAFE_POINT_OP(os, );
    }
}

MAlignment & MAlignment::operator=(const MAlignment &other) {
    clear();

    alphabet = other.alphabet;
    length = other.length;
    info = other.info;

    U2OpStatusImpl os;
    for (int i = 0; i < other.rows.size(); i++) {
        const MAlignmentRow r = createRow(other.rows.at(i), os);
        addRow(r, other.length, i, os);
        SAFE_POINT_OP(os, *this);
    }

    return *this;
}

void MAlignment::setAlphabet(const DNAAlphabet* al) {
    SAFE_POINT(NULL != al, "Internal error: attempted to set NULL alphabet fro an alignment!",);
    alphabet = al;
}

bool MAlignment::trim( bool removeLeadingGaps ) {
    MAStateCheck check(this);
    Q_UNUSED(check);

    bool result = false;

    if ( removeLeadingGaps ) {
        // Verify if there are leading columns of gaps
        // by checking the first gap in each row
        qint64 leadingGapColumnsNum = 0;
        foreach (MAlignmentRow row, rows) {
            if (row.getGapModel().count() > 0) {
                U2MsaGap firstGap = row.getGapModel().first();
                if (firstGap.offset > 0) {
                    leadingGapColumnsNum = 0;
                    break;
                }
                else {
                    if (leadingGapColumnsNum == 0) {
                        leadingGapColumnsNum = firstGap.gap;
                    }
                    else {
                        leadingGapColumnsNum = qMin(leadingGapColumnsNum, firstGap.gap);
                    }
                }
            }
            else {
                leadingGapColumnsNum = 0;
                break;
            }
        }

        // If there are leading gap columns, remove them
        U2OpStatus2Log os;
        if (leadingGapColumnsNum > 0) {
            for (int i = 0; i < rows.count(); ++i) {
                rows[i].removeChars(0, leadingGapColumnsNum, os);
                CHECK_OP(os, true);
                result = true;
            }
        }
    }

    // Verify right side of the alignment (trailing gaps and rows' lengths)
    int newLength = 0;
    foreach (MAlignmentRow row, rows) {
        if (newLength == 0) {
            newLength = row.getRowLengthWithoutTrailing();
        }
        else {
            newLength = qMax(row.getRowLengthWithoutTrailing(), newLength);
        }
    }

    if (newLength != length) {
        length = newLength;
        result = true;
    }

    return result;
}

bool MAlignment::simplify() {
    MAStateCheck check(this);
    Q_UNUSED(check);

    int newLen = 0;
    bool changed = false;
    for (int i=0, n = rows.size(); i < n; i++) {
        MAlignmentRow& r = rows[i];
        changed = r.simplify() || changed;
        newLen = qMax(newLen, r.getCoreEnd());
    }
    if (!changed) {
        assert(length == newLen);
        return false;
    }
    length = newLen;
    return true;
}

bool MAlignment::hasEmptyGapModel( ) const {
    for ( int i = 0, n = rows.size( ); i < n; ++i ) {
        const MAlignmentRow &row = rows.at( i );
        if ( !row.getGapModel( ).isEmpty( ) ) {
            return false;
        }
    }
    return true;
}

bool MAlignment::hasEqualLength() const {
    const int defaultSequenceLength = -1;
    int sequenceLength = defaultSequenceLength;
    for ( int i = 0, n = rows.size( ); i < n; ++i ) {
        const MAlignmentRow &row = rows.at( i );
        if ( defaultSequenceLength != sequenceLength
            && sequenceLength != row.getUngappedLength() )
        {
            return false;
        } else {
            sequenceLength = row.getUngappedLength();
        }
    }
    return true;
}

void MAlignment::clear() {
    MAStateCheck check(this);
    Q_UNUSED(check);

    rows.clear();
    length = 0;
}

MAlignment MAlignment::mid(int start, int len) const {
    static MAlignment emptyAlignment;
    SAFE_POINT(start >= 0 && start + len <= length,
        QString("Incorrect parameters were passed to MAlignment::mid:"
        "start '%1', len '%2', the alignment length is '%3'!").arg(start).arg(len).arg(length),
        emptyAlignment);

    MAlignment res(getName(), alphabet);
    MAStateCheck check(&res);
    Q_UNUSED(check);

    U2OpStatus2Log os;
    foreach(const MAlignmentRow& r, rows) {
        MAlignmentRow mRow = r.mid(start, len, os);
        mRow.setParentAlignment(&res);
        res.rows.append(mRow);
    }
    res.length = len;
    return res;
}


MAlignment& MAlignment::operator+=(const MAlignment& ma) {
    MAStateCheck check(this);
    Q_UNUSED(check);

    SAFE_POINT(ma.alphabet == alphabet, "Different alphabets in MAlignment::operator+= !", *this);

    int nSeq = getNumRows();
    SAFE_POINT(ma.getNumRows() == nSeq, "Different number of rows in MAlignment::operator+= !", *this);

    U2OpStatus2Log os;
    for (int i=0; i < nSeq; i++) {
        MAlignmentRow& myRow = rows[i];
        const MAlignmentRow& anotherRow = ma.rows.at(i);
        myRow.append(anotherRow, length, os);
    }

    length += ma.length;
    return *this;
}

bool MAlignment::operator==(const MAlignment& other) const {
    bool lengthsAreEqual = (length==other.length);
    bool alphabetsAreEqual = (alphabet == other.alphabet);
    bool rowsAreEqual = (rows == other.rows);
//    bool infosAreEqual = (info == other.info);

    return lengthsAreEqual && alphabetsAreEqual && rowsAreEqual;// && infosAreEqual;
}

bool MAlignment::operator!=(const MAlignment& other) const {
    return !operator==(other);
}

int MAlignment::estimateMemorySize() const {
    int result = info.size() * 20; //approximate info size estimation
    foreach(const MAlignmentRow& r, rows) {
        result += r.getCoreLength() + getName().length() * 2  + 12; //+12 -> overhead for byte array
    }
    return result;
}

bool MAlignment::crop(const U2Region& region, const QSet<QString>& rowNames, U2OpStatus& os) {
    if (!(region.startPos >= 0 && region.length > 0 && region.length < length && region.startPos < length)) {
        os.setError(QString("Incorrect region was passed to MAlignment::crop,"
                            "startPos '%1', length '%2'!").arg(region.startPos).arg(region.length));
        return false;
    }

    int cropLen = region.length;
    if (region.endPos() > length) {
        cropLen -=  (region.endPos() - length);
    }

    MAStateCheck check(this);
    Q_UNUSED(check);

    QList<MAlignmentRow> newList;
    for (int i = 0 ; i < rows.size(); i++) {
        MAlignmentRow row = rows[i];
        const QString& rowName = row.getName();
        if (rowNames.contains(rowName)){
            row.crop(region.startPos, cropLen, os);
            CHECK_OP(os, false);
            newList.append(row);
        }
    }
    rows = newList;

    length = cropLen;
    return true;
}

bool MAlignment::crop(const U2Region &region, U2OpStatus& os) {
    return crop(region, getRowNames().toSet(), os);
}

bool MAlignment::crop(int start, int count, U2OpStatus& os) {
    return crop(U2Region(start, count), os);
}

MAlignmentRow MAlignment::createRow(const QString& name, const QByteArray& bytes, U2OpStatus& /* os */) {
    QByteArray newSequenceBytes;
    QList<U2MsaGap> newGapsModel;

    MAlignmentRow::splitBytesToCharsAndGaps(bytes, newSequenceBytes, newGapsModel);
    DNASequence newSequence(name, newSequenceBytes);

    U2MsaRow row;
    row.rowId = MAlignmentRow::invalidRowId();

    return MAlignmentRow(row, newSequence, newGapsModel, this);
}

MAlignmentRow MAlignment::createRow(const U2MsaRow& rowInDb, const DNASequence& sequence, const QList<U2MsaGap>& gaps, U2OpStatus& os) {
    QString errorDescr = "Failed to create a multiple alignment row!";
    if (-1 != sequence.constSequence().indexOf(MAlignment_GapChar)) {
        coreLog.trace("Attempted to create an alignment row from a sequence with gaps!");
        os.setError(errorDescr);
        return MAlignmentRow();
    }

    int length = sequence.length();
    foreach (const U2MsaGap& gap, gaps) {
        if (gap.offset > length || !gap.isValid()) {
            coreLog.trace("Incorrect gap model was passed to MAlignmentRow::createRow!");
            os.setError(errorDescr);
            return MAlignmentRow();
        }
        length += gap.gap;
    }

    return MAlignmentRow(rowInDb, sequence, gaps, this);
}

MAlignmentRow MAlignment::createRow(const MAlignmentRow &r, U2OpStatus &/*os*/) {
    return MAlignmentRow(r, this);
}


void MAlignment::addRow(const MAlignmentRow& row, int rowLenWithTrailingGaps, int rowIndex, U2OpStatus& /* os */) {
    MAStateCheck check(this);
    Q_UNUSED(check);

    length = qMax(rowLenWithTrailingGaps, length);
    int idx = rowIndex == -1 ? getNumRows() : qBound(0, rowIndex, getNumRows());
    rows.insert(idx, row);
}

void MAlignment::addRow(const QString& name, const QByteArray& bytes, U2OpStatus& os) {
    MAlignmentRow newRow = createRow(name, bytes, os);
    CHECK_OP(os, );

    addRow(newRow, bytes.size(), -1, os);
}

void MAlignment::addRow(const QString& name, const QByteArray& bytes, int rowIndex, U2OpStatus& os) {
    MAlignmentRow newRow = createRow(name, bytes, os);
    CHECK_OP(os, );

    addRow(newRow, bytes.size(), rowIndex, os);
}

void MAlignment::addRow(const U2MsaRow& rowInDb, const DNASequence& sequence, U2OpStatus& os) {
    MAlignmentRow newRow = createRow(rowInDb, sequence, rowInDb.gaps, os);
    CHECK_OP(os, );

    addRow(newRow, rowInDb.length, -1, os);
}

void MAlignment::addRow(const QString& name, const DNASequence &sequence, const QList<U2MsaGap> &gaps, U2OpStatus &os) {
    U2MsaRow row;
    row.rowId = MAlignmentRow::invalidRowId();

    MAlignmentRow newRow = createRow(row, sequence, gaps, os);
    CHECK_OP(os, );

    int len = sequence.length();
    foreach (const U2MsaGap& gap, gaps) {
        len += gap.gap;
    }

    addRow(newRow, len, -1, os);
}

void MAlignment::removeRow(int rowIndex, U2OpStatus& os) {
    if (rowIndex < 0 || rowIndex >= getNumRows()) {
        coreLog.trace(QString("Internal error: incorrect parameters was passed to MAlignment::removeRow,"
            "rowIndex '%1', the number of rows is '%2'!").arg(rowIndex).arg(getNumRows()));
        os.setError("Failed to remove a row!");
        return;
    }
    MAStateCheck check(this);
    Q_UNUSED(check);

    rows.removeAt(rowIndex);

    if (rows.isEmpty()) {
        length = 0;
    }
}

void MAlignment::insertGaps(int row, int pos, int count, U2OpStatus& os) {
    if (row >= getNumRows() || row < 0 || pos > length || pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed"
            " to MAlignment::insertGaps: row index '%1', pos '%2', count '%3'!").arg(row).arg(pos).arg(count));
        os.setError("Failed to insert gaps into an alignment!");
        return;
    }

    if (pos == length) {
        // add trailing gaps --> just increase alignment len
        length += count;
        return;
    }

    MAStateCheck check(this);
    Q_UNUSED(check);

    MAlignmentRow& r = rows[row];
    if (pos >= r.getRowLengthWithoutTrailing()) {
        length += count;
        return;
    }
    r.insertGaps(pos, count, os);

    int rowLength = r.getRowLengthWithoutTrailing();
    length = qMax(length, rowLength);
}

void MAlignment::appendChars(int row, const char* str, int len) {
    SAFE_POINT(0 <= row && row < getNumRows(),
        QString("Incorrect row index '%1' in MAlignment::appendChars!").arg(row),);

    U2OpStatus2Log os;
    MAlignmentRow appendedRow = createRow("", QByteArray(str, len), os);
    CHECK_OP(os, );

    int rowLength = rows[row].getRowLength();;

    rows[row].append(appendedRow, rowLength, os);
    CHECK_OP(os, );

    length = qMax(length, rowLength + len);
}

void MAlignment::appendChars(int row, int afterPos, const char *str, int len) {
    SAFE_POINT(0 <= row && row < getNumRows(),
        QString("Incorrect row index '%1' in MAlignment::appendChars!").arg(row),);

    U2OpStatus2Log os;
    MAlignmentRow appendedRow = createRow("", QByteArray(str, len), os);
    CHECK_OP(os, );

    rows[row].append(appendedRow, afterPos, os);
    CHECK_OP(os, );

    length = qMax(length, afterPos + len);

}

void MAlignment::appendRow(int row, const MAlignmentRow &r, bool ignoreTrailingGaps, U2OpStatus& os) {
    appendRow(row, ignoreTrailingGaps ? rows[row].getRowLengthWithoutTrailing()
                                      : rows[row].getRowLength(), r, os);
}

void MAlignment::appendRow(int row, int afterPos, const MAlignmentRow &r, U2OpStatus& os) {
    SAFE_POINT(0 <= row && row < getNumRows(),
        QString("Incorrect row index '%1' in MAlignment::appendRow!").arg(row),);

    rows[row].append(r, afterPos, os);
    CHECK_OP(os, );

    length = qMax(length, afterPos + r.getRowLength());
}

void MAlignment::removeChars(int row, int pos, int count, U2OpStatus& os) {
    if (row >= getNumRows() || row < 0 || pos > length || pos < 0 || count < 0) {
        coreLog.trace(QString("Internal error: incorrect parameters were passed"
            " to MAlignment::removeChars: row index '%1', pos '%2', count '%3'!").arg(row).arg(pos).arg(count));
        os.setError("Failed to remove chars from an alignment!");
        return;
    }

    MAStateCheck check(this);
    Q_UNUSED(check);

    MAlignmentRow& r = rows[row];
    r.removeChars(pos, count, os);
}

void MAlignment::removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows) {
    SAFE_POINT(startPos >= 0 && startPos + nBases <= length && nBases > 0,
        QString("Incorrect parameters were passed to MAlignment::removeRegion: startPos '%1',"
        " nBases '%2', the length is '%3'!").arg(startPos).arg(nBases).arg(length),);
    SAFE_POINT(startRow >= 0 && startRow + nRows <= getNumRows() && nRows > 0,
        QString("Incorrect parameters were passed to MAlignment::removeRegion: startRow '%1',"
        " nRows '%2', the number of rows is '%3'!").arg(startRow).arg(nRows).arg(getNumRows()),);

    MAStateCheck check(this);
    Q_UNUSED(check);

    U2OpStatus2Log os;
    for (int i = startRow + nRows; --i >= startRow;) {
        MAlignmentRow& r = rows[i];

        r.removeChars(startPos, nBases, os);
        SAFE_POINT_OP(os, );

        if (removeEmptyRows && (0 == r.getSequence().length())) {
            rows.removeAt(i);
        }
    }

    if (nRows == rows.size()) {
        // full columns were removed
        length -= nBases;
        if (length == 0) {
            rows.clear();
        }
    }
}

void MAlignment::setLength(int newLength) {
    SAFE_POINT(newLength >=0, QString("Internal error: attempted to set length '%1' for an alignment!").arg(newLength),);

    MAStateCheck check(this);
    Q_UNUSED(check);

    if (newLength >= length) {
        length = newLength;
        return;
    }

    U2OpStatus2Log os;
    for (int i=0, n = getNumRows(); i < n; i++) {
        MAlignmentRow& row = rows[i];
        row.crop(0, newLength, os);
        CHECK_OP(os, );
    }
    length = newLength;
}

void MAlignment::renameRow(int row, const QString& name) {
    SAFE_POINT(row >= 0 && row < getNumRows(),
        QString("Incorrect row index '%1' was passed to MAlignment::renameRow: "
        " the number of rows is '%2'!").arg(row).arg(getNumRows()),);
    SAFE_POINT(!name.isEmpty(),
        "Incorrect parameter 'name' was passed to MAlignment::renameRow: "
        " Can't set the name of a row to an empty string!",);
    MAlignmentRow& r = rows[row];
    r.setName(name);
}


void MAlignment::replaceChars(int row, char origChar, char resultChar) {
    SAFE_POINT(row >= 0 && row < getNumRows(),
        QString("Incorrect row index '%1' in MAlignment::replaceChars").arg(row),);

    if (origChar == resultChar) {
        return;
    }
    MAlignmentRow& r = rows[row];
    U2OpStatus2Log os;
    r.replaceChars(origChar, resultChar, os);
}

void MAlignment::setRowContent(int row, const QByteArray& sequence, int offset) {
    SAFE_POINT(row >= 0 && row < getNumRows(),
        QString("Incorrect row index '%1' was passed to MAlignment::setRowContent: "
        " the number of rows is '%2'!").arg(row).arg(getNumRows()),);
    MAStateCheck check(this);
    Q_UNUSED(check);

    MAlignmentRow& r = rows[row];

    U2OpStatus2Log os;
    r.setRowContent(sequence, offset, os);
    SAFE_POINT_OP(os, );

    length = qMax(length, sequence.size() + offset);
}

void MAlignment::toUpperCase() {
    for (int i = 0, n = getNumRows(); i < n; i++) {
        MAlignmentRow& row = rows[i];
        row.toUpperCase();
    }
}

class CompareMARowsByName {
public:
    CompareMARowsByName(bool _asc = true) : asc(_asc){}
    bool operator()(const MAlignmentRow& row1, const MAlignmentRow& row2) const {
        bool res = row1.getName() > row2.getName();
        return asc ? !res : res;
    }

    bool asc;
};

void MAlignment::sortRowsByName(bool asc) {
    MAStateCheck check(this);

    qStableSort(rows.begin(), rows.end(), CompareMARowsByName(asc));
}

bool MAlignment::sortRowsBySimilarity(QVector<U2Region>& united) {
    QList<MAlignmentRow> oldRows = rows;
    QList<MAlignmentRow> sortedRows;
    while (!oldRows.isEmpty()) {
        const MAlignmentRow& r = oldRows.takeFirst();
        sortedRows.append(r);
        int start = sortedRows.size() - 1;
        int len = 1;
        QMutableListIterator<MAlignmentRow> iter(oldRows);
        while (iter.hasNext()) {
            const MAlignmentRow& next = iter.next();
            if(next.isRowContentEqual(r)) {
                sortedRows.append(next);
                iter.remove();
                ++len;
            }
        }
        if (len > 1) {
            united.append(U2Region(start, len));
        }
    }
    if(rows != sortedRows) {
        rows = sortedRows;
        return true;
    }
    return false;
}

void MAlignment::moveRowsBlock(int startRow, int numRows, int delta)
{
    MAStateCheck check(this);

    // Assumption: numRows is rather big, delta is small (1~2)
    // It's more optimal to move abs(delta) of rows then the block itself

    int i = 0;
    int k = qAbs(delta);

    SAFE_POINT(( delta > 0 && startRow + numRows + delta - 1 < rows.length())
        || (delta < 0 && startRow + delta >= 0),
        QString("Incorrect parameters in MAlignment::moveRowsBlock: "
        "startRow: '%1', numRows: '%2', delta: '%3'").arg(startRow).arg(numRows).arg(delta),);

    QList<MAlignmentRow> toMove;
    int fromRow = delta > 0 ? startRow + numRows  : startRow + delta;

    while (i <  k) {
        MAlignmentRow row = rows.takeAt(fromRow);
        toMove.append(row);
        i++;
    }

    int toRow = delta > 0 ? startRow : startRow + numRows - k;

    while (toMove.count() > 0) {
        int n = toMove.count();
        MAlignmentRow row = toMove.at(n - 1);
        toMove.removeAt(n - 1);
        rows.insert(toRow, row);
    }
}

QStringList MAlignment::getRowNames() const {
    QStringList rowNames;
    foreach (const MAlignmentRow& r, rows) {
        rowNames.append(r.getName());
    }
    return rowNames;
}

QList<qint64> MAlignment::getRowsIds() const {
    QList<qint64> rowIds;
    foreach (const MAlignmentRow& row, rows) {
        rowIds.append(row.getRowId());
    }
    return rowIds;
}

MAlignmentRow MAlignment::getRowByRowId(qint64 rowId, U2OpStatus& os) const {
    foreach (const MAlignmentRow& row, rows) {
        if (row.getRowId() == rowId) {
            return row;
        }
    }
    os.setError("Failed to find a row in an alignment!");
    return MAlignmentRow();
}

int MAlignment::getRowIndexByRowId( qint64 rowId, U2OpStatus &os ) const {
    for ( int rowIndex = 0; rowIndex < rows.size( ); ++rowIndex ) {
        if ( rows.at( rowIndex ).getRowId( ) == rowId ) {
            return rowIndex;
        }
    }
    os.setError("Invalid row id!");
    return MAlignmentRow::invalidRowId();
}

char MAlignment::charAt(int rowIndex, int pos) const {
    const MAlignmentRow& mai = rows[rowIndex];
    char c = mai.charAt(pos);
    return c;
}

void MAlignment::setRowGapModel(int rowIndex, const QList<U2MsaGap>& gapModel) {
    SAFE_POINT(rowIndex >= 0 && rowIndex < getNumRows(), "Invalid row index!", );
    MAlignmentRow& row = rows[rowIndex];
    length = qMax(length, MsaRowUtils::getGapsLength(gapModel) + row.sequence.length());
    row.setGapModel(gapModel);
}

void MAlignment::setRowId(int rowIndex, qint64 rowId) {
    SAFE_POINT(rowIndex >= 0 && rowIndex < getNumRows(), "Invalid row index!", );

    MAlignmentRow& row = rows[rowIndex];
    row.setRowId(rowId);
}

void MAlignment::setSequenceId(int rowIndex, U2DataId sequenceId) {
    SAFE_POINT(rowIndex >= 0 && rowIndex < getNumRows(), "Invalid row index!", );

    MAlignmentRow& row = rows[rowIndex];
    row.setSequenceId(sequenceId);
}

void MAlignment::check() const {
#ifdef DEBUG
    assert(getNumRows() != 0 || length == 0);
    for (int i = 0, n = getNumRows(); i < n; i++) {
        const MAlignmentRow& row = rows.at(i);
        assert(row.getCoreEnd() <= length);
    }
#endif
}

bool MAlignment::sortRowsByList(const QStringList& rowsOrder) {
    MAStateCheck check(this);

    const QStringList& rowNames = getRowNames();
    foreach(const QString& rowName, rowNames) {
        CHECK(rowsOrder.contains(rowName), false);
    }

    QList<MAlignmentRow> sortedRows;
    foreach(const QString& rowName, rowsOrder) {
        int rowIndex = rowNames.indexOf(rowName);
        if(rowIndex >= 0) {
            const MAlignmentRow& curRow = rows.at(rowIndex);
            sortedRows.append(curRow);
        }
    }

    rows = sortedRows;
    return true;
}

const MAlignmentRow& MAlignment::getRow( QString name ) const{
    static MAlignmentRow emptyRow;
    for(int i = 0;i < rows.count();i++){
        if(rows.at(i).getName() == name){
            return rows.at(i);
        }
    }
    SAFE_POINT(false,
        "Internal error: row name passed to MAlignmnet::getRow function not exists!",
        emptyRow);
}

static bool _registerMeta() {
    qRegisterMetaType<MAlignment>("MAlignment");
    return true;
}

bool MAlignment::registerMeta = _registerMeta();

} // namespace U2
