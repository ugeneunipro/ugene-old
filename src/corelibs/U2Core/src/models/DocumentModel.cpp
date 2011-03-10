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

    if (c.checkRawData && checkRawData(c.rawData) < c.minDataCheckResult) {
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
    
    addUnloadedObjects(unloadedObjects);
    initModLocks(instanceModLockDesc, false);
    checkUnloadedState();
}

Document::Document(DocumentFormat* _df, IOAdapterFactory* _io, const GUrl& _url, 
                   const QList<GObject*>& _objects, const QVariantMap& hints, const QString& instanceModLockDesc)
: StateLockableTreeItem(), df(_df), io(_io), url(_url)
{
    ctxState = new GHintsDefaultImpl(hints);
    name = url.fileName();
    
    qFill(modLocks, modLocks + DocumentModLock_NUM_LOCKS, (StateLock*)NULL);

    foreach(GObject* o, _objects) {
        _addObject(o);
    }

    initModLocks(instanceModLockDesc, true);
    
    checkLoadedState();
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
        obj->setGHints(new GHintsDefaultImpl());
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

void Document::_addObjectToHierarchy(GObject* obj, bool ignoreLocks) {
    assert(!isStateLocked() || ignoreLocks);
    
    bool modify = !ignoreLocks;
    obj->setParentStateLockItem(this, ignoreLocks, modify);
    obj->setGHints(new ModTrackHints(this, obj->getGHintsMap(), true));
    objects.append(obj);
}

void Document::_addObject(GObject* obj, bool ignoreLocks) {
    obj->setModified(false);
    _addObjectToHierarchy(obj, ignoreLocks);

    assert(objects.size() == getChildItems().size());
    emit si_objectAdded(obj);
}
void Document::removeObject(GObject* obj) {
    assert(df->isObjectOpSupported(this, DocumentFormat::DocObjectOp_Remove, obj->getGObjectType()));
    assert(!obj->isTreeItemModified());
    _removeObject(obj);
}

void Document::_removeObject(GObject* obj, bool ignoreLocks) {
    assert(obj->getParentStateLockItem() == this);
    obj->setModified(false);

    bool modify = !ignoreLocks;
    obj->setParentStateLockItem(NULL, ignoreLocks, modify);
    objects.removeOne(obj);
    obj->setGHints(new GHintsDefaultImpl());

    assert(objects.size() == getChildItems().size());
    emit si_objectRemoved(obj);
    
    delete obj;
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

Document* Document::clone() const {
    Document* doc = new Document(df, io, url, QList<UnloadedObjectInfo>(), ctxState->getMap());
    doc->loadFrom(this);
    return doc;
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

void Document::loadFrom(const Document* d) {
    assert(d->getDocumentFormat() == df);
    assert(!isLoaded() && d->isLoaded());

    d->checkLoadedState();
    checkUnloadedState();

    int nDocLocks = 0; 
    qCount(modLocks, modLocks + DocumentModLock_NUM_LOCKS, NULL_LOCK, nDocLocks);
    nDocLocks = DocumentModLock_NUM_LOCKS - nDocLocks;

#ifdef _DEBUG
    int totalLocks = locks.count();    
    assert(totalLocks == nDocLocks);
#endif

    QMap<QString, UnloadedObjectInfo> unloadedInfo;
    foreach(GObject* obj, objects) { //remove all unloaded objects
        unloadedInfo.insert(obj->getGObjectName(), UnloadedObjectInfo(obj));
        _removeObject(obj, true);
    }

    ctxState->setMap(d->getGHints()->getMap());

    //copy instance modlocks if any
    StateLock* mLock = modLocks[DocumentModLock_FORMAT_AS_INSTANCE];
    if (mLock!=NULL) {
        unlockState(mLock);
        delete mLock;
        modLocks[DocumentModLock_FORMAT_AS_INSTANCE] = NULL;
    }
    StateLock* dLock = d->modLocks[DocumentModLock_FORMAT_AS_INSTANCE];
    if (dLock!=NULL) {
        modLocks[DocumentModLock_FORMAT_AS_INSTANCE] = new StateLock(dLock->getUserDesc());
        lockState(modLocks[DocumentModLock_FORMAT_AS_INSTANCE]);
    }

    setLoaded(true); 

    QList<GObject*> objects = d->getObjects();
    foreach(GObject* origObj, objects) {
        GObject* clonedObj = origObj->clone();
        //merge hints, TODO: add constrains to ObjectRelations!!
        UnloadedObjectInfo info = unloadedInfo.value(clonedObj->getGObjectName());
        if (info.type == clonedObj->getGObjectType()) {
            QVariantMap mergedHints = clonedObj->getGHintsMap();
            foreach(const QString& k, info.hints.keys()) {
                mergedHints.insert(k, info.hints.value(k));
            }
            clonedObj->getGHints()->setMap(mergedHints);
        }
        _addObject(clonedObj, true);
    }
    
    //TODO: rebind local objects relations if url!=d.url
    setModified(false);
    
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

bool Document::unload() {
    assert(isLoaded());
    
    bool liveLocked = hasLocks(StateLockableTreeFlags_ItemAndChildren, StateLockFlag_LiveLock);
    if (liveLocked) {
        assert(0);
        return false;
    }

    int nDocLocks = 0; 
    qCount(modLocks, modLocks + DocumentModLock_NUM_LOCKS, NULL_LOCK, nDocLocks);
    nDocLocks = DocumentModLock_NUM_LOCKS - nDocLocks;
    int totalLocks = locks.count();    
    if (nDocLocks!=totalLocks) {
        assert(0);
        return false;
    }
    QList<UnloadedObjectInfo> unloadedInfo;
    foreach(GObject* obj, objects) { //Note: foreach copies object list
        unloadedInfo.append(UnloadedObjectInfo(obj));
        _removeObject(obj, true);
    }
    addUnloadedObjects(unloadedInfo);
    
    StateLock* fl =modLocks[DocumentModLock_FORMAT_AS_INSTANCE];
    if (fl!=NULL) {
        unlockState(fl);
        modLocks[DocumentModLock_FORMAT_AS_INSTANCE] = NULL;
    }
    
    setLoaded(false);
    setModified(false);

    return true;
}

void Document::setGHints(GHints* newHints) {
    assert(newHints!=NULL);

    //gobjects in document keep states in parent document map -> preserve gobject hints
    QList<QVariantMap> objStates;
    for (int i=0;i<objects.size();i++) {
        objStates.append(objects[i]->getGHintsMap());
    }
    
    delete ctxState;
    ctxState = newHints;

    for (int i=0;i<objects.size();i++) {
        objects[i]->getGHints()->setMap(objStates[i]);
    }

}

void Document::addUnloadedObjects(const QList<UnloadedObjectInfo>& info) {
    foreach(const UnloadedObjectInfo& oi, info) {
        UnloadedObject* obj = new UnloadedObject(oi);
        _addObjectToHierarchy(obj, true);
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
}//namespace

