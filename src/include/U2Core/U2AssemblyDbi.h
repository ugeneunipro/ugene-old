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

#ifndef _U2_ASSEMBLY_DBI_H_
#define _U2_ASSEMBLY_DBI_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2Assembly.h>

namespace U2 {

/** Class used to iterate huge amount of assembly reads and optimize assembly import operation to DBI */
class U2AssemblyReadsIterator {
public:
    /** returns true if there are more reads to iterate*/
    virtual bool hasNext() = 0;

    /** returns next read or error */
    virtual U2AssemblyRead next(U2OpStatus& os) = 0;
};


/**
    An interface to obtain  access to assembly data
*/
class U2CORE_EXPORT U2AssemblyDbi : public U2ChildDbi {
protected:
    U2AssemblyDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi) {} 

public:
    /** Reads assembly objects by id */
    virtual U2Assembly getAssemblyObject(const U2DataId& id, U2OpStatus& os) = 0;

    /** 
        Return number of reads in assembly that are located near or intersect the region.
        The region should be a valid region within alignment bounds, i.e. non-negative and less than alignment length.
        
        Note: 'near' here means that DBI is not forced to return precise number of reads that intersects the region
        and some deviations is allowed in order to apply performance optimizations. 

    */
    virtual qint64 countReads(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) = 0;

    /** 
        Return reads that intersect given region
        Note: iterator instance must be deallocated by caller method
    */
    virtual U2DbiIterator<U2AssemblyRead>* getReads(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) = 0;

    /** 
        Return reads with packed row value >= min, <= max that intersect given region 
        Note: iterator instance must be deallocated by caller method
    */
    virtual U2DbiIterator<U2AssemblyRead>* getReadsByRow(const U2DataId& assemblyId, const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os) = 0;

    /** 
        Return reads with a specified name. Used to find paired reads that must have equal names 
        Note: iterator instance must be deallocated by caller method
    */
    virtual U2DbiIterator<U2AssemblyRead>* getReadsByName(const U2DataId& assemblyId, const QByteArray& name, U2OpStatus& os) = 0;

    /** 
        Return max packed row at the given coordinate
        'Intersect' here means that region(leftmost pos, rightmost pos) intersects with 'r'
    */
    virtual qint64 getMaxPackedRow(const U2DataId& assemblyId, const U2Region& r, U2OpStatus& os) = 0;


    /** Count 'length of assembly' - position of the rightmost base of all reads */
    virtual quint64 getMaxEndPos(const U2DataId& assemblyId, U2OpStatus& os) = 0;


    /** 
        Creates new empty assembly object. Reads iterator can be NULL 
        Requires: U2DbiFeature_WriteAssembly feature support
    */
    virtual void createAssemblyObject(U2Assembly& assembly, const QString& folder,  U2DbiIterator<U2AssemblyRead>* it, U2OpStatus& os) = 0;

    /** 
        Updates assembly object fields 
        Requires: U2DbiFeature_WriteAssembly feature support
    */
    virtual void updateAssemblyObject(U2Assembly& assembly, U2OpStatus& os) = 0;

    /** 
        Removes sequences from assembly
        Automatically removes affected sequences that are not anymore accessible from folders
        Requires: U2DbiFeature_WriteAssembly feature support
    */
    virtual void removeReads(const U2DataId& assemblyId, const QList<U2DataId>& rowIds, U2OpStatus& os) = 0;

    /**  
        Adds sequences to assembly
        Reads got their ids assigned.
        Requires: U2DbiFeature_WriteAssembly feature support
    */
    virtual void addReads(const U2DataId& assemblyId, QList<U2AssemblyRead>& rows, U2OpStatus& os) = 0;

    /**  
        Packs assembly rows: assigns packedViewRow value for every read in assembly 
        Requires: U2DbiFeature_WriteAssembly and U2DbiFeature_AssemblyReadsPacking features support
    */
    virtual void pack(const U2DataId& assemblyId, U2OpStatus& os) = 0;

};
    
} //namespace

#endif
