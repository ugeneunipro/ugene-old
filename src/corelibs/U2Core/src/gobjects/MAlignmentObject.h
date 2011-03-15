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
    MAlignmentModInfo() : sequenceContentChanged(true), sequenceListChanged(true) {}
    bool sequenceContentChanged;
    bool sequenceListChanged;

private:
    static bool registerMeta;
};

class U2CORE_EXPORT MAlignmentObject : public GObject {
    Q_OBJECT
public:

    explicit MAlignmentObject(const MAlignment& a, const QVariantMap& hintsMap = QVariantMap())
        : GObject(GObjectTypes::MULTIPLE_ALIGNMENT, a.getName(), hintsMap), msa(a){};

    const MAlignment& getMAlignment() const {return msa;}

    char charAt(int seqNum, int pos) const {return msa.charAt(seqNum, pos);}

    virtual GObject* clone() const;

    void insertGap(int seqNum, int pos, int nGaps);
    
    void insertGap(int pos, int nGaps);

    void insertGap(U2Region seqences, int pos, int nGaps);

    int deleteGap(int seqNum, int pos, int maxGaps);
    
    int deleteGap(int pos, int maxGaps);

    void addRow(const DNASequence& seq, int seqIdx = -1);

    void removeRow(int seqNum);

    void renameRow(int seqNum, const QString& newName);

    void removeRegion(int startPos, int startRow, int nBases, int nRows, bool removeEmptyRows, bool changeAlignment = true);

    void crop(U2Region window, const QSet<QString>& rowNames);

    void setMAlignment(const MAlignment& ma);

    DNAAlphabet* getAlphabet() const { return msa.getAlphabet(); }

    virtual void setGObjectName(const QString& newName);

    int getLength() const {return msa.getLength();}

    void geleteGapsByAbsoluteVal(int val);
    void geleteGapsByRelativeVal(int val);
    void deleteAllGapColumn();

signals:
    void si_alignmentChanged(const MAlignment& maBefore, const MAlignmentModInfo& modInfo);
    
protected:
    MAlignment msa;
};


}//namespace

#endif
