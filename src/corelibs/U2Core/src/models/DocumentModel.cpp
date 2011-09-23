
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

#include <U2Core/DocumentModel.h>
#include <U2Core/GObject.h>
#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/GHints.h>
#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/GUrlUtils.h>

#include <U2Core/GObjectUtils.h>
#include <U2Core/UnloadedObject.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QFileInfo>

#include <memory>

/* TRANSLATOR U2::DocumentFormat */

namespace U2 {

const QString DocumentFormat::CREATED_NOT_BY_UGENE = DocumentFormat::tr( "The document is created not by UGENE" );
const QString DocumentFormat::MERGED_SEQ_LOCK = DocumentFormat::tr( "Document sequences were merged" );
const QString DocumentMimeData::MIME_TYPE("application/x-ugene-document-mime");

Document* DocumentFormat::createNewDocument(IOAdapterFactory* io, const GUrl& url, const QVariantMap& hints) {
    Document* d = new Document(this, io, url, QList<UnloadedObjectInfo>(), hints);
    d->setLoaded(true);
    return d;
}

Document* DocumentFormat::loadDocument(IOAdapterFactory* iof, const GUrl& url, TaskStateInfo& ti, const QVariantMap& hints) {
    std::auto_ptr<IOAdapter> io(iof->createIOAdapter());
    if (!io->open(url, IOAdapterMode_Read)) {
        ti.setError(L10N::errorOpeningFileRead(url));
        return NULL;
    }
    Document * doc = loadDocument(io.get(), ti, hints, DocumentLoadMode_Whole);
    return doc;
}

GObject* DocumentFormat::loadObject(IOAdapter*, TaskStateInfo& ti) {
    ti.setError("This document format is not support streaming reading mode");
    return NULL;
}

DNASequence* DocumentFormat::loadSequence(IOAdapter*, TaskStateInfo& ti) {
    ti.setError("This document format is not support streaming reading mode");
    return NULL;
}

void DocumentFormat::storeDocument( Document* , TaskStateInfo& ts, IOAdapter* ) {
    assert(0);
    ts.setError(tr("Writing is not supported for this format (%1). Feel free to send a feature request though.").arg(getFormatName()));
}

void DocumentFormat::storeDocument(Document* doc, TaskStateInfo& ti, IOAdapterFactory* iof, const GUrl& newDocURL) {
    if (!formatFlags.testFlag(DocumentFormatFlag_SupportWriting)) {
        assert(0);
        ti.setError(tr("Writing is not supported for this format (%1). Feel free to send a feature request though.").arg(getFormatName()));
        return;
    }
    
    assert(doc->getDocumentModLock(DocumentModLock_FORMAT_AS_INSTANCE) == NULL);
    if (iof == NULL) {
        iof = doc->getIOAdapterFactory();
    }
    
    //prepare URL
    GUrl url = newDocURL.isEmpty() ? doc->getURL() : newDocURL;
    if (url.isLocalFile()) {
        QString error;
        QString res = GUrlUtils::prepareFileLocation(url.getURLString(), error);
        if (!error.isEmpty()) {
            ti.setError(error);
            return;
        }
        Q_UNUSED(res);
        assert(res == url.getURLString()); //ensure that GUrls are always canonical
    }
    
    std::auto_ptr<IOAdapter> io(iof->createIOAdapter());
    if (!io->open(url, IOAdapterMode_Write)) {
        ti.setError(L10N::errorOpeningFileWrite(url));
        return;
    }
    
    storeDocument(doc, ti, io.get());
}

bool DocumentFormat::checkConstraints(const DocumentFormatConstraints& c) const {
    assert(!supportedObjectTypes.isEmpty()); //extra check for DF state validation

    if (!checkFlags(c.flagsToSupport)) {
        return false; //requested to support writing or streaming but doesn't
    }

    if ((int(c.flagsToExclude) & int(formatFlags)) != 0) {
        return false; // filtered by exclude flags
    }

    if (c.checkRawData && checkRawData(c.rawData).score < c.minDataCheckResult) {
        return false; //raw data is not matched
    }

    foreach (GObjectType objType, c.supportedObjectTypes) {
        if (!supportedObjectTypes.contains(objType)) { // the object type is not in the supported list
            return false;
        }
    }
    
    return true;
}

bool DocumentFormat::isObjectOpSupported(const Document* d, DocObjectOp op, GObjectType t) const {
    assert(d->getDocumentFormat() == this);

    if (!supportedObjectTypes.contains(t)) {
        return false;
    }
    
    if (!checkFlags(DocumentFormatFlag_SupportWriting)) {
        return false;
    }

    if (op == DocObjectOp_Add) {
        int nObjects = d->getObjects().size();
        if (nObjects != 0 && checkFlags(DocumentFormatFlag_SingleObjectFormat)) {
            return false;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
///Document

Document::Document(DocumentFormat* _df, IOAdapterFactory* _io, const GUrl& _url, const QList<UnloadedObjectInfo>& unloadedObjects,
                   const QVariantMap& hints, const QString& instanceModLockDesc)
                   : StateLockableTreeItem(), df(_df), io(_io), url(_url)
{
    ctxState = new GHintsDefaultImpl(hints);
    name = url.fileName();
    
    qFill(modLocks, modLocks + DocumentModLock_NUM_LOCKS, (StateLock*)NULL);    

    loadStateChangeMode = true;
    addUnloadedObjects(unloadedObjects);
    loadStateChangeMode = false;
    
    initModLocks(instanceModLockDesc, false);
    checkUnloadedState();
    assert(!isModified());
}

Document::Document(DocumentFormat* _df, IOAdapterFactory* _io, const GUrl& _url, 
                   const QList<GObject*>& _objects, const QVariantMap& hints, const QString& instanceModLockDesc)
                   : StateLockableTreeItem(), df(_df), io(_io), url(_url)
{
    ctxState = new GHintsDefaultImpl(hints);
    name = url.fileName();

    loadStateChangeMode = true;
    qFill(modLocks, modLocks + DocumentModLock_NUM_LOCKS, (StateLock*)NULL);
    foreach(GObject* o, _objects) {
        _addObject(o);
    }
    loadStateChangeMode = false;

    initModLocks(instanceModLockDesc, true);
    
    checkLoadedState();
    assert(!isModified());
}

Document::~Document() {
    for (int i=0;i<DocumentModLock_NUM_LOCKS; i++) {
        StateLock* sl = modLocks[i];
        if (sl!=NULL) {
            unlockState(sl);
            delete sl;
        }
    }
    foreach(GObject* obj, objects) {
        obj->setGHints(NULL);
    }
    delete ctxState;
}

void Document::addObject(GObject* obj){
    assert(obj != NULL && obj->getDocument()==NULL);
    assert(df->isObjectOpSupported(this, DocumentFormat::DocObjectOp_Add, obj->getGObjectType()));
    //assert(findGObjectByName(obj->getGObjectName())==NULL);
    assert(isLoaded());
    assert(obj->getGObjectType()!=GObjectTypes::UNLOADED);
    assert(!obj->isTreeItemModified());

    _addObject(obj);
}

void Document::_addObjectToHierarchy(GObject* obj) {
    obj->setParentStateLockItem(this);
    obj->setGHints(new ModTrackHints(this, obj->getGHintsMap(), true));
    obj->setModified(false);
    objects.append(obj);
}

void Document::_addObject(GObject* obj) {
    _addObjectToHierarchy(obj);
    assert(objects.size() == getChildItems().size());
    emit si_objectAdded(obj);
}
void Document::removeObject(GObject* obj) {
    assert(df->isObjectOpSupported(this, DocumentFormat::DocObjectOp_Remove, obj->getGObjectType()));
    assert(!obj->isTreeItemModified());
    _removeObject(obj);
}

void Document::_removeObject(GObject* obj, bool deleteObjects) {
    assert(obj->getParentStateLockItem() == this);
    obj->setModified(false);

    obj->setParentStateLockItem(NULL);
    objects.removeOne(obj);
    obj->setGHints(new GHintsDefaultImpl());

    assert(objects.size() == getChildItems().size());

    emit si_objectRemoved(obj);
    
    if (deleteObjects) {
        delete obj;
    }
}


void Document::makeClean()  {
    if (!isTreeItemModified()) {
        return;
    }
    setModified(false);
    foreach(GObject* obj, objects) {
        obj->setModified(false);
    }
}



GObject* Document::findGObjectByName(const QString& name) const {
    foreach(GObject* obj, objects) {
        if (obj->getGObjectName() == name) {
            return obj;
        }
    }
    return NULL;
}

QList<GObject*> Document::findGObjectByType(GObjectType t, UnloadedObjectFilter f) const {
    return GObjectUtils::select(objects, t, f);
}


void Document::checkUnloadedState() const {
#ifdef _DEBUG
    assert(!isLoaded());
    bool hasNoLoadedObjects = findGObjectByType(GObjectTypes::UNLOADED, UOF_LoadedAndUnloaded).count() == objects.count();
    assert(hasNoLoadedObjects);
    checkUniqueObjectNames();
#endif
}

void Document::checkUniqueObjectNames() const {
#ifdef _DEBUG
    QSet<QString> names;
    foreach(GObject* o, objects) {
        const QString& name = o->getGObjectName();
        assert(!names.contains(name));
        names.insert(name);
    }
#endif
}
void Document::checkLoadedState() const {
#ifdef _DEBUG
    assert(isLoaded());
    bool hasNoUnloadedObjects = findGObjectByType(GObjectTypes::UNLOADED, UOF_LoadedAndUnloaded).isEmpty();
    assert(hasNoUnloadedObjects);
    checkUniqueObjectNames();
#endif
}


static StateLock* NULL_LOCK = NULL;

void Document::loadFrom(Document* sourceDoc) {
    SAFE_POINT(!isLoaded(), QString("Document is already loaded: ").arg(getURLString()), )

    sourceDoc->checkLoadedState();
    checkUnloadedState();

    loadStateChangeMode = true;

    QMap<QString, UnloadedObjectInfo> unloadedInfo;
    foreach(GObject* obj, objects) { //remove all unloaded objects but save hints
        unloadedInfo.insert(obj->getGObjectName(), UnloadedObjectInfo(obj));
        _removeObject(obj);
    }
    ctxState->setAll(sourceDoc->getGHints()->getMap());
    
    lastUpdateTime = sourceDoc->getLastUpdateTime();

    //copy instance mod-locks if any
    StateLock* mLock = modLocks[DocumentModLock_FORMAT_AS_INSTANCE];
    StateLock* dLock = sourceDoc->modLocks[DocumentModLock_FORMAT_AS_INSTANCE];
    if (mLock != NULL) {
        if (dLock == NULL) {
            unlockState(mLock);
            delete mLock;
            modLocks[DocumentModLock_FORMAT_AS_INSTANCE] = NULL;
        } else {
            mLock->setUserDesc(dLock->getUserDesc());
        }
    } else if (dLock!=NULL) {
        modLocks[DocumentModLock_FORMAT_AS_INSTANCE] = new StateLock(dLock->getUserDesc());
        lockState(modLocks[DocumentModLock_FORMAT_AS_INSTANCE]);
    }
    
    QList<GObject*> sourceObjects = sourceDoc->getObjects();
    sourceDoc->unload(false);
    foreach(GObject* obj, sourceObjects) {
        //TODO: add constrains to ObjectRelations!!
        UnloadedObjectInfo info = unloadedInfo.value(obj->getGObjectName());
        if (info.type == obj->getGObjectType()) {
            QVariantMap mergedHints = obj->getGHintsMap();
            foreach(const QString& k, info.hints.keys()) {
                if (!mergedHints.contains(k)) {
                    mergedHints.insert(k, info.hints.value(k));
                }
            }
            obj->getGHints()->setMap(mergedHints);
        }
        _addObject(obj);
    }
    
    setLoaded(true); 
    
    //TODO: rebind local objects relations if url!=d.url
    
    loadStateChangeMode = false;
    
    checkLoadedState();
}

void Document::setLoaded(bool v) {
    if (v == isLoaded()) {
        return;
    }
    StateLock* l = modLocks[DocumentModLock_UNLOADED_STATE];
    if (v) {
        unlockState(l);
        modLocks[DocumentModLock_UNLOADED_STATE] = NULL;
        delete l;
        checkLoadedState();
    } else {
        assert(l == NULL);
        l = new StateLock(tr("Document is not loaded"));
        modLocks[DocumentModLock_UNLOADED_STATE] = l;
        lockState(l);
        checkUnloadedState();
    }
    emit si_loadedStateChanged();
}

void Document::initModLocks(const QString& instanceModLockDesc, bool loaded) {
    setLoaded(loaded);
    
    // must be locked for modifications if io-adapter does not support writes
    if (!io->isIOModeSupported(IOAdapterMode_Write)) {
        modLocks[DocumentModLock_IO] = new StateLock(tr("IO adapter does not support write operation"));
        lockState(modLocks[DocumentModLock_IO]);
    }

    // must be locked for modifications if not document format does not support writes
    if (!df->checkFlags(DocumentFormatFlag_SupportWriting)) {
        modLocks[DocumentModLock_FORMAT_AS_CLASS] = new StateLock(tr("No write support for document format"));
        lockState(modLocks[DocumentModLock_FORMAT_AS_CLASS]);
    }

    if (!instanceModLockDesc.isEmpty()) {
        modLocks[DocumentModLock_FORMAT_AS_INSTANCE] = new StateLock(instanceModLockDesc);
        lockState(modLocks[DocumentModLock_FORMAT_AS_INSTANCE]);
    }
}

void Document::setName(const QString& newName) {
    if (name == newName) {
        return;
    }
    name = newName;
    emit si_nameChanged();
}

void Document::setURL(const GUrl& newUrl) {
    assert(!isLoaded() || !isStateLocked());
    if (url == newUrl) {
        return;
    }
    url = newUrl;
    emit si_urlChanged();
}

bool Document::checkConstraints(const Document::Constraints& c) const {
    if (c.stateLocked != TriState_Unknown) {
        if (c.stateLocked == TriState_No && isStateLocked()) {
            return false;
        }
        if (c.stateLocked == TriState_Yes && !isStateLocked()) {
            return false;
        }
    }

    if (!c.formats.isEmpty()) {
        bool found = false;
        foreach(DocumentFormatId f, c.formats) {
            if (df->getFormatId() == f) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    foreach(DocumentModLock l, c.notAllowedStateLocks) {
        if (modLocks[l]!=NULL) {
            return false;
        }
    }
    
    if (!c.objectTypeToAdd.isNull() && !df->isObjectOpSupported(this, DocumentFormat::DocObjectOp_Add, c.objectTypeToAdd)) {
        return false;
    }

    
    return true;
}


void Document::setUserModLock(bool v) {
    if (hasUserModLock() == v) {
        return;
    }
    if (v) {
        StateLock* sl = new StateLock(tr("Locked by user"));
        modLocks[DocumentModLock_USER] = sl;
        lockState(sl);
    } else {
        StateLock* sl = modLocks[DocumentModLock_USER];
        modLocks[DocumentModLock_USER] = NULL;
        unlockState(sl);
        delete sl;
    }

    //hack: readonly settings are stored in project, so if document is in project -> mark project as modified
    if (getParentStateLockItem()!=NULL) { 
        getParentStateLockItem()->setModified(true);
    }
}

bool Document::unload(bool deleteObjects) {
    assert(isLoaded());
    
    bool liveLocked = hasLocks(StateLockableTreeFlags_ItemAndChildren, StateLockFlag_LiveLock);
    if (liveLocked) {
        assert(0);
        return false;
    }

    loadStateChangeMode = true;

    QList<UnloadedObjectInfo> unloadedInfo;
    foreach(GObject* obj, objects) { //Note: foreach copies object list
        unloadedInfo.append(UnloadedObjectInfo(obj));
        _removeObject(obj, deleteObjects);
    }
    addUnloadedObjects(unloadedInfo);
    
    StateLock* fl = modLocks[DocumentModLock_FORMAT_AS_INSTANCE];
    if (fl != NULL) {
        unlockState(fl);
        modLocks[DocumentModLock_FORMAT_AS_INSTANCE] = NULL;
    }
    
    setLoaded(false);
    
    loadStateChangeMode = false;

    return true;
}

void Document::setModified(bool modified, const QString& modType) {
    if (loadStateChangeMode && modified && modType == StateLockModType_AddChild) { //ignore modification events during loading/unloading
        return;    
    }
    StateLockableTreeItem::setModified(modified, modType);
}

bool Document::isModificationAllowed(const QString& modType) {
    bool ok = loadStateChangeMode && modType == StateLockModType_AddChild;
    ok = ok || StateLockableTreeItem::isModificationAllowed(modType);
    return ok;
}


void Document::setGHints(GHints* newHints) {
    assert(newHints!=NULL);
    //gobjects in document keep states in parent document map -> preserve gobject hints
    QList<QVariantMap> objectHints;
    for (int i = 0; i < objects.size(); i++) {
        GObject* obj = objects[i];
        objectHints.append(obj->getGHintsMap());
    }
    
    delete ctxState;
    ctxState = newHints;

    for (int i = 0;i < objects.size(); i++) {
        const QVariantMap& hints = objectHints[i];
        GObject* obj = objects[i];
        obj->getGHints()->setMap(hints);
    }
}

void Document::addUnloadedObjects(const QList<UnloadedObjectInfo>& info) {
    foreach(const UnloadedObjectInfo& oi, info) {
        UnloadedObject* obj = new UnloadedObject(oi);
        _addObjectToHierarchy(obj);
        assert(obj->getDocument() == this);
        emit si_objectAdded(obj);    
    }
}

QVariantMap Document::getGHintsMap() const {
    return ctxState->getMap();
}
void Document::setupToEngine(QScriptEngine *engine)
{
    qScriptRegisterMetaType(engine, toScriptValue, fromScriptValue);
};
QScriptValue Document::toScriptValue(QScriptEngine *engine, Document* const &in)
{
    return engine->newQObject(in); 
};
void Document::fromScriptValue(const QScriptValue &object, Document* &out) 
{
    out = qobject_cast<Document*>(object.toQObject()); 
};

void Document::setLastUpdateTime() {
    QFileInfo fi(getURLString());
    if (fi.exists()) {
        lastUpdateTime = fi.lastModified();
    }
}
void Document::propagateModLocks(Document* doc)  const {
    for (int i = 0; i < DocumentModLock_NUM_LOCKS; i++) {
        StateLock* lock = modLocks[i];
        if (lock != NULL && doc->modLocks[i] != NULL) {
            StateLock* newLock = new StateLock(lock->getUserDesc(), lock->getFlags());
            doc->modLocks[i] = newLock;
            doc->lockState(newLock);
        }
    }
}


}//namespace

