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
#include <U2Core/U2SqlHelpers.h>

#include "assembly/AssemblyPackAlgorithm.h"

namespace U2 {

class SQLiteQuery;
class AssemblyAdapter;

class SQLiteAssemblyDbi : public U2AssemblyDbi, public SQLiteChildDBICommon {

public:
    SQLiteAssemblyDbi(SQLiteDbi* dbi);
    ~SQLiteAssemblyDbi();

    /** Reads assembly objects by id */
    virtual U2Assembly getAssemblyObject(const U2DataId& assemblyId, U2OpStatus& os);

    /** 
        Return number of reads in assembly that intersect given region 
        'Intersect' here means that region(leftmost pos, rightmost pos) intersects with 'r'
    */
    virtual qint64 countReads(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os);

    /** 
        Return reads that intersect given region
        Note: iterator instance must be deallocated by caller method
    */
    virtual U2DbiIterator<U2AssemblyRead>* getReads(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os);

    /** 
        Return reads with packed row value >= min, <= max that intersect given region 
        Note: iterator instance must be deallocated by caller method
    */
    virtual U2DbiIterator<U2AssemblyRead>* getReadsByRow(const U2DataId& assemblyId, const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os);

    /** 
        Return reads with a specified name. Used to find paired reads that must have equal names 
        Note: iterator instance must be deallocated by caller method
    */
    virtual U2DbiIterator<U2AssemblyRead>* getReadsByName(const U2DataId& assemblyId, const QByteArray& name, U2OpStatus& os);


    /** 
        Return max packed row at the given coordinate
        'Intersect' here means that region(leftmost pos, rightmost pos) intersects with 'r'
    */
    virtual qint64 getMaxPackedRow(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os);

    /** Count 'length of assembly' - position of the rightmost base of all reads */
    virtual qint64 getMaxEndPos(const U2DataId& assemblyId, U2OpStatus& os);


    /** 
        Creates new empty assembly object. Reads iterator can be NULL 
        Requires: U2DbiFeature_WriteAssembly feature support
    */
    virtual void createAssemblyObject(U2Assembly& assembly, const QString& folder,  U2DbiIterator<U2AssemblyRead>* it, U2OpStatus& os);

    /** 
        Updates assembly object fields 
        Requires: U2DbiFeature_WriteAssembly feature support
    */
    virtual void updateAssemblyObject(U2Assembly&, U2OpStatus& os);


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

    virtual void initSqlSchema(U2OpStatus& os);
    virtual void shutdown(U2OpStatus& os);

private:
    /** Return assembly storage adapter for the given assembly */
    AssemblyAdapter* getAdapter(const U2DataId& assemblyId, U2OpStatus& os);
    
    /** Adapters by database assembly id */
    QHash<qint64, AssemblyAdapter*> adaptersById;
};

// reserved for future use;
class AssemblyCompressor {
};


class AssemblyAdapter {
public:
    AssemblyAdapter(const U2DataId& assemblyId, const AssemblyCompressor* compressor, DbRef* ref);
    
    virtual void createReadsTables(U2OpStatus& os) {};
    virtual void createReadsIndexes(U2OpStatus& ) {};
    virtual void shutdown(U2OpStatus& ) {};

    virtual qint64 countReads(const U2Region& r, U2OpStatus& os) = 0;

    virtual qint64 getMaxPackedRow(const U2Region& r, U2OpStatus& os) = 0;
    virtual qint64 getMaxEndPos(U2OpStatus& os) = 0;

    virtual U2DbiIterator<U2AssemblyRead>* getReads(const U2Region& r, U2OpStatus& os) = 0;
    virtual U2DbiIterator<U2AssemblyRead>* getReadsByRow(const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os) = 0;
    virtual U2DbiIterator<U2AssemblyRead>* getReadsByName(const QByteArray& name, U2OpStatus& os) = 0;
    
    virtual void addReads(QList<U2AssemblyRead>& rows, U2OpStatus& os) = 0;
    virtual void addReadsInternal(QList<U2AssemblyRead>& reads, bool delayedIndex, U2OpStatus& os);
    virtual void removeReads(const QList<U2DataId>& rowIds, U2OpStatus& os) = 0;

    virtual void pack(U2OpStatus& os) = 0;

    virtual int getInsertGroupSize() const {return 100*1000;}

protected:
    U2DataId                    assemblyId;
    const AssemblyCompressor*   compressor;
    DbRef*                      db;
};


/** This bit indicates that DNA Ext alphabet is used*/
#define BIT_EXT_DNA_ALPHABET        0

/** This bit indicates that read is located on complement thread */
#define BIT_COMPLEMENTARY_STRAND    1

/** This bit indicates that read is a paired read */
#define BIT_PAIRED_READ             2

/** Compression method for assembly data */
enum SQLiteAssemblyDataMethod {
    /** Merges Name, Sequence, Cigar and Quality values into single byte array separated by '\n' character. Merge prefix is '0'*/
    SQLiteAssemblyDataMethod_NSCQ = 1
};

class SQLiteAssemblyUtils {
public:
    static bool isExtendedAlphabet(qint64 flags) {
        return flags & (1 << BIT_EXT_DNA_ALPHABET);
    }

    static bool isComplementaryRead(qint64 flags) {
        return flags & (1 << BIT_COMPLEMENTARY_STRAND);
    }

    static bool isPairedRead(qint64 flags) {
        return flags & (1 << BIT_PAIRED_READ);
    }

    static QByteArray packData(SQLiteAssemblyDataMethod method, const QByteArray& name, const QByteArray& seq, 
        const QByteArray& cigarText, const QByteArray& qualityString, U2OpStatus& os);
    
    static void unpackData(const QByteArray& packed, QByteArray& name, QByteArray& sequence, QByteArray& cigarText, QByteArray& qualityString, U2OpStatus& os);
};

class SQLiteAssemblyNameFilter : public SqlRSFilter<U2AssemblyRead> {
public:
    SQLiteAssemblyNameFilter(const QByteArray& expectedName) : name (expectedName) {}
    virtual bool filter(const U2AssemblyRead& r)  {return name == r->name;}
protected:
    QByteArray name;

};

class SimpleAssemblyReadLoader: public SqlRSLoader<U2AssemblyRead> {
public:
    U2AssemblyRead load(SQLiteQuery* q);
};

class SimpleAssemblyReadPackedDataLoader : public SqlRSLoader<PackAlgorithmData> {
public:
    virtual PackAlgorithmData load(SQLiteQuery* q);
};



} //namespace

#endif
