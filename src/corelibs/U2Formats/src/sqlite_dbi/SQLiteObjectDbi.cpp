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

#include "SQLiteMsaDbi.h"
#include "SQLiteObjectDbi.h"
#include "SQLitePackUtils.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/SQLiteModDbi.h>
#include <U2Formats/SQLiteSequenceDbi.h>


namespace U2 {

using namespace SQLite;

SQLiteObjectDbi::SQLiteObjectDbi(SQLiteDbi* dbi) : U2ObjectDbi(dbi), SQLiteChildDBICommon(dbi) {

}

void SQLiteObjectDbi::upgrade(U2OpStatus &os) {
    SQLiteQuery q("PRAGMA table_info(Object)", db, os);
    CHECK_OP(os, );

    bool hasModTrack = false;
    while (q.step()) {
        QString colName = q.getString(1);
        if ("trackMod" == colName) {
            hasModTrack = true;
            break;
        }
    }
    if (hasModTrack) {
        return;
    }

    SQLiteQuery("ALTER TABLE Object ADD trackMod INTEGER NOT NULL DEFAULT 0", db, os).execute();
}

void SQLiteObjectDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }

    // objects table - stores IDs and types for all objects. It also stores 'top_level' flag to simplify queries
    // rank: see SQLiteDbiObjectRank
    // name is a visual name of the object shown to user.
    SQLiteQuery("CREATE TABLE Object (id INTEGER PRIMARY KEY AUTOINCREMENT, type INTEGER NOT NULL, "
                                    "version INTEGER NOT NULL DEFAULT 1, rank INTEGER NOT NULL, "
                                    "name TEXT NOT NULL, trackMod INTEGER NOT NULL DEFAULT 0)", db, os).execute();

    // parent-child object relation
    SQLiteQuery("CREATE TABLE Parent (parent INTEGER, child INTEGER, "
                       "FOREIGN KEY(parent) REFERENCES Object(id), "
                       "FOREIGN KEY(child) REFERENCES Object(id) )", db, os).execute();
    SQLiteQuery("CREATE INDEX Parent_parent_child on Parent(parent, child)" , db, os).execute();
    SQLiteQuery("CREATE INDEX Parent_child on Parent(child)" , db, os).execute();

    // folders 
    SQLiteQuery("CREATE TABLE Folder (id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT UNIQUE NOT NULL,  "
                                    "vlocal INTEGER NOT NULL DEFAULT 1, vglobal INTEGER NOT NULL DEFAULT 1 )", db, os).execute();

    // folder-object relation
    SQLiteQuery("CREATE TABLE FolderContent (folder INTEGER, object INTEGER, "
                        "FOREIGN KEY(folder) REFERENCES Folder(id),"
                        "FOREIGN KEY(object) REFERENCES Object(id) )", db, os).execute();
}

//////////////////////////////////////////////////////////////////////////
// Read methods for objects
#define TOP_LEVEL_FILTER  ("rank = " + QString::number(SQLiteDbiObjectRank_TopLevel))

qint64 SQLiteObjectDbi::countObjects(U2OpStatus& os) {
    return SQLiteQuery("COUNT (*) FROM Object WHERE " + TOP_LEVEL_FILTER, db, os).selectInt64();
}

qint64 SQLiteObjectDbi::countObjects(U2DataType type, U2OpStatus& os) {
    SQLiteQuery q("COUNT (*) FROM Object WHERE " + TOP_LEVEL_FILTER + " AND type = ?1", db, os);
    q.bindType(1, type);
    return q.selectInt64();
}

QList<U2DataId> SQLiteObjectDbi::getObjects(qint64 offset, qint64 count, U2OpStatus& os) {
    return SQLiteQuery("SELECT id, type FROM Object WHERE " + TOP_LEVEL_FILTER, offset, count, db, os).selectDataIdsExt();
}

QList<U2DataId> SQLiteObjectDbi::getObjects(U2DataType type, qint64 offset, qint64 count, U2OpStatus& os) {
    SQLiteQuery q("SELECT id, type FROM Object WHERE " + TOP_LEVEL_FILTER + " AND type = ?1", offset, count, db, os );
    q.bindType(1, type);
    return q.selectDataIdsExt();
}

QList<U2DataId> SQLiteObjectDbi::getParents(const U2DataId& entityId, U2OpStatus& os) {
    SQLiteQuery q("SELECT o.id AS id, o.type AS type FROM Parent AS p, Object AS o WHERE p.parent = o.id AND p.child = ?1", db, os);
    q.bindDataId(1, entityId);
    return q.selectDataIdsExt();
}

U2DbiIterator<U2DataId>* SQLiteObjectDbi::getObjectsByVisualName(const QString& visualName, U2DataType type, U2OpStatus& os) {
    bool checkType = (type != U2Type::Unknown);
    QString query = "SELECT id, type FROM Object WHERE " + TOP_LEVEL_FILTER 
            + " AND name = ?1 " + (checkType ? "AND type = ?2" : "");
    SQLiteQuery* q = new SQLiteQuery(query, db, os);
    q->bindString(1, visualName);
    if (checkType) {
        q->bindType(2, type);
    }
    return new SqlRSIterator<U2DataId>(q, new SqlDataIdRSLoaderEx(), NULL, U2DataId(), os);
}

//////////////////////////////////////////////////////////////////////////
// Write methods for objects

void SQLiteObjectDbi::removeObject(const U2DataId& dataId, const QString& folder, U2OpStatus& os) {
    removeObjectImpl(dataId, folder, os);
    CHECK_OP(os, );
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

void SQLiteObjectDbi::updateObjectCore(U2Object &obj, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    static const QString queryString("UPDATE Object SET name = ?1, version = version WHERE id = ?2");
    SQLiteQuery *q = t.getPreparedQuery(queryString, db, os);
    SAFE_POINT_OP(os, );
    q->bindString(1, obj.visualName);
    q->bindDataId(2, obj.id);
    q->execute();
}

bool SQLiteObjectDbi::removeObjectImpl(const U2DataId& objectId, const QString& folder, U2OpStatus& os) {
    SQLiteTransaction t(db, os);

    U2DataType type = getRootDbi()->getEntityTypeById(objectId);
    if (!U2Type::isObjectType(type)) {
        os.setError(SQLiteL10N::tr("Not an object! Id: %1, type: %2").arg(SQLiteUtils::text(objectId)).arg(type));
        return false;
    }
    if (folder.isEmpty()) {
        static const QString deleteString("DELETE FROM FolderContent WHERE object = ?1");
        SQLiteQuery *deleteQ = t.getPreparedQuery(deleteString, db, os);
        deleteQ->bindDataId(1, objectId);
        CHECK_OP(os, false);
    } else {
        static const QString selectString("SELECT id FROM Folder WHERE path = ?1");
        SQLiteQuery *selectQ = t.getPreparedQuery(selectString, db, os);
        selectQ->bindString(1, folder);
        qint64 folderId = selectQ->selectInt64();
        CHECK_OP(os, false);

        static const QString deleteString("DELETE FROM FolderContent WHERE folder = ?1 AND object = ?2");
        SQLiteQuery *deleteQ = t.getPreparedQuery(deleteString, db, os);
        deleteQ->bindInt64(1, folderId);
        deleteQ->bindDataId(2, objectId);
        deleteQ->update();
        CHECK_OP(os, false);
    }
    
    QStringList folders = getObjectFolders(objectId, os);
    CHECK_OP(os, false);

    if (!folders.isEmpty()) { // object is a part of another folder ->  do not erase
        return false;
    }    
    QList<U2DataId> parents = getParents(objectId, os);
    CHECK_OP(os, false);

    if (!parents.isEmpty()) { // object is a part of another object ->  do not erase
        //update top_level flag!
        static const QString toplevelString("UPDATE Object SET rank = ?1 WHERE id = ?2");
        SQLiteQuery *toplevelQ = t.getPreparedQuery(toplevelString, db, os);
        toplevelQ->bindInt32(1, SQLiteDbiObjectRank_Child);
        toplevelQ->bindDataId(2, objectId);
        toplevelQ->execute();
        return false;
    }

    // now erase object

    // remove all attributes first

    removeObjectAttributes(objectId, os);

    CHECK_OP(os, false);

    switch (type) {
        case U2Type::Sequence:
            SQLiteUtils::remove("Sequence", "object", objectId, 1, db, os);
            SQLiteUtils::remove("SequenceData", "sequence", objectId, -1, db, os);
            break;
        case U2Type::VariantTrack:
            SQLiteUtils::remove("VariantTrack", "object", objectId, 1, db, os);
            SQLiteUtils::remove("Variant", "track", objectId, -1, db, os);
            break;
        case U2Type::Msa:
            {
                // Remove rows
                SQLiteMsaDbi* sqliteMsaDbi = dbi->getSQLiteMsaDbi();
                if (NULL != sqliteMsaDbi) {
                    sqliteMsaDbi->removeAllRows(objectId, os);
                }
                else {
                    os.setError("SQLiteMsaDbi is NULL during removing an MSA object!");
                }
                // Remove the MSA record
                SQLiteUtils::remove("Msa", "object", objectId, 1, db, os);
            }
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
            os.setError(SQLiteL10N::tr("Unknown object type! Id: %1, type: %2").arg(SQLiteUtils::text(objectId)).arg(type));
    }
    CHECK_OP(os, false);

    // Remove modifications history
    // Note: affected modification steps of child objects are also removed
    removeObjectModHistory(objectId, os);
    CHECK_OP(os, false);

    SQLiteUtils::remove("Object", "id", objectId, 1, db, os);
    return !os.hasError();
}

void SQLiteObjectDbi::removeObjectAttributes(const U2DataId& id, U2OpStatus& os) {
    U2AttributeDbi* attributeDbi = dbi->getAttributeDbi();
    attributeDbi->removeObjectAttributes(id, os);
}

void SQLiteObjectDbi::removeObjectModHistory(const U2DataId& id, U2OpStatus& os) {
    U2ModDbi* modDbi = dbi->getModDbi();
    SAFE_POINT(NULL != modDbi, "NULL Mod Dbi!", );

    modDbi->removeObjectMods(id, os);
}

//////////////////////////////////////////////////////////////////////////
// Read methods for folders

QStringList SQLiteObjectDbi::getFolders(U2OpStatus& os) {
    return SQLiteQuery("SELECT path FROM Folder ORDER BY path", db, os).selectStrings();
}

qint64 SQLiteObjectDbi::countObjects(const QString& folder, U2OpStatus& os) {
    SQLiteQuery q("SELECT COUNT(fc.*) FROM FolderContent AS fc, Folder AS f WHERE f.path = ?1 AND fc.folder = f.id", db, os);
    q.bindString(1, folder);
    return q.selectInt64();
}

QList<U2DataId> SQLiteObjectDbi::getObjects(const QString& folder, qint64 , qint64 , U2OpStatus& os) {
    SQLiteQuery q("SELECT o.id, o.type FROM Object AS o, FolderContent AS fc, Folder AS f WHERE f.path = ?1 AND fc.folder = f.id AND fc.object=o.id", db, os);
    q.bindString(1, folder);
    return q.selectDataIdsExt();
}

QStringList SQLiteObjectDbi::getObjectFolders(const U2DataId& objectId, U2OpStatus& os) {
    SQLiteQuery q("SELECT f.path FROM FolderContent AS fc, Folder AS f WHERE fc.object = ?1 AND fc.folder = f.id", db, os);
    q.bindDataId(1, objectId);
    return q.selectStrings();
}

//////////////////////////////////////////////////////////////////////////
// Write methods for  folders

void SQLiteObjectDbi::createFolder(const QString& path, U2OpStatus& os) {
    //TODO: validate folder name
    SQLiteQuery q("INSERT INTO Folder(path) VALUES(?1)", db, os);
    q.bindString(1, path);
    q.execute();
    
    if (!os.hasError()) {
        onFolderUpdated(path);
    }
}

void SQLiteObjectDbi::removeFolder(const QString& folder, U2OpStatus& os) {
    // remove subfolders first
    SQLiteQuery q("SELECT path FROM Folder WHERE path LIKE ?1", db, os);
    q.bindString(1, folder + "/%");
    QStringList subfolders = q.selectStrings();
    if (os.hasError()) {
        return;
    }
    subfolders.sort(); //remove innermost folders first
    for (int i = subfolders.length(); --i >= 0 && !os.hasError();) {
        const QString& subfolder = subfolders.at(i);
        removeFolder(subfolder, os);
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
    dq.bindString(1, folder);
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
    SQLiteQuery toplevelQ("UPDATE Object SET rank = " + QString::number(SQLiteDbiObjectRank_TopLevel) + " WHERE id = ?1", db, os);

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

void SQLiteObjectDbi::incrementVersion(const U2DataId& id, U2OpStatus& os) {
    SQLiteQuery q("UPDATE Object SET version = version + 1 WHERE id = ?1", db, os);
    CHECK_OP(os, );

    q.bindDataId(1, id);
    q.update(1);
}

void SQLiteObjectDbi::setVersion(const U2DataId& id, qint64 version, U2OpStatus& os) {
    SQLiteQuery q("UPDATE Object SET version = ?1 WHERE id = ?2", db, os);
    SAFE_POINT_OP(os, );

    q.bindInt64(1, version);
    q.bindDataId(2, id);
    q.update(1);
}

bool SQLiteObjectDbi::canUndo(const U2DataId& objId, U2OpStatus& os) {
    return dbi->getSQLiteModDbi()->canUndo(objId, os);
}

bool SQLiteObjectDbi::canRedo(const U2DataId& objId, U2OpStatus& os) {
    return dbi->getSQLiteModDbi()->canRedo(objId, os);
}

void SQLiteObjectDbi::undo(const U2DataId& objId, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    QString errorDescr = SQLiteL10N::tr("Can't undo an operation for the object!");

    // Get the object
    U2Object obj;
    getObject(obj, objId, os);
    if (os.hasError()) {
        coreLog.trace("Error getting an object: " + os.getError());
        os.setError(errorDescr);
        return;
    }

    // Verify that modifications tracking is enabled for the object
    if (TrackOnUpdate != obj.trackModType) {
        coreLog.trace("Called 'undo' for an object without modifications tracking enabled!");
        os.setError(errorDescr);
        return;
    }

    // Get all single modifications steps
    qint64 userModStepVersion = dbi->getSQLiteModDbi()->getNearestUserModStepVersion(objId, obj.version - 1, os);
    if (os.hasError()) {
        coreLog.trace("Error getting the nearest userModStep version: " + os.getError());
        os.setError(errorDescr);
        return;
    }

    QList< QList<U2SingleModStep> > modSteps = dbi->getSQLiteModDbi()->getModSteps(objId, userModStepVersion, os);
    if (os.hasError()) {
        coreLog.trace("Error getting modSteps for an object: " + os.getError());
        os.setError(errorDescr);
        return;
    }


    QList< QList<U2SingleModStep> >::const_iterator multiIt = modSteps.end();
    while (multiIt != modSteps.begin()) {
        --multiIt;
        QList<U2SingleModStep> multiStepSingleSteps = *multiIt;

        QList<U2SingleModStep>::const_iterator singleIt = multiStepSingleSteps.end();
        while (singleIt != multiStepSingleSteps.begin()) {
            --singleIt;
            U2SingleModStep modStep = *singleIt;

            // Call an appropriate "undo" depending on the object type
            if (U2ModType::isMsaModType(modStep.modType)) {
                dbi->getSQLiteMsaDbi()->undo(modStep.objectId, modStep.modType, modStep.details, os);
            }
            else if (U2ModType::isSequenceModType(modStep.modType)) {
                dbi->getSQLiteSequenceDbi()->undo(modStep.objectId, modStep.modType, modStep.details, os);
            }
            else if (U2ModType::isObjectModType(modStep.modType)) {
                if (U2ModType::objUpdatedName == modStep.modType) {
                    undoUpdateObjectName(modStep.objectId, modStep.details, os);
                    CHECK_OP(os, );
                }
                else {
                    coreLog.trace(QString("Can't undo an unknown operation: '%1'!").arg(QString::number(modStep.modType)));
                    os.setError(errorDescr);
                    return;
                }
            }
            if (os.hasError()) {
                coreLog.trace(QString("Can't undo a single step: '%1'!").arg(os.getError()));
                os.setError(errorDescr);
                return;
            }

            setVersion(modStep.objectId, modStep.version, os);
            if (os.hasError()) {
                coreLog.trace("Failed to set an object version: " + os.getError());
                os.setError(errorDescr);
                return;
            }
        }
    }

    setVersion(objId, userModStepVersion, os);
    if (os.hasError()) {
        coreLog.trace("Failed to set an object version: " + os.getError());
        os.setError(errorDescr);
        return;
    }
}

void SQLiteObjectDbi::redo(const U2DataId& objId, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    QString errorDescr = SQLiteL10N::tr("Can't redo an operation for the object!");

    // Get the object
    U2Object obj;
    getObject(obj, objId, os);
    if (os.hasError()) {
        coreLog.trace("Error getting an object: " + os.getError());
        os.setError(errorDescr);
        return;
    }

    // Verify that modifications tracking is enabled for the object
    if (TrackOnUpdate != obj.trackModType) {
        coreLog.trace("Called 'redo' for an object without modifications tracking enabled!");
        os.setError(errorDescr);
        return;
    }

    // Get all single modification steps
    QList< QList<U2SingleModStep> > modSteps = dbi->getSQLiteModDbi()->getModSteps(objId, obj.version, os);
    if (os.hasError()) {
        coreLog.trace("Error getting modSteps for an object: " + os.getError());
        os.setError(errorDescr);
        return;
    }

    foreach (QList<U2SingleModStep> multiStepSingleSteps, modSteps) {
        QSet<U2DataId> objectIds;

        foreach (U2SingleModStep modStep, multiStepSingleSteps) {
            if (U2ModType::isMsaModType(modStep.modType)) {
                dbi->getSQLiteMsaDbi()->redo(modStep.objectId, modStep.modType, modStep.details, os);
            }
            else if (U2ModType::isSequenceModType(modStep.modType)) {
                dbi->getSQLiteSequenceDbi()->redo(modStep.objectId, modStep.modType, modStep.details, os);
            }
            else if (U2ModType::isObjectModType(modStep.modType)) {
                if (U2ModType::objUpdatedName == modStep.modType) {
                    redoUpdateObjectName(modStep.objectId, modStep.details, os);
                    CHECK_OP(os, );
                }
                else {
                    coreLog.trace(QString("Can't redo an unknown operation: '%1'!").arg(QString::number(modStep.modType)));
                    os.setError(errorDescr);
                    return;
                }
            }

            objectIds.insert(modStep.objectId);
        }
        objectIds.insert(objId);

        foreach (U2DataId objId, objectIds) {
            incrementVersion(objId, os);
            if (os.hasError()) {
                coreLog.trace("Can't increment an object version!");
                os.setError(errorDescr);
                return;
            }
        }
    }
}

void SQLiteObjectDbi::undoUpdateObjectName(const U2DataId& id, const QByteArray& modDetails, U2OpStatus& os) {
    // Parse the input
    QString oldName;
    QString newName;
    bool ok = PackUtils::unpackObjectNameDetails(modDetails, oldName, newName);
    if (!ok) {
        os.setError("An error occurred during updating an object name!");
        return;
    }

    // Update the value
    SQLiteQuery q("UPDATE Object SET name = ?1 WHERE id = ?2", db, os);
    CHECK_OP(os, );

    q.bindString(1, oldName);
    q.bindDataId(2, id);
    q.update(1);
}

void SQLiteObjectDbi::redoUpdateObjectName(const U2DataId& id, const QByteArray& modDetails, U2OpStatus& os) {
    QString oldName;
    QString newName;
    bool ok = PackUtils::unpackObjectNameDetails(modDetails, oldName, newName);
    if (!ok) {
        os.setError("An error occurred during updating an object name!");
        return;
    }

    U2Object obj;
    getObject(obj, id, os);
    CHECK_OP(os, );

    obj.visualName = newName;
    updateObjectCore(obj, os);
    CHECK_OP(os, );
}

void SQLiteObjectDbi::removeParent(const U2DataId& parentId, const U2DataId& childId, bool removeDeadChild, U2OpStatus& os) {
    SQLiteQuery q("DELETE FROM Parent WHERE parent = ?1 AND child = ?2", db, os);
    q.bindDataId(1, parentId);
    q.bindDataId(2, childId);
    /*qint64 res = */q.update(1);
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


void SQLiteObjectDbi::ensureParent(const U2DataId& parentId, const U2DataId& childId, U2OpStatus& os) {
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

void SQLiteObjectDbi::incrementVersion(const U2DataId& objectId, DbRef* db, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    static const QString queryString("UPDATE Object SET version = version + 1 WHERE id = ?1");
    SQLiteQuery *q = t.getPreparedQuery(queryString, db, os);
    q->bindDataId(1, objectId);
    q->update(1);
}

qint64 SQLiteObjectDbi::getObjectVersion(const U2DataId& objectId, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    static const QString queryString("SELECT version FROM Object WHERE id = ?1");
    SQLiteQuery *q = t.getPreparedQuery(queryString, db, os);
    q->bindDataId(1, objectId);
    return q->selectInt64();
}

void SQLiteObjectDbi::setTrackModType(const U2DataId& objectId, U2TrackModType trackModType, U2OpStatus& os) {
    SQLiteQuery q("UPDATE Object SET trackMod = ?1 WHERE id = ?2", db, os);
    CHECK_OP(os, );

    q.bindInt32(1, trackModType);
    q.bindDataId(2, objectId);
    q.update(1);
    CHECK_OP(os, );

    { // update child objects
        SQLiteQuery q("UPDATE Object SET trackMod = ?1 WHERE id IN "
                      "(SELECT o.id FROM Object o, Parent p WHERE p.parent = ?2 AND p.child = o.id)", db, os);
        CHECK_OP(os, );
        q.bindInt32(1, trackModType);
        q.bindDataId(2, objectId);
        q.update(1);
    }
}

U2TrackModType SQLiteObjectDbi::getTrackModType(const U2DataId& objectId, U2OpStatus& os) {
    SQLiteQuery q("SELECT trackMod FROM Object WHERE id = ?1", db, os);
    CHECK_OP(os, NoTrack);

    q.bindDataId(1, objectId); 
    if (q.step()) {
        int res = q.getInt32(0);
        SAFE_POINT(res >= 0 && res < TRACK_MOD_TYPE_NR_ITEMS, "Incorrect trackMod value!", NoTrack);
        q.ensureDone();
        return (U2TrackModType)res;
    }
    else if (!os.hasError()) {
        os.setError(SQLiteL10N::tr("Object not found!"));
        return NoTrack;
    }

    return NoTrack;
}

U2DataId SQLiteObjectDbi::createObject(U2Object & object, const QString& folder, SQLiteDbiObjectRank rank, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    U2DataType type = object.getType();
    const QString &vname = object.visualName;
    int trackMod = object.trackModType;
    static const QString i1String("INSERT INTO Object(type, rank, name, trackMod) VALUES(?1, ?2, ?3, ?4)");
    SQLiteQuery *i1 = t.getPreparedQuery(i1String, db, os);
    i1->bindType(1, type);
    i1->bindInt32(2, rank);
    i1->bindString(3, vname);
    i1->bindInt32(4, trackMod);
    U2DataId res = i1->insert(type);
    if (os.hasError()) {
        return res;
    }
    if (!folder.isEmpty()) {
        assert(rank == SQLiteDbiObjectRank_TopLevel);
        qint64 folderId = getFolderId(folder, true, db, os);
        if (os.hasError()) {
            return res;
        }

        static const QString i2String("INSERT INTO FolderContent(folder, object) VALUES(?1, ?2)");
        SQLiteQuery *i2 = t.getPreparedQuery(i2String, db, os);
        i2->bindInt64(1, folderId);
        i2->bindDataId(2, res);    
        i2->execute();
    }

    object.id = res;
    object.dbiId = dbi->getDbiId();
    object.version = getObjectVersion(object.id, os);
    return res;
}

void SQLiteObjectDbi::getObject(U2Object& object, const U2DataId& id, U2OpStatus& os) {
    SQLiteQuery q("SELECT name, version, trackMod FROM Object WHERE id = ?1", db, os);
    q.bindDataId(1, id);
    if (q.step()) {
        object.id = id;
        object.dbiId = dbi->getDbiId();
        object.visualName = q.getString(0);
        object.version = q.getInt64(1);
        int trackMod = q.getInt32(2);
        if (trackMod >= 0 && trackMod < TRACK_MOD_TYPE_NR_ITEMS) {
            object.trackModType = (U2TrackModType)trackMod;
        }
        else {
            os.setError("Incorrect trackMod value in an object!");
            object.trackModType = NoTrack;
        }
        q.ensureDone();
    } else if (!os.hasError()) {
        os.setError(SQLiteL10N::tr("Object not found."));
    }
}

void SQLiteObjectDbi::updateObject(U2Object& obj, U2OpStatus& os) {
    updateObjectCore(obj, os);
    SAFE_POINT_OP(os, );

    obj.version = getObjectVersion(obj.id, os);
}

qint64 SQLiteObjectDbi::getFolderId(const QString& path, bool mustExist, DbRef* db, U2OpStatus& os) {
    static const QString queryString("SELECT id FROM Folder WHERE path = ?1");
    SQLiteQuery q(queryString, db, os);
    q.bindString(1, path);
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
    q.bindString(1, folder);
    return q.selectInt64();
}

qint64 SQLiteObjectDbi::getFolderGlobalVersion(const QString& folder, U2OpStatus& os) {
    SQLiteQuery q("SELECT vglobal FROM Folder WHERE path = ?1", db, os);
    q.bindString(1, folder);
    return q.selectInt64();
}

void SQLiteObjectDbi::onFolderUpdated(const QString& ) {
    //TODO: update local version of the given folder & global for all parents
}


//////////////////////////////////////////////////////////////////////////
// cross references dbi

SQLiteCrossDatabaseReferenceDbi::SQLiteCrossDatabaseReferenceDbi(SQLiteDbi* dbi)
: U2CrossDatabaseReferenceDbi(dbi), SQLiteChildDBICommon(dbi)
{
}

void SQLiteCrossDatabaseReferenceDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }
    // cross database reference object
    // factory - remote dbi factory
    // dbi - remote dbi id (url)
    // rid  - remote object id
    // version - remove object version
    SQLiteQuery("CREATE TABLE CrossDatabaseReference (object INTEGER, factory TEXT NOT NULL, dbi TEXT NOT NULL, "
                            "rid BLOB NOT NULL, version INTEGER NOT NULL, "
                            " FOREIGN KEY(object) REFERENCES Object(id) )", db, os).execute();
}


void SQLiteCrossDatabaseReferenceDbi::createCrossReference(U2CrossDatabaseReference& reference, U2OpStatus& os) {
    dbi->getSQLiteObjectDbi()->createObject(reference, QString(), SQLiteDbiObjectRank_TopLevel, os);
    if (os.hasError()) {
        return;
    }

    SQLiteQuery q("INSERT INTO CrossDatabaseReference(object, factory, dbi, rid, version) VALUES(?1, ?2, ?3, ?4, ?5)", db, os);
    q.bindDataId(1, reference.id);
    q.bindString(2, reference.dataRef.dbiRef.dbiFactoryId);
    q.bindString(3, reference.dataRef.dbiRef.dbiId);
    q.bindBlob(4, reference.dataRef.entityId);
    q.bindInt64(5, reference.dataRef.version);
    q.execute();
}

U2CrossDatabaseReference SQLiteCrossDatabaseReferenceDbi::getCrossReference(const U2DataId& objectId, U2OpStatus& os) {
    U2CrossDatabaseReference res(objectId, dbi->getDbiId(), 0);
    SQLiteQuery q("SELECT r.factory, r.dbi, r.rid, r.version, o.name, o.version FROM CrossDatabaseReference AS r, Object AS o "
        " WHERE o.id = ?1 AND r.object = o.id", db, os);
    q.bindDataId(1, objectId);
    if (q.step())  {
        res.dataRef.dbiRef.dbiFactoryId= q.getString(0);
        res.dataRef.dbiRef.dbiId = q.getString(1);
        res.dataRef.entityId = q.getBlob(2);
        res.dataRef.version = q.getInt64(3);
        res.visualName = q.getString(4);
        res.version = q.getInt64(5);
        q.ensureDone();
    } 
    return res;
}

void SQLiteCrossDatabaseReferenceDbi::updateCrossReference(const U2CrossDatabaseReference& reference, U2OpStatus& os) {
    SQLiteQuery q("UPDATE CrossDatabaseReference SET factory = ?1, dbi = ?2, rid = ?3, version = ?4 WHERE object = ?5", db, os);
    q.bindString(1, reference.dataRef.dbiRef.dbiFactoryId);
    q.bindString(2, reference.dataRef.dbiRef.dbiId);
    q.bindBlob(3, reference.dataRef.entityId);
    q.bindInt64(4, reference.dataRef.version);
    q.bindDataId(5, reference.id);
    q.execute();    
}


ModificationAction::ModificationAction(SQLiteDbi* _dbi, const U2DataId& _masterObjId)
    : dbi(_dbi),
      masterObjId(_masterObjId),
      trackMod(NoTrack)
{
    objIds.insert(masterObjId);
}

U2TrackModType ModificationAction::prepare(U2OpStatus& os) {
    trackMod = dbi->getObjectDbi()->getTrackModType(masterObjId, os);
    if (os.hasError()) {
        trackMod = NoTrack;
        FAIL("Failed to get trackMod!", NoTrack);
    }

    if (TrackOnUpdate == trackMod) {
        masterObjVersionToTrack = dbi->getObjectDbi()->getObjectVersion(masterObjId, os);
        SAFE_POINT_OP(os, trackMod);

        // If a user mod step has already been created for this action
        // then it can not be deleted. The version must be incremented.
        if (dbi->getSQLiteModDbi()->isUserStepStarted()) {
            masterObjVersionToTrack++;
        }

        dbi->getSQLiteModDbi()->removeModsWithGreaterVersion(masterObjId, masterObjVersionToTrack, os);
        if (os.hasError()) {
            dbi->getSQLiteModDbi()->cleanUpAllStepsOnError();
            return trackMod;
        }
    }

    return trackMod;
}

void ModificationAction::addModification(const U2DataId& objId, qint64 modType, const QByteArray& modDetails, U2OpStatus& os) {
    objIds.insert(objId);

    if (TrackOnUpdate == trackMod) {
        SAFE_POINT(!modDetails.isEmpty(), "Empty modification details!", );

        qint64 objVersion = dbi->getObjectDbi()->getObjectVersion(objId, os);
        SAFE_POINT_OP(os, );

        U2SingleModStep singleModStep;
        singleModStep.objectId = objId;
        singleModStep.version = objVersion;
        singleModStep.modType = modType;
        singleModStep.details = modDetails;

        singleSteps.append(singleModStep);
    }
}

void ModificationAction::complete(U2OpStatus& os) {
    // Save modification tracks, if required
    if (TrackOnUpdate == trackMod) {
        if (0 == singleSteps.size()) {
            // do nothing
        }
        else if (1 == singleSteps.size()) {
            dbi->getSQLiteModDbi()->createModStep(masterObjId, singleSteps.first(), os);
            SAFE_POINT_OP(os, );
        }
        else {
            U2UseCommonMultiModStep multi(dbi, masterObjId, os);
            SAFE_POINT_OP(os, );

            foreach (U2SingleModStep singleStep, singleSteps) {
                dbi->getSQLiteModDbi()->createModStep(masterObjId, singleStep, os);
                SAFE_POINT_OP(os, );
            }
        }
    }

    // Increment versions of all objects
    foreach (const U2DataId& objId, objIds) {
        SQLiteObjectDbi::incrementVersion(objId, dbi->getDbRef(), os);
        SAFE_POINT_OP(os, );
    }
}


/************************************************************************/
/* SQLiteObjectDbiUtils */
/************************************************************************/
void SQLiteObjectDbiUtils::renameObject(SQLiteDbi *dbi, U2Object &object, const QString &newName, U2OpStatus &os) {
    SAFE_POINT(NULL != dbi, "NULL dbi!", );
    SQLiteTransaction t(dbi->getDbRef(), os);
    Q_UNUSED(t);

    ModificationAction updateAction(dbi, object.id);
    U2TrackModType trackMod = updateAction.prepare(os);
    SAFE_POINT_OP(os, );

    renameObject(updateAction, dbi, object, newName, os);
    SAFE_POINT_OP(os, );

    // Increment version; track the modification, if required
    updateAction.complete(os);
    SAFE_POINT_OP(os, );
}

void SQLiteObjectDbiUtils::renameObject(ModificationAction& updateAction, SQLiteDbi *dbi, U2Object &object, const QString &newName, U2OpStatus &os) {
    SAFE_POINT(NULL != dbi, "NULL dbi!", );
    SQLiteTransaction t(dbi->getDbRef(), os);
    Q_UNUSED(t);

    QByteArray modDetails;
    if (TrackOnUpdate == updateAction.getTrackModType()) {
        modDetails = PackUtils::packObjectNameDetails(object.visualName, newName);
    }

    object.visualName = newName;
    dbi->getSQLiteObjectDbi()->updateObject(object, os);
    SAFE_POINT_OP(os, );

    updateAction.addModification(object.id, U2ModType::objUpdatedName, modDetails, os);
    SAFE_POINT_OP(os, );
}

} //namespace
