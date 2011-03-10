#include "MAlignmentObject.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequence.h>
#include <util_algorithm/MSAUtils.h>

namespace U2 {

GObject* MAlignmentObject::clone() const {
    MAlignmentObject* cln = new MAlignmentObject(msa, getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    return cln;
}


void MAlignmentObject::insertGap(int seqNum, int pos, int nGaps) {
    assert(!isStateLocked());
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
    assert(nGaps > 0);
    assert(!isStateLocked());

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

void MAlignmentObject::insertGap( U2Region seqences, int pos, int nGaps )
{
    assert(!isStateLocked());
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
    assert(!isStateLocked());

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
    assert(!isStateLocked());

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
    assert(!isStateLocked());
    MAlignment maBefore = msa;

    DNAAlphabet* newAlphabet = DNAAlphabet::deriveCommonAlphabet(seq.alphabet, getAlphabet());
    assert(newAlphabet != NULL);
    msa.setAlphabet(newAlphabet);

    MAlignmentRow row(seq.getName(), seq.seq, 0);
    msa.addRow(row, seqIdx);
    
    setModified(true);

    MAlignmentModInfo mi;
    emit si_alignmentChanged(maBefore, mi);
}

void MAlignmentObject::removeRow(int seqNum) {
    assert(!isStateLocked());

    MAlignment maBefore = msa;
    msa.removeRow(seqNum);
    setModified(true);

    MAlignmentModInfo mi;
    mi.sequenceContentChanged = false;
    emit si_alignmentChanged(maBefore, mi);
    
}


void MAlignmentObject::setMAlignment(const MAlignment& newMa) {
    assert(!isStateLocked());

    MAlignment maBefore = msa;

    msa = newMa;
    msa.setName( getGObjectName() );
    
    setModified(true);

    MAlignmentModInfo mi;
    emit si_alignmentChanged(maBefore, mi);
}

void MAlignmentObject::setGObjectName(const QString& newName) {
    msa.setName( newName );
    GObject::setGObjectName(newName);
}

void MAlignmentObject::removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows, bool changeAlignment) {
    assert(!isStateLocked());
    MAlignment maBefore = msa;

    msa.removeRegion(startPos, startRow, nBases, nRows, removeEmptyRows);
    
    setModified(true);
    MAlignmentModInfo mi;
    if(changeAlignment) {
        emit si_alignmentChanged(maBefore, mi);
    }

    
}

void MAlignmentObject::renameRow( int seqNum, const QString& newName ) {
    assert(seqNum >= 0 && seqNum < msa.getNumRows());
    assert(!isStateLocked());
    assert(!newName.isEmpty());

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

void MAlignmentObject::crop( U2Region window, const QSet<QString>& rowNames )
{
    assert(!isStateLocked());
    MAlignment maBefore = msa;

    msa.crop(window, rowNames);

    setModified(true);
    MAlignmentModInfo mi;
    emit si_alignmentChanged(maBefore, mi);    
}

void MAlignmentObject::geleteGapsByAbsoluteVal(int val) {
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

    MAlignmentModInfo mi;
    emit si_alignmentChanged(maBefore, mi);   
}

void MAlignmentObject::geleteGapsByRelativeVal(int val) {
    int length = msa.getLength();
    MAlignment maBefore = msa;
    for(int i = 0; i < length; i++) { //columns
        int gapCount = 0;
        for(int j = 0; j < msa.getNumRows(); j++) { //sequences
            if(charAt(j,i) == '-') {
                gapCount++;
            }
        }

        if(100*gapCount/msa.getNumRows() >= val) {
            removeRegion(i, 0, 1, msa.getNumRows(), true, false);
            length--;
            i--;
        }
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


}//namespace


