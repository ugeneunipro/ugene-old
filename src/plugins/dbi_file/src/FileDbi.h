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

#ifndef _U2_FILE_DBI_H_
#define _U2_FILE_DBI_H_

#include <U2Core/U2Dbi.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/GUrl.h>
#include <U2Core/DocumentModel.h>

#include "AnnotationObjectDbi.h"
#include "MsaObjectDbi.h"

namespace U2 {

class Task;
class MAlignmentObject;
class DNASequenceObject;
class AnnotationTableObject;
class Annotation;
class DNAAlphabet;

class FileDbi;

/**
An interface used to read topObjects info
*/
class ObjFolderDbi : public U2FolderDbi {
    friend class FileDbi;
protected:
    ObjFolderDbi(FileDbi* rootDbi);
    FileDbi* root;

public:

    // Object methods
     
    /**  Returns number of top-level U2Objects in database */
    virtual qint64 countObjects(U2OpStatus& os);

    /**  Returns number of top-level U2Objects in database */
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

    /** Removes object from database */
    virtual void removeObject(U2DataId dataId, const QString& folder, U2OpStatus& os);

    /** Removes collection of objects from database */
    virtual void removeObjects(const QList<U2DataId>& dataIds, const QString& folder, U2OpStatus& os);
    // Folder methods

    /**  Returns list of topObjects stored in database. 
    Folders are separated by '/' character.
    At least one root folder is required. 
    */
    virtual QStringList getFolders(U2OpStatus& os);

    /** Returns version of the folder. 
        The folder version increases if new object(s)/subfolder(s) are added into this folder
        Note that if object(s)/folder(s) are added into one of the subfolders the folder version is not changed
    */
    virtual qint64 getFolderLocalVersion(const QString& folder, U2OpStatus& os) {return 0;}

    /** Returns version of the folder that changes every time object(s)/folder(s) added 
        to the specified folder or any of its child folders
    */
    virtual qint64 getFolderGlobalVersion(const QString& folder, U2OpStatus& os) {return 0;}
    

    /** Returns version of the given object */
    virtual qint64 getObjectVersion(U2DataId objectId, U2OpStatus& os) {return 0;}

    /** Returns number of top-level U2Objects in folder */
    virtual qint64 countObjects(const QString& folder, U2OpStatus& os);

    /** Lists database top-level objects of the specified type, starts with 'offset' and limits by 'count' */
    virtual QList<U2DataId> getObjects(const QString& folder, qint64 offset, qint64 count, U2OpStatus& os);

    /**  Returns all topObjects this object must be shown in  */
    virtual QStringList getObjectFolders(U2DataId objectId, U2OpStatus& os);

    /** Creates folder in the database.
    The specified path must be valid unique path, not existing in the database.
    It is not required that parent topObjects must exist, they are created automatically.
    */
    virtual void createFolder(const QString& path, U2OpStatus& os);

    /** Moves objects between topObjects.
    'fromFolder' must be existing path containing all specified objects.
    'toFolder' must be existing path or empty string.
    If 'toFolder' is empty, removes the objects from 'fromFolder' and 
    deletes non-top-level objects without parents, if any appear in the specified list.
    Otherwise, moves the specified objects between the specified topObjects, omitting duplicates.
    */
    virtual void moveObjects(const QList<U2DataId>& objectId, const QString& fromFolder, const QString& toFolder, U2OpStatus& os);

    /** Adds objects to the specified folder.
    All objects must exist and have a top-level type.
    */
    virtual void addObjectsToFolder(const QList<U2DataId>& objectIds, const QString& toFolder, U2OpStatus& os);

    /** Removes folder. The folder must be existing path. Runs GC check for all objects in the folder */
    virtual void removeFolder(const QString& folder, U2OpStatus& os);
};

/**
An interface to obtain access to sequence objects
*/
class SequenceObjectDbi : public U2SequenceRWDbi {
protected:
    friend class FileDbi;
    SequenceObjectDbi(FileDbi* rootDbi);

public:
    /** Reads sequence object from database */
    virtual U2Sequence getSequenceObject(U2DataId sequenceId, U2OpStatus& os);

    /** Reads specified sequence data region from database.
    The region must be valid region within sequence bounds. */
    virtual QByteArray getSequenceData(U2DataId sequenceId, const U2Region& region, U2OpStatus& os);

    /**  Adds empty sequence instance into database */
    virtual void createSequenceObject(U2Sequence& sequence, const QString& folder, U2OpStatus& os);

    /** 
    Updates sequence region. 
    Note: regionToReplace length can differ from dataToInsert length, so the method can be used to add/remove sequence regions
    */
    virtual void updateSequenceData(U2DataId sequenceId, const U2Region& regionToReplace, const QByteArray& dataToInsert, U2OpStatus& os);
private:
    FileDbi* root;
};


class FileDbi : public U2AbstractDbi {
public:

    FileDbi();
    ~FileDbi();

    virtual void init(const QHash<QString, QString>& properties, const QVariantMap& persistentData, U2OpStatus& os);
    /** Stops the database and frees up used resources. */
    virtual QVariantMap shutdown(U2OpStatus& os);

    virtual bool flush(U2OpStatus& os){ return true;}

    /** Returns database meta-info. Any set of properties to be shown to user */
    virtual QHash<QString, QString> getDbiMetaInfo(U2OpStatus& os);

    /** Unique database id. Used for cross-database references. */
    virtual QString getDbiId() const;


    virtual U2DataType getEntityTypeById(U2DataId id) const;

    /** Database interface to access objects. Must not be NULL! */
    virtual U2ObjectRDbi* getObjectRDbi() {return folderDbi;}

    /**  Generic database interface to remove objects from database */
    virtual U2ObjectRWDbi* getObjectRWDbi() {return folderDbi;}

    /** Database interface to manipulate with topObjects */
    virtual U2FolderDbi* getFolderDbi() {return folderDbi;}

    /**  U2Sequence read ops. Not null if DBI supports the whole set of sequence reading operations */
    virtual U2SequenceRDbi* getSequenceRDbi() {return seqDbi;}

    /** U2Sequence read and write ops. Not null if DBI supports the whole set of sequence writing operations */
    virtual U2SequenceRWDbi* getSequenceRWDbi() {return seqDbi;}

    /**  U2Annotation read ops. Not null if DBI supports the whole set of annotation reading operations */
    virtual U2AnnotationRDbi* getAnnotationRDbi() {return annDbi;}

    /** U2Annotation write ops. Not null if DBI supports the whole set of annotation writing operations */
    virtual U2AnnotationRWDbi* getAnnotationRWDbi() {return annDbi;}

    /** 
    U2Msa read ops. Not null if DBI supports the whole set of msa reading operations
    Support of U2MsaRDbi requires U2SequenceRDbi support
    */
    virtual U2MsaRDbi* getMsaRDbi() {return msaDbi;}

    /** 
    U2Msa read ops. Not null if DBI supports the whole set of msa reading operations
    Support of U2MsaRWDbi requires U2SequenceRWDbi support
    */        
    virtual U2MsaRWDbi* getMsaRWDbi() {return msaDbi;}

    /** 
    U2Assembly read ops. Not null if DBI supports the whole set of assembly reading operations
    U2AssemblyRDbi requires U2SequenceRDbi support
    */
    virtual U2AssemblyRDbi* getAssemblyRDbi() {return assemblyDbi;}

    /** 
    U2Msa read ops. Not null if DBI supports the whole set of msa reading operations
    Support of U2MsaRWDbi requires U2SequenceRWDbi support
    */        
    virtual U2AssemblyRWDbi* getAssemblyRWDbi() {return assemblyDbi;}

    /**  U2Attribute read ops. Not null if DBI supports the whole set of attributes reading operations */
    virtual U2AttributeRDbi* getAttributeRDbi() {return NULL;}

    /**  U2Annotation write ops. Not null if DBI supports the whole set of attribute writing operations */
    virtual U2AttributeRWDbi* getAttributeRWDbi() {return NULL;}

    virtual QHash<QString, QString> getInitProperties() const {return initProps;}
private:
    Document* doc;
    GUrl url;
    mutable QReadWriteLock* sync;
    QHash<QString, QString> initProps;

    QMultiMap<MAlignmentObject*, DNASequenceObject*> msaSeqs;
    QMap<quint64, Annotation*> annIds;
    QMultiMap<GObject*, QString> topObjects;
    QList<QString> folders;
    QVariantMap saved;

    SequenceObjectDbi* seqDbi;
    MsaObjectDbi* msaDbi;
    AssemblyObjectDbi* assemblyDbi;
    AnnotationObjectDbi* annDbi;
    ObjFolderDbi* folderDbi;

    bool isReady(U2OpStatus& os) const;
    bool canModify(U2OpStatus& os) const;
    bool canCreate(GObjectType t, U2OpStatus& os) const;
    QString newName(GObjectType t) const;
    DNAAlphabet* checkAlphabet(const QString& id, U2OpStatus& os) const;
    bool checkFolder(const QString& path, U2OpStatus& os) const;

    MAlignmentObject* findMA(U2DataId id, U2OpStatus& os) const;
    DNASequenceObject* findSeq(U2DataId id, U2OpStatus& os) const;
    GObject* findTop(U2DataId id, U2OpStatus& os) const;
    Annotation* findAntn(U2DataId an, U2OpStatus& os) const;

    U2DataId an2id(Annotation* an) const;
    U2DataId obj2id(GObject* go) const;

friend class MsaObjectDbi;
friend class AssemblyObjectDbi;
friend class DocObjectDbi;
friend class SequenceObjectDbi;
friend class AnnotationObjectDbi;
friend class ObjFolderDbi;
};

class FileDbiFactory : public U2DbiFactory {
public:
    virtual U2Dbi *createDbi();
    virtual U2DbiFactoryId getId()const;

    /** Checks that data pointed by properties is a valid DBI resource */
    virtual bool isValidDbi(const QHash<QString, QString>& properties, const QByteArray& rawData, U2OpStatus& os) const {return false;}

public:
    static const U2DbiFactoryId ID;
};

class FileDbiL10N : public QObject {

};

} //ns
#endif
