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

#ifndef _U2_ATTR_DBI_H_
#define _U2_ATTR_DBI_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2Attribute.h>

namespace U2 {

/**
    A configuration for sorting by attribute operation
*/
class U2CORE_EXPORT U2DbiSortConfig {
public:
    U2DbiSortConfig() : ascending(true), ignoreFolder(true) {}

    /** Sort column*/
    QString     sortColumnName;
    
    /** Type of the sort column */
    U2DataType  columnType;
    
    /** Tells  if sorting is ascending or descending */
    bool        ascending;
    
    /** Ignore folder information during sorting. If 'false' only objects from the given folders are sorted and put into result-set*/
    bool        ignoreFolder;
    
    /** Folder to localize sorting  */
    QString     folder;
};

/**
    An interface to obtain access to object attributes
*/
class U2CORE_EXPORT U2AttributeDbi: public U2ChildDbi {
protected:
    U2AttributeDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi){}

public:
    /** Returns all attribute names available in the database */
    virtual QStringList getAvailableAttributeNames(U2OpStatus& os) = 0;
    
    /** Returns all attribute ids for the given object */
    virtual QList<U2DataId> getObjectAttributes(const U2DataId& objectId, U2OpStatus& os) = 0;

    /** Returns all attribute ids for the given object */
    virtual QList<U2DataId> getObjectPairAttributes(const U2DataId& objectId, const U2DataId& childId, U2OpStatus& os) = 0;

    /** Loads int32 attribute by id */
    virtual U2Int32Attribute getInt32Attribute(const U2DataId& attributeId, U2OpStatus& os) = 0;

    /** Loads int64 attribute by id */
    virtual U2Int64Attribute getInt64Attribute(const U2DataId& attributeId, U2OpStatus& os) = 0;

    /** Loads real64 attribute by id */
    virtual U2Real64Attribute getReal64Attribute(const U2DataId& attributeId, U2OpStatus& os) = 0;

    /** Loads String attribute by id */
    virtual U2StringAttribute getStringAttribute(const U2DataId& attributeId, U2OpStatus& os) = 0;

    /** Loads byte attribute by id */
    virtual U2ByteArrayAttribute getByteArrayAttribute(const U2DataId& attributeId, U2OpStatus& os) = 0;
    
    /** Loads date-time attribute by id */
    virtual U2DateTimeAttribute getDateTimeAttribute(const U2DataId& attributeId, U2OpStatus& os) = 0;

    /** Loads range int32-values attribute by id */
    virtual U2RangeInt32StatAttribute getRangeInt32StatAttribute(const U2DataId& attributeId, U2OpStatus& os) = 0;

    /** Loads range real64-values attribute by id */
    virtual U2RangeReal64StatAttribute getRangeReal64StatAttribute(const U2DataId& attributeId, U2OpStatus& os) = 0;

    /** Sorts all objects in database according to U2DbiSortConfig provided  */
    virtual QList<U2DataId> sort(const U2DbiSortConfig& sc, qint64 offset, qint64 count, U2OpStatus& os) = 0;


    /** 
        Removes attribute from database 
        Requires U2DbiFeature_WriteAttribute feature support
    */
    virtual void removeAttribute(const U2DataId& attributeId, U2OpStatus& os) = 0;
    
    /** 
        Creates int32 attribute in database. ObjectId must be already set in attribute and present in the same database 
        Requires U2DbiFeature_WriteAttribute feature support
    */
    virtual void createInt32Attribute(U2Int32Attribute& a, U2OpStatus& os) = 0;

    /** 
        Creates int64 attribute in database. ObjectId must be already set in attribute and present in the same database 
        Requires U2DbiFeature_WriteAttribute feature support
    */    
    virtual void createInt64Attribute(U2Int64Attribute& a, U2OpStatus& os) = 0;

    /** 
        Creates real64 attribute in database. ObjectId must be already set in attribute and present in the same database 
        Requires U2DbiFeature_WriteAttribute feature support
    */    
    virtual void createReal64Attribute(U2Real64Attribute& a, U2OpStatus& os) = 0;

    /** 
        Creates String attribute in database. ObjectId must be already set in attribute and present in the same database 
        Requires U2DbiFeature_WriteAttribute feature support
    */    
    virtual void createStringAttribute(U2StringAttribute& a, U2OpStatus& os) = 0;

    /** 
        Creates Byte attribute in database. ObjectId must be already set in attribute and present in the same database 
        Requires U2DbiFeature_WriteAttribute feature support
    */    
    virtual void createByteArrayAttribute(U2ByteArrayAttribute& a, U2OpStatus& os) = 0;

    /** 
        Creates Date-time attribute in database. ObjectId must be already set in attribute and present in the same database 
        Requires U2DbiFeature_WriteAttribute feature support   
     */    
    virtual void createDateTimeAttribute(U2DateTimeAttribute& a, U2OpStatus& os) = 0;

    /** 
        Creates range int32-values attribute in database. ObjectId must be already set in attribute and present in the same database 
        Requires U2DbiFeature_WriteAttribute feature support
     */    
    virtual void createRangeInt32StatAttribute(U2RangeInt32StatAttribute& a, U2OpStatus& os) = 0;

    /** 
        Creates range real32-values attribute in database. ObjectId must be already set in attribute and present in the same database 
        Requires U2DbiFeature_WriteAttribute feature support
     */    
    virtual void createRangeReal64StatAttribute(U2RangeReal64StatAttribute& a, U2OpStatus& os) = 0;
};


} //namespace

#endif
