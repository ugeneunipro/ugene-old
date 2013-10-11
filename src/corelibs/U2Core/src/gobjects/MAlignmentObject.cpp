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
#include <U2Core/U2Mod.h>
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

void MAlignmentObject::updateCachedMAlignment(MAlignmentModInfo mi,
    const QList<qint64> &modifiedRowIds, const QList<qint64> &removedRowIds)
{
    MAlignment maBefore = cachedMAlignment;
    QString oldName = maBefore.getName();

    U2OpStatus2Log os;
    MAlignmentExporter alExporter;
    if ( modifiedRowIds.isEmpty( ) && removedRowIds.isEmpty( ) ) { // suppose that in this case all the alignment has changed
        cachedMAlignment = alExporter.getAlignment(entityRef.dbiRef, entityRef.entityId, os);
        SAFE_POINT_OP(os, );
    } else { // only specified rows were changed
        if ( !removedRowIds.isEmpty( ) ) {
            foreach ( qint64 rowId, removedRowIds ) {
                const int rowIndex = cachedMAlignment.getRowIndexByRowId( rowId, os );
                SAFE_POINT_OP(os, );
                cachedMAlignment.removeRow( rowIndex, os );
                SAFE_POINT_OP(os, );
            }
        }
        if ( !modifiedRowIds.isEmpty( ) ) {
            QList<MAlignmentRowReplacementData> rowsAndSeqs = alExporter.getAlignmentRows(
                entityRef.dbiRef, entityRef.entityId, modifiedRowIds, os);
            SAFE_POINT_OP(os, );
            foreach ( const MAlignmentRowReplacementData &data, rowsAndSeqs ) {
                const int rowIndex = cachedMAlignment.getRowIndexByRowId( data.row.rowId, os );
                SAFE_POINT_OP(os, );
                cachedMAlignment.setRowContent( rowIndex, data.sequence.seq );
                cachedMAlignment.setRowGapModel( rowIndex, data.row.gaps );
                cachedMAlignment.renameRow( rowIndex, data.sequence.getName( ) );
            }
        }
    }

    setModified(true);
    if (mi.middleState == false) {
        emit si_alignmentChanged(maBefore, mi);

        QString newName = cachedMAlignment.getName();
        if (newName != oldName) {
            GObject::setGObjectName(cachedMAlignment.getName());
        }
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

void MAlignmentObject::copyGapModel(const QList<MAlignmentRow> &copyRows) {
    const QList<MAlignmentRow> &oldRows = getMAlignment().getRows();
    SAFE_POINT(oldRows.count() == copyRows.count(), "Different rows count", );

    QMap<qint64, QList<U2MsaGap> > newGapModel;
    QList<MAlignmentRow>::ConstIterator ori = oldRows.begin();
    QList<MAlignmentRow>::ConstIterator cri = copyRows.begin();
    for (; ori != oldRows.end(); ori++, cri++) {
        newGapModel[ori->getRowId()] = cri->getGapModel();
    }

    U2OpStatus2Log os;
    updateGapModel(newGapModel, os);
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
    int startSeq = rows.startPos;
    int endSeq = startSeq + rows.length;

    QList<qint64> rowIdsToInsert;
    for (int i = startSeq; i < endSeq; ++i) {
        qint64 rowId = msa.getRow(i).getRowId();
        rowIdsToInsert.append(rowId);
    }

    U2OpStatus2Log os;
    MsaDbiUtils::insertGaps(entityRef, rowIdsToInsert, pos, count, os);
    SAFE_POINT_OP(os, );

    MAlignmentModInfo mi;
    mi.sequenceListChanged = false;
    updateCachedMAlignment(mi, rowIdsToInsert);
}

int MAlignmentObject::getMaxWidthOfGapRegion( const U2Region &rows, int pos, int maxGaps,
    U2OpStatus &os )
{
    MAlignment msa = getMAlignment( );
    SAFE_POINT_EXT( U2Region( 0, msa.getNumRows( ) ).contains( rows ) && 0 <= pos && 0 <= maxGaps
        && msa.getLength( ) > pos, os.setError( "Illegal parameters of the gap region!" ), 0 );

    const int maxRemovedGaps = qBound(0, maxGaps, msa.getLength() - pos);
    // check if there is nothing to remove
    if ( 0 == maxRemovedGaps ) {
        return 0;
    }

    int removingGapColumnCount = maxRemovedGaps;
    bool isRegionInRowTrailingGaps = true;
    // iterate through given rows to determine the width of the continuous gap region
    for ( int rowCount = rows.startPos; rowCount < rows.endPos( ); ++rowCount ) {
        int gapCountInCurrentRow = 0;
        // iterate through current row bases to determine gap count
        while ( gapCountInCurrentRow < maxRemovedGaps ) {
            const char currentSymbol = msa.charAt( rowCount,
                pos + maxGaps - gapCountInCurrentRow - 1 );
            if ( MAlignment_GapChar != currentSymbol ) {
                break;
            }
            gapCountInCurrentRow++;
        }

        // determine if the given area intersects a row in the area of trailing gaps
        if ( 0 != gapCountInCurrentRow && isRegionInRowTrailingGaps ) {
            int trailingPosition = pos + maxRemovedGaps - gapCountInCurrentRow;
            if ( msa.getLength( ) != trailingPosition ) {
                while ( msa.getLength( ) > trailingPosition && isRegionInRowTrailingGaps ) {
                    isRegionInRowTrailingGaps &= ( MAlignment_GapChar == msa.charAt( rowCount,
                        trailingPosition ) );
                    ++trailingPosition;
                }
            }
        } else if ( isRegionInRowTrailingGaps ) {
            isRegionInRowTrailingGaps = false;
        }

        if ( 0 == gapCountInCurrentRow ) {
            // don't do anything if there is a row without gaps
            return 0;
        }
        removingGapColumnCount = qMin( removingGapColumnCount, gapCountInCurrentRow );
    }

    if ( isRegionInRowTrailingGaps ) {
        return 0;
    }

    return removingGapColumnCount;
}

int MAlignmentObject::deleteGap( const U2Region &rows, int pos, int maxGaps, U2OpStatus &os ) {
    SAFE_POINT( !isStateLocked( ), "Alignment state is locked!", 0 );

    const int removingGapColumnCount = getMaxWidthOfGapRegion( rows, pos, maxGaps, os );
    SAFE_POINT_OP( os, 0 );
    if ( 0 == removingGapColumnCount ) {
        return 0;
    } else if ( removingGapColumnCount < maxGaps ) {
        pos += maxGaps - removingGapColumnCount;
    }
    QList<qint64> modifiedRowIds;
    modifiedRowIds.reserve( rows.length );

    MAlignment msa = getMAlignment( );
    // iterate through given rows to update each of them in DB
    for ( int rowCount = rows.startPos; rowCount < rows.endPos( ); ++rowCount ) {
        msa.removeChars( rowCount, pos, removingGapColumnCount, os );
        CHECK_OP( os, 0 );

        const MAlignmentRow &row = msa.getRow( rowCount );
        MsaDbiUtils::updateRowGapModel( entityRef, row.getRowId( ), row.getGapModel( ), os );
        CHECK_OP( os, 0 );
        modifiedRowIds << row.getRowId( );
    }

    updateCachedMAlignment( MAlignmentModInfo( ), modifiedRowIds );
    return removingGapColumnCount;
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

    QList<qint64> modifiedRowIds;

    QList<qint64> removedRowIds;
    removedRowIds << rowId;

    updateCachedMAlignment(mi, modifiedRowIds, removedRowIds);
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
}

void MAlignmentObject::setGObjectName(const QString& newName) {
    U2OpStatus2Log os;
    MsaDbiUtils::renameMsa(entityRef, newName, os);
    SAFE_POINT_OP(os, );

    updateCachedMAlignment();

    GObject::setGObjectName(newName);
}

QList<qint64> getRowsAffectedByDeletion( const MAlignment &msa,
    const QList<qint64> &removedRowIds )
{
    QList<qint64> rowIdsAffectedByDeletion;
    U2OpStatus2Log os;
    const QList<qint64> msaRows = msa.getRowsIds( );
    int previousRemovedRowIndex = -1;
    foreach ( qint64 removedRowId, removedRowIds ) {
        if ( -1 != previousRemovedRowIndex ) {
            const int currentRemovedRowIndex = msa.getRowIndexByRowId( removedRowId, os );
            SAFE_POINT_OP(os, QList<qint64>( ) );
            SAFE_POINT( currentRemovedRowIndex > previousRemovedRowIndex,
                "Rows order violation!", QList<qint64>( ) );
            const int countOfUnchangedRowsBetween = currentRemovedRowIndex
                - previousRemovedRowIndex - 1;
            if ( 0 < countOfUnchangedRowsBetween ) {
                for ( int middleRowIndex = previousRemovedRowIndex + 1;
                    middleRowIndex < currentRemovedRowIndex; ++middleRowIndex )
                {
                    rowIdsAffectedByDeletion += msaRows[middleRowIndex];
                }
            }
        }
        previousRemovedRowIndex = msa.getRowIndexByRowId( removedRowId, os );
        SAFE_POINT_OP(os, QList<qint64>( ) );
    }
    const int lastDeletedRowIndex = msa.getRowIndexByRowId( removedRowIds.last( ), os );
    SAFE_POINT_OP(os, QList<qint64>( ) );
    if ( lastDeletedRowIndex < msaRows.size( ) - 1 ) { // if the last removed row was not in the bottom of the msa
        rowIdsAffectedByDeletion += msaRows.mid( lastDeletedRowIndex + 1 );
    }
    return rowIdsAffectedByDeletion;
}

template<typename T>
inline QList<T> mergeLists( const QList<T> &first, const QList<T> &second ) {
    QList<T> result = first;
    foreach ( const T &item, second ) {
        if ( !result.contains( item ) ) {
            result.append( item );
        }
    }
    return result;
}

void MAlignmentObject::removeRegion(int startPos, int startRow, int nBases, int nRows,
    bool removeEmptyRows, bool track)
{
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );
    QList<qint64> modifiedRowIds;
    const MAlignment msa = getMAlignment();
    const QList<MAlignmentRow> msaRows = msa.getRows();
    SAFE_POINT(nRows > 0 && startRow >= 0 && startRow + nRows <= msaRows.size(),
        "Invalid parameters!", );
    QList<MAlignmentRow>::ConstIterator it = msaRows.begin() + startRow;
    QList<MAlignmentRow>::ConstIterator end = it + nRows;
    for (; it != end; it++) {
        modifiedRowIds << it->getRowId();
    }
    const bool trimmingIsNeeded = ( 0 == startPos || msa.getLength( ) == startPos + nBases );

    U2OpStatus2Log os;
    MsaDbiUtils::removeRegion(entityRef, modifiedRowIds, startPos, nBases, os);
    SAFE_POINT_OP(os, );

    QList<qint64> trimmedRowIds; // include this rows into modified rows further
    if ( trimmingIsNeeded ) {
        trimmedRowIds = MsaDbiUtils::trim(entityRef, os);
        SAFE_POINT_OP(os, );
    }

    QList<qint64> removedRows;
    if (removeEmptyRows) {
        removedRows = MsaDbiUtils::removeEmptyRows(entityRef, modifiedRowIds, os);
        SAFE_POINT_OP(os, );
        if ( !removedRows.isEmpty( ) ) { // suppose that if at least one row in msa was removed then
            // all the rows below it were changed
            const QList<qint64> rowIdsAffectedByDeletion = getRowsAffectedByDeletion( msa,
                removedRows );
            foreach ( qint64 removedRowId, removedRows ) { // removed rows ain't need to be update
                modifiedRowIds.removeAll( removedRowId );
                trimmedRowIds.removeAll( removedRowId );
            }
            modifiedRowIds = mergeLists( modifiedRowIds, rowIdsAffectedByDeletion );
        }
    }
    modifiedRowIds = mergeLists( modifiedRowIds, trimmedRowIds );
    if (track) {
        updateCachedMAlignment( MAlignmentModInfo( ), modifiedRowIds, removedRows );
    }
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

void MAlignmentObject::deleteColumnWithGaps(int requiredGapCount) {
    MAlignment msa = getMAlignment();
    const int length = msa.getLength();
    if (GAP_COLUMN_ONLY == requiredGapCount) {
        requiredGapCount = msa.getNumRows();
    }
    QList<qint64> colsForDelete;
    for(int i = 0; i < length; i++) { //columns
        int gapCount = 0;
        for(int j = 0; j < msa.getNumRows(); j++) { //sequences
            if(charAt(j,i) == '-') {
                gapCount++;
            }
        }

        if(gapCount >= requiredGapCount) {
            colsForDelete.prepend(i); //invert order
        }
    }
    if (length == colsForDelete.count()) {
        return;
    }
    QList<qint64>::const_iterator column = colsForDelete.constBegin();
    const QList<qint64>::const_iterator end = colsForDelete.constEnd();
    for ( ; column != end; ++column) {
        if (*column >= cachedMAlignment.getLength()) {
            continue;
        }
        removeRegion(*column, 0, 1, msa.getNumRows(), true, (end - 1 == column));
    }
    //removeColumns(colsForDelete, true);
    msa = getMAlignment();
    updateCachedMAlignment();
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

int MAlignmentObject::shiftRegion( int startPos, int startRow, int nBases, int nRows, int shift )
{
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", 0 );
    SAFE_POINT(!isRegionEmpty(startPos, startRow, nBases, nRows), "Region is empty!", 0 );
    SAFE_POINT( 0 <= startPos && 0 <= startRow && 0 < nBases && 0 < nRows,
        "Invalid parameters of selected region encountered", 0 );

    int n = 0;
    if (shift > 0) {
        insertGap(U2Region(startRow,nRows), startPos, shift);
        n = shift;
    } else if ( 0 < startPos ) {
        if (0 > startPos + shift) {
            shift = -startPos;
        }
        U2OpStatus2Log os;
        n = -deleteGap(U2Region(startRow, nRows), startPos + shift, -shift, os);
        SAFE_POINT_OP( os, 0 );
    }
    return n;
}

bool MAlignmentObject::isRegionEmpty(int startPos, int startRow, int numChars, int numRows) const
{
    MAlignment msa = getMAlignment();
    bool isBlockEmpty = true;
    for (int row = startRow; row < startRow + numRows && isBlockEmpty; ++row ) {
        for( int pos = startPos; pos < startPos + numChars; ++pos ) {
            const MAlignmentRow& r = msa.getRows().at(row);
            if (r.charAt(pos) != MAlignment_GapChar) {
                isBlockEmpty = false;
                break;
            }
        }
    }
    return isBlockEmpty;
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

void MAlignmentObject::sortRowsByList(const QStringList& order) {
    GTIMER(c, t, "MAlignmentObject::sortRowsByList");
    SAFE_POINT(!isStateLocked(), "Alignment state is locked!", );

    MAlignment msa = getMAlignment();
    msa.sortRowsByList(order);

    U2OpStatusImpl os;
    MsaDbiUtils::updateRowsOrder(entityRef, msa.getRowsIds(), os);
    SAFE_POINT_OP(os, );

    updateCachedMAlignment();
}

}//namespace
