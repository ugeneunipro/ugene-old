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

#define DEFAULT_ROWS_PER_TABLE 5000

MultiTableAssemblyAdapter::MultiTableAssemblyAdapter(SQLiteDbi* _dbi, const U2DataId& assemblyId, 
                                                       const AssemblyCompressor* compressor, 
                                                       DbRef* db, U2OpStatus& os)
                                                       : AssemblyAdapter(assemblyId, compressor, db)
{
    dbi = _dbi;
    version = -1;
    syncTables(os);
    rowsPerRange = DEFAULT_ROWS_PER_TABLE;
}

MultiTableAssemblyAdapter::~MultiTableAssemblyAdapter() {
    clearTableAdaptersInfo();
}

void MultiTableAssemblyAdapter::clearTableAdaptersInfo() {
    qDeleteAll(adapters);
    adaptersGrid.clear();
    idExtras.clear();
    elenRanges.clear();
}

void MultiTableAssemblyAdapter::syncTables(U2OpStatus& os) {
    qint64 versionInDb = dbi->getObjectDbi()->getObjectVersion(assemblyId, os);
    if (versionInDb <= version) {
        return;
    }
    SQLiteQuery q("SELECT idata FROM Assembly WHERE object = ?1", db, os);
    q.bindDataId(1, assemblyId);
    if (q.step()) {
        QByteArray data = q.getBlob(0);
        rereadTables(data, os);
        if (!os.hasError()) {
            version = versionInDb;        
        }
    }
}

static QVector<U2Region> toRange(const QVector<int>& startPos) {
    QVector<U2Region> res;
    int prev = 0;
    foreach(int pos, startPos){
        res << U2Region(prev, pos - prev);
        prev = pos;
    }
    return res;
}

void MultiTableAssemblyAdapter::initTables(QList<U2AssemblyRead>& reads, U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }
    assert(elenRanges.isEmpty());
    int nReads = reads.size();
    if (false && nReads > 1000) {
    /*    // get reads distribution first
        QVector<int> distribution(nReads / 10, 0);
        int* data = distribution.data();
        foreach(const U2AssemblyRead& read, reads) {
            int elen = read->readSequence.size() + U2AssemblyUtils::getCigarExtraLength(read->cigar);
            int idx = elen / 10;
            data[idx]++;
        }
        // derive regions
        // TODO:*/
    } else {
        QVector<int> starts;
        starts << 50 << 200 << 800 << 4*1000 << 25*1000 << 100*1000 << 500*1000 << 2*1000*1000;
        elenRanges << toRange( starts);
    }
    
    QByteArray idata;
    for(int i = 0; i < elenRanges.size(); i++) {
        int rangeStart = elenRanges[i].startPos;
        if (!idata.isEmpty()) {
            idata.append(',');
        }
        idata.append(QByteArray::number(rangeStart));
    }
    idata.append('|').append(QByteArray::number(rowsPerRange)).append(',').append(QByteArray::number(1));

    SQLiteQuery q("UPDATE Assembly SET idata = ?1 WHERE object = ?2", db, os);
    q.bindBlob(1, idata);
    q.bindDataId(2, assemblyId);
    q.execute();
    if (os.hasError()) {
        return;
    }
    initAdaptersGrid(1, elenRanges.size());
}

void MultiTableAssemblyAdapter::rereadTables(const QByteArray& idata, U2OpStatus& os) {
    QWriteLocker wl(&tablesSyncLock);
    
    clearTableAdaptersInfo();
    
    // format: N, N, N, N | N, N |.....
    // elements are separated by | sign. First element encodes ranges, second prow step and max prow, others are for future extension
    if (idata.isEmpty()) {
        //assembly is empty - no index data was created
        return;
    }
    QList<QByteArray> elements = idata.split('|');
    if (elements.size() < 2) {
        os.setError(SQLiteL10n::tr("Failed to detect assembly storage format: %1").arg(idata.constData()));
        return;
    }
    QByteArray elenData = elements.at(0);
    QByteArray prowData = elements.at(1);

    
    QList<QByteArray> elenTokens = elenData.split(',');
    U2Region prev(-1, 1);
    bool parseOk = true;
    foreach(const QByteArray& elenTok, elenTokens) {
        int start = elenTok.toInt(&parseOk);
        if (!parseOk || start < prev.endPos()) {
            os.setError(QString("Failed to parse range: %1, full: %2").arg(elenTok.constData()).arg(elenData.constData()));
            return;
        }
        U2Region region(prev.endPos(), start - prev.endPos());
        elenRanges << region;
        prev = region;
    }
    elenRanges << U2Region(prev.endPos(), INT_MAX);

    QList<QByteArray> prowTokens = prowData.split(',');
    int prange = prowTokens.at(0).toInt(&parseOk);
    if (prange < 1 || !parseOk) {
        os.setError(SQLiteL10n::tr("Failed to parse packed row range info %1").arg(idata.constData()));
        return;
    }
    if (prowTokens.size() != 2) {
        os.setError(SQLiteL10n::tr("Failed to parse packed row range info %1").arg(idata.constData()));     
        return;
    }
    int maxProw = prowTokens.at(1).toInt(&parseOk);
    if (maxProw < 0 || !parseOk) {
        os.setError(SQLiteL10n::tr("Failed to parse packed row range info %1").arg(idata.constData()));
        return;
    }

    // ok, all regions parsed, now create adapters
    int nRows = maxProw / prange + ((maxProw % prange) == 0 ? 0 : 1);
    int nElens = elenRanges.size();
    initAdaptersGrid(nRows, nElens);
    for (int rowPos = 0; rowPos < nRows; rowPos++) {
        for (int elenPos = 0; elenPos < nElens; elenPos++) {
            QString suffix = getTableSuffix(rowPos, elenPos);
            QString tableName = SingleTableAssemblyAdapter::getReadsTableName(assemblyId, 'M', suffix);
            MTASingleTableAdapter* ma = NULL;
            if (SQLiteUtils::isTableExists(tableName, db, os)) {
                createAdapter(rowPos, elenPos, os);                
            }
        }
    }
}

QString MultiTableAssemblyAdapter::getTableSuffix(int rowPos, int elenPos) {
    U2Region eRegion = elenRanges[elenPos];
    bool last = (elenPos + 1== elenRanges.size());
    return QString("%1_%2_%3").arg(eRegion.startPos).arg(last ? QString("U") : QString::number(eRegion.endPos())).arg(rowPos);
}

void MultiTableAssemblyAdapter::initAdaptersGrid(int nRows, int nElens) {
    assert(adaptersGrid.isEmpty());
    adaptersGrid.resize(nRows);
    for (int i = 0; i < nRows; i++) {
        adaptersGrid[i] = QVector<MTASingleTableAdapter*>(nElens, NULL);
    }
}

MTASingleTableAdapter* MultiTableAssemblyAdapter::createAdapter(int rowPos, int elenPos, U2OpStatus& os) {
    assert(adaptersGrid.at(rowPos).at(elenPos) == NULL);

    QString suffix = getTableSuffix(rowPos, elenPos);
    SingleTableAssemblyAdapter* sa = new SingleTableAssemblyAdapter(dbi, assemblyId, 'M', suffix, compressor, db, os);
    const U2Region& elenRange = elenRanges.at(elenPos);
    sa->enableRangeTableMode(elenRange.startPos, elenRange.endPos());
    QByteArray idExtra = getIdExtra(rowPos, elenPos);
    MTASingleTableAdapter * ma = new MTASingleTableAdapter(sa, rowPos, elenPos, idExtra);
    ma->singleTableAdapter->createReadsTables(os);
    adapters << ma;
    idExtras << idExtra;
    adaptersGrid[rowPos][elenPos] =  ma;
    return ma;
}

void MultiTableAssemblyAdapter::createReadsIndexes(U2OpStatus& os) {
    foreach(MTASingleTableAdapter* a, adapters) {
        a->singleTableAdapter->createReadsIndexes(os);
        if (os.hasError()) {
            break;
        }
    }
}

QByteArray MultiTableAssemblyAdapter::getIdExtra(int rowPos, int elenPos) {
    QByteArray res(4, 0);
    qint16* data = (qint16*)res.data();
    data[0] = (qint16)rowPos;
    data[1] = (qint16)elenPos;
    return res;
}


qint64 MultiTableAssemblyAdapter::countReads(const U2Region& r, U2OpStatus& os) {
    qint64 sum = 0;
    // use more sensitive algorithm for smaller regions 
    // and not-very sensitive for huge regions
    int nReadsToUseNotPreciseAlgorithms = 1000 / (r.length + 1);
    foreach(MTASingleTableAdapter* a, adapters) {
        int n = a->singleTableAdapter->countReads(r, os);
        if (n != 0 && n < nReadsToUseNotPreciseAlgorithms) {
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
    // process only hi row adapters
    for (int rowPos = adaptersGrid.size(); --rowPos>=0 && max == 0;) {
        QVector<MTASingleTableAdapter*> elenAdapters = adaptersGrid.at(rowPos);
        for (int elenPos = 0, nRanges = elenAdapters.size(); elenPos < nRanges; elenPos++) {
            MTASingleTableAdapter* a = elenAdapters.at(elenPos);
            if (a == NULL) {
                continue;
            }
            qint64 n = a->singleTableAdapter->getMaxPackedRow(r, os);
            max = qMax(max, n);
        }
    }
    return max;
}

qint64 MultiTableAssemblyAdapter::getMaxEndPos(U2OpStatus& os) {
    //TODO: optimize by using gstart + maxReadLen for first n-1 tables
    qint64 max = 0;
    foreach(MTASingleTableAdapter* a, adapters) {
        qint64 n = a->singleTableAdapter->getMaxEndPos(os);
        if (os.hasError()) {
            break;
        }
        max = qMax(max, n);
    }
    return max;
}

U2DbiIterator<U2AssemblyRead>* MultiTableAssemblyAdapter::getReads(const U2Region& r, U2OpStatus& os) {
    QVector< U2DbiIterator<U2AssemblyRead>* > iterators;
    foreach(MTASingleTableAdapter* a, adapters) {
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
    QVector< U2DbiIterator<U2AssemblyRead>* > iterators;
    U2Region targetRowRange(minRow, maxRow - minRow);
    foreach(MTASingleTableAdapter* a, adapters) {
        const U2Region rowRegion(a->rowPos * rowsPerRange, rowsPerRange);
        if (!rowRegion.intersects(targetRowRange)) {
            continue;
        }
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
    QVector< U2DbiIterator<U2AssemblyRead>* > iterators;
    foreach(MTASingleTableAdapter* a, adapters) {
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

int MultiTableAssemblyAdapter::getElenRangePosById(const U2DataId& id) const {
    QByteArray extra = SQLiteUtils::toDbExtra(id);
    assert(extra.size() == 4);
    if (extra.size() != 4) {
        return -1;
    } 
    const qint16* data = (const qint16*)extra.constData();
    return int(data[1]);
}

int MultiTableAssemblyAdapter::getElenRangePosByLength(qint64 readLength) const {
    for (int i = 0, nRanges = elenRanges.size(); i < nRanges; i++) {
        const U2Region& r = elenRanges[i];
        if (r.contains(readLength)) {
            return i;
        }
    }
    assert(0);
    return elenRanges.size() - 1;
}

int MultiTableAssemblyAdapter::getRowRangePosByRow(quint64 row) const {
    return row / rowsPerRange;
}

int MultiTableAssemblyAdapter::getRowRangePosById(const U2DataId& id) const {
    QByteArray extra = SQLiteUtils::toDbExtra(id);
    assert(extra.size() == 4);
    if (extra.size() != 4) {
        return -1;
    } 
    const qint16* data = (const qint16*)extra.constData();
    return int(data[0]);
}

static U2DataId addTable2Id(const U2DataId& id, const QByteArray& idExtra) {
    assert(SQLiteUtils::toDbExtra(id).isEmpty());
    U2DataId res = SQLiteUtils::toU2DataId(SQLiteUtils::toDbiId(id), U2Type::AssemblyRead, idExtra); 
    return res;
}

void MultiTableAssemblyAdapter::addReads(QList<U2AssemblyRead>& reads, U2OpStatus& os) {
    addReadsInternal(reads, false, os);
}

void MultiTableAssemblyAdapter::addReadsInternal(QList<U2AssemblyRead>& reads, bool delayedIndex, U2OpStatus& os) {
    if (adaptersGrid.isEmpty()) {
        initTables(reads, os);
        if (os.hasError()) {
            return;
        }
    }
    int nReads = reads.size();
    int nElens = elenRanges.size();
    
    QVector < QList<U2AssemblyRead> > readsByRange(nElens); //reads sorted by range
    QVector< QVector<int> > readsIndex(nElens); // back-mapping of read index to original 'reads' list
    for (int i = 0; i < nReads; i++) {
        U2AssemblyRead& read = reads[i];
        int readLen = read->readSequence.length();
        read->effectiveLen = readLen + U2AssemblyUtils::getCigarExtraLength(read->cigar);
        int elenPos = getElenRangePosByLength(read->effectiveLen);
        read->packedViewRow = 0;
        readsByRange[elenPos] << read;
        readsIndex[elenPos] << i;
        
    }
    for (int i = 0; i < nElens; i++) {
        QList<U2AssemblyRead>& rangeReads = readsByRange[i];
        if (rangeReads.isEmpty()) {
            continue;
        }
        MTASingleTableAdapter* adapter = getAdapterByRowAndElenRange(0, i, true, os);
        adapter->singleTableAdapter->addReads(rangeReads, os);
        if (os.hasError()) {
            break;
        }
        if (!delayedIndex) {
            //create index tables if needed
            adapter->singleTableAdapter->createReadsIndexes(os);
        }


        //now back-map all reads to initial list
        const QVector<int>& idxMap = readsIndex[i];
        for (int j = 0, n = rangeReads.size(); j < n; j++) {
            int idx = idxMap[j];
            U2AssemblyRead& r  = rangeReads[j];
            r->id = addTable2Id(r->id, adapter->idExtra);
            reads[idx] = r;
        }
    }
}

MTASingleTableAdapter* MultiTableAssemblyAdapter::getAdapterByRowAndElenRange(int rowPos, int elenPos, bool createIfNotExits, U2OpStatus& os) {
    int nElens = elenRanges.size();
    assert(elenPos < nElens);
    if (rowPos >= adaptersGrid.size()) {
        assert(createIfNotExits);
        if (!createIfNotExits) {
            return NULL;
        }
        int oldRowSize = adaptersGrid.size();
        int newRowSize = rowPos + 1;
        adaptersGrid.resize(newRowSize);
        for (int i = oldRowSize; i < newRowSize; i++) {
            adaptersGrid[i].resize(nElens);
        }
    }
    QVector<MTASingleTableAdapter*> elenAdapters = adaptersGrid.at(rowPos);
    assert(elenAdapters.size() == nElens);
    MTASingleTableAdapter* adapter = elenAdapters.at(elenPos);
    if (adapter == NULL && createIfNotExits) {
        adapter = createAdapter(rowPos, elenPos, os);
    }
    return adapter;
}

void MultiTableAssemblyAdapter::removeReads(const QList<U2DataId>& readIds, U2OpStatus& os) {
    int nReads = readIds.size();
    
    QHash<MTASingleTableAdapter*, QList<U2DataId> > readsByAdapter;
    for (int i = 0; i < nReads; i++) {
        const U2DataId& readId = readIds[i];
        int rowRange = getRowRangePosById(readId);
        int elenPos = getElenRangePosById(readId);
        MTASingleTableAdapter* a = getAdapterByRowAndElenRange(rowRange, elenPos, false, os);
        assert(a !=NULL);
        if (!readsByAdapter.contains(a)) {
            readsByAdapter[a] = QList<U2DataId>();
        }
        readsByAdapter[a].append(readId);
    }
    foreach(MTASingleTableAdapter* a, readsByAdapter.keys()) {
        QList<U2DataId>& rangeReadIds = readsByAdapter[a];
        a->singleTableAdapter->removeReads(rangeReadIds, os);
        //TODO: remove adapters for empty tables. And tables as well
    }
}

void MultiTableAssemblyAdapter::pack(U2OpStatus& os) {
    MultiTablePackAlgorithmAdapter packAdapter(this);
    AssemblyPackAlgorithm::pack(packAdapter, os);
}

//////////////////////////////////////////////////////////////////////////
// pack adapter

MultiTablePackAlgorithmAdapter::MultiTablePackAlgorithmAdapter(MultiTableAssemblyAdapter* ma) {
    multiTableAdapter = ma;
    DbRef* db = multiTableAdapter->getDbRef();
    int nElens = multiTableAdapter->getNumberOfElenRanges();
    ensureGridSize(nElens);
    foreach(MTASingleTableAdapter* a , multiTableAdapter->getAdapters()) {
        SingleTablePackAlgorithmAdapter* sa = new SingleTablePackAlgorithmAdapter(db, a->singleTableAdapter->getReadsTableName());
        packAdapters << sa;
        if (packAdaptersGrid.size() <= a->rowPos) {
            packAdaptersGrid.resize(a->rowPos + 1);
        }
        packAdaptersGrid[a->rowPos][a->elenPos] = sa;
    }
}

U2DbiIterator<PackAlgorithmData>* MultiTablePackAlgorithmAdapter::selectAllReads(U2OpStatus& os) {
    QVector< U2DbiIterator<PackAlgorithmData>* > iterators;
    foreach(SingleTablePackAlgorithmAdapter* a, packAdapters) {
        iterators << a->selectAllReads(os);
    }
    return new MTAPackAlgorithmDataIterator(iterators, multiTableAdapter->getIdExtrasPerRange());
}

MultiTablePackAlgorithmAdapter::~MultiTablePackAlgorithmAdapter() {
    qDeleteAll(packAdapters);
}

void MultiTablePackAlgorithmAdapter::assignProw(const U2DataId& readId, qint64 prow, U2OpStatus& os) {
    int elenPos = multiTableAdapter->getElenRangePosById(readId);
    int oldRowPos = multiTableAdapter->getRowRangePosById(readId);
    int newRowPos = multiTableAdapter->getRowRangePosByRow(prow);
    
    SingleTablePackAlgorithmAdapter* sa = NULL;
    if (newRowPos == oldRowPos) {
        sa = packAdaptersGrid[oldRowPos][elenPos];
    } else {
        ensureGridSize(newRowPos + 1);
        sa = packAdaptersGrid[newRowPos][elenPos];
        if (sa == NULL) {
            MTASingleTableAdapter* a = multiTableAdapter->getAdapterByRowAndElenRange(newRowPos, elenPos, true, os);
            sa = new SingleTablePackAlgorithmAdapter(multiTableAdapter->getDbRef(), a->singleTableAdapter->getReadsTableName());
            packAdapters << sa;
            packAdaptersGrid[newRowPos][elenPos] = sa;
        }
    }
    sa->assignProw(readId, prow, os);
}


void MultiTablePackAlgorithmAdapter::ensureGridSize(int nRows) {
    int oldNRows = packAdaptersGrid.size();
    if (oldNRows < nRows) {
        int nElens = multiTableAdapter->getNumberOfElenRanges();
        packAdaptersGrid.resize(nRows);
        for (int i = oldNRows; i < nRows; i++) {
            packAdaptersGrid[i].resize(nElens);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// MTAReadsIterator

MTAReadsIterator::MTAReadsIterator(QVector< U2DbiIterator<U2AssemblyRead>* >& i, const QVector<QByteArray>& ie)
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

MTAPackAlgorithmDataIterator::MTAPackAlgorithmDataIterator(QVector< U2DbiIterator<PackAlgorithmData>* >& i, const QVector<QByteArray>& ie)
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
