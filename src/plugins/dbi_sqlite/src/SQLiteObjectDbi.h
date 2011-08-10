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

#ifndef _U2_SQLITE_OBJECT_DBI_H_
#define _U2_SQLITE_OBJECT_DBI_H_

#include "SQLiteDbi.h"

namespace U2 {

class SQLiteObjectDbi : public U2ObjectDbi, public SQLiteChildDBICommon {
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
    virtual QList<U2DataId> getParents(const U2DataId& entityId, U2OpStatus& os);


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
    virtual QStringList getObjectFolders(const U2DataId& objectId, U2OpStatus& os);



    // Write methods for objects

    /** 
        Removes object from the specified folder. If folder is empty - removes object from all folders.
        Note: the object & all related data is automatically removed from database when
        object is not placed in any folder or is not a part of any other more complex object (ex: sequence in msa)
    */
    virtual void removeObject(const U2DataId& dataId, const QString& folder, U2OpStatus& os);
    
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
        Creates database entry for 'object'. Puts object into the 'folder'.
        Retrieves type and name from 'object', sets its id and version.

        Returns result object data id
    */
    U2DataId createObject(U2Object & object, const QString& folder, SQLiteDbiObjectRank rank, U2OpStatus& os);

    /**
        Retrieves U2Object fields from database entry with 'id'
        and sets these fields for 'object'
    */
    void getObject(U2Object& object, const U2DataId& id, U2OpStatus& os);

    /**
        Updates database entry for 'obj'. Increments its version.
    */
    void updateObject(U2Object& obj, U2OpStatus& os);

    /** 
        Returns internal database folder id 
        Fails if folder not found and 'mustExist' == true
    */
    static qint64 getFolderId(const QString& path, bool mustExist, DbRef* db, U2OpStatus& os);

    /** Increment object version count */
    static void incrementVersion(const U2DataId& id, DbRef* db, U2OpStatus& os);

    /** Returns version of the given object */
    virtual qint64 getObjectVersion(const U2DataId& objectId, U2OpStatus& os);

    /** 
        Removes objects parent relation.
        If child object has no parents and is not top level 
        it will be automatically removed if 'removeDeadChild' is true
    */
    void removeParent(const U2DataId& parentId, const U2DataId& childId, bool removeDeadChild, U2OpStatus& os);

    /** Ensures that corresponding parent record exists in db. Adds record if not exists */
    void ensureParent(const U2DataId& parentId, const U2DataId& childId, U2OpStatus& os);

    virtual void initSqlSchema(U2OpStatus& os);

private:

    /** Removes object from database, returns 'true' if object is completely erased */
    bool removeObjectImpl(const U2DataId& id, const QString& folder, U2OpStatus& os);

    void removeObjectAttributes(const U2DataId& id, U2OpStatus& os);

    /** Updates versions */
    void onFolderUpdated(const QString& folder);
};


class SQLiteCrossDatabaseReferenceDbi : public U2CrossDatabaseReferenceDbi, public SQLiteChildDBICommon {
public:
    SQLiteCrossDatabaseReferenceDbi(SQLiteDbi* dbi);

    /**
        Adds new remote object to database.
        Sets local object id assigned to the new value
        Requires: U2DbiFeature_WriteCrossDatabaseReferences
    */
    virtual void createCrossReference(U2CrossDatabaseReference& reference, U2OpStatus& os);

    /**
        Loads remote object information from DB
        Requires: U2DbiFeature_ReadCrossDatabaseReferences
    */
    virtual U2CrossDatabaseReference getCrossReference(const U2DataId& objectId, U2OpStatus& os);

    /**
        Updates all fields of cross database reference object
        Requires: U2DbiFeature_WriteCrossDatabaseReferences
    */
    virtual void updateCrossReference(const U2CrossDatabaseReference& reference, U2OpStatus& os);

    virtual void initSqlSchema(U2OpStatus& os);
};

} //namespace

#endif
