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

#include "MSAEditorTasks.h"
#include "MSAEditor.h"
#include "MSAEditorFactory.h"
#include "MSAEditorState.h"

#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentModel.h>

#include <U2Core/MAlignmentObject.h>
#include <U2Core/UnloadedObject.h>
#include <U2Core/GObjectTypes.h>


#include <QtCore/QSet>

namespace U2 {

/* TRANSLATOR U2::MSAEditor */
/* TRANSLATOR U2::ObjectViewTask */

//////////////////////////////////////////////////////////////////////////
/// open new view

OpenMSAEditorTask::OpenMSAEditorTask(MAlignmentObject* _obj) 
: ObjectViewTask(MSAEditorFactory::ID), msaObject(_obj)
{
    assert(!msaObject.isNull());
}

OpenMSAEditorTask::OpenMSAEditorTask(UnloadedObject* _obj) 
: ObjectViewTask(MSAEditorFactory::ID), unloadedReference(_obj)
{
    assert(_obj->getLoadedObjectType() == GObjectTypes::MULTIPLE_ALIGNMENT);
    documentsToLoad.append(_obj->getDocument());
}

OpenMSAEditorTask::OpenMSAEditorTask(Document* doc) 
: ObjectViewTask(MSAEditorFactory::ID), msaObject(NULL)
{
    assert(!doc->isLoaded());
    documentsToLoad.append(doc);
}

void OpenMSAEditorTask::open() {
    if (stateInfo.hasError() || (msaObject.isNull() && documentsToLoad.isEmpty())) {
        return;
    }
    if (msaObject.isNull()) {
        Document* doc = documentsToLoad.first();
        QList<GObject*> objects;
        if (unloadedReference.isValid()) {
            GObject* obj = doc->findGObjectByName(unloadedReference.objName);
            if (obj!=NULL && obj->getGObjectType() == GObjectTypes::MULTIPLE_ALIGNMENT) {
                msaObject = qobject_cast<MAlignmentObject*>(obj);
            }
        } else {
            QList<GObject*> objects = doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT, UOF_LoadedAndUnloaded);
            msaObject = objects.isEmpty() ? NULL : qobject_cast<MAlignmentObject*>(objects.first());
        }
        if (msaObject.isNull()) {
            stateInfo.setError(tr("Multiple alignment object not found"));
            return;
        }
    }
    viewName = GObjectViewUtils::genUniqueViewName(msaObject->getDocument(), msaObject);
    uiLog.details(tr("Opening MSA editor for object: %1").arg(msaObject->getGObjectName()));

    MSAEditor* v = new MSAEditor(viewName, msaObject);
    GObjectViewWindow* w = new GObjectViewWindow(v, viewName, false);
    MWMDIManager* mdiManager = AppContext::getMainWindow()->getMDIManager();
    mdiManager->addMDIWindow(w);

}

//////////////////////////////////////////////////////////////////////////
// open view from state


OpenSavedMSAEditorTask::OpenSavedMSAEditorTask(const QString& viewName, const QVariantMap& stateData) 
: ObjectViewTask(MSAEditorFactory::ID, viewName, stateData)
{
    MSAEditorState state(stateData);
    GObjectReference ref = state.getMSAObject();
    Document* doc = AppContext::getProject()->findDocumentByURL(ref.docUrl);
    if (doc == NULL) {
        doc = createDocumentAndAddToProject(ref.docUrl, AppContext::getProject());
        if (!doc) {
            stateIsIllegal = true;
            stateInfo.setError(L10N::errorDocumentNotFound(ref.docUrl));
            return;
        }
    }
    if (!doc->isLoaded()) {
        documentsToLoad.append(doc);
    }

}

void OpenSavedMSAEditorTask::open() {
    if (stateInfo.hasError()) {
        return;
    }
    MSAEditorState state(stateData);
    GObjectReference ref = state.getMSAObject();
    Document* doc = AppContext::getProject()->findDocumentByURL(ref.docUrl);
    if (doc == NULL) {
        stateIsIllegal = true;
        stateInfo.setError(L10N::errorDocumentNotFound(ref.docUrl));
        return;
    }
    GObject* obj = doc->findGObjectByName(ref.objName);
    if (obj == NULL || obj->getGObjectType() != GObjectTypes::MULTIPLE_ALIGNMENT) {
        stateIsIllegal = true;
        stateInfo.setError(tr("DNA sequence object not found: %1").arg(ref.objName));
        return;
    }
    MAlignmentObject* msaObject = qobject_cast<MAlignmentObject*>(obj);
    assert(msaObject!=NULL);

    MSAEditor* v = new MSAEditor(viewName, msaObject);
    GObjectViewWindow* w = new GObjectViewWindow(v, viewName, true);
    MWMDIManager* mdiManager =     AppContext::getMainWindow()->getMDIManager();
    mdiManager->addMDIWindow(w);

    updateRanges(stateData, v);
}

void OpenSavedMSAEditorTask::updateRanges(const QVariantMap& stateData, MSAEditor* ctx) {
    Q_UNUSED(ctx);
    MSAEditorState state(stateData);

    QFont f = state.getFont();
    if (!f.isCopyOf(QFont())) {
        ctx->setFont(f);
    }

    int firstPos = state.getFirstPos();
    ctx->setFirstVisibleBase(firstPos);

    float zoomFactor = state.getZoomFactor();
    ctx->setZoomFactor(zoomFactor);
}


//////////////////////////////////////////////////////////////////////////
// update
UpdateMSAEditorTask::UpdateMSAEditorTask(GObjectView* v, const QString& stateName, const QVariantMap& stateData) 
: ObjectViewTask(v, stateName, stateData)
{
}

void UpdateMSAEditorTask::update() {
    if (view.isNull() || view->getFactoryId() != MSAEditorFactory::ID) {
        return; //view was closed;
    }

    MSAEditor* msaView = qobject_cast<MSAEditor*>(view.data());
    assert(msaView!=NULL);

    OpenSavedMSAEditorTask::updateRanges(stateData, msaView);
}

} // namespace
