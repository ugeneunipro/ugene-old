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
#include "ReadShortReadsSubTask.h"
#include "GenomeAlignerTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/TextUtils.h>
#include <U2Core/Timer.h>

namespace U2 {

#define ALIGN_DATA_SIZE 1000

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
        SearchQuery *query = NULL;
        if (NULL == *lastQuery) {
            query = seqReader->read();
        } else {
            query = *lastQuery;
        }
        if (NULL == query) {
            if (!seqReader->isEnd()) {
                setError("Short-reads object type must be a sequence, but not a multiple alignment");
                readingFinishedWakeAll();
                return;
            }
            break;
        }

        if ( qualityThreshold > 0 && query->hasQuality() ) {
            // simple quality filtering
            bool ok = isDnaQualityAboveThreshold(query->getQuality(), qualityThreshold);
            if (!ok) {
                continue;
            }
        }

        if (GenomeAlignerTask::MIN_SHORT_READ_LENGTH <= query->length()) {
            if (alignContext.minReadLength > query->length()) {
                alignContext.minReadLength = query->length();
            }
            if (alignContext.maxReadLength < query->length()) {
                alignContext.maxReadLength = query->length();
            }
        }

        n = alignContext.absMismatches ? alignContext.nMismatches+1 : (query->length()*alignContext.ptMismatches/100)+1;

        qint64 qualLength = 0;
        if (query->hasQuality()){
            qualLength = query->getQuality().qualCodes.length();
        }
        qint64 memoryRequiredForOneRead = n*24 +  // 2*(long long + int) == 24
            sizeof(SearchQuery) +
            ONE_SEARCH_QUERY_SIZE + query->length() +
            query->getNameLength() +
            qualLength;
        memoryRequiredForOneRead *= 4; // FIXME: UGENE-1114


        if (alignReversed) {
            m -= 2*memoryRequiredForOneRead;
            alignBunchSize += 2;
        } else {
            m -= memoryRequiredForOneRead;
            alignBunchSize++;
        }
        if (m<=0) {
            delete *lastQuery;
            *lastQuery = query;
            break;
        }

        if (!add(CMAX, W, q, readNum, query, parent)) {
            delete query;
            continue;
        }

        ++bunchSize;
        *lastQuery = NULL;

        if (alignReversed) {
            QByteArray reversed(query->constSequence());
            TextUtils::reverse(reversed.data(), reversed.count());
            SearchQuery *rQu = new SearchQuery(new DNASequence(QString("%1_rev").arg(query->getName()), reversed, NULL), query);
            transl->translate(const_cast<char*>(rQu->constData()), rQu->length());
            if (rQu->constSequence() != query->constSequence()) {
                query->setRevCompl(rQu);
                add(CMAX, W, q, readNum, rQu, parent);
            } else {
                delete rQu;
            }
        }

        if (!alignContext.openCL) {
            if (alignBunchSize > ALIGN_DATA_SIZE) {
                alignContext.readShortReadsWait.wakeAll();
                alignBunchSize=0; // it's nice to have a warm computer in winter, but let aligner threads sleep more
            }
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
