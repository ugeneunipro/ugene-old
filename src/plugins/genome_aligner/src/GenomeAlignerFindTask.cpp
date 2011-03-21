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

#include "GenomeAlignerFindTask.h"
#include "GenomeAlignerIndex.h"
#include "GenomeAlignerTask.h"

namespace U2 {

GenomeAlignerFindTask::GenomeAlignerFindTask(U2::GenomeAlignerIndex *i, const SearchContext &s)
: Task("GenomeAlignerFindTask", TaskFlag_None), findInPartTask(NULL), prepVecTask(NULL),
index(i), settings(new SearchContext(s)), memCacheTaskCount(-1),
bitMaskTaskCount(-1), partTaskCount(-1)
{
    bitValuesV = NULL;
    readNumbersV = NULL;
    positionsAtReadV = NULL;
    bitMaskResults = NULL;
    currentPart = 0;
    maxPtMismatches = settings->ptMismatches;
    maxNMismatches = settings->nMismatches;
    partLoaded = false;
}

void GenomeAlignerFindTask::prepare() {
    if (settings->bestMode) {
        settings->nMismatches = 0;
        settings->ptMismatches = 0;
    }
    settings->w = GenomeAlignerTask::calculateWindowSize(settings->absMismatches,
        settings->nMismatches, settings->ptMismatches, settings->minReadLength, settings->maxReadLength);
    settings->bitFilter = ((quint64)0 - 1)<<(62 - settings->w*2);
    prepareBitValues();
    if (isCanceled() || hasErrors()) {
        return;
    }
    findInMemCache();
}

void GenomeAlignerFindTask::prepareBitValues() {
    taskLog.details("start to calculate bitValues");
    int CMAX = maxNMismatches;
    int W = 0;
    int q = 0;

    SearchQuery *qu;
    int readNum = 0;
    int w = GenomeAlignerTask::calculateWindowSize(settings->absMismatches,
        maxNMismatches, maxPtMismatches, settings->minReadLength, settings->maxReadLength);
    for (QueryIter it=settings->queries.begin(); it!=settings->queries.end(); it++, readNum++) {
        qu = *it;
        W = qu->shortRead.length();
        if (!settings->absMismatches) {
            CMAX = (W * maxPtMismatches) / MAX_PERCENTAGE;
        }
        q = W / (CMAX + 1);
        assert(q >= w);

        const char* querySeq = qu->shortRead.constData();
        for (int i = 0; i < W - w + 1; i+=q) {
            const char *seq = querySeq + i;
            settings->bitValuesV.push_back(index->getBitValue(seq, qMin(GenomeAlignerIndex::charsInMask, W - i)));
            settings->readNumbersV.push_back(readNum);
            settings->positionsAtReadV.push_back(i);
        }
    }
    settings->partNumbers = new int[settings->bitValuesV.size()];
    taskLog.details("finish to calculate bitValues");
}

void GenomeAlignerFindTask::run() {

}

QList<Task*> GenomeAlignerFindTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> subTasks;
    if (subTask->hasErrors() || isCanceled()) {
        return subTasks;
    }

    int partsInMemCache = index->getPartsInMemCache();
    if (subTask == prepVecTask) {
        delete[] settings->partNumbers;
        settings->partNumbers = NULL;

        for (currentPart = 0; currentPart < partsInMemCache; currentPart++) {
            subTasks = findInBitMask(currentPart);
            if (subTasks.size() > 0) {
                return subTasks;
            }
        }
        return subTasks;
    }

    if (memCacheTaskCount > 0) {
        memCacheTaskCount--;
        if (0 == memCacheTaskCount) { //search in memCache is finished
            prepVecTask = new PrepareVectorsSubTask();
            subTasks.append(prepVecTask);
            return subTasks;
        } else {
            return subTasks;
        }
    }

    if (bitMaskTaskCount > 0) {
        bitMaskTaskCount--;
    }

    if (partTaskCount > 0) {
        partTaskCount--;

        if (0 == partTaskCount) {
            stateInfo.progress += 70/partsInMemCache;
            currentPart++;
            if (currentPart < partsInMemCache && !isCanceled()) {
                for (; currentPart < partsInMemCache; currentPart++) {
                    subTasks = findInBitMask(currentPart);
                    if (subTasks.size() > 0) {
                        return subTasks;
                    }
                }
                return subTasks;
            }
            if (settings->bestMode) {
                if (!(settings->absMismatches?
                    settings->nMismatches>=maxNMismatches :
                    settings->ptMismatches>=maxPtMismatches)) {
                    if (settings->absMismatches) {
                        settings->nMismatches++;
                    } else {
                        settings->ptMismatches += 3;
                        if (settings->ptMismatches > maxPtMismatches) {
                            settings->ptMismatches = maxPtMismatches;
                        }
                    }
                    if (settings->nMismatches > maxNMismatches
                        || settings->ptMismatches > maxPtMismatches) {
                            return subTasks;
                    }
                    settings->w = GenomeAlignerTask::calculateWindowSize(settings->absMismatches,
                        settings->nMismatches, settings->ptMismatches, settings->minReadLength, settings->maxReadLength);
                    settings->bitFilter = ((quint64)0 - 1)<<(62 - settings->w*2);
                    currentPart = 0;
                    return findInBitMask(currentPart);
                }
            }
        }
        return subTasks;
    }

    if (0 == bitMaskTaskCount && !isCanceled()) {
        return findInPart(currentPart);
    }

    return subTasks;
}

void GenomeAlignerFindTask::findInMemCache() {
    int bitValuesCount = settings->bitValuesV.size();
    int nThreads = AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();
    setMaxParallelSubtasks(nThreads);
    int partSize = bitValuesCount/nThreads;
    if (0 == partSize || 1 == index->getPartsInMemCache()) { //bitValuesCount < nThreads
        partSize = bitValuesCount;
        memCacheTaskCount = 1;
    } else {
        memCacheTaskCount = nThreads;
    }    
    int it = 0;
    int length = 0;
    for (int i=0; i<memCacheTaskCount; i++) {
        int first = it;
        it += partSize;
        if (memCacheTaskCount-1 == i) {
            length = bitValuesCount - i*partSize;
        } else {
            length = partSize;
        }
        FindInMemCacheSubTask *subTask = new FindInMemCacheSubTask(index, settings, first, length);
        subTask->setSubtaskProgressWeight(0.1f/memCacheTaskCount);
        addSubTask(subTask);
    }
}

QList<Task*> GenomeAlignerFindTask::findInBitMask(int part) {
    QList<Task*> subTasks;

    int bitValuesCount = bitValuesV[part].size();
    if (0 == bitValuesCount) {
        bitMaskTaskCount = 0;
        return subTasks;
    }
    int nThreads = AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();
    int partSize = bitValuesCount/nThreads;

    partLoaded = false;
    if (settings->openCL) {
        bitMaskTaskCount = 1;
        FindInBitMaskSubTask *subTask = new FindInBitMaskSubTask(index, settings, part, 0, bitValuesCount,
            bitValuesV[part].data(), readNumbersV[part].data(), &bitMaskResults);
        subTask->setSubtaskProgressWeight(0.4f/bitMaskTaskCount);
        subTasks.append(subTask);
    } else {
        if (0 == partSize) { //bitValuesCount < nThreads
            partSize = bitValuesCount;
            bitMaskTaskCount = 1;
        } else {
            bitMaskTaskCount = nThreads;
        }
        int it = 0;
        int length = 0;
        for (int i=0; i<bitMaskTaskCount; i++) {
            int first = it;
            it += partSize;
            if (bitMaskTaskCount-1 == i) {
                length = bitValuesCount - i*partSize;
            } else {
                length = partSize;
            }
            FindInBitMaskSubTask *subTask = new FindInBitMaskSubTask(index, settings, part, first, length,
                bitValuesV[part].data(), readNumbersV[part].data(), &bitMaskResults);
            subTask->setSubtaskProgressWeight(0.4f/bitMaskTaskCount);
            subTasks.append(subTask);
        }
    }

    return subTasks;
}

QList<Task*> GenomeAlignerFindTask::findInPart(int part) {
    QList<Task*> subTasks;

    int bitValuesCount = bitValuesV[part].size();
    int nThreads = AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();
    int partSize = bitValuesCount/nThreads;

    if (0 == partSize) { //bitValuesCount < nThreads
        partSize = bitValuesCount;
        partTaskCount = 1;
    } else {
        partTaskCount = nThreads;
    }
    int it = 0;
    int length = 0;
    for (int i=0; i<partTaskCount; i++) {
        int first = it;
        it += partSize;
        if (partTaskCount-1 == i) {
            length = bitValuesCount - i*partSize;
        } else {
            length = partSize;
        }
        FindInPartSubTask *subTask = new FindInPartSubTask(index, settings, first, length,
                                                           bitValuesV[part].data(),
                                                           readNumbersV[part].data(),
                                                           positionsAtReadV[part].data(),
                                                           bitMaskResults);
        int partsInMemCache = index->getPartsInMemCache();
        subTask->setSubtaskProgressWeight(0.5f/(partTaskCount*partsInMemCache));
        subTasks.append(subTask);
    }

    return subTasks;
}

GenomeAlignerFindTask::~GenomeAlignerFindTask() {
    delete settings;
    delete[] bitValuesV;
    delete[] readNumbersV;
    delete[] positionsAtReadV;
    delete[] bitMaskResults;
}

FindInMemCacheSubTask::FindInMemCacheSubTask(GenomeAlignerIndex *i, SearchContext *s, int f, int l)
: Task("FindInMemCacheSubTask", TaskFlag_None), index(i), settings(s), first(f), length(l)
{

}

void FindInMemCacheSubTask::run() {
    if (1 == index->getPartsInMemCache()) {
        return;
    }
    quint64 *bitValues = settings->bitValuesV.data();
    int last = first + length;
    quint64 bitFilter = ((quint64)0 - 1)<<(62 - GenomeAlignerTask::MIN_BIT_MASK_LENGTH);
    taskLog.details(QString("start to find in memCache from %1 to %2").arg(first).arg(last-1));
    for (int i=first; i<last; i++) {
        if (isCanceled()) {
            break;
        }
        settings->partNumbers[i] = index->findInCache(bitValues[i], bitFilter);
    }
    taskLog.details(QString("finish to find in memCache from %1 to %2").arg(first).arg(last-1));
}

FindInBitMaskSubTask::FindInBitMaskSubTask(GenomeAlignerIndex *i, SearchContext *s, int p,
                                           int f, int l, quint64 *bv, int *rn, ResType **bmr)
: Task("FindInBitMaskSubTask", TaskFlag_None), index(i), settings(s), part(p),
first(f), length(l), bitValues(bv), readNumbers(rn), bitMaskResults(bmr)
{

}

void FindInBitMaskSubTask::run() {
    GenomeAlignerFindTask *parent = static_cast<GenomeAlignerFindTask*>(getParentTask());
    QMutex &m = parent->getPartLoadMutex();
    m.lock();
    if (!parent->isPartLoaded()) {
        index->loadPart(part);
    }
    m.unlock();
    
    taskLog.details(QString("start to find in bitMask from %1 to %2").arg(first).arg(first + length - 1));
    if (settings->openCL) {
        delete[] *bitMaskResults;
        *bitMaskResults = index->findBitOpenCL(bitValues, length, settings->bitFilter);
        if (NULL == *bitMaskResults) {
            setError("OpenCL binary find error");
            return;
        }
    } else {
        int end = first + length;
        ResType *bmr = *bitMaskResults;

        for (int i=first; i<end; i++) {
            int readNum = readNumbers[i];
            if (!settings->queries.at(readNum)->results.isEmpty() && settings->bestMode) {
                continue;
            }
            bmr[i] = index->findBit(bitValues[i], settings->bitFilter);
        }
    }
    taskLog.details(QString("finish to find in bitMask from %1 to %2").arg(first).arg(first + length - 1));
}

FindInPartSubTask::FindInPartSubTask(GenomeAlignerIndex *i, SearchContext *s, int f, int l,
    quint64 *bv, int *rn, int *par, ResType *bmr)
: Task("FindInPartSubTask", TaskFlag_None), index(i), settings(s), first(f), length(l),
bitValues(bv), readNumbers(rn), positionsAtRead(par), bitMaskResults(bmr)
{

}

void FindInPartSubTask::run() {
    taskLog.details(QString("start to find in part from %1 to %2").arg(first).arg(first + length - 1));
    QFile *refFile = index->openRefFile();
    int last = first + length;
    for (int i=first; i<last; i++) {
        int readNum = readNumbers[i];
        if (!settings->queries.at(readNum)->results.isEmpty() && settings->bestMode) {
            continue;
        }
        index->findInPart(refFile, settings->queries.at(readNum)->shortRead.constSequence(), positionsAtRead[i],
            bitMaskResults[i], bitValues[i], settings->queries.at(readNum)->results, settings);
    }
    refFile->close();
    delete refFile;
    taskLog.details(QString("finish to find in part from %1 to %2").arg(first).arg(first + length - 1));
}

PrepareVectorsSubTask::PrepareVectorsSubTask()
: Task("PrepareVectorsSubTask", TaskFlag_None)
{

}

void PrepareVectorsSubTask::run() {
    GenomeAlignerFindTask *parent = static_cast<GenomeAlignerFindTask*>(getParentTask()) ;
    taskLog.details("start to prepare vectors after searching in memCache");
    int partsInMemCache = parent->index->getPartsInMemCache();
    parent->bitValuesV = new QVector<quint64>[partsInMemCache];
    parent->readNumbersV = new QVector<int>[partsInMemCache];
    parent->positionsAtReadV = new QVector<int>[partsInMemCache];

    if (1 == partsInMemCache) {
        parent->bitValuesV[0] << parent->settings->bitValuesV;
        parent->readNumbersV[0] << parent->settings->readNumbersV;
        parent->positionsAtReadV[0] << parent->settings->positionsAtReadV;

        parent->settings->bitValuesV.clear();
        parent->settings->readNumbersV.clear();
        parent->settings->positionsAtReadV.clear();
    } else {
        int part = 0;
        int size = parent->settings->bitValuesV.size();
        quint64 *bv = parent->settings->bitValuesV.data();
        int *rn = parent->settings->readNumbersV.data();
        int *par = parent->settings->positionsAtReadV.data();
        for (int i=0; i<size; i++) {
            part = parent->settings->partNumbers[i];
            if (part >= 0) {
                parent->bitValuesV[part].push_back(bv[i]);
                parent->readNumbersV[part].push_back(rn[i]);
                parent->positionsAtReadV[part].push_back(par[i]);
            }
        }
        parent->settings->bitValuesV.clear();
        parent->settings->readNumbersV.clear();
        parent->settings->positionsAtReadV.clear();
    }

    if (!parent->settings->openCL) {
        int maxSize = 0;
        for (int i=0; i<partsInMemCache; i++) {
            if (maxSize < parent->bitValuesV[i].size()) {
                maxSize = parent->bitValuesV[i].size();
            }
        }
        if (maxSize > 0) {
            parent->bitMaskResults = new ResType[maxSize];
        }
    }
    taskLog.details("finish to prepare vectors after searching in memCache");
}

} // U2
