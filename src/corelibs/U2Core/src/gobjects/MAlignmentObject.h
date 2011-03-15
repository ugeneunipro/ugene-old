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
