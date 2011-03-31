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

#include "RTreeAssemblyAdapter.h"
#include "../SQLiteDbi.h"
#include "../SQLiteAssemblyDbi.h"
#include "../SQLiteObjectDbi.h"


#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2SqlHelpers.h>

namespace U2 {

    
RTreeAssemblyAdapter::RTreeAssemblyAdapter(SQLiteDbi* _dbi, const U2DataId& assemblyId, 
                                            const AssemblyCompressor* compressor, 
                                            DbRef* db, U2OpStatus& os)
: AssemblyAdapter(assemblyId, compressor, db, os)
{
    dbi = _dbi;
    readsTable = QString("AssemblyRead_R%1").arg(SQLiteUtils::toDbiId(assemblyId));
    indexTable = readsTable + "_RTree";
}

void RTreeAssemblyAdapter::createReadsTables(U2OpStatus& os) {
    static QString q1 = "CREATE TABLE %1 (id INTEGER PRIMARY KEY AUTOINCREMENT, sequence INTEGER NOT NULL, prow INTEGER NOT NULL, "
        "data BLOB NOT NULL, flags INTEGER NOT NULL, cigar TEXT NOT NULL)";

    static QString q2 = "CREATE VIRTUAL TABLE %1 USING rtree_i32(id, gstart, gend)";

    SQLiteQuery(q1.arg(readsTable), db, os).execute();
    if (os.hasError()) {
        return;
    }
    SQLiteQuery(q2.arg(indexTable), db, os).execute();
    if (os.hasError()) {
        coreLog.error(SQLiteL10N::tr("Error during RTree index creation: %1! Check if SQLite library has RTree index support!").arg(os.getError()));
    }
}

#define RANGE_CONDITION_CHECK   QString(" (i.gstart < ?1 AND i.gend > ?2) ")
#define ALL_READ_FIELDS         QString(" r.id, r.sequence, r.prow, r.data, i.gstart, i.gend, r.flags, r.cigar ")
#define SAME_IDX                QString(" (i.id == r.id) ")
#define FROM_2TABLES            QString(" FROM %1 AS r, %2 AS i ")

qint64 RTreeAssemblyAdapter::countReads(const U2Region& r, U2OpStatus& os) {
    QString qStr = QString("SELECT COUNT(*) FROM %1 AS i WHERE " + RANGE_CONDITION_CHECK).arg(indexTable);
    SQLiteQuery q(qStr, db, os);
    q.bindInt64(1, r.endPos());
    q.bindInt64(2, r.startPos);
    return q.selectInt64();
}

qint64 RTreeAssemblyAdapter::getMaxPackedRow(const U2Region& r, U2OpStatus& os) {
    SQLiteQuery q(QString("SELECT MAX(r.prow) "+FROM_2TABLES+" WHERE " + SAME_IDX + " AND (" + RANGE_CONDITION_CHECK + ")")
        .arg(readsTable).arg(indexTable), db, os);
    q.bindInt64(1, r.endPos());
    q.bindInt64(2, r.startPos);
    return q.selectInt64();
}

quint64 RTreeAssemblyAdapter::getMaxEndPos(U2OpStatus& os) {
    return SQLiteQuery(QString("SELECT MAX(gend) FROM %1").arg(indexTable), db, os).selectInt64();
}

U2DbiIterator<U2AssemblyRead>* RTreeAssemblyAdapter::getReads(const U2Region& r, U2OpStatus& os) const {
    QString qStr = QString("SELECT " + ALL_READ_FIELDS + FROM_2TABLES + " WHERE " + SAME_IDX + " AND "+ RANGE_CONDITION_CHECK )
        .arg(readsTable).arg(indexTable);
    SQLiteQuery* q = new SQLiteQuery(qStr, db, os);
    q->bindInt64(1, r.endPos());
    q->bindInt64(2, r.startPos);
    return new SqlRSIterator<U2AssemblyRead>(q, new RTreeAssemblyAdapterReadLoader(), U2AssemblyRead(), os);
}

U2DbiIterator<U2AssemblyRead>* RTreeAssemblyAdapter::getReadsByRow(const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os) {
    QString qStr = QString("SELECT " + ALL_READ_FIELDS + FROM_2TABLES + " WHERE " + SAME_IDX + " AND "+ RANGE_CONDITION_CHECK + 
                        " AND (r.prow >= ?3 AND r.prow < ?4)").arg(readsTable).arg(indexTable);
    SQLiteQuery* q = new SQLiteQuery(qStr, db, os);
    q->bindInt64(1, r.endPos());
    q->bindInt64(2, r.startPos);
    q->bindInt64(3, minRow);
    q->bindInt64(4, maxRow);
    return new SqlRSIterator<U2AssemblyRead>(q, new RTreeAssemblyAdapterReadLoader(), U2AssemblyRead(), os);
}

void RTreeAssemblyAdapter::addReads(QList<U2AssemblyRead>& rows, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    QString q1 = "INSERT INTO %1(sequence, prow, flags, data, cigar) VALUES (?1, ?2, ?3, ?4, ?5)";
    SQLiteQuery insertRQ(q1.arg(readsTable), db, os);

    QString q2 = "INSERT INTO %1(id, gstart, gend) VALUES (?1, ?2, ?3)";
    SQLiteQuery insertIQ(q2.arg(indexTable), db, os);

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
        row->effectiveLen = effectiveRowLength;

        
        insertRQ.reset();
        insertRQ.bindDataId(1, row->sequenceId);
        insertRQ.bindInt64(2, row->packedViewRow);
        insertRQ.bindInt64(3, flags);
        insertRQ.bindBlob(4, row->readSequence, false);
        insertRQ.bindText(5, cigarText);
        
        row->id = insertRQ.insert(U2Type::AssemblyRead);

        if (os.hasError()) {
            break;
        }
        insertIQ.reset();
        insertIQ.bindDataId(1, row->id);
        insertIQ.bindInt64(2, row->leftmostPos);
        insertIQ.bindInt64(3, row->leftmostPos + row->effectiveLen);
        insertIQ.execute();

//#define U2_SQLITE_CHECK_RTREE_
#ifdef U2_SQLITE_CHECK_RTREE_
// Consistency check. To be removed after all known rtree issues are resolved
        qint64 dbId = SQLiteUtils::toDbiId(row->id);
        SQLiteQuery cq("SELECT gstart, gend FROM " + indexTable + " WHERE id = " + QString::number(dbId), db, os);
        cq.step();
        qint64 cstart =  cq.getInt64(0);
        qint64 cend =  cq.getInt64(1);
        assert(cstart == row->leftmostPos);
        assert(cend == row->leftmostPos + row->effectiveLen);
#endif
    }
}

void RTreeAssemblyAdapter::removeReads(const QList<U2DataId>& rowIds, U2OpStatus& os) {
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
        SQLiteUtils::remove(indexTable, "id", rowId, 1, db, os);
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

void RTreeAssemblyAdapter::pack(U2OpStatus& os) {
    RTreePackAlgorithmAdapter packAdapter(db, readsTable, indexTable);
    AssemblyPackAlgorithm::pack(packAdapter, os);
}

//TODO: reuse single table adapter code.
U2AssemblyRead RTreeAssemblyAdapterReadLoader::load(SQLiteQuery* q) {
    U2AssemblyRead read(new U2AssemblyReadData());

    read->id = q->getDataId(0, U2Type::AssemblyRead);
    read->sequenceId = q->getDataId(1, U2Type::Sequence);
    read->packedViewRow = q->getInt64(2);
    if (q->hasError()) {
        return U2AssemblyRead();
    }
    read->readSequence = q->getBlob(3);
    read->leftmostPos = q->getInt64(4);
    qint64 endPos = q->getInt64(5);
    read->effectiveLen = endPos - read->leftmostPos;
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
    //additional check to ensure that db contains correct info
    if (read->sequenceId.isEmpty()) {
        qint64 effectiveLengthFromCigar = read->readSequence.length() + U2AssemblyUtils::getCigarExtraLength(read->cigar);
        assert(effectiveLengthFromCigar == read->effectiveLen);
    }
#endif
    return read;
}


U2DbiIterator<PackAlgorithmData>* RTreePackAlgorithmAdapter::selectAllReads(U2OpStatus& os) {
    SQLiteQuery* q = new SQLiteQuery("SELECT id, gstart, gend FROM " + indexTable, db, os);
    return new SqlRSIterator<PackAlgorithmData>(q, new RTreeAssemblyAdapterPackedReadLoader(), PackAlgorithmData(), os);
}

RTreePackAlgorithmAdapter::~RTreePackAlgorithmAdapter() {
    delete updateQuery;
}

U2DbiIterator<U2DataId>* RTreePackAlgorithmAdapter::selectNotAssignedReads(U2OpStatus& os) {
    QString qStr = QString("SELECT i.id, i.gstart, i.gend " + FROM_2TABLES + " WHERE r.prow = -1 AND " + SAME_IDX).arg(readsTable).arg(indexTable);
    SQLiteQuery* q = new SQLiteQuery(qStr, db, os);
    return new SqlRSIterator<U2DataId>(q, new SqlDataIdRSLoader(U2Type::AssemblyRead), U2DataId(), os);
}

void RTreePackAlgorithmAdapter::assignProw(const U2DataId& readId, qint64 prow, U2OpStatus& os) {
    if (updateQuery == NULL) {
        updateQuery = new SQLiteQuery("UPDATE " + readsTable + " SET prow = ?1 WHERE id = ?2", db, os);
    }
    updateQuery->setOpStatus(os);
    updateQuery->reset();
    updateQuery->bindInt64(1, prow);
    updateQuery->bindDataId(2, readId);
    updateQuery->execute();
}

PackAlgorithmData RTreeAssemblyAdapterPackedReadLoader::load(SQLiteQuery* q) {
    PackAlgorithmData data;
    data.readId = q->getDataId(0, U2Type::AssemblyRead);
    data.leftmostPos = q->getInt64(1);
    data.effectiveLen = q->getInt64(2) - data.leftmostPos;
    return data;
}


} //namespace
