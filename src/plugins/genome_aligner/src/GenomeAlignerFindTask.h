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

#ifndef _U2_GENOME_ALIGNER_FIND_TASK_H_
#define _U2_GENOME_ALIGNER_FIND_TASK_H_

#include "GenomeAlignerSearchQuery.h"

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>
#include <U2Core/DNASequence.h>
#include <U2Core/U2AssemblyUtils.h>

#include <QtCore/QMutex>
#include <QVector>
#include <QMutex>

#define ResType qint64

namespace U2 {

class GenomeAlignerIndex;
class FindInPartSubTask;
class PrepareVectorsSubTask;

class SearchContext {
public:
    int w;
    int ptMismatches;
    int nMismatches;
    bool absMismatches;
    bool bestMode;
    bool openCL;
    int minReadLength;
    int maxReadLength;
    quint64 bitFilter;
    QVector<quint64> bitValuesV;
    QVector<int> readNumbersV;
    QVector<int> positionsAtReadV;
    int *partNumbers;
    QVector<SearchQuery*> queries;
};

#define MAX_PERCENTAGE 100
class GenomeAlignerFindTask : public Task {
    Q_OBJECT
    friend class PrepareVectorsSubTask;
public:
    GenomeAlignerFindTask(GenomeAlignerIndex *i, const SearchContext &s);
    ~GenomeAlignerFindTask();
    virtual void run();
    virtual void prepare();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    QMutex &getPartLoadMutex() {return partLoadMutex;}
    bool isPartLoaded() const {return partLoaded;}
    void setPartLoaded() {partLoaded = true;}

private:
    FindInPartSubTask *findInPartTask;
    PrepareVectorsSubTask *prepVecTask;
    GenomeAlignerIndex *index;
    SearchContext *settings;
    int memCacheTaskCount;
    QVector<quint64> *bitValuesV;
    QVector<int> *readNumbersV;
    QVector<int> *positionsAtReadV;
    ResType *bitMaskResults;
    int currentPart;
    int bitMaskTaskCount;
    int partTaskCount;
    bool partLoaded;
    QMutex partLoadMutex;

    void findInMemCache();
    QList<Task*> findInBitMask(int part);
    QList<Task*> findInPart(int part);
    void prepareBitValues();
    void prepareVectors();
};

typedef QVector<SearchQuery*>::iterator QueryIter;

class FindInMemCacheSubTask : public Task {
    Q_OBJECT
public:
    FindInMemCacheSubTask(GenomeAlignerIndex *index,
                          SearchContext *settings,
                          int first, int length);
    virtual void run();
private:
    GenomeAlignerIndex *index;
    SearchContext *settings;
    int first;
    int length;
};

class FindInBitMaskSubTask : public Task {
    Q_OBJECT
public:
    FindInBitMaskSubTask(GenomeAlignerIndex *index,
                         SearchContext *settings,
                         int part,
                         int first, int length,
                         quint64 *bitValues,
                         int *readNumbers,
                         ResType **bitMaskResults);
    virtual void run();
private:
    GenomeAlignerIndex *index;
    SearchContext *settings;
    int part;
    int first;
    int length;
    quint64 *bitValues;
    int *readNumbers;
    ResType **bitMaskResults;
};

class FindInPartSubTask : public Task {
    Q_OBJECT
public:
    FindInPartSubTask(GenomeAlignerIndex *index,
                      SearchContext *settings,
                      int first, int length,
                      quint64 *bitValues,
                      int *readNumbers,
                      int *positionsAtRead,
                      ResType *bitMaskResults);
    virtual void run();
private:
    GenomeAlignerIndex *index;
    SearchContext *settings;
    int first;
    int length;
    quint64 *bitValues;
    int *readNumbers;
    int *positionsAtRead;
    ResType *bitMaskResults;
};

class PrepareVectorsSubTask : public Task {
    Q_OBJECT
public:
    PrepareVectorsSubTask();
    virtual void run();
};

} //U2

#endif // _U2_GENOME_ALIGNER_FIND_TASK_H_
