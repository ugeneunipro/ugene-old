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
#include "MSAEditorTreeManager.h"
#include <ov_phyltree/TreeViewerTasks.h>

#include <U2Core/U2SafePoints.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/TaskSignalMapper.h>

#include <U2View/CreatePhyTreeDialogController.h>
#include <U2View/TreeViewer.h>
#include <U2View/MSAEditorMultiTreeViewer.h>
#include <U2View/TreeOptionsWidgetFactory.h>
#include <U2View/GraphicsRectangularBranchItem.h>
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>
#include <U2View/MSAEditorNameList.h>
#include <U2View/MSAEditorDataList.h>

#include <U2Algorithm/PhyTreeGeneratorTask.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>

#include <QtGui/QFileDialog>

#include <U2Core/BaseDocumentFormats.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/ExportDocumentDialogController.h>
#include <U2Gui/OpenViewTask.h>

#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include <U2Algorithm/MSADistanceAlgorithm.h>

#include <ov_phyltree/TreeViewerTasks.h>

#include "AddTreeDialog.h"


namespace U2 {
MSAEditorTreeManager::MSAEditorTreeManager(MSAEditor* _editor )
 : QObject(_editor), editor(_editor), msaObject(NULL), treeGeneratorTask(NULL), addExistingTree(false) {
     SAFE_POINT(NULL != editor, "Invlalid parameter were passed into constructor MSAEditorTreeManager",);
}

void MSAEditorTreeManager::loadRelatedTrees() {
    msaObject = editor->getMSAObject();
    QList<GObjectRelation> relatedTrees = editor->getMSAObject()->findRelatedObjectsByRole(GObjectRelationRole::PHYLOGENETIC_TREE); 
    CHECK(!relatedTrees.isEmpty(),);
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    foreach(const GObjectRelation rel, relatedTrees) {
        const QString& treeFileName = rel.getDocURL();
        loadTreeFromFile(treeFileName);
    }
}

void MSAEditorTreeManager::buildTreeWithDialog() {
    msaObject = editor->getMSAObject();
    PhyTreeGeneratorRegistry* registry = AppContext::getPhyTreeGeneratorRegistry();
    QStringList list = registry->getNameList();
    addExistingTree = false;
    if (list.size() == 0) {
        QMessageBox::information(editor->getUI(), tr("Calculate phy tree"),
            tr("No algorithms for building phylogenetic tree are available.") );
        return;
    }        
    CreatePhyTreeDialogController dlg(editor->getUI(), msaObject, settings);

    int rc = dlg.exec();
    CHECK(rc == QDialog::Accepted, );
    settings.rowsOrder = msaObject->getMAlignment().getRowNames();
    buildTree(settings);
}

void MSAEditorTreeManager::buildTree(const CreatePhyTreeSettings& buildSettings) {
    const MAlignment& ma = msaObject->getMAlignment();

    settings = buildSettings;

    treeGeneratorTask = new PhyTreeGeneratorLauncherTask(ma, settings);
    connect(treeGeneratorTask, SIGNAL(si_stateChanged()), SLOT(sl_openTree()));
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    scheduler->registerTopLevelTask(treeGeneratorTask);
}

void MSAEditorTreeManager::sl_refreshTree(MSAEditorTreeViewer& treeViewer) {
    CHECK(canRefreshTree(&treeViewer),);
    refreshingTree = &treeViewer;
    const MAlignment& ma = msaObject->getMAlignment();
    settings = treeViewer.getCreatePhyTreeSettings();

    treeGeneratorTask = new PhyTreeGeneratorLauncherTask(ma, settings);
    connect(treeGeneratorTask, SIGNAL(si_stateChanged()), SLOT(sl_treeRebuildingFinished()));
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    scheduler->registerTopLevelTask(treeGeneratorTask);
}
void MSAEditorTreeManager::sl_treeRebuildingFinished() {
    bool taskFailed = treeGeneratorTask->getState() != Task::State_Finished || treeGeneratorTask->hasError() || treeGeneratorTask->isCanceled();
    CHECK(!taskFailed, );
    PhyTreeObject *treeObj = refreshingTree->getPhyObject();
    PhyTree res = treeGeneratorTask->getResult();

    treeObj->setTree(treeGeneratorTask->getResult());
}

bool MSAEditorTreeManager::canRefreshTree(MSAEditorTreeViewer* treeViewer) {
    bool canRefresh = (treeViewer->getParentAlignmentName() == msaObject->getMAlignment().getName());
    return canRefresh;
}

void MSAEditorTreeManager::sl_openTree() {
    bool taskIsFailed = treeGeneratorTask->getState() != Task::State_Finished || treeGeneratorTask->hasError() || treeGeneratorTask->isCanceled();
    CHECK(!taskIsFailed, )

    const GUrl& msaURL = msaObject->getDocument()->getURL();
    SAFE_POINT(!msaURL.isEmpty(), QString("Tree URL in MSAEditorTreeManager::sl_openTree() is empty"),);

    Project* p = AppContext::getProject();
    d = NULL;
    PhyTreeObject *newObj;
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    QString treeFileName = settings.fileUrl.getURLString();
    if (treeFileName.isEmpty()) {
        treeFileName = GUrlUtils::rollFileName(msaURL.dirPath() + "/" + msaURL.baseFileName() + ".nwk", DocumentUtils::getNewDocFileNameExcludesHint());
    }

    DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::NEWICK);
    IOAdapterFactory *iof = IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE);

    const QList<Document *> documents = AppContext::getProject()->getDocuments();
    bool isNewDocument = true;
    foreach(Document *doc, documents) {
        if(treeFileName == doc->getURLString()) {
            d = doc;
            isNewDocument = false;
            break;
        }
    }

    if(NULL == d) {
        U2OpStatus2Log os;
        d = df->createNewLoadedDocument(iof, treeFileName, os);
        CHECK_OP(os, );
    }

    if(isNewDocument) {
        newObj = new PhyTreeObject(treeGeneratorTask->getResult(), "Tree");
        d->addObject(newObj);
    }
    else {
        if(!d->isLoaded()) {
            phyTree = treeGeneratorTask->getResult();
            LoadUnloadedDocumentTask* t = new LoadUnloadedDocumentTask(d);
            bool res = connect(new TaskSignalMapper(t), SIGNAL(si_taskSucceeded(Task*)), SLOT(sl_onPhyTreeDocLoaded(Task*)));
            AppContext::getTaskScheduler()->registerTopLevelTask(t);
            return;
        }
        else {
            const QList<GObject*>& objects = d->getObjects();
            foreach(GObject* obj, objects) {
                PhyTreeObject* treeObj = qobject_cast<PhyTreeObject*>(obj);
                if(treeObj) {
                    treeObj->setTree(treeGeneratorTask->getResult());
                    newObj = treeObj;
                }
            }
        }
    }

    if(!p->getDocuments().contains(d)) {
        p->addDocument(d);
    }

    if(isNewDocument) {
        GObjectReference treeRef(treeFileName, "", GObjectTypes::PHYLOGENETIC_TREE);
        treeRef.objName = newObj->getGObjectName();
        msaObject->addObjectRelation(GObjectRelation(treeRef, GObjectRelationRole::PHYLOGENETIC_TREE));
    }

    Task* saveTask = new SaveDocumentTask(d);
    scheduler->registerTopLevelTask(saveTask);

    openTreeViewer(newObj);
}

void MSAEditorTreeManager::sl_onPhyTreeDocLoaded(Task* task) {
    LoadUnloadedDocumentTask* loadTask = qobject_cast<LoadUnloadedDocumentTask*>(task);
    d = loadTask->getDocument();
    PhyTreeObject* treeObj = NULL;
    foreach(GObject* obj, d->getObjects()) {
        treeObj = qobject_cast<PhyTreeObject*>(obj);
        if(treeObj) {
            treeObj->setTree(phyTree);
            break;
        }
    }
    openTreeViewer(treeObj);
}

void MSAEditorTreeManager::openTreeViewer(PhyTreeObject* treeObj) {
    Task* openTask = NULL;
    if(true == settings.displayWithAlignmentEditor) {
        openTask = new MSAEditorOpenTreeViewerTask(treeObj, this);
    }
    else {
        openTask = new OpenTreeViewerTask(treeObj, this);
    }
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    scheduler->registerTopLevelTask(openTask);
}

void MSAEditorTreeManager::sl_openTreeTaskFinished(Task* t) {
    CreateMSAEditorTreeViewerTask* task = qobject_cast<CreateMSAEditorTreeViewerTask*> (t);
    if(NULL != task) {
        if(settings.displayWithAlignmentEditor) {
            MSAEditorTreeViewer* treeView = dynamic_cast<MSAEditorTreeViewer*>(task->getTreeViewer());
            SAFE_POINT(NULL != treeView, tr("Can not convert TreeViewer* to MSAEditorTreeViewer* in function MSAEditorTreeManager::sl_openTreeTaskFinished(Task* t)"),);
            GObjectViewWindow* w = new GObjectViewWindow(task->getTreeViewer(), editor->getName(), !task->getStateData().isEmpty());

            connect(w, SIGNAL(si_windowClosed(GObjectViewWindow*)), this, SLOT(sl_onWindowClosed(GObjectViewWindow*)));

            MSAEditorUI* msaUI = editor->getUI();
            msaUI->addTreeView(w);

            if(!addExistingTree) {
                treeView->setCreatePhyTreeSettings(settings);
                treeView->setParentAignmentName(msaObject->getMAlignment().getName());
            }

            const TreeViewerUI* treeUI = treeView->getTreeViewerUI();

            treeView->setMSAEditor(editor);
            treeView->setSynchronizationMode(settings.syncAlignmentWithTree ? FullSynchronization : OnlySeqsSelection);
            
            connect(treeView, SIGNAL(si_refreshTree(MSAEditorTreeViewer&)), SLOT(sl_refreshTree(MSAEditorTreeViewer&)));
        }
        else {
            GObjectViewWindow* w = new GObjectViewWindow(task->getTreeViewer(), editor->getName(), !task->getStateData().isEmpty());
            MWMDIManager* mdiManager = AppContext::getMainWindow()->getMDIManager();
            mdiManager->addMDIWindow(w);
        }
    }
}


void MSAEditorTreeManager::addTreeToMSA() {
    LastUsedDirHelper h;
    QString filter = DialogUtils::prepareDocumentsFileFilter(BaseDocumentFormats::NEWICK, false, QStringList());
    QString file = QFileDialog::getOpenFileName(QApplication::activeWindow(), tr("Select files to open..."), h.dir,  filter);
    CHECK(!file.isEmpty(),);
    if (QFileInfo(file).exists()) {
        h.url = file;
        loadTreeFromFile(file);
    }
}

void MSAEditorTreeManager::loadTreeFromFile(const QString& treeFileName) {
    TaskScheduler* scheduler = AppContext::getTaskScheduler();

    DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::NEWICK);
    IOAdapterFactory *iof = IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE);
    U2OpStatus2Log os;

    const QList<Document *> documents = AppContext::getProject()->getDocuments();
    addExistingTree = true;
    bool isNewDocument = true;
    Document *doc = NULL;
    foreach(doc, documents) {
        if(treeFileName == doc->getURLString()) {
            isNewDocument = false;
            break;
        }
    }

    if(isNewDocument || !doc->isLoaded()) {
        if(!isNewDocument && !doc->isLoaded()) {
            if(AppContext::getProject()->getDocuments().contains(doc)) {
                AppContext::getProject()->removeDocument(doc);
            }
        }
        doc = df->loadDocument(iof, treeFileName, QVariantMap(), os);
        CHECK(NULL != doc,);
        AppContext::getProject()->addDocument(doc);
    }

    const QList<GObject*>& objects = doc->getObjects();
    const MSAEditorMultiTreeViewer* mtv = editor->getUI()->getMultiTreeViewer();
    foreach(GObject* obj, objects) {
        if(GObjectTypes::PHYLOGENETIC_TREE == obj->getGObjectType()) {
            PhyTreeObject* treeObject = qobject_cast<PhyTreeObject*>(obj);
            if(NULL == treeObject) {
                continue;
            }
            if(NULL != mtv) {
                CHECK(!mtv->getTreeNames().contains(doc->getName()),);
            }
            Task* task = new MSAEditorOpenTreeViewerTask(treeObject, this);
            scheduler->registerTopLevelTask(task);
        }
    }
}

void MSAEditorTreeManager::showAddTreeDialog() {
    AddTreeDialog dlg(editor->getUI(), editor);
    int rc = dlg.exec();
    CHECK(rc == QDialog::Accepted, );
    if(dlg.buildTreeCheck->isChecked()) {
        buildTreeWithDialog();
    }
    else {
        addTreeToMSA();
    }
}

void MSAEditorTreeManager::sl_onWindowClosed(GObjectViewWindow* viewWindow) {
    editor->getUI()->getMultiTreeViewer()->sl_onTabCloseRequested(viewWindow);
}

}//namespace
