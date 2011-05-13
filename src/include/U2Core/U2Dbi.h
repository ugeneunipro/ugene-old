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

#ifndef _U2_DBI_H_
#define _U2_DBI_H_

#include "U2Type.h"

#include <U2Core/U2Annotation.h>
#include <U2Core/U2Assembly.h>
#include <U2Core/U2Attribute.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2Sequence.h>

#include <QtCore/QHash>
#include <QtCore/QSet>

namespace U2 {

// For the classes below, see description in class definition
class U2ObjectDbi;
class U2SequenceDbi;
class U2AnnotationDbi;
class U2CrossDatabaseReferenceDbi;
class U2MsaDbi;
class U2AssemblyDbi;
class U2AttributeDbi;
class U2OpStatus;
class U2Dbi;

/** A constant to retrieve all available data. */
#define U2_DBI_NO_LIMIT -1

/** Init time DBI parameter name to specify URL of the database */
#define U2_DBI_OPTION_URL       "url"

/** Init time DBI parameter name to specify that database must be created if not exists */
#define U2_DBI_OPTION_CREATE    "create"

/** Init time DBI parameter value. Indicates boolean 'Yes' or 'true'. */
#define U2_DBI_VALUE_ON "1"


/**
    Operational state of the database.
*/
enum U2CORE_EXPORT U2DbiState {
    U2DbiState_Void = 1,
    U2DbiState_Starting = 2,
    U2DbiState_Ready = 3,
    U2DbiState_Stopping = 4
};

/**
    DBI feature flags
*/
enum U2CORE_EXPORT U2DbiFeature {
    
    /** DBI supports sequence reading methods */
    U2DbiFeature_ReadSequence                 = 1,
    /** DBI supports MSA reading methods */
    U2DbiFeature_ReadMsa                      = 2,
    /** DBI supports Assembly reading methods */
    U2DbiFeature_ReadAssembly                 = 3,
    /** DBI supports sequence annotations reading methods */
    U2DbiFeature_ReadSequenceAnnotations      = 4,
    /** DBI supports read methods for attributes */
    U2DbiFeature_ReadAttributes               = 5,
    /** DBI supports read methods for remote objects  */
    U2DbiFeature_ReadCrossDatabaseReferences  = 6,

    /** DBI supports changing/storing sequences */
    U2DbiFeature_WriteSequence                = 101,
    /** DBI supports changing/storing multiple sequence alignments */
    U2DbiFeature_WriteMsa                     = 102,
    /** DBI supports changing/storing assemblies */
    U2DbiFeature_WriteAssembly                = 103
    /** DBI supports changing/storing sequence annotations*/,
    U2DbiFeature_WriteSequenceAnnotations     = 104,
    /** DBI supports changing/storing attributes */
    U2DbiFeature_WriteAttributes              = 105,
    /** DBI supports cross database references */
    U2DbiFeature_WriteCrossDatabaseReferences = 106,

    /** DBI supports removal of objects */
    U2DbiFeature_RemoveObjects                = 200,
    /** DBI supports set folder modification operations */
    U2DbiFeature_ChangeFolders                = 201,
    
    /** DBI provides optimized algorithm for assembly reads packing */
    U2DbiFeature_AssemblyReadsPacking         = 300,
    /** DBI provides optimized algorithm for coverage calculation */
    U2DbiFeature_AssemblyCoverageStat         = 301,

    /** DBI provides optimized algorithm for sorting attributes */
    U2DbiFeature_AttributeSorting             = 400,

};


/** 
    DBI factory provides functions to create new DBI instances
    and check file content to ensure that file is a valid database file
*/
class U2DbiFactory {
public:
    U2DbiFactory() {}
    virtual ~U2DbiFactory(){};

    /** Creates new DBI instance */
    virtual U2Dbi *createDbi() = 0;

    /** Returns DBI type ID */
    virtual U2DbiFactoryId getId() const = 0;

    /** 
        Checks that data pointed by properties is a valid DBI resource 
        rawData param is used for compatibility with UGENE 1.x format detection 
        and can be used by factory directly to check database header
    */
    virtual bool isValidDbi(const QHash<QString, QString>& properties, const QByteArray& rawData, U2OpStatus& os) const = 0;
};


/**
    Database access interface. 
    Database examples: fasta file, genbank file, BAM file, SQLite file
*/
class U2CORE_EXPORT U2Dbi {
public:
    virtual ~U2Dbi(){}
    //////////////////////////////////////////////////////////////////////////
    // base methods that any DBI must support


    /** 
        Boots the database up to functional state. 
        Some property names are reserved:
            'url' - url of the DBI
            'create'  - '1' or '0'- if DBI does not exist, asks to create instance if possible
            'user', 'password' - user and password to access to the DBI
        The 'persistentData' parameter provides database state saved from previous session in a project or workspace, if any.

        @see shutdown()
    */
    virtual void init(const QHash<QString, QString>& properties, const QVariantMap& persistentData, U2OpStatus& os) = 0;

    /** Stops the database and frees up used resources. 
    Returns persistent database state for external storage in a project or workspace, if any. 
    For example, plain-file-based DBI can store certain settings or preferences between sessions, 
    which may not fit into particular file format.

    Note: this method will call flush()
    @see init()
    */
    virtual QVariantMap shutdown(U2OpStatus& os) = 0;

    /** 
        Ensures that dbi state is synchronized with storage 
        Return 'true' of operation is successful
    */
    virtual bool flush(U2OpStatus& os) = 0;
    
    /**  Unique database id. Usually is an URL of the database */
    virtual U2DbiId getDbiId() const = 0;

    /** Return factory instance for this DBI */
    virtual U2DbiFactoryId getFactoryId() const  = 0;

    /** Returns all features supported by this DBI instance */
    virtual const QSet<U2DbiFeature>& getFeatures() const = 0;

    /** Returns properties used to initialized the database */
    virtual QHash<QString, QString> getInitProperties() const = 0;

    /** Returns database meta-info. Any set of properties to be shown to user */
    virtual QHash<QString, QString> getDbiMetaInfo(U2OpStatus&) = 0;

    /** Returns type of the entity referenced by the given ID */
    virtual U2DataType getEntityTypeById(const U2DataId& id) const = 0;

    /** Returns current DBI state */
    virtual U2DbiState getState() const = 0;

    /** 
        Database interface to access objects
        All dbi implementation must support a subset of this interface
    */
    virtual U2ObjectDbi* getObjectDbi() = 0;

    /**  
        U2Sequence related DBI routines 
        Not NULL only if U2DbiFeature_ReadSequences supported
    */
    virtual U2SequenceDbi* getSequenceDbi() = 0;

    /**  
        U2Annotation related DBI routines 
        Not NULL only if U2DbiFeature_ReadAnnotations supported
    */
    virtual U2AnnotationDbi* getAnnotationRDbi() = 0;

    /**  
        U2Annotation related DBI routines 
        Not NULL only if U2DbiFeature_ReadMsa supported
    */
    virtual U2MsaDbi* getMsaDbi() = 0;

    /**  
        U2Annotation related DBI routines 
        Not NULL only if U2DbiFeature_ReadAssembly supported
    */
    virtual U2AssemblyDbi* getAssemblyDbi()  = 0;

    /**  
        U2Attribute related DBI routines 
        Not NULL only if U2DbiFeature_ReadAttributes supported
    */
    virtual U2AttributeDbi* getAttributeDbi()  = 0;

    /** 
        Cross database references handling routines 
        Not NULL only if U2DbiFeature_ReadCrossDatabaseReferences supported
    */
    virtual U2CrossDatabaseReferenceDbi* getCrossDatabaseReferenceDbi() = 0;
};

/** 
    Base class for all *Dbi classes provided by U2Dbi
    Contains reference to root-level dbi
*/
class U2CORE_EXPORT U2ChildDbi {
protected:
    U2ChildDbi(U2Dbi* _rootDbi) : rootDbi (_rootDbi){}
    
    virtual ~U2ChildDbi(){}

public:
    U2Dbi* getRootDbi() const { return rootDbi; }

private:
    U2Dbi* rootDbi;
};

} //namespace

#endif
