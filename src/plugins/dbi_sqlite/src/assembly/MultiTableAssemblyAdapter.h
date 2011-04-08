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

#ifndef _U2_SQLITE_ASSEMBLY_MULTI_TABLE_DBI_H_
#define _U2_SQLITE_ASSEMBLY_MULTI_TABLE_DBI_H_

#include "SingleTableAssemblyAdapter.h"
#include "AssemblyPackAlgorithm.h"

#include <U2Core/U2SqlHelpers.h>

namespace U2 {

class MTASingleTableAdapter {
public:
    MTASingleTableAdapter(SingleTableAssemblyAdapter* a, const U2Region& r, const QByteArray& extra) 
        : singleTableAdapter(a), readsRange(r), idExtra(extra) {}

    SingleTableAssemblyAdapter* singleTableAdapter;
    U2Region                    readsRange;
    QByteArray                  idExtra;
};
enum MultiTableAssemblyAdapterMode {
    MultiTableAssemblyAdapterMode_4Tables
};

class MultiTableAssemblyAdapter : public AssemblyAdapter {
public:
    MultiTableAssemblyAdapter(SQLiteDbi* dbi, const U2DataId& assemblyId, const AssemblyCompressor* compressor, 
        MultiTableAssemblyAdapterMode mode, DbRef* ref, U2OpStatus& os);

    ~MultiTableAssemblyAdapter();

    virtual void createReadsTables(U2OpStatus& os);
    virtual void createReadsIndexes(U2OpStatus& os);

    virtual qint64 countReads(const U2Region& r, U2OpStatus& os);

    virtual qint64 getMaxPackedRow(const U2Region& r, U2OpStatus& os);
    virtual qint64 getMaxEndPos(U2OpStatus& os);

    virtual U2DbiIterator<U2AssemblyRead>* getReads(const U2Region& r, U2OpStatus& os);
    virtual U2DbiIterator<U2AssemblyRead>* getReadsByRow(const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os);
    virtual U2DbiIterator<U2AssemblyRead>* getReadsByName(const QByteArray& name, U2OpStatus& os);

    virtual void addReads(QList<U2AssemblyRead>& reads, U2OpStatus& os);
    virtual void removeReads(const QList<U2DataId>& readIds, U2OpStatus& os);

    virtual void pack(U2OpStatus& os);

    int getReadRange(qint64 readLength) const;
    int getReadRange(const U2DataId& id) const;
    const QList<MTASingleTableAdapter*>& getTableAdapters() const {return tableAdapters;}

protected:
    void addTableAdapter(int minLen, int maxLen, const U2DataId& assemblyId, const AssemblyCompressor* compressor, bool last, U2OpStatus& os);

    SQLiteDbi*                      dbi;
    QList<MTASingleTableAdapter*>   tableAdapters;
};


class MultiTablePackAlgorithmAdapter : public PackAlgorithmAdapter {
public:
    MultiTablePackAlgorithmAdapter(DbRef* db, MultiTableAssemblyAdapter* a); 
    ~MultiTablePackAlgorithmAdapter();
    
    virtual U2DbiIterator<PackAlgorithmData>* selectAllReads(U2OpStatus& os);
    virtual void assignProw(const U2DataId& readId, qint64 prow, U2OpStatus& os);
private:
    MultiTableAssemblyAdapter*              multiTableAdapter;
    QList<SingleTablePackAlgorithmAdapter*> packAdapters;
};

// Class that multiplexes multiple read iterators into 1
class MTAReadsIterator : public U2DbiIterator<U2AssemblyRead> {
public:
    MTAReadsIterator(QList< U2DbiIterator<U2AssemblyRead>* >& iterators, const MultiTableAssemblyAdapter* a);

    virtual ~MTAReadsIterator();

    virtual bool hasNext();

    virtual U2AssemblyRead next();

    virtual U2AssemblyRead peek();

private:
    const MultiTableAssemblyAdapter*        multiTableAdapter;
    QList<U2DbiIterator<U2AssemblyRead>*>   iterators;
    int                                     currentRange;
};

// Class that multiplexes multiple read packed data iterators into 1 and supports ordering
class MTAPackAlgorithmDataIterator : public U2DbiIterator<PackAlgorithmData> {
public:
    MTAPackAlgorithmDataIterator(QList< U2DbiIterator<PackAlgorithmData>* >& iterators, const MultiTableAssemblyAdapter* a);

    virtual ~MTAPackAlgorithmDataIterator();

    virtual bool hasNext();

    virtual PackAlgorithmData next();

    virtual PackAlgorithmData peek();

private:
    void fetchNextData();

    const MultiTableAssemblyAdapter*            multiTableAdapter;
    QList<U2DbiIterator<PackAlgorithmData>*>    iterators;
    PackAlgorithmData                           nextData;
};

} //namespace

#endif
