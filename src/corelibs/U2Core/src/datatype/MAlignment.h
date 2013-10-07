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

#ifndef _U2_MALIGNMENT_H_
#define _U2_MALIGNMENT_H_

#include "MAlignmentInfo.h"

#include <U2Core/DNASequence.h>
#include <U2Core/MsaRowUtils.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2SafePoints.h>


namespace U2 {

/** Default name for a multiple alignment */
#define MA_OBJECT_NAME QString("Multiple alignment")

/** Gap character */
#define MAlignment_GapChar '-'

class DNAAlphabet;
class MAlignment;

/**
 * A row in a multiple alignment structure.
 * The row consists of a sequence without gaps
 * and a gap model.
 * A row core is an obsolete concept. Currently,
 * it exactly equals to the row (offset always equals to zero).
 */
class U2CORE_EXPORT MAlignmentRow {
    friend class MAlignment;

public:
    /** Name of the row (equals to the sequence name), can be empty */
    QString getName() const { return sequence.getName(); }
    void setName(const QString& name) { sequence.setName(name); }

    /** Returns the list of gaps for the row */
    inline const QList<U2MsaGap>& getGapModel() const;

    /** Careful, the new gap model is not validated! */
    void setGapModel(const QList<U2MsaGap>& newGapModel) { gaps = newGapModel; }

    /** Returns the row sequence (without gaps) */
    inline const DNASequence& getSequence() const;

    /**
     * Sets a new sequence. Be careful, gap model validity is not verified.
     * The sequence must not contain gaps.
     */
    void setSequence(const DNASequence& newSequence);

    /** Returns ID of the row in the database. */
    qint64 getRowId() const { return initialRowInDb.rowId; }

    void setRowId(qint64 rowId) { initialRowInDb.rowId = rowId; }

    void setSequenceId(U2DataId sequenceId) { initialRowInDb.sequenceId = sequenceId; }

    /** Returns ID of the row sequence in the database. */
    U2MsaRow getRowDBInfo() const;

    /**
     * The length must be greater or equal to the row length.
     * When the specified length is greater, an appropriate number of
     * trailing gaps are appended to the end of the byte array.
     */
    QByteArray toByteArray(int length, U2OpStatus& os) const;

    /** Returns length of the sequence + number of gaps including trailing gaps (if any) */
    inline int getRowLength() const;

    /** Packed version. Client must apply offsets manually */
    QByteArray getCore() const;

    /** Obsolete. Always return the row length (non-inclusive!) */
    inline int getCoreEnd() const;

    /** Obsolete. Always returns zero. */
    inline int getCoreStart() const;

    /** Obsolete. The length of the row core */
    int getCoreLength() const;

    /** Removes all gaps. Returns true if changed. */
    inline bool simplify();

    /** Adds anotherRow data to this row, "lengthBefore" must be greater than this row's length. */
    void append(const MAlignmentRow& anotherRow, int lengthBefore, U2OpStatus& os);

    /**
     * Sets new sequence and gap model.
     * If the sequence is empty, the offset is ignored (if any).
     */
    void setRowContent(const QByteArray& bytes, int offset, U2OpStatus& os);

    /**
     * Inserts 'count' gaps into the specified position, if possible.
     * If position is bigger than the row length or negative, does nothing.
     * Returns incorrect status if 'count' is negative.
     */
    void insertGaps(int pos, int count, U2OpStatus& os);

    /**
     * Removes up to 'count' characters starting from the specified position
     * If position is bigger than the row length, does nothing.
     * Returns incorrect status if 'pos' or 'count' is negative.
     */
    void removeChars(int pos, int count, U2OpStatus& os);

    /**
     * Returns a character in row at the specified position.
     * If the specified position is outside the row bounds, returns a gap.
     */
    char charAt(int pos) const;

    /** Length of the sequence without gaps */
    inline int getUngappedLength() const;

    /**
     * If character at 'pos' position is not a gap, returns the char position in sequence.
     * Otherwise returns '-1'.
     */
    int getUngappedPosition(int pos) const;

    /**
     * Exactly compares the rows. Sequences and gap models must match.
     * However, the rows are considered equal if they differ by trailing gaps only.
     */ 
    bool isRowContentEqual(const MAlignmentRow& row) const;

    /** Compares 2 rows. Rows are equal if their contents and names are equal. */
    inline bool operator!=(const MAlignmentRow& r) const;
    bool operator==(const MAlignmentRow& r) const;

    /**
     * Crops the row -> keeps only specified region in the row.
     * 'pos' and 'pos + count' can be greater than the row length.
     * Keeps trailing gaps.
     */
    void crop(int pos, int count, U2OpStatus& os);

    /**
     * Returns new row of the specified 'count' length, started from 'pos'.
     * 'pos' and 'pos + count' can be greater than the row length.
     * Keeps trailing gaps.
     */
    MAlignmentRow mid(int pos, int count, U2OpStatus& os) const;

    /** Converts the row sequence to upper case */
    void toUpperCase();

    /**
     * Replaces all occurrences of 'origChar' by 'resultChar'.
     * The 'origChar' must be a non-gap character.
     * The 'resultChar' can be a gap, gaps model is recalculated in this case.
     */
    void replaceChars(char origChar, char resultChar, U2OpStatus& os);
    /** Returns length of the sequence + number of gaps. Doesn't include trailing gaps. */
    qint64 getRowLengthWithoutTrailing() const;

    inline static qint64 invalidRowId() { return -1; }

private:
    /** Create a new row (sequence + gap model) from the bytes */
    static MAlignmentRow createRow(const QString& name,
        const QByteArray& bytes,
        U2OpStatus& os);

    /**
     * Sequence must not contain gaps.
     * All gaps in the gaps model (in 'rowInDb') must be valid and have an offset within the bound of the sequence.
     */
    static MAlignmentRow createRow(
        const U2MsaRow& rowInDb,
        const DNASequence& sequence,
        const QList<U2MsaGap>& gaps,
        U2OpStatus& os);

    /** Do NOT create a row without an alignment! */
    MAlignmentRow();

    /** Creates a row in memory. */
    MAlignmentRow(const U2MsaRow& rowInDb, const DNASequence& sequence, const QList<U2MsaGap>& gaps);

    /** Splits input to sequence bytes and gaps model */
    static void splitBytesToCharsAndGaps(const QByteArray& input, QByteArray& seqBytes, QList<U2MsaGap>& gapModel);

    /**
     * Add "offset" of gaps to the beginning of the row
     * Warning: it is not verified that the row sequence is not empty.
     */
    static void addOffsetToGapModel(QList<U2MsaGap>& gapModel, int offset);

    /**
     * Joins sequence chars and gaps into one byte array.
     * "keepOffset" specifies to take into account gaps at the beginning of the row.
     */
    QByteArray joinCharsAndGaps(bool keepOffset) const;

    /** Gets the length of all gaps */
    inline int getGapsLength() const;

    /** If there are consecutive gaps in the gaps model, merges them into one gap */
    void mergeConsecutiveGaps();

    /** The row must not contain trailing gaps, this method is used to assure it after the row modification */
    void removeTrailingGaps();

    /**
     * Calculates start and end position in the sequence,
     * depending on the start position in the row and the 'count' character from it
     */
    void getStartAndEndSequencePositions(int pos, int count, int& startPosInSeq, int& endPosInSeq);

    /** Removing gaps from the row between position 'pos' and 'pos + count' */
    void removeGapsFromGapModel(int pos, int count);

    /** The sequence of the row without gaps (cached) */
    DNASequence         sequence;

    /** Gaps model of the row */
    QList<U2MsaGap>     gaps;

    /** The row in the database */
    U2MsaRow            initialRowInDb;
};


inline int MAlignmentRow::getGapsLength() const {
    return MsaRowUtils::getGapsLength(gaps);
}

inline int MAlignmentRow::getCoreStart() const {
    return 0;
}

inline int MAlignmentRow::getCoreEnd() const {
    return getRowLength();
}



inline int MAlignmentRow::getRowLength() const {
    return MsaRowUtils::getRowLength(sequence.seq, gaps);
}

inline int MAlignmentRow::getUngappedLength() const {
    return sequence.length();
}

inline bool MAlignmentRow::simplify() {
    if (gaps.count() > 0) {
        gaps.clear();
        return true;
    }
    return false;
}

inline const QList<U2MsaGap>& MAlignmentRow::getGapModel() const {
    return gaps;
}

inline const DNASequence& MAlignmentRow::getSequence() const {
    return sequence;
}

inline bool MAlignmentRow::operator!=(const MAlignmentRow& row) const {
    return !(*this == row);
}


/**
 * Multiple sequence alignment
 * The length of the alignment is the maximum length of its rows.
 * There are minimal checks on the alignment's alphabet, but the client of the class
 * is expected to keep the conformance of the data and the alphabet.
 */
class U2CORE_EXPORT MAlignment {
public:
    /**
     * Creates a new alignment.
     * The name must be provided if this is not default alignment.
     */
    MAlignment(const QString& name = QString(), 
               DNAAlphabet* alphabet = NULL,
               const QList<MAlignmentRow>& rows = QList<MAlignmentRow>());
        
    /**
     * Clears the alignment. Makes alignment length == 0.
     * Doesn't change alphabet or name
     */
    void clear();

    /** Returns  the name of the alignment */
    QString getName() const { return MAlignmentInfo::getName(info); }

    /** Sets the name of the alignment */
    void setName(const QString& newName) { MAlignmentInfo::setName(info, newName); }

    /** Returns the alphabet of the alignment */
    DNAAlphabet* getAlphabet() const { return alphabet; }

    /**
     * Sets the alphabet of the alignment, the value can't be NULL.
     * Warning: rows already present in the alignment are not verified to correspond to this alphabet
     */
    void setAlphabet(DNAAlphabet* al);

    /** Returns the alignment info */
    QVariantMap getInfo() const { return info; }

    /** Sets the alignment info */
    void setInfo(const QVariantMap& _info) { info = _info; }

    /** Returns true if the length of the alignment is 0 */
    bool isEmpty() const { return getLength() == 0; }

    /** Returns the length of the alignment */
    int getLength() const { return length; }

    /** Sets the length of the alignment. The length must be >= 0. */
    void setLength(int len);

    /** Returns the number of rows in the alignment */
    int getNumRows() const { return rows.size(); }

    /**
     * Recomputes the length of the alignment and makes it as minimal
     * as possible. All leading gaps columns are removed by default.
     * Returns "true" if the alignment has been modified.
     */
    bool trim( bool removeLeadingGaps = true );

    /**
     * Removes all gaps from all columns in the alignment.
     * Returns "true" if the alignment has been changed.
     */
    bool simplify();

    /** Sorts rows by name */
    void sortRowsByName(bool asc = true);

    /**
     * Sorts rows by similarity making identical rows sequential.
     * Returns 'true' if the rows were resorted, and 'false' otherwise.
     */
    bool sortRowsBySimilarity(QVector<U2Region>& united);

    /** Returns row of the alignment */
    inline const MAlignmentRow& getRow(int row) const;

    const MAlignmentRow& getRow(QString name) const;

    /** Returns all rows in the alignment */
    const QList<MAlignmentRow>& getRows() const { return rows; }

    /** Returns IDs of the alignment rows in the database */
    QList<qint64> getRowsIds() const;

    MAlignmentRow getRowByRowId(qint64 rowId, U2OpStatus& os) const;

    /** Returns all rows' names in the alignment */
    QStringList getRowNames() const;

    int getRowIndexByRowId(qint64 rowId, U2OpStatus &os) const;

    /** Returns a character (a gap or a non-gap) in the specified row and position */
    char charAt(int rowIndex, int pos) const;

    /**
     * Inserts 'count' gaps into the specified position.
     * Can increase the overall alignment length.
     */
    void insertGaps(int row, int pos, int count, U2OpStatus& os);

    /**
     * Removes up to n characters starting from the specified position.
     * Can decrease the overall alignment length.
     */
    void removeChars(int row, int pos, int count, U2OpStatus& os);

    /**
     * Removes a region from the alignment.
     * If "removeEmptyRows" is "true", removes all empty rows from the processed region.
     * The alignment is trimmed after removing the region.
     * Can decrease the overall alignment length.
     */
    void removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows);

    /**
     * Renames the row with the specified index.
     * Assumes that the row index is valid and the name is not empty.
     */
    void renameRow(int row, const QString& name);

    /**
     * Sets the new content for the row with the specified index.
     * Assumes that the row index is valid.
     * Can modify the overall alignment length (increase or decrease).
     */
    void setRowContent(int row, const QByteArray& sequence);

    /** Converts all rows' sequences to upper case */
    void toUpperCase();

    /**
     * Modifies the alignment by keeping data from the specified region and rows only.
     * Assumes that the region start is not negative, but it can be greater than a row length.
     */
    bool crop(const U2Region& region, const QSet<QString>& rowNames);

    /**
     * Creates a new alignment from the sub-alignment. Do not trims the result.
     * Assumes that 'start' >= 0, and 'start + len' is less or equal than the alignment length.
     */
    MAlignment mid(int start, int len) const;


    void setRowGapModel(int rowIndex, const QList<U2MsaGap>& gapModel);

    /** Updates row ID of the row at 'rowIndex' position */
    void setRowId(int rowIndex, qint64 rowId);

    void setSequenceId(int rowIndex, U2DataId sequenceId);

    /**
     * Adds a new row to the alignment.
     * If rowIndex == -1 -> appends the row to the alignment.
     * Otherwise, if rowIndex is incorrect, the closer bound is used (the first or the last row).
     * Does not trim the original alignment.
     * Can increase the overall alignment length.
     */
    void addRow(const QString& name, const QByteArray& bytes, U2OpStatus& os);
    void addRow(const QString& name, const QByteArray& bytes, int rowIndex, U2OpStatus& os);
    void addRow(const U2MsaRow& rowInDb, const DNASequence& sequence, U2OpStatus& os);

    /**
     * Removes a row from alignment.
     * The alignment is changed only (to zero) if the alignment becomes empty.
     */
    void removeRow(int rowIndex, U2OpStatus& os);

    /**
     * Shifts a selection of consequent rows.
     * 'delta' can be positive or negative.
     * It is assumed that indexes of shifted rows are within the bounds of the [0, number of rows).
     */
    void moveRowsBlock(int startRow, int numRows, int delta);

    /**
     * Replaces all occurrences of 'origChar' by 'resultChar' in the row with the specified index.
     * The 'origChar' must be a non-gap character.
     * The 'resultChar' can be a gap, gaps model is recalculated in this case.
     * The index must be valid as well.
     */
    void replaceChars(int row, char origChar, char resultChar);

    /**
     * Appends chars to the row with the specified index.
     * The chars are appended to the alignment end, not to the row end
     * (i.e. the alignment length is taken into account).
     * Does NOT recalculate the alignment length!
     * The index must be valid.
     */
    void appendChars(int row, const char* str, int len);

    /** returns "True" if there are no gaps in the alignment */
    bool hasEmptyGapModel() const;

    /**  returns "True" if all sequences in the alignment have equal lengths */
    bool hasEqualLength() const;

    /**
     * Joins two alignments. Alignments must have the same size and alphabet.
     * Increases the alignment length.
     */
    MAlignment& operator+=(const MAlignment& ma);
    
    /**
     * Compares two alignments: lengths, alphabets, rows and infos (that include names).
     */
    bool operator==(const MAlignment& ma) const;
    bool operator!=(const MAlignment& ma) const;
    

    /** Estimates memory size consumed by alignment structure in bytes */
    int estimateMemorySize() const;

    /** Checks model consistency */
    void check() const;

    /** Arranges rows in lists order*/
    bool sortRowsByList(const QStringList& order);

    QList<QByteArray> toByteArrayList() const;

private:
    /** Helper-method for adding a row to the alignment */
    void addRow(const MAlignmentRow& row, int rowIndex, U2OpStatus& os);

    /** Computes minimal length to hold the whole alignment */
    int calculateMinLength() const;

    /** Alphabet for all sequences in the alignment */
    DNAAlphabet*            alphabet;

    /** Alignment rows (each row = sequence + gap model) */
    QList<MAlignmentRow>    rows;

    /** The length of the longest row in the alignment */
    int                     length;

    /** Additional alignment info */
    QVariantMap             info;

private:
    static bool registerMeta;
};

inline const MAlignmentRow& MAlignment::getRow(int rowIndex) const {
    static MAlignmentRow emptyRow;
    int rowsCount = rows.count();
    SAFE_POINT(0 != rowsCount, "No rows!", emptyRow);
    SAFE_POINT(rowIndex >= 0 && (rowIndex < rowsCount),
        "Internal error: unexpected row index was passed to MAlignmnet::getRow!",
        emptyRow);
    return rows.at(rowIndex);
}


}//namespace

Q_DECLARE_METATYPE(U2::MAlignment)

#endif
