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

#include "MultiTableAssemblyAdapter.h"
#include "../SQLiteDbi.h"
#include "../SQLiteAssemblyDbi.h"
#include "../SQLiteObjectDbi.h"

#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2SqlHelpers.h>

namespace U2 {


MultiTableAssemblyAdapter::MultiTableAssemblyAdapter(SQLiteDbi* _dbi, const U2DataId& assemblyId, 
                                                       const AssemblyCompressor* compressor, 
                                                       MultiTableAssemblyAdapterMode mode,
                                                       DbRef* db, U2OpStatus& os)
                                                       : AssemblyAdapter(assemblyId, compressor, db, os)
{
    dbi = _dbi;
    assert( mode == MultiTableAssemblyAdapterMode_4Tables);
    if (mode != MultiTableAssemblyAdapterMode_4Tables) {
        os.setError(SQLiteL10N::tr("Unsupported multi-table mode for assembly reads. Mode: %1").arg(mode));
    }
    addTableAdapter(0, 50, assemblyId, compressor, false, os);
    addTableAdapter(50, 100, assemblyId, compressor, false, os);
    addTableAdapter(100, 700, assemblyId, compressor, false, os);
    addTableAdapter(700, INT_MAX, assemblyId, compressor, true, os);
}

MultiTableAssemblyAdapter::~MultiTableAssemblyAdapter() {
    qDeleteAll(tableAdapters);
}

void MultiTableAssemblyAdapter::addTableAdapter(int minLen, int maxLen, const U2DataId& assemblyId, 
                                                const AssemblyCompressor* compressor, bool last, U2OpStatus& os) 
{
    QString tableSuffix  = QString("_%1_%2").arg(minLen).arg(last ? QString("U") : QString::number(maxLen));
    SingleTableAssemblyAdapter* adapter = new SingleTableAssemblyAdapter(dbi, assemblyId, tableSuffix, compressor, db, os);
    if (!last) {
        adapter->enableRangeTableMode(minLen, maxLen);
    }
    QByteArray idExtra;
    idExtra.append(char('0' + tableAdapters.size()));
    tableAdapters << new MTASingleTableAdapter(adapter, U2Region(minLen, maxLen - minLen), idExtra);
    idExtras << idExtra;
}

void MultiTableAssemblyAdapter::createReadsTables(U2OpStatus& os) {
    foreach(MTASingleTableAdapter* a, tableAdapters) {
        a->singleTableAdapter->createReadsTables(os);
        if (os.hasError()) {
            break;
        }
    }
}

void MultiTableAssemblyAdapter::createReadsIndexes(U2OpStatus& os) {
    foreach(MTASingleTableAdapter* a, tableAdapters) {
        a->singleTableAdapter->createReadsIndexes(os);
        if (os.hasError()) {
            break;
        }
    }
}

#define MAX_READS_TO_USE_PRECISE_COUNT 100
qint64 MultiTableAssemblyAdapter::countReads(const U2Region& r, U2OpStatus& os) {
    qint64 sum = 0;
    foreach(MTASingleTableAdapter* a, tableAdapters) {
        int n = a->singleTableAdapter->countReads(r, os);
        if (n < MAX_READS_TO_USE_PRECISE_COUNT) {
            n = a->singleTableAdapter->countReadsPrecise(r, os);    
        }
        if (os.hasError()) {
            break;
        }
        sum+=n;
    }
    return sum;
}

qint64 MultiTableAssemblyAdapter::getMaxPackedRow(const U2Region& r, U2OpStatus& os) {
    qint64 max = 0;
    foreach(MTASingleTableAdapter* a, tableAdapters) {
        qint64 n = a->singleTableAdapter->getMaxPackedRow(r, os);
        if (os.hasError()) {
            break;
        }
        max = qMax(max, n);
    }
    return max;
}

qint64 MultiTableAssemblyAdapter::getMaxEndPos(U2OpStatus& os) {
    //TODO: optimize by using gstart + maxReadLen for first n-1 tables
    qint64 max = 0;
    foreach(MTASingleTableAdapter* a, tableAdapters) {
        qint64 n = a->singleTableAdapter->getMaxEndPos(os);
        if (os.hasError()) {
            break;
        }
        max = qMax(max, n);
    }
    return max;
}

U2DbiIterator<U2AssemblyRead>* MultiTableAssemblyAdapter::getReads(const U2Region& r, U2OpStatus& os) {
    QList< U2DbiIterator<U2AssemblyRead>* > iterators;
    foreach(MTASingleTableAdapter* a, tableAdapters) {
        iterators << a->singleTableAdapter->getReads(r, os);
        if (os.hasError()) {
            break;
        }
    }
    if (os.hasError()) {
        qDeleteAll(iterators);
        return NULL;
    } 
    return new MTAReadsIterator(iterators, idExtras);
}

U2DbiIterator<U2AssemblyRead>* MultiTableAssemblyAdapter::getReadsByRow(const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os) {
    QList< U2DbiIterator<U2AssemblyRead>* > iterators;
    foreach(MTASingleTableAdapter* a, tableAdapters) {
        iterators << a->singleTableAdapter->getReadsByRow(r, minRow, maxRow, os);
        if (os.hasError()) {
            break;
        }
    }
    if (os.hasError()) {
        qDeleteAll(iterators);
        return NULL;
    } 
    return new MTAReadsIterator(iterators, idExtras);
}

U2DbiIterator<U2AssemblyRead>* MultiTableAssemblyAdapter::getReadsByName(const QByteArray& name, U2OpStatus& os) {
    QList< U2DbiIterator<U2AssemblyRead>* > iterators;
    foreach(MTASingleTableAdapter* a, tableAdapters) {
        iterators << a->singleTableAdapter->getReadsByName(name, os);
        if (os.hasError()) {
            break;
        }
    }
    if (os.hasError()) {
        qDeleteAll(iterators);
        return NULL;
    } 
    return new MTAReadsIterator(iterators, idExtras);
}

int MultiTableAssemblyAdapter::getReadRange(const U2DataId& id) const {
    QByteArray extra = SQLiteUtils::toDbExtra(id);
    assert(extra.size() == 1);
    if (extra.size() != 1) {
        return -1;
    } 
    return extra[0] - '0';
}

int MultiTableAssemblyAdapter::getReadRange(qint64 readLength) const {
    for (int i = 0, nRanges = tableAdapters.size(); i < nRanges; i++) {
        const MTASingleTableAdapter* a = tableAdapters.at(i);
        if (a->readsRange.contains(readLength)) {
            return i;
        }
    }
    assert(0);
    return tableAdapters.size() - 1;
}

static U2DataId addTable2Id(const U2DataId& id, const QByteArray& idExtra) {
    assert(SQLiteUtils::toDbExtra(id).isEmpty());
    U2DataId res = SQLiteUtils::toU2DataId(SQLiteUtils::toDbiId(id), U2Type::AssemblyRead, idExtra); 
    return res;
}

void MultiTableAssemblyAdapter::addReads(QList<U2AssemblyRead>& reads, U2OpStatus& os) {
    int nReads = reads.size();
    int nRanges = tableAdapters.size();
    QVector < QList<U2AssemblyRead> > readsByRange(nRanges); //reads sorted by range
    QVector< QList<int> > readsIndex(nRanges); // back-mapping of read index to original 'reads' list
    for (int i = 0; i < nReads; i++) {
        U2AssemblyRead& read = reads[i];
        int readLen = read->readSequence.length();
        read->effectiveLen = readLen + U2AssemblyUtils::getCigarExtraLength(read->cigar);
        int rangeNum = getReadRange(read->effectiveLen);
        readsByRange[rangeNum] << read;
        readsIndex[rangeNum] << i;
        
    }
    for (int i = 0; i < nRanges; i++) {
        QList<U2AssemblyRead>& rangeReads = readsByRange[i];
        MTASingleTableAdapter* adapter = tableAdapters[i];
        adapter->singleTableAdapter->addReads(rangeReads, os);
        if (os.hasError()) {
            break;
        }

        //now back-map all reads to initial list
        const QList<int>& idxMap = readsIndex[i];
        for (int j = 0, n = rangeReads.size(); j < n; j++) {
            int idx = idxMap[j];
            U2AssemblyRead& r  = rangeReads[j];
            r->id = addTable2Id(r->id, adapter->idExtra);
            reads[idx] = r;
        }
    }
}

void MultiTableAssemblyAdapter::removeReads(const QList<U2DataId>& readIds, U2OpStatus& os) {
    int nReads = readIds.size();
    int nRanges = tableAdapters.size();
    QVector < QList<U2DataId> > readIdsByRange(nRanges); //reads sorted by range
    for (int i = 0; i < nReads; i++) {
        const U2DataId& readId = readIds[i];
        int rangeNum = getReadRange(readId);
        readIdsByRange[rangeNum] << readId;
    }
    for (int i = 0; i < nRanges && !os.isCoR(); i++) {
        QList<U2DataId>& rangeReadIds = readIdsByRange[i];
        MTASingleTableAdapter* adapter = tableAdapters[i];
        adapter->singleTableAdapter->removeReads(rangeReadIds, os);
    }
}

void MultiTableAssemblyAdapter::pack(U2OpStatus& os) {
    MultiTablePackAlgorithmAdapter packAdapter(db, this);
    AssemblyPackAlgorithm::pack(packAdapter, os);
}

//////////////////////////////////////////////////////////////////////////
// pack adapter

MultiTablePackAlgorithmAdapter::MultiTablePackAlgorithmAdapter(DbRef* db, MultiTableAssemblyAdapter* ma) {
    multiTableAdapter = ma;
    foreach(MTASingleTableAdapter* a , multiTableAdapter->getTableAdapters()) {
        packAdapters  << new SingleTablePackAlgorithmAdapter(db, a->singleTableAdapter->getReadsTableName());
    }
}

U2DbiIterator<PackAlgorithmData>* MultiTablePackAlgorithmAdapter::selectAllReads(U2OpStatus& os) {
    QList< U2DbiIterator<PackAlgorithmData>* > iterators;
    foreach(SingleTablePackAlgorithmAdapter* a, packAdapters) {
        iterators << a->selectAllReads(os);
    }
    return new MTAPackAlgorithmDataIterator(iterators, multiTableAdapter->getIdExtrasPerRange());
}

MultiTablePackAlgorithmAdapter::~MultiTablePackAlgorithmAdapter() {
    qDeleteAll(packAdapters);
}

void MultiTablePackAlgorithmAdapter::assignProw(const U2DataId& readId, qint64 prow, U2OpStatus& os) {
    int range = multiTableAdapter->getReadRange(readId);
    packAdapters[range]->assignProw(readId, prow, os);
}

//////////////////////////////////////////////////////////////////////////
// MTAReadsIterator

MTAReadsIterator::MTAReadsIterator(QList< U2DbiIterator<U2AssemblyRead>* >& i, const QList<QByteArray>& ie)
: iterators (i), currentRange(0), idExtras(ie)
{
}

MTAReadsIterator::~MTAReadsIterator() {
    qDeleteAll(iterators);
}

bool MTAReadsIterator::hasNext() {
    bool res = currentRange < iterators.size();
    if (res) {
        do {
            U2DbiIterator<U2AssemblyRead>* it = iterators[currentRange];
            res = it->hasNext();
            if (res) {
                break;
            }
            currentRange++;
        }  while (currentRange < iterators.size());
    }
    return res;
}

U2AssemblyRead MTAReadsIterator::next() {
    U2AssemblyRead res;
    if (currentRange < iterators.size()) {
        do {
            U2DbiIterator<U2AssemblyRead>* it = iterators[currentRange];
            if (it->hasNext()) {
                res = it->next();
                const QByteArray& idExtra = idExtras.at(currentRange);;
                res->id = addTable2Id(res->id, idExtra);
                break;
            }
            currentRange++;
        } while (currentRange < iterators.size());
    }
    return res;
}

U2AssemblyRead MTAReadsIterator::peek() {
    U2AssemblyRead res;
    if (currentRange < iterators.size()) {
        do {
            U2DbiIterator<U2AssemblyRead>* it = iterators[currentRange];
            if (it->hasNext()) {
                res = it->peek();
                const QByteArray& idExtra = idExtras.at(currentRange);;
                res->id = addTable2Id(res->id, idExtra);
                break;
            }
            currentRange++;
        } while (currentRange < iterators.size());
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////
// MTAPackAlgorithmDataIterator

MTAPackAlgorithmDataIterator::MTAPackAlgorithmDataIterator(QList< U2DbiIterator<PackAlgorithmData>* >& i, const QList<QByteArray>& ie)
:  iterators (i), idExtras(ie)
{
    fetchNextData();
}

MTAPackAlgorithmDataIterator::~MTAPackAlgorithmDataIterator() {
    qDeleteAll(iterators);
}

bool MTAPackAlgorithmDataIterator::hasNext() {
    return !nextData.readId.isEmpty();
}

PackAlgorithmData MTAPackAlgorithmDataIterator::next() {
    PackAlgorithmData res = nextData;
    fetchNextData();
    return res;
}

PackAlgorithmData MTAPackAlgorithmDataIterator::peek() {
    return nextData;
}

void MTAPackAlgorithmDataIterator::fetchNextData() {
    PackAlgorithmData bestCandidate;
    int bestRange = 0;
    for (int i = 0; i < iterators.size(); i++) {
        U2DbiIterator<PackAlgorithmData>* it = iterators[i];
        if (!it->hasNext()) {
            continue;
        }
        PackAlgorithmData d = it->peek();
        if (bestCandidate.readId.isEmpty() || bestCandidate.leftmostPos > d.leftmostPos) {
            bestCandidate = d;
            bestRange = i;
        }
    }
    nextData = bestCandidate;
    if (!nextData.readId.isEmpty()) {
        iterators[bestRange]->next();
        const QByteArray& idExtra = idExtras.at(bestRange);
        nextData.readId = addTable2Id(nextData.readId, idExtra);
    }
}


} //namespace
