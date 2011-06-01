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

#include <U2Algorithm/BitsTable.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/Timer.h>

#include "GenomeAlignerIndex.h"
#include "GenomeAlignerTask.h"
#include "SuffixSearchCUDA.h"

#include <time.h>

#include "GenomeAlignerFindTask.h"

namespace U2 {

const int GenomeAlignerFindTask::ALIGN_DATA_SIZE = 100000;

GenomeAlignerFindTask::GenomeAlignerFindTask(U2::GenomeAlignerIndex *i, AlignContext *s, GenomeAlignerWriteTask *w)
: Task("GenomeAlignerFindTask", TaskFlag_None),
index(i), writeTask(w), alignContext(s), bitMaskResults(NULL)
{
    partLoaded = false;
    openCLFinished = false;
    nextElementToGive = 0;
    indexLoadTime = 0;
    waiterCount = 0;
    alignerTaskCount = 0;
}

void GenomeAlignerFindTask::prepare() {
    alignContext->w = GenomeAlignerTask::calculateWindowSize(alignContext->absMismatches,
        alignContext->nMismatches, alignContext->ptMismatches, alignContext->minReadLength, alignContext->maxReadLength);
    alignContext->bitFilter = ((quint64)0 - 1)<<(62 - alignContext->w*2);

    if (alignContext->useCUDA) {
        //proceed to run function
        return;
    }

    alignerTaskCount = AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();
    setMaxParallelSubtasks(alignerTaskCount);
    for (int i=0; i<alignerTaskCount; i++) {
        waiterCount = 0;
        nextElementToGive = 0;
        Task *subTask = new ShortReadAligner(index, alignContext, writeTask);
        subTask->setSubtaskProgressWeight(1.0f/alignerTaskCount);
        addSubTask(subTask);
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
            index->loadPart(part);
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
        taskLog.details(QString("loading part %1").arg(part));
        index->loadPart(part);
        partLoaded = true;
        openCLFinished = false;
        nextElementToGive = 0;
        taskLog.details(QString("finish to load part %1").arg(part));
    }
}

void GenomeAlignerFindTask::unsafeGetData(int &first, int &length) {
    int bitValuesCount = alignContext->bitValuesV.size();

    first = nextElementToGive;

    if (first >= bitValuesCount) {
        length = 0;
    } else if (first + ALIGN_DATA_SIZE > bitValuesCount) {
        length = bitValuesCount - first;
    } else {
        length = ALIGN_DATA_SIZE;
    }

    QVector<int> &rn = alignContext->readNumbersV;
    int it = first + length;
    for (int last=it-1; it<bitValuesCount; it++) {
        if (rn[last] == rn[it]) {
            length++;
        } else {
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

void GenomeAlignerFindTask::getDataForAligning(int &first, int &length) {
    QMutexLocker lock(&shortReadsMutex);
    unsafeGetData(first, length);
}

void GenomeAlignerFindTask::waitDataForAligning(int &first, int &length) {
    QMutexLocker lock(&shortReadsMutex);
    while ((!alignContext->isReadingFinished && alignContext->bitValuesV.size() - nextElementToGive < ALIGN_DATA_SIZE)
        || !alignContext->isReadingStarted) { //while (not enough read) wait
        alignContext->alignerWait.wait(&shortReadsMutex);
    }
    unsafeGetData(first, length);
    if (alignContext->isReadingFinished) {
        alignContext->alignerWait.wakeAll();
    }
}

bool GenomeAlignerFindTask::runOpenCLBinarySearch() {
    QMutexLocker lock(&openCLMutex);
    if (!openCLFinished) {
        openCLFinished = true;
        delete[] bitMaskResults;
        bitMaskResults = index->bitMaskBinarySearchOpenCL(alignContext->bitValuesV.constData(), alignContext->bitValuesV.size(), alignContext->bitFilter);
        if (NULL == bitMaskResults) {
            setError("OpenCL binary find error");
            return false;
        }
    }

    if (NULL == bitMaskResults) {
        return false;
    }

    return true;
}

GenomeAlignerFindTask::~GenomeAlignerFindTask() {
    delete[] bitMaskResults;
}

ShortReadAligner::ShortReadAligner(GenomeAlignerIndex *i, AlignContext *s, GenomeAlignerWriteTask *w)
: Task("ShortReadAligner", TaskFlag_None), index(i), alignContext(s), writeTask(w)
{
}

void ShortReadAligner::run() {
    GenomeAlignerFindTask *parent = static_cast<GenomeAlignerFindTask*>(getParentTask());
    SearchQuery *shortRead = NULL;
    SearchQuery *revCompl = NULL;
    int first = 0;
    int last = 0;
    int length = 0;
    BinarySearchResult bmr = 0;
    BinarySearchResult *bitMaskResults = NULL;

    //for thread safe:
    SearchQuery **q = const_cast<SearchQuery**>(alignContext->queries.constData());
    BMType bv = 0;
    int rn = 0;
    int rn1 = 0;
    int pos = 0;
    int w = GenomeAlignerTask::calculateWindowSize(alignContext->absMismatches,
        alignContext->nMismatches, alignContext->ptMismatches, alignContext->minReadLength, alignContext->maxReadLength);
    BMType bitFilter = ((quint64)0 - 1)<<(62 - w*2);

    for (int part=0; part < index->getPartCount(); part++) {
        q = const_cast<SearchQuery**>(alignContext->queries.constData());
        const BMType *bitValues = alignContext->bitValuesV.constData();
        const int *readNumbers = alignContext->readNumbersV.constData();
        const int *par = alignContext->positionsAtReadV.constData();

        stateInfo.setProgress(100*part/index->getPartCount());
        parent->loadPartForAligning(part);
        stateInfo.setProgress(stateInfo.getProgress() + 25/index->getPartCount());
        if (alignContext->openCL) {
            if (!parent->runOpenCLBinarySearch()) {
                return;
            }
            bitMaskResults = parent->bitMaskResults;
            stateInfo.setProgress(stateInfo.getProgress() + 50/index->getPartCount());
        }

        if (part > 0 || alignContext->openCL) {
            parent->getDataForAligning(first, length);
        } else { //if (0 == part) then wait for reading shortreads
            parent->waitDataForAligning(first, length);
        }
        while (length > 0) {
            if (!(part > 0 || alignContext->openCL)) {
                w = GenomeAlignerTask::calculateWindowSize(alignContext->absMismatches,
                    alignContext->nMismatches, alignContext->ptMismatches, alignContext->minReadLength, alignContext->maxReadLength);
                bitFilter = ((quint64)0 - 1)<<(62 - w*2);
            }

            last = first + length;
            for (int i=first; i<last; i++) {
                if (part > 0 || alignContext->openCL) { //for avoiding a QVector deep copy
                    bv = bitValues[i];
                    rn = readNumbers[i];
                    pos = par[i];
                    if (i < last - 1) {
                        rn1 = readNumbers[i+1];
                    }
                    shortRead = q[rn];
                } else {
                    QMutexLocker lock(&alignContext->listM);
                    bv = alignContext->bitValuesV[i];
                    rn = alignContext->readNumbersV[i];
                    pos = alignContext->positionsAtReadV[i];
                    if (i < last - 1) {
                        rn1 = alignContext->readNumbersV[i+1];
                    }
                    shortRead = alignContext->queries[rn];
                }
                revCompl = shortRead->getRevCompl();
                if (alignContext->bestMode) {
                    if (0 == shortRead->firstMCount()) {
                        continue;
                    }
                    if (NULL != revCompl && 0 == revCompl->firstMCount()) {
                        continue;
                    }
                }

                if (alignContext->openCL) {
                    bmr = bitMaskResults[i];
                } else {
                    bmr = index->bitMaskBinarySearch(bv, bitFilter);
                }
                index->alignShortRead(shortRead, bv, pos, bmr, alignContext, bitFilter, w);

                if (!alignContext->bestMode) {
                    if ((i == last - 1) || (rn1 != rn)) {
                        if (shortRead->haveResult()) {
                            writeTask->addResult(shortRead);
                        }
                        shortRead->onPartChanged();
                    }
                }
            }
            if (part > 0 || alignContext->openCL) {
                parent->getDataForAligning(first, length);
            } else {
                parent->waitDataForAligning(first, length);
            }
            w = GenomeAlignerTask::calculateWindowSize(alignContext->absMismatches,
                alignContext->nMismatches, alignContext->ptMismatches, alignContext->minReadLength, alignContext->maxReadLength);
            bitFilter = ((quint64)0 - 1)<<(62 - w*2);
        }
    }
}

} // U2
