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

#include <QtCore/QVarLengthArray>

#include <U2Core/Timer.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>

#include <memory>

namespace U2 {

SQLiteAssemblyDbi::SQLiteAssemblyDbi(SQLiteDbi* dbi) : U2AssemblyDbi(dbi), SQLiteChildDBICommon(dbi) {
}

AssemblyAdapter* SQLiteAssemblyDbi::getAdapter(const U2DataId& assemblyId, U2OpStatus& os) {
    SQLiteQuery q("SELECT elen_method, compression_method FROM Assembly WHERE object = ?1", db, os);
    q.bindDataId(1, assemblyId);
    if (!q.step()) {
        return NULL;
    }
    QString elen = q.getString(0);
    assert(!elen.isEmpty());
    //TODO    QString comp = q.getString(1);


    AssemblyAdapter* res = NULL;
    if (elen == SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_SINGLE_TABLE) {
        res = new SingleTableAssemblyAdapter(dbi, assemblyId, "", NULL, db, os);
    } else if (elen == SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_MULTITABLE_V1) {
        res = new MultiTableAssemblyAdapter(dbi, assemblyId, NULL, MultiTableAssemblyAdapterMode_4Tables, db, os);
    } else if (elen == SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_RTREE) {
        res = new RTreeAssemblyAdapter(dbi, assemblyId, NULL, db, os);
    } else {
        os.setError(SQLiteL10N::tr("Unsupported reads storage type: %1").arg(elen));
        return NULL;
    }
    return res;
}


U2Assembly SQLiteAssemblyDbi::getAssemblyObject(const U2DataId& assemblyId, U2OpStatus& os) {
    U2Assembly res(assemblyId, dbi->getDbiId(), 0);
    SQLiteQuery q("SELECT Assembly.reference, Object.name, Object.version FROM Assembly, Object "
                " WHERE Object.id = ?1 AND Assembly.object = Object.id", db, os);
    q.bindDataId(1, assemblyId);
    if (q.step())  {
        res.referenceId = q.getDataId(0, U2Type::Assembly);
        res.visualName = q.getString(1);
        res.version = q.getInt64(2);
        q.ensureDone();
    } 
    return res;
}

qint64 SQLiteAssemblyDbi::countReads(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::countReadsAt -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::countReadsAt");
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    return a->countReads(r, os);
}


U2DbiIterator<U2AssemblyRead>* SQLiteAssemblyDbi::getReads(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::getReadsAt -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::getReadsAt");
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    return a->getReads(r, os);
}

U2DbiIterator<U2AssemblyRead>* SQLiteAssemblyDbi::getReadsByRow(const U2DataId& assemblyId, const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::getReadsAt -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::getReadsAt");
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    return a->getReadsByRow(r, minRow, maxRow, os);
}

U2DbiIterator<U2AssemblyRead>* SQLiteAssemblyDbi::getReadsByName(const U2DataId& assemblyId, const QByteArray& name, U2OpStatus& os)  {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::getReadsByName -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::getReadsByName");
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    return a->getReadsByName(name, os);
}


qint64 SQLiteAssemblyDbi::getMaxPackedRow(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) {
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    return a->getMaxPackedRow(r, os);
}



qint64 SQLiteAssemblyDbi::getMaxEndPos(const U2DataId& assemblyId, U2OpStatus& os) {
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    return a->getMaxEndPos(os);
}



void SQLiteAssemblyDbi::createAssemblyObject(U2Assembly& assembly, const QString& folder,  U2DbiIterator<U2AssemblyRead>* it, U2OpStatus& os) {
    assembly.id = SQLiteObjectDbi::createObject(U2Type::Assembly, folder, assembly.visualName,  SQLiteDbiObjectRank_TopLevel, db, os);
    if (os.hasError()) {
        return;
    }
    
    QString elenMethod = dbi->getProperty(SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_KEY, SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_RTREE, os);
    //QString elenMethod = dbi->getProperty(SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_KEY, SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_MULTITABLE_V1, os);

    SQLiteQuery q("INSERT INTO Assembly(object, reference, elen_method, compression_method) VALUES(?1, ?2, ?3, ?4)", db, os);
    q.bindDataId(1, assembly.id);
    q.bindDataId(2, assembly.referenceId);
    q.bindString(3, elenMethod);
    q.bindString(4, SQLITE_DBI_ASSEMBLY_READ_COMPRESSION_METHOD_NO_COMPRESSION);
    q.execute();


    std::auto_ptr<AssemblyAdapter> a(getAdapter(assembly.id, os));
    if (os.hasError()) {
        return;
    }

    a->createReadsTables(os);
    if (os.hasError()) {
        return;
    }

    int insertGroupSize = a->getInsertGroupSize();
    if (it != NULL) {
        do {
            QList<U2AssemblyRead>  reads;
            for (int i = 0; i < insertGroupSize && it->hasNext() && !os.hasError(); i++) {
                reads.append(it->next());
            }
            if (!reads.isEmpty()) {
                a->addReads(reads, os);
            }
        } while (it->hasNext() && !os.hasError());
    }

    a->createReadsIndexes(os);
}
 
void SQLiteAssemblyDbi::updateAssemblyObject(U2Assembly& assembly, U2OpStatus& os) {
    SQLiteTransaction(db, os);
    
    SQLiteQuery q1("UPDATE Assembly SET reference = ?1 WHERE object = ?2", db, os);
    q1.bindDataId(1, assembly.referenceId);
    q1.bindDataId(2, assembly.id);
    q1.execute();

    SQLiteQuery q2("UPDATE Object SET version = version + 1 WHERE id = ?2", db, os);
    q2.bindDataId(1, assembly.id);
    q2.execute();
}

void SQLiteAssemblyDbi::removeReads(const U2DataId& assemblyId, const QList<U2DataId>& rowIds, U2OpStatus& os){
    SQLiteObjectDbi* objDbi = dbi->getSQLiteObjectDbi();
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    a->removeReads(rowIds, os);
}

void SQLiteAssemblyDbi::addReads(const U2DataId& assemblyId, QList<U2AssemblyRead>& rows, U2OpStatus& os) {
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    a->addReads(rows, os);
}


/**  Packs assembly rows: assigns packedViewRow value for every read in assembly */
void SQLiteAssemblyDbi::pack(const U2DataId& assemblyId, U2OpStatus& os) {
    std::auto_ptr<AssemblyAdapter> a(getAdapter(assemblyId, os));
    a->pack(os);
}

//////////////////////////////////////////////////////////////////////////
// AssemblyAdapter

AssemblyAdapter::AssemblyAdapter(const U2DataId& _assemblyId, const AssemblyCompressor* _compressor, DbRef* _db, U2OpStatus& _os) 
:assemblyId(_assemblyId), compressor(_compressor), db(_db), os(_os)
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
    int flags = q->getInt64(4);
    read->complementary = SQLiteAssemblyUtils::isComplementaryRead(flags);
    read->paired = SQLiteAssemblyUtils::isPairedRead(flags);
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
