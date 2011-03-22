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

#include <U2Core/U2Bits.h>

#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2SqlHelpers.h>

#include <QtCore/QVarLengthArray>

#include <U2Core/Timer.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>

namespace U2 {

SQLiteAssemblyDbi::SQLiteAssemblyDbi(SQLiteDbi* dbi) : U2AssemblyDbi(dbi), SQLiteChildDBICommon(dbi) {
    dnaAlpha = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT())->getAlphabetChars();
    dnaExtAlpha = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_EXTENDED())->getAlphabetChars();
    //dnaAlpha.remove('-'); //this symbol will be removed from alpha chars in 2.0
    //dnaExtAlpha.remove('-');
    cigarAlpha = U2AssemblyUtils::getCigarAlphabetChars();
    dnaAlphaNums = U2BitCompression::prepareCharNumsMask(dnaAlpha);
    dnaExtAlphaNums = U2BitCompression::prepareCharNumsMask(dnaExtAlpha);
    cigarAlphaNums = U2BitCompression::prepareCharNumsMask(cigarAlpha);
}


/** This bit indicates that DNA Ext alphabet is used*/
#define BIT_EXT_DNA_ALPHABET        0

/** This bit indicates that read is located on complement thread */
#define BIT_COMPLEMENTARY_STRAND    1


static bool isExtendedAlphabet(qint64 flags) {
    return flags & BIT_EXT_DNA_ALPHABET;
}

static bool isComplementaryRead(qint64 flags) {
    return flags & BIT_COMPLEMENTARY_STRAND;
}


void SQLiteAssemblyDbi::createReadsTable(const U2DataId& id, U2OpStatus& os) {
    // id -> read id
    // sequence -> sequence object id (if read is represented as a sequence)
    // prow -> packed row position
    // cigar -> cigar text
    // data -> sequence
    // gstart -> global start position of the read in assembly
    // elen -> effective read len = read sequence len + CIGAR adjustment
    // flags -> read flags

    QString tableName = getReadsTableName(id);
    QString q;
    if (dbi->isAssemblyReadsCompressionEnabled()) {
        q = "CREATE TABLE %1 (id INTEGER PRIMARY KEY AUTOINCREMENT, sequence INTEGER NOT NULL, prow INTEGER NOT NULL, "
            "data BLOB NOT NULL, gstart INTEGER NOT NULL, elen INTEGER NOT NULL, flags INTEGER NOT NULL)";
    } else {
        q = "CREATE TABLE %1 (id INTEGER PRIMARY KEY AUTOINCREMENT, sequence INTEGER NOT NULL, prow INTEGER NOT NULL, "
            "data BLOB NOT NULL, gstart INTEGER NOT NULL, elen INTEGER NOT NULL, flags INTEGER NOT NULL, cigar TEXT NOT NULL)";
    }
    SQLiteQuery(q.arg(tableName), db, os).execute();
}

void SQLiteAssemblyDbi::createReadsIndexes(const U2DataId& id, U2OpStatus& os) {
    QString tableName = getReadsTableName(id);

    SQLiteQuery(QString("CREATE INDEX %1_gstart ON %1(gstart)").arg(tableName), db, os).execute();

    // TODO: decide if we need this indices (slows down insertions)
    //SQLiteQuery(QString("CREATE INDEX %1_elen ON %1(elen)").arg(tableName), db, os).execute();
    //SQLiteQuery(QString("CREATE INDEX %1_prow ON %1(prow)").arg(tableName), db, os).execute();
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

QString SQLiteAssemblyDbi::getReadsTableName(const U2DataId& id) {
    int dbId = SQLiteUtils::toDbiId(id);
    QString result = QString("AssemblyRead_%1").arg(dbId);
    return result;
}

// TODO: implement efficient tree-like structure that will keep assembly read length mapping per range
#define MAX_READ_LENGTH 500

qint64 SQLiteAssemblyDbi::getMaximumReadLengthInRegion(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) {
    // in v1.0 lextra keep max read length per whole assembly
    SQLiteQuery q("SELECT lextra FROM Assembly WHERE object = ?1", db, os);
    q.bindDataId(1, assemblyId);
    return q.selectInt64(MAX_READ_LENGTH);
}

void SQLiteAssemblyDbi::setMaximumReadLengthInRegion(const U2DataId& assemblyId, const U2Region& r, int val, U2OpStatus& os) {
    // in v1.0 lextra keep max read length per whole assembly
    SQLiteQuery q("UPDATE Assembly SET lextra = ?1 WHERE object = ?2", db, os);
    q.bindInt32(1, val);
    q.bindDataId(2, assemblyId);
    q.execute();
}

qint64 SQLiteAssemblyDbi::countReadsAt(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::countReadsAt -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::countReadsAt");
    QString readsTable = getReadsTableName(assemblyId);
    qint64 maxReadLen = getMaximumReadLengthInRegion(assemblyId, r, os);
    SQLiteQuery q(QString("SELECT COUNT(*) FROM %1 WHERE gstart >= ?1 AND gstart < ?2").arg(readsTable), db, os);
    q.bindInt64(1, r.startPos - maxReadLen);
    q.bindInt64(2, r.endPos());
    return q.selectInt64();
}

QList<U2DataId> SQLiteAssemblyDbi::getReadIdsAt(const U2DataId& assemblyId, const U2Region& r, qint64 offset, qint64 count, U2OpStatus& os) {
    QString readsTable = getReadsTableName(assemblyId);
    qint64 maxReadLen = getMaximumReadLengthInRegion(assemblyId, r, os);
    SQLiteQuery q(QString("SELECT id FROM %1 WHERE gstart >= ?1 AND gstart < ?2").arg(readsTable), db, os);
    q.bindInt64(1, r.startPos - maxReadLen);
    q.bindInt64(2, r.endPos());
    return q.selectDataIds(U2Type::AssemblyRead);
}

void SQLiteAssemblyDbi::unpackSequenceAndCigar(qint64 flags, const QByteArray& data, QByteArray& sequence, QByteArray& cigar, U2OpStatus& os) {
    // data content in bits: [packed sequence][gap to align to byte boundary][packed cigar]
    bool extAlpha = isExtendedAlphabet(flags);
    const char* packedData = data.constData();
    int compressedCigarBytes = U2Bits::readInt16((const uchar*)packedData, 0);
    QString err;
    cigar = U2BitCompression::uncompress(packedData + 16, cigarAlpha, os);
    if (os.hasError()) {
        return;
    }
    sequence = U2BitCompression::uncompress(packedData + 16 + compressedCigarBytes, extAlpha ? dnaExtAlpha : dnaAlpha,  os);
}

void SQLiteAssemblyDbi::readRow(U2AssemblyRead& row, SQLiteQuery& q, U2OpStatus& os) {
    bool compressedMode = dbi->isAssemblyReadsCompressionEnabled();

    row->id = q.getDataId(0, U2Type::AssemblyRead);
    row->sequenceId = q.getDataId(1, U2Type::Sequence);
    row->packedViewRow = q.getInt64(2);
    if (q.hasError()) {
        return;
    }
    int flags = q.getInt64(3);
    row->leftmostPos= q.getInt64(4);
    QByteArray data = q.getBlob(5);
    row->complementary = isComplementaryRead(flags);
    QByteArray cigar;
    if (compressedMode) {
        unpackSequenceAndCigar(flags, data, row->readSequence, cigar, os);
    } else {
        row->readSequence = data;
        cigar = q.getCString(6);
    }
    QString err;
    row->cigar = U2AssemblyUtils::parseCigar(cigar, err);;
    if (!err.isEmpty()) {
        q.setError(err);
    }
}

QList<U2AssemblyRead> SQLiteAssemblyDbi::readRows(SQLiteQuery& q, U2OpStatus& os) {
    QList<U2AssemblyRead> res;
    while (q.step())  {
        U2AssemblyRead row(new U2AssemblyReadData());
        readRow(row, q, os);
        if (q.hasError()) {
            break;
        }
        res.append(row);
    } 
    return res;
}

QString SQLiteAssemblyDbi::getReadFields() const {
    if (dbi->isAssemblyReadsCompressionEnabled()) {
        return "id, sequence, prow, flags, gstart, data";
    } else {
        return "id, sequence, prow, flags, gstart, data, cigar";
    }
}

QList<U2AssemblyRead> SQLiteAssemblyDbi::getReadsAt(const U2DataId& assemblyId, const U2Region& r, qint64 offset, qint64 count, U2OpStatus& os) {
    GCOUNTER(c1, t1, "SQLiteAssemblyDbi::getReadsAt -> calls");
    GTIMER(c2, t2, "SQLiteAssemblyDbi::getReadsAt");
    QString readsTable = getReadsTableName(assemblyId);
    qint64 maxReadLen = getMaximumReadLengthInRegion(assemblyId, r, os);
    SQLiteQuery q(QString("SELECT " + getReadFields() + " FROM %1 WHERE gstart >= ?1 AND gstart < ?2").arg(readsTable), offset, count, db, os);
    q.bindInt64(1, r.startPos - maxReadLen);
    q.bindInt64(2, r.endPos());
    return readRows(q, os);
}

qint64 SQLiteAssemblyDbi::getMaxPackedRow(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) {
    QString readsTable = getReadsTableName(assemblyId);
    qint64 maxReadLen = getMaximumReadLengthInRegion(assemblyId, r, os);
    SQLiteQuery q(QString("SELECT MAX(prow) FROM %1 WHERE gstart >= ?1 AND gstart < ?2").arg(readsTable), db, os);
    q.bindInt64(1, r.startPos - maxReadLen);
    q.bindInt64(2, r.endPos());
    return q.selectInt64();
}

QList<U2AssemblyRead> SQLiteAssemblyDbi::getReadsByRow(const U2DataId& assemblyId, const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os) {
    QString readsTable = getReadsTableName(assemblyId);
    qint64 maxReadLen = getMaximumReadLengthInRegion(assemblyId, r, os);
    SQLiteQuery q(QString("SELECT " + getReadFields() + " FROM %1 WHERE "
                            " gstart >= ?1 AND gstart < ?2 AND prow >= ?3 AND prow <= ?4").arg(readsTable), db, os);
    q.bindInt64(1, r.startPos - maxReadLen);
    q.bindInt64(2, r.endPos());
    q.bindInt64(3, minRow);
    q.bindInt64(4, maxRow);
    
    return readRows(q, os);
}

    
quint64 SQLiteAssemblyDbi::getMaxEndPos(const U2DataId& assemblyId, U2OpStatus& os) {
    //TODO: cache value in assembly structure?
    return SQLiteQuery(QString("SELECT MAX(gstart + elen) FROM %1").arg(getReadsTableName(assemblyId)), db, os).selectInt64();
}


#define INSERT_CHUNK_SIZE (100*1000)

void SQLiteAssemblyDbi::createAssemblyObject(U2Assembly& assembly, const QString& folder, U2AssemblyReadsIterator* it, U2OpStatus& os) {
    assembly.id = SQLiteObjectDbi::createObject(U2Type::Assembly, folder, assembly.visualName, db, os);
    if (os.hasError()) {
        return;
    }
    
    createReadsTable(assembly.id, os);
    if (os.hasError()) {
        return;
    }

    QString readsTable = getReadsTableName(assembly.id);
    SQLiteQuery q("INSERT INTO Assembly(object, reference, lextra) VALUES(?1, ?2, ?3)", db, os);
    q.bindDataId(1, assembly.id);
    q.bindDataId(2, assembly.referenceId);
    q.bindInt64(3, 0);
    q.execute();

    if (it != NULL) {
        do {
            QList<U2AssemblyRead>  reads;
            for (int i = 0; i < INSERT_CHUNK_SIZE && it->hasNext() && !os.hasError(); i++) {
                reads.append(it->next(os));
            }
            if (!reads.isEmpty()) {
                addReads(assembly.id, reads, os);
            }
        } while (it->hasNext() && !os.hasError());
    }

    createReadsIndexes(assembly.id, os);
}
    
void SQLiteAssemblyDbi::removeReads(const U2DataId& assemblyId, const QList<U2DataId>& rowIds, U2OpStatus& os){
    SQLiteObjectDbi* objDbi = dbi->getSQLiteObjectDbi();
    QString readsTable = getReadsTableName(assemblyId);
    SQLiteQuery selectSequenceQuery(QString("SELECT sequence FROM %1 WHERE id = ?1").arg(readsTable), db, os);
    foreach(U2DataId rowId, rowIds) {
        selectSequenceQuery.reset();
        selectSequenceQuery.bindDataId(1, rowId);
        U2DataId sequenceId = selectSequenceQuery.selectDataId(U2Type::Sequence);
        if (os.hasError()) {
            break;
        }
        SQLiteUtils::remove(readsTable, "id", rowId, 1, db, os);
        if (os.hasError()) {
            break;
        }
        if (!sequenceId.isEmpty()) { 
            objDbi->removeParent(assemblyId, sequenceId, true, os);
            if (os.hasError()) {
                break;
            }
        } 
    }
    SQLiteObjectDbi::incrementVersion(assemblyId, db, os);
}

void SQLiteAssemblyDbi::addReads(const U2DataId& assemblyId, QList<U2AssemblyRead>& rows, U2OpStatus& os) {
    int maxReadLength = getMaximumReadLengthInRegion(assemblyId, U2Region(), os);
    int oldMaxReadLength = maxReadLength;
    if (os.hasError()) {
        return;
    }
    QByteArray tableName = "AssemblyRead_X";
    GTIMER(c1, t1, "SQLiteAssemblyDbi::addReads");
    SQLiteTransaction t(db, os);
    QString readsTable = getReadsTableName(assemblyId);
    bool compressMode = dbi->isAssemblyReadsCompressionEnabled();
    QString q = compressMode ? "INSERT INTO %1(sequence, prow, flags, gstart, elen, data) VALUES (?1, ?2, ?3, ?4, ?5, ?6)"
        : "INSERT INTO %1(sequence, prow, flags, gstart, elen, data, cigar) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7)";
    
    SQLiteQuery insertQ(q.arg(readsTable), db, os);

    QByteArray data;
    for (int i = 0, n = rows.size(); i < n && !os.isCoR(); i++) {
        U2AssemblyRead& row = rows[i];
        int rowLen = 0;
        bool dnaExt = false;
        data.clear();
        
        QByteArray cigarText = U2AssemblyUtils::cigar2String(row->cigar);
        
        int flags = 0;
        flags = flags | (row->complementary ? (1 << BIT_COMPLEMENTARY_STRAND) : 0);
        flags = flags | (dnaExt ? (1 << BIT_EXT_DNA_ALPHABET) : 0 );
        
        if (!row->sequenceId.isEmpty()) {
            U2Sequence rowSeq = getRootDbi()->getSequenceDbi()->getSequenceObject(row->sequenceId, os);
            if (os.hasError()) {
                break;
            }
            rowLen = rowSeq.length;
            dbi->getSQLiteObjectDbi()->ensureParent(assemblyId, row->sequenceId, os);
            //TODO: dnaExt = rowSeq.alphabet ...
        } else {
            rowLen = row->readSequence.length();
            dnaExt = false;//TODO: isExtAlphabet(row.readSequence);
        }
        int effectiveRowLength = rowLen + U2AssemblyUtils::getCigarExtraLength(row->cigar);

        if (compressMode) {
            QByteArray compressedCigar = U2BitCompression::compress(cigarText.constData(), cigarText.size(), 
                                                                    cigarAlpha.size(), cigarAlphaNums.constData(), os);
            if (compressedCigar.size() >= (1<<16) ) { // supported cigar size is up to 65k
                os.setError(SQLiteL10n::tr("CIGAR text length is too long!"));
                break;
            }
            QByteArray compressedReadSequence = U2BitCompression::compress(row->readSequence.constData(), row->readSequence.size(), 
                            dnaExt ? dnaExtAlpha.size() : dnaAlpha.size(), dnaExt ? dnaExtAlphaNums.constData() : dnaAlphaNums.constData(), os); 
            data.resize(2 + compressedCigar.size() + compressedReadSequence.size());
            U2Bits::writeInt16((uchar*)data.data(), 0, (qint16)compressedCigar.size());
            qMemCopy(data.data() + 2, compressedCigar.constData(), compressedCigar.size());
            qMemCopy(data.data() + 2 + compressedCigar.size(), compressedReadSequence.constData(), compressedReadSequence.size());
        } else {
            data = row->readSequence;
        }
        
        
        insertQ.reset();
        insertQ.bindDataId(1, row->sequenceId);
        insertQ.bindInt64(2, row->packedViewRow);
        insertQ.bindInt64(3, flags);
        insertQ.bindInt64(4, row->leftmostPos);
        insertQ.bindInt64(5, effectiveRowLength);;
        insertQ.bindBlob(6, data, false);
        
        if (!compressMode) {
            insertQ.bindText(7, cigarText);
        }

        row->id = insertQ.insert(U2Type::AssemblyRead, tableName);
        //insertQ.execute();
        maxReadLength = qMax(maxReadLength, effectiveRowLength);
    }
    
    if (maxReadLength > oldMaxReadLength && !os.hasError()) {
        setMaximumReadLengthInRegion(assemblyId, U2Region(), maxReadLength, os);
    }
}

#define TAIL_SIZE 10000

static qint64 selectProw(qint64* tails, qint64 start, qint64 end ){
    for (int i = 0; i < TAIL_SIZE; i++) {
        if (tails[i] < start) {
            tails[i] = end;
            return i;
        }
    }
    return -1;
}


/**  Packs assembly rows: assigns packedViewRow value for every read in assembly */
void SQLiteAssemblyDbi::pack(const U2DataId& assemblyId, U2OpStatus& os) {
    //Algorithm idea: 
    //  select * reads ordered by start position
    //  keep tack (tail) of used rows to assign packed row for reads (N elements)
    //  if all elements are used -> assign -1 to read and postprocess it later

    GTIMER(c1, t1, "SQLiteAssemblyDbi::pack");
    QVarLengthArray<qint64, TAIL_SIZE> tails;
    tails.resize(0);
    qFill(tails.data(), tails.data() + TAIL_SIZE, -1);

    QString readsTable = getReadsTableName(assemblyId);
    SQLiteQuery updateReadQuery = SQLiteQuery(QString("UPDATE %1 SET prow = ?1 WHERE id = ?2").arg(readsTable), db, os);
    SQLiteQuery selectAllReadsQuery = SQLiteQuery(QString("SELECT id, gstart, elen FROM %1 ORDER BY gstart").arg(readsTable), db, os);
    //selectAllReadsQuery.bindDataId(1, assemblyId);
    while (selectAllReadsQuery.step() && !os.hasError()) {
        U2DataId readId = selectAllReadsQuery.getDataId(0, U2Type::AssemblyRead);
        qint64 startPos = selectAllReadsQuery.getInt64(1);
        qint64 endPos = startPos + selectAllReadsQuery.getInt64(2);
        int prow = selectProw(tails.data(), startPos, endPos);
        updateReadQuery.reset();
        updateReadQuery.bindDataId(2, readId);
        updateReadQuery.bindInt64(1, prow);
        updateReadQuery.execute();
    }
    if (os.hasError()) {
        return;
    }

    // now process all unassigned reads with simple algorithm: increment their row
    int prow = TAIL_SIZE;
    SQLiteQuery selectUnassignedQuery = SQLiteQuery(QString("SELECT id FROM %1 WHERE prow = -1").arg(readsTable), db, os);
    //selectUnassignedQuery.bindDataId(1, assemblyId);
    while (selectUnassignedQuery.step() && !os.hasError()) {
        U2DataId readId = selectUnassignedQuery.getDataId(0, U2Type::AssemblyRead);
        updateReadQuery.reset();
        updateReadQuery.bindDataId(1, readId);
        updateReadQuery.bindInt64(2, prow++);
        updateReadQuery.execute();    
    }
}

} //namespace
