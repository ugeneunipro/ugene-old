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

#include "SaveDocumentTask.h"

#include <U2Core/IOAdapter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/Log.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/L10n.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/GObjectUtils.h>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>

#include <memory>

namespace U2 {

SaveDocumentTask::SaveDocumentTask(Document* _doc, IOAdapterFactory* _io, const GUrl& _url)
: Task(tr("Save document"), TaskFlag_None), doc(_doc), iof(_io), url(_url), flags(SaveDoc_Overwrite)
{
    assert(doc!=NULL);
    if (iof == NULL) {
        iof = doc->getIOAdapterFactory();
    }
    if (url.isEmpty()) {
        url = doc->getURLString();
    }
    lock = NULL;
}

SaveDocumentTask::SaveDocumentTask(Document* _doc, SaveDocFlags f, const QSet<QString>& _excludeFileNames)
: Task(tr("Save document"), TaskFlag_None), 
doc(_doc), iof(doc->getIOAdapterFactory()), url(doc->getURL()), flags(f), excludeFileNames(_excludeFileNames)
{
    assert(doc!=NULL);
}

void SaveDocumentTask::addFlag(SaveDocFlag f) {
    flags|=f;
}

void SaveDocumentTask::prepare() {
    if (doc.isNull()) {
        setError("Document was removed");
        return;
    }
    lock = new StateLock(getTaskName());
    doc->lockState(lock);
}

void SaveDocumentTask::run() {
    if (flags.testFlag(SaveDoc_Roll) && !GUrlUtils::renameFileWithNameRoll(url.getURLString(), stateInfo, excludeFileNames, &coreLog)) {
        return;
    }

    coreLog.info(tr("Saving document %1\n").arg(url.getURLString()));
    DocumentFormat* df = doc->getDocumentFormat();

    std::auto_ptr<IOAdapter> io(IOAdapterUtils::open(url, stateInfo, flags.testFlag(SaveDoc_Append)? IOAdapterMode_Append: IOAdapterMode_Write));
    CHECK_OP(stateInfo, );
    df->storeDocument(doc, io.get(), stateInfo);
}

Task::ReportResult SaveDocumentTask::report() {
    if (lock!=NULL) {
        assert(!doc.isNull());
        doc->unlockState(lock);
        delete lock;
        lock = NULL;
    }
    CHECK_OP(stateInfo, ReportResult_Finished);
    
    if (url == doc->getURL() && iof == doc->getIOAdapterFactory()) {
        doc->makeClean();
    }
    if (doc) {
        doc->setLastUpdateTime();
    }
    if (flags.testFlag(SaveDoc_DestroyAfter)) {
        doc->unload();
        delete doc;
    }
    return Task::ReportResult_Finished;
}


//////////////////////////////////////////////////////////////////////////
/// save multiple

SaveMiltipleDocuments::SaveMiltipleDocuments(const QList<Document*>& docs, bool askBeforeSave)
: Task(tr("Save multiple documents"), TaskFlag_NoRun)
{
    bool saveAll = false;
    foreach(Document* doc, docs) {
        bool save=true;
        if (askBeforeSave) {
            QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes) | QMessageBox::No;
            if (docs.size() > 1) {
                buttons = buttons | QMessageBox::YesToAll | QMessageBox::NoToAll;
            }

            QMessageBox::StandardButton res = saveAll ? QMessageBox::YesToAll : QMessageBox::question(QApplication::activeWindow(),
                tr("Question?"), tr("Save document: %1").arg(doc->getURLString()),
                buttons, QMessageBox::Yes);

            if (res == QMessageBox::NoToAll) {
                break;
            }
            if (res == QMessageBox::YesToAll) {
                saveAll = true;
            }
            if (res == QMessageBox::No) {
                save = false;
            }
        }
        if (save) {
            addSubTask(new SaveDocumentTask(doc));
        }
    }
}


QList<Document*> SaveMiltipleDocuments::findModifiedDocuments(const QList<Document*>& docs) {
    QList<Document*> res;
    foreach(Document* doc, docs) {
        if (doc->isTreeItemModified()) {
            res.append(doc);
        }
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////
// save a copy and add to project
SaveCopyAndAddToProjectTask::SaveCopyAndAddToProjectTask(Document* doc, IOAdapterFactory* iof, const GUrl& _url)
: Task (tr("Save a copy %1").arg(url.getURLString()), TaskFlags_NR_FOSCOE), url(_url)
{
    origURL = doc->getURL();
    df = doc->getDocumentFormat();
    hints = doc->getGHintsMap();
    
    saveTask = new SaveDocumentTask(doc, iof, url);
    saveTask->setExcludeFileNames(DocumentUtils::getNewDocFileNameExcludesHint());
    addSubTask(saveTask);

    foreach(GObject* obj, doc->getObjects()) {
        info.append(UnloadedObjectInfo(obj));
    }
}

Task::ReportResult SaveCopyAndAddToProjectTask::report() {
    CHECK_OP(stateInfo, ReportResult_Finished);
    Project* p = AppContext::getProject();
    CHECK_EXT(p != NULL, setError(tr("No active project found")), ReportResult_Finished);
    CHECK_EXT(!p->isStateLocked(), setError(tr("Project is locked")), ReportResult_Finished);
        
    const GUrl& url = saveTask->getURL();
    if (p->findDocumentByURL(url)) {
        setError(tr("Document is already added to the project %1").arg(url.getURLString()));
        return ReportResult_Finished;
    }
    Document* doc = df->createNewUnloadedDocument(saveTask->getIOAdapterFactory(), url, stateInfo, hints, info);
    CHECK_OP(stateInfo, ReportResult_Finished);
    foreach(GObject* o, doc->getObjects()) {
        GObjectUtils::updateRelationsURL(o, origURL, url);
    }
    doc->setModified(false);
    p->addDocument(doc);
    return ReportResult_Finished;
}

///////////////////////////////////////////////////////////////////////////
// relocate task

RelocateDocumentTask::RelocateDocumentTask(const GUrl& fu, const GUrl& tu)
: Task (tr("Relocate document %1 -> %2").arg(fu.getURLString()).arg(tu.getURLString()), TaskFlag_NoRun), fromURL(fu), toURL(tu)
{
}

Task::ReportResult RelocateDocumentTask::report() {
    Project* p = AppContext::getProject();
    if (p == NULL) {
        setError(tr("No active project found"));
        return ReportResult_Finished;
    }
    if (p->isStateLocked()) {
        setError(tr("Project is locked"));
        return ReportResult_Finished;
    }
    Document* d = p->findDocumentByURL(fromURL);
    if (d == NULL) {
        setError(L10N::errorDocumentNotFound(fromURL));
        return ReportResult_Finished;
    }
    if (d->isLoaded()) {
        setError(tr("Only unloaded objects can be relocated"));
        return ReportResult_Finished;
    }

    d->setURL(toURL);
    if (fromURL.baseFileName() == d->getName() || fromURL.fileName() == d->getName()) { // if document name is default -> update it too
        d->setName(toURL.baseFileName());
    }

    //update relations to new url
    foreach(Document* d, p->getDocuments()) {
        foreach(GObject* o, d->getObjects()) {
            GObjectUtils::updateRelationsURL(o, fromURL, toURL);
        }
    }

    return ReportResult_Finished;
}

}//namespace
