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

#ifndef _U2_TYPE_H_
#define _U2_TYPE_H_

#include <U2Core/global.h>

namespace U2 {


/**
    UGENE built-in data-types
    Note: unsigned used to reveal all possible math ops with it.

    U2DataType - data types supported by U2Dbi
    U2DataId - object ids associated with Data by U2Dbi. 
    Note that U2DataId == empty check must be suppoted by DBI to check empty fields.
*/
typedef quint16 U2DataType;
typedef QByteArray U2DataId;

/** 
    ID of the DBI Factory. Defines type of the DBI driver.
    Examples: 'sqlite', 'mysql', 'fasta'....
*/
typedef QString U2DbiFactoryId;

/** 
    ID of the DBI. Points to DBI instance inside of the given DBI factory
    Usually is an URL or the DBI file/resource
*/

typedef QString U2DbiId;




/** 
    Built in types 
    Note: Maximum value used for type must be <=4096
*/

class U2CORE_EXPORT U2Type {
public:
    /** Type is unknown. Default value. */
    static const U2DataType Unknown     = 0;

    /** Object types */
    static const U2DataType Sequence    = 1;
    static const U2DataType Msa         = 2;
    static const U2DataType PhyTree     = 3;
    static const U2DataType Assembly    = 4;
    static const U2DataType SnpTrack    = 5;
    static const U2DataType CrossDatabaseReference    = 999;

    /** SCO (non-object, non-root) types */
    static const U2DataType Annotation              = 1000;
    static const U2DataType AnnotationGroup         = 1001;
    
    /**  Assembly read */
    static const U2DataType AssemblyRead            = 1100;

    /**  MSA */
    static const U2DataType MsaRow                  = 1200;

    /**  Attribute types */
    static const U2DataType AttributeInteger          = 2001;
    static const U2DataType AttributeReal             = 2002;
    static const U2DataType AttributeString           = 2003;
    static const U2DataType AttributeByteArray        = 2004;

    static bool isObjectType(U2DataType type) {return type > 0 && type < 999;}

    static bool isAttributeType(U2DataType type) {return type >=2000 && type < 2100;}

};

/** 
    Cross database data reference
*/
class U2CORE_EXPORT U2DataRef {
public:
    U2DataRef() {}
    U2DataRef(const QString& _dbiId, const U2DataId& _entityId, const U2DbiFactoryId& fid) : dbiId(_dbiId),entityId(_entityId), factoryId(fid){}

    /** database  id */
    QString         dbiId;

    /** DB local data reference */
    U2DataId        entityId;

    /** Object version number this reference is valid for */
    qint64          version;
    
    /** Type of the dbi driver (factory)*/
    U2DbiFactoryId  factoryId;
};

/** 
    Base class for all data types that can be referenced by some ID
*/
class U2CORE_EXPORT U2Entity {
public:
    U2Entity(){}
    U2Entity(U2DataId _id) : id(_id){}
    virtual ~U2Entity(){}

    U2DataId id;
};


/** 
    Base marker class for all First-class-objects stored in the database
*/
class U2CORE_EXPORT U2Object : public U2Entity {
public:
    U2Object() : version(0){}
    U2Object(U2DataId id, const QString& _dbId, qint64 v) : U2Entity(id), dbiId(_dbId), version(v) {}
    
    /** Source of the object: database id */
    QString     dbiId;

    /** Version of the object. Same as modification count of the object */
    qint64      version;

    /** The name of the object shown to user. Any reasonably short text */
    QString     visualName;

    /** The type of the object. Should be overriden in subclasses */
    virtual U2DataType getType() { return U2Type::Unknown; }
};


/** 
    If database keeps annotations/attributes for data entity stored in another database
    U2CrossDatabaseReference used as a parent object for all local data
*/
class U2CORE_EXPORT U2CrossDatabaseReference : public U2Object {
public:
    U2CrossDatabaseReference() {}
    U2CrossDatabaseReference(U2DataId id, QString dbId, qint64 version) : U2Object(id, dbId, version) {}


    // remote data element id;
    U2DataRef   dataRef;

    // implement U2Object
    virtual U2DataType getType() { return U2Type::CrossDatabaseReference; }
};


/** Template class for DBI iterators */
template<class T> class U2DbiIterator {
public:
    virtual ~U2DbiIterator(){}

    /** returns true if there are more reads to iterate*/
    virtual bool hasNext() = 0;

    /** returns next read and shifts one element*/
    virtual T next() = 0;

    /** returns next read without shifting*/
    virtual T peek() = 0;
};

} //namespace

#endif
