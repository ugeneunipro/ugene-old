#ifndef _U2_SARRAY_BASED_FIND_TASK_H_
#define _U2_SARRAY_BASED_FIND_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>

#include <QtCore/QMutex>

#include "BitsTable.h"

namespace U2 {

class SArrayIndex;

#define MAX_PERCENTAGE 100

struct U2ALGORITHM_EXPORT SArrayBasedSearchSettings {
    SArrayBasedSearchSettings() : query(""), useBitMask(false), unknownChar('\0'),
        bitMask(NULL), ptMismatches(0), nMismatches(0), absMismatches(true) {}
    QByteArray query;
    bool useBitMask;
    char unknownChar;
    quint32 bitMaskCharBitsNum;
    const quint32*  bitMask;
    int ptMismatches;
    int nMismatches;
    bool absMismatches;
}; 

class U2ALGORITHM_EXPORT SArrayBasedFindTask : public Task {
    Q_OBJECT
public:
    SArrayBasedFindTask(SArrayIndex* i, const SArrayBasedSearchSettings& s, bool onlyFirstMatch = false);
    virtual void run();
    virtual void cleanup();
    const QList<int> getResults() const { return results; }

private:
    void runSearch();
    void runSearchWithMismatches();
    SArrayIndex* index;
    SArrayBasedSearchSettings* config;
    QList<int> results;
    QMutex lock;
    bool onlyFirstMatch;
};

} //namespace

#endif // _U2_SARRAY_BASED_FIND_TASK_H_
