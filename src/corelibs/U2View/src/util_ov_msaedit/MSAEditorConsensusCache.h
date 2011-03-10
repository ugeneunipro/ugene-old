#ifndef _U2_MSA_EDITOR_CONSENSUS_CACHE_H_
#define _U2_MSA_EDITOR_CONSENSUS_CACHE_H_

#include <QtCore/QObject>
#include <QtCore/QVector>

namespace U2 {

class MAlignmentObject;
class MAlignment;
class MAlignmentModInfo;
class MSAConsensusAlgorithm;
class MSAConsensusAlgorithmFactory;

class MSAEditorConsensusCache : public QObject {
    Q_OBJECT
public:
    MSAEditorConsensusCache(QObject* p, MAlignmentObject* aliObj, MSAConsensusAlgorithmFactory* algo);
    ~MSAEditorConsensusCache();

    char getConsensusChar(int pos);
    
    int getConsensusCharPercent(int pos);

    void setConsensusAlgorithm(MSAConsensusAlgorithmFactory* algo);

    MSAConsensusAlgorithm* getConsensusAlgorithm() const {return algorithm;}

    QByteArray getConsensusLine(bool withGaps);

private slots:
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&);
    void sl_thresholdChanged(int newValue);

private:
    struct CacheItem {
        CacheItem(int v = 0, char c = '-', int tc = 0) : version(v), topChar(c), topPercent(tc){}
        int     version;
        char    topChar;
        char    topPercent;
    };

    void updateCacheItem(int pos);

    int                     currentVersion;
    QVector<CacheItem>      cache;
    MAlignmentObject*       aliObj;
    MSAConsensusAlgorithm*  algorithm;
};

}//namespace;

#endif
