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

    /** DBI supports cross database references */
    U2DbiFeature_WriteCrossDatabaseReferences = 100,
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

    /** DBI supports removal of objects */
    U2DbiFeature_RemoveObjects                = 200,
    /** DBI supports set folder modification operations */
    U2DbiFeature_ChangeFolders                = 201,
    
    /** DBI provides optimized algorithm for assembly reads packing */
    U2DbiFeature_AssemblyReadsPacking         = 300,
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
};

/** 
    Base class for all *Dbi classes provided by U2Dbi
    Contains reference to root-level dbi
*/
class U2CORE_EXPORT U2ChildDbi {
protected:
    U2ChildDbi(U2Dbi* _rootDbi) : rootDbi (_rootDbi){}
    
    virtual ~U2ChildDbi(){}

    U2Dbi* getRootDbi() const { return rootDbi; }

private:
    U2Dbi* rootDbi;
};


/**
    An primary interface to access database content
    Any database contains objects that are placed into folders
    Folders have unique string IDs - constructed similar to full folders names on Unix systems
    The root folder "/" is required for any DBI
*/
class U2CORE_EXPORT U2ObjectDbi : public U2ChildDbi {
protected:
    U2ObjectDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi) {}

public:
    
    /**  Returns number of top-level U2Objects in database */
    virtual qint64 countObjects(U2OpStatus& os) = 0;

    /**  Returns number of top-level U2Objects with the specified type in database */
    virtual qint64 countObjects(U2DataType type, U2OpStatus& os) = 0;

    /** Lists database top-level objects, starts with 'offset' and limits by 'count'. 
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getObjects(qint64 offset, qint64 count, U2OpStatus& os) = 0;

    /** Lists database top-level objects of the specified type, starts with 'offset' and limits by 'count'. 
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getObjects(U2DataType type, qint64 offset, qint64 count, U2OpStatus& os) = 0;

    /**  Returns parents for the entity.
        If entity is object, returns other object this object is a part of
        If object is not a part of any other object and does not belongs to any folder - it's automatically removed.
     */
    virtual QList<U2DataId> getParents(const U2DataId& entityId, U2OpStatus& os) = 0;


    /**  Returns list of folders stored in database. 
        Folders are represented as paths, separated by '/' character.
        At least one root folder is required. 
    */
    virtual QStringList getFolders(U2OpStatus& os) = 0;

    /** Returns version of the folder. 
        The folder version increases if new object(s)/subfolder(s) are added into this folder
        Note that if object(s)/folder(s) are added into one of the subfolders the folder version is not changed
    */
    virtual qint64 getFolderLocalVersion(const QString& folder, U2OpStatus& os) = 0;

    /** Returns version of the folder that changes every time object(s)/folder(s) added 
        to the specified folder or any of its child folders
    */
    virtual qint64 getFolderGlobalVersion(const QString& folder, U2OpStatus& os) = 0;

    /** Returns number of top-level U2Objects in folder */
    virtual qint64 countObjects(const QString& folder, U2OpStatus& os) = 0;

    /** Lists database top-level objects of the specified type, starts with 'offset' and limits by 'count'. 
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getObjects(const QString& folder, qint64 offset, qint64 count, U2OpStatus& os) = 0;

    /**  Returns all folders this object must be shown in  */
    virtual QStringList getObjectFolders(const U2DataId& objectId, U2OpStatus& os) = 0;

    /** Returns version of the given object */
    virtual qint64 getObjectVersion(const U2DataId& objectId, U2OpStatus& os) = 0;

    /** 
        Removes object from the specified folder. If folder is empty - removes object from all folders.
        Note: the object & all related data is automatically removed from database when
        object is not placed in any folder or is not a part of any other more complex object (ex: sequence in msa)
        Requires: U2DbiFeature_RemoveObjects feature support
    */
    virtual void removeObject(const U2DataId& dataId, const QString& folder, U2OpStatus& os) = 0;
    
    /** 
        Removes collection of objects from the specified folder. If folder is empty - removes object from all folders.
        Note: the object & all related data is automatically removed from database when
        object is not placed in any folder or is not a part of any other more complex object (ex: sequence in msa)
        Requires: U2DbiFeature_RemoveObjects feature support
    */
    virtual void removeObjects(const QList<U2DataId>& dataIds, const QString& folder, U2OpStatus& os) = 0;


    /** Creates folder in the database.
        The specified path must be a valid unique path, not existing in the database.
        It is not required that parent folders must exist, they are created automatically.
        Requires: U2DbiFeature_ChangeFolders feature support
    */
    virtual void createFolder(const QString& path, U2OpStatus& os) = 0;

    /** 
        Removes folder. The folder must be existing path. Runs GC check for all objects in the folder 
        Requires: U2DbiFeature_ChangeFolders feature support
    */
    virtual void removeFolder(const QString& folder, U2OpStatus& os) = 0;

    /** Adds objects to the specified folder.
        All objects must exist and have a top-level type.
        Requires: U2DbiFeature_ChangeFolders feature support
    */
    virtual void addObjectsToFolder(const QList<U2DataId>& objectIds, const QString& toFolder, U2OpStatus& os) = 0;

    /** Moves objects between folders.
        'fromFolder' must be existing path containing all specified objects.
        'toFolder' must be existing path or empty string.
        If 'toFolder' is empty, removes the objects from 'fromFolder' and 
        deletes non-top-level objects without parents, if any appear in the specified list.
        Otherwise, moves the specified objects between the specified folders, omitting duplicates.
        Requires: U2DbiFeature_ChangeFolders feature support
    */
    virtual void moveObjects(const QList<U2DataId>& objectIds, const QString& fromFolder, const QString& toFolder, U2OpStatus& os) = 0;
};

/**
    An interface to access to sequence objects
*/
class U2CORE_EXPORT U2SequenceDbi : public U2ChildDbi {
protected:
    U2SequenceDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi){}

public:
    /** Reads sequence object from database */
    virtual U2Sequence getSequenceObject(const U2DataId& sequenceId, U2OpStatus& os) = 0;
    
    /**  
    Reads specified sequence data region from database.
    The region must be valid region within sequence bounds.
    */
    virtual QByteArray getSequenceData(const U2DataId& sequenceId, const U2Region& region, U2OpStatus& os) = 0;

    /**  Adds new (empty) sequence instance into database, sets the assigned id on the passed U2Sequence instance. 
        The folder must exist in the database.
        Use 'updateSequenceData' method to supply data to the created sequence.

        Requires: U2DbiFeature_WriteSequence feature support
    */
    virtual void createSequenceObject(U2Sequence& sequence, const QString& folder, U2OpStatus& os) = 0;

    /** 
        Updates sequence region. 
        The region must be valid region within sequence bounds.
        Note: regionToReplace length can differ from dataToInsert length, so the method can be used to add/remove sequence regions.

        //TODO think about annotations: should we fix locations automatically?? If yes, emit notifications??
        // varlax: I think this should be left to user, no automatic fixes.

        Requires: U2DbiFeature_WriteSequence feature support
    */
    virtual void updateSequenceData(const U2DataId& sequenceId, const U2Region& regionToReplace, const QByteArray& dataToInsert, U2OpStatus& os) = 0;
};


/**
    An interface to obtain 'read' access to sequence annotations
*/
class U2CORE_EXPORT U2AnnotationDbi : public U2ChildDbi {
protected:
    U2AnnotationDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi){}

public:
    /** 
        Returns number of annotations for the given sequence object  that belongs to the group specified
        If group name is empty - all annotations are counted
    */
    virtual qint64 countAnnotations(const U2DataId& sequenceId, const QString& group,  U2OpStatus& os) = 0;
    
    /** 
        Returns number of annotations for the given sequence object in the given region.
        Counts all annotations whose location intersects the region.
    */
    virtual qint64 countAnnotations(const U2DataId& sequenceId, const U2Region& region, U2OpStatus& os) = 0;

    /** 
        Returns annotations for the given sequence object that belongs to the group specified
        If group is empty searches globally in all groups
        Orders result by qualifier if not empty
        The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. 
    */
    virtual QList<U2DataId> getAnnotations(const U2DataId& sequenceId, const QString& group, const QString& orderByQualifier,
                                            qint64 offset, qint64 count, U2OpStatus& os) = 0;
    
    /** 
        Returns annotations for the given sequence object in the given region.
        Counts all annotations whose location intersects the region.
        Orders result by qualifier if not empty.
        The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. 
    */
    virtual QList<U2DataId> getAnnotations(const U2DataId& sequenceId, const U2Region& region, const QString& orderByQualifier, 
                                            qint64 offset, qint64 count, U2OpStatus& os) = 0;

    /** Reads annotation entity by id */
    virtual U2Annotation getAnnotation(const U2DataId& annotationId, U2OpStatus& os) = 0;


    /** 
        Adds new annotation. Assigns Id to annotation 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void createAnnotation(U2Annotation& a, U2OpStatus& os) = 0;

    /** 
        Adds list of new annotations. Assigns Ids to annotations added 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void createAnnotations(QList<U2Annotation>& annotations, U2OpStatus& os) = 0;

    
    /** 
        Removes annotation from database 
        Requires: U2DbiFeature_WriteAnnotation feature support
     */
    virtual void removeAnnotation(const U2DataId& annotationId, U2OpStatus& os) = 0;

    /**  
        Removes annotations from database  
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void removeAnnotations(const QList<U2DataId>& annotationIds, U2OpStatus& os) = 0;
    
    /** 
        Changes annotations location 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void updateLocation(const U2DataId& annotationId, const U2Location& location, U2OpStatus& os) = 0;  
    
    /** 
        Changes annotations name 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void updateName(const U2DataId& annotationId, const QString& newName, U2OpStatus& os) = 0;  
    
    /** 
        Adds new qualifier to annotation  
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void createQualifier(const U2DataId& annotationId, const U2Qualifier& q, U2OpStatus& os) = 0;
    
    /** 
        Removes existing qualifier from annotation  
        Requires: U2DbiFeature_WriteAnnotation feature support
     */    
    virtual void removeQualifier(const U2DataId& annotationId, const U2Qualifier& q, U2OpStatus& os) = 0; 

    /** 
        Adds annotation to the specified group 
        Requires: U2DbiFeature_WriteAnnotation feature support
    */    
    virtual void addToGroup(const U2DataId& annotationId, const QString& group, U2OpStatus& os) = 0; 
    
    /** 
        Removes annotation from the specified group 
        If annotation belongs to no group, it is removed
        Requires: U2DbiFeature_WriteAnnotation feature support
    */
    virtual void removeFromGroup(const U2DataId& annotationId, const QString& group, U2OpStatus& os) = 0; 
};


/**
    An interface to obtain access to multiple sequence alignment
*/
class U2CORE_EXPORT U2MsaDbi : public U2ChildDbi {
protected:
    U2MsaDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi) {} 

public:
    /** Reads Msa objects by id */
    virtual U2Msa getMsaObject(const U2DataId& id, U2OpStatus& os) = 0;

    /** Returns number of sequences in MSA*/
    virtual qint64 getSequencesCount(const U2DataId& msaId, U2OpStatus& os) = 0;

    /** Returns region of Msa rows. Total number of MSA rows is equal to number of sequences in MSA.
    The 'firstRow' and 'numRows' must specify valid subset of rows in the alignment. */
    virtual QList<U2MsaRow> getRows(const U2DataId& msaId, qint32 firstRow, qint32 numRows, U2OpStatus& os) = 0;
    
    /** 
        Return number of sequences in alignment that intersect given coord.
        'Intersect' here means that first non gap character is <= coord <= last non gap character.
        The coord should be a valid coordinate within alignment bounds, i.e. non-negative and less than alignment length.
    */
    virtual qint32 countSequencesAt(const U2DataId& msaId, qint64 coord, U2OpStatus& os) = 0;

    /** Return 'count' sequences starting with 'offset' that intersect given coordinate.
    The coord should be a valid coordinate within alignment bounds, i.e. non-negative and less than alignment length.
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getSequencesAt(const U2DataId& msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os) = 0;
    
    /** Return number of sequences in alignment that intersect given region.
    The region should be a valid region within alignment bounds, i.e. non-negative and less than alignment length.
        'Intersect' here means that first non gap character is <= coord <= last non gap character
    */
    virtual qint32 countSequencesAt(const U2DataId& msaId, const U2Region& r, U2OpStatus& os) = 0;

    /** Return 'count' sequences starting with 'offset' that intersect given region.
    The region should be a valid region within alignment bounds, i.e. non-negative and less than alignment length.
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getSequencesAt(const U2DataId& msaId, const U2Region& r, qint32 offset, qint32 count, U2OpStatus& os) = 0;

    /** Return number of sequences in alignment that that have non-gap character at the given coord.
    The coord should be a valid coordinate within alignment bounds, i.e. non-negative and less than alignment length.*/
    virtual qint32 countSequencesWithoutGapAt(const U2DataId& msaId, qint64 coord, U2OpStatus& os) = 0;
    
    /** Return 'count' sequence starting with 'offset' alignment that that have non-gap character at the given coord.
    The coord should be a valid coordinate within alignment bounds, i.e. non-negative and less than alignment length.
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getSequencesWithoutGapAt(const U2DataId& msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os) = 0;

    /** 
        Creates new empty Msa object  
        Requires: U2DbiFeature_WriteMsa feature support
    */
    virtual void createMsaObject(U2Msa& msa, const QString& folder, U2OpStatus& os) = 0;
    
    /** 
        Removes sequences from MSA
        Automatically removes affected sequences that are not anymore located in some folder nor Msa object
        Requires: U2DbiFeature_WriteMsa feature support
    */
    virtual void removeSequences(U2Msa& msa, const QList<U2DataId> sequenceIds, U2OpStatus& os) = 0;

    /**  
        Adds sequences to MSA 
        Requires: U2DbiFeature_WriteMsa feature support
    */
    virtual void addSequences(U2Msa& msa, const QList<U2MsaRow>& rows, U2OpStatus& os) = 0;

};

/** Class used to iterate huge amount of assembly reads and optimize assembly import operation to DBI */
class U2AssemblyReadsIterator {
public:
    /** returns true if there are more reads to iterate*/
    virtual bool hasNext() = 0;

    /** returns next read or error */
    virtual U2AssemblyRead next(U2OpStatus& os) = 0;
};

/** Template class for DBI iterators */
template<class T> class U2DbiIterator {
public:
    virtual ~U2DbiIterator(){}

    /** returns true if there are more reads to iterate*/
    virtual bool hasNext() = 0;

    /** returns next read and shifts one element*/
    virtual const T& next() = 0;

    /** returns next read without shifting*/
    virtual const T& peek() = 0;
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
