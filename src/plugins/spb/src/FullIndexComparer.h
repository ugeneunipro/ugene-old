#ifndef _SPB_FULL_INDEX_COMPARER_H_
#define _SPB_FULL_INDEX_COMPARER_H_

#include "ComparingAlgorithm.h"

namespace SPB {

class SequenceMapping {
public:
    SequenceMapping();
    ~SequenceMapping();

    void initMapping(int seqCount);
    void clearMapping();
    void createMapping(quint64 start, int num);
    int getSeqByGlobalPos(quint64 pos);

private:
    quint64 *mappingArray;
    int *seqNumsForMapping;
    int mappingSize;
};

class SequencesStorage;

class FullIndexComparer {
public:
    FullIndexComparer(double accuracy, SequencesStorage &sequences, const QString &algoId);
    virtual ~FullIndexComparer();
    void initialize();
    bool isInitialized() const;

    void findSimilars(const QByteArray &seq);
    bool hasSimilars(const QByteArray &seq);
    void removeFoundedSimilars();
    void removeSimilars(int seqNum);

    SequencesStorage & getSequences();

private:
    void calculateWindowSize(quint64 minLength, quint64 maxLength);
    void createIndex();
    void clearAlignInfo();

    int getWindowsSizeByLength(quint64 len);
    int getMatchCountByLength(quint64 len);

private:
    bool initialized;
    SequencesStorage &sequences;
    ComparingAlgorithm *comparer;
    double accuracy;
    QByteArray fullSeq;
    int w;
    SArrayIndex *index;
    SequenceMapping mapping;
    char *founded;
    short *currentSym;
    QByteArray leaderSeq;
};

class SequencesStorage {
public:
    SequencesStorage(const QList<SharedDbiDataHandler> &seqIds);
    void initialize(DbiDataStorage *dbiStorage, U2OpStatus &os);
    bool isInitialized() const;

    int getInitialSeqCount() const;
    int getSeqCount() const;
    bool isEmpty() const;
    const QMap<int, DNASequence> & getSeqMap() const;

    QByteArray & getSequence(int seqNum);
    bool isRemoved(int seqNum) const;
    void removeSequence(int seqNum);

    int getRandomSeqNum() const;

private:
    bool initialized;
    QList<SharedDbiDataHandler> seqIds;
    QMap<int, DNASequence> seqMap;
    QList<int> removed;
};

} // SPB

#endif // _SPB_FULL_INDEX_COMPARER_H_
