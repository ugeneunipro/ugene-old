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
#include "ReadShortReadsSubTask.h"
#include "GenomeAlignerTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/TextUtils.h>
#include <U2Core/Timer.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

#define ALIGN_DATA_SIZE 1000000

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

static bool checkDnaQuality(SearchQuery* query, int qualityThreshold) {
    if (!(qualityThreshold > 0 && query->hasQuality())) {
        return true;
    }

    // simple quality filtering
    return isDnaQualityAboveThreshold(query->getQuality(), qualityThreshold);
}

static void updateMinMaxReadLengths(AlignContext& alignContext, int l) {
    if (GenomeAlignerTask::MIN_SHORT_READ_LENGTH <= l) {
        if (alignContext.minReadLength > l) {
            alignContext.minReadLength = l;
        }
        if (alignContext.maxReadLength < l) {
            alignContext.maxReadLength = l;
        }
    }
}

static SearchQuery* createRevComplQuery(SearchQuery* query, DNATranslation* transl) {
    SAFE_POINT(query != NULL, "Query is null", NULL);
    SAFE_POINT(transl != NULL, "Transl is null", NULL);

    QByteArray reversed(query->constSequence());
    TextUtils::reverse(reversed.data(), reversed.count());

    DNASequence dnaSeq(QString("%1_rev").arg(query->getName()), reversed, NULL);
    SearchQuery *rQu = new SearchQuery(&dnaSeq, query);
    transl->translate(const_cast<char*>(rQu->constData()), rQu->length());

    if (rQu->constSequence() == query->constSequence()) {
        delete rQu; rQu = NULL;
        return NULL;
    }

    query->setRevCompl(rQu);
    return rQu;
}

static void cleanAlignContextVectors(AlignContext& alignContext) {
    foreach (SearchQuery *qu, alignContext.queries) {
        delete qu;
    }

    {
        // Noone can touch these vectors without sync
        QMutexLocker lock(&alignContext.listM);
        const int vectorsReserve = 2*1024*1024; // why realloc often; little optimization
        alignContext.queries.resize(0); alignContext.queries.reserve(vectorsReserve);
        alignContext.bitValuesV.resize(0); alignContext.bitValuesV.reserve(vectorsReserve);
        alignContext.windowSizes.resize(0); alignContext.windowSizes.reserve(vectorsReserve);
        alignContext.readNumbersV.resize(0); alignContext.readNumbersV.reserve(vectorsReserve);
        alignContext.positionsAtReadV.resize(0); alignContext.positionsAtReadV.reserve(vectorsReserve);
    }
}

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

void ReadShortReadsSubTask::readingFinishedWakeAll() {
    taskLog.trace("Wake all");

    QMutexLocker(&alignContext.readingStatusMutex);
    alignContext.isReadingFinished = true;
    alignContext.readShortReadsWait.wakeAll();
}

void ReadShortReadsSubTask::run() {
    stateInfo.setProgress(0);
    GTIMER(cvar, tvar, "ReadSubTask");
    GenomeAlignerTask *parent = static_cast<GenomeAlignerTask*>(getParentTask());
    if (!alignContext.bestMode) {
        parent->pWriteTask->flush();
    }

    cleanAlignContextVectors(alignContext);

    if (isCanceled()) {
        readingFinishedWakeAll();
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
        SearchQuery *query = seqReader->read();
        if (NULL == query) {
            if (!seqReader->isEnd()) {
                setError("Short-reads object type must be a sequence, but not a multiple alignment");
                readingFinishedWakeAll();
                return;
            }
            break;
        }
        ++bunchSize;
 
        if (!checkDnaQuality(query, qualityThreshold)) {
            continue;
        }
        updateMinMaxReadLengths(alignContext, query->length());

        if (!add(CMAX, W, q, readNum, query, parent)) {
            delete query;
            continue;
        }
        m -= query->memoryHint();
        alignBunchSize++;

        if (alignReversed) {
            SearchQuery *rQu = createRevComplQuery(query, transl);
            if (rQu) {
                add(CMAX, W, q, readNum, rQu, parent);
                m -= rQu->memoryHint();
                alignBunchSize++;
            }
        }

        qint64 alignContextMemoryHint = alignContext.memoryHint();
        if (m - alignContextMemoryHint<=0) {
            break;
        }

        if (alignBunchSize > ALIGN_DATA_SIZE && !alignContext.openCL) {
            taskLog.trace("Wake up");
            taskLog.trace(QString("Readed %1 reads").arg(bunchSize));

            alignContext.readShortReadsWait.wakeOne();
            alignBunchSize=0;
        }
    }

    readingFinishedWakeAll();
}

inline bool ReadShortReadsSubTask::add(int &CMAX, int &W, int &q, int &readNum, SearchQuery *query, GenomeAlignerTask *parent) {
    // ReadShortReadsSubTask is adding new data what can lead to realloc. Noone can touch these vectors without sync
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

//     // !!!Stress testing!!!
//     alignContext.bitValuesV.squeeze();
//     alignContext.readNumbersV.squeeze();
//     alignContext.positionsAtReadV.squeeze();
//     alignContext.windowSizes.squeeze();
//     alignContext.queries.squeeze();

    return true;
}

} // U2
