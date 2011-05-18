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

const int GenomeAlignerFindTask::BITMASK_SEARCH_DATA_SIZE = 100000;
const int GenomeAlignerFindTask::PART_SEARCH_DATA_SIZE = 100000;

GenomeAlignerFindTask::GenomeAlignerFindTask(U2::GenomeAlignerIndex *i, const SearchContext &s, GenomeAlignerWriteTask *w)
: Task("GenomeAlignerFindTask", TaskFlag_None),
index(i), writeTask(w), settings(new SearchContext(s)), bitMaskResults(NULL),
bitMaskTaskCount(-1), partTaskCount(-1), startS(NULL), endS(NULL)
{
    currentPart = 0;
    partLoaded = false;
    nextElementToGive = 0;
    nextElementToCalculateBitmask = 0;
    indexLoadTime = 0;
    wholeBitmaskTime = 0;
    startBitmaskTime = 0;
}

void GenomeAlignerFindTask::prepare() {
    settings->w = GenomeAlignerTask::calculateWindowSize(settings->absMismatches,
        settings->nMismatches, settings->ptMismatches, settings->minReadLength, settings->maxReadLength);
    settings->bitFilter = ((quint64)0 - 1)<<(62 - settings->w*2);

    prepareBitValues();
    if (isCanceled() || hasError()) {
        return;
    }
    
    currentPart = 0;
    
    if (settings->useCUDA) {
        //proceed to run function
        return;
    }
    
    QList<Task*> subTasks = findInBitMask(currentPart);
    foreach (Task *subTask, subTasks) {
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
    if (!settings->openCL && !settings->useCUDA) {
        bitMaskResults = new ResType[bitValuesV.size()];
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
            loadPart(part);
            cudaHelper.alignReads(index->getLoadedPart(),settings, stateInfo);
            if (hasError()) {
                return;
            }
        }

    } else {
        taskLog.details(tr("Bit mask time = %1").arg(wholeBitmaskTime));
    }
}

QList<Task*> GenomeAlignerFindTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> subTasks;
    if (subTask->hasError() || isCanceled()) {
        return subTasks;
    }

    if (bitMaskTaskCount > 0) {
        bitMaskTaskCount--;
    }
    if (0 == bitMaskTaskCount && !isCanceled()) {
        wholeBitmaskTime += time(NULL) - startBitmaskTime;
        bitMaskTaskCount = -1;
        return findInPart(currentPart);
    }

    if (partTaskCount > 0) {
        partTaskCount--;
    }
    if (0 == partTaskCount) {
        partTaskCount = -1;
        int partCount = index->getPartCount();
        stateInfo.progress += 70/partCount;
        currentPart++;
        if (currentPart < partCount && !isCanceled()) {
            subTasks = findInBitMask(currentPart);
            if (subTasks.size() > 0) {
                return subTasks;
            }
        }
    }

    return subTasks;
}

QList<Task*> GenomeAlignerFindTask::findInBitMask(int part) {
    QList<Task*> subTasks;

    int bitValuesCount = bitValuesV.size();
    if (0 == bitValuesCount) {
        bitMaskTaskCount = 0;
        return subTasks;
    }
    int nThreads = AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();
#ifdef _DEBUG
    setMaxParallelSubtasks(1);
#else
    setMaxParallelSubtasks(nThreads);
#endif
    partLoaded = false;
    nextElementToGive = 0;
    if (settings->openCL || settings->useCUDA) {
        bitMaskTaskCount = 1;
    } else {
        bitMaskTaskCount = nThreads;
    }

    for (int i=0; i<bitMaskTaskCount; i++) {
        FindInBitMaskSubTask *subTask = new FindInBitMaskSubTask(index, settings, part,
            bitValuesV.data(), readNumbersV.data(), &bitMaskResults);
        subTask->setSubtaskProgressWeight(0.4f/bitMaskTaskCount);
        subTasks.append(subTask);
    }

    startS.reset(new QSemaphore(bitMaskTaskCount));
    endS.reset(new QSemaphore(bitMaskTaskCount));

    return subTasks;
}

QList<Task*> GenomeAlignerFindTask::findInPart(int part) {
    QList<Task*> subTasks;

    int bitValuesCount = bitValuesV.size();
    int nThreads = AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();
    int partSize = bitValuesCount/nThreads;
    partTaskCount = nThreads;

    nextElementToGive = 0;
    for (int i=0; i<partTaskCount; i++) {
        FindInPartSubTask *subTask = new FindInPartSubTask(index, writeTask, settings,
                                                           bitValuesV.data(),
                                                           readNumbersV.data(),
                                                           positionsAtReadV.data(),
                                                           bitMaskResults);
        int partsInMemCache = index->getPartCount();
        subTask->setSubtaskProgressWeight(0.5f/(partTaskCount*partsInMemCache));
        subTasks.append(subTask);
    }

    return subTasks;
}

void GenomeAlignerFindTask::loadPart(int part) {
    //some parts are commented because of not understandable performance leak

    //SAType first = 0;
    //SAType length = 0;

    mutex.lock();
    if (!partLoaded) {
        taskLog.details(QString("loading part %1").arg(part));
        time_t loadStartTime = time(NULL);
        index->loadPart(part);
        indexLoadTime += time(NULL) - loadStartTime;
        partLoaded = true;
        taskLog.details(QString("finish to load part %1").arg(part));
        startBitmaskTime = time(NULL);
        //endS->acquire(bitMaskTaskCount);
        //startS->acquire(bitMaskTaskCount);
        //nextElementToCalculateBitmask = 0;
    }

    //SAType bitMaskSize = index->getSArraySize();
    //first = nextElementToCalculateBitmask;
    //SAType partSize = bitMaskSize/bitMaskTaskCount + 1;

    //if (first >= bitMaskSize) {
    //    length = 0;
    //} else if (first + partSize > bitMaskSize) {
    //    length = bitMaskSize - first;
    //} else {
    //    length = partSize;
    //}
    //nextElementToCalculateBitmask += length;
    mutex.unlock();

    //SAType last = first + length;
    //index->indexPart.createBitmask((int)first, (int)last);
    //startS->release(1);
    //if (startS->available() == bitMaskTaskCount) {
    //    endS->release(bitMaskTaskCount);
    //}
    //endS->acquire(1);
}

void GenomeAlignerFindTask::getDataForBitMaskSearch(int &first, int &length) {
    if (settings->openCL || settings->useCUDA) {
        first = 0;
        length = bitValuesV.size();
        return;
    }
    mutex.lock();
    int bitValuesCount = bitValuesV.size();
    first = nextElementToGive;
    if (first >= bitValuesCount) {
        length = 0;
    } else if (first + BITMASK_SEARCH_DATA_SIZE > bitValuesCount) {
        length = bitValuesCount - first;
    } else {
        length = BITMASK_SEARCH_DATA_SIZE;
    }
    nextElementToGive += length;
    mutex.unlock();
}

void GenomeAlignerFindTask::getDataForPartSearch(int &first, int &length) {
    mutex.lock();
    int bitValuesCount = bitValuesV.size();
    first = nextElementToGive;
    if (first >= bitValuesCount) {
        length = 0;
    } else if (first + BITMASK_SEARCH_DATA_SIZE > bitValuesCount) {
        length = bitValuesCount - first;
    } else {
        length = BITMASK_SEARCH_DATA_SIZE;
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
    mutex.unlock();
}

GenomeAlignerFindTask::~GenomeAlignerFindTask() {
    delete settings;
    delete[] bitMaskResults;
}

FindInBitMaskSubTask::FindInBitMaskSubTask(GenomeAlignerIndex *i, SearchContext *s, int p,
                                           BMType *bv, int *rn, ResType **bmr)
: Task("FindInBitMaskSubTask", TaskFlag_None), index(i), settings(s), part(p),
bitValues(bv), readNumbers(rn), bitMaskResults(bmr)
{

}

void FindInBitMaskSubTask::run() {
    int first = 0;
    int length = 0;
    GenomeAlignerFindTask *parent = static_cast<GenomeAlignerFindTask*>(getParentTask());
    parent->loadPart(part);
    parent->getDataForBitMaskSearch(first, length);
    SearchQuery **q = settings->queries.data();
    
    taskLog.details(QString("start to find in bitMask"));
    if (settings->openCL) {
        delete[] *bitMaskResults;
        *bitMaskResults = index->findBitOpenCL(bitValues, length, settings->bitFilter);
        if (NULL == *bitMaskResults) {
            setError("OpenCL binary find error");
            return;
        }
    }else if (settings->useCUDA) {
        delete[] *bitMaskResults;
        *bitMaskResults = index->findBitValuesUsingCUDA(bitValues, length, settings->bitFilter);
        if (NULL == *bitMaskResults) {
            setError("CUDA binary search error");
            return;
        }
    }else {
        while (length > 0) {
            int end = first + length;
            ResType *bmr = *bitMaskResults;

            for (int i=first; i<end; i++) {
                int readNum = readNumbers[i];
                if (settings->bestMode && q[readNum]->haveResult()) {
                    continue;
                }
                bmr[i] = index->findBit(bitValues[i], settings->bitFilter);
            }
            parent->getDataForBitMaskSearch(first, length);
        }
    }
    taskLog.details(QString("finish to find in bitMask"));
}

FindInPartSubTask::FindInPartSubTask(GenomeAlignerIndex *i, GenomeAlignerWriteTask *w,
    SearchContext *s, BMType *bv, int *rn, int *par, ResType *bmr)
: Task("FindInPartSubTask", TaskFlag_None), index(i), writeTask(w), settings(s),
bitValues(bv), readNumbers(rn), positionsAtRead(par), bitMaskResults(bmr)
{
}

void FindInPartSubTask::run() {
    int first = 0;
    int length = 0;
    GenomeAlignerFindTask *parent = static_cast<GenomeAlignerFindTask*>(getParentTask());
    parent->getDataForPartSearch(first, length);
    SearchQuery **q = settings->queries.data();
    SearchQuery *revCompl;
        
    taskLog.details(QString("start to find in part"));
    while (length > 0) {
        int last = first + length;
        for (int i=first; i<last; i++) {
            int readNum = readNumbers[i];
            revCompl = q[readNum]->getRevCompl();

            if (settings->bestMode) {
                if (0 == q[readNum]->firstMCount()) {
                    continue;
                }
                if (NULL != revCompl && 0 == revCompl->firstMCount()) {
                    continue;
                }
            }
            index->findInPart(positionsAtRead[i], bitMaskResults[i], bitValues[i], q[readNum], settings);

            if (!settings->bestMode && q[readNum]->haveResult()) {
                if ((i == last - 1) || (readNumbers[i+1] != readNum)) {
                    writeTask->addResult(q[readNum]);
                    q[readNum]->onPartChanged();
                }
            }
        }
        parent->getDataForPartSearch(first, length);
    }
    taskLog.details(QString("finish to find in part"));
}

} // U2
