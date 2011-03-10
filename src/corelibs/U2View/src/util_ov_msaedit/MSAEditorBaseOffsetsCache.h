#ifndef _U2_MSA_EDITOR_BASEOFFSET_CACHE_H_
#define _U2_MSA_EDITOR_BASEOFFSET_CACHE_H_

#include <QtCore/QObject>
#include <QtCore/QVector>

namespace U2 {

class MAlignmentObject;
class MAlignment;
class MAlignmentModInfo;

//TODO: optimize this cache example -> use sparse cache for 1 of 100 positions in every sequence

class MSAEditorBaseOffsetCache : public QObject {
    Q_OBJECT
public:
    MSAEditorBaseOffsetCache(QObject* p, MAlignmentObject* aliObj);

    int getBaseCounts(int seqNum, int aliPos, bool inclAliPos);
    
    MAlignmentObject* getMSAObject() const {return aliObj;}
private slots:
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&);

private:
    void updateCacheSize();
    void updateCacheRow(int seqNum);
    int  _getBaseCounts(int seqNum, int aliPos, int& cachedEndPos);

    class RowCache {
    public:
        RowCache() : cacheVersion(0){}
        int          cacheVersion;
        QVector<int> rowOffsets;
    };

    
    MAlignmentObject*       aliObj;
    QVector<RowCache>       cache;
    int                     objVersion;
    int                     globVersion;
};

}//namespace;

#endif
