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
#include <U2Core/U2SafePoints.h>
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
#include "ReadShortReadsSubTask.h"
#include "WriteAlignedReadsSubTask.h"

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

    if (subTask == createIndexTask) {
        SAFE_POINT(createIndexTask != NULL, "Create index task error", subTasks);
        delete index; index = NULL;
        index = createIndexTask->index;
    }

    if (justBuildIndex || hasError() || isCanceled()) {
        if (!justBuildIndex && !alignContext.bestMode) {
            pWriteTask->setFinished();
        }
        return subTasks;
    }

    assert(createIndexTask != NULL);
    qint64 time=(subTask->getTimeInfo().finishTime - subTask->getTimeInfo().startTime);
    if (subTask == createIndexTask) {
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
        taskLog.details(QString("Genome aligner index creation time: %1 sec.").arg((double)time/(1000*1000)));
    }

    if (subTask == findTask) {
        taskLog.details(QString("%1 reads with up to %2 mismatches aligned in %3 sec.")
            .arg(alignContext.queries.size()).arg(alignContext.nMismatches).arg((double)time/(1000*1000)));
        indexLoadTime += findTask->getIndexLoadTime();

        if (alignContext.bestMode) {
            // ReadShortReadsSubTask can add new data what can lead to realloc. Noone can touch these vectors without sync
            writeTask  = new WriteAlignedReadsSubTask(alignContext.listM, seqWriter, alignContext.queries, readsAligned);
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
        taskLog.details(QString("GenomeAlignerTask: %1 short reads loaded and complemented in %2 sec, file progress %3%.")
            .arg(readTask->bunchSize).arg(time / (double)1000000, 0, 'f', 2).arg(seqReader->getProgress()));
    }

    if (subTask == createIndexTask || subTask == findTask || subTask == writeTask) {
        if (subTask == writeTask) {
            resultWriteTime += time;
            shortreadIOTime += time;
        }
        if (!noDataToAlign) {
            alignContext.listM.lock();
            alignContext.isReadingStarted = false;
            alignContext.isReadingFinished = false;
            alignContext.listM.unlock();

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

    qint64 aligned = readsAligned;
    if (!alignContext.bestMode) {
        SAFE_POINT_EXT(NULL != pWriteTask,
            stateInfo.setError("No parallel write task in non best mode"), ReportResult_Finished);
        aligned = pWriteTask->getWrittenReadsCount();
    }

    if (readsCount > 0) {
        taskLog.info(tr("The aligning is finished."));
        taskLog.info(tr("Whole working time = %1.").arg((GTimer::currentTimeMicros() - inf.startTime)/(1000*1000)));
        taskLog.info(tr("%1% reads aligned.").arg(100*(double)aligned/readsCount));
        if (alignContext.bestMode) { // not parallel writing could be measured
            taskLog.info(tr("Short-reads loading time = %1").arg(shortreadLoadTime/(1000*1000)));
            taskLog.info(tr("Results writing time = %1").arg(resultWriteTime/(1000*1000)));
        }
        taskLog.info(tr("Index loading time = %1").arg(indexLoadTime));
        taskLog.info(tr("Short-reads IO time = %1").arg(shortreadIOTime/(1000*1000)));
    }

    haveResults = (aligned > 0);

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
} // U2
