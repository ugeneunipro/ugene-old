/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GENOME_ALIGNER_FIND_TASK_H_
#define _U2_GENOME_ALIGNER_FIND_TASK_H_

#include "GenomeAlignerSearchQuery.h"
#include "GenomeAlignerWriteTask.h"

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>
#include <U2Core/DNASequence.h>
#include <U2Core/U2AssemblyUtils.h>

#include <QtCore/QMutex>
#include <QVector>
#include <QMutex>
#include <QSemaphore>
#include <memory>

#define BinarySearchResult qint64

namespace U2 {

class GenomeAlignerIndex;
class FindInPartSubTask;
class PrepareVectorsSubTask;

class AlignContext {
public:
    AlignContext(): w(-1), ptMismatches(0), nMismatches(0), absMismatches(0), bestMode(false),
        openCL(false), minReadLength(-1), maxReadLength(-1), /*bitFilter(0),*/ isReadingFinished(false) {}
    int w;
    int ptMismatches;
    int nMismatches;
    bool absMismatches;
    bool bestMode;
    bool openCL;
    int minReadLength;
    int maxReadLength;
    QVector<SearchQuery*> queries;
    QVector<BMType> bitValuesV;
    QVector<int> windowSizes;
    QVector<int> readNumbersV;
    QVector<int> positionsAtReadV;

    bool isReadingFinished;
    bool isReadingStarted;
    QMutex listM, readingStatusMutex;
    QWaitCondition readShortReadsWait;

    qint64 memoryHint() const {
        qint64 m = sizeof(*this);

        m += queries.capacity() * (qint64)sizeof(SearchQuery*);
        m += bitValuesV.capacity() * (qint64)sizeof(BMType);
        m += readNumbersV.capacity() * (qint64)sizeof(int);
        m += positionsAtReadV.capacity() * (qint64)sizeof(int);
        m += windowSizes.capacity() * (qint64)sizeof(int);

        return m;
    }
};

#define MAX_PERCENTAGE 100
class GenomeAlignerFindTask : public Task {
    Q_OBJECT
    friend class ShortReadAlignerCPU;
    friend class ShortReadAlignerOpenCL;
public:
    GenomeAlignerFindTask(GenomeAlignerIndex *i, AlignContext *s, GenomeAlignerWriteTask *writeTask);
    virtual void run();
    virtual void prepare();

    void loadPartForAligning(int part);
    void waitDataForAligning(int &first, int &length);

    qint64 getIndexLoadTime() const {return indexLoadTime;}

private:
    GenomeAlignerIndex *index;
    GenomeAlignerWriteTask *writeTask;
    AlignContext *alignContext;

    int alignerTaskCount;
    int waiterCount;
    int nextElementToGive;
    qint64 indexLoadTime;
    bool partLoaded;

    QMutex loadPartMutex;
    QMutex waitDataForAligningMutex;
    QMutex waitMutex;
    QWaitCondition waiter;

    void unsafeGetData(int &first, int &length);

    static const int ALIGN_DATA_SIZE;
};

typedef QVector<SearchQuery*>::iterator QueryIter;

class ShortReadAlignerCPU : public Task {
    Q_OBJECT
public:
    ShortReadAlignerCPU(int taskNo, GenomeAlignerIndex *index, AlignContext *alignContext, GenomeAlignerWriteTask *writeTask);
    virtual void run();
private:
    int taskNo;
    GenomeAlignerIndex *index;
    AlignContext *alignContext;
    GenomeAlignerWriteTask *writeTask;
};

class ShortReadAlignerOpenCL : public Task {
    Q_OBJECT
public:
    ShortReadAlignerOpenCL(int taskNo, GenomeAlignerIndex *index, AlignContext *alignContext, GenomeAlignerWriteTask *writeTask);
    virtual void run();
private:
    int taskNo;
    GenomeAlignerIndex *index;
    AlignContext *alignContext;
    GenomeAlignerWriteTask *writeTask;
};

} //U2

#endif // _U2_GENOME_ALIGNER_FIND_TASK_H_
