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

#include "GenomeAlignerFindTask.h"
#include "GenomeAlignerIndex.h"
#include "GenomeAlignerTask.h"
#include "SuffixSearchCUDA.h"

#include <time.h>

namespace U2 {

const int GenomeAlignerFindTask::ALIGN_DATA_SIZE = 100000;

GenomeAlignerFindTask::GenomeAlignerFindTask(U2::GenomeAlignerIndex *i, const SearchContext &s, GenomeAlignerWriteTask *w)
: Task("GenomeAlignerFindTask", TaskFlag_None),
index(i), writeTask(w), settings(new SearchContext(s)), bitMaskResults(NULL)
{
    partLoaded = false;
    openCLFinished = false;
    nextElementToGive = 0;
    indexLoadTime = 0;
    waiterCount = 0;
    alignerTaskCount = 0;
}

void GenomeAlignerFindTask::prepare() {
    settings->w = GenomeAlignerTask::calculateWindowSize(settings->absMismatches,
        settings->nMismatches, settings->ptMismatches, settings->minReadLength, settings->maxReadLength);
    settings->bitFilter = ((quint64)0 - 1)<<(62 - settings->w*2);

    prepareBitValues();
    if (isCanceled() || hasError()) {
        return;
    }

    if (settings->useCUDA) {
        //proceed to run function
        return;
    }

    alignerTaskCount = AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();
    setMaxParallelSubtasks(alignerTaskCount);
    for (int i=0; i<alignerTaskCount; i++) {
        waiterCount = 0;
        nextElementToGive = 0;
        Task *subTask = new ShortReadAligner(index, settings, writeTask);
        subTask->setSubtaskProgressWeight(1.0f/alignerTaskCount);
        addSubTask(subTask);
    }
}

void GenomeAlignerFindTask::prepareBitValues() {
    taskLog.details("start to calculate bitValues");
    int CMAX = settings->nMismatches;
    int W = 0;
    int q = 0;
    int readNum = 0;
    for (QueryIter it=settings->queries.begin(); it!=settings->queries.end(); it++, readNum++) {
        SearchQuery *qu = *it;
        W = qu->length();
        if (!settings->absMismatches) {
            CMAX = (W * settings->ptMismatches) / MAX_PERCENTAGE;
        }
        q = W / (CMAX + 1);
        assert(q >= settings->w);

        const char* querySeq = qu->constData();
        for (int i = 0; i < W - settings->w + 1; i+=q) {
            const char *seq = querySeq + i;
            BMType bv = index->getBitValue(seq, qMin(GenomeAlignerIndex::charsInMask, W - i));
            bitValuesV.push_back(bv);
            readNumbersV.push_back(readNum);
            positionsAtReadV.push_back(i);
        }
    }
    taskLog.details("finish to calculate bitValues");
}

void GenomeAlignerFindTask::run() {
    if (settings->useCUDA) {
        
        GenomeAlignerCUDAHelper cudaHelper;
        
        cudaHelper.loadShortReads(settings->queries, stateInfo);
        if (hasError()) {
            return;
        }

        for (int part = 0; part < index->getPartCount(); ++part) {
            index->loadPart(part);
            cudaHelper.alignReads(index->getLoadedPart(),settings, stateInfo);
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
        taskLog.details(QString("finish to load part %1").arg(part));
    }
}

void GenomeAlignerFindTask::getDataForAligning(int &first, int &length) {
    QMutexLocker lock(&shortReadsMutex);
    int bitValuesCount = bitValuesV.size();
    first = nextElementToGive;

    if (first >= bitValuesCount) {
        length = 0;
    } else if (first + ALIGN_DATA_SIZE > bitValuesCount) {
        length = bitValuesCount - first;
    } else {
        length = ALIGN_DATA_SIZE;
    }

    int *rn = readNumbersV.data();
    int it = first + length;
    for (int last=it-1; it<bitValuesCount; it++) {
        if (rn[last] == rn[it]) {
            length++;
        } else {
            SearchQuery *lastQu = settings->queries.at(rn[last]);
            SearchQuery *qu = settings->queries.at(rn[it]);
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

bool GenomeAlignerFindTask::runOpenCLBinarySearch() {
    QMutexLocker lock(&openCLMutex);
    if (!openCLFinished) {
        openCLFinished = true;
        delete[] bitMaskResults;
        bitMaskResults = index->bitMaskBinarySearchOpenCL(bitValuesV.data(), bitValuesV.size(), settings->bitFilter);
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
    delete settings;
    delete[] bitMaskResults;
}

ShortReadAligner::ShortReadAligner(GenomeAlignerIndex *i, SearchContext *s, GenomeAlignerWriteTask *w)
: Task("ShortReadAligner", TaskFlag_None), index(i), settings(s), writeTask(w)
{
}

void ShortReadAligner::run() {
    GenomeAlignerFindTask *parent = static_cast<GenomeAlignerFindTask*>(getParentTask());
    SearchQuery **q = settings->queries.data();
    SearchQuery *shortRead = NULL;
    SearchQuery *revCompl = NULL;
    int first = 0;
    int last = 0;
    int length = 0;
    ResType bmr = 0;
    const BMType *bitValues = parent->bitValuesV.constData();
    const int *readNumbers = parent->readNumbersV.constData();
    const int *par = parent->positionsAtReadV.constData();
    ResType *bitMaskResults = NULL;

    for (int part=0; part < index->getPartCount(); part++) {
        stateInfo.setProgress(100*part/index->getPartCount());
        parent->loadPartForAligning(part);
        stateInfo.setProgress(stateInfo.getProgress() + 25/index->getPartCount());
        if (settings->openCL) {
            if (!parent->runOpenCLBinarySearch()) {
                return;
            }
            bitMaskResults = parent->bitMaskResults;
            stateInfo.setProgress(stateInfo.getProgress() + 50/index->getPartCount());
        }

        parent->getDataForAligning(first, length);
        while (length > 0) {
            last = first + length;
            for (int i=first; i<last; i++) {
                int readNum = readNumbers[i];
                shortRead = q[readNum];
                revCompl = shortRead->getRevCompl();
                if (settings->bestMode) {
                    if (0 == shortRead->firstMCount()) {
                        continue;
                    }
                    if (NULL != revCompl && 0 == revCompl->firstMCount()) {
                        continue;
                    }
                }

                if (settings->openCL) {
                    bmr = bitMaskResults[i];
                } else {
                    bmr = index->bitMaskBinarySearch(bitValues[i], settings->bitFilter);
                }
                index->alignShortRead(shortRead, bitValues[i], par[i], bmr, settings);

                if (!settings->bestMode && shortRead->haveResult()) {
                    if ((i == last - 1) || (readNumbers[i+1] != readNum)) {
                        writeTask->addResult(shortRead);
                        shortRead->onPartChanged();
                    }
                }
            }
            parent->getDataForAligning(first, length);
        }
    }
}

} // U2
