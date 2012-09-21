#include <U2Core/Timer.h>

#include "FullIndexComparer.h"

namespace SPB {

/************************************************************************/
/* FullIndexComparer */
/************************************************************************/
FullIndexComparer::FullIndexComparer(double _accuracy, SequencesStorage &_sequences, const QString &algoId)
: initialized(false), sequences(_sequences), index(NULL), founded(NULL), currentSym(NULL)
{
    accuracy = _accuracy;
    w = 20;
    comparer = ComparingAlgorithmFactory::createAlgorithm(algoId);
}

FullIndexComparer::~FullIndexComparer() {
    delete index;
    delete[] founded;
    delete[] currentSym;
    delete comparer;
}

void FullIndexComparer::findSimilars(const QByteArray &seq) {
    GTIMER(cvar, tvar, "FullIndexComparingAlgorithm::findSimilars");
    leaderSeq = seq;
    clearAlignInfo();

    const quint32 maxPos = index->getSArraySize();
    SArrayIndex::SAISearchContext context;
    for (int i=0; (i <= leaderSeq.length() - w); i++) {
        const char *s = leaderSeq.constData() + i;
        if (index->find(&context, s)) {
            quint32 pos = index->getSArray()[context.currPos];
            while (-1 != pos) {
                int seqNum = mapping.getSeqByGlobalPos(pos);
                if (currentSym[seqNum] <= i) {
                    founded[seqNum]++;
                    currentSym[seqNum] = i + w;
                }
                pos = index->nextArrSeqPos(&context);
            }
        }
    }
    tvar.stop();
}

bool FullIndexComparer::hasSimilars(const QByteArray &seq) {
    findSimilars(seq);

    int matchCount = getMatchCountByLength(leaderSeq.length());
    for (int i=0; i<sequences.getInitialSeqCount(); i++) {
        if (founded[i] >= matchCount) {
            if (!sequences.isRemoved(i)) {
                double res = comparer->compare(sequences.getSequence(i), leaderSeq);
                if (res >= accuracy) {
                    return true;
                }
            }
        }
    }
    return false;
}

void FullIndexComparer::removeFoundedSimilars() {
    int matchCount = getMatchCountByLength(leaderSeq.length());
    for (int i=0; i<sequences.getInitialSeqCount(); i++) {
        if (founded[i] >= matchCount) {
            if (!sequences.isRemoved(i)) {
                double res = comparer->compare(sequences.getSequence(i), leaderSeq);
                if (res >= accuracy) {
                    sequences.removeSequence(i);
                }
            }
        }
    }
}

void FullIndexComparer::removeSimilars(int leaderIdx) {
    QByteArray leaderSeq = sequences.getSequence(leaderIdx);
    sequences.removeSequence(leaderIdx);

    findSimilars(leaderSeq);
    removeFoundedSimilars();
}

inline int FullIndexComparer::getWindowsSizeByLength(quint64 len) {
    double misRatio = (100.0 - accuracy) / 100;
    int mis = int(len * misRatio);
    return len / (2*mis);
}

inline int FullIndexComparer::getMatchCountByLength(quint64 len) {
    return (len) / (2*w);
}

bool FullIndexComparer::isInitialized() const {
    return initialized;
}

void FullIndexComparer::initialize() {
    GTIMER(cvar, tvar, "FullIndexComparingAlgorithm::initialize");
    delete index; index = NULL;
    mapping.clearMapping();
    createIndex();
    initialized = true;
}

void FullIndexComparer::createIndex() {
    fullSeq.clear();
    const QMap<int, DNASequence> &seqMap = sequences.getSeqMap();
    quint64 minLength = INT_MAX;
    quint64 maxLength = 0;
    mapping.initMapping(seqMap.size());
    foreach (int i, seqMap.keys()) {
        quint64 startPos = fullSeq.length();
        const QByteArray &seq = seqMap.value(i).seq;
        mapping.createMapping(startPos, i);
        fullSeq += seq;
        if (minLength > seq.length()) {
            minLength = seq.length();
        }
        if (maxLength < seq.length()) {
            maxLength = seq.length();
        }
    }
    calculateWindowSize(minLength, maxLength);
    TaskStateInfo os;
    index = new SArrayIndex(fullSeq.constData(), fullSeq.size(), w, os);

    if (NULL == founded) {
        founded = new char[sequences.getInitialSeqCount()];
    }
    if (NULL == currentSym) {
        currentSym = new short[sequences.getInitialSeqCount()];
    }
}

void FullIndexComparer::calculateWindowSize(quint64 minLength, quint64 maxLength) {
    w = INT_MAX;
    for (quint64 len=minLength; len<maxLength; len++) {
        int newW = getWindowsSizeByLength(len);

        if (newW < w) {
            w = newW;
        }
    }
}

void FullIndexComparer::clearAlignInfo() {
    for (int i=0; i<sequences.getInitialSeqCount(); i++) {
        founded[i] = false;
        currentSym[i] = 0;
    }
}

SequencesStorage & FullIndexComparer::getSequences() {
    return sequences;
}

/************************************************************************/
/* SequencesStorage */
/************************************************************************/
SequencesStorage::SequencesStorage(const QList<SharedDbiDataHandler> &_seqIds)
: initialized(false), seqIds(_seqIds)
{
    qsrand(QDateTime::currentDateTime().toTime_t());
}

bool SequencesStorage::isInitialized() const {
    return initialized;
}

void SequencesStorage::initialize(DbiDataStorage *dbiStorage, U2OpStatus &os) {
    GTIMER(cvar, tvar, "SequencesStorage::initialize");
    for (int i=0; i<getInitialSeqCount(); i++) {
        SharedDbiDataHandler id = seqIds.at(i);
        QScopedPointer<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(dbiStorage, id));
        if (NULL == seqObj.data()) {
            os.setError("NULL sequence object");
            return;
        }
        seqMap[i] = seqObj->getWholeSequence();
    }
    initialized = true;
}

inline int SequencesStorage::getInitialSeqCount() const {
    return seqIds.size();
}

int SequencesStorage::getSeqCount() const {
    return seqMap.size();
}

bool SequencesStorage::isEmpty() const {
    return seqMap.isEmpty();
}

inline QByteArray & SequencesStorage::getSequence(int seqNum) {
    return seqMap[seqNum].seq;
}

inline bool SequencesStorage::isRemoved(int seqNum) const {
    return removed.contains(seqNum);
}

inline void SequencesStorage::removeSequence(int seqNum) {
    seqMap.remove(seqNum);
    removed << seqNum;
}

int SequencesStorage::getRandomSeqNum() const {
    int num = rand() % seqMap.size();
    QMap<int, DNASequence>::const_iterator iter = seqMap.begin() + num;
    return iter.key();
}

inline const QMap<int, DNASequence> & SequencesStorage::getSeqMap() const {
    return seqMap;
}

/************************************************************************/
/* SequenceMapping */
/************************************************************************/
SequenceMapping::SequenceMapping() {
    mappingArray = NULL;
    seqNumsForMapping = NULL;
    mappingSize = 0;
}

SequenceMapping::~SequenceMapping() {
    clearMapping();
}

void SequenceMapping::initMapping(int seqCount) {
    mappingArray = new quint64[seqCount];
    seqNumsForMapping = new int[seqCount];
    mappingSize = 0;
}

void SequenceMapping::clearMapping() {
    delete[] mappingArray; mappingArray = NULL;
    delete[] seqNumsForMapping; seqNumsForMapping = NULL;
    mappingSize = 0;
}

void SequenceMapping::createMapping(quint64 start, int num) {
    mappingArray[mappingSize] = start;
    seqNumsForMapping[mappingSize] = num;
    mappingSize++;
}

inline int SequenceMapping::getSeqByGlobalPos(quint64 findV) {
    // binary search of the nearest
    int end = mappingSize - 1;
    int low = 0;
    int high = end;
    quint64 *a = mappingArray;
    while (low <= high) {
        int mid = (low + high) / 2;
        quint64 midV = mappingArray[mid];
        if (midV < findV) {
            if (mid >= end) {
                return seqNumsForMapping[mid];
            }
            low = mid + 1;
            if (findV < mappingArray[low]) {
                return seqNumsForMapping[mid];
            }
        } else if (midV > findV) {
            if (0 == mid) {
                return seqNumsForMapping[mid];
            }
            high = mid - 1;
        } else {
            return seqNumsForMapping[mid];
        }
    }
    assert(false);
    return seqNumsForMapping[0];
}

} // SPB
