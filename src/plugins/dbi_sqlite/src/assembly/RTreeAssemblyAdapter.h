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

#ifndef _U2_SQLITE_ASSEMBLY_RTREE_DBI_H_
#define _U2_SQLITE_ASSEMBLY_RTREE_DBI_H_

#include "SQLiteAssemblyDbi.h"
#include "AssemblyPackAlgorithm.h"

#include <U2Core/U2SqlHelpers.h>

namespace U2 {


class RTreeAssemblyAdapter : public AssemblyAdapter {
public:
    RTreeAssemblyAdapter(SQLiteDbi* dbi, const U2DataId& assemblyId, const AssemblyCompressor* compressor, DbRef* ref, U2OpStatus& os);

    virtual void createReadsTables(U2OpStatus& os);
    virtual void createReadsIndexes(U2OpStatus& os);

    virtual qint64 countReads(const U2Region& r, U2OpStatus& os);

    virtual qint64 getMaxPackedRow(const U2Region& r, U2OpStatus& os);
    virtual quint64 getMaxEndPos(U2OpStatus& os);

    virtual U2DbiIterator<U2AssemblyRead>* getReads(const U2Region& r, U2OpStatus& os) const;
    virtual U2DbiIterator<U2AssemblyRead>* getReadsByRow(const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os);
    virtual U2DbiIterator<U2AssemblyRead>* getReadsByName(const QByteArray& name, U2OpStatus& os);

    virtual void addReads(QList<U2AssemblyRead>& rows, U2OpStatus& os);
    virtual void removeReads(const QList<U2DataId>& rowIds, U2OpStatus& os);

    virtual void pack(U2OpStatus& os);

protected:
    SQLiteDbi*  dbi;
    QString     readsTable;
    QString     indexTable;
};

class RTreeAssemblyAdapterReadLoader : public SqlRSLoader<U2AssemblyRead> {
public:
    U2AssemblyRead load(SQLiteQuery* q);
};

class RTreePackAlgorithmAdapter : public PackAlgorithmAdapter {
public:
    RTreePackAlgorithmAdapter(DbRef* _db, const QString& _readsTable, const QString& _indexTable) 
        : db(_db), readsTable(_readsTable), indexTable(_indexTable), updateQuery(NULL){}
    ~RTreePackAlgorithmAdapter();

    virtual U2DbiIterator<PackAlgorithmData>* selectAllReads(U2OpStatus& os);
    virtual U2DbiIterator<U2DataId>* selectNotAssignedReads(U2OpStatus& os);
    virtual void assignProw(const U2DataId& readId, qint64 prow, U2OpStatus& os);
private:
    DbRef*          db;
    QString         readsTable;
    QString         indexTable;
    SQLiteQuery*    updateQuery;
};

class RTreeAssemblyAdapterPackedReadLoader : public SqlRSLoader<PackAlgorithmData> {
public:
    virtual PackAlgorithmData load(SQLiteQuery* q);
};

} //namespace

#endif
