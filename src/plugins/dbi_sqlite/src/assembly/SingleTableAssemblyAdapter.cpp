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

#include "SingleTableAssemblyAdapter.h"
#include "../SQLiteDbi.h"
#include "../SQLiteAssemblyDbi.h"
#include "../SQLiteObjectDbi.h"


#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2SqlHelpers.h>

namespace U2 {

SingleTableAssemblyAdapter::SingleTableAssemblyAdapter(SQLiteDbi* _dbi, const U2DataId& assemblyId, 
                                                       const AssemblyCompressor* compressor, 
                                                       DbRef* db, U2OpStatus& os)
                                                       : AssemblyAdapter(assemblyId, compressor, db, os)
{
    dbi = _dbi;
    readsTable = QString("AssemblyRead_S%1").arg(SQLiteUtils::toDbiId(assemblyId));
}

void SingleTableAssemblyAdapter::createReadsTables(U2OpStatus& os) {
    static QString q = "CREATE TABLE %1 (id INTEGER PRIMARY KEY AUTOINCREMENT, sequence INTEGER NOT NULL, prow INTEGER NOT NULL, "
        "data BLOB NOT NULL, gstart INTEGER NOT NULL, elen INTEGER NOT NULL, flags INTEGER NOT NULL, cigar TEXT NOT NULL)";

    SQLiteQuery(q.arg(readsTable), db, os).execute();
}

void SingleTableAssemblyAdapter::createReadsIndexes(U2OpStatus& os) {
    //TODO: check if we can have a benefit with 2-colum index here: gstart + elen
    static QString q = "CREATE INDEX %1_gstart ON %1(gstart)";
    SQLiteQuery(q.arg(readsTable), db, os).execute();
}

#define RANGE_CONDITION_CHECK   QString(" (gstart < ?1 AND gstart + elen > ?2) ")
#define ALL_READ_FIELDS         QString(" id, sequence, prow, data, gstart, elen, flags, cigar ")

qint64 SingleTableAssemblyAdapter::countReads(const U2Region& r, U2OpStatus& os) {
    QString qStr = QString("SELECT COUNT(*) FROM %1 WHERE " + RANGE_CONDITION_CHECK).arg(readsTable);
    SQLiteQuery q(qStr, db, os);
    q.bindInt64(1, r.endPos());
    q.bindInt64(2, r.startPos);
    return q.selectInt64();
}

qint64 SingleTableAssemblyAdapter::getMaxPackedRow(const U2Region& r, U2OpStatus& os) {
    SQLiteQuery q(QString("SELECT MAX(prow) FROM %1 WHERE " + RANGE_CONDITION_CHECK).arg(readsTable), db, os);
    q.bindInt64(1, r.endPos());
    q.bindInt64(2, r.startPos);
    return q.selectInt64();
}

quint64 SingleTableAssemblyAdapter::getMaxEndPos(U2OpStatus& os) {
    return SQLiteQuery(QString("SELECT MAX(gstart + elen) FROM %1").arg(readsTable), db, os).selectInt64();
}

U2DbiIterator<U2AssemblyRead>* SingleTableAssemblyAdapter::getReads(const U2Region& r, U2OpStatus& os) const {
    QString qStr = QString("SELECT " + ALL_READ_FIELDS + " FROM %1 WHERE " + RANGE_CONDITION_CHECK).arg(readsTable);
    SQLiteQuery* q = new SQLiteQuery(qStr, db, os);
    q->bindInt64(1, r.endPos());
    q->bindInt64(2, r.startPos);
    return new SqlRSIterator<U2AssemblyRead>(q, new SingleTableAssemblyAdapterReadLoader(), U2AssemblyRead(), os);
}

U2DbiIterator<U2AssemblyRead>* SingleTableAssemblyAdapter::getReadsByRow(const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os) {
    QString qStr = QString("SELECT " + ALL_READ_FIELDS + " FROM %1 WHERE " + RANGE_CONDITION_CHECK + " AND (prow >= ?3 AND prow < ?4)")
        .arg(readsTable);
    SQLiteQuery* q = new SQLiteQuery(qStr, db, os);
    q->bindInt64(1, r.endPos());
    q->bindInt64(2, r.startPos);
    q->bindInt64(3, minRow);
    q->bindInt64(4, maxRow);
    return new SqlRSIterator<U2AssemblyRead>(q, new SingleTableAssemblyAdapterReadLoader(), U2AssemblyRead(), os);
}

void SingleTableAssemblyAdapter::addReads(QList<U2AssemblyRead>& rows, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    QString q = "INSERT INTO %1(sequence, prow, flags, gstart, elen, data, cigar) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7)";
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
            U2Sequence rowSeq = dbi->getSequenceDbi()->getSequenceObject(row->sequenceId, os);
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


        insertQ.reset();
        insertQ.bindDataId(1, row->sequenceId);
        insertQ.bindInt64(2, row->packedViewRow);
        insertQ.bindInt64(3, flags);
        insertQ.bindInt64(4, row->leftmostPos);
        insertQ.bindInt64(5, effectiveRowLength);;
        insertQ.bindBlob(6, row->readSequence, false);
        insertQ.bindText(7, cigarText);

        row->id = insertQ.insert(U2Type::AssemblyRead);
    }
}

void SingleTableAssemblyAdapter::removeReads(const QList<U2DataId>& rowIds, U2OpStatus& os) {
    SQLiteObjectDbi* objDbi = dbi->getSQLiteObjectDbi();
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

void SingleTableAssemblyAdapter::pack(U2OpStatus& os) {
    SingleTablePackAlgorithmAdapter packAdapter(db, readsTable);
    AssemblyPackAlgorithm::pack(packAdapter, os);
}


U2AssemblyRead SingleTableAssemblyAdapterReadLoader::load(SQLiteQuery* q) {
    U2AssemblyRead read(new U2AssemblyReadData());

    read->id = q->getDataId(0, U2Type::AssemblyRead);
    read->sequenceId = q->getDataId(1, U2Type::Sequence);
    read->packedViewRow = q->getInt64(2);
    if (q->hasError()) {
        return U2AssemblyRead();
    }
    read->readSequence = q->getBlob(3);
    read->leftmostPos= q->getInt64(4);
    read->effectiveLen = q->getInt64(5);
    int flags = q->getInt64(6);
    read->complementary = SQLiteAssemblyUtils::isComplementaryRead(flags);
    QByteArray cigar = q->getCString(7);
    if (q->hasError()) {
        return U2AssemblyRead();
    }
    QString err;
    read->cigar = U2AssemblyUtils::parseCigar(cigar, err);;
    if (!err.isEmpty()) {
        q->setError(err);
        return U2AssemblyRead();
    }
#ifdef _DEBUG
    //additional check to ensure that db stores correct info
    if (read->sequenceId.isEmpty()) {
        qint64 effectiveLengthFromCigar = read->readSequence.length() + U2AssemblyUtils::getCigarExtraLength(read->cigar);
        assert(effectiveLengthFromCigar == read->effectiveLen);
    }
#endif
    return read;
}


U2DbiIterator<PackAlgorithmData>* SingleTablePackAlgorithmAdapter::selectAllReads(U2OpStatus& os) {
    SQLiteQuery* q = new SQLiteQuery("SELECT id, gstart, elen FROM " + readsTable, db, os);
    return new SqlRSIterator<PackAlgorithmData>(q, new SingleTableAssemblyAdapterPackedReadLoader(), PackAlgorithmData(), os);
}

SingleTablePackAlgorithmAdapter::~SingleTablePackAlgorithmAdapter() {
    delete updateQuery;
}

U2DbiIterator<U2DataId>* SingleTablePackAlgorithmAdapter::selectNotAssignedReads(U2OpStatus& os) {
    SQLiteQuery* q = new SQLiteQuery("SELECT id, gstart, elen FROM " + readsTable + " WHERE prow = -1", db, os);
    return new SqlRSIterator<U2DataId>(q, new SqlDataIdRSLoader(U2Type::AssemblyRead), U2DataId(), os);
}

void SingleTablePackAlgorithmAdapter::assignProw(const U2DataId& readId, qint64 prow, U2OpStatus& os) {
    if (updateQuery == NULL) {
        updateQuery = new SQLiteQuery("UPDATE " + readsTable + " SET prow = ?1 WHERE id = ?2", db, os);
    }
    updateQuery->setOpStatus(os);
    updateQuery->reset();
    updateQuery->bindInt64(1, prow);
    updateQuery->bindDataId(2, readId);
    updateQuery->execute();
}

PackAlgorithmData SingleTableAssemblyAdapterPackedReadLoader::load(SQLiteQuery* q) {
    PackAlgorithmData data;
    data.readId = q->getDataId(0, U2Type::AssemblyRead);
    data.leftmostPos = q->getInt64(1);
    data.effectiveLen = q->getInt64(2);
    return data;
}


} //namespace
