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

#include "TreeViewerTasks.h"
#include "TreeViewer.h"
#include "TreeViewerFactory.h"
#include "TreeViewerState.h"
#include "CreateRectangularBranchesTask.h"
#include "GraphicsRectangularBranchItem.h"

#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentModel.h>

#include <U2Core/PhyTreeObject.h>
#include <U2Core/UnloadedObject.h>
#include <U2Core/GObjectTypes.h>

#include <QtCore/QSet>

namespace U2 {

/* TRANSLATOR U2::TreeViewer */
/* TRANSLATOR U2::ObjectViewTask */

//////////////////////////////////////////////////////////////////////////
/// open new view

OpenTreeViewerTask::OpenTreeViewerTask(PhyTreeObject* _obj)
: ObjectViewTask(TreeViewerFactory::ID), phyObject(_obj) {
    assert(!phyObject.isNull());
}

OpenTreeViewerTask::OpenTreeViewerTask(UnloadedObject* _obj)
: ObjectViewTask(TreeViewerFactory::ID), unloadedReference(_obj) {
    assert(_obj->getLoadedObjectType() == GObjectTypes::PHYLOGENETIC_TREE);
    documentsToLoad.append(_obj->getDocument());
}

OpenTreeViewerTask::OpenTreeViewerTask(Document* doc) 
: ObjectViewTask(TreeViewerFactory::ID), phyObject(NULL)
{
    assert(!doc->isLoaded());
    documentsToLoad.append(doc);
}

void OpenTreeViewerTask::open() {
    if (stateInfo.hasErrors() || (phyObject.isNull() && documentsToLoad.isEmpty())) {
        return;
    }
    if (phyObject.isNull()) {
        Document* doc = documentsToLoad.first();
        QList<GObject*> objects;
        if (unloadedReference.isValid()) {
            GObject* obj = doc->findGObjectByName(unloadedReference.objName);
            if (obj != NULL && obj->getGObjectType() == GObjectTypes::PHYLOGENETIC_TREE) {
                phyObject = qobject_cast<PhyTreeObject*>(obj);
            }
        } else {
            QList<GObject*> objects = doc->findGObjectByType(GObjectTypes::PHYLOGENETIC_TREE, UOF_LoadedAndUnloaded);
            phyObject = objects.isEmpty() ? NULL : qobject_cast<PhyTreeObject*>(objects.first());
        }
        if (phyObject.isNull()) {
            stateInfo.setError(tr("Phylogenetic tree object not found"));
            return;
        }
    }
    viewName = GObjectViewUtils::genUniqueViewName(phyObject->getDocument(), phyObject);
    uiLog.details(tr("Opening tree viewer for object %1").arg(phyObject->getGObjectName()));

    Task* createTask = new CreateTreeViewerTask(viewName, phyObject, stateData);
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    scheduler->registerTopLevelTask(createTask);
}

//////////////////////////////////////////////////////////////////////////
// open view from state


OpenSavedTreeViewerTask::OpenSavedTreeViewerTask(const QString& viewName, const QVariantMap& stateData) 
: ObjectViewTask(TreeViewerFactory::ID, viewName, stateData)
{
    TreeViewerState state(stateData);
    GObjectReference ref = state.getPhyObject();
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

void OpenSavedTreeViewerTask::open() {
    if (stateInfo.hasErrors()) {
        return;
    }
    TreeViewerState state(stateData);
    GObjectReference ref = state.getPhyObject();
    Document* doc = AppContext::getProject()->findDocumentByURL(ref.docUrl);
    if (doc == NULL) {
        stateIsIllegal = true;
        stateInfo.setError(L10N::errorDocumentNotFound(ref.docUrl));
        return;
    }
    GObject* obj = doc->findGObjectByName(ref.objName);
    if (obj == NULL || obj->getGObjectType() != GObjectTypes::PHYLOGENETIC_TREE) {
        stateIsIllegal = true;
        stateInfo.setError(tr("DNA sequence object not found: %1").arg(ref.objName));
        return;
    }
    PhyTreeObject* phyObject = qobject_cast<PhyTreeObject*>(obj);
    assert(phyObject != NULL);

    Task* createTask = new CreateTreeViewerTask(viewName, phyObject, stateData);
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    scheduler->registerTopLevelTask(createTask);
}

void OpenSavedTreeViewerTask::updateRanges(const QVariantMap& stateData, TreeViewer* ctx) {
    TreeViewerState state(stateData);

    QTransform m = state.getTransform();
    if (m != QTransform()) {
        ctx->setTransform(m);
    }

    qreal zoom = state.getZoom();
    ctx->setZoom(zoom);

    ctx->setSettingsState(stateData);
}

//////////////////////////////////////////////////////////////////////////
// update
UpdateTreeViewerTask::UpdateTreeViewerTask(GObjectView* v, const QString& stateName, const QVariantMap& stateData) 
: ObjectViewTask(v, stateName, stateData)
{
}

void UpdateTreeViewerTask::update() {
    if (view.isNull() || view->getFactoryId() != TreeViewerFactory::ID) {
        return; //view was closed;
    }

    TreeViewer* phyView = qobject_cast<TreeViewer*>(view.data());
    assert(phyView != NULL);

    OpenSavedTreeViewerTask::updateRanges(stateData, phyView);
}



//////////////////////////////////////////////////////////////////////////
/// create view

CreateTreeViewerTask::CreateTreeViewerTask(const QString& name, const QPointer<PhyTreeObject>& obj, const QVariantMap& sData)
: Task("Open tree viewer", TaskFlag_NoRun), viewName(name), phyObj(obj), subTask(NULL), stateData(sData) {}

void CreateTreeViewerTask::prepare() {
    subTask = new CreateRectangularBranchesTask(phyObj->getTree()->rootNode);
    addSubTask(subTask);
}

Task::ReportResult CreateTreeViewerTask::report() {
    GraphicsRectangularBranchItem* root = dynamic_cast<GraphicsRectangularBranchItem*>(subTask->getResult());
    TreeViewer* v = new TreeViewer(viewName, phyObj, root, subTask->getScale());

    GObjectViewWindow* w = new GObjectViewWindow(v, viewName, !stateData.isEmpty());
    MWMDIManager* mdiManager = AppContext::getMainWindow()->getMDIManager();
    mdiManager->addMDIWindow(w);
    if (!stateData.isEmpty()) {
        OpenSavedTreeViewerTask::updateRanges(stateData, v);
    }
    return Task::ReportResult_Finished;
}

} // namespace
