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

#include "MAlignmentObject.h"

#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/DNASequence.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/MSAUtils.h>

namespace U2 {

GObject* MAlignmentObject::clone(const U2DbiRef&, U2OpStatus&) const {
    MAlignmentObject* cln = new MAlignmentObject(msa, getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    return cln;
}


void MAlignmentObject::insertGap(int seqNum, int pos, int nGaps) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment maBefore = msa;
    int length = msa.getLength();
    for(int i = 0; i < seqNum; i++) {
        msa.insertChars(i, length, MAlignment_GapChar, nGaps);
    }
    msa.insertChars(seqNum, pos, MAlignment_GapChar, nGaps);
    for(int i = seqNum + 1; i < msa.getNumRows(); i++) {
        msa.insertChars(i, length, MAlignment_GapChar, nGaps);
    }
    msa.trim();

    setModified(true);
    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    emit si_alignmentChanged(maBefore, mi);
}

void MAlignmentObject::insertGap(int pos, int nGaps) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );
    SAFE_POINT(nGaps > 0, "Invalid number of gaps!",);
    
    MAlignment maBefore = msa;
    QByteArray gap(nGaps, MAlignment_GapChar);
    for (int i=0, n = msa.getNumRows(); i < n; i++) {
        msa.insertChars(i, pos, MAlignment_GapChar, nGaps);
    }

    setModified(true);

    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    emit si_alignmentChanged(maBefore, mi);
}

void MAlignmentObject::insertGap( U2Region seqences, int pos, int nGaps ) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );
    MAlignment maBefore = msa;
    int length = msa.getLength();
    int startSeq = seqences.startPos;
    int endSeq = startSeq + seqences.length;

    for(int i = 0; i < startSeq; i++) {
        msa.insertChars(i, length, MAlignment_GapChar, nGaps);
    }
    
    for (int i = startSeq; i < endSeq; ++i ) {
        msa.insertChars( i, pos, MAlignment_GapChar, nGaps);
    }
    
    for(int i = endSeq; i < msa.getNumRows(); i++) {
        msa.insertChars(i, length, MAlignment_GapChar, nGaps);
    }
    msa.trim();

    setModified(true);
    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    emit si_alignmentChanged(maBefore, mi);
}

int MAlignmentObject::deleteGap(int seqNum, int pos, int maxGaps) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", 0);

    MAlignment maBefore = msa;

    int n = 0, max = qBound(0, maxGaps, msa.getLength() - pos);
    while (n < max) {
        char c = msa.charAt(seqNum, pos + n);
        if (c != MAlignment_GapChar) { //not a gap
            break;
        }
        n++;
    }
    if (n == 0) {
        return 0;
    }
    msa.removeChars(seqNum, pos, n);
    
    setModified(true);

    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    emit si_alignmentChanged(maBefore, mi);

    return n;
}

int MAlignmentObject::deleteGap(int pos, int maxGaps) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", 0);

    MAlignment maBefore = msa;
    //find min gaps in all sequences starting from pos
    int minGaps = maxGaps;
    int max = qBound(0, maxGaps, msa.getLength() - pos);
    foreach(const MAlignmentRow& row, msa.getRows()) {
        int nGaps = 0;
        for (int i = pos; i < pos + max; i++, nGaps++) {
            if (row.chatAt(i) != MAlignment_GapChar) { 
                break;
            }
        }
        minGaps = qMin(minGaps, nGaps);
        if (minGaps == 0) {
            break;
        }
    }
    if (minGaps == 0) {
        return  0;
    }
    int nDeleted = minGaps;
    for (int i = 0, n = msa.getNumRows(); i < n; i++) {
        msa.removeChars(i, pos, nDeleted);
    }
    msa.setLength(msa.getLength() - nDeleted);

    setModified(true);

    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    emit si_alignmentChanged(maBefore, mi);

    return nDeleted;
}


void MAlignmentObject::addRow(const DNASequence& seq, int seqIdx) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment maBefore = msa;

    DNAAlphabet* newAlphabet = U2AlphabetUtils::deriveCommonAlphabet(seq.alphabet, getAlphabet());
    assert(newAlphabet != NULL);
    msa.setAlphabet(newAlphabet);

    MAlignmentRow row(seq.getName(), seq.seq, 0);
    msa.addRow(row, seqIdx);
    
    setModified(true);

    MAlignmentModInfo mi;
    emit si_alignmentChanged(maBefore, mi);
}

void MAlignmentObject::removeRow(int seqNum) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment maBefore = msa;
    msa.removeRow(seqNum);
    setModified(true);

    MAlignmentModInfo mi;
    mi.sequenceContentChanged = false;
    emit si_alignmentChanged(maBefore, mi);
    
}


void MAlignmentObject::setMAlignment(const MAlignment& newMa, const QVariantMap& hints) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment maBefore = msa;

    msa = newMa;
    msa.setName( getGObjectName() );
    
    setModified(true);

    MAlignmentModInfo mi;
    mi.hints = hints;
    emit si_alignmentChanged(maBefore, mi);
}

void MAlignmentObject::setGObjectName(const QString& newName) {
    msa.setName( newName );
    GObject::setGObjectName(newName);
}

void MAlignmentObject::removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows, bool changeAlignment) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment maBefore = msa;

    msa.removeRegion(startPos, startRow, nBases, nRows, removeEmptyRows);
    
    setModified(true);
    MAlignmentModInfo mi;
    if(changeAlignment) {
        emit si_alignmentChanged(maBefore, mi);
    }

    
}

void MAlignmentObject::renameRow( int seqNum, const QString& newName ) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );
    SAFE_POINT(seqNum >= 0 && seqNum < msa.getNumRows(), QString("Invalid sequence number: %1").arg(seqNum), );
    SAFE_POINT(!newName.isEmpty(), "New sequence name is empty!",);

    const QString& curName = msa.getRow(seqNum).getName();
    if (curName == newName) {
        return;
    }

    MAlignment maBefore = msa;
    msa.renameRow(seqNum, newName);
    setModified(true);

    MAlignmentModInfo mi;
    emit si_alignmentChanged(maBefore, mi);
}

void MAlignmentObject::crop( U2Region window, const QSet<QString>& rowNames ) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment maBefore = msa;

    msa.crop(window, rowNames);

    setModified(true);
    MAlignmentModInfo mi;
    emit si_alignmentChanged(maBefore, mi);    
}

void MAlignmentObject::deleteGapsByAbsoluteVal(int val) {
    int length = msa.getLength();
    MAlignment maBefore = msa;
    for(int i = 0; i < length; i++) { //columns
        int gapCount = 0;
        for(int j = 0; j < msa.getNumRows(); j++) { //sequences
            if(charAt(j,i) == '-') {
                gapCount++;
            }
        }

        if(gapCount >= val) {
            removeRegion(i, 0, 1, msa.getNumRows(), true, false);
            length--;
            i--;
        }
    }
    
    if (msa.isEmpty()) {
        msa = maBefore;
        return;
    }

    MAlignmentModInfo mi;
    emit si_alignmentChanged(maBefore, mi);   
}

void MAlignmentObject::deleteAllGapColumn() {
    int length = msa.getLength();
    MAlignment maBefore = msa;
    for(int i = 0; i < length; i++) { //columns
        int gapCount = 0;
        for(int j = 0; j < msa.getNumRows(); j++) { //sequences
            if(charAt(j,i) == '-') {
                gapCount++;
            }
        }

        if(gapCount == msa.getNumRows()) {
            removeRegion(i, 0, 1, msa.getNumRows(), true, false);
            length--;
            i--;
        }
    }

    MAlignmentModInfo mi;
    emit si_alignmentChanged(maBefore, mi);   
}

void MAlignmentObject::moveRowsBlock( int firstRow, int numRows, int shift )
{
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment maBefore = msa;

    msa.moveRowsBlock(firstRow, numRows, shift);

    setModified(true);
    MAlignmentModInfo mi;
    emit si_alignmentChanged(maBefore, mi);

}


bool MAlignmentObject::shiftRegion( int startPos, int startRow, int nBases, int nRows, int shift )
{
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", false );
    SAFE_POINT(!isRegionEmpty(startPos, startRow, nBases, nRows), "Region is empty!", false );

    MAlignment maBefore = msa;

    int n = 0;
    if (shift > 0) {
        insertGap(U2Region(startRow,nRows), startPos, shift);
        n = shift;
    } else {
        if (startPos + shift < 0) {
            return false;
        }
        int endRow = startRow + nRows;
        for (int row = startRow; row < endRow; ++row) {
            n += deleteGap(row, startPos + shift, qAbs(shift));
        }
    }

    setModified(true);
    MAlignmentModInfo mi;
    emit si_alignmentChanged(maBefore, mi);
    
    return n > 0;

}


bool MAlignmentObject::isRegionEmpty(int startPos, int startRow, int numChars, int numRows) const
{
    bool emptyBlock = true;
    for (int row = startRow; row < startRow + numRows; ++row ) {
        for( int pos = startPos; pos < startPos + numChars; ++pos ) {
            const MAlignmentRow& r = msa.getRows().at(row);
            if (r.chatAt(pos) != MAlignment_GapChar) {
                emptyBlock = false;
                break;
            }
        }
    }

    return emptyBlock;
}

static bool _registerMeta() {
    qRegisterMetaType<MAlignmentModInfo>("MAlignmentModInfo");
    return true;
}

bool MAlignmentModInfo::registerMeta = _registerMeta();

}//namespace


