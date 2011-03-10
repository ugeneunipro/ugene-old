#include "FileDbi.h"

#include <U2Core/GObjectUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/MultiTask.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/TextUtils.h>
//#include <U2Core/AnnotationUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/U2OpStatus.h>

#include <QtCore/QHash>
#include <QtCore/QtAlgorithms>

namespace U2 {

static GObjectType U2toG2(U2DataType ut) {
    switch (ut) {
        case U2Type::Sequence: return GObjectTypes::SEQUENCE;
        case U2Type::Msa:   return GObjectTypes::MULTIPLE_ALIGNMENT;
        case U2Type::Assembly:   return GObjectTypes::MULTIPLE_ALIGNMENT;
        case U2Type::Annotation:  return GObjectTypes::ANNOTATION_TABLE;
        default: return GObjectTypes::UNKNOWN;
    }
}

static U2DataType G2toU2(const GObjectType& t) {
    if (t == GObjectTypes::SEQUENCE) {
        return U2Type::Sequence;
    } 
    if (t == GObjectTypes::MULTIPLE_ALIGNMENT) {
        return U2Type::Msa;
    }
    if (t == GObjectTypes::ANNOTATION_TABLE) {
        return U2Type::Annotation;
    }
    return U2Type::Unknown;
}

// FileDbiFactory


U2Dbi *FileDbiFactory::createDbi() {
    return new FileDbi();
}

U2DbiFactoryId FileDbiFactory::getId()const {
    return ID;
}

const U2DbiFactoryId FileDbiFactory::ID = "FileDbi";

U2DataId FileDbi::obj2id(GObject* go) const {
    U2DataId res = qHash(go->getGObjectName());
    return res += ((U2DataId)G2toU2(go->getGObjectType()))<<(64-16);
}

U2DataId FileDbi::an2id(Annotation* an) const {
    U2DataId res = (U2DataId)an;
    res &= Q_UINT64_C(0x0000FFFFFFFFFFFF);
    return res += ((U2DataId)U2Type::Annotation)<<(64-16);
}

U2DataType FileDbi::getEntityTypeById(U2DataId id) const {
    id &= Q_UINT64_C(0xFFFF000000000000);
    return id >> (64-16);
}

QString FileDbi::getDbiId() const
{
    return url.getURLString();
}

QHash<QString, QString> FileDbi::getDbiMetaInfo(U2OpStatus& os){
    QHash<QString, QString> res;
    res["url"] = getDbiId();
    return res;
}

FileDbi::FileDbi() : doc(NULL),  sync(new QReadWriteLock(QReadWriteLock::Recursive)) {
    folderDbi = new ObjFolderDbi(this);
    seqDbi = new SequenceObjectDbi(this);
    msaDbi = new MsaObjectDbi(this);
    assemblyDbi = new AssemblyObjectDbi(this);
    annDbi = new AnnotationObjectDbi(this);
}

FileDbi::~FileDbi() {
    delete folderDbi;
    delete seqDbi;
    delete msaDbi;
    delete assemblyDbi;
    delete annDbi;
}



QVariantMap FileDbi::shutdown(U2OpStatus& os)
{
    QWriteLocker lock(sync);
    if (state != U2DbiState_Ready) {
        os.setError(FileDbiL10N::tr("Invalid dbi state: %1").arg(state));
        return QVariantMap();
    }
    Task* t = new SaveDocumentTask(doc);
    // TODO
    //connect(new TaskSignalMapper(t),SIGNAL(si_taskFinished(Task*)),SLOT(onDocClosed(Task*)));
    //AppContext::getTaskScheduler()->registerTopLevelTask(t);
    state = U2DbiState_Stopping;

    //TODO persist folders

    assert(state == U2DbiState_Stopping);

    bool ok = doc->unload();
    if (!ok) {
        //stateInfo.setError(errPrefix + tr("unexpected error"));
        //coreLog.error(stateInfo.getError());
    }
    delete doc;
    url = QString("");
    msaSeqs.clear();
    annIds.clear();
    topObjects.clear();
    folders.clear();

    state = U2DbiState_Void;
    return QVariantMap();
}

bool FileDbi::isReady( U2OpStatus& os ) const
{
    switch(state) {
        case U2DbiState_Ready: return true;
        default:
        os.setError(FileDbiL10N::tr("Document is not loaded"));
        return false;
    }
}

bool FileDbi::canModify(U2OpStatus& os) const {
    if (!isReady(os)) {
        return false;
    }
    if (doc->isStateLocked()) {
        os.setError(FileDbiL10N::tr("Document is locked"));
        return false;
    }
    return true;
}

bool FileDbi::canCreate(GObjectType t, U2OpStatus& os) const {
    if (!canModify(os)) {
        return false;
    }
    Document::Constraints check;
    check.objectTypeToAdd = t;
    if (!doc->checkConstraints(check)) {
        os.setError(FileDbiL10N::tr("Cannot add object due to document format constraints"));
        return false;
    }
    return true;
}

void FileDbi::init(const QHash<QString, QString>& props, const QVariantMap& persistentData, U2OpStatus& os)
{
    initProps = props;
    QWriteLocker lock(sync);
    if (state != U2DbiState_Void) {
        os.setError(FileDbiL10N::tr("Invalid dbi state: %1").arg(state));
        return;
    }
    url = props.value(U2_DBI_OPTION_URL);
    if (url.isEmpty()) {
        os.setError(FileDbiL10N::tr("URL is not specified"));
        return;
    }
    saved = persistentData;

    Task* t = LoadDocumentTask::getDefaultLoadDocTask(url);
    if (t == NULL) {
        os.setError(FileDbiL10N::tr("Unknown document format"));
        return;
    }
    // TODO: connect(new TaskSignalMapper(t),SIGNAL(si_taskFinished(Task*)),SLOT(onDocLoaded(Task*)));
    //AppContext::getTaskScheduler()->registerTopLevelTask(t);
    state = U2DbiState_Starting;

    LoadDocumentTask* loader = qobject_cast<LoadDocumentTask*>(t);
    doc = loader->takeDocument();

    //TODO parse saved
    //populate topObjects
    folders << "/";
    foreach (GObject* go, doc->getObjects()) {
        if (go->getGObjectType() != GObjectTypes::ANNOTATION_TABLE) {
            topObjects.insert(go, "/");
        }
    }

    //populate msaSeq
    QList<GObject*> msaList = GObjectUtils::select(doc->getObjects(), GObjectTypes::MULTIPLE_ALIGNMENT, UOF_LoadedOnly);
    foreach(GObject* go, msaList) {
        MAlignmentObject* mao = qobject_cast<MAlignmentObject*>(go);
        const MAlignment& ma = mao->getMAlignment();
        for(int i = 0; i < ma.getNumRows(); i++) {
            DNASequence seq(ma.getRowNames()[i], ma.getRow(i).getCore(), ma.getAlphabet());
            msaSeqs.insert(mao,new DNASequenceObject(seq.getName(), seq));
        }
    }

    //populate annIds
    QList<GObject*> annList = GObjectUtils::select(doc->getObjects(), GObjectTypes::ANNOTATION_TABLE, UOF_LoadedOnly);
    foreach(GObject* go, annList) {
        AnnotationTableObject* ato = qobject_cast<AnnotationTableObject*>(go);
        foreach(Annotation* a, ato->getAnnotations()) {
            annIds[an2id(a)] = a;
        }
    }

    state = U2DbiState_Ready;
}

GObject* FileDbi::findTop(U2DataId dataId, U2OpStatus& os ) const
{
    if (!isReady(os)) {
        return NULL;
    }
    foreach(GObject* go, topObjects.keys()) {
        if (dataId == obj2id(go)) {
            return go;
        }
    }

    os.setError(FileDbiL10N::tr("Object not found"));
    return NULL;
}

MAlignmentObject* FileDbi::findMA( U2DataId id, U2OpStatus& os ) const
{
    return qobject_cast<MAlignmentObject*>(findTop(id, os));
}

DNASequenceObject* FileDbi::findSeq( U2DataId id, U2OpStatus& os ) const
{
    if (!isReady(os)) {
        return NULL;
    }
    DNASequenceObject* go = NULL;
    foreach(GObject* obj, topObjects.keys()) {
        if (id == obj2id(obj)) {
            go = qobject_cast<DNASequenceObject*>(obj);
            break;
        }
    }
    if (go == NULL) {
        QMapIterator<MAlignmentObject*,DNASequenceObject*> i(msaSeqs);
        while (i.hasNext()) {
            i.next();
            if (id == obj2id(i.value())) {
                go = i.value();
                break;
            }
        }
    }
    if (go == NULL) {
        os.setError(FileDbiL10N::tr("Object not found"));
    }
    return go;
}

Annotation* FileDbi::findAntn( U2DataId id, U2OpStatus& os ) const
{
    Annotation* ann = NULL;
    if (!isReady(os)) {
        return ann;
    }
    ann = annIds.value(id);
    if (ann == NULL) {
        os.setError(FileDbiL10N::tr("Object not found"));
    }
    return ann;
}

QString FileDbi::newName( GObjectType t ) const
{
    QString name = t;
    while (doc->findGObjectByName(name)) {
        name = TextUtils::variate(name, "_", QSet<QString>(), true);
    }
    return name;
}

DNAAlphabet* FileDbi::checkAlphabet( const QString& id, U2OpStatus& os ) const
{
    DNAAlphabet* al = AppContext::getDNAAlphabetRegistry()->findById(id);
    if (al == NULL) {
        os.setError(FileDbiL10N::tr("Unknown alphabet: %1").arg(id));
    }
    return al;
}

bool FileDbi::checkFolder( const QString& path, U2OpStatus& os ) const
{
    if (!folders.contains(path)) {
        os.setError(FileDbiL10N::tr("Folder does not exist"));
        return false;
    }
    return true;
}

ObjFolderDbi::ObjFolderDbi(FileDbi* rootDbi) : U2FolderDbi(rootDbi), root(rootDbi) { 
}

qint64 ObjFolderDbi::countObjects( U2OpStatus& os ) {
    QReadLocker lock(root->sync);
    if (root->isReady(os)) {
        return root->topObjects.size();
    }
    return 0;
}

qint64 ObjFolderDbi::countObjects( U2DataType type, U2OpStatus& os ) {
    QReadLocker lock(root->sync);
    if (!root->isReady(os)) {
        return 0;
    }
    return GObjectUtils::select(root->topObjects.keys(), U2toG2(type), UOF_LoadedOnly).size();
}

QList<U2DataId> ObjFolderDbi::getObjects( U2DataType type, qint64 offset, qint64 count, U2OpStatus& os ) 
{
    QList<U2DataId> res;
    QReadLocker lock(root->sync);
    if (!root->isReady(os)) {
        return res;
    }
    if (offset < 0) {
        os.setError(L10N::badArgument("offset"));
        return res;
    }

    if (count < -1) {
        os.setError(L10N::badArgument("count"));
        return res;
    }

    QList<GObject*> gl = GObjectUtils::select(root->topObjects.keys(), U2toG2(type), UOF_LoadedOnly);
    foreach(GObject* go, gl.mid(offset, count)) {
        res.push_back(root->obj2id(go));
    }
    return res;
}

QList<U2DataId> ObjFolderDbi::getParents(U2DataId entityId, U2OpStatus& os )
{
    QReadLocker lock(root->sync);
    QList<U2DataId> res;
    if (!root->isReady(os)) {
        return res;
    }
    U2DataType type = getRootDbi()->getEntityTypeById(entityId);
    if (type == U2Type::Annotation) {
        Annotation* obj = root->findAntn(entityId, os);
        if (obj == NULL) {
            return res;
        }
        QList<GObject*> seq = GObjectUtils::findObjectsRelatedToObjectByRole(obj->getGObject(), GObjectTypes::SEQUENCE, 
            GObjectRelationRole::SEQUENCE, root->doc->getObjects(), UOF_LoadedOnly);
        assert(seq.size() == 1);
        res.push_back(root->obj2id(seq.first()));
    } 
    else if (type == U2Type::Sequence) {
        QMapIterator<MAlignmentObject*,DNASequenceObject*> i(root->msaSeqs);
        while (i.hasNext()) {
            i.next();
            if (entityId == root->obj2id(i.value())) {
                res.push_back(root->obj2id(i.key()));
            }
        }
    }
    return res;    
}
SequenceObjectDbi::SequenceObjectDbi( FileDbi* rootDbi ) : U2SequenceRWDbi(rootDbi), root(rootDbi)
{
}

U2Sequence SequenceObjectDbi::getSequenceObject(U2DataId sequenceId, U2OpStatus& os ) {
    U2Sequence seq;
    QReadLocker lock(root->sync);
    DNASequenceObject* gseq = root->findSeq(sequenceId, os);
    if (gseq == NULL) {
        return seq;
    }
    seq.id = sequenceId;
    seq.dbiId = root->getDbiId();
    seq.alphabet.id = gseq->getAlphabet()->getId();
    seq.length = gseq->getSequenceLen();
    return seq;
}

QByteArray SequenceObjectDbi::getSequenceData(U2DataId sequenceId, const U2Region& region, U2OpStatus& os ) {
    QReadLocker lock(root->sync);
    DNASequenceObject* gseq = root->findSeq(sequenceId, os);
    if (gseq == NULL) {
        return QByteArray();
    }
    if (region.startPos < 0 || region.endPos() < region.startPos || region.endPos() > gseq->getSequenceLen()) {
        os.setError(FileDbiL10N::tr("Invalid region"));
        return QByteArray();
    }
    return gseq->getSequence().mid(region.startPos, region.length);
}

void SequenceObjectDbi::createSequenceObject( U2Sequence& sequence, const QString& folder, U2OpStatus& os )
{
    QWriteLocker lock(root->sync);
    if (!root->canCreate(GObjectTypes::SEQUENCE, os)) {
        return;
    }
    if (!root->checkFolder(folder, os)) {
        return;
    }

    DNASequence dnaSeq;
    dnaSeq.alphabet = root->checkAlphabet(sequence.alphabet.id, os);
    if (dnaSeq.alphabet == NULL) {
        return;
    }

    QString name = root->newName(GObjectTypes::SEQUENCE);
    DNASequenceObject* gseq = new DNASequenceObject(name, dnaSeq);
    root->doc->addObject(gseq);
    sequence.dbiId = root->getDbiId();
    sequence.id = root->obj2id(gseq);
    lock.unlock(); // this should not open possibility of race with GC, 
    //as any writes should happen in main thread only
    if (!folder.isEmpty()) {
        root->folderDbi->addObjectsToFolder(QList<U2DataId>() << sequence.id, folder, os);
    }
}

void SequenceObjectDbi::updateSequenceData(U2DataId sequenceId, const U2Region& region, 
                                           const QByteArray& dataToInsert, U2OpStatus& os )
{
    QWriteLocker lock(root->sync);
    if (!root->canModify(os)) {
        return;
    }
    DNASequenceObject* gseq = root->findSeq(sequenceId, os);
    if (gseq == NULL) {
        return;
    }
    if (region.startPos < 0 || region.endPos() < region.startPos || region.endPos() > gseq->getSequenceLen()) {
        os.setError(FileDbiL10N::tr("Invalid region"));
        return;
    }

    /*
    QList<GObject*> annotationTablesList = root->doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
    U2Region lreg(region.startPos, region.length);
    foreach(GObject *table, annotationTablesList){
        AnnotationTableObject *ato = qobject_cast<AnnotationTableObject*>(table);
        if (ato->hasObjectRelation(gseq, GObjectRelationRole::SEQUENCE)){
            QList<Annotation*> annList = ato->getAnnotations();
            foreach(Annotation *an, annList) {
                QVector<U2Region> locs = an->getLocation();
                AnnotationUtils::fixLocationsForReplacedRegion(lreg, dataToInsert.length(), locs, 
                    AnnotationUtils::AnnotationStrategyForResize_Resize);
                if(!locs.isEmpty()){
                    an->replaceLocationRegions(locs);
                }else{
                    ato->removeAnnotation(an);
                    root->annIds.remove(root->an2id(an));
                }
            }
        }
    }*/
    DNASequence seq = gseq->getDNASequence();
    seq.seq.replace(region.startPos, dataToInsert.length(), dataToInsert);
    gseq->setSequence(seq);

    lock.unlock();
}

QStringList ObjFolderDbi::getFolders( U2OpStatus& os ) {
    QReadLocker lock(root->sync);
    if (!root->isReady(os)) {
        return QStringList();
    }
    return root->folders;
}

qint64 ObjFolderDbi::countObjects( const QString& folder, U2OpStatus& os ) 
{
    QReadLocker lock(root->sync);
    if (!root->isReady(os) || !root->checkFolder(folder, os)) {
        return 0;
    }
    return root->topObjects.keys(folder).size();
}

QList<U2DataId> ObjFolderDbi::getObjects( const QString& folder, qint64 offset, qint64 count, U2OpStatus& os ) {
    QList<U2DataId> res;
    QReadLocker lock(root->sync);
    if (!root->isReady(os) || !root->checkFolder(folder, os)) {
        return res;
    }
    foreach(GObject* go, root->topObjects.keys(folder).mid(offset, count)) {
        res << root->obj2id(go);
    }
    return res;
}

QStringList ObjFolderDbi::getObjectFolders( U2DataId objectId, U2OpStatus& os )
{
    QReadLocker lock(root->sync);
    if (!root->isReady(os)) {
        return QStringList();
    }
    foreach(GObject* go, root->topObjects.keys()) {
        if (objectId == root->obj2id(go)) {
            return root->topObjects.values(go);
        }
    }
    return QStringList();
}

void ObjFolderDbi::createFolder( const QString& path, U2OpStatus& os )
{
    QWriteLocker lock(root->sync);
    if (!root->isReady(os)) {
        return;
    }
    //TODO validate path
    if (root->folders.contains(path)) {
        os.setError(FileDbiL10N::tr("Folder already exists"));
        return;
    }
    root->folders.push_back(path);
    lock.unlock();
}

QList<U2DataId> ObjFolderDbi::getObjects(qint64 offset, qint64 count, U2OpStatus& os ) {
    QList<U2DataId> res;
    QReadLocker lock(root->sync);
    if (!root->isReady(os)) {
        return res;
    }
    foreach(GObject* go, root->topObjects.keys().mid(offset, count)) {
        res.push_back(root->obj2id(go));
    }
    return res;
}

void ObjFolderDbi::removeObject(U2DataId id, const QString& folder, U2OpStatus& os )
{
    if (!root->canModify(os)) {
        return;
    }
    if (folder.isEmpty()) {
        //TODO GC
    } else {
        moveObjects(QList<U2DataId>() << id, folder, "", os);
    }
}

void ObjFolderDbi::removeObjects( const QList<U2DataId>& dataIds, const QString& folder, U2OpStatus& os )
{
    if (!root->canModify(os)) {
        return;
    }
    if (folder.isEmpty()) {
        //TODO GC
    } else {
        moveObjects(dataIds, folder, "", os);
    }
}

void ObjFolderDbi::moveObjects( const QList<U2DataId>& objects, const QString& fromFolder, const QString& toFolder, U2OpStatus& os)
{
    QWriteLocker lock(root->sync);
    if (!root->isReady(os) || objects.isEmpty() || !root->checkFolder(fromFolder, os)) {
        return;
    }
    if (!toFolder.isEmpty() && !root->checkFolder(toFolder, os)) {
        return;
    }

    QSet<GObject*> gos;
    foreach(U2DataId entity, objects) {
        GObject* go = root->findTop(entity, os);
        if (go) {
            gos << go;
        } else {
            return;
        }
    }
    QMultiMap<GObject*, QString>& folders = root->topObjects;
    foreach(GObject* go, gos) {
        if (!folders.values(go).contains(fromFolder)) {
            os.setError(FileDbiL10N::tr("Object does not belong to folder."));
            return;
        }
    }
    foreach(GObject* go, gos) {
        root->topObjects.remove(go, fromFolder);
        if (toFolder.isEmpty()) {
            if (folders.values(go).isEmpty()) {
                // TODO GC
            }            
        } else {
            root->topObjects.insert(go, toFolder);
        }
    }

    lock.unlock();
}

void ObjFolderDbi::addObjectsToFolder( const QList<U2DataId>& objects, const QString& toFolder, U2OpStatus& os)
{
    QWriteLocker lock(root->sync);
    if (!root->isReady(os) || objects.isEmpty() || !root->checkFolder(toFolder, os)) {
        return;
    }
    //TODO check if all ids are top-level types

    QSet<GObject*> gos;
    foreach(U2DataId entity, objects) {
        GObject* go;
        U2DataType type = root->getEntityTypeById(entity);
        if (type == U2Type::Sequence) {
            go = root->findSeq(entity, os);
        } else {
            go = root->findTop(entity, os);
        }
        if (go) {
            gos << go;
        } else {
            return;
        }
    }
    QMultiMap<GObject*, QString>& tops = root->topObjects;
    foreach(GObject* go, gos) {
        if (tops.values(go).isEmpty()) {
            //TODO add to the document??
        }
        tops.insert(go, toFolder);
    }
    lock.unlock();
}

void ObjFolderDbi::removeFolder( const QString& folder, U2OpStatus& os )
{
    QWriteLocker lock(root->sync);
    if (!root->canModify(os) || !root->checkFolder(folder, os)) {
        return;
    }
    root->folders.removeOne(folder);

    QMultiMap<GObject*, QString>& tops = root->topObjects;
    QList<GObject*> lst = tops.keys(folder);
    foreach(GObject* go, lst) {
        tops.remove(go, folder);
    }
    lock.unlock();
    if (!lst.isEmpty()) {
        //TODO GC
    }
}


}//ns

