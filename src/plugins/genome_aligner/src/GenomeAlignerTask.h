#ifndef _U2_GENOME_ALIGNER_TASK_H_
#define _U2_GENOME_ALIGNER_TASK_H_

#include <U2Algorithm/DnaAssemblyTask.h>
#include <U2Formats/StreamSequenceReader.h>
#include <U2Formats/StreamSequenceWriter.h>
#include "GenomeAlignerFindTask.h"
#include "GenomeAlignerIO.h"

namespace U2 {

class DNASequenceObject;
class DNATranslation;
class LoadDocumentTask;
class GenomeAlignerIndexTask;
class GenomeAlignerIndex;
class ReadShortReadsSubTask;
class WriteAlignedReadsSubTask;

class GenomeAlignerTask : public DnaAssemblyToReferenceTask {
    Q_OBJECT
public:
    GenomeAlignerTask(const DnaAssemblyToRefTaskSettings& settings, bool justBuildIndex = false);
    ~GenomeAlignerTask();
    virtual void prepare();
    virtual void run();
    virtual ReportResult report();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    QString getIndexPath();
    static const QString OPTION_READS_READER;
    static const QString OPTION_READS_WRITER;
    static const QString OPTION_ALIGN_REVERSED;
    static const QString OPTION_OPENCL;
    static const QString OPTION_IF_ABS_MISMATCHES;
    static const QString OPTION_MISMATCHES;
    static const QString OPTION_PERCENTAGE_MISMATCHES;
    static const QString OPTION_PREBUILT_INDEX;
    static const QString OPTION_INDEX_URL;
    static const QString OPTION_QUAL_THRESHOLD;
    static const QString OPTION_BEST;
    static const QString INDEX_EXTENSION;
    static const int MIN_SHORT_READ_LENGTH = 30;
    static const int MIN_BIT_MASK_LENGTH = 14; //2*7, where 7 = min chars in bitMask
    static int calculateWindowSize(bool absMismatches, int nMismatches, int ptMismatches);

    DNA_ASSEMBLEY_TO_REF_TASK_FACTORY(GenomeAlignerTask)
private:
    GenomeAlignerIndexTask *createIndexTask;
    ReadShortReadsSubTask *readTask;
    GenomeAlignerFindTask *findTask;
    WriteAlignedReadsSubTask *writeTask;
    GenomeAlignerReader *seqReader;
    GenomeAlignerWriter *seqWriter;
    bool justBuildIndex;
    uint windowSize, bunchSize, nMismatches, ptMismatches;
    bool absMismatches;
    bool prebuiltIdx;
    bool bestMode;
    bool openCL;
    QString indexFileName;
    bool alignReversed;
    GenomeAlignerIndex *index;
    int qualityThreshold;
    QVector<SearchQuery*> queries;
    const DNASequenceObject *lastObj;
    void setupCreateIndexTask();
};

class ReadShortReadsSubTask : public Task {
    Q_OBJECT
public:
    ReadShortReadsSubTask(const DNASequenceObject **lastObj,
                          GenomeAlignerReader *seqReader,
                          QVector<SearchQuery*> &queries,
                          const DnaAssemblyToRefTaskSettings& settings,
                          quint64 freeMemorySize,
                          quint64 freeGPUSize);
    virtual void run();

    uint bunchSize;

private:
    const DNASequenceObject **lastObj;
    GenomeAlignerReader *seqReader;
    QVector<SearchQuery*> &queries;
    const DnaAssemblyToRefTaskSettings &settings;
    quint64 freeMemorySize;
    quint64 freeGPUSize;

    static const int ONE_SEARCH_QUERY_SIZE = 700; //~700 bytes for one search query
};

class WriteAlignedReadsSubTask : public Task {
    Q_OBJECT
public:
    WriteAlignedReadsSubTask(GenomeAlignerWriter *seqWriter, QVector<SearchQuery*> &queries);
    virtual void run();
private:
    GenomeAlignerWriter *seqWriter;
    QVector<SearchQuery*> &queries;
};

} //namespace

#endif // _U2_GENOME_ALIGNER_TASK_H_
