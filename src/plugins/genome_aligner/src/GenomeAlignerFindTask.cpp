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

#include <U2Algorithm/BitsTable.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/Timer.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/GAutoDeleteList.h>

#include "GenomeAlignerIndex.h"
#include "GenomeAlignerTask.h"
#include "SuffixSearchCUDA.h"

#include <time.h>

#include "GenomeAlignerFindTask.h"

namespace U2 {

const int GenomeAlignerFindTask::ALIGN_DATA_SIZE = 100000;

GenomeAlignerFindTask::GenomeAlignerFindTask(U2::GenomeAlignerIndex *i, AlignContext *s, GenomeAlignerWriteTask *w)
: Task("GenomeAlignerFindTask", TaskFlag_None),
index(i), writeTask(w), alignContext(s), binarySearchResults(NULL)
{
    partLoaded = false;
    openCLFinished = false;
    nextElementToGive = 0;
    indexLoadTime = 0;
    waiterCount = 0;
    alignerTaskCount = 0;
}

void GenomeAlignerFindTask::prepare() {
    if (alignContext->useCUDA) {
        //proceed to run function
        return;
    }

    if(alignContext->openCL) {
        // no reason to have several parallel subtasks using openCL since they'll be waiting on the same mutex anyway
        waiterCount = 0;
        nextElementToGive = 0;
        alignerTaskCount = 1;
        Task *subTask = new ShortReadAlignerOpenCL(0, index, alignContext, writeTask);
        subTask->setSubtaskProgressWeight(1.0f);
        addSubTask(subTask);
    } else {
        alignerTaskCount = AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();
        setMaxParallelSubtasks(alignerTaskCount);
        for (int i = 0; i < alignerTaskCount; i++) {
            waiterCount = 0;
            nextElementToGive = 0;
            Task *subTask = new ShortReadAlignerCPU(i, index, alignContext, writeTask);
            subTask->setSubtaskProgressWeight(1.0f / alignerTaskCount);
            addSubTask(subTask);
        }
    }
}

void GenomeAlignerFindTask::run() {
    if (alignContext->useCUDA) {
        
        GenomeAlignerCUDAHelper cudaHelper;
        
        cudaHelper.loadShortReads(alignContext->queries, stateInfo);
        if (hasError()) {
            return;
        }

        for (int part = 0; part < index->getPartCount(); ++part) {
            if (!index->loadPart(part)) {
                setError("Incorrect index file. Please, try to create a new index file.");
            }
            cudaHelper.alignReads(index->getLoadedPart(),alignContext, stateInfo);
            if (hasError()) {
                return;
            }
        }
    }
}

void GenomeAlignerFindTask::loadPartForAligning(int part) {
    waitMutex.lock();
    waiterCount++;
    if (waiterCount != alignerTaskCount) {
        waiter.wait(&waitMutex);
        waiter.wakeOne();
    } else {
        waiterCount = 0;
        partLoaded = false;
        waiter.wakeOne();
    }
    waitMutex.unlock();

    QMutexLocker lock(&loadPartMutex);
    if (!partLoaded) {
        taskLog.trace(QString("loading index part %1").arg(part + 1));
        if (!index->loadPart(part)) {
            setError("Incorrect index file. Please, try to create a new index file.");
        }
        partLoaded = true;
        openCLFinished = false;
        nextElementToGive = 0;
        taskLog.trace(QString("finished loading index part %1").arg(part + 1));
    }
}

void GenomeAlignerFindTask::unsafeGetData(int &first, int &length) {
    // ReadShortReadsSubTask can add new data what can lead to realloc. Noone can touch these vectors without sync
    QMutexLocker lock(&alignContext->listM);

    int bitValuesCount = alignContext->bitValuesV.size();
    first = nextElementToGive;

    if (first >= bitValuesCount) {
        length = 0;
    } else if (first + ALIGN_DATA_SIZE > bitValuesCount) {
        length = bitValuesCount - first;
    } else {
        length = ALIGN_DATA_SIZE;
    }

    // must be const& if we want to use good const operator[] without QVector performing deep copy
    const QVector<int> &rn = alignContext->readNumbersV;
    int it = first + length;
    for (int last=it-1; it<bitValuesCount; it++) {
        if (rn[last] == rn[it]) {
            length++;
        } else {
            int queriesSize = alignContext->queries.size();
            SAFE_POINT((queriesSize > rn[last]) && (queriesSize > rn[it]), "unsafeGetData error",);

            SearchQuery *lastQu = alignContext->queries.at(rn[last]);
            SearchQuery *qu = alignContext->queries.at(rn[it]);
            if (lastQu->getRevCompl() == qu) {
                last = it;
                length++;
            } else {
                break;
            }
        }
    }

    nextElementToGive += length;
}

void GenomeAlignerFindTask::waitDataForAligning(int &first, int &length) {
    QMutexLocker lock(&waitDataForAligningMutex);

    bool isReadingStarted = false;
    bool isReadingFinished = false;
    if (alignContext->openCL) {
        do {
            alignContext->listM.lock();
            isReadingStarted = alignContext->isReadingStarted;
            isReadingFinished = alignContext->isReadingFinished;
            alignContext->listM.unlock();

            if (isReadingStarted && isReadingFinished) {
                break;
            }

            alignContext->readShortReadsWait.wait(&waitDataForAligningMutex);
        }
        while (true);
    } else {
        int bitValuesVSize = 0;
        do {
            // ReadShortReadsSubTask can add new data what can lead to realloc. Noone can touch these vectors without sync
            alignContext->listM.lock();
            bitValuesVSize = alignContext->bitValuesV.size();

            isReadingStarted = alignContext->isReadingStarted;
            isReadingFinished = alignContext->isReadingFinished;
            alignContext->listM.unlock();

            if (isReadingStarted && isReadingFinished) {
                break;
            }

            alignContext->readShortReadsWait.wait(&waitDataForAligningMutex);
        }
        while (!(isReadingStarted && bitValuesVSize-nextElementToGive >= ALIGN_DATA_SIZE)); //while (not enough reads) wait
    }

    unsafeGetData(first, length);
//     if (alignContext->isReadingFinished) {
//         alignContext->readShortReadsWait.wakeAll();
//     }
}

#ifdef OPENCL_SUPPORT
bool GenomeAlignerFindTask::runOpenCLBinarySearch() {
    QMutexLocker lock(&openCLMutex);

    // ReadShortReadsSubTask can add new data what can lead to realloc. Noone can touch these vectors without sync
    QMutexLocker vectorsLock(&alignContext->listM); // should never happen, because in this realization there is only one OpenCL thread, waiting until all reads are read

    if (!openCLFinished) {
        openCLFinished = true;
        delete[] binarySearchResults;
        if (0 == alignContext->bitValuesV.size()) {
            binarySearchResults = NULL;
            return false;
        }
        binarySearchResults = index->bitMaskBinarySearchOpenCL(alignContext->bitValuesV.constData(), alignContext->bitValuesV.size(), 
            alignContext->windowSizes.constData());
        if (NULL == binarySearchResults) {
            setError("OpenCL binary find error");
            return false;
        }
    }

    if (NULL == binarySearchResults) {
        return false;
    }

    return true;
}
#endif

GenomeAlignerFindTask::~GenomeAlignerFindTask() {
    if(alignContext->openCL) {
        // otherwise it's a pointer to QVector data, which will be deleted automatically
        delete[] binarySearchResults;
    }
}

ShortReadAlignerCPU::ShortReadAlignerCPU(int taskNo, GenomeAlignerIndex *i, AlignContext *s, GenomeAlignerWriteTask *w)
: Task("ShortReadAlignerCPU", TaskFlag_None), taskNo(taskNo), index(i), alignContext(s), writeTask(w)
{
}

void ShortReadAlignerCPU::run() {
    assert(!alignContext->openCL);
    GenomeAlignerFindTask *parent = static_cast<GenomeAlignerFindTask*>(getParentTask());
    SearchQuery *shortRead = NULL;
    SearchQuery *revCompl = NULL;
    int first = 0;
    int last = 0;
    int length = 0;
    BinarySearchResult bmr = 0;

    //for thread safe:
    BMType bv = 0;
    int rn = 0;
    int rn1 = 0;
    int pos = 0;

    for (int part = 0; part < index->getPartCount(); part++) {
        stateInfo.setProgress(100*part/index->getPartCount());
        parent->loadPartForAligning(part);
        if (parent->hasError()) {
            return;
        }
        stateInfo.setProgress(stateInfo.getProgress() + 25/index->getPartCount());
        if(0 == parent->index->getLoadedPart().getLoadedPartSize()) {
            algoLog.trace(tr("Index size for part %1/%2 is zero, skipping it.").arg(part + 1).arg(index->getPartCount()));
            continue;
        }

        do {
            // fetch a batch of reads
            parent->waitDataForAligning(first, length);
            if(0 == length) {
                break;
            }

            // ReadShortReadsSubTask can add new data what can lead to realloc. Noone can touch these vectors without sync
            alignContext->listM.lock();

            last = first + length;
            for (int i = first; i < last; i++) {
                int currentW = alignContext->windowSizes.at(i);
                if(0 == currentW) {
                    continue;
                }
                BMType currentBitFilter = ((quint64)0 - 1) << (62 - currentW * 2);
                bv = alignContext->bitValuesV.at(i);
                rn = alignContext->readNumbersV.at(i);
                pos = alignContext->positionsAtReadV.at(i);
                if (i < last - 1) {
                    rn1 = alignContext->readNumbersV.at(i + 1);
                }
                shortRead = alignContext->queries.at(rn);

                revCompl = shortRead->getRevCompl();
                if (alignContext->bestMode) {
                    if (0 == shortRead->firstMCount()) {
                        continue;
                    }
                    if (NULL != revCompl && 0 == revCompl->firstMCount()) {
                        continue;
                    }
                }

                bmr = index->bitMaskBinarySearch(bv, currentBitFilter);
                index->alignShortRead(shortRead, bv, pos, bmr, alignContext, currentBitFilter, currentW);

                if (!alignContext->bestMode) {
                    if ((i == last - 1) || (rn1 != rn)) {
                        if (shortRead->haveResult()) {
                            writeTask->addResult(shortRead);
                        }
                        shortRead->onPartChanged();
                    }
                }
            }

            alignContext->listM.unlock();
        } while(true);
    }
}


ShortReadAlignerOpenCL::ShortReadAlignerOpenCL(int taskNo, GenomeAlignerIndex *i, AlignContext *s, GenomeAlignerWriteTask *w)
: Task("ShortReadAlignerOpenCL", TaskFlag_None), taskNo(taskNo), index(i), alignContext(s), writeTask(w)
{
}

void ShortReadAlignerOpenCL::run() {
#ifdef OPENCL_SUPPORT
    assert(alignContext->openCL);

    GenomeAlignerFindTask *parent = static_cast<GenomeAlignerFindTask*>(getParentTask());
    SearchQuery *shortRead = NULL;
    SearchQuery *revCompl = NULL;
    int first = 0;
    int length = 0;
    BinarySearchResult bmr = 0;

    //for thread safe:
    BMType bv = 0;
    int rn = 0;
    int rn1 = 0;
    int pos = 0;

    for (int part = 0; part < index->getPartCount(); part++) {
        stateInfo.setProgress(100 * part / index->getPartCount());
        quint64 t0 = GTimer::currentTimeMicros();
        parent->loadPartForAligning(part);
        algoLog.trace(QString("Index part %1 loaded in %2 sec.").arg(part + 1).arg((GTimer::currentTimeMicros() - t0) / double(1000000), 0, 'f', 3));
        if (parent->hasError()) {
            return;
        }
        stateInfo.setProgress(stateInfo.getProgress() + 25 / index->getPartCount());
        if(0 == parent->index->getLoadedPart().getLoadedPartSize()) {
            algoLog.trace(tr("Index size for part %1/%2 is zero, skipping it.").arg(part + 1).arg(index->getPartCount()));
            continue;
        }

        // wait until all short reads are loaded
        t0 = GTimer::currentTimeMicros();
        do {
            parent->waitDataForAligning(first, length);
        } while(length > 0);

        if (!parent->runOpenCLBinarySearch()) {
            return;
        }
        stateInfo.setProgress(stateInfo.getProgress() + 50/index->getPartCount());


        // ReadShortReadsSubTask can add new data what can lead to realloc. Noone can touch these vectors without sync
        alignContext->listM.lock();

        const int totalResults = alignContext->bitValuesV.size();
        t0 = GTimer::currentTimeMicros();
        for (int i = 0; i < totalResults; i++) {
            int currentW = alignContext->windowSizes.at(i);
            if(0 == currentW) {
                continue;
            }
            BMType currentBitFilter = ((quint64)0 - 1) << (62 - currentW * 2);

            bv = alignContext->bitValuesV.at(i);
            rn = alignContext->readNumbersV.at(i);
            pos = alignContext->positionsAtReadV.at(i);
            if (i < totalResults - 1) {
                rn1 = alignContext->readNumbersV.at(i + 1);
            }
            shortRead = alignContext->queries.at(rn);

            revCompl = shortRead->getRevCompl();
            if (alignContext->bestMode) {
                if (0 == shortRead->firstMCount()) {
                    continue;
                }
                if (NULL != revCompl && 0 == revCompl->firstMCount()) {
                    continue;
                }
            }

            bmr = parent->binarySearchResults[i];
            index->alignShortRead(shortRead, bv, pos, bmr, alignContext, currentBitFilter, currentW);

            if (!alignContext->bestMode) {
                if ((i == totalResults - 1) || (rn1 != rn)) {
                    if (shortRead->haveResult()) {
                        writeTask->addResult(shortRead);
                    }
                    shortRead->onPartChanged();
                }
            }
        }
        algoLog.trace(QString("binary search results applied in %1 ms").arg((GTimer::currentTimeMicros() - t0) / double(1000), 0, 'f', 3));
        alignContext->listM.unlock();
    }

#endif
}

} // U2
