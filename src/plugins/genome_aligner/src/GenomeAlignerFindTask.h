#ifndef _U2_GENOME_ALIGNER_FIND_TASK_H_
#define _U2_GENOME_ALIGNER_FIND_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>
#include <U2Core/DNASequence.h>

#include <QtCore/QMutex>
#include <QVector>

#define ResType qint64

namespace U2 {

class GenomeAlignerIndex;
class FindInPartSubTask;
class PrepareVectorsSubTask;

class CacheResult {
public:
    int posAtShortRead;
    int numberOfPart;
    quint64 bitValue;
};

class SearchQuery {
public:
    DNASequence shortRead;
    QList<CacheResult> cacheResults; //must be sorted by numberOfPart
    QList<quint32> results;
};

class SearchContext {
public:
    int w;
    int ptMismatches;
    int nMismatches;
    bool absMismatches;
    bool bestMode;
    bool openCL;
    quint64 bitFilter;
    QVector<quint64> bitValuesV;
    QVector<int> readNumbersV;
    QVector<int> positionsAtReadV;
    int *partNumbers;
    QVector<SearchQuery*> queries;
};

#define MAX_PERCENTAGE 100
class GenomeAlignerFindTask : public Task {
    Q_OBJECT
    friend class PrepareVectorsSubTask;
public:
    GenomeAlignerFindTask(GenomeAlignerIndex *i, const SearchContext &s);
    ~GenomeAlignerFindTask();
    virtual void run();
    virtual void prepare();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);

private:
    FindInPartSubTask *findInPartTask;
    PrepareVectorsSubTask *prepVecTask;
    GenomeAlignerIndex *index;
    SearchContext *settings;
    int memCacheTaskCount;
    QVector<quint64> *bitValuesV;
    QVector<int> *readNumbersV;
    QVector<int> *positionsAtReadV;
    ResType *bitMaskResults;
    int currentPart;
    int bitMaskTaskCount;
    int partTaskCount;
    int maxPtMismatches;
    int maxNMismatches;

    void findInMemCache();
    QList<Task*> findInBitMask(int part);
    QList<Task*> findInPart(int part);
    void prepareBitValues();
    void prepareVectors();
};

typedef QVector<SearchQuery*>::iterator QueryIter;

class FindInMemCacheSubTask : public Task {
    Q_OBJECT
public:
    FindInMemCacheSubTask(GenomeAlignerIndex *index,
                          SearchContext *settings,
                          int first, int length);
    virtual void run();
private:
    GenomeAlignerIndex *index;
    SearchContext *settings;
    int first;
    int length;
};

class FindInBitMaskSubTask : public Task {
    Q_OBJECT
public:
    FindInBitMaskSubTask(GenomeAlignerIndex *index,
                         SearchContext *settings,
                         int first, int length,
                         quint64 *bitValues,
                         int *readNumbers,
                         ResType **bitMaskResults);
    virtual void run();
private:
    GenomeAlignerIndex *index;
    SearchContext *settings;
    int first;
    int length;
    quint64 *bitValues;
    int *readNumbers;
    ResType **bitMaskResults;
};

class FindInPartSubTask : public Task {
    Q_OBJECT
public:
    FindInPartSubTask(GenomeAlignerIndex *index,
                      SearchContext *settings,
                      int first, int length,
                      quint64 *bitValues,
                      int *readNumbers,
                      int *positionsAtRead,
                      ResType *bitMaskResults);
    virtual void run();
private:
    GenomeAlignerIndex *index;
    SearchContext *settings;
    int first;
    int length;
    quint64 *bitValues;
    int *readNumbers;
    int *positionsAtRead;
    ResType *bitMaskResults;
};

class PrepareVectorsSubTask : public Task {
    Q_OBJECT
public:
    PrepareVectorsSubTask();
    virtual void run();
};

} //U2

#endif // _U2_GENOME_ALIGNER_FIND_TASK_H_
