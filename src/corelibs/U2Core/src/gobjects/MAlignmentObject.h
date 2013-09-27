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

#ifndef _U2_MALIGNMENT_OBJECT_H_
#define _U2_MALIGNMENT_OBJECT_H_

#include <U2Core/GObject.h>
#include <U2Core/U2Region.h>
#include <U2Core/MAlignment.h>

#include "GObjectTypes.h"

const int GAP_COLUMN_ONLY = -1;

namespace U2 {

class GHints;
class DNASequence;

class MAlignmentModInfo {
public:
    MAlignmentModInfo() : sequenceContentChanged(true), sequenceListChanged(true), middleState(false) {}
    bool sequenceContentChanged;
    bool sequenceListChanged;
    bool middleState;
    QVariantMap hints;

private:
    static bool registerMeta;
};

class MSAMemento{
public:
    ~MSAMemento(){}
private:     
    friend class MAlignmentObject;
    MSAMemento();
    MAlignment getState() const;
    void setState(const MAlignment&);
private:
    MAlignment lastState;
};

class U2CORE_EXPORT MAlignmentObject : public GObject {
    Q_OBJECT

public:
    explicit MAlignmentObject(const QString& name, const U2EntityRef& msaRef, const QVariantMap& hintsMap = QVariantMap());
    ~MAlignmentObject();

    /** Sets type of modifications tracking for the alignment */
    void setTrackMod(U2TrackModType trackMod, U2OpStatus& os);

    MAlignment getMAlignment() const;
    void setMAlignment(const MAlignment& ma, MAlignmentModInfo mi = MAlignmentModInfo(), const QVariantMap& hints = QVariantMap());
    void copyGapModel(const QList<MAlignmentRow> &copyRows);

    /** GObject methods */
    virtual GObject* clone(const U2DbiRef&, U2OpStatus&) const;
    virtual void setGObjectName(const QString& newName);

    /** Const getters */
    char charAt(int seqNum, int pos) const;
    bool isRegionEmpty(int x, int y, int width, int height) const;
    DNAAlphabet* getAlphabet() const;
    qint64 getLength() const;
    qint64 getNumRows() const;
    const MAlignmentRow& getRow(int row) const;

    /** Methods that modify the gap model only */
    void insertGap(U2Region rows, int pos, int nGaps);

    /**
     * Removes gap region that extends from the @pos column and is no longer than @maxGaps.
     * If the region starting from @pos and having width of @maxGaps includes any non-gap symbols
     * then its longest subset starting from @pos and containing gaps only is removed.
     *
     * If the given region is a subset of a trailing gaps area then nothing happens.
     */
    int deleteGap(const U2Region &rows, int pos, int maxGaps, U2OpStatus &os);

    /**
     * Updates a gap model of the alignment.
     * The map must contain valid row IDs and corresponding gap models.
     */
    void updateGapModel(QMap<qint64, QList<U2MsaGap> > rowsGapModel, U2OpStatus& os);

    /** Methods to work with rows */
    void addRow(U2MsaRow& rowInDb, const DNASequence& sequence, int rowIdx = -1);
    void removeRow(int rowIdx);
    void updateRow(int rowIdx, const QString& name, const QByteArray& seqBytes, const QList<U2MsaGap>& gapModel, U2OpStatus& os);
    void renameRow(int rowIdx, const QString& newName);
    void moveRowsBlock(int firstRow, int numRows, int delta);

    /**
     * Updates the rows order.
     * There must be one-to-one correspondence between the specified rows IDs
     * and rows IDs of the alignment.
     */
    void updateRowsOrder(const QList<qint64>& rowIds, U2OpStatus& os);


    /** Method that affect the whole alignment, including sequences */
    void removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows, bool track = true);
    void crop(U2Region window, const QSet<QString>& rowNames);
    /**
     * Performs shift of the region specified by parameters @startPos (leftmost column number),
     * @startRow (top row number), @nBases (region width), @nRows (region height) in no more
     * than @shift bases.
     *
     * @startPos and @startRow must be non-negative numbers, @nBases and @nRows - strictly
     * positive. The sign of @shift parameter specifies the direction of shifting: positive
     * for right direction, negative for left one. If 0 == @shift nothing happens.
     *
     * Shifting to the left may be performed only if a region preceding the selection
     * and having the same height consists of gaps only. In this case selected region
     * is moved to the left in the width of the preceding gap region but no more
     * than |@shift| bases.
     *
     * Returns shift size, besides sign of the returning value specifies direction of the shift
     */
    int shiftRegion(int startPos, int startRow, int nBases, int nRows, int shift);
    void deleteColumnWithGaps(int requiredGapCount = GAP_COLUMN_ONLY);
    void updateCachedMAlignment(MAlignmentModInfo mi = MAlignmentModInfo(),
        const QList<qint64> &modifiedRowIds = QList<qint64>(),
        const QList<qint64> &removedRowIds = QList<qint64>());
    void sortRowsByList(const QStringList& order);

    void saveState();
    void releaseState();

signals:
    void si_alignmentChanged(const MAlignment& maBefore, const MAlignmentModInfo& modInfo);
    void si_completeStateChanged(bool complete);
    
private:
    /**
     * Returns maximum count of subsequent gap columns in the region that starts from column
     * with @pos number, has width of @maxGaps and includes the rows specified by @rows.
     * @maxGaps, @pos are to be non-negative numbers. Gap columns should finish in column
     * having @pos + @maxGaps number, otherwise 0 is returned. If the region is located
     * in the MSA trailing gaps area, then 0 is returned.
     */
    int getMaxWidthOfGapRegion( const U2Region &rows, int pos, int maxGaps, U2OpStatus &os );

    MAlignment cachedMAlignment;
    MSAMemento*     memento;
};


}//namespace

#endif
