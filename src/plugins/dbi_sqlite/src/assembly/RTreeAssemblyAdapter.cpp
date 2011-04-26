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

#define RANGE_CONDITION_CHECK   QString(" (i.gstart < ?1 AND i.gend > ?2) ")
#define ALL_READ_FIELDS         QString(" r.id, i.prow1, i.gstart, i.gend - i.gstart, r.flags, r.mq, r.data")
#define SAME_IDX                QString(" (i.id == r.id) ")
#define FROM_2TABLES            QString(" FROM %1 AS r, %2 AS i ")


RTreeAssemblyAdapter::RTreeAssemblyAdapter(SQLiteDbi* _dbi, const U2DataId& assemblyId, 
                                            const AssemblyCompressor* compressor,
                                            DbRef* db, U2OpStatus& os)
: AssemblyAdapter(assemblyId, compressor, db)
{
    dbi = _dbi;
    readsTable = QString("AssemblyRead_R%1").arg(SQLiteUtils::toDbiId(assemblyId));
    indexTable = readsTable + "_RTree";
}

void RTreeAssemblyAdapter::createReadsTables(U2OpStatus& os) {
    // name - name hash
    // flags - flags
    // mq - mapping quality
    // data - compressed name/sequence/cigar/mapping
    static QString q1 = "CREATE TABLE %1 (id INTEGER PRIMARY KEY AUTOINCREMENT, name INTEGER NOT NULL, "
        "flags INTEGER NOT NULL, mq INTEGER NOT NULL, data BLOB NOT NULL)";
    
    // gstart, gend - start and end read position
    // prow1, prow2 - packed view row. prow1 always the same as prow2
    static QString q2 = "CREATE VIRTUAL TABLE %1 USING rtree_i32(id, gstart, gend, prow1, prow2)";
    SQLiteQuery(q1.arg(readsTable), db, os).execute();
    if (os.hasError()) {
        return;
    }
    SQLiteQuery(q2.arg(indexTable), db, os).execute();
    if (os.hasError()) {
        coreLog.error(SQLiteL10N::tr("Error during RTree index creation: %1! Check if SQLite library has RTree index support!").arg(os.getError()));
    }
}

void RTreeAssemblyAdapter::createReadsIndexes(U2OpStatus& os) {
    static QString q = "CREATE INDEX %1_name ON %1(name)";
    SQLiteQuery(q.arg(readsTable), db, os).execute();
}

qint64 RTreeAssemblyAdapter::countReads(const U2Region& r, U2OpStatus& os) {
    if (r == U2_ASSEMBLY_REGION_MAX) {
        return SQLiteQuery(QString("SELECT COUNT(*) FROM %1").arg(readsTable), db, os).selectInt64();
    }
    QString qStr = QString("SELECT COUNT(*) FROM %1 AS i WHERE " + RANGE_CONDITION_CHECK).arg(indexTable);
    SQLiteQuery q(qStr, db, os);
    q.bindInt64(1, r.endPos());
    q.bindInt64(2, r.startPos);
    return q.selectInt64();
}

qint64 RTreeAssemblyAdapter::getMaxPackedRow(const U2Region& r, U2OpStatus& os) {
    SQLiteQuery q(QString("SELECT MAX(prow1) FROM %1 AS i WHERE (" + RANGE_CONDITION_CHECK + ")").arg(indexTable), db, os);
    q.bindInt64(1, r.endPos());
    q.bindInt64(2, r.startPos);
    return q.selectInt64();
}

qint64 RTreeAssemblyAdapter::getMaxEndPos(U2OpStatus& os) {
    return SQLiteQuery(QString("SELECT MAX(gend) FROM %1").arg(indexTable), db, os).selectInt64();
}

U2DbiIterator<U2AssemblyRead>* RTreeAssemblyAdapter::getReads(const U2Region& r, U2OpStatus& os) {
    QString qStr = QString("SELECT " + ALL_READ_FIELDS + FROM_2TABLES + " WHERE " + SAME_IDX + " AND "+ RANGE_CONDITION_CHECK )
        .arg(readsTable).arg(indexTable);
    SQLiteQuery* q = new SQLiteQuery(qStr, db, os);
    q->bindInt64(1, r.endPos());
    q->bindInt64(2, r.startPos);
    return new SqlRSIterator<U2AssemblyRead>(q, new SimpleAssemblyReadLoader(), NULL, U2AssemblyRead(), os);
}

U2DbiIterator<U2AssemblyRead>* RTreeAssemblyAdapter::getReadsByRow(const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os) {
    QString qStr = QString("SELECT " + ALL_READ_FIELDS + FROM_2TABLES + " WHERE " + SAME_IDX + " AND "+ RANGE_CONDITION_CHECK + 
                        " AND (i.prow1 >= ?3 AND i.prow2 < ?4)").arg(readsTable).arg(indexTable);
    SQLiteQuery* q = new SQLiteQuery(qStr, db, os);
    q->bindInt64(1, r.endPos());
    q->bindInt64(2, r.startPos);
    q->bindInt64(3, minRow);
    q->bindInt64(4, maxRow);
    return new SqlRSIterator<U2AssemblyRead>(q, new SimpleAssemblyReadLoader(), NULL, U2AssemblyRead(), os);
}

U2DbiIterator<U2AssemblyRead>* RTreeAssemblyAdapter::getReadsByName(const QByteArray& name, U2OpStatus& os) {
    QString qStr = QString("SELECT " + ALL_READ_FIELDS + " FROM %1 WHERE name = ?1").arg(readsTable);
    SQLiteQuery* q = new SQLiteQuery(qStr, db, os);
    int hash = qHash(name);
    q->bindInt64(1, hash);
    return new SqlRSIterator<U2AssemblyRead>(q, new SimpleAssemblyReadLoader(), 
        new SQLiteAssemblyNameFilter(name), U2AssemblyRead(), os);
}


void RTreeAssemblyAdapter::addReads(QList<U2AssemblyRead>& reads, U2OpStatus& os) {
    static QString q1 = "INSERT INTO %1(name, flags, mq, data) VALUES (?1, ?2, ?3, ?4)";
    static QString q2 = "INSERT INTO %1(id, gstart, gend, prow1, prow2) VALUES (?1, ?2, ?3, ?4, ?5)";

    SQLiteTransaction t(db, os);
    SQLiteQuery insertRQ(q1.arg(readsTable), db, os);
    SQLiteQuery insertIQ(q2.arg(indexTable), db, os);

    for (int i = 0, n = reads.size(); i < n && !os.isCoR(); i++) {
        U2AssemblyRead& read = reads[i];
        bool dnaExt = false; //TODO
        
        QByteArray cigarText = U2AssemblyUtils::cigar2String(read->cigar);

        int flags = 0;
        flags = flags | (read->complementary ? (1 << BIT_COMPLEMENTARY_STRAND) : 0);
        flags = flags | (dnaExt ? (1 << BIT_EXT_DNA_ALPHABET) : 0 );
        flags = flags | (read->paired ? (1 << BIT_PAIRED_READ) : 0 );
        flags = flags | (read->mapped ? (1 << BIT_MAPPED_READ) : 0);
        
        int readLen = read->readSequence.length();
        int effectiveReadLength = readLen + U2AssemblyUtils::getCigarExtraLength(read->cigar);
        read->effectiveLen = effectiveReadLength;
        
        int hash = qHash(read->name);
        insertRQ.reset();
        insertRQ.bindInt64(1, hash);
        insertRQ.bindInt64(2, flags);
        insertRQ.bindInt32(3, read->mappingQuality);
        QByteArray packedData = SQLiteAssemblyUtils::packData(SQLiteAssemblyDataMethod_NSCQ, read->name, read->readSequence, cigarText, read->quality, os);
        insertRQ.bindBlob(4, packedData, false);

        read->id = insertRQ.insert(U2Type::AssemblyRead);

        if (os.hasError()) {
            break;
        }
        insertIQ.reset();
        insertIQ.bindDataId(1, read->id);
        insertIQ.bindInt64(2, read->leftmostPos);
        insertIQ.bindInt64(3, read->leftmostPos + read->effectiveLen);
        insertIQ.bindInt64(4, read->packedViewRow);
        insertIQ.bindInt64(5, read->packedViewRow);

        insertIQ.execute();

//#define U2_SQLITE_CHECK_RTREE_
#ifdef U2_SQLITE_CHECK_RTREE_
// Consistency check. To be removed after all known rtree issues are resolved
        qint64 dbId = SQLiteUtils::toDbiId(read->id);
        SQLiteQuery cq("SELECT gstart, gend FROM " + indexTable + " WHERE id = " + QString::number(dbId), db, os);
        cq.step();
        qint64 cstart =  cq.getInt64(0);
        qint64 cend =  cq.getInt64(1);
        assert(cstart == read->leftmostPos);
        assert(cend == read->leftmostPos + read->effectiveLen);
#endif
    }
}

void RTreeAssemblyAdapter::removeReads(const QList<U2DataId>& readIds, U2OpStatus& os) {
    SQLiteObjectDbi* objDbi = dbi->getSQLiteObjectDbi();
    foreach(const U2DataId& readId, readIds) {
        SQLiteUtils::remove(readsTable, "id", readId, 1, db, os);
        SQLiteUtils::remove(indexTable, "id", readId, 1, db, os);
    }
    SQLiteObjectDbi::incrementVersion(assemblyId, db, os);
}

void RTreeAssemblyAdapter::pack(U2AssemblyPackStat& stat, U2OpStatus& os) {
    RTreePackAlgorithmAdapter packAdapter(db, readsTable, indexTable);
    AssemblyPackAlgorithm::pack(packAdapter, stat, os);
}


U2DbiIterator<PackAlgorithmData>* RTreePackAlgorithmAdapter::selectAllReads(U2OpStatus& os) {
    SQLiteQuery* q = new SQLiteQuery("SELECT id, gstart, gend - gstart FROM " + indexTable + " ORDER BY gstart", db, os);
    return new SqlRSIterator<PackAlgorithmData>(q, new SimpleAssemblyReadPackedDataLoader(), NULL, PackAlgorithmData(), os);
}

RTreePackAlgorithmAdapter::~RTreePackAlgorithmAdapter() {
    delete updateQuery;
}

void RTreePackAlgorithmAdapter::assignProw(const U2DataId& readId, qint64 prow, U2OpStatus& os) {
    if (updateQuery == NULL) {
        updateQuery = new SQLiteQuery("UPDATE " + indexTable + " SET prow1 = ?1, prow2=?1 WHERE id = ?2", db, os);
    } 
    updateQuery->setOpStatus(os);
    updateQuery->reset();
    updateQuery->bindInt64(1, prow);
    updateQuery->bindDataId(2, readId);
    updateQuery->execute();
}

} //namespace
