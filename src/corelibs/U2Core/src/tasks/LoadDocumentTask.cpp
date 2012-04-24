/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "LoadDocumentTask.h"

#include <U2Core/FormatSettings.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Log.h>
#include <U2Core/ResourceTracker.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/GObject.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/GHints.h>
#include <U2Core/AppResources.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/ZlibAdapter.h>

#include <U2Core/GObjectSelection.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/SequenceUtils.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/L10n.h>

#include <QtCore/QFileInfo>

#include <QtGui/QApplication>

#include <memory>

#define GObjectHint_NamesList  "gobject-hint-names-list"

namespace U2 {

/* TRANSLATOR U2::LoadUnloadedDocumentTask */    

//////////////////////////////////////////////////////////////////////////
// LoadUnloadedDocumentTask

//TODO: support subtask sharing!
//TODO: avoid multiple load tasks when opening view for unloaded doc!

LoadUnloadedDocumentTask::LoadUnloadedDocumentTask(Document* d, const LoadDocumentTaskConfig& _config)
: DocumentProviderTask ("", TaskFlags_NR_FOSCOE | TaskFlag_MinimizeSubtaskErrorText), loadTask(NULL), unloadedDoc(d), config(_config)
{
    assert(config.checkObjRef.objType != GObjectTypes::UNLOADED);
    assert(unloadedDoc != NULL);

    setVerboseLogMode(true);
    setTaskName(tr("Load '%1'").arg(d->getName()));
    setUseDescriptionFromSubtask(true);
    docOwner = false;
    resultDocument = d;
}

void LoadUnloadedDocumentTask::prepare() {
    if (unloadedDoc == NULL) {
        stateInfo.setError(tr("Document not found"));
        return;
    }
    DocumentFormatId format = unloadedDoc->getDocumentFormatId();
    QString formatName = AppContext::getDocumentFormatRegistry()->getFormatById(format)->getFormatName();
    IOAdapterFactory* iof = unloadedDoc->getIOAdapterFactory();
    const GUrl& url = unloadedDoc->getURL();
    coreLog.details(tr("Starting load document from %1, document format %2").arg(url.getURLString()).arg(formatName));
    QVariantMap hints = unloadedDoc->getGHintsMap();
    QStringList namesList;
    foreach(GObject* obj, unloadedDoc->getObjects()) {
        namesList << obj->getGObjectName();
    }
    hints[GObjectHint_NamesList] = namesList;
    loadTask = new LoadDocumentTask(format, url, iof, hints, config);
    addSubTask(loadTask);

    resName = getResourceName(unloadedDoc);
    AppContext::getResourceTracker()->registerResourceUser(resName, this);
}

void LoadUnloadedDocumentTask::clearResourceUse() {
    if (!resName.isEmpty()) {
        AppContext::getResourceTracker()->unregisterResourceUser(resName, this);
        resName.clear();
    }
}

Task::ReportResult LoadUnloadedDocumentTask::report() {
    Task::ReportResult res = ReportResult_Finished;
    Project* p = AppContext::getProject();

    if (unloadedDoc == NULL) {
        stateInfo.setError(tr("Document was removed"));
    } else {
        propagateSubtaskError();
    }

    if (hasError()) {
        if (!resName.isEmpty()) {
            clearResourceUse();
            resName.clear();
        }
    } else if (isCanceled() || (loadTask!=NULL && loadTask->isCanceled())) {
        //do nothing
    } else if (unloadedDoc->isLoaded()) {
        //do nothing
    } else if (p && p->isStateLocked()) {
        res = ReportResult_CallMeAgain; //wait until project is unlocked
    } else {
        assert(unloadedDoc->isStateLocked()); // all unloaded docs are always state locked
        
        //todo: move to utility method?
        const QList<StateLock*>& locks = unloadedDoc->getStateLocks();
        bool readyToLoad = true;
        foreach(StateLock* lock, locks) {
            if  (  lock != unloadedDoc->getDocumentModLock(DocumentModLock_IO)
                && lock != unloadedDoc->getDocumentModLock(DocumentModLock_USER) 
                && lock != unloadedDoc->getDocumentModLock(DocumentModLock_FORMAT_AS_CLASS)
                && lock != unloadedDoc->getDocumentModLock(DocumentModLock_FORMAT_AS_INSTANCE)
                && lock != unloadedDoc->getDocumentModLock(DocumentModLock_UNLOADED_STATE))
            {
                readyToLoad = false;
            }
        }
        if (!readyToLoad) {
            stateInfo.setError(tr("Document is locked")); //todo: wait instead?
        }  else {
            Document* sourceDoc = loadTask->getDocument();
            unloadedDoc->loadFrom(sourceDoc); // get all data from source doc;
            assert(!unloadedDoc->isTreeItemModified());
            assert(unloadedDoc->isLoaded());
        }
    }
    if (res == ReportResult_Finished) {
        clearResourceUse();
    }
    return res;
}


QString LoadUnloadedDocumentTask::getResourceName(Document* d) {
    return QString(LoadUnloadedDocumentTask::tr("Project Document:") + ":" + d->getURLString());
}

LoadUnloadedDocumentTask* LoadUnloadedDocumentTask::findActiveLoadingTask(Document* d) {
    QString res = getResourceName(d);
    QList<Task*> tasks = AppContext::getResourceTracker()->getResourceUsers(res);
    foreach(Task* t, tasks) {
        LoadUnloadedDocumentTask* lut = qobject_cast<LoadUnloadedDocumentTask*>(t);
        if (lut!=NULL) {
            return lut;
        }
    }
    return NULL;
}

bool LoadUnloadedDocumentTask::addLoadingSubtask(Task* t, const LoadDocumentTaskConfig& config) {
    GObject* o = GObjectUtils::selectObjectByReference(config.checkObjRef, UOF_LoadedAndUnloaded);
    if (o == NULL) {
        t->setError(tr("Annotation object not found"));
        return false;
    }
    if (o->isUnloaded()) {
        t->addSubTask(new LoadUnloadedDocumentTask(o->getDocument(), config));
        return true;
    }
    return false;
}

Document* LoadUnloadedDocumentTask::getDocument(bool ) {
    if (unloadedDoc.isNull()) {
        return NULL;
    }
    return DocumentProviderTask::getDocument();
}

//////////////////////////////////////////////////////////////////////////
// Load Document


LoadDocumentTask::LoadDocumentTask(DocumentFormatId f, const GUrl& u, 
                                   IOAdapterFactory* i, const QVariantMap& map, const LoadDocumentTaskConfig& _config)
: DocumentProviderTask("", TaskFlag_None), format(NULL), url(u), iof(i), hints(map), config(_config)
{
    setTaskName(tr("Read document: '%1'").arg(u.fileName()));
    documentDescription = u.getURLString();
    format = AppContext::getDocumentFormatRegistry()->getFormatById(f);
    init();
}

LoadDocumentTask::LoadDocumentTask(DocumentFormat* f, const GUrl& u, 
                                   IOAdapterFactory* i, const QVariantMap& map, const LoadDocumentTaskConfig& _config)
                                   : DocumentProviderTask("", TaskFlag_None), format(NULL), url(u), iof(i), hints(map), config(_config)
{
    setTaskName(tr("Read document: '%1'").arg(u.fileName()));
    documentDescription = u.getURLString();
    format = f;
    init();
}

static bool isLoadAsMergedDocument(QVariantMap& hints){
    if(hints.value(ProjectLoaderHint_MergeMode_Flag, false).toBool() == true){ // if that document was/is merged
        if(!QFile::exists(hints[ProjectLoaderHint_MergeMode_URLDocument].toString())){// if not exist - load as merge
            return true;
        }
        hints.remove(ProjectLoaderHint_MergeMode_Flag); // if exist - remove hints indicated that document is merged. Now document is genbank
        hints[DocumentReadingMode_SequenceMergeGapSize] = -1;
    }

    return false;
}

void LoadDocumentTask::init() {
    tpm = Progress_Manual;
    CHECK_EXT(format != NULL,  setError(tr("Document format is NULL!")), );
    CHECK_EXT(iof != NULL, setError(tr("IO adapter factory is NULL!")), );
    documentDescription = url.getURLString();
    if (format->getSupportedObjectTypes().contains(GObjectTypes::SEQUENCE)) {
        CaseAnnotationsMode mode = AppContext::getAppSettings()->getFormatAppsSettings()->getCaseAnnotationsMode();
        hints[GObjectHint_CaseAnns] = (int)mode;
    }
}

LoadDocumentTask * LoadDocumentTask::getDefaultLoadDocTask(const GUrl& url) {
    if( url.isEmpty() ) {
        return NULL;
    }
    IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( IOAdapterUtils::url2io( url ) );
    if ( iof == NULL ) {
        return NULL;
    }
    QList<FormatDetectionResult> dfs = DocumentUtils::detectFormat(url);
    if( dfs.isEmpty() ) {
        return NULL;
    }
    DocumentFormat * df = dfs.first().format;
    return new LoadDocumentTask( df->getFormatId(), url, iof );
}

static bool isLoadToMem(const DocumentFormatId& id){
	// files that use dbi not loaded to memory
	if(id == BaseDocumentFormats::FASTA || id ==  BaseDocumentFormats::PLAIN_GENBANK || 
		id == BaseDocumentFormats::RAW_DNA_SEQUENCE || id == BaseDocumentFormats::FASTQ 
		|| id == BaseDocumentFormats::GFF || id == BaseDocumentFormats::PDW){		
			return false;
	}	
	return true;	
}

void LoadDocumentTask::prepare() {
    if(hasError() || isCanceled()) {
        return;
    }
    
    int memUseMB = 0;
	
    if(!format->getFlags().testFlag(DocumentFormatFlag_NoFullMemoryLoad) && isLoadToMem(format->getFormatId())) { // document is fully loaded to memory
        QFileInfo file(url.getURLString());
        memUseMB = file.size() / (1024*1024);

        double DEFAULT_COMPRESS_RATIO = 2.5;
        if (iof->getAdapterId() == BaseIOAdapters::GZIPPED_LOCAL_FILE) {
            qint64 fileSizeInBytes = ZlibAdapter::getUncompressedFileSizeInBytes(url);
            if (fileSizeInBytes < 0) {
                memUseMB *= DEFAULT_COMPRESS_RATIO; //Need to calculate compress level
            } else {
                memUseMB = fileSizeInBytes / (1024*1024);
            }
        } else if (iof->getAdapterId() == BaseIOAdapters::GZIPPED_HTTP_FILE) {
            memUseMB *= DEFAULT_COMPRESS_RATIO; //Need to calculate compress level  
        }
        coreLog.trace(QString("load document:Memory resource %1").arg(memUseMB));
    }

    if (memUseMB > 0) {
        QString error;
        Project *p = AppContext::getProject();
        if (p) {
            if (!p->lockResources(memUseMB, url.getURLString(), error)) {
                stateInfo.setError(error);
            }
        } else {
            addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, memUseMB, false));
        }
    }
}

static Document* loadMergedDocument(IOAdapterFactory* iof, const QVariantMap& fs, U2OpStatus& os){
    QStringList urls = fs[ProjectLoaderHint_MergeMode_URLsDocumentConsistOf].toStringList();
    QList<Document*> docs;

    bool saveDoc = fs.value(ProjectLoaderHint_MergeMode_SaveDocumentFlag, false).toBool();

    os.setProgress(0);

    int curentDocIdx = 0;
    foreach(const QString& url, urls){
        FormatDetectionConfig conf;
        conf.useImporters = true;
        conf.bestMatchesOnly = false;
        GUrl gurl(url); 
        QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(gurl, conf);

        int len = 100 / urls.size();
        U2OpStatusChildImpl localOs(&os, U2OpStatusMapping(curentDocIdx * len,
            (curentDocIdx == urls.size() - 1) ?(100 -  curentDocIdx * len) : len));

        QVariantMap fsLocal;
        fsLocal.unite(fs);

        DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(formats[0].format->getFormatId());
        docs << df->loadDocument(iof ,gurl, fsLocal, localOs);
        CHECK_OP(os, NULL);
        curentDocIdx++;
    }

    Document* doc = U1SequenceUtils::mergeSequences(docs, fs , os);

    if(saveDoc){
        std::auto_ptr<IOAdapter> io(iof->createIOAdapter());
        QString url = doc->getURLString();
        if(!io->open(url ,IOAdapterMode_Write)){
            os.setError(L10N::errorOpeningFileWrite(url));
        }
        else{
            //TODO remove after genbank can storing without getWholeSequence
            try{
                doc->getDocumentFormat()->storeDocument(doc, io.get(), os);
            }
            catch(std::bad_alloc&){
                os.setError(QString("Not enough memory to storing %1 file").arg(doc->getURLString()));
            }
        }
    }

    return doc;    
}

void LoadDocumentTask::run() {
    CHECK_OP(stateInfo, );
    if (config.createDoc && iof->isResourceAvailable(url) == TriState_No) {
        CHECK_EXT(iof->isIOModeSupported(IOAdapterMode_Write), setError(tr("Document not found %1").arg(url.getURLString())), );
        resultDocument = format->createNewLoadedDocument(iof, url, stateInfo, hints);
        return;
    } 

    QStringList renameList = hints.value(GObjectHint_NamesList).toStringList();
    // removing this value from hints -> name list changes are not tracked in runtime
    // and used for LoadUnloadedDocument & LoadDocument privately
    hints.remove(GObjectHint_NamesList);

    try {
        if(isLoadAsMergedDocument(hints)){
            resultDocument = loadMergedDocument(iof, hints, stateInfo);
        }
        else{
                resultDocument = format->loadDocument(iof, url, hints, stateInfo);
        }             
    }
    catch(std::bad_alloc) {
        resultDocument = NULL;
        setError(tr("Not enough memory to load document %1").arg(url.getURLString()));
    }

    if (resultDocument != NULL) {
        if (!renameList.isEmpty()) {
            renameObjects(resultDocument, renameList);
        }
        Document* convertedDoc = createCopyRestructuredWithHints(resultDocument, stateInfo);
        if (convertedDoc != NULL) {
            delete resultDocument;
            resultDocument = convertedDoc;
        }
        if (hints.contains(DocumentReadingMode_MaxObjectsInDoc) ) {
            int maxObjects = hints.value(DocumentReadingMode_MaxObjectsInDoc).toInt();
            int docObjects = resultDocument->getObjects().size();
            if (docObjects > maxObjects) {
                setError(tr("Maximum number of objects per document limit reached for %1. Try different options for opening the document!").arg(resultDocument->getURLString()));
                delete resultDocument;
                resultDocument = NULL;
            }
        }
    }
    if (config.checkObjRef.isValid() && !hasError()) {
        processObjRef();
    }
    assert(stateInfo.isCoR() || resultDocument != NULL);
    assert(resultDocument == NULL || resultDocument->isLoaded());
}

Task::ReportResult LoadDocumentTask::report() {
    if (stateInfo.hasError() || isCanceled()) {
        return ReportResult_Finished;
    }
    resultDocument->setLastUpdateTime();
    return ReportResult_Finished;
}


void LoadDocumentTask::processObjRef() {
    assert(config.checkObjRef.isValid());
    assert(resultDocument!=NULL);

    if (GObjectUtils::selectObjectByReference(config.checkObjRef, resultDocument->getObjects(), UOF_LoadedOnly) == NULL) {
        if (config.objFactory == NULL) {
            stateInfo.setError(tr("Object not found: %1").arg(config.checkObjRef.objName));
        } else {
            assert(!resultDocument->isStateLocked());
            Document::Constraints c;
            c.objectTypeToAdd.append(config.checkObjRef.objType);
            bool ok = resultDocument->checkConstraints(c);
            if (!ok) {
                stateInfo.setError(tr("Can't add object. Document format constraints check failed: %1").arg(resultDocument->getName()));
            } else {
                GObject* obj = config.objFactory->create(config.checkObjRef);
                assert(obj!=NULL);
                resultDocument->addObject(obj);
            }
        }
    }
}


Document* LoadDocumentTask::createCopyRestructuredWithHints(const Document* doc, U2OpStatus& os) {
    Document *resultDoc = NULL;
    const QVariantMap& hints = doc->getGHintsMap();
    if (hints.value(DocumentReadingMode_SequenceAsAlignmentHint).toBool()) {
        QList<U2SequenceObject*> seqObjects;
        MAlignment ma = MSAUtils::seq2ma(doc->getObjects(), os);
        if (ma.isEmpty()) {
            return NULL;
        }
        ma.trim();

        MAlignmentObject* maObj = new MAlignmentObject(ma);
        QList<GObject*> objects;
        objects << maObj;

        DocumentFormatConstraints objTypeConstraints;
        objTypeConstraints.supportedObjectTypes << GObjectTypes::MULTIPLE_ALIGNMENT;
        bool makeReadOnly = !doc->getDocumentFormat()->checkConstraints(objTypeConstraints);

        resultDoc = new Document(doc->getDocumentFormat(), doc->getIOAdapterFactory(), doc->getURL(), U2DbiRef(), objects, hints, 
            makeReadOnly ? tr("Format does not support writing of alignments") : QString());

        doc->propagateModLocks(resultDoc);
    } else if (hints.contains(DocumentReadingMode_SequenceMergeGapSize)) {
        int mergeGap = hints.value(DocumentReadingMode_SequenceMergeGapSize).toInt();
        if (mergeGap < 0 || doc->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedOnly).count() <= 1) {
            return NULL;
        }
        resultDoc = U1SequenceUtils::mergeSequences(doc, mergeGap, os);
        if (os.hasError()) {
            delete resultDoc;
            resultDoc = NULL;
        }
    }
    return resultDoc;
}

void LoadDocumentTask::renameObjects(Document* doc, const QStringList& names) {
    if (doc->getObjects().size() != names.size()) {
        coreLog.trace(QString("Objects renaming failed! Objects in doc: %1, names: %2").arg(doc->getObjects().size()).arg(names.size()));
        return;
    }
    
    //drop names first
    QSet<QString> usedNames;
    QSet<GObject*> notRenamedObjects;
    foreach(GObject* obj, doc->getObjects()) {
        notRenamedObjects.insert(obj);
        usedNames.insert(obj->getGObjectName());
    }
    const QList<GObject*>& objects = doc->getObjects();
    int nObjects = objects.size();
    int maxIters = nObjects;
    int currentIter = 0; //to avoid endless loop in case of duplicate names
    while (!notRenamedObjects.isEmpty() && currentIter < maxIters) {
        for (int i = 0; i < nObjects;  i++) {
            GObject* obj = objects[i];
            if (!notRenamedObjects.contains(obj)) {
                continue;
            }
            QString newName = names[i];
            if (usedNames.contains(newName)) {
                continue;
            }
            QString oldName = obj->getGObjectName();
            obj->setGObjectName(newName);
            usedNames.remove(oldName);
            usedNames.insert(newName);
            notRenamedObjects.remove(obj);
        }
        currentIter++;
    }
}

QString LoadDocumentTask::getURLString() const {
    return url.getURLString();
}

GObject* LDTObjectFactory::create(const GObjectReference& ref) {
    assert(ref.objType == GObjectTypes::ANNOTATION_TABLE); //TODO: handle other core types
    return new AnnotationTableObject(ref.objName);
}

}//namespace
