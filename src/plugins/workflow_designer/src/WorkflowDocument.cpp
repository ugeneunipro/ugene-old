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

#include <QtCore/QFile>
#include <QtXml/qdom.h>

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/Task.h>
#include <U2Core/SelectionUtils.h>
#include <U2Lang/HRSchemaSerializer.h>
#include <U2Lang/WorkflowIOTasks.h>
#include <U2Lang/WorkflowUtils.h>

#include "WorkflowViewController.h"
#include "HRSceneSerializer.h"
#include "WorkflowDocument.h"

/* TRANSLATOR U2::IOAdapter */

namespace U2 {

const GObjectType WorkflowGObject::TYPE("workflow-obj");
const GObjectViewFactoryId WorkflowViewFactory::ID("workflow-view-factory");

const DocumentFormatId WorkflowDocFormat::FORMAT_ID = "WorkflowDocFormat";

bool WorkflowGObject::isTreeItemModified () const {
    if (view) {
        return view->getScene()->isModified();
    }
    return GObject::isItemModified();
}

void WorkflowGObject::setView(WorkflowView* _view) {
    view = _view;
}

void WorkflowGObject::setSceneRawData(const QString & data) {
    assert(view != NULL);
    assert(!view->getScene()->isModified());
    serializedScene = data;
}

GObject* WorkflowGObject::clone() const {
    WorkflowGObject* copy = new WorkflowGObject(getGObjectName(), serializedScene, getGHintsMap());
    assert(!view);
    return copy;
}

//////////////////////////////////////////////////////////////////////////
/// Workflow document format

WorkflowDocFormat::WorkflowDocFormat(QObject* p) 
: DocumentFormat(p, DocumentFormatFlags_W1, QStringList(WorkflowUtils::WD_FILE_EXTENSIONS) << WorkflowUtils::WD_XML_FORMAT_EXTENSION),
  formatName(tr("Workflow Schema")) {
    supportedObjectTypes += WorkflowGObject::TYPE;
	formatDescription = tr("WorkflowDoc is a format used for creating/editing/storing/retrieving"
		"workflow schema with the text file");
}

Document* WorkflowDocFormat::createNewDocument(IOAdapterFactory* io, const QString& url, const QVariantMap& fs) {
    Document* d = DocumentFormat::createNewDocument(io, url, fs);
    GObject* o = new WorkflowGObject(tr("Workflow Schema"), "");
    d->addObject(o);
    return d;
}

#define BUFF_SIZE 1024

Document* WorkflowDocFormat::loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode) {
    Q_UNUSED(mode);
    QByteArray  rawData;
    QByteArray block(BUFF_SIZE, '\0');
    int blockLen = 0;
    while ((blockLen = io->readBlock(block.data(), BUFF_SIZE)) > 0) {
        rawData.append(block.data(), blockLen);
        ti.progress = io->getProgress();
    }
    
    if (checkRawData(rawData).score != FormatDetection_Matched) {
        ti.setError(tr("Invalid header. %1 expected").arg(HRSchemaSerializer::HEADER_LINE));
        rawData.clear();
        return NULL;
    }
    //todo: check file-readonly status?
    
    QList<GObject*> objects;
    QString data = QString::fromUtf8(rawData.data(), rawData.size());
    objects.append(new WorkflowGObject(tr("Workflow Schema"), data));
    return new Document(this, io->getFactory(), io->getURL(), objects, fs);
}

void WorkflowDocFormat::storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io) {
    Q_UNUSED(ts)
    assert(d->getDocumentFormat() == this);
    assert(d->getObjects().size() ==1);

    WorkflowGObject* wo = qobject_cast<WorkflowGObject*>(d->getObjects().first());
    assert(wo && wo->getView());

    QByteArray rawData = HRSceneSerializer::scene2String(wo->getView()->getScene(), wo->getView()->getMeta()).toUtf8();
    int nWritten = 0;
    int nTotal = rawData.size();
    while(nWritten < nTotal) {
        int d = io->writeBlock(rawData.data() + nWritten, nTotal - nWritten);
        assert(d > 0);
        nWritten+= d;
    }
    wo->getView()->getScene()->setModified(false);
    wo->setSceneRawData(rawData);
}

FormatCheckResult WorkflowDocFormat::checkRawData(const QByteArray& data, const GUrl&) const {
    LoadWorkflowTask::FileFormat format = LoadWorkflowTask::detectFormat(data);
    bool ok  = format == LoadWorkflowTask::HR || format == LoadWorkflowTask::XML;
    return ok ? FormatDetection_Matched  : FormatDetection_NotMatched;
}

bool WorkflowViewFactory::canCreateView(const MultiGSelection& multiSelection) {
    foreach(GObject* go, SelectionUtils::findObjects(WorkflowGObject::TYPE, &multiSelection, UOF_LoadedOnly)) {
        if (!qobject_cast<WorkflowGObject*>(go)->getView()) {
            return true;
        }
    }
    return false;
}

Task* WorkflowViewFactory::createViewTask(const MultiGSelection& multiSelection, bool single) {
    QSet<Document*> documents = SelectionUtils::findDocumentsWithObjects(WorkflowGObject::TYPE, &multiSelection, UOF_LoadedAndUnloaded, true);
    if (documents.size() == 0) {
        return NULL;
    }
    Task* result = (single || documents.size() == 1) ? NULL : new Task(tr("Open multiple views"), TaskFlag_NoRun);
    foreach(Document* d, documents) {
        Task* t = new OpenWorkflowViewTask(d);
        if (result == NULL) {
            return t;
        } 
        result->addSubTask(t);
    }
    return result;
}

OpenWorkflowViewTask::OpenWorkflowViewTask(Document* doc) 
: ObjectViewTask(WorkflowViewFactory::ID)
{
    if (!doc->isLoaded()) {
        documentsToLoad.append(doc);
    } else {
        foreach(GObject* go, doc->findGObjectByType(WorkflowGObject::TYPE)) {
            selectedObjects.append(go) ;
        }
        assert(!selectedObjects.isEmpty());
    }
}

void OpenWorkflowViewTask::open() {
    if (stateInfo.hasError()) {
        return;
    }
    if (!documentsToLoad.isEmpty()) {
        foreach(GObject* go, documentsToLoad.first()->findGObjectByType(WorkflowGObject::TYPE)) {
            selectedObjects.append(go) ;
        }
    }
    foreach(QPointer<GObject> po, selectedObjects) {
        WorkflowGObject* o = qobject_cast<WorkflowGObject*>(po);
        assert(o && !o->getView());
        WorkflowView* view = new WorkflowView(o);
        AppContext::getMainWindow()->getMDIManager()->addMDIWindow(view);
        AppContext::getMainWindow()->getMDIManager()->activateWindow(view);
    }
}

}//namespace
