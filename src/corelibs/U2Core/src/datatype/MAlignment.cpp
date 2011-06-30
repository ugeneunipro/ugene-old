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

#include <QtCore/QStack>

#include "MAlignment.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/TextUtils.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
// MAlignmentRow

QByteArray MAlignmentRow::toByteArray(int length) const {
    assert(length >= getCoreEnd());

    if (offset == 0 && sequence.length() == length) {
        return sequence;
    }

    QByteArray result;
    result.resize(length);
    char* data = result.data();
    if (offset > 0) {
        qMemSet(data, MAlignment_GapChar, offset);
    }
    int seqLen = sequence.length();
    qMemCopy(data + offset, sequence.constData(), seqLen);
    int readyLen = offset + seqLen;
    if (readyLen < length) {
        int dlen = length - readyLen;
        qMemSet(data + readyLen, MAlignment_GapChar, dlen);
    }
    return result;
}

void MAlignmentRow::setSequence(const QByteArray& _sequence, int _offset) {
    offset = _offset;
    sequence = _sequence;
}

int MAlignmentRow::getFirstNonGapIdx() const {
    int nonGapIdx = TextUtils::firstIndexOfNotEqualChar(sequence.constData(), sequence.size(), MAlignment_GapChar);
    if (nonGapIdx == -1) {
        return -1;
    }
    return offset + nonGapIdx;
}

int MAlignmentRow::getLastNonGapIdx() const {
    int nonGapIdx = TextUtils::lastIndexOfNotEqualChar(sequence.constData(), sequence.size(), MAlignment_GapChar);
    if (nonGapIdx == -1) {
        return -1;
    }
    return offset + nonGapIdx;
}

void MAlignmentRow::crop(MAlignmentRow* row, int startPos, int len) {
    int coreEnd = row->getCoreEnd();
    if (startPos + len <= row->offset || startPos >= coreEnd) {
        //the result is empty row
        row->offset = 0;
        row->sequence.clear();
    } else if (startPos <= row->offset && startPos + len >= coreEnd) {
        //offset changes, sequence remains
        row->offset -= startPos;
    } else  if (startPos <= row->offset) {
        // the result is a prefix of the sequence
        int prefixLen = len - (row->offset - startPos);
        assert(prefixLen > 0 && prefixLen < row->sequence.length());
        row->offset -= startPos;
        row->sequence = row->sequence.mid(0, prefixLen);
    } else if (startPos > row->offset && startPos + len < coreEnd) {
        // the result is a middle of the sequence
        int seqStartPos = startPos - row->offset;
        row->offset = 0;
        row->sequence = row->sequence.mid(seqStartPos, len);
    } else {
        // the result is a suffix of the sequence
        assert(startPos > row->offset && startPos + len >= coreEnd);
        int suffixStart = startPos - row->offset;
        int suffixLen = row->sequence.length() - suffixStart;
        row->offset = 0;
        row->sequence = row->sequence.mid(suffixStart, suffixLen);
    }
}

void MAlignmentRow::crop(int startPos, int length) {
    crop(this, startPos, length);
}

MAlignmentRow MAlignmentRow::mid(int startPos, int length) const {
    MAlignmentRow res = *this;
    crop(&res, startPos, length);
    return res;
}

void MAlignmentRow::append(const MAlignmentRow& anotherRow, int lengthBefore) {
    assert(lengthBefore >= getCoreEnd());
    if (anotherRow.getCoreLength() == 0) {
        return;
    }
    int coreLenBefore = getCoreLength();
    int coreEndBefore = getCoreEnd();
    int sizeAfter = lengthBefore + anotherRow.offset + anotherRow.sequence.size() - offset;
    sequence.resize(sizeAfter);
    int gapSpaceLen = (lengthBefore - coreEndBefore) + anotherRow.offset;
    if (gapSpaceLen > 0) {
        qMemSet(sequence.data() + coreLenBefore, MAlignment_GapChar, gapSpaceLen);
    }
    qMemCopy(sequence.data() + coreLenBefore + gapSpaceLen, anotherRow.sequence.constData(), anotherRow.sequence.size());
}

bool MAlignmentRow::minimize() {
    int leftMargin = getFirstNonGapIdx() - offset;
    int rightMargin = getLastNonGapIdx() + 1 - offset;
    bool shrinkLeft = leftMargin > 0;
    bool shrinkRight = rightMargin < sequence.size();
    if (!shrinkLeft && !shrinkRight) {
        return false;
    }
    if (shrinkLeft && shrinkRight) {
        offset+=leftMargin;
        sequence = sequence.mid(leftMargin, rightMargin - leftMargin);
    } else if (shrinkLeft) {
        offset+=leftMargin;
        sequence = sequence.mid(leftMargin);
    } else {
        assert(shrinkRight);
        sequence = sequence.mid(0, rightMargin);
    }
    return true;
}

bool MAlignmentRow::simplify() {
    bool changed = false;
    QBitArray gapMap(256);
    gapMap[MAlignment_GapChar] = true;
    int newLen = TextUtils::remove(sequence.data(), sequence.length(), sequence.data(), gapMap);
    if (newLen != sequence.length()) {
        sequence.resize(newLen);
        changed = true;
    }
    changed = changed || offset > 0;
    offset = 0;
    return changed;

}

void MAlignmentRow::insertChar(int pos, char c) {
    int coreEnd = getCoreEnd();
    if (pos > offset && pos < coreEnd) { //insert inside of the sequence
        int seqInsertPos = pos - offset;
        sequence.insert(seqInsertPos, c);
    } else if ( c != MAlignment_GapChar) {
        if (pos >= coreEnd) { //expand sequence right
            int oldSize = sequence.size();
            int newSize = pos - offset;
            int dSize = newSize - oldSize;
            sequence.resize(newSize);
            qMemSet(sequence.data() + oldSize, MAlignment_GapChar, dSize - 1);
            sequence[newSize-1] = c;
        } else { //expand sequence left
            assert(pos <= offset);
            int oldSize = sequence.size();
            int newSize = coreEnd - pos + 1;
            int dSize = newSize - oldSize;
            sequence.resize(newSize);
            memmove(sequence.data() + dSize, sequence.constData(), dSize);
            qMemSet(sequence.data(), MAlignment_GapChar, dSize);
            sequence[0] = c;
            offset = pos;
        }
    }
}

void MAlignmentRow::insertChars(int pos, char c, int count) {
    int coreEnd = getCoreEnd();
    if (pos >= offset && pos < coreEnd) { //insert inside of the sequence
        int seqInsertPos = pos - offset;
        int suffixLen = sequence.size() - seqInsertPos;
        sequence.resize(sequence.size() + count);
        memmove(sequence.data() + pos + count, sequence.constData() + pos, suffixLen);
        qMemSet(sequence.data() + pos, c, count);
    } else if ( c != MAlignment_GapChar) {
        if (pos >= coreEnd) { //expand sequence right
            int oldSize = sequence.size();
            int newSize = pos - offset + count;
            int dSize = newSize - oldSize;
            sequence.resize(newSize);
            qMemSet(sequence.data() + oldSize, MAlignment_GapChar, dSize - count);
            qMemSet(sequence.data() + oldSize + dSize - count, c, count);
        } else { //expand sequence left
            assert(pos <= offset);
            int oldSize = sequence.size();
            int newSize = coreEnd - pos + count;
            int dSize = newSize - oldSize;
            sequence.resize(newSize);
            memmove(sequence.data() + dSize, sequence.constData(), dSize);
            qMemSet(sequence.data(), c, count);
            qMemSet(sequence.data() + count, MAlignment_GapChar, dSize - count);
            offset = pos;
        }
    } else if (pos >= coreEnd) {
        int oldSize = sequence.size();
        int newSize = pos - offset + count;
        int dSize = newSize - oldSize;
        sequence.resize(newSize);
        qMemSet(sequence.data() + oldSize, MAlignment_GapChar, dSize - count);
        qMemSet(sequence.data() + oldSize + dSize - count, c, count);
    } else  { // pos < offset
        offset+=count;    
    } 
}

void MAlignmentRow::insertChars(int pos, const char* str, int len) {
    int coreEnd = getCoreEnd();
    if (pos > offset && pos < coreEnd) { //insert inside of the sequence
        int seqInsertPos = pos - offset;
        int suffixLen = sequence.size() - seqInsertPos;
        sequence.resize(sequence.size() + len);
        memmove(sequence.data() + pos + len, sequence.constData() + pos, suffixLen);
        qMemCopy(sequence.data() + pos, str, len);
    } else if (pos >= coreEnd) { //expand sequence right
        int oldSize = sequence.size();
        int newSize = pos - offset + len;
        int dSize = newSize - oldSize;
        sequence.resize(newSize);
        qMemSet(sequence.data() + oldSize, MAlignment_GapChar, dSize - len);
        qMemCopy(sequence.data() + oldSize + dSize - len, str, len);
    } else { //expand sequence left
        assert(pos <= offset);
        int oldSize = sequence.size();
        int newSize = coreEnd - pos + len;
        int dSize = newSize - oldSize;
        sequence.resize(newSize);
        memmove(sequence.data() + dSize, sequence.constData(), dSize);
        qMemCopy(sequence.data(), str, len);
        qMemSet(sequence.data() + len, MAlignment_GapChar, dSize - len);
        offset = pos;
    }
}


// Removes up to n characters starting from the specified position
void MAlignmentRow::removeChars(int pos, int n) {
    int coreEnd = getCoreEnd();
    assert(pos >=0 && n>=0 && pos + n <= coreEnd);
    if (pos >= coreEnd) {  //out of the sequence range -> nothing to do
        //empty
    } else if (pos + n < offset) {  // before the sequence -> reduce offset
        offset-=n;
    } else if (pos  < offset) { //left clip
        offset = 0;
        int dLen = n - offset;
        sequence = sequence.mid(dLen);
    } else if (n < sequence.length()) { //mid clip
        int startClip = pos - offset;
        sequence = sequence.remove(startClip, n);
        QByteArray gaps(n, '-');
        //gaps.fill('-');
        sequence.append(gaps);
    } else { //right clip
        assert(pos + n >= coreEnd);
        int newLen = pos - offset;
        sequence = sequence.mid(0, newLen);
    }
}


void MAlignmentRow::removeChars(const QList<int>& _positions) {
    QList<int> positions = _positions;
    qSort(positions);
    bool sequenceChanged = false;
    QBitArray removedMap(256);
    removedMap[0] = true;
    for (int i = positions.size(); --i>=0; ) {
        int pos = positions.at(i);
        if (pos >= getCoreEnd()) {
            continue;;
        }
        if (pos < getCoreStart()) {
            offset-=(i + 1);
            break;
        }
        sequenceChanged = true;
        sequence[pos - offset] = 0;
    }
    if (sequenceChanged) {
        int newLen = TextUtils::remove(sequence.data(), sequence.length(), removedMap);
        assert(newLen < sequence.length());
        sequence.resize(newLen);
    }
}

void MAlignmentRow::setName(const QString& newName) {
    assert(!newName.isEmpty());
    name = newName;
}

bool MAlignmentRow::operator==(const MAlignmentRow& row) const {
    if (row.name != this->name) {
        return false;
    }
    if (offset == row.offset && sequence == row.sequence) {
        return true;
    }
    int ngs1 = getFirstNonGapIdx();
    int ngs2 = row.getFirstNonGapIdx();
    if (ngs1 != ngs2) {
        return false;
    }
    int nge1 = getLastNonGapIdx();
    int nge2 = row.getLastNonGapIdx();
    if (nge1 != nge2) {
        return false;
    }
    bool equal = true;
    for (int i = ngs1 + 1; i < nge1 && equal; i++) {
        char c1 = chatAt(i);
        char c2 = row.chatAt(i);
        equal = c1 ==  c2;
    }
    return equal;
}

void MAlignmentRow::setQuality( const DNAQuality& quality )
{
    assert(quality.qualCodes.length() == sequence.length());
    if (quality.qualCodes.length() == sequence.length()) {
        dnaQuality = quality;
    }
}

//////////////////////////////////////////////////////////////////////////
// MAlignment impl

//helper class to call MAlignment state check
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

MAlignment::MAlignment(const QString& _name, DNAAlphabet* al, const QList<MAlignmentRow>& r) 
: alphabet(al), rows(r)
{
    MAStateCheck check(this);

    assert(al==NULL || !_name.isEmpty()); //if not default constructor -> name must be provided
    setName(_name );
    length = 0;
    for (int i=0, n = rows.size(); i < n; i++) {
        const MAlignmentRow& r = rows.at(i);
        length = qMax(length, r.getCoreEnd());
    }
}

bool MAlignment::trim() {
    MAStateCheck check(this);

    int firstNonGap = length - 1;
    int lastNonGap = 0;

    for (int i=0, n = rows.size(); i < n; i++) {
        const MAlignmentRow& r = rows.at(i);
        int firstRowNonGap = r.getFirstNonGapIdx();
        if (firstRowNonGap == -1) {
            continue;
        }
        int lastRowNonGap = r.getLastNonGapIdx();
        assert(lastRowNonGap!=-1);
        firstNonGap = qMin(firstNonGap, firstRowNonGap);
        lastNonGap = qMax(lastNonGap, lastRowNonGap);
    }
    if (firstNonGap == 0 && lastNonGap == length - 1) {
        return false; //nothing to trim
    }
    int resultLen = lastNonGap - firstNonGap + 1;
    for (int i=0, n = rows.size(); i < n; i++) {
        MAlignmentRow& r = rows[i];
        //r.crop(firstNonGap + 1, resultLen);
        r.crop(firstNonGap, resultLen);
    }
    length = resultLen;
    return true;
}

bool MAlignment::simplify() {
    MAStateCheck check(this);

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


bool MAlignment::hasGaps() const {
    for (int i=0, n = rows.size();  i < n; i++) {
        const MAlignmentRow& row = rows.at(i);
        if (row.getCoreStart() > 0 || row.getCoreEnd() < length) {
            return true;
        }
        if (row.getFirstNonGapIdx() != row.getCoreStart()) {
            return true;
        }
        if (row.getLastNonGapIdx() != row.getCoreEnd() - 1) {
            return true;
        }
    }
    return false;
}

void MAlignment::clear() {
    MAStateCheck check(this);

    rows.clear();
    length = 0;
}


MAlignment MAlignment::mid(int start, int len) const {
    assert(start >=0 && start + len <= length);

    MAlignment res(getName(), alphabet);
    MAStateCheck check(&res);

    foreach(const MAlignmentRow& r, rows) {
        MAlignmentRow mRow = r.mid(start, len);
        res.rows.append(mRow);
    }
    res.length = len;
    return res;
}


MAlignment& MAlignment::operator+=(const MAlignment& ma) {
    MAStateCheck check(this);

    assert(ma.alphabet == alphabet);
    int nSeq = getNumRows();
    assert(ma.getNumRows() == nSeq);
    
    for (int i=0; i < nSeq; i++) {
        MAlignmentRow& myRow = rows[i];
        const MAlignmentRow& anotherRow = ma.rows.at(i);
        myRow.append(anotherRow, length);
    }
    length += ma.length;
    return *this;
}

int MAlignment::estimateMemorySize() const {
    int result = info.size() * 20; //approximate info size estimation
    foreach(const MAlignmentRow& r, rows) {
        result += r.getCoreLength() + getName().length() * 2  + 12; //+12 -> overhead for byte array
    }
    return result;
}

bool MAlignment::crop(const U2Region& range, const QSet<QString>& seqNames ){
    MAStateCheck check(this);

    QList<MAlignmentRow> newList, oldList = rows;
    foreach(const MAlignmentRow& row, rows){
        const QString& rowName = row.getName();
        if (seqNames.contains(rowName)){
            MAlignmentRow newRow = row.mid(range.startPos, range.length);
            newList.append(newRow);
        }
    }
    //TODO: is modified?
    rows = newList;
    length = range.length;
    return true;
}

bool MAlignment::operator!=( const MAlignment& other ) const {
    return length!=other.length || alphabet != other.alphabet || rows != other.rows;
}

void MAlignment::addRow(const MAlignmentRow& row, int rowIndex) {
    MAStateCheck check(this);

    length = qMax(row.getCoreEnd(), length);
    int idx = rowIndex == -1 ? getNumRows() : qBound(0, rowIndex, getNumRows());
    rows.insert(idx, row);
}

void MAlignment::removeRow(int rowIndex) {
    MAStateCheck check(this);

    rows.removeAt(rowIndex);
    if (rows.isEmpty()) {
        length = 0;
    }
}

void MAlignment::insertChar(int row, int pos, char c) {
    assert(alphabet->contains(c));
    assert(row < getNumRows() && row >=0 );
    assert(pos < length && pos >=0);

    MAStateCheck check(this);
    MAlignmentRow& r = rows[row];
    r.insertChar(pos, c);
    length = qMax(length, r.getCoreEnd());
}

void MAlignment::insertChars(int row, int pos, char c, int count) {
    assert(alphabet->contains(c));
    assert(row < getNumRows() && row >=0 );
    assert(pos <= length && pos >=0);

    MAStateCheck check(this);
    MAlignmentRow& r = rows[row];
    r.insertChars(pos, c, count);
    length = qMax(length, r.getCoreEnd());
}

void MAlignment::appendChars(int row, const char* str, int len) {
    const MAlignmentRow& r = getRow(row);
    insertChars(row, r.getCoreEnd(), str, len);
}

void MAlignment::insertChars(int row, int pos, const char* str, int len) {
    assert(row < getNumRows() && row >=0 );
    assert(pos <= length && pos >=0);

    MAStateCheck check(this);
    MAlignmentRow& r = rows[row];
    r.insertChars(pos, str, len);
    length = qMax(length, r.getCoreEnd());
}

int MAlignment::calculateMinLength() const {
    int res = 0;
    foreach(const MAlignmentRow& row, rows) {
        res = qMax(row.getLastNonGapIdx() + 1, res);
    }
    return res;
}

void MAlignment::removeChars(int row, int pos, int n) {
    assert(row >= 0 && row < getNumRows());
    assert(pos >= 0 && pos + n <= length && n > 0 && n > 0);

    MAStateCheck check(this);

    MAlignmentRow& r = rows[row];
    bool lengthHolder = length == r.getCoreEnd();
    r.removeChars(pos, n);
    if (lengthHolder) {
        length = qMax(length - n, calculateMinLength());
    }
}

// Removes all columns from the row
void MAlignment::removeChars(int row, const QList<int>& columns) {
    int n = columns.length();
    assert(row >=0 && row < getNumRows());
    assert(n > 0);

    MAStateCheck check(this);

    MAlignmentRow& r = rows[row];
    bool lengthHolder = length == r.getCoreEnd();
    r.removeChars(columns);
    if (lengthHolder) {
        length = qMax(length - n, calculateMinLength());
    }
}



void MAlignment::removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows) {
    assert(startPos >= 0 && startPos + nBases <= length && nBases > 0);
    assert(startRow >= 0 && startRow + nRows <= getNumRows() && nRows > 0);

    MAStateCheck check(this);
    bool lengthHolder = false;
    for (int i = startRow + nRows; --i >= startRow;) {
        MAlignmentRow& r = rows[i];
        lengthHolder = lengthHolder || length == r.getCoreEnd();
        r.removeChars(startPos, nBases);
        if (removeEmptyRows && r.getCoreLength() == 0) {
            rows.removeAt(i);
        }
    }

    trim();

    if (lengthHolder) {
        length = qMax(length - nBases, calculateMinLength());
    }
}

void MAlignment::setLength(int newLength) {
    assert(newLength >=0);

    MAStateCheck check(this);

    if (newLength >= length) {
        length = newLength;
        return;
    }
    for (int i=0, n = getNumRows(); i < n; i++) {
        MAlignmentRow& row = rows[i];
        row.crop(0, newLength);
    }
    length = newLength;
}

void MAlignment::renameRow(int row, const QString& name) {
    assert(row >= 0 && row < getNumRows());
    assert(!name.isEmpty());
    MAlignmentRow& r = rows[row];
    r.setName(name);
}

void MAlignment::setAlphabet(DNAAlphabet* al) {
    assert(al!=NULL);
    alphabet = al;
}


void MAlignment::replaceChars(int row, char origChar, char resultChar) {
    assert(row >= 0 && row < getNumRows());
    if (origChar == resultChar) {
        return;
    }
    MAlignmentRow& r = rows[row];
    r.sequence.replace(origChar, resultChar);
}

void MAlignment::setRowSequence(int row, const QByteArray& sequence, int offset) {
    assert(row >= 0 && row < getNumRows());
    MAStateCheck check(this);
    MAlignmentRow& r = rows[row];
    r.setSequence(sequence, offset);
    length = qMax(length, r.getCoreEnd());
}

void MAlignment::toUpperCase() {
    for (int i = 0, n = getNumRows(); i < n; i++) {
        MAlignmentRow& row = rows[i];
        TextUtils::translate(TextUtils::UPPER_CASE_MAP, row.sequence.data(), row.sequence.length());
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


void MAlignment::moveRowsBlock( int startRow, int numRows, int delta )
{
    MAStateCheck check(this);

    // Assumption: numRows is rather big, delta is small (1~2) 
    // It's more optimal to move abs(delta) of rows then the block itself
    
    int i = 0;
    int k = qAbs(delta);

    assert ( ( delta > 0 && startRow + numRows + delta - 1 < rows.length() )
        || (delta < 0 && startRow + delta >= 0 && startRow + qAbs(delta) <= rows.length() )  );

    QStack<MAlignmentRow> toMove;
    int fromRow = delta > 0 ? startRow + numRows  : startRow + delta; 

    while (i <  k ) {
        MAlignmentRow row = rows.takeAt(fromRow);
        toMove.push(row);
        i++;
    }
    
    int toRow = delta > 0 ? startRow : startRow + numRows - k;

    while ( toMove.count() > 0 ) {
        MAlignmentRow row = toMove.pop();
        rows.insert(toRow, row);
    }
    
}


QStringList MAlignment::getRowNames() const {
    QStringList rowNames;
    foreach(const MAlignmentRow& r, rows) {
        rowNames.append(r.getName());
    }
    return rowNames;
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

static bool _registerMeta() {
    qRegisterMetaType<MAlignment>("MAlignment");
    return true;
}

bool MAlignment::registerMeta = _registerMeta();

}//namespace U2
