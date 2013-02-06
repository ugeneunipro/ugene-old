/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_OBJECT_DBI_H_
#define _U2_OBJECT_DBI_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Dbi.h>

namespace U2 {
/**
    An primary interface to access database content
    Any database contains objects that are placed into folders
    Folders have unique string IDs - constructed similar to full folders names on Unix systems
    The root folder "/" is required for any DBI
*/
class U2ObjectDbi : public U2ChildDbi {
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


    /** Returns objects iterator with a given name and type. If type is Unknown -> checks objects of all types */
    virtual U2DbiIterator<U2DataId>* getObjectsByVisualName(const QString& visualName, U2DataType type, U2OpStatus& os) = 0;


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

    /** Specified whether modifications in object must be tracked or not */
    virtual void setTrackModType(const U2DataId& objectId, U2TrackModType trackModType, U2OpStatus& os) = 0;

    /** Gets the trackMod value for the object */
    virtual U2TrackModType getTrackModType(const U2DataId& objectId, U2OpStatus& os) = 0;

    /** 
        Removes object from the specified folder. If folder is empty - removes object from all folders.
        Note: the object & all related data is automatically removed from database when
        object is not placed in any folder or is not a part of any other more complex object (ex: sequence in msa)
        Requires: U2DbiFeature_RemoveObjects feature support
    */
    virtual void removeObject(const U2DataId& dataId, const QString& folder, U2OpStatus& os) = 0;
    virtual void removeObject(const U2DataId& dataId, U2OpStatus& os) { removeObject(dataId, QString(), os); }
    
    /** 
        Removes collection of objects from the specified folder. If folder is empty - removes object from all folders.
        Note: the object & all related data is automatically removed from database when
        object is not placed in any folder or is not a part of any other more complex object (ex: sequence in msa)
        Requires: U2DbiFeature_RemoveObjects feature support
    */
    virtual void removeObjects(const QList<U2DataId>& dataIds, const QString& folder, U2OpStatus& os) = 0;
    virtual void removeObjects(const QList<U2DataId>& dataIds, U2OpStatus& os) { removeObjects(dataIds, QString(), os); }


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

} //namespace

#endif
