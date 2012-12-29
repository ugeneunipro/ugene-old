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

#include "MAlignmentObject.h"

#include <U2Core/DNASequence.h>
#include <U2Core/MAlignmentExporter.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>


namespace U2 {

MSAMemento::MSAMemento():lastState(MAlignment()){}

MAlignment MSAMemento::getState() const{
    return lastState;
}

void MSAMemento::setState(const MAlignment& state){
    lastState = state;
}


MAlignmentObject::MAlignmentObject(const QString& name, const U2EntityRef& msaRef, const QVariantMap& hintsMap /* = QVariantMap */)
    : GObject(GObjectTypes::MULTIPLE_ALIGNMENT, name, hintsMap),
      cachedMAlignment(MAlignment()),
      memento(new MSAMemento)
{
    entityRef = msaRef;

    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os,);

    MAlignmentExporter alExporter;
    cachedMAlignment = alExporter.getAlignment(entityRef.dbiRef, entityRef.entityId, os);
}

MAlignmentObject::~MAlignmentObject(){
    delete memento;
}

MAlignment MAlignmentObject::getMAlignment() const {
    return cachedMAlignment;
}

void MAlignmentObject::setMAlignment(const MAlignment& newMa, MAlignmentModInfo mi, const QVariantMap& hints) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment maBefore = cachedMAlignment;

    U2OpStatus2Log os;
    MAlignmentImporter::updateAlignment(entityRef, newMa, os);

    setModified(true);
    cachedMAlignment = newMa;

    mi.hints = hints;
    if (mi.middleState == false) {
        emit si_alignmentChanged(maBefore, mi);
    }
}

char MAlignmentObject::charAt(int seqNum, int pos) const {
    MAlignment msa = getMAlignment();
    return msa.charAt(seqNum, pos);
}

void MAlignmentObject::saveState(){
    MAlignment msa = getMAlignment();
    emit si_completeStateChanged(false);
    memento->setState(msa);
}

void MAlignmentObject::releaseState(){
    if(!isStateLocked()) {
        emit si_completeStateChanged(true);

        MAlignment maBefore = memento->getState();
        setModified(true);
        MAlignmentModInfo mi;

        emit si_alignmentChanged(maBefore, mi);
    }
}


GObject* MAlignmentObject::clone(const U2DbiRef& dbiRef, U2OpStatus& os) const {
    MAlignment msa = getMAlignment();
    U2EntityRef clonedMsaRef = MAlignmentImporter::createAlignment(dbiRef, msa, os);
    CHECK_OP(os, NULL);

    MAlignmentObject* clonedObj = new MAlignmentObject(msa.getName(), clonedMsaRef, getGHintsMap());
    clonedObj->setIndexInfo(getIndexInfo());
    return clonedObj;
}


void MAlignmentObject::insertGap(int seqNum, int pos, int nGaps) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );
    MAlignment msa = getMAlignment();

    int length = msa.getLength();
    U2OpStatus2Log os;
    for(int i = 0; i < seqNum; i++) {
        msa.insertGaps(i, length, nGaps, os);
    }
    msa.insertGaps(seqNum, pos, nGaps, os);
    for(int i = seqNum + 1; i < msa.getNumRows(); i++) {
        msa.insertGaps(i, length, nGaps, os);
    }
    msa.trim();

    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    setMAlignment(msa, mi);
}

void MAlignmentObject::insertGap(int pos, int nGaps) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );
    SAFE_POINT(nGaps > 0, "Invalid number of gaps!",);

    MAlignment msa = getMAlignment();
    U2OpStatus2Log os;
    for (int i=0, n = msa.getNumRows(); i < n; i++) {
        msa.insertGaps(i, pos, nGaps, os);
    }

    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    setMAlignment(msa, mi);
}

void MAlignmentObject::insertGap( U2Region seqences, int pos, int nGaps ) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );
    MAlignment msa = getMAlignment();
    int length = msa.getLength();
    int startSeq = seqences.startPos;
    int endSeq = startSeq + seqences.length;

    U2OpStatus2Log os;
    for(int i = 0; i < startSeq; i++) {
        msa.insertGaps(i, length, nGaps, os);
    }
    
    for (int i = startSeq; i < endSeq; ++i ) {
        msa.insertGaps( i, pos, nGaps, os);
    }
    
    for(int i = endSeq; i < msa.getNumRows(); i++) {
        msa.insertGaps(i, length, nGaps, os);
    }
    msa.trim();

    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    setMAlignment(msa, mi);
}

int MAlignmentObject::deleteGap(int seqNum, int pos, int maxGaps) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", 0);

    MAlignment msa = getMAlignment();

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
    U2OpStatus2Log os;
    msa.removeChars(seqNum, pos, n, os);

    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    setMAlignment(msa, mi);

    return n;
}

int MAlignmentObject::deleteGap(int pos, int maxGaps) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", 0);

    MAlignment msa = getMAlignment();
    //find min gaps in all sequences starting from pos
    int minGaps = maxGaps;
    int max = qBound(0, maxGaps, msa.getLength() - pos);
    foreach(const MAlignmentRow& row, msa.getRows()) {
        int nGaps = 0;
        for (int i = pos; i < pos + max; i++, nGaps++) {
            if (row.charAt(i) != MAlignment_GapChar) { 
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
    U2OpStatus2Log os;
    for (int i = 0, n = msa.getNumRows(); i < n; i++) {
        msa.removeChars(i, pos, nDeleted, os);
    }
    msa.setLength(msa.getLength() - nDeleted);

    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    setMAlignment(msa, mi);

    return nDeleted;
}


void MAlignmentObject::addRow(const DNASequence& seq, int seqIdx) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment msa = getMAlignment();

    DNAAlphabet* newAlphabet = U2AlphabetUtils::deriveCommonAlphabet(seq.alphabet, getAlphabet());
    assert(newAlphabet != NULL);
    msa.setAlphabet(newAlphabet);

    U2OpStatus2Log os;
    msa.addRow(seq.getName(), seq.seq, seqIdx, os);
    
    setMAlignment(msa);
}

void MAlignmentObject::removeRow(int seqNum) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    U2OpStatus2Log os;
    MAlignment msa = getMAlignment();
    msa.removeRow(seqNum, os);

    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    setMAlignment(msa, mi);
}

void MAlignmentObject::setGObjectName(const QString& newName) {
    MAlignment msa = getMAlignment();
    msa.setName( newName );
    GObject::setGObjectName(newName);
    setMAlignment(msa);
}

void MAlignmentObject::removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows, bool changeAlignment) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment msa = getMAlignment();

    msa.removeRegion(startPos, startRow, nBases, nRows, removeEmptyRows);

    MAlignmentModInfo mi;
    if (changeAlignment == false) {
        mi.middleState = true;
    }
    setMAlignment(msa, mi);
}

void MAlignmentObject::renameRow( int seqNum, const QString& newName ) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );
    MAlignment msa = getMAlignment();

    SAFE_POINT(seqNum >= 0 && seqNum < msa.getNumRows(), QString("Invalid sequence number: %1").arg(seqNum), );
    SAFE_POINT(!newName.isEmpty(), "New sequence name is empty!",);

    const QString& curName = msa.getRow(seqNum).getName();
    if (curName == newName) {
        return;
    }

    msa.renameRow(seqNum, newName);

    setMAlignment(msa);
}

void MAlignmentObject::crop( U2Region window, const QSet<QString>& rowNames ) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment msa = getMAlignment();

    msa.crop(window, rowNames);

    setMAlignment(msa);
}

void MAlignmentObject::deleteGapsByAbsoluteVal(int val) {
    MAlignment msa = getMAlignment();
    int length = msa.getLength();
    QList<int> colsForDelete;
    for(int i = 0; i < length; i++) { //columns
        int gapCount = 0;
        for(int j = 0; j < msa.getNumRows(); j++) { //sequences
            if(charAt(j,i) == '-') {
                gapCount++;
            }
        }

        if(gapCount >= val) {
            colsForDelete.prepend(i);               //invert order
        }
    }
    if (msa.getLength() == colsForDelete.count()) {
        return;
    } else {
        foreach (int colNumber, colsForDelete) {
            removeRegion(colNumber, 0, 1, msa.getNumRows(), true, false);
        }
        msa = getMAlignment();
    }
    setMAlignment(msa);
}

void MAlignmentObject::deleteAllGapColumn() {
    MAlignment msa = getMAlignment();
    int length = msa.getLength();
    for(int i = 0; i < length; i++) { //columns
        int gapCount = 0;
        for(int j = 0; j < msa.getNumRows(); j++) { //sequences
            if(charAt(j,i) == '-') {
                gapCount++;
            }
        }

        if(gapCount == msa.getNumRows()) {
            removeRegion(i, 0, 1, msa.getNumRows(), true, false);
            msa = getMAlignment();
            length--;
            i--;
        }
    }

    setMAlignment(msa);
}

void MAlignmentObject::moveRowsBlock( int firstRow, int numRows, int shift )
{
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment msa = getMAlignment();

    msa.moveRowsBlock(firstRow, numRows, shift);

    setMAlignment(msa);
}


bool MAlignmentObject::shiftRegion( int startPos, int startRow, int nBases, int nRows, int shift )
{
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", false );
    SAFE_POINT(!isRegionEmpty(startPos, startRow, nBases, nRows), "Region is empty!", false );

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
            n += deleteGap(row, startPos + shift, ~shift + 1 /*equivalent to shift * (-1)*/);
        }
    }    

    return n > 0;
}


bool MAlignmentObject::isRegionEmpty(int startPos, int startRow, int numChars, int numRows) const
{
    MAlignment msa = getMAlignment();
    bool emptyBlock = true;
    for (int row = startRow; row < startRow + numRows; ++row ) {
        for( int pos = startPos; pos < startPos + numChars; ++pos ) {
            const MAlignmentRow& r = msa.getRows().at(row);
            if (r.charAt(pos) != MAlignment_GapChar) {
                emptyBlock = false;
                break;
            }
        }
    }

    return emptyBlock;
}

DNAAlphabet* MAlignmentObject::getAlphabet() const {
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, NULL);
    U2AlphabetId alphabetId = con.dbi->getMsaDbi()->getMsaObject(entityRef.entityId, os).alphabet;
    CHECK_OP(os, NULL);

    return U2AlphabetUtils::getById(alphabetId);
}

qint64 MAlignmentObject::getLength() const {
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, -1);

    return con.dbi->getMsaDbi()->getMsaObject(entityRef.entityId, os).length;
}

qint64 MAlignmentObject::getNumRows() const {
    U2OpStatus2Log os;
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, -1);

    return con.dbi->getMsaDbi()->getMsaObject(entityRef.entityId, os).numOfRows;
}

const MAlignmentRow& MAlignmentObject::getRow(int row) const {
    MAlignment msa = getMAlignment();
    return msa.getRow(row);
}

static bool _registerMeta() {
    qRegisterMetaType<MAlignmentModInfo>("MAlignmentModInfo");
    return true;
}

bool MAlignmentModInfo::registerMeta = _registerMeta();

}//namespace


