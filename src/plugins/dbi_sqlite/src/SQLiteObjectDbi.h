#ifndef _U2_SQLITE_OBJECT_DBI_H_
#define _U2_SQLITE_OBJECT_DBI_H_

#include "SQLiteDbi.h"

namespace U2 {

class SQLiteObjectDbi : public U2FolderDbi, public SQLiteChildDBICommon {
public:
    SQLiteObjectDbi(SQLiteDbi* dbi);

    // Read methods for objects

    /**  Returns number of top-level U2Objects in database */
    virtual qint64 countObjects(U2OpStatus& os);

    /**  Returns number of top-level U2Objects with the specified type in database */
    virtual qint64 countObjects(U2DataType type, U2OpStatus& os);

    /** Lists database top-level objects, starts with 'offset' and limits by 'count' */
    virtual QList<U2DataId> getObjects(qint64 offset, qint64 count, U2OpStatus& os);

    /** Lists database top-level objects of the specified type, starts with 'offset' and limits by 'count' */
    virtual QList<U2DataId> getObjects(U2DataType type, qint64 offset, qint64 count, U2OpStatus& os);

    /**  Returns parents for entity.
        If entity is object, returns other object this object is a part of
        If object is not a part of any other object and does not belongs to any folder - it's automatically removed.
     */
    virtual QList<U2DataId> getParents(U2DataId entityId, U2OpStatus& os);


    // Read methods for folders

    
    /**  Returns list of folders stored in database. 
        Folders are separated by '/' character.
        At least one root folder is required. 
    */
    virtual QStringList getFolders(U2OpStatus& os);

    /** Returns number of top-level U2Objects in folder */
    virtual qint64 countObjects(const QString& folder, U2OpStatus& os);

    /** Lists database top-level objects of the specified type, starts with 'offset' and limits by 'count' */
    virtual QList<U2DataId> getObjects(const QString& folder, qint64 offset, qint64 count, U2OpStatus& os);

    /**  Returns all folders this object must be shown in  */
    virtual QStringList getObjectFolders(U2DataId objectId, U2OpStatus& os);



    // Write methods for objects

    /** 
        Removes object from the specified folder. If folder is empty - removes object from all folders.
        Note: the object & all related data is automatically removed from database when
        object is not placed in any folder or is not a part of any other more complex object (ex: sequence in msa)
    */
    virtual void removeObject(U2DataId dataId, const QString& folder, U2OpStatus& os);
    
    /** 
        Removes collection of objects from the specified folder. If folder is empty - removes object from all folders.
        Note: the object & all related data is automatically removed from database when
        object is not placed in any folder or is not a part of any other more complex object (ex: sequence in msa)
    */
    virtual void removeObjects(const QList<U2DataId>& dataIds, const QString& folder, U2OpStatus& os);


    // Write methods for folders
    
    /** Creates folder in the database.
    The specified path must be a valid unique path, not existing in the database.
    It is not required that parent folders must exist, they are created automatically.
    */
    virtual void createFolder(const QString& path, U2OpStatus& os);

    /** Removes folder. The folder must be existing path. Runs GC check for all objects in the folder */
    virtual void removeFolder(const QString& folder, U2OpStatus& os);

    /** Returns version of the folder. 
        The folder version increases if new object(s)/subfolder(s) are added into this folder
        Note that if object(s)/folder(s) are added into one of the subfolders the folder version is not changed
    */
    virtual qint64 getFolderLocalVersion(const QString& folder, U2OpStatus& os);

    /** Returns version of the folder that changes every time object(s)/folder(s) added 
        to the specified folder or any of its child folders
    */
    virtual qint64 getFolderGlobalVersion(const QString& folder, U2OpStatus& os);


    /** Adds objects to the specified folder.
    All objects must exist and have a top-level type.*/
    virtual void addObjectsToFolder(const QList<U2DataId>& objectIds, const QString& toFolder, U2OpStatus& os);

    /** Moves objects between folders.
    'fromFolder' must be existing path containing all specified objects.
    'toFolder' must be existing path or empty string.
    If 'toFolder' is empty, removes the objects from 'fromFolder' and 
    deletes non-top-level objects without parents, if any appear in the specified list.
    Otherwise, moves the specified objects between the specified folders, omitting duplicates.
    */
    virtual void moveObjects(const QList<U2DataId>& objectIds, const QString& fromFolder, const QString& toFolder, U2OpStatus& os);

   

    //////////////////////////////////////////////////////////////////////////
    // Helper methods

    /**
        Creates new object of the specified type. Puts it into the 'folder'
        Returns result object data id
    */
    static U2DataId createObject(U2DataType type, const QString& folder, const QString& objectName, DbRef* db, U2OpStatus& os);

    /** 
        Returns internal database folder id 
        Fails if folder not found and 'mustExist' == true
    */
    static qint64 getFolderId(const QString& path, bool mustExist, DbRef* db, U2OpStatus& os);

    /** Increment object version count */
    static void incrementVersion(U2DataId id, DbRef* db, U2OpStatus& os);

    /** Returns version of the given object */
    virtual qint64 getObjectVersion(U2DataId objectId, U2OpStatus& os);

    /** 
        Removes objects parent relation.
        If child object has no parents and is not top level 
        it will be automatically removed if 'removeDeadChild' is true
    */
    void removeParent(U2DataId parentId, U2DataId childId, bool removeDeadChild, U2OpStatus& os);

    /** Ensures that corresponding parent record exists in db. Adds record if not exists */
    void ensureParent(U2DataId parentId, U2DataId childId, U2OpStatus& os);

private:

    /** Removes object from database, returns 'true' if object is completely erased */
    bool removeObjectImpl(U2DataId id, const QString& folder, U2OpStatus& os);

    /** Updates versions */
    void onFolderUpdated(const QString& folder);
};


} //namespace

#endif
