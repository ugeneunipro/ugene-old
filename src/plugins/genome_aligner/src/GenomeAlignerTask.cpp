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
#include "GenomeAlignerFindTask.h"
#include "GenomeAlignerIndexTask.h"
#include "GenomeAlignerIndex.h"
#include "SuffixSearchCUDA.h"
#include "GenomeAlignerTask.h"

namespace U2 {

const QString GenomeAlignerTask::taskName(tr("UGENE genome aligner"));
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
loadDbiTask(NULL), createIndexTask(NULL), readTask(NULL), findTask(NULL), writeTask(NULL), seqReader(NULL),
seqWriter(NULL),
justBuildIndex(_justBuildIndex), windowSize(0), bunchSize(0), index(NULL), lastQuery(NULL)
{
    GCOUNTER(cvar,tvar, "GenomeAlignerTask");
    haveResults = true;
    readsCount = 0;
    readsAligned = 0;
    shortreadLoadTime = 0;
    resultWriteTime = 0;
    searchTime = 0;
    indexLoadTime = 0;
    
    dbiIO = false;//settings.getCustomValue(OPTION_DBI_IO, true).toBool();
    if (!justBuildIndex && !dbiIO) {
        seqReader = settings.getCustomValue(OPTION_READS_READER, qVariantFromValue(GenomeAlignerReaderContainer()))
            .value<GenomeAlignerReaderContainer>().reader;
        if (NULL == seqReader) {
            seqReader = new GenomeAlignerUrlReader(settings.shortReadUrls);
        }

        if (seqReader->isEnd()) {
            setError(tr("Can not init short reads loader."));
            return;
        }

        seqWriter = settings.getCustomValue(OPTION_READS_WRITER, qVariantFromValue(GenomeAlignerWriterContainer()))
            .value<GenomeAlignerWriterContainer>().writer;
        if (NULL == seqWriter) {
            seqWriter = new GenomeAlignerUrlWriter(settings.resultFileName, settings.refSeqUrl.baseFileName());
        }
    }

    alignReversed = settings.getCustomValue(OPTION_ALIGN_REVERSED, true).toBool();
    openCL = settings.getCustomValue(OPTION_OPENCL, false).toBool();
    useCUDA = settings.getCustomValue(OPTION_USE_CUDA, false).toBool();
    absMismatches = settings.getCustomValue(OPTION_IF_ABS_MISMATCHES, true).toBool();
    nMismatches = settings.getCustomValue(OPTION_MISMATCHES, 0).toInt();
    ptMismatches = settings.getCustomValue(OPTION_PERCENTAGE_MISMATCHES, 0).toInt();
    qualityThreshold = settings.getCustomValue(OPTION_QUAL_THRESHOLD, 0).toInt();
    bestMode = settings.getCustomValue(OPTION_BEST, false).toBool();
    seqPartSize = settings.getCustomValue(OPTION_SEQ_PART_SIZE, 10).toInt();
    readMemSize = settings.getCustomValue(OPTION_READS_MEMORY_SIZE, 10).toInt();
    prebuiltIndex = settings.prebuiltIndex;

    if (settings.indexFileName.isEmpty()) {
        if (prebuiltIndex) {
            indexFileName = settings.refSeqUrl.dirPath() + "/" + settings.refSeqUrl.baseFileName();
        } else {
            QString tempDir = AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath();
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
    if (openCL) {
        addTaskResource(TaskResourceUsage(RESOURCE_OPENCL_GPU, 1, true));
    } else if (useCUDA) {
        addTaskResource(TaskResourceUsage(RESOURCE_CUDA_GPU, 1 ,true));
    }
}

GenomeAlignerTask::~GenomeAlignerTask() {
    qDeleteAll(queries);
    delete index;
}

void GenomeAlignerTask::prepare() {
    if (!justBuildIndex && dbiIO) {
        assert(settings.shortReadUrls.size() > 0);
        QList<DocumentFormat*> detectedFormats = DocumentUtils::detectFormat(settings.shortReadUrls.first());
        if (!detectedFormats.isEmpty()) {
            IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
            loadDbiTask = new LoadDocumentTask(detectedFormats.first()->getFormatId(), settings.shortReadUrls.first(), factory);
            addSubTask(loadDbiTask);
        } else {
            setError("Unsupported dbi file format.");
            return;
        }
    } else {
        setupCreateIndexTask();
        addSubTask(createIndexTask);
    }
}

QList<Task*> GenomeAlignerTask::onSubTaskFinished( Task* subTask ) {
    QList<Task*> subTasks;
    if (hasError() || isCanceled()) {
        return subTasks;
    }

    if (dbiIO && subTask == loadDbiTask) {
        Document *d = loadDbiTask->getDocument();
        QList<GObject*> objects = d->findGObjectByType(GObjectTypes::ASSEMBLY);

        if (objects.size() < 1) {
            setError("Can't find assembly objects.");
            return subTasks;
        }
        AssemblyObject *assObj = qobject_cast<AssemblyObject*>(objects.first());
        U2OpStatusImpl status;
        handle = QSharedPointer<DbiHandle>(new DbiHandle(assObj->getDbiRef().factoryId, assObj->getDbiRef().dbiId, status));
        U2Dbi *dbi = handle->dbi;
        U2Assembly assm = dbi->getAssemblyDbi()->getAssemblyObject(assObj->getDbiRef().entityId, status);
        seqReader = new GenomeAlignerDbiReader(dbi->getAssemblyDbi(), assm);
        seqWriter = new GenomeAlignerDbiWriter(dbi->getAssemblyDbi(), assm);
        //seqWriter = new GenomeAlignerUrlWriter(settings.resultFileName, settings.refSeqUrl.baseFileName());
        
        setupCreateIndexTask();
        subTasks.append(createIndexTask);
        return subTasks;
    }

    assert(createIndexTask != NULL);
    if (justBuildIndex) {
        return subTasks;
    }
    qint64 time=(subTask->getTimeInfo().finishTime - subTask->getTimeInfo().startTime);
    if (subTask == createIndexTask) {
        index = createIndexTask->index;
        seqWriter->setReferenceName(index->getSeqName());
        taskLog.details(QString("Genome aligner index creation time: %1").arg((double)time/(1000*1000)));
    }

    if (subTask == createIndexTask || subTask == writeTask) {
        if (subTask == writeTask) {
            taskLog.details(QString("Results writing time: %1").arg((double)time/(1000*1000)));
            resultWriteTime += time;
        }
        // Read next bunch of sequences
        readTask = new ReadShortReadsSubTask(&lastQuery, seqReader, queries,  settings, readMemSize*1024*1024);
        readTask->setSubtaskProgressWeight(0.33f);
        subTasks.append(readTask);
        return subTasks;
    }
    
    if (subTask == readTask) {
        shortreadLoadTime += time;
        if (queries.count() == 0) {
            // no more reads to align
            return subTasks;
        }

        readsCount += queries.count();
        taskLog.details(QString("Reading (and complementing) of %1 short-reads  time: %2")
            .arg(readTask->bunchSize).arg((double)time/(1000*1000)));
        SearchContext s;
        s.absMismatches = absMismatches;
        s.nMismatches = nMismatches;
        s.ptMismatches = ptMismatches;
        s.bestMode = bestMode;
        s.queries = queries;
        s.openCL = openCL;
        s.useCUDA = useCUDA;
        s.minReadLength = readTask->minReadLength;
        s.maxReadLength = readTask->maxReadLength;
        findTask = new GenomeAlignerFindTask(index, s);
        findTask->setSubtaskProgressWeight(0.33f);
        subTasks.append(findTask);
        return subTasks;
    }

    if (subTask == findTask) {
        taskLog.details(QString("Bunch of reads search time: %1").arg((double)time/(1000*1000)));
        searchTime += time;
        indexLoadTime += findTask->getIndexLoadTime();
        writeTask = new WriteAlignedReadsSubTask(seqWriter, queries, readsAligned);
        writeTask->setSubtaskProgressWeight(0.33f);
        subTasks.append(writeTask);
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
    createIndexTask->setSubtaskProgressWeight(0.0f);
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
    seqWriter->close();
    if (readsCount > 0) {
        taskLog.info(tr("The aligning is finished."));
        taskLog.info(tr("Whole working time = %1.").arg((GTimer::currentTimeMicros() - inf.startTime)/(1000*1000)));
        taskLog.info(tr("%1% reads aligned.").arg(100*(double)readsAligned/readsCount));
        taskLog.info(tr("Short-reads loading time = %1.").arg((shortreadLoadTime/(1000*1000))));
        taskLog.info(tr("Result writing time = %1.").arg(resultWriteTime/(1000*1000)));
        taskLog.info(tr("Aligning time = %1 (Index loading time = %2)").arg(searchTime/(1000*1000))
        .arg(indexLoadTime));
    }
    
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

ReadShortReadsSubTask::ReadShortReadsSubTask(SearchQuery **_lastQuery,
                                             GenomeAlignerReader *_seqReader,
                                             QVector<SearchQuery*> &_queries,
                                             const DnaAssemblyToRefTaskSettings &_settings,
                                             quint64 m)
: Task("ReadShortReadsSubTask", TaskFlag_None), lastQuery(_lastQuery),
seqReader(_seqReader), queries(_queries), settings(_settings),
freeMemorySize(m)
{
    minReadLength = INT_MAX;
    maxReadLength = 0;
}

void ReadShortReadsSubTask::run() {
    GTIMER(cvar, tvar, "ReadSubTask");
    foreach (SearchQuery *qu, queries) {
        delete qu;
    }
    if (isCanceled()) {
        return;
    }
    queries.clear();
    bunchSize = 0;
    qint64 m = freeMemorySize;
    taskLog.details(QString("Memory size is %1").arg(m));
    bool alignReversed = settings.getCustomValue(GenomeAlignerTask::OPTION_ALIGN_REVERSED, true).toBool();
    bool useCuda = settings.getCustomValue(GenomeAlignerTask::OPTION_USE_CUDA, false).toBool();
    bool absMismatches = settings.getCustomValue(GenomeAlignerTask::OPTION_IF_ABS_MISMATCHES, true).toBool();
    int nMismatches = settings.getCustomValue(GenomeAlignerTask::OPTION_MISMATCHES, 0).toInt();
    int ptMismatches = settings.getCustomValue(GenomeAlignerTask::OPTION_PERCENTAGE_MISMATCHES, 0).toInt();
    int qualityThreshold = settings.getCustomValue(GenomeAlignerTask::OPTION_QUAL_THRESHOLD, 0).toInt();
    int s = sizeof(SearchQuery);
    int n = 0;

    int i=0;
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
                return;
            }
            break;
        }

        if (GenomeAlignerTask::MIN_SHORT_READ_LENGTH > query->length()) {
            delete query;
            continue;
        }
        if (minReadLength > query->length()) {
            minReadLength = query->length();
        }
        if (maxReadLength < query->length()) {
            maxReadLength = query->length();
        }
        if ( qualityThreshold > 0 && query->hasQuality() ) {
            // simple quality filtering
            bool ok = isDnaQualityAboveThreshold(query->getQuality(), qualityThreshold);
            if (!ok) {
                continue;
            }
        }

        n = absMismatches ? nMismatches+1 : (query->length()*ptMismatches/100)+1;
        if (alignReversed) {
            m -= 2*(n*24 + sizeof(SearchQuery) + ONE_SEARCH_QUERY_SIZE + query->length() + query->getNameLength());  // 2*(long long + int) == 24
        } else {
            m -= n*24 + sizeof(SearchQuery) + ONE_SEARCH_QUERY_SIZE + query->length() + query->getNameLength();
        }
        if (m<=0) {
            delete *lastQuery;
            *lastQuery = query;
            break;
        }

        queries.append(query);
        ++bunchSize;
        *lastQuery = NULL;
    }

    if (bunchSize == 0) {
        return;
    }

    if (!isCanceled() && alignReversed) {
        DNATranslation* transl = AppContext::getDNATranslationRegistry()->
            lookupTranslation(BaseDNATranslationIds::NUCL_DNA_DEFAULT_COMPLEMENT);
        foreach (SearchQuery *qu, queries) {
            QByteArray reversed(qu->constSequence());
            TextUtils::reverse(reversed.data(), reversed.count());
            SearchQuery *rQu = new SearchQuery(new DNASequence(QString("%1 rev").arg(qu->getName()), reversed, NULL));
            transl->translate(const_cast<char*>(rQu->constData()), rQu->length());
            if (rQu->constSequence() != qu->constSequence()) {
                queries.append(rQu);
                ++bunchSize;
            } else {
                delete rQu;
            }
        }
    }
   
}

WriteAlignedReadsSubTask::WriteAlignedReadsSubTask(GenomeAlignerWriter *_seqWriter, QVector<SearchQuery*> &_queries, quint64 &r)
: Task("WriteAlignedReadsSubTask", TaskFlag_None), seqWriter(_seqWriter), queries(_queries), readsAligned(r)
{

}

void WriteAlignedReadsSubTask::run() {
    foreach (SearchQuery *qu, queries) {
        const QVector<SAType> &findResults = qu->getResults();
        if (!findResults.isEmpty()) {
            readsAligned++;
            foreach (SAType offset, findResults) {
                seqWriter->write(qu, offset);
            }
        }
    }
}

} // U2
