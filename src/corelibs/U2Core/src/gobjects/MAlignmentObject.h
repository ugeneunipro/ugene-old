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
    int deleteGap(int seqNum, int pos, int maxGaps);
    int deleteGap(int pos, int maxGaps);
    void deleteAllGapColumn();

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
    void removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows);
    void crop(U2Region window, const QSet<QString>& rowNames);
    bool shiftRegion(int startPos, int startRow, int nBases, int nRows, int shift);
    void deleteGapsByAbsoluteVal(int val);
    void updateCachedMAlignment(MAlignmentModInfo mi = MAlignmentModInfo());
    void sortRowsByList(const QStringList& order);

    void saveState();
    void releaseState();

signals:
    void si_alignmentChanged(const MAlignment& maBefore, const MAlignmentModInfo& modInfo);
    void si_completeStateChanged(bool complete);
    
private:
    MAlignment cachedMAlignment;
    MSAMemento*     memento;
};


}//namespace

#endif
