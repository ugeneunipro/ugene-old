#ifndef _U2_DBI_H_
#define _U2_DBI_H_

#include "U2Type.h"

#include <U2Core/U2Annotation.h>
#include <U2Core/U2Assembly.h>
#include <U2Core/U2Attribute.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2Sequence.h>

#include <QtCore/QHash>

namespace U2 {

// For the classes below, see description in class definition
class U2ObjectRDbi;
class U2ObjectRWDbi;
class U2FolderDbi;
class U2SequenceRDbi;
class U2SequenceRWDbi;
class U2AnnotationRDbi;
class U2AnnotationRWDbi;
class U2MsaRDbi;
class U2MsaRWDbi;
class U2AssemblyRDbi;
class U2AssemblyRWDbi;
class U2AttributeRDbi;
class U2AttributeRWDbi;
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
    
    /** 
        Unique database id. Used for cross-database references. 
        Usually is an URL of the database;
    */
    virtual QString getDbiId() const = 0;

    /** Returns properties used to initialized the database */
    virtual QHash<QString, QString> getInitProperties() const = 0;

    /** Returns database meta-info. Any set of properties to be shown to user */
    virtual QHash<QString, QString> getDbiMetaInfo(U2OpStatus&) = 0;

    /** Returns type of the entity referenced by the given ID */
    virtual U2DataType getEntityTypeById(U2DataId id) const = 0;

    /** Returns current DBI state */
    virtual U2DbiState getState() const = 0;

    /** Database interface to access objects. Must not be NULL! */
    virtual U2ObjectRDbi* getObjectRDbi() = 0;

    /**  Generic database interface to remove objects from database */
    virtual U2ObjectRWDbi* getObjectRWDbi() {return NULL;}

    /** Database interface to modify folder information. */
    virtual U2FolderDbi* getFolderDbi() const {return NULL;};

    /**  U2Sequence read ops. Not null if DBI supports the whole set of sequence reading operations */
    virtual U2SequenceRDbi* getSequenceRDbi() {return NULL;}

    /** U2Sequence read and write ops. Not null if DBI supports the whole set of sequence writing operations */
    virtual U2SequenceRWDbi* getSequenceRWDbi() {return NULL;}

    /**  U2Annotation read ops. Not null if DBI supports the whole set of annotation reading operations */
    virtual U2AnnotationRDbi* getAnnotationRDbi() {return NULL;}

    /** U2Annotation write ops. Not null if DBI supports the whole set of annotation writing operations */
    virtual U2AnnotationRWDbi* getAnnotationRWDbi() {return NULL;}

    /** 
        U2Msa read ops. Not null if DBI supports the whole set of msa reading operations
        U2MsaRDbi requires U2SequenceRDbi support
    */
    virtual U2MsaRDbi* getMsaRDbi() {return NULL;}

    /** 
        U2Msa read ops. Not null if DBI supports the whole set of msa reading operations
        U2MsaRWDbi requires U2SequenceRWDbi support
    */        
    virtual U2MsaRWDbi* getMsaRWDbi() {return NULL;}

    /** 
        U2Assembly read ops. Not null if DBI supports the whole set of assembly reading operations
        U2AssemblyRDbi requires U2SequenceRDbi support
    */
    virtual U2AssemblyRDbi* getAssemblyRDbi() {return NULL;}

    /** 
        U2Msa read ops. Not null if DBI supports the whole set of msa reading operations
        Support of U2MsaRWDbi requires U2SequenceRWDbi support
    */        
    virtual U2AssemblyRWDbi* getAssemblyRWDbi() {return NULL;}


    /**  U2Attribute read ops. Not null if DBI supports the whole set of attributes reading operations */
    virtual U2AttributeRDbi* getAttributeRDbi() {return NULL;}

    /**  U2Annotation write ops. Not null if DBI supports the whole set of attribute writing operations */
    virtual U2AttributeRWDbi* getAttributeRWDbi() {return NULL;}

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
class U2CORE_EXPORT U2ObjectRDbi : public U2ChildDbi {
protected:
    U2ObjectRDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi) {}

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

    /**  Returns parents for entity.
        If entity is object, returns other object this object is a part of
        If object is not a part of any other object and does not belongs to any folder - it's automatically removed.
     */
    virtual QList<U2DataId> getParents(U2DataId entityId, U2OpStatus& os) = 0;


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
    virtual QStringList getObjectFolders(U2DataId objectId, U2OpStatus& os) = 0;

    /** Returns version of the given object */
    virtual qint64 getObjectVersion(U2DataId objectId, U2OpStatus& os) = 0;
};

/**
    An interface used to add/remove objects from database
*/
class U2CORE_EXPORT U2ObjectRWDbi : public U2ObjectRDbi {
protected:
    U2ObjectRWDbi(U2Dbi* rootDbi) : U2ObjectRDbi(rootDbi) {}

public:
    
    /** 
        Removes object from the specified folder. If folder is empty - removes object from all folders.
        Note: the object & all related data is automatically removed from database when
        object is not placed in any folder or is not a part of any other more complex object (ex: sequence in msa)
    */
    virtual void removeObject(U2DataId dataId, const QString& folder, U2OpStatus& os) = 0;
    
    /** 
        Removes collection of objects from the specified folder. If folder is empty - removes object from all folders.
        Note: the object & all related data is automatically removed from database when
        object is not placed in any folder or is not a part of any other more complex object (ex: sequence in msa)
    */
    virtual void removeObjects(const QList<U2DataId>& dataIds, const QString& folder, U2OpStatus& os) = 0;
};


/**
    An interface used to add/remove folders
    Requires U2ObjectRWDbi to present
*/
class U2CORE_EXPORT U2FolderDbi : public U2ObjectRWDbi {
protected:
    U2FolderDbi(U2Dbi* rootDbi): U2ObjectRWDbi(rootDbi) {}

public:
    /** Creates folder in the database.
    The specified path must be a valid unique path, not existing in the database.
    It is not required that parent folders must exist, they are created automatically.
    */
    virtual void createFolder(const QString& path, U2OpStatus& os) = 0;

    /** Removes folder. The folder must be existing path. Runs GC check for all objects in the folder */
    virtual void removeFolder(const QString& folder, U2OpStatus& os) = 0;

    /** Adds objects to the specified folder.
        All objects must exist and have a top-level type.
        Counter role: number of objects added
    */
    virtual void addObjectsToFolder(const QList<U2DataId>& objectIds, const QString& toFolder, 
                                U2OpStatus& os) = 0;

    /** Moves objects between folders.
    'fromFolder' must be existing path containing all specified objects.
    'toFolder' must be existing path or empty string.
    If 'toFolder' is empty, removes the objects from 'fromFolder' and 
    deletes non-top-level objects without parents, if any appear in the specified list.
    Otherwise, moves the specified objects between the specified folders, omitting duplicates.
    Counter role: number of objects moved
    */
    virtual void moveObjects(const QList<U2DataId>& objectIds, const QString& fromFolder, const QString& toFolder, 
        U2OpStatus& os) = 0;
};

/**
    An interface to obtain 'read' access to sequence objects
*/
class U2CORE_EXPORT U2SequenceRDbi : public U2ChildDbi {
protected:
    U2SequenceRDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi){}

public:
    /** Reads sequence object from database */
    virtual U2Sequence getSequenceObject(U2DataId sequenceId, U2OpStatus& os) = 0;
    
    /**  
    Reads specified sequence data region from database.
    The region must be valid region within sequence bounds.
    */
    virtual QByteArray getSequenceData(U2DataId sequenceId, const U2Region& region, U2OpStatus& os) = 0;
};

/**
    An interface to obtain 'write' access to sequence objects
*/
class U2CORE_EXPORT U2SequenceRWDbi : public U2SequenceRDbi {
protected:
    U2SequenceRWDbi(U2Dbi* rootDbi) : U2SequenceRDbi(rootDbi){}

public:

    /**  Adds new (empty) sequence instance into database, sets the assigned id on the passed U2Sequence instance. 
        The folder must exist in the database.
        Use 'updateSequenceData' method to supply data to the created sequence.

        //TODO do we ever need to allow empty folder??
    */
    virtual void createSequenceObject(U2Sequence& sequence, const QString& folder, U2OpStatus& os) = 0;

    /** 
        Updates sequence region. 
        The region must be valid region within sequence bounds.
        Note: regionToReplace length can differ from dataToInsert length, so the method can be used to add/remove sequence regions.

        //TODO think about annotations: should we fix locations automatically?? If yes, emit notifications??
        // varlax: I think this should be left to user, no automatic fixes.
    */
    virtual void updateSequenceData(U2DataId sequenceId, const U2Region& regionToReplace, const QByteArray& dataToInsert, U2OpStatus& os) = 0;
};


/**
    An interface to obtain 'read' access to sequence annotations
*/
class U2CORE_EXPORT U2AnnotationRDbi : public U2ChildDbi {
protected:
    U2AnnotationRDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi){}

public:
    /** 
        Returns number of annotations for the given sequence object  that belongs to the group specified
        If group name is empty - all annotations are counted
    */
    virtual qint64 countAnnotations(U2DataId sequenceId, const QString& group,  U2OpStatus& os) = 0;
    
    /** 
        Returns number of annotations for the given sequence object in the given region.
        Counts all annotations whose location intersects the region.
    */
    virtual qint64 countAnnotations(U2DataId sequenceId, const U2Region& region, U2OpStatus& os) = 0;

    /** 
        Returns annotations for the given sequence object that belongs to the group specified
        If group is empty searches globally in all groups
        Orders result by qualifier if not empty
        The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. 
    */
    virtual QList<U2DataId> getAnnotations(U2DataId sequenceId, const QString& group, const QString& orderByQualifier,
                                            qint64 offset, qint64 count, U2OpStatus& os) = 0;
    
    /** 
        Returns annotations for the given sequence object in the given region.
        Counts all annotations whose location intersects the region.
        Orders result by qualifier if not empty.
        The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. 
    */
    virtual QList<U2DataId> getAnnotations(U2DataId sequenceId, const U2Region& region, const QString& orderByQualifier, 
                                            qint64 offset, qint64 count, U2OpStatus& os) = 0;

    /** Reads annotation entity by id */
    virtual U2Annotation getAnnotation(U2DataId annotationId, U2OpStatus& os) = 0;
};

/**
    An interface to obtain 'write' access to sequence annotations
*/
class U2CORE_EXPORT U2AnnotationRWDbi : public U2AnnotationRDbi {
protected:
    U2AnnotationRWDbi(U2Dbi* rootDbi) : U2AnnotationRDbi(rootDbi){}

public:

    /** Adds new annotation. Assigns Id to annotation */
    virtual void createAnnotation(U2Annotation& a, U2OpStatus& os) = 0;

    /** 
        Adds list of new annotations. Assigns Ids to annotations added 
        Counter role: number of annotations created
    */
    virtual void createAnnotations(QList<U2Annotation>& annotations, U2OpStatus& os) = 0;

    
    /* Removes annotation from database */
    virtual void removeAnnotation(U2DataId annotationId, U2OpStatus& os) = 0;

    /**  
        Removes annotations from database  
        Counter role: number of annotations removed
    */
    virtual void removeAnnotations(const QList<U2DataId>& annotationIds, U2OpStatus& os) = 0;
    
    /** Changes annotations location */
    virtual U2Annotation updateLocation(U2DataId annotationId, const U2Location& location, U2OpStatus& os) = 0;  
    
    /** Changes annotations name */
    virtual U2Annotation updateName(U2DataId annotationId, const QString& newName, U2OpStatus& os) = 0;  
    
    /** Adds new qualifier to annotation  */
    virtual U2Annotation createQualifier(U2DataId annotationId, const U2Qualifier& q, U2OpStatus& os) = 0;
    
    /** Removes existing qualifier from annotation  */    
    virtual U2Annotation removeQualifier(U2DataId annotationId, const U2Qualifier& q, U2OpStatus& os) = 0; 

    /** Adds annotation to the specified group */    
    virtual U2Annotation addToGroup(U2DataId annotationId, const QString& group, U2OpStatus& os) = 0; 
    
    /** 
        Removes annotation from the specified group 
        If annotation belongs to no group, it is removed
    */
    virtual U2Annotation removeFromGroup(U2DataId annotationId, const QString& group, U2OpStatus& os) = 0; 
};


/**
    An interface to obtain 'read' access to multiple sequence alignment
*/
class U2CORE_EXPORT U2MsaRDbi : public U2ChildDbi {
protected:
    U2MsaRDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi) {} 

public:
    /** Reads Msa objects by id */
    virtual U2Msa getMsaObject(U2DataId id, U2OpStatus& os) = 0;

    /** Returns number of sequences in MSA*/
    virtual qint64 getSequencesCount(U2DataId msaId, U2OpStatus& os) = 0;

    /** Returns region of Msa rows. Total number of MSA rows is equal to number of sequences in MSA.
    The 'firstRow' and 'numRows' must specify valid subset of rows in the alignment. */
    virtual QList<U2MsaRow> getRows(U2DataId msaId, qint32 firstRow, qint32 numRows, U2OpStatus& os) = 0;
    
    /** 
        Return number of sequences in alignment that intersect given coord.
        'Intersect' here means that first non gap character is <= coord <= last non gap character.
        The coord should be a valid coordinate within alignment bounds, i.e. non-negative and less than alignment length.
    */
    virtual qint32 countSequencesAt(U2DataId msaId, qint64 coord, U2OpStatus& os) = 0;

    /** Return 'count' sequences starting with 'offset' that intersect given coordinate.
    The coord should be a valid coordinate within alignment bounds, i.e. non-negative and less than alignment length.
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getSequencesAt(U2DataId msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os) = 0;
    
    /** Return number of sequences in alignment that intersect given region.
    The region should be a valid region within alignment bounds, i.e. non-negative and less than alignment length.
        'Intersect' here means that first non gap character is <= coord <= last non gap character
    */
    virtual qint32 countSequencesAt(U2DataId msaId, const U2Region& r, U2OpStatus& os) = 0;

    /** Return 'count' sequences starting with 'offset' that intersect given region.
    The region should be a valid region within alignment bounds, i.e. non-negative and less than alignment length.
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getSequencesAt(U2DataId msaId, const U2Region& r, qint32 offset, qint32 count, U2OpStatus& os) = 0;

    /** Return number of sequences in alignment that that have non-gap character at the given coord.
    The coord should be a valid coordinate within alignment bounds, i.e. non-negative and less than alignment length.*/
    virtual qint32 countSequencesWithoutGapAt(U2DataId msaId, qint64 coord, U2OpStatus& os) = 0;
    
    /** Return 'count' sequence starting with 'offset' alignment that that have non-gap character at the given coord.
    The coord should be a valid coordinate within alignment bounds, i.e. non-negative and less than alignment length.
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getSequencesWithoutGapAt(U2DataId msaId, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os) = 0;


};

/**
    An interface to obtain 'write' access to multiple sequence alignment
*/
class U2CORE_EXPORT U2MsaRWDbi : public U2MsaRDbi {
protected:
    U2MsaRWDbi(U2Dbi* rootDbi) : U2MsaRDbi(rootDbi) {} 

public:
    /** Creates new empty Msa object  */
    virtual void createMsaObject(U2Msa& msa, const QString& folder, U2OpStatus& os) = 0;
    
    /** 
        Removes sequences from MSA
        Automatically removes affected sequences that are not anymore located in some folder nor Msa object
        Counter role: number of sequences removed
    */
    virtual void removeSequences(U2Msa& msa, const QList<U2DataId> sequenceIds, U2OpStatus& os) = 0;

    /**  
        Adds sequences to MSA 
        Counter role: number of sequences added
    */
    virtual void addSequences(U2Msa& msa, const QList<U2MsaRow>& rows, U2OpStatus& os) = 0;

};

/**
    An interface to obtain 'read' access to assembly data
*/
class U2CORE_EXPORT U2AssemblyRDbi : public U2ChildDbi {
protected:
    U2AssemblyRDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi) {} 

public:
    /** Reads assembly objects by id */
    virtual U2Assembly getAssemblyObject(U2DataId id, U2OpStatus& os) = 0;

    /** 
        Return number of reads in assembly that intersect given region.
        The region should be a valid region within alignment bounds, i.e. non-negative and less than alignment length.
        'Intersect' here means that region(leftmost pos, rightmost pos) intersects with 'r'
    */
    virtual qint64 countReadsAt(U2DataId assemblyId, const U2Region& r, U2OpStatus& os) = 0;

    /** Return 'count' row ids starting with 'offset' that intersect given region.
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2DataId> getReadIdsAt(U2DataId assemblyId, const U2Region& r, qint64 offset, qint64 count, U2OpStatus& os) = 0;

    /** Return 'count' rows starting with 'offset' that intersect given region.
    The 'offset' and 'count' can be arbitrarily large but should not be negative. Also, 'count' can have special value 'DBI_NO_LIMIT'. */
    virtual QList<U2AssemblyRead> getReadsAt(U2DataId assemblyId, const U2Region& r, qint64 offset, qint64 count, U2OpStatus& os) = 0;

    /** Return assembly row structure by id */
    virtual U2AssemblyRead getReadById(U2DataId rowId, U2OpStatus& os) = 0;

    /** 
        Return max packed row at the given coordinate
        'Intersect' here means that region(leftmost pos, rightmost pos) intersects with 'r'
    */
    virtual qint64 getMaxPackedRow(U2DataId assemblyId, const U2Region& r, U2OpStatus& os) = 0;

    /** Return reads with packed row value >= min, <= max that intersect given region */
    virtual QList<U2AssemblyRead> getReadsByRow(U2DataId assemblyId, const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os) = 0;


    /** Count 'length of assembly' - position of the rightmost base of all reads */
    virtual quint64 getMaxEndPos(U2DataId assemblyId, U2OpStatus& os) = 0;

};

/** Class used to iterate huge amount of assembly reads and optimize assembly import operation to DBI */
class U2AssemblyReadsIterator {
public:
    /** returns true if there are more reads to iterate*/
    virtual bool hasNext() = 0;
    
    /** returns next read or error */
    virtual U2AssemblyRead next(U2OpStatus& os) = 0;
};
    

/**
    An interface to obtain 'write' access to multiple sequence alignment
*/
class U2CORE_EXPORT U2AssemblyRWDbi : public U2AssemblyRDbi {
protected:
    U2AssemblyRWDbi(U2Dbi* rootDbi) : U2AssemblyRDbi(rootDbi) {} 

public:
    /** Creates new empty assembly object, reads iterator can be NULL  */
    virtual void createAssemblyObject(U2Assembly& assembly, const QString& folder, U2AssemblyReadsIterator* it, U2OpStatus& os) = 0;
    
    /** 
        Removes sequences from assembly
        Automatically removes affected sequences that are not anymore accessible from folders
        Counter role: number of reads removed
    */
    virtual void removeReads(U2DataId assemblyId, const QList<U2DataId>& rowIds, U2OpStatus& os) = 0;

    /**  
        Adds sequences to assembly
        Reads got their ids assigned.
        Counter role: number of reads added
    */
    virtual void addReads(U2DataId assemblyId, QList<U2AssemblyRead>& rows, U2OpStatus& os) = 0;

    /**  
        Packs assembly rows: assigns packedViewRow value for every read in assembly 
        Counter role: percent of completion
    */
    virtual void pack(U2DataId assemblyId, U2OpStatus& os) = 0;

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
    An interface to obtain 'read' access to object attributes
*/
class U2CORE_EXPORT U2AttributeRDbi: public U2ChildDbi {
protected:
    U2AttributeRDbi(U2Dbi* rootDbi) : U2ChildDbi(rootDbi){}

public:
    /** Returns all attribute names available in the database */
    virtual QStringList getAvailableAttributeNames(U2OpStatus& os) = 0;
    
    /** Returns all attribute ids for the given object */
    virtual QList<U2DataId> getObjectAttributes(const U2DataId& objectId, U2OpStatus& os) = 0;

    /** Returns all attribute ids for the given object */
    virtual QList<U2DataId> getObjectPairAttributes(const U2DataId& objectId, const U2DataId& childId, U2OpStatus& os) = 0;

    /** Loads int32 attribute by id */
    virtual U2Int32Attribute getInt32Attribute(U2DataId attributeId, U2OpStatus& os) = 0;

    /** Loads int64 attribute by id */
    virtual U2Int64Attribute getInt64Attribute(U2DataId attributeId, U2OpStatus& os) = 0;

    /** Loads real64 attribute by id */
    virtual U2Real64Attribute getReal64Attribute(U2DataId attributeId, U2OpStatus& os) = 0;

    /** Loads String attribute by id */
    virtual U2StringAttribute getStringAttribute(U2DataId attributeId, U2OpStatus& os) = 0;

    /** Loads byte attribute by id */
    virtual U2ByteArrayAttribute getByteArrayAttribute(U2DataId attributeId, U2OpStatus& os) = 0;
    
    /** Loads date-time attribute by id */
    virtual U2DateTimeAttribute getDateTimeAttribute(U2DataId attributeId, U2OpStatus& os) = 0;

    /** Loads range int32-values attribute by id */
    virtual U2RangeInt32StatAttribute getRangeInt32StatAttribute(U2DataId attributeId, U2OpStatus& os) = 0;

    /** Loads range real64-values attribute by id */
    virtual U2RangeReal64StatAttribute getRangeReal64StatAttribute(U2DataId attributeId, U2OpStatus& os) = 0;

    /** Sorts all objects in database according to U2DbiSortConfig provided  */
    virtual QList<U2DataId> sort(const U2DbiSortConfig& sc, qint64 offset, qint64 count, U2OpStatus& os) = 0;
};

/** 
    An interface to create / remove attributes
    Note: when attribute is created, the old one with the same name must be automatically deleted
*/
class U2CORE_EXPORT U2AttributeRWDbi: public U2AttributeRDbi {
protected:
    U2AttributeRWDbi(U2Dbi* rootDbi) : U2AttributeRDbi(rootDbi){}

public:
    /** Removes attribute from database */
    virtual void removeAttribute(const U2DataId& attributeId, U2OpStatus& os) = 0;
    
    /** Creates int32 attribute in database. ObjectId must be already set in attribute and present in the same database */
    virtual void createInt32Attribute(U2Int32Attribute& a, U2OpStatus& os) = 0;

    /** Creates int64 attribute in database. ObjectId must be already set in attribute and present in the same database */    
    virtual void createInt64Attribute(U2Int64Attribute& a, U2OpStatus& os) = 0;

    /** Creates real64 attribute in database. ObjectId must be already set in attribute and present in the same database */    
    virtual void createReal64Attribute(U2Real64Attribute& a, U2OpStatus& os) = 0;

    /** Creates String attribute in database. ObjectId must be already set in attribute and present in the same database */    
    virtual void createStringAttribute(U2StringAttribute& a, U2OpStatus& os) = 0;

    /** Creates Byte attribute in database. ObjectId must be already set in attribute and present in the same database */    
    virtual void createByteArrayAttribute(U2ByteArrayAttribute& a, U2OpStatus& os) = 0;

    /** Creates Date-time attribute in database. ObjectId must be already set in attribute and present in the same database */    
    virtual void createDateTimeAttribute(U2DateTimeAttribute& a, U2OpStatus& os) = 0;

    /** Creates range int32-values attribute in database. ObjectId must be already set in attribute and present in the same database */    
    virtual void createRangeInt32StatAttribute(U2RangeInt32StatAttribute& a, U2OpStatus& os) = 0;

    /** Creates range real32-values attribute in database. ObjectId must be already set in attribute and present in the same database */    
    virtual void createRangeReal64StatAttribute(U2RangeReal64StatAttribute& a, U2OpStatus& os) = 0;
};


} //namespace

#endif
