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

#ifndef _U2_MALIGNMENT_H_
#define _U2_MALIGNMENT_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include "MAlignmentInfo.h"
#include "DNAQuality.h"
#include <QtCore/QStringList>

namespace U2 {

class DNAAlphabet;

// Default name for malignment
#define MA_OBJECT_NAME QString("Multiple alignment")

// Gap character 
#define MAlignment_GapChar '-'

class MAlignment;

//A row in multiple alignment structure
class U2CORE_EXPORT MAlignmentRow {
    friend class MAlignment;
public:
    MAlignmentRow(const QString& _name = QString(),
                  const QByteArray& _sequence = QByteArray(), int _offset = 0)
                  : name(_name), sequence(_sequence), offset(_offset) {}

    // Name of the row
    const QString& getName() const {return name;}

    // unpacked row version: array size == length;
    // All leading & trailing gaps are added to the result array
    // Length must be >= packed array length
    QByteArray toByteArray(int length) const;

    // Packed version. Client must apply offsets manually
    const QByteArray& getCore() const {return sequence;}

    // Start offset of the packed data
    int getCoreStart() const {return offset;}

    // Gets the length of the packed sequence
    int getCoreLength() const {return sequence.size();}

    // Return true if the packed sequence has DNA quality scores
    bool  hasQuality() const { return (!dnaQuality.isEmpty()); }

    // Returns the DNA quality of the packed sequence
    const DNAQuality&  getCoreQuality() const { return dnaQuality; }

    // End offset of the packed data. Not inclusive
    int getCoreEnd() const {return getCoreStart() + getCoreLength();}

    // Sets the packed sequence value and offset
    void setSequence(const QByteArray& coreSequence, int offset = 0);

    // Sets the DNA quality for the packed sequence
    void setQuality(const DNAQuality& quality);

    // Returns symbol in the specified position
    inline char chatAt(int pos) const;

    // Compares 2 rows. Rows are equal if their offsets, packed and names are equal
    inline bool operator!=(const MAlignmentRow& r) const;
    bool operator==(const MAlignmentRow& r) const;
    bool isRowContentEqual(const MAlignmentRow& r) const;

    // The first non-gap character position in the row
    // - 1 if not found
    int getFirstNonGapIdx() const;

    // The last non-gap character in the row
    // -1 if not found
    int getLastNonGapIdx() const;

    // Crops the row -> keeps only specified region in the row
    void crop(int startPos, int length);

    // Returns new row of specified length, started with startPos
    MAlignmentRow mid(int startPos, int length) const;

    // Adds anotherRow data to this raw
    void append(const MAlignmentRow& anotherRow, int lengthBefore);

    // Inserts single character 'c' into specified position.
    void insertChar(int pos, char c);

    // Inserts 'count' characters 'c' into specified position.
    void insertChars(int pos, char c, int count);

    // Inserts string 'str' of length 'len' into specified position.
    void insertChars(int pos, const char* str, int len);

    // Removes single character in the specified position
    void removeChar(int pos) {removeChars(pos, 1);}

    // Removes up to n characters starting from the specified position
    void removeChars(int pos, int n);

    // Removes chars in specified positions
    void removeChars(const QList<int>& positions);

    void setName(const QString& name);

    // Tries to minimize packed data length by removing leading & trailing gaps in packed data.
    // Do not change the content of the item.
    // Return true if succeed
    bool minimize();

    // Removes all gaps. Sets offset to 0
    // Returns true if changed
    bool simplify();

private:
    //helper method, common code for crop & mid
    static void crop(MAlignmentRow* row, int startPos, int length);

    // Row name
    QString     name;
    // Row data, starts with offset
    QByteArray  sequence;
    // DNA sequence quality scores
    DNAQuality  dnaQuality;
    // Offset of the first char in 'sequence'
    int         offset;
};

// Multiple sequence alignment
class U2CORE_EXPORT MAlignment {
public:
    // Constructs new alignment.
    // The length of the alignment == max row length in the list
    MAlignment(const QString& name = QString(), 
                DNAAlphabet* al = NULL,
                const QList<MAlignmentRow>& rows = QList<MAlignmentRow>());
        
    // Clears the alignment. Makes alignment length == 0. Doesn't change alphabet or name
    void clear();

    // Returns  the name of the alignment
    QString getName() const { return MAlignmentInfo::getName(info); }

    // Sets the name of the alignment
    void setName( const QString& newName ) {MAlignmentInfo::setName( info, newName );}

    // Returns alignment alphabet
    DNAAlphabet* getAlphabet() const {return alphabet;}

    void setAlphabet(DNAAlphabet* al);

    QVariantMap getInfo() const {return info;}

    void setInfo(const QVariantMap& _info) {info = _info;}

    // Returns true if alignment length is 0
    bool isEmpty() const {return getLength() == 0;}

    // Returns the length of the alignment
    int getLength() const { return length; }

    // Sets alignment length
    void setLength(int len);

    // Returns number of sequences (rows) in the alignment
    int getNumRows() const {return rows.size();}

    // Returns true if alignment contains gaps
    bool hasGaps() const;
    
    // Recomputes the length of the alignment and makes it as minimal as possible by
    // removing all leading and trailing gap columns
    // Returns true if the alignment is modified
    bool trim();

    // Removes all gaps from all columns in the alignment
    // Returns true if alignment was changed
    bool simplify();

    // Modifies the alignment by keeping data from the specified region and rows only
    bool crop(const U2Region& region, const QSet<QString>& rowNames);

    // creates sub alignment from the alignment. Do not trims the result
    MAlignment mid(int start, int len) const;

    // Adds new row to the alignment
    // Tries to trim the row if possible to avoid alignment change
    // Do not trim the original alignment
    // Note: debug version ensures that row alphabet is the same with alignment alphabet
    // If rowIndex == -1 -> appends row to the alignment
    void addRow(const MAlignmentRow& row, int rowIndex = -1);

    // Removes row from alignment. Does not change the length
    void removeRow(int rowIndex);

    // Joins two alignments. Alignments must have the same size
    MAlignment& operator+=(const MAlignment& ma);

    // Compares two alignments. Only alignments of the same length and content can be equal
    bool operator!=(const MAlignment& ma) const;

    // Returns a symbol in the specified sequence and position
    inline char charAt(int seqNum, int pos) const;

    // Estimates memory size consumed by alignment structure in bytes
    int estimateMemorySize() const;

    // Returns row of the alignment
    const MAlignmentRow& getRow(int row) const {return rows.at(row);}

    // Returns all rows in the alignment
    const QList<MAlignmentRow>& getRows() const {return rows;}

    // Returns all rows in the alignment
    QStringList getRowNames() const;

    // Inserts single character 'c' into specified position.
    // Can increase the overall alignment length
    void insertChar(int row, int pos, char c);

    // Inserts 'count' characters 'c' into specified position.
    // Can increase the overall alignment length
    void insertChars(int row, int pos, char c, int count);

    // Inserts string 'str' of length 'len' into specified position.
    // Can increase the overall alignment length
    void insertChars(int row, int pos, const char* str, int len);

    void appendChars(int row, const char* str, int len);

    // Removes single character in the specified position
    void removeChar(int row, int pos) { removeChars(row, pos, 1); }

    // Removes up to n characters starting from the specified position
    void removeChars(int row, int pos, int n = 1);

    // Removes all columns from the row
    void removeChars(int row, const QList<int>& columns);

    // Removes region from the alignment.
    // If 'removeEmptyRows' is true, removes all empty rows from the processed region
    void removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows);

    // Renames the row
    void renameRow(int row, const QString& name);

    // Sets the row sequence
    void setRowSequence(int row, const QByteArray& sequence, int offset = 0);

    // Replaces chars in the row
    void replaceChars(int row, char origChar, char resultChar);

    // Converts all rows sequences to upper case
    void toUpperCase();

    // Sorts rows by name
    void sortRowsByName(bool asc = true);

    // Sorts rows by similarity making identical rows sequential
    void sortRowsBySimilarity(QVector<U2Region>& united);

    // Shifts a selection of consequent rows
    void moveRowsBlock( int startRow, int numRows, int delta );

    // Checks model consistency
    void check() const;

private:
    // Computes minimal length to hold the whole alignment
    int calculateMinLength() const;

    // Alphabet for all sequences in alignment
    DNAAlphabet*            alphabet;

    // Sequences in the alignment
    QList<MAlignmentRow>    rows;

    // The length of the alignment
    int                     length;

    // Additional alignment info
    QVariantMap             info;

private:
    static bool registerMeta;
};

inline char MAlignment::charAt(int seqNum, int pos) const {
    const MAlignmentRow& mai = rows[seqNum];
    char c = mai.chatAt(pos);
    return c;
}

inline char MAlignmentRow::chatAt(int pos) const {
    int seqPos = pos - offset;
    if (seqPos < 0 || seqPos >= sequence.size()) {
        return MAlignment_GapChar;
    }
    return sequence.at(seqPos);
}

inline bool MAlignmentRow::operator!=(const MAlignmentRow& row) const {
    return !(*this == row);

}


}//namespace

Q_DECLARE_METATYPE(U2::MAlignment)

#endif
