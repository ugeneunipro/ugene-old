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

#ifndef _U2_SQLITE_ASSEMBLY_DBI_H_
#define _U2_SQLITE_ASSEMBLY_DBI_H_

#include "SQLiteDbi.h"

namespace U2 {

class SQLiteQuery;

class SQLiteAssemblyDbi : public U2AssemblyDbi, public SQLiteChildDBICommon {

public:
    SQLiteAssemblyDbi(SQLiteDbi* dbi);

    /** Reads assembly objects by id */
    virtual U2Assembly getAssemblyObject(const U2DataId& assemblyId, U2OpStatus& os);

    /** 
        Return number of reads in assembly that intersect given region 
        'Intersect' here means that region(leftmost pos, rightmost pos) intersects with 'r'
    */
    virtual qint64 countReadsAt(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os);

    /** Return 'count' row ids starting with 'offset' that intersect given region */
    virtual QList<U2DataId> getReadIdsAt(const U2DataId& assemblyId, const U2Region& r, qint64 offset, qint64 count, U2OpStatus& os);

    /** Return 'count' rows starting with 'offset' that intersect given region */
    virtual QList<U2AssemblyRead> getReadsAt(const U2DataId& assemblyId, const U2Region& r, qint64 offset, qint64 count, U2OpStatus& os);
    
    /** 
        Return max packed row at the given coordinate
        'Intersect' here means that region(leftmost pos, rightmost pos) intersects with 'r'
    */
    virtual qint64 getMaxPackedRow(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os);

    /** Return reads with packed row value >= min, <=max that intersect given region */
    virtual QList<U2AssemblyRead> getReadsByRow(const U2DataId& assemblyId, const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os);

    /** Count 'length of assembly' - position of the rightmost base of all reads */
    virtual quint64 getMaxEndPos(const U2DataId& assemblyId, U2OpStatus& os);


    /** Creates new empty assembly object, reads iterator can be NULL  */
    virtual void createAssemblyObject(U2Assembly& assembly, const QString& folder, U2AssemblyReadsIterator* it, U2OpStatus& os);
    
    /** 
        Removes sequences from assembly
        Automatically removes affected sequences that are not anymore accessible from folders
    */
    virtual void removeReads(const U2DataId& assemblyId, const QList<U2DataId>& rowIds, U2OpStatus& os);

    /**  
        Adds sequences to assembly
        Reads got their ids assigned.
    */
    virtual void addReads(const U2DataId& assemblyId, QList<U2AssemblyRead>& rows, U2OpStatus& os);

    /**  Packs assembly rows: assigns packedViewRow value for every read in assembly */
    virtual void pack(const U2DataId& assemblyId, U2OpStatus& os);

private:
    QList<U2AssemblyRead> readRows(SQLiteQuery& q, U2OpStatus& os);
    
    void readRow(U2AssemblyRead& row, SQLiteQuery& q, U2OpStatus& os);

    void createReadsTable(const U2DataId& id, U2OpStatus& os);
    
    void createReadsIndexes(const U2DataId& id, U2OpStatus& os);

    QString getReadsTableName(const U2DataId& id);

    qint64 getMaximumReadLengthInRegion(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os);
    
    void setMaximumReadLengthInRegion(const U2DataId& assemblyId, const U2Region& r, int val, U2OpStatus& os);

    void unpackSequenceAndCigar(qint64 flags, const QByteArray& data, QByteArray& sequence, QByteArray& cigar, U2OpStatus& os);

    QString getReadFields() const;

    /** Symbols used in different alphabets */
    QByteArray dnaAlpha, dnaExtAlpha, cigarAlpha;
    
    /** Positional symbol numbers in alphabets */
    QVector<int> dnaAlphaNums, dnaExtAlphaNums, cigarAlphaNums;
};


} //namespace

#endif
