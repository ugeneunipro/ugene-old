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

#include "LoadDocumentTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Log.h>
#include <U2Core/ResourceTracker.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/GObject.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/GHints.h>
#include <U2Core/AppResources.h>
#include <U2Core/DocumentUtils.h>

#include <U2Core/GObjectSelection.h>

#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectRelationRoles.h>

#include <U2Core/AnnotationTableObject.h>
#include <QtCore/QFileInfo>
#include <QtGui/QApplication>

namespace U2 {

/* TRANSLATOR U2::LoadUnloadedDocumentTask */    


//////////////////////////////////////////////////////////////////////////
// LoadUnloadedDocumentTask

//TODO: support subtask sharing!
//TODO: avoid multiple load tasks when opening view for unloaded doc!

LoadUnloadedDocumentTask::LoadUnloadedDocumentTask(Document* d, const LoadDocumentTaskConfig& _config)
: Task("", TaskFlags_NR_FOSCOE | TaskFlag_MinimizeSubtaskErrorText), subtask(NULL), unloadedDoc(d), config(_config)
{
    assert(config.checkObjRef.objType != GObjectTypes::UNLOADED);
    setVerboseLogMode(true);
    setTaskName(tr("Load '%1'").arg(d->getName()));
    setUseDescriptionFromSubtask(true);
    assert(d!=NULL);
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
    subtask = new LoadDocumentTask(format, url, iof, unloadedDoc->getGHintsMap(), config);
    addSubTask(subtask);

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

    if (hasErrors()) {
        coreLog.error(tr("Error: %1").arg(stateInfo.getError()));
        if (!resName.isEmpty()) {
            clearResourceUse();
            resName.clear();
        }
    } else if (isCanceled() || (subtask!=NULL && subtask->isCanceled())) {
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
            Document* doc = subtask->getDocument();
            unloadedDoc->loadFrom(doc); // doc was load in a separate thread -> clone all GObjects
            assert(!unloadedDoc->isTreeItemModified());
            assert(unloadedDoc->isLoaded());
        }
    }
    if (res == ReportResult_Finished) {
        clearResourceUse();
    }
    return res;
}

Document* LoadUnloadedDocumentTask::getDocument() const {
    return unloadedDoc;
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
    return false;
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

//////////////////////////////////////////////////////////////////////////
// Load Document


LoadDocumentTask::LoadDocumentTask(DocumentFormatId f, const GUrl& u, 
                                   IOAdapterFactory* i, const QVariantMap& map, const LoadDocumentTaskConfig& _config)
: Task("", TaskFlag_None), format(f), url(u), iof(i), hints(map), result(NULL), config(_config)
{
    setTaskName(tr("Read document: '%1'").arg(u.fileName()));
 
    tpm = Progress_Manual;
    assert(iof!=NULL);
}

LoadDocumentTask * LoadDocumentTask::getDefaultLoadDocTask(const GUrl& url) {
    if( url.isEmpty() ) {
        return NULL;
    }
    IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( BaseIOAdapters::url2io( url ) );
    if( NULL == iof ) {
        return NULL;
    }
    QList< DocumentFormat* > dfs = DocumentUtils::detectFormat(url);
    if( dfs.isEmpty() ) {
        return NULL;
    }
    DocumentFormat * df = dfs.first();
    
    return new LoadDocumentTask( df->getFormatId(), url, iof );
}

LoadDocumentTask::~LoadDocumentTask() {
    cleanup();
}

void LoadDocumentTask::cleanup() {
    delete result;
    result = NULL;
}

void LoadDocumentTask::prepare() {
    QFileInfo file(url.getURLString());
    qint64 memUseMB = file.size()/(1024*1024);
    if(iof->getAdapterId() == BaseIOAdapters::GZIPPED_LOCAL_FILE || iof->getAdapterId() == BaseIOAdapters::GZIPPED_HTTP_FILE) {
        memUseMB *= 2.5; //Need to calculate compress level
    }
    if(memUseMB < 30) {
        memUseMB = 30;
    }
    coreLog.trace(QString("load document:Memory resource %1").arg(memUseMB));
    QString error;
    Project *p = AppContext::getProject();
    if(p) {
        if(!AppContext::getProject()->lockResources(memUseMB, url.getURLString(), error)) {
            stateInfo.setError(error);
        }
    } else {
        TaskResourceUsage memUsg(RESOURCE_MEMORY, memUseMB, false);
        taskResources.append(memUsg);
    }
}

void LoadDocumentTask::run() {
    DocumentFormat* f = AppContext::getDocumentFormatRegistry()->getFormatById(format);
    if (f == NULL) {
        setError(tr("Invalid document format %1").arg(format));
        return;
    }
    if (config.createDoc && iof->isResourceAvailable(url) == TriState_No) {
        if (iof->isIOModeSupported(IOAdapterMode_Write)) {
            result = f->createNewDocument(iof, url, hints);
        } else {
            setError(tr("Document not found %1").arg(url.getURLString()));
        }
    } else {
        result = f->loadDocument(iof, url, stateInfo, hints);
    }
    if (config.checkObjRef.isValid() && !hasErrors()) {
        processObjRef();
    }
    assert(isCanceled() || result!=NULL || hasErrors());
    assert(result == NULL || result->isLoaded());
}


Task::ReportResult LoadDocumentTask::report() {
    if (stateInfo.hasErrors() || isCanceled()) {
        return ReportResult_Finished;
    }
    assert(result!=NULL);
    return ReportResult_Finished;
}


void LoadDocumentTask::processObjRef() {
    assert(config.checkObjRef.isValid());
    assert(result!=NULL);

    if (GObjectUtils::selectObjectByReference(config.checkObjRef, result->getObjects(), UOF_LoadedOnly) == NULL) {
        if (config.objFactory == NULL) {
            stateInfo.setError(tr("Object not found: %1").arg(config.checkObjRef.objName));
        } else {
            assert(!result->isStateLocked());
            Document::Constraints c;
            c.objectTypeToAdd.append(config.checkObjRef.objType);
            bool ok = result->checkConstraints(c);
            if (!ok) {
                stateInfo.setError(tr("Can't add object. Document format constraints check failed: %1").arg(result->getName()));
            } else {
                GObject* obj = config.objFactory->create(config.checkObjRef);
                assert(obj!=NULL);
                result->addObject(obj);
            }
        }
    }
}

GObject* LDTObjectFactory::create(const GObjectReference& ref) {
    assert(ref.objType == GObjectTypes::ANNOTATION_TABLE); //TODO: handle other core types
    return new AnnotationTableObject(ref.objName);
}

}//namespace
