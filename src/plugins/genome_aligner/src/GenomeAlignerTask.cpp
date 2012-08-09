/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/TextUtils.h>
#include <U2Core/Counter.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/Log.h>
#include <U2Core/Timer.h>
#include <U2Core/AssemblyObject.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Algorithm/FindAlgorithmTask.h>
#include <U2Algorithm/SArrayIndex.h>
#include <U2Algorithm/SArrayBasedFindTask.h>

#include <U2Gui/Notification.h>
#include <limits.h>
#include "GenomeAlignerFindTask.h"
#include "GenomeAlignerIndexTask.h"
#include "GenomeAlignerIndex.h"
#include "SuffixSearchCUDA.h"
#include "GenomeAlignerTask.h"

namespace U2 {

const QString GenomeAlignerTask::taskName(tr("UGENE Genome Aligner"));
const QString GenomeAlignerTask::OPTION_READS_READER("rreader");
const QString GenomeAlignerTask::OPTION_READS_WRITER("rwriter");
const QString GenomeAlignerTask::OPTION_ALIGN_REVERSED("align_reversed");
const QString GenomeAlignerTask::OPTION_OPENCL("use_gpu_optimization");
const QString GenomeAlignerTask::OPTION_USE_CUDA("use_cuda");
const QString GenomeAlignerTask::OPTION_IF_ABS_MISMATCHES("if_absolute_mismatches_value");
const QString GenomeAlignerTask::OPTION_MISMATCHES("mismatches_allowed");
const QString GenomeAlignerTask::OPTION_PERCENTAGE_MISMATCHES("mismatches_percentage_allowed");
const QString GenomeAlignerTask::OPTION_INDEX_DIR("dir_of_the_index_file");
const QString GenomeAlignerTask::OPTION_BEST("best_mode");
const QString GenomeAlignerTask::OPTION_DBI_IO("dbi_io");
const QString GenomeAlignerTask::OPTION_QUAL_THRESHOLD("quality_threshold");
const QString GenomeAlignerTask::OPTION_READS_MEMORY_SIZE("reads_mem_size");
const QString GenomeAlignerTask::OPTION_SEQ_PART_SIZE("seq_part_size");

GenomeAlignerTask::GenomeAlignerTask( const DnaAssemblyToRefTaskSettings& settings, bool _justBuildIndex )
: DnaAssemblyToReferenceTask(settings, TaskFlags_NR_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled, _justBuildIndex),
loadDbiTask(NULL), createIndexTask(NULL), readTask(NULL), findTask(NULL), writeTask(NULL), pWriteTask(NULL), seqReader(NULL),
seqWriter(NULL),
justBuildIndex(_justBuildIndex), bunchSize(0), index(NULL), lastQuery(NULL)
{
    GCOUNTER(cvar,tvar, "GenomeAlignerTask");
    setMaxParallelSubtasks(3);
    haveResults = true;
    readsCount = 0;
    readsAligned = 0;
    shortreadLoadTime = 0;
    resultWriteTime = 0;
    searchTime = 0;
    indexLoadTime = 0;
    shortreadIOTime = 0;
    currentProgress = 0.0f;
    noDataToAlign = false;

    alignReversed = settings.getCustomValue(OPTION_ALIGN_REVERSED, true).toBool();
    alignContext.openCL = settings.getCustomValue(OPTION_OPENCL, false).toBool();
    alignContext.useCUDA = settings.getCustomValue(OPTION_USE_CUDA, false).toBool();
    alignContext.absMismatches = settings.getCustomValue(OPTION_IF_ABS_MISMATCHES, true).toBool();
    alignContext.nMismatches = settings.getCustomValue(OPTION_MISMATCHES, 0).toInt();
    alignContext.ptMismatches = settings.getCustomValue(OPTION_PERCENTAGE_MISMATCHES, 0).toInt();
    qualityThreshold = settings.getCustomValue(OPTION_QUAL_THRESHOLD, 0).toInt();
    alignContext.bestMode = settings.getCustomValue(OPTION_BEST, false).toBool();
    seqPartSize = settings.getCustomValue(OPTION_SEQ_PART_SIZE, 10).toInt();
    readMemSize = settings.getCustomValue(OPTION_READS_MEMORY_SIZE, 10).toInt();
    prebuiltIndex = settings.prebuiltIndex;

    if (settings.indexFileName.isEmpty()) {
        if (prebuiltIndex) {
            indexFileName = settings.refSeqUrl.dirPath() + "/" + settings.refSeqUrl.baseFileName();
        } else {
            QString tempDir = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath("aligner");
            QString indexDir = settings.getCustomValue(OPTION_INDEX_DIR, tempDir).toString();
            indexFileName = indexDir + "/" + settings.refSeqUrl.baseFileName() + "." + GenomeAlignerIndex::HEADER_EXTENSION;
        }
    } else {
        indexFileName = settings.indexFileName;
    }

    taskLog.details(tr("Genome Aligner settings"));
    taskLog.details(tr("Index file name: %1").arg(indexFileName));
    taskLog.details(tr("Use prebuilt index: %2").arg(prebuiltIndex));

    qint64 memUseMB = seqPartSize*13;
    if (!justBuildIndex) {
        memUseMB += readMemSize;
    }
    addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, memUseMB, true));
    if (alignContext.openCL) {
        addTaskResource(TaskResourceUsage(RESOURCE_OPENCL_GPU, 1, true));
    } else if (alignContext.useCUDA) {
        addTaskResource(TaskResourceUsage(RESOURCE_CUDA_GPU, 1 ,true));
    }
}

GenomeAlignerTask::~GenomeAlignerTask() {
    qDeleteAll(alignContext.queries);
    delete index;
}

void GenomeAlignerTask::prepare() {
    setupCreateIndexTask();
    addSubTask(createIndexTask);
    if (!justBuildIndex && !alignContext.bestMode) {
        pWriteTask = new GenomeAlignerWriteTask(seqWriter);
        pWriteTask->setSubtaskProgressWeight(0.0f);
        addSubTask(pWriteTask);
    }
}

QList<Task*> GenomeAlignerTask::onSubTaskFinished( Task* subTask ) {
    QList<Task*> subTasks;
    if (hasError() || isCanceled()) {
        if (!justBuildIndex && !alignContext.bestMode) {
            pWriteTask->setFinished();
        }
        return subTasks;
    }

    assert(createIndexTask != NULL);
    if (justBuildIndex) {
        return subTasks;
    }
    qint64 time=(subTask->getTimeInfo().finishTime - subTask->getTimeInfo().startTime);
    if (subTask == createIndexTask) {
        index = createIndexTask->index;
        seqReader = settings.getCustomValue(OPTION_READS_READER, qVariantFromValue(GenomeAlignerReaderContainer()))
            .value<GenomeAlignerReaderContainer>().reader;
        if (NULL == seqReader) {
            seqReader = new GenomeAlignerUrlReader(settings.shortReadUrls);
        }

        if (seqReader->isEnd()) {
            if (!hasError()){
                setError(tr("Can not init short reads loader."));
            }
            return subTasks;
        }

        seqWriter = settings.getCustomValue(OPTION_READS_WRITER, qVariantFromValue(GenomeAlignerWriterContainer()))
            .value<GenomeAlignerWriterContainer>().writer;
        if (NULL == seqWriter) {
            if (settings.samOutput) {
                seqWriter = new GenomeAlignerUrlWriter(settings.resultFileName, index->getSeqName(), index->getSeqLength());
            } else {
                try {
                    seqWriter = new GenomeAlignerDbiWriter(settings.resultFileName.getURLString(), index->getSeqName(), index->getSeqLength());
                } catch (QString exeptionMessage) {
                    setError(exeptionMessage);
                    return subTasks;
                }
            }
        }
        seqWriter->setReferenceName(index->getSeqName());
        if (!alignContext.bestMode) {
            pWriteTask->setSeqWriter(seqWriter);
        }
        taskLog.details(QString("Genome aligner index creation time: %1").arg((double)time/(1000*1000)));
    }

    if (subTask == findTask) {
        taskLog.details(QString("Bunch of reads aligning time: %1").arg((double)time/(1000*1000)));
        indexLoadTime += findTask->getIndexLoadTime();

        if (alignContext.bestMode) {
            writeTask  = new WriteAlignedReadsSubTask(seqWriter, alignContext.queries, readsAligned);
            writeTask->setSubtaskProgressWeight(0.0f);
            subTasks.append(writeTask);
            return subTasks;
        }
    }

    if (subTask == readTask) {
        shortreadLoadTime += time;
        shortreadIOTime += time;
        if (alignContext.queries.count() == 0) {
            // no more reads to align
            if (!alignContext.bestMode) {
                pWriteTask->setFinished();
            }
            seqWriter->close();
            noDataToAlign = true;
            return subTasks;
        }

        readsCount += readTask->bunchSize;
        taskLog.details(QString("Reading (and complementing) of %1 short-reads  time: %2")
            .arg(readTask->bunchSize).arg((double)time/(1000*1000)));

        /*findTask = new GenomeAlignerFindTask(index, &alignContext, pWriteTask);
        findTask->setSubtaskProgressWeight(0.0f);
        subTasks.append(findTask);
        findTask->setSubtaskProgressWeight(seqReader->getProgress()/100.0f - currentProgress);
        currentProgress = seqReader->getProgress()/100.0f;*/
    }

    if (subTask == createIndexTask || subTask == findTask || subTask == writeTask) {
        if (subTask == writeTask) {
            resultWriteTime += time;
            shortreadIOTime += time;
        }
        if (!noDataToAlign) {
            alignContext.isReadingStarted = false;
            alignContext.isReadingFinished = false;
            alignContext.minReadLength = INT_MAX;
            alignContext.maxReadLength = 0;
            readTask = new ReadShortReadsSubTask(&lastQuery, seqReader, settings, alignContext, readMemSize*1024*1024);
            readTask->setSubtaskProgressWeight(0.0f);
            subTasks.append(readTask);
            findTask = new GenomeAlignerFindTask(index, &alignContext, pWriteTask);
            findTask->setSubtaskProgressWeight(0.0f);
            subTasks.append(findTask);
        }
        return subTasks;
    }

    return subTasks;
}

static bool isDnaQualityAboveThreshold(const DNAQuality &dna, int threshold) {
    assert(!dna.isEmpty());
    for (int i = 0; i < dna.qualCodes.length(); ++i) {
        int qValue = dna.getValue(i);
        if (qValue < threshold) {
            return false;
        }
    }

    return true;
}

void GenomeAlignerTask::setupCreateIndexTask() {
    GenomeAlignerIndexSettings s;
    s.refFileName = settings.refSeqUrl.getURLString();
    s.indexFileName = indexFileName;
    s.justBuildIndex = justBuildIndex;
    s.seqPartSize = seqPartSize;
    s.prebuiltIndex = prebuiltIndex;
    createIndexTask = new GenomeAlignerIndexTask(s);
    if (justBuildIndex) {
        createIndexTask->setSubtaskProgressWeight(1.0f);
    } else {
        createIndexTask->setSubtaskProgressWeight(0.0f);
    }
}

Task::ReportResult GenomeAlignerTask::report() {
    TaskTimeInfo inf=getTimeInfo();
    if (hasError()) {
        return ReportResult_Finished;
    }

    if (justBuildIndex) {
        return ReportResult_Finished;
    }
    
    if (seqWriter->getWrittenReadsCount() == 0) {
        haveResults = false;
        return ReportResult_Finished;
    }
    
    if (readsCount > 0) {
        taskLog.info(tr("The aligning is finished."));
        taskLog.info(tr("Whole working time = %1.").arg((GTimer::currentTimeMicros() - inf.startTime)/(1000*1000)));
        if (alignContext.bestMode) {
            taskLog.info(tr("%1% reads aligned.").arg(100*(double)readsAligned/readsCount));
            taskLog.info(tr("Short-reads loading time = %1").arg(shortreadLoadTime/(1000*1000)));
            taskLog.info(tr("Results writing time = %1").arg(resultWriteTime/(1000*1000)));
        } else {
            taskLog.info(tr("%1% reads aligned.").arg(100*(double)pWriteTask->getWrittenReadsCount()/readsCount));
        }
        taskLog.info(tr("Index loading time = %1").arg(indexLoadTime));
        taskLog.info(tr("Short-reads IO time = %1").arg(shortreadIOTime/(1000*1000)));
    }

    haveResults = (readsAligned > 0);
    
    return ReportResult_Finished;
}

int GenomeAlignerTask::calculateWindowSize(bool absMismatches, int nMismatches, int ptMismatches, int minReadLength, int maxReadLength) {
    int CMAX = nMismatches;
    int windowSize = MAX_BIT_MASK_LENGTH;
    int q = 0;
    for (int len = minReadLength; len <= maxReadLength; len++) {
        if (!absMismatches) {
            CMAX = len*ptMismatches/MAX_PERCENTAGE;
        }
        q = len/(CMAX + 1);
        if (windowSize > q) {
            windowSize = q;
        }
    }
    return windowSize;
}

QString GenomeAlignerTask::getIndexPath() {
    return indexFileName;
}

#define ALIGN_DATA_SIZE 1000

ReadShortReadsSubTask::ReadShortReadsSubTask(SearchQuery **_lastQuery,
                                             GenomeAlignerReader *_seqReader,
                                             const DnaAssemblyToRefTaskSettings &_settings,
                                             AlignContext &_alignContext,
                                             quint64 m)
: Task("ReadShortReadsSubTask", TaskFlag_None), lastQuery(_lastQuery),
seqReader(_seqReader), settings(_settings), alignContext(_alignContext),
freeMemorySize(m)
{
    minReadLength = INT_MAX;
    maxReadLength = 0;
}

void ReadShortReadsSubTask::run() {
    stateInfo.setProgress(0);
    GTIMER(cvar, tvar, "ReadSubTask");
    GenomeAlignerTask *parent = static_cast<GenomeAlignerTask*>(getParentTask());
    if (!alignContext.bestMode) {
        parent->pWriteTask->flush();
    }
    foreach (SearchQuery *qu, alignContext.queries) {
        delete qu;
    }
    alignContext.queries.clear();
    alignContext.bitValuesV.clear();
	alignContext.windowSizes.clear();
    alignContext.readNumbersV.clear();
    alignContext.positionsAtReadV.clear();

    if (isCanceled()) {
        alignContext.isReadingFinished = true;
        alignContext.alignerWait.wakeAll();
        return;
    }
    bunchSize = 0;
    qint64 m = freeMemorySize;
    taskLog.details(QString("Memory size is %1").arg(m));
    bool alignReversed = settings.getCustomValue(GenomeAlignerTask::OPTION_ALIGN_REVERSED, true).toBool();
    int qualityThreshold = settings.getCustomValue(GenomeAlignerTask::OPTION_QUAL_THRESHOLD, 0).toInt();
    //int s = sizeof(SearchQuery);
    int n = 0;
    int CMAX = alignContext.nMismatches;
    int W = 0;
    int q = 0;
    int readNum = 0;
    int alignBunchSize = 0;

    DNATranslation* transl = AppContext::getDNATranslationRegistry()->
        lookupTranslation(BaseDNATranslationIds::NUCL_DNA_DEFAULT_COMPLEMENT);
    alignContext.isReadingStarted = true;
    while(!seqReader->isEnd()) {
        SearchQuery *query = NULL;
        if (NULL == *lastQuery) {
            query = seqReader->read();
        } else {
            query = *lastQuery;
        }
        if (NULL == query) {
            if (!seqReader->isEnd()) {
                setError("Short-reads object type must be a sequence, but not a multiple alignment");
                alignContext.isReadingFinished = true;
                alignContext.alignerWait.wakeAll();
                return;
            }
            break;
        }

        if ( qualityThreshold > 0 && query->hasQuality() ) {
            // simple quality filtering
            bool ok = isDnaQualityAboveThreshold(query->getQuality(), qualityThreshold);
            if (!ok) {
                continue;
            }
        }

        if (GenomeAlignerTask::MIN_SHORT_READ_LENGTH <= query->length()) {
            if (alignContext.minReadLength > query->length()) {
                alignContext.minReadLength = query->length();
            }
            if (alignContext.maxReadLength < query->length()) {
                alignContext.maxReadLength = query->length();
            }
        }

        n = alignContext.absMismatches ? alignContext.nMismatches+1 : (query->length()*alignContext.ptMismatches/100)+1;

        qint64 qualLength = 0;
        if (query->hasQuality()){
            qualLength = query->getQuality().qualCodes.length();
        }
        qint64 memoryRequiredForOneRead = n*24 +  // 2*(long long + int) == 24
            sizeof(SearchQuery) +
            ONE_SEARCH_QUERY_SIZE + query->length() +
            query->getNameLength() +
            qualLength;
        memoryRequiredForOneRead *= 2; // FIXME: UGENE-1114


        if (alignReversed) {
            m -= 2*memoryRequiredForOneRead;
            alignBunchSize += 2;
        } else {
            m -= memoryRequiredForOneRead;
            alignBunchSize++;
        }
        if (m<=0) {
            delete *lastQuery;
            *lastQuery = query;
            break;
        }

        if (!add(CMAX, W, q, readNum, query, parent)) {
            delete query;
            continue;
        }
        ++bunchSize;
        *lastQuery = NULL;

        if (alignReversed) {
            QByteArray reversed(query->constSequence());
            TextUtils::reverse(reversed.data(), reversed.count());
            SearchQuery *rQu = new SearchQuery(new DNASequence(QString("%1_rev").arg(query->getName()), reversed, NULL), query);
            transl->translate(const_cast<char*>(rQu->constData()), rQu->length());
            if (rQu->constSequence() != query->constSequence()) {
                query->setRevCompl(rQu);
                add(CMAX, W, q, readNum, rQu, parent);
            } else {
                delete rQu;
            }
        }

        if (!alignContext.openCL) {
            if (alignBunchSize > ALIGN_DATA_SIZE) {
                alignContext.alignerWait.wakeAll();
            }
        }
    }

    alignContext.isReadingFinished = true;
	algoLog.details(QString("ReadShortReadsSubTask finished loading %1 more short reads, total progress: %2%").arg(alignContext.queries.size()).arg(seqReader->getProgress()));
    alignContext.alignerWait.wakeAll();
}

inline bool ReadShortReadsSubTask::add(int &CMAX, int &W, int &q, int &readNum, SearchQuery *query, GenomeAlignerTask *parent) {
    QMutexLocker lock(&alignContext.listM);
    W = query->length();
    if (!alignContext.absMismatches) {
        CMAX = (W * alignContext.ptMismatches) / MAX_PERCENTAGE;
    }
    q = W / (CMAX + 1);

    if (0 == q) {
        return false;
    }

    const char* querySeq = query->constData();

	int win = query->length() < GenomeAlignerTask::MIN_SHORT_READ_LENGTH ?
		GenomeAlignerTask::calculateWindowSize(alignContext.absMismatches,
			alignContext.nMismatches, alignContext.ptMismatches, query->length(), query->length()) :
		GenomeAlignerTask::calculateWindowSize(alignContext.absMismatches,
			alignContext.nMismatches, alignContext.ptMismatches, alignContext.minReadLength, alignContext.maxReadLength);

    for (int i = 0; i < W - q + 1; i+=q) {
        const char *seq = querySeq + i;
        BMType bv = parent->index->getBitValue(seq, qMin(GenomeAlignerIndex::charsInMask, W - i));
        alignContext.bitValuesV.append(bv);
        alignContext.readNumbersV.append(readNum);
        alignContext.positionsAtReadV.append(i);
		alignContext.windowSizes.append(win);
    }
    readNum++;
    alignContext.queries.append(query);
    return true;
}

WriteAlignedReadsSubTask::WriteAlignedReadsSubTask(GenomeAlignerWriter *_seqWriter, QVector<SearchQuery*> &_queries, quint64 &r)
: Task("WriteAlignedReadsSubTask", TaskFlag_None), seqWriter(_seqWriter), queries(_queries), readsAligned(r)
{

}

void WriteAlignedReadsSubTask::setReadWritten(SearchQuery *read, SearchQuery *revCompl) {
    if (!read->isWroteResult()) {
        readsAligned++;
        read->writeResult();
        if (NULL != revCompl) {
            revCompl->writeResult();
        }
    }
}

void WriteAlignedReadsSubTask::run() {
    stateInfo.setProgress(0);
    SearchQuery *read = NULL;
    SearchQuery *revCompl = NULL;
    SearchQuery **q = queries.data();
    int size = queries.size();

    try {
        for (int i=0; i<size; i++) {
            read = q[i];
            revCompl = read->getRevCompl();

            if (i<size-1 && revCompl == q[i+1]) {
                continue;
            }

            if (NULL == revCompl && read->haveResult()) {
                seqWriter->write(read, read->firstResult());
                readsAligned++;
            } else if (NULL != revCompl) {
                int c = read->firstMCount();
                int cRev = revCompl->firstMCount();

                if (c <= cRev && c < INT_MAX) {
                    seqWriter->write(read, read->firstResult());
                    readsAligned++;
                } else if (cRev < INT_MAX) {
                    seqWriter->write(revCompl, revCompl->firstResult());
                    readsAligned++;
                }
            }
        }
    } catch (QString exeptionMessage) {
        setError(exeptionMessage);
        return;
    }
}

} // U2
