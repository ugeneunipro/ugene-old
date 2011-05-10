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

#ifndef _U2_GENOME_ALIGNER_TASK_H_
#define _U2_GENOME_ALIGNER_TASK_H_

#include <QtCore/QSharedPointer>

#include <U2Algorithm/DnaAssemblyTask.h>
#include <U2Formats/StreamSequenceReader.h>
#include <U2Formats/StreamSequenceWriter.h>
#include "GenomeAlignerFindTask.h"
#include "GenomeAlignerIO.h"
#include "GenomeAlignerIndexPart.h"

namespace U2 {

class DNASequenceObject;
class DNATranslation;
class LoadDocumentTask;
class GenomeAlignerIndexTask;
class GenomeAlignerIndex;
class ReadShortReadsSubTask;
class WriteAlignedReadsSubTask;
class DbiHandle;

class GenomeAlignerTask : public DnaAssemblyToReferenceTask {
    Q_OBJECT
public:
    GenomeAlignerTask(const DnaAssemblyToRefTaskSettings& settings, bool justBuildIndex = false);
    ~GenomeAlignerTask();
    virtual void prepare();
    virtual ReportResult report();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    QString getIndexPath();
    static const QString OPTION_READS_READER;
    static const QString OPTION_READS_WRITER;
    static const QString OPTION_ALIGN_REVERSED;
    static const QString OPTION_OPENCL;
    static const QString OPTION_USE_CUDA;
    static const QString OPTION_IF_ABS_MISMATCHES;
    static const QString OPTION_MISMATCHES;
    static const QString OPTION_PERCENTAGE_MISMATCHES;
    static const QString OPTION_INDEX_DIR;
    static const QString OPTION_QUAL_THRESHOLD;
    static const QString OPTION_BEST;
    static const QString OPTION_DBI_IO;
    static const QString OPTION_READS_MEMORY_SIZE;
    static const QString OPTION_SEQ_PART_SIZE;
    static const int MIN_SHORT_READ_LENGTH = 30;
    static int calculateWindowSize(bool absMismatches, int nMismatches, int ptMismatches, int minReadLength, int maxReadLength);

    DNA_ASSEMBLEY_TO_REF_TASK_FACTORY(GenomeAlignerTask)
private:
    LoadDocumentTask *loadDbiTask;
    GenomeAlignerIndexTask *createIndexTask;
    ReadShortReadsSubTask *readTask;
    GenomeAlignerFindTask *findTask;
    WriteAlignedReadsSubTask *writeTask;
    GenomeAlignerReader *seqReader;
    GenomeAlignerWriter *seqWriter;
    QSharedPointer<DbiHandle> handle;
    bool justBuildIndex;
    uint windowSize, bunchSize, nMismatches, ptMismatches;
    bool absMismatches;
    bool bestMode;
    bool openCL;
    bool useCUDA;
    bool dbiIO;
    QString indexFileName;
    bool alignReversed;
    bool prebuiltIndex;
    GenomeAlignerIndex *index;
    int qualityThreshold;
    int readMemSize;
    int seqPartSize;
    QVector<SearchQuery*> queries;
    SearchQuery *lastQuery;

    //statistics
    quint64 readsCount;
    quint64 readsAligned;
    qint64 shortreadLoadTime;
    qint64 resultWriteTime;
    qint64 searchTime;
    qint64 indexLoadTime;

    void setupCreateIndexTask();
};

class ReadShortReadsSubTask : public Task {
    Q_OBJECT
public:
    ReadShortReadsSubTask(SearchQuery **lastQuery,
                          GenomeAlignerReader *seqReader,
                          QVector<SearchQuery*> &queries,
                          const DnaAssemblyToRefTaskSettings& settings,
                          quint64 freeMemorySize);
    virtual void run();

    uint bunchSize;
    int minReadLength;
    int maxReadLength;

private:
    SearchQuery **lastQuery;
    GenomeAlignerReader *seqReader;
    QVector<SearchQuery*> &queries;
    const DnaAssemblyToRefTaskSettings &settings;
    quint64 freeMemorySize;

    static const int ONE_SEARCH_QUERY_SIZE = 38; //~38 bytes for one search query?
};

class WriteAlignedReadsSubTask : public Task {
    Q_OBJECT
public:
    WriteAlignedReadsSubTask(GenomeAlignerWriter *seqWriter, QVector<SearchQuery*> &queries, quint64 &readsAligned);
    virtual void run();
private:
    GenomeAlignerWriter *seqWriter;
    QVector<SearchQuery*> &queries;
    quint64 &readsAligned;
};

} //namespace

#endif // _U2_GENOME_ALIGNER_TASK_H_
