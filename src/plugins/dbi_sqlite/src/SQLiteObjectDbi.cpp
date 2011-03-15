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

#include "SQLiteObjectDbi.h"

#include <U2Core/U2SqlHelpers.h>

namespace U2 {

SQLiteObjectDbi::SQLiteObjectDbi(SQLiteDbi* dbi) : U2FolderDbi(dbi), SQLiteChildDBICommon(dbi) {

}

//////////////////////////////////////////////////////////////////////////
// Read methods for objects

qint64 SQLiteObjectDbi::countObjects(U2OpStatus& os) {
    return SQLiteQuery("COUNT (*) FROM Object WHERE top_level = 1", db, os).selectInt64();
}

qint64 SQLiteObjectDbi::countObjects(U2DataType type, U2OpStatus& os) {
    SQLiteQuery q("COUNT (*) FROM Object WHERE top_level = 1 AND type = ?1", db, os);
    q.bindType(1, type);
    return q.selectInt64();
}

QList<U2DataId> SQLiteObjectDbi::getObjects(qint64 offset, qint64 count, U2OpStatus& os) {
    return SQLiteQuery("SELECT id, type FROM Object WHERE top_level = 1", offset, count, db, os).selectDataIdsExt();
}

QList<U2DataId> SQLiteObjectDbi::getObjects(U2DataType type, qint64 offset, qint64 count, U2OpStatus& os) {
    SQLiteQuery q("SELECT id, type FROM Object WHERE top_level = 1 AND type = ?1", offset, count, db, os );
    q.bindType(1, type);
    return q.selectDataIdsExt();
}

QList<U2DataId> SQLiteObjectDbi::getParents(U2DataId entityId, U2OpStatus& os) {
    SQLiteQuery q("SELECT o.id AS id, o.type AS type FROM Parent AS p, Object AS o WHERE p.child = ?1 and p.parent = o.id", db, os);
    q.bindDataId(1, entityId);
    return q.selectDataIdsExt();
}


//////////////////////////////////////////////////////////////////////////
// Write methods for objects

void SQLiteObjectDbi::removeObject(U2DataId dataId, const QString& folder, U2OpStatus& os) {
    removeObjectImpl(dataId, folder, os);
    if (os.hasError()) {
        return;
    }
    onFolderUpdated(folder);
}

void SQLiteObjectDbi::removeObjects(const QList<U2DataId>& dataIds, const QString& folder, U2OpStatus& os) {
    foreach (U2DataId id, dataIds) {
        removeObjectImpl(id, folder, os);
        if (os.hasError()) {
            break;
        }
    }        
    onFolderUpdated(folder);
}

bool SQLiteObjectDbi::removeObjectImpl(U2DataId objectId, const QString& folder, U2OpStatus& os) {
    SQLiteTransaction trans(db, os);

    U2DataType type = getRootDbi()->getEntityTypeById(objectId);
    if (!U2Type::isObjectType(type)) {
        os.setError(SQLiteL10N::tr("Not an object! Id: %1, type: %2").arg(objectId).arg(type));
        return false;
    }
    qint64 folderId = SQLiteQuery("SELECT id FROM Folder WHERE path = '"+folder+"'", db, os).selectInt64();
    if (os.hasError()) {
        return false;
    }
    SQLiteQuery deleteQ("DELETE FROM FolderContent WHERE folder = ?1 AND object = ?2", db, os);
    deleteQ.bindInt64(1, folderId);
    deleteQ.bindDataId(2, objectId);
    int nRecords = deleteQ.update();
    if (os.hasError()) {
        return false;
    }
    if (nRecords == 0) {
        os.setError(SQLiteL10N::tr("Object: %1 not found in folder: %2").arg(objectId).arg(folder));
        return false;
    }
    QStringList folders = getObjectFolders(objectId, os);
    if (os.hasError()) {
        return false;
    }
    if (!folders.isEmpty()) { // object is a part of another folder ->  do not erase
        return false;
    }    
    QList<U2DataId> parents = getParents(objectId, os);
    if (os.hasError()) {
        return false;
    }
    if (parents.isEmpty()) { // object is a part of another object ->  do not erase
        //update top_level flag!
        SQLiteQuery toplevelQ("UPDATE Object SET(top_level = 0) WHERE id = ?1", db, os);
        toplevelQ.bindDataId(1, objectId);
        toplevelQ.execute();
        return false;
    }

    // now erase object
    // remove all attributes first

    //TODO: removeObjectAttributes();
    if (os.hasError()) {
        return false;
    }
    switch (type) {
        case U2Type::Sequence:
            SQLiteUtils::remove("Sequence", "object", objectId, 1, db, os);
            SQLiteUtils::remove("SequenceData", "sequence", objectId, -1, db, os);
            break;
        case U2Type::Msa:
            //TODO: removeMsaObject(objectId);
            break;
        case U2Type::PhyTree:
            //TODO: removePhyTreeObject(objectId);
            break;
        case U2Type::Assembly:
            //TODO: removeAssemblyObject(objectId);
            break;
        case U2Type::CrossDatabaseReference:
            //TODO: removeCrossDatabaseReferenceObject(objectId.id);
            break;
        default:
            os.setError(SQLiteL10N::tr("Unknown object type! Id: %1, type: %2").arg(objectId).arg(type));
    }
    if (os.hasError()) {
        return false;
    }
    SQLiteUtils::remove("Object", "id", objectId, 1, db, os);
    return !os.hasError();
}

//////////////////////////////////////////////////////////////////////////
// Read methods for folders

QStringList SQLiteObjectDbi::getFolders(U2OpStatus& os) {
    return SQLiteQuery("SELECT path FROM Folder ORDER BY path", db, os).selectStrings();
}

qint64 SQLiteObjectDbi::countObjects(const QString& folder, U2OpStatus& os) {
    SQLiteQuery q("SELECT COUNT(fc.*) FROM FolderContent AS fc, Folder AS f WHERE f.path = ?1 AND fc.folder = f.id", db, os);
    q.bindText(1, folder);
    return q.selectInt64();
}

QList<U2DataId> SQLiteObjectDbi::getObjects(const QString& folder, qint64 offset, qint64 count, U2OpStatus& os) {
    SQLiteQuery q("SELECT o.id, o.type FROM Object AS o, FolderContent AS fc, Folder AS f WHERE f.path = ?1 AND fc.folder = f.id AND fc.object=o.id", db, os);
    q.bindText(1, folder);
    return q.selectDataIdsExt();
}

QStringList SQLiteObjectDbi::getObjectFolders(U2DataId objectId, U2OpStatus& os) {
    SQLiteQuery q("SELECT f.path FROM FolderContent AS fc, Folder AS f WHERE fc.object = ?1 AND fc.folder = f.id", db, os);
    q.bindDataId(1, objectId);
    return q.selectStrings();
}

//////////////////////////////////////////////////////////////////////////
// Write methods for  folders

void SQLiteObjectDbi::createFolder(const QString& path, U2OpStatus& os) {
    //TODO: validate folder name
    SQLiteQuery q("INSERT INTO Folder(path) VALUES(?1)", db, os);
    q.bindText(1, path);
    q.execute();
    
    if (!os.hasError()) {
        onFolderUpdated(path);
    }
}

void SQLiteObjectDbi::removeFolder(const QString& folder, U2OpStatus& os) {
    // remove subfolders first
    SQLiteQuery q("SELECT path FROM Folder WHERE path LIKE ?1", db, os);
    q.bindText(1, folder + "/%");
    QStringList subfolders = q.selectStrings();
    if (os.hasError()) {
        return;
    }
    subfolders.sort(); //remove innermost folders first
    for (int i = subfolders.length(); --i >= 0 && !os.hasError();) {
        const QString& subfolder = subfolders.at(i);
        removeFolder(folder, os);
    }
    if (os.hasError()) {
        return;
    }

    // remove all objects from folder
    qint64 nObjects = countObjects(folder, os);
    if (os.hasError()) {
        return;
    }
    int nObjectsPerIteration = 1000;
    for (int i = 0; i < nObjects; i += nObjectsPerIteration) {
        QList<U2DataId> objects = getObjects(folder, i, nObjectsPerIteration, os);
        if (os.hasError()) {
            return;
        }

        // Remove all objects in the folder
        if (!objects.isEmpty()) {
            removeObjects(objects, folder, os);
            if (os.hasError()) {
                return;
            }
        }
    }

    // remove folder record
    SQLiteQuery dq("DELETE FROM Folder WHERE path = ?1", db, os);
    dq.bindText(1, folder);
    dq.execute();
    if (os.hasError()) {
        return;
    }
    onFolderUpdated(folder);
}

void SQLiteObjectDbi::addObjectsToFolder(const QList<U2DataId>& objectIds, const QString& folder, U2OpStatus& os) {
    qint64 folderId = getFolderId(folder, true, db, os);
    if (os.hasError()) {
        return;
    }
    QList<U2DataId> addedObjects;
    SQLiteQuery countQ("SELECT count(object) FROM FolderContent WHERE folder = ?1", db, os);
    SQLiteQuery insertQ("INSERT INTO FolderContent(folder, object) VALUES(?1, ?2)", db, os);
    SQLiteQuery toplevelQ("UPDATE Object SET (top_level = 1) WHERE id = ?1", db, os);

    foreach(const U2DataId& objectId, objectIds) {
        countQ.reset();
        countQ.bindInt64(1, folderId);
        int c = countQ.selectInt64();
        if (c != 0) {
            continue; // object is already in folder, skip it
        }
        insertQ.reset();
        insertQ.bindInt64(1, folderId);
        insertQ.bindDataId(2, objectId);
        insertQ.execute();
        
        toplevelQ.reset();
        toplevelQ.bindDataId(1, objectId);
        toplevelQ.execute();

        if (os.hasError()) {
            break;
        }

        addedObjects.append(objectId);
    }
    onFolderUpdated(folder);
}

void SQLiteObjectDbi::moveObjects(const QList<U2DataId>& objectIds, const QString& fromFolder, const QString& toFolder, U2OpStatus& os)  {
    if (fromFolder == toFolder) {
        return;
    }
    if (!toFolder.isEmpty()) {
        addObjectsToFolder(objectIds, toFolder, os);
        if (os.hasError()) {
            return;
        }
    }
    removeObjects(objectIds, fromFolder, os);
}


void SQLiteObjectDbi::removeParent(U2DataId parentId, U2DataId childId, bool removeDeadChild, U2OpStatus& os) {
    SQLiteQuery q("DELETE FROM Parent WHERE parent = ?1 AND child = ?2", db, os);
    q.bindDataId(1, parentId);
    q.bindDataId(2, childId);
    qint64 res = q.update(1);
    if (os.hasError()) {
        return;
    }
    if (!removeDeadChild) {
        return;
    }
    QList<U2DataId> parents = getParents(childId, os);
    if (!parents.isEmpty() || os.hasError()) {
        return;
    }
    QList<QString> folders = getObjectFolders(childId, os);
    if (!folders.isEmpty() || os.hasError()) {
        return;
    }
    removeObjects(QList<U2DataId>() << childId, "", os);
}


void SQLiteObjectDbi::ensureParent(U2DataId parentId, U2DataId childId, U2OpStatus& os) {
    SQLiteQuery checkQ("SELECT COUNT(*) FROM Parent WHERE parent = ?1 AND child = ?2", db, os);
    checkQ.bindDataId(1, parentId);
    checkQ.bindDataId(2, childId);
    if (checkQ.selectInt64() == 1) {
        return;
    }
    SQLiteQuery insertQ("INSERT INTO Parent (parent, child) VALUES (?1, ?2)", db, os);
    insertQ.bindDataId(1, parentId); 
    insertQ.bindDataId(2, childId);
    insertQ.execute();

}

//////////////////////////////////////////////////////////////////////////
// Helper methods

void SQLiteObjectDbi::incrementVersion(U2DataId objectId, DbRef* db, U2OpStatus& os) {
    SQLiteQuery q("UPDATE Object SET version = version + 1 WHERE id = ?1", db, os);
    q.bindDataId(1, objectId);
    q.update(1);
}

qint64 SQLiteObjectDbi::getObjectVersion(U2DataId objectId, U2OpStatus& os) {
    SQLiteQuery q("SELECT version FROM Object WHERE id = ?1", db, os);
    q.bindDataId(1, objectId);
    return q.selectInt64();
}

U2DataId SQLiteObjectDbi::createObject(U2DataType type, const QString& folder, const QString& vname, DbRef* db, U2OpStatus& os) {
    SQLiteQuery i1("INSERT INTO Object(type, top_level, name) VALUES(?1, ?2, ?3)", db, os);
    i1.bindType(1, type);
    i1.bindBool(2, true);
    i1.bindText(3, vname);
    U2DataId res = i1.insert(type);
    if (os.hasError()) {
        return res;
    }
    if (folder.isEmpty()) {
        return res;
    }
    qint64 folderId = getFolderId(folder, true, db, os);
    if (os.hasError()) {
        return res;
    }

    SQLiteQuery i2("INSERT INTO FolderContent(folder, object) VALUES(?1, ?2)", db, os);
    i2.bindInt64(1, folderId);
    i2.bindDataId(2, res);    
    i2.execute();

    return res;
}

qint64 SQLiteObjectDbi::getFolderId(const QString& path, bool mustExist, DbRef* db, U2OpStatus& os) {
    SQLiteQuery q("SELECT id FROM Folder WHERE path = ?1", db, os);
    q.bindText(1, path);
    qint64 res = q.selectInt64();
    if (os.hasError()) {
        return -1;
    }
    if (mustExist && res == -1) {
        os.setError(SQLiteL10N::tr("Folder not found :%1").arg(path));
    }
    return res;
}

qint64 SQLiteObjectDbi::getFolderLocalVersion(const QString& folder, U2OpStatus& os) {
    SQLiteQuery q("SELECT vlocal FROM Folder WHERE path = ?1", db, os);
    q.bindText(1, folder);
    return q.selectInt64();
}

qint64 SQLiteObjectDbi::getFolderGlobalVersion(const QString& folder, U2OpStatus& os) {
    SQLiteQuery q("SELECT vglobal FROM Folder WHERE path = ?1", db, os);
    q.bindText(1, folder);
    return q.selectInt64();
}

void SQLiteObjectDbi::onFolderUpdated(const QString& folder) {
    //TODO: update local version of the given folder & global for all parents
}

} //namespace
