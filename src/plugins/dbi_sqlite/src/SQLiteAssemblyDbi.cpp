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

#include "SQLiteAssemblyDbi.h"
#include "SQLiteObjectDbi.h"
#include "assembly/SingleTableAssemblyAdapter.h"
#include "assembly/RTreeAssemblyAdapter.h"
#include "assembly/MultiTableAssemblyAdapter.h"

#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QVarLengthArray>

#include <U2Core/Timer.h>
#include <U2Core/AppContext.h>

#include <memory>

namespace U2 {

SQLiteAssemblyDbi::SQLiteAssemblyDbi(SQLiteDbi* dbi) : U2AssemblyDbi(dbi), SQLiteChildDBICommon(dbi) {
}

SQLiteAssemblyDbi::~SQLiteAssemblyDbi() {
    assert(adaptersById.isEmpty());
}

void SQLiteAssemblyDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }
    // assembly object
    // reference            - reference sequence id
    // imethod - indexing method - method used to handle read location
    // cmethod - method used to handle compression of reads data
    // idata - additional indexing method data
    // cdata - additional compression method data
    SQLiteQuery("CREATE TABLE Assembly (object INTEGER, reference INTEGER, imethod TEXT NOT NULL, cmethod TEXT NOT NULL, "
        "idata BLOB, cdata BLOB, " 
        " FOREIGN KEY(object) REFERENCES Object(id), "
        " FOREIGN KEY(reference) REFERENCES Sequence(object) )", db, os).execute();
}

void SQLiteAssemblyDbi::shutdown(U2OpStatus& os) {
    foreach(AssemblyAdapter* a, adaptersById.values()) {
        a->shutdown(os);
        delete a;
    }
    adaptersById.clear();
}

AssemblyAdapter* SQLiteAssemblyDbi::getAdapter(const U2DataId& assemblyId, U2OpStatus& os) {
    qint64 sqliteId = SQLiteUtils::toDbiId(assemblyId);
    AssemblyAdapter* res = adaptersById.value(sqliteId);
    if (res != NULL) {
        return res;
    }
    
    SQLiteQuery q("SELECT imethod, cmethod FROM Assembly WHERE object = ?1", db, os);
    q.bindDataId(1, assemblyId);
    if (!q.step()) {
        os.setError(SQLiteL10N::tr("There is no assembly object with the specified id."));
        return NULL;
    }
    QString indexMethod = q.getString(0);
    QByteArray idata = q.getBlob(2);
    assert(!indexMethod.isEmpty());
    //TODO    QString comp = q.getString(1);


    if (indexMethod == SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_SINGLE_TABLE) {
        res = new SingleTableAssemblyAdapter(dbi, assemblyId, 'S', "", NULL, db, os);
    } else if (indexMethod == SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_MULTITABLE_V1) {
        res = new MultiTableAssemblyAdapter(dbi, assemblyId, NULL, db, os);
    } else if (indexMethod == SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_RTREE) {
        res = new RTreeAssemblyAdapter(dbi, assemblyId, NULL, db, os);
    } else {
        os.setError(SQLiteL10N::tr("Unsupported reads storage type: %1").arg(indexMethod));
        return NULL;
    }
    adaptersById[sqliteId] = res;
    return res;
}


U2Assembly SQLiteAssemblyDbi::getAssemblyObject(const U2DataId& assemblyId, U2OpStatus& os) {
    U2Assembly res;
    SQLiteQuery q("SELECT Assembly.reference, Object.name, Object.version FROM Assembly, Object "
                " WHERE Object.id = ?1 AND Assembly.object = Object.id", db, os);
    q.bindDataId(1, assemblyId);
    if (q.step())  {
        res.id = assemblyId;
        res.dbiId = dbi->getDbiId();
        res.version = 0;
        res.referenceId = q.getDataId(0, U2Type::Assembly);
        res.visualName = q.getString(1);
        res.version = q.getInt64(2);
        q.ensureDone();
    } else if (!os.hasError()) {
        os.setError(SQLiteL10N::tr("Assembly object not found."));
    }
    return res;
}

qint64 SQLiteAssemblyDbi::countReads(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::countReadsAt -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::countReadsAt");
    AssemblyAdapter* a = getAdapter(assemblyId, os);
    if ( a == NULL ) {
        return -1;
    }
    return a->countReads(r, os);
}


U2DbiIterator<U2AssemblyRead>* SQLiteAssemblyDbi::getReads(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::getReadsAt -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::getReadsAt");
    AssemblyAdapter* a = getAdapter(assemblyId, os);
    if ( a != NULL ) {
        return a->getReads(r, os);
    }
    return NULL;
}

U2DbiIterator<U2AssemblyRead>* SQLiteAssemblyDbi::getReadsByRow(const U2DataId& assemblyId, const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::getReadsAt -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::getReadsAt");
    
    quint64 t0 = GTimer::currentTimeMicros();
    AssemblyAdapter* a = getAdapter(assemblyId, os);

    if ( a == NULL ) {
        return NULL;
    }

    U2DbiIterator<U2AssemblyRead>* res = a->getReadsByRow(r, minRow, maxRow, os);

    t2.stop();
    perfLog.trace(QString("Assembly: reads 2D select time: %1 seconds").arg((GTimer::currentTimeMicros() - t0) / float(1000*1000)));

    return res;
}

U2DbiIterator<U2AssemblyRead>* SQLiteAssemblyDbi::getReadsByName(const U2DataId& assemblyId, const QByteArray& name, U2OpStatus& os)  {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::getReadsByName -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::getReadsByName");
    AssemblyAdapter* a = getAdapter(assemblyId, os);
    if ( a != NULL ) {
        return a->getReadsByName(name, os);
    }
    return NULL;
}


qint64 SQLiteAssemblyDbi::getMaxPackedRow(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) {
    quint64 t0 = GTimer::currentTimeMicros();
    
    AssemblyAdapter* a = getAdapter(assemblyId, os);

    if ( a == NULL ) {
        return -1;
    }
    qint64 res = a->getMaxPackedRow(r, os);
    
    perfLog.trace(QString("Assembly: get max packed row: %1 seconds").arg((GTimer::currentTimeMicros() - t0) / (1000*1000)));
    return res;
}



qint64 SQLiteAssemblyDbi::getMaxEndPos(const U2DataId& assemblyId, U2OpStatus& os) {
    quint64 t0 = GTimer::currentTimeMicros();

    AssemblyAdapter* a = getAdapter(assemblyId, os);
    if ( a == NULL ) {
        return -1;
    }
    quint64 res = a->getMaxEndPos(os);
    
    perfLog.trace(QString("Assembly: get max end pos: %1 seconds").arg((GTimer::currentTimeMicros() - t0) / (1000*1000)));
    return res;
}



void SQLiteAssemblyDbi::createAssemblyObject(U2Assembly& assembly, const QString& folder,  
                                             U2DbiIterator<U2AssemblyRead>* it, 
                                             U2AssemblyReadsImportInfo& importInfo, 
                                             U2OpStatus& os) 
{
    assembly.id = SQLiteObjectDbi::createObject(U2Type::Assembly, folder, assembly.visualName,  SQLiteDbiObjectRank_TopLevel, db, os);
    SAFE_POINT_OP(os,);
    
    //QString elenMethod = dbi->getProperty(SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_KEY, SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_RTREE, os);
    QString elenMethod = dbi->getProperty(SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_KEY, SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_MULTITABLE_V1, os);
    //QString elenMethod = dbi->getProperty(SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_KEY, SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_SINGLE_TABLE, os);

    SQLiteQuery q("INSERT INTO Assembly(object, reference, imethod, cmethod) VALUES(?1, ?2, ?3, ?4)", db, os);
    q.bindDataId(1, assembly.id);
    q.bindDataId(2, assembly.referenceId);
    q.bindString(3, elenMethod);
    q.bindString(4, SQLITE_DBI_ASSEMBLY_READ_COMPRESSION_METHOD_NO_COMPRESSION);
    q.execute();
    SAFE_POINT_OP(os,);

    AssemblyAdapter* a = getAdapter(assembly.id, os);
    SAFE_POINT_OP(os,);

    a->createReadsTables(os);
    SAFE_POINT_OP(os,);
    
    if (it != NULL) {
        addReads(a, it, importInfo, os);
        SAFE_POINT_OP(os,);
    }

    a->createReadsIndexes(os);
    SAFE_POINT_OP(os,);
}

 
void SQLiteAssemblyDbi::updateAssemblyObject(U2Assembly& assembly, U2OpStatus& os) {
    SQLiteTransaction(db, os);
    
    SQLiteQuery q("UPDATE Assembly SET reference = ?1 WHERE object = ?2", db, os);
    q.bindDataId(1, assembly.referenceId);
    q.bindDataId(2, assembly.id);
    q.execute();

    SAFE_POINT_OP(os, );

    dbi->getSQLiteObjectDbi()->updateObject(assembly, os);
}

void SQLiteAssemblyDbi::removeReads(const U2DataId& assemblyId, const QList<U2DataId>& rowIds, U2OpStatus& os){
    AssemblyAdapter* a = getAdapter(assemblyId, os);
    if ( a != NULL ) {
        a->removeReads(rowIds, os);
    }
}

void SQLiteAssemblyDbi::addReads(AssemblyAdapter* a, U2DbiIterator<U2AssemblyRead>* it, U2AssemblyReadsImportInfo& ii, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::addReads");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::addReads");

    quint64 t0 = GTimer::currentTimeMicros();

    a->addReads(it, ii, os);

    t2.stop();
    perfLog.trace(QString("Assembly: %1 reads added in %2 seconds. Auto-packing: %3")
        .arg(ii.nReads).arg((GTimer::currentTimeMicros() - t0) / float(1000*1000)).arg(ii.packStat.readsCount > 0 ? "yes" : "no"));
}

void SQLiteAssemblyDbi::addReads(const U2DataId& assemblyId, U2DbiIterator<U2AssemblyRead>* it, U2OpStatus& os) {
    AssemblyAdapter* a = getAdapter(assemblyId, os);
    if ( a != NULL ) {
        U2AssemblyReadsImportInfo ii;
        addReads(a, it, ii, os);
    }
}


/**  Packs assembly rows: assigns packedViewRow value for every read in assembly */
void SQLiteAssemblyDbi::pack(const U2DataId& assemblyId, U2AssemblyPackStat& stat, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::pack");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::pack");

    quint64 t0 = GTimer::currentTimeMicros();

    AssemblyAdapter* a = getAdapter(assemblyId, os);
    if ( a == NULL ) {
        return;
    }
    stat.readsCount = a->countReads(U2_REGION_MAX, os);
    a->pack(stat, os);
    perfLog.trace(QString("Assembly: full pack time: %1 seconds").arg((GTimer::currentTimeMicros() - t0) / float(1000*1000)));
}

void SQLiteAssemblyDbi::calculateCoverage(const U2DataId& assemblyId, const U2Region& region, U2AssemblyCoverageStat& c, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::calculateCoverage");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::calculateCoverage");

    quint64 t0 = GTimer::currentTimeMicros();

    AssemblyAdapter* a = getAdapter(assemblyId, os);
    if ( a == NULL ) {
        return;
    }
    a->calculateCoverage(region, c, os);
    perfLog.trace(QString("Assembly: full coverage calculation time: %1 seconds").arg((GTimer::currentTimeMicros() - t0) / float(1000*1000)));
}

//////////////////////////////////////////////////////////////////////////
// AssemblyAdapter

AssemblyAdapter::AssemblyAdapter(const U2DataId& _assemblyId, const AssemblyCompressor* _compressor, DbRef* _db) 
:assemblyId(_assemblyId), compressor(_compressor), db(_db)
{
}

//////////////////////////////////////////////////////////////////////////
// SQLiteAssemblyUtils

QByteArray SQLiteAssemblyUtils::packData(SQLiteAssemblyDataMethod method, const QByteArray& name, const QByteArray& seq, const QByteArray& cigarText, 
                           const QByteArray& qualityString, U2OpStatus& os)
{
    assert(method == SQLiteAssemblyDataMethod_NSCQ);
    if (method != SQLiteAssemblyDataMethod_NSCQ) {
        os.setError(SQLiteL10N::tr("Packing method is not supported: %1").arg(method));
        return QByteArray();
    }
    int nBytes = 1 + name.length() + 1  + seq.length() + 1 + cigarText.length() + 1 + qualityString.length();
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
    QByteArray res(nBytes, Qt::Uninitialized);
#else
    QByteArray res(nBytes, char(0));
#endif
    char* data = res.data();
    int pos = 0;
    
    // packing type
    data[pos] = '0';
    pos++;

    // name
    qMemCopy(data + pos, name.constData(), name.length());
    pos+=name.length();
    data[pos] = '\n';
    pos++;

    // sequence
    qMemCopy(data + pos, seq.constData(), seq.length());
    pos+=seq.length();
    data[pos] = '\n';
    pos++;

    // cigar
    qMemCopy(data + pos, cigarText.constData(), cigarText.length());
    pos+=cigarText.length();
    data[pos] = '\n';
    pos++;

    // quality
    qMemCopy(data + pos, qualityString.constData(), qualityString.length());

//#define _SQLITE_CHECK_ASSEMBLY_DATA_PACKING_
#ifdef _SQLITE_CHECK_ASSEMBLY_DATA_PACKING_
    QByteArray n, s, c, q;
    unpackData(res, n, s, c, q, os);
    assert(n == name);
    assert(s == seq);
    assert(c == cigarText);
    assert(q == qualityString);
#endif
    return res;
}

void SQLiteAssemblyUtils::unpackData(const QByteArray& packedData, QByteArray& name, QByteArray& sequence, QByteArray& cigarText, QByteArray& qualityString, U2OpStatus& os) {
    if (packedData.isEmpty()) {
        os.setError(SQLiteL10N::tr("Packed data is empty!"));
        return;
    }
    const char* data = packedData.constData();
    if (data[0] != '0') {
        os.setError(SQLiteL10N::tr("Packing method prefix is not supported: %1").arg(data));
        return;
    }
    int nameStart = 1;
    int nameEnd = packedData.indexOf('\n', nameStart);
    if (nameEnd == -1) {
        os.setError(SQLiteL10N::tr("Data is corrupted, no name end marker found: %1").arg(data));
        return;
    }
    name.append(QByteArray(data + nameStart, nameEnd - nameStart));

    int sequenceStart = nameEnd + 1;
    int sequenceEnd = packedData.indexOf('\n', sequenceStart);
    if (sequenceEnd == -1) {
        os.setError(SQLiteL10N::tr("Data is corrupted, no sequence end marker found: %1").arg(data));
        return;
    }
    sequence.append(data + sequenceStart, sequenceEnd - sequenceStart);
    
    int cigarStart = sequenceEnd + 1;
    int cigarEnd = packedData.indexOf('\n', cigarStart);
    if (sequenceEnd == -1) {
        os.setError(SQLiteL10N::tr("Data is corrupted, no CIGAR end marker found: %1").arg(data));
        return;
    }
    cigarText.append(data + cigarStart, cigarEnd - cigarStart);

    int qualityStart = cigarEnd + 1;
    if (qualityStart < packedData.length()) {
        qualityString.append(data + qualityStart, packedData.length() - qualityStart);
    }
}

void SQLiteAssemblyUtils::calculateCoverage(SQLiteQuery& q, const U2Region& r, U2AssemblyCoverageStat& c, U2OpStatus& os) {
    int csize = c.coverage.size();
    SAFE_POINT(csize > 0, "illegal coverage vector size!", );

    U2Range<int>* cdata = c.coverage.data();
    double basesPerRange = double(r.length) / csize;
    while (q.step() && !os.isCoR()) {
        qint64 startPos = q.getInt64(0);
        qint64 len = q.getInt64(1);
        U2Region readRegion(startPos, len);
        U2Region readCroppedRegion = readRegion.intersect(r);

        if (readCroppedRegion.isEmpty()) {
            continue;
        }
        
        int firstCoverageIdx = (int)((readCroppedRegion.startPos - r.startPos)/ basesPerRange);
        int lastCoverageIdx = (int)((readCroppedRegion.startPos + readCroppedRegion.length - 1 - r.startPos ) / basesPerRange);
        for (int i = firstCoverageIdx; i <= lastCoverageIdx && i < csize; i++) {
            cdata[i].minValue++;
            cdata[i].maxValue++;
        }
    }
}

#define MAX_COVERAGE_VECTOR_SIZE (1000*1000)

static void remapCoverage(U2AssemblyReadsImportInfo& ii, int newSize, int newBasesPerPoint) {
    int oldSize = ii.coverage.coverage.size();
    SAFE_POINT(oldSize < newSize, "Invalid new coverage vector size!",);
    SAFE_POINT(ii.coverageBasesPerPoint <= newBasesPerPoint, "Invalid new coverage bases per point value!",);
    SAFE_POINT(newSize < MAX_COVERAGE_VECTOR_SIZE, "New coverage vector size is too large", );

    ii.coverage.coverage.resize(newSize);
    if (ii.coverageBasesPerPoint == newBasesPerPoint || oldSize == 0) {
        return;
    }
    //remap
    int nPointsToMerge = newBasesPerPoint / ii.coverageBasesPerPoint;
    U2Range<int>* data = ii.coverage.coverage.data();
    int oldSizeInNewVector = oldSize / nPointsToMerge;
    int oldI = 0;
    for (int i = 0; i < oldSizeInNewVector; i++) {
        U2Range<int> res = data[oldI];
        for (int oldEnd = oldI + nPointsToMerge; ++oldI < oldEnd;) {
            U2Range<int> oldVal = data[oldI];
            res = res + oldVal;
        }
        data[i] = res;
    }
    qFill(data + oldSizeInNewVector, data + oldSize, U2Range<int>());
}


void SQLiteAssemblyUtils::addToCoverage(U2AssemblyReadsImportInfo& ii, const U2AssemblyRead& read) {
    if (!ii.computeCoverage) {
        return;
    }
    int csize = ii.coverage.coverage.size();
    int basesPerPoint = qMax(1, ii.coverageBasesPerPoint);
    int endPos = (read->leftmostPos + read->effectiveLen) / basesPerPoint;
    if (endPos >= csize) {
        // resize and remap coverage info
        int newCSize = ii.maxEndPosHint > 0 ? ii.maxEndPosHint : endPos * 2;
        while (newCSize > MAX_COVERAGE_VECTOR_SIZE) {
            basesPerPoint*=2;
            if (newCSize/2 == MAX_COVERAGE_VECTOR_SIZE) {
                newCSize = MAX_COVERAGE_VECTOR_SIZE;
                break;
            }
            newCSize/=2;
        }
        remapCoverage(ii, newCSize, basesPerPoint);
        endPos = (read->leftmostPos + read->effectiveLen) / basesPerPoint;
        ii.coverageBasesPerPoint = basesPerPoint;
    }
    int startPos = read->leftmostPos / basesPerPoint;
    U2Range<int>* coverageData = ii.coverage.coverage.data();
    for (int i = startPos; i <= endPos; i++) {
        coverageData[i].minValue++;
        coverageData[i].maxValue++;
    }
}

//////////////////////////////////////////////////////////////////////////
// read loader
U2AssemblyRead SimpleAssemblyReadLoader::load(SQLiteQuery* q) {
    U2AssemblyRead read(new U2AssemblyReadData());

    read->id = q->getDataId(0, U2Type::AssemblyRead);
    read->packedViewRow = q->getInt64(1);
    if (q->hasError()) {
        return U2AssemblyRead();
    }
    read->leftmostPos= q->getInt64(2);
    read->effectiveLen = q->getInt64(3);
    read->flags = q->getInt64(4);
    read->mappingQuality = (quint8)q->getInt32(5);
    QByteArray data = q->getBlob(6);
    if (q->hasError()) {
        return U2AssemblyRead();
    }
    QByteArray cigarText;
    SQLiteAssemblyUtils::unpackData(data, read->name, read->readSequence, cigarText, read->quality, q->getOpStatus());
    if (q->hasError()) {
        return U2AssemblyRead();
    }
    QString err;
    read->cigar = U2AssemblyUtils::parseCigar(cigarText, err);
    if (!err.isEmpty()) {
        q->setError(err);
        return U2AssemblyRead();
    }
#ifdef _DEBUG
    //additional check to ensure that db stores correct info
    qint64 effectiveLengthFromCigar = read->readSequence.length() + U2AssemblyUtils::getCigarExtraLength(read->cigar);
    assert(effectiveLengthFromCigar == read->effectiveLen);
#endif
    return read;
}


PackAlgorithmData SimpleAssemblyReadPackedDataLoader::load(SQLiteQuery* q) {
    PackAlgorithmData data;
    data.readId = q->getDataId(0, U2Type::AssemblyRead);
    data.leftmostPos = q->getInt64(1);
    data.effectiveLen = q->getInt64(2);
    return data;
}


} //namespace
