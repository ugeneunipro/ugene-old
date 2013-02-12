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

#include "MAlignmentObject.h"

#include <U2Core/DNASequence.h>
#include <U2Core/MAlignmentExporter.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2ObjectDbi.h>
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

    updateCachedMAlignment();
}

MAlignmentObject::~MAlignmentObject(){
    delete memento;
}

void MAlignmentObject::setTrackMod(U2TrackModType trackMod, U2OpStatus& os) {
    // Prepare the connection
    DbiConnection con(entityRef.dbiRef, os);
    CHECK_OP(os, );

    U2ObjectDbi* objDbi = con.dbi->getObjectDbi();
    SAFE_POINT(NULL != objDbi, "NULL Object Dbi!",);

    // Set the new status
    objDbi->setTrackModType(entityRef.entityId, trackMod, os);
}

MAlignment MAlignmentObject::getMAlignment() const {
    return cachedMAlignment;
}

void MAlignmentObject::updateCachedMAlignment(MAlignmentModInfo mi) {
    MAlignment maBefore = cachedMAlignment;

    U2OpStatus2Log os;
    MAlignmentExporter alExporter;
    cachedMAlignment = alExporter.getAlignment(entityRef.dbiRef, entityRef.entityId, os);

    setModified(true);
    if (mi.middleState == false) {
        emit si_alignmentChanged(maBefore, mi);
    }
}

void MAlignmentObject::setMAlignment(const MAlignment& newMa, MAlignmentModInfo mi, const QVariantMap& hints) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    U2OpStatus2Log os;
    MsaDbiUtils::updateMsa(entityRef, newMa, os);
    SAFE_POINT_OP(os, );

    mi.hints = hints;
    updateCachedMAlignment(mi);
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

void MAlignmentObject::insertGap(U2Region rows, int pos, int count) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );
    MAlignment msa = getMAlignment();
    int length = msa.getLength();
    int startSeq = rows.startPos;
    int endSeq = startSeq + rows.length;

    U2OpStatus2Log os;
    QList<qint64> rowIdsToInsert;
    QList<qint64> rowIdsToShift;
    for (int i = 0; i < startSeq; ++i) {
        qint64 rowId = msa.getRow(i).getRowId();
        rowIdsToShift.append(rowId);
    }
    for (int i = startSeq; i < endSeq; ++i) {
        qint64 rowId = msa.getRow(i).getRowId();
        rowIdsToInsert.append(rowId);
    }
    for (int i = endSeq; i < msa.getNumRows(); ++i) {
        qint64 rowId = msa.getRow(i).getRowId();
        rowIdsToShift.append(rowId);
    }

    MsaDbiUtils::insertGaps(entityRef, rowIdsToInsert, pos, count, os);
    SAFE_POINT_OP(os, );

    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    updateCachedMAlignment(mi);
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

void MAlignmentObject::addRow(U2MsaRow& rowInDb, const DNASequence& seq, int rowIdx) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment msa = getMAlignment();

    DNAAlphabet* newAlphabet = U2AlphabetUtils::deriveCommonAlphabet(seq.alphabet, getAlphabet());
    assert(newAlphabet != NULL);
    msa.setAlphabet(newAlphabet);

    U2OpStatus2Log os;
    MsaDbiUtils::addRow(entityRef, rowIdx, rowInDb, os);
    SAFE_POINT_OP(os, );

    updateCachedMAlignment();
}

void MAlignmentObject::removeRow(int rowIdx) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    SAFE_POINT(rowIdx >= 0 && rowIdx < cachedMAlignment.getNumRows(), "Invalid row index!", );
    const MAlignmentRow& row = cachedMAlignment.getRow(rowIdx);
    qint64 rowId = row.getRowDBInfo().rowId;

    U2OpStatus2Log os;
    MsaDbiUtils::removeRow(entityRef, rowId, os);
    SAFE_POINT_OP(os, );

    MAlignmentModInfo mi;
    mi.sequenceContentChanged = false;
    updateCachedMAlignment(mi);
}

void MAlignmentObject::updateRow(int rowIdx, const QString& name, const QByteArray& seqBytes, const QList<U2MsaGap>& gapModel, U2OpStatus& os) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    SAFE_POINT(rowIdx >= 0 && rowIdx < cachedMAlignment.getNumRows(), "Invalid row index!", );
    const MAlignmentRow& row = cachedMAlignment.getRow(rowIdx);
    qint64 rowId = row.getRowDBInfo().rowId;

    MsaDbiUtils::updateRowContent(entityRef, rowId, seqBytes, gapModel, os);
    CHECK_OP(os, );

    MsaDbiUtils::renameRow(entityRef, rowId, name, os);
    CHECK_OP(os, );

    updateCachedMAlignment();
}

void MAlignmentObject::setGObjectName(const QString& newName) {
    U2OpStatus2Log os;
    MsaDbiUtils::renameMsa(entityRef, newName, os);
    SAFE_POINT_OP(os, );

    updateCachedMAlignment();

    GObject::setGObjectName(newName);
}

void MAlignmentObject::removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows, bool changeAlignment) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment msa = getMAlignment();

    msa.removeRegion(startPos, startRow, nBases, nRows, removeEmptyRows);
    MAlignmentModInfo mi;
    if (false == changeAlignment) {
        mi.middleState = true;
    }
    setMAlignment(msa, mi);

    U2OpStatus2Log os;
    MsaDbiUtils::trim(entityRef, os);
    SAFE_POINT_OP(os, );

    //QList<qint64> rowIds;
    //SAFE_POINT(nRows > 0 && startRow >= 0 && startRow + nRows < msa.getNumRows(), "Invalid parameters!", );
    //for (int i = startRow; i < startRow + nRows; ++i) {
    //    qint64 rowId = msa.getRow(i).getRowId();
    //    rowIds.append(rowId);
    //}

    //U2OpStatus2Log os;
    //MsaDbiUtils::removeRegion(entityRef, rowIds, startPos, nBases, os);
    //SAFE_POINT_OP(os, );

    //updateCachedMAlignment();
}

void MAlignmentObject::renameRow(int rowIdx, const QString& newName) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    SAFE_POINT(rowIdx >= 0 && rowIdx < cachedMAlignment.getNumRows(), "Invalid row index!", );
    const MAlignmentRow& row = cachedMAlignment.getRow(rowIdx);
    qint64 rowId = row.getRowDBInfo().rowId;

    U2OpStatus2Log os;
    MsaDbiUtils::renameRow(entityRef, rowId, newName, os);
    SAFE_POINT_OP(os, );

    updateCachedMAlignment();
}

void MAlignmentObject::crop(U2Region window, const QSet<QString>& rowNames) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );
    MAlignment msa = getMAlignment();

    QList<qint64> rowIds;
    for (int i = 0; i < msa.getNumRows(); ++i) {
        QString rowName = msa.getRow(i).getName();
        if (rowNames.contains(rowName)) {
            qint64 rowId = msa.getRow(i).getRowId();
            rowIds.append(rowId);
        }
    }

    U2OpStatus2Log os;
    MsaDbiUtils::crop(entityRef, rowIds, window.startPos, window.length, os);
    SAFE_POINT_OP(os, );

    updateCachedMAlignment();
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
            if (colNumber >= cachedMAlignment.getLength()) {
                continue;
            }
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

void MAlignmentObject::updateGapModel(QMap<qint64, QList<U2MsaGap> > rowsGapModel, U2OpStatus& os) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment msa = getMAlignment();

    foreach (qint64 rowId, rowsGapModel.keys()) {
        if (!msa.getRowsIds().contains(rowId)) {
            os.setError("Can't update gaps of a multiple alignment!");
            return;
        }

        MsaDbiUtils::updateRowGapModel(entityRef, rowId, rowsGapModel.value(rowId), os);
        CHECK_OP(os, );
    }

    updateCachedMAlignment();
}

void MAlignmentObject::moveRowsBlock(int firstRow, int numRows, int shift)
{
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    QList<qint64> rowIds = cachedMAlignment.getRowsIds();
    QList<qint64> rowsToMove;

    for (int i = 0; i < numRows; ++i) {
        rowsToMove << rowIds[firstRow + i];
    }

    U2OpStatusImpl os;
    MsaDbiUtils::moveRows(entityRef, rowsToMove, shift, os);
    CHECK_OP(os, );

    updateCachedMAlignment();
}

void MAlignmentObject::updateRowsOrder(const QList<qint64>& rowIds, U2OpStatus& os) {
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MsaDbiUtils::updateRowsOrder(entityRef, rowIds, os);
    CHECK_OP(os, );

    updateCachedMAlignment();
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
    return cachedMAlignment.getAlphabet();
}

qint64 MAlignmentObject::getLength() const {
    return cachedMAlignment.getLength();
}

qint64 MAlignmentObject::getNumRows() const {
    return cachedMAlignment.getNumRows();
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


