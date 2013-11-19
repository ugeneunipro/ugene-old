/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "ProjectTreeController.h"
#include "UnloadDocumentTask.h"

#include <U2Core/ProjectModel.h>
#include <U2Core/GObject.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/ResourceTracker.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/Log.h>
#include <U2Core/RemoveDocumentTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/UnloadedObject.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/MainWindow.h>
#include <U2Gui/ProjectView.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>

#include <QtCore/QMimeData>
#include <QtCore/QMap>
#include <QtGui/QDrag>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QKeyEvent>
#include <QtGui/QApplication>

/* TRANSLATOR U2::ProjectTreeController */

namespace U2 {


class TreeUpdateHelper {
public:
    
    TreeUpdateHelper(QSet<ProjViewItem*>& _itemsToUpdate) 
        :itemsToUpdate(_itemsToUpdate) { assert(itemsToUpdate.isEmpty());}
    
    virtual ~TreeUpdateHelper() {
        foreach(ProjViewItem* i, itemsToUpdate) {
            i->updateVisual();
        }
        itemsToUpdate.clear();
    }
    QSet<ProjViewItem*>& itemsToUpdate;
};

bool PTCObjectRelationFilter::filter(GObject* o) const {
    return !o->hasObjectRelation(rel);
}


ProjectTreeController::ProjectTreeController(QObject* parent, QTreeWidget* _tree, const ProjectTreeControllerModeSettings& m) 
: QObject(parent), mode(m) {

    markActiveView = NULL;
    setObjectName("ProjectTreeController");

    tree = _tree;
    tree->setSelectionMode(mode.allowMultipleSelection ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
    tree->headerItem()->setHidden(true);
    tree->setContextMenuPolicy(Qt::CustomContextMenu);
    ProjItemDelegate *delegate = new ProjItemDelegate(tree);
    tree->setItemDelegate(delegate);
    connect(delegate, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)),
        SLOT(sl_onCloseEditor(QWidget *, QAbstractItemDelegate::EndEditHint)));

    documentIcon.addFile(":/core/images/document.png");
    roDocumentIcon.addFile(":/core/images/ro_document.png");
    
    addObjectToDocumentAction = new QAction(QIcon(":core/images/add_gobject.png"), tr("Add object to document"), this);
    tree->addAction(addObjectToDocumentAction);
    connect(addObjectToDocumentAction, SIGNAL(triggered()), SLOT(sl_onAddObjectToSelectedDocument()));
    
    removeSelectedDocumentsAction = new QAction(QIcon(":core/images/remove_selected_documents.png"), tr("Remove selected documents"), this);
    removeSelectedDocumentsAction->setShortcut(QKeySequence::Delete);
    removeSelectedDocumentsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    tree->addAction(removeSelectedDocumentsAction);
    connect(removeSelectedDocumentsAction, SIGNAL(triggered()), SLOT(sl_onRemoveSelectedDocuments()));
    
    removeSelectedObjectsAction = new QAction(QIcon(":core/images/remove_gobject.png"), tr("Remove object from document"), this);
    connect(removeSelectedObjectsAction, SIGNAL(triggered()), SLOT(sl_onRemoveSelectedObjects()));

    loadSelectedDocumentsAction = new QAction(QIcon(":core/images/load_selected_documents.png"), tr("Load selected documents"), this);
    loadSelectedDocumentsAction->setObjectName("action_load_selected_documents");
    loadSelectedDocumentsAction->setShortcuts(QList<QKeySequence>() << Qt::Key_Enter << Qt::Key_Return);
    loadSelectedDocumentsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    tree->addAction(loadSelectedDocumentsAction);
    connect(loadSelectedDocumentsAction, SIGNAL(triggered()), SLOT(sl_onLoadSelectedDocuments()));

    addReadonlyFlagAction = new QAction(tr("Lock document for editing"), this);
    addReadonlyFlagAction->setObjectName(ACTION_DOCUMENT__LOCK);
    connect(addReadonlyFlagAction, SIGNAL(triggered()), SLOT(sl_onToggleReadonly()));

    removeReadonlyFlagAction = new QAction(tr("Unlock document for editing"), this);
    removeReadonlyFlagAction->setObjectName(ACTION_DOCUMENT__UNLOCK);
    connect(removeReadonlyFlagAction, SIGNAL(triggered()), SLOT(sl_onToggleReadonly()));

    unloadSelectedDocumentsAction = new QAction( QIcon(":core/images/unload_document.png"), tr("Unload selected document"), this);
    connect(unloadSelectedDocumentsAction, SIGNAL(triggered()), SLOT(sl_onUnloadSelectedDocuments()));

    renameAction = new QAction(tr("Rename..."), this);
    connect(renameAction, SIGNAL(triggered()), SLOT(sl_onRename()));
    renameAction->setObjectName("Rename");

    groupByDocumentAction = new QAction(tr("Group by document"), this);
    groupByDocumentAction->setObjectName("Group_by_document_action");
    groupByDocumentAction->setCheckable(true);
    groupByDocumentAction->setChecked(mode.groupMode == ProjectTreeGroupMode_ByDocument);
    connect(groupByDocumentAction, SIGNAL(triggered()), SLOT(sl_onGroupByDocument()));

    groupByTypeAction = new QAction(tr("Group by type"), this);
    groupByTypeAction->setObjectName("Group_by_type_action");
    groupByTypeAction->setCheckable(true);
    groupByTypeAction->setChecked(mode.groupMode == ProjectTreeGroupMode_ByType);
    connect(groupByTypeAction, SIGNAL(triggered()), SLOT(sl_onGroupByType()));

    groupFlatAction = new QAction(tr("No groups"), this);
    groupFlatAction->setObjectName("No_groups_action");
    groupFlatAction->setCheckable(true);
    groupFlatAction->setChecked(mode.groupMode == ProjectTreeGroupMode_Flat);
    connect(groupFlatAction, SIGNAL(triggered()), SLOT(sl_onGroupFlat()));

    QActionGroup* ag = new QActionGroup(this);
    ag->addAction(groupByDocumentAction);
    ag->addAction(groupByTypeAction);
    ag->addAction(groupFlatAction);

    connect(tree, SIGNAL(itemSelectionChanged()), SLOT(sl_onTreeSelectionChanged()));
    connect(tree, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sl_onContextMenuRequested(const QPoint &)));
    connect(tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(sl_onItemDoubleClicked(QTreeWidgetItem*, int )));
    tree->installEventFilter(this);

    Project* pr = AppContext::getProject();
    connect(pr, SIGNAL(si_documentAdded(Document*)), SLOT(sl_onDocumentAddedToProject(Document*)));
    connect(pr, SIGNAL(si_documentRemoved(Document*)), SLOT(sl_onDocumentRemovedFromProject(Document*)));

    MWMDIManager* mdi = AppContext::getMainWindow()->getMDIManager();
    connect(mdi, SIGNAL(si_windowActivated(MWMDIWindow*)), SLOT(sl_windowActivated(MWMDIWindow*)));

    connect(tree, SIGNAL(itemChanged ( QTreeWidgetItem *, int)), SLOT(sl_updateAfterItemchange(QTreeWidgetItem *, int)));

    connectModel();

    updateSettings(mode);

    connectToResourceTracker();

    tree->setExpandsOnDoubleClick(false);

    // if any window is active - check it content 
    sl_windowActivated(mdi->getActiveWindow());
}

void ProjectTreeController::connectToResourceTracker() {
    connect(AppContext::getResourceTracker(), 
        SIGNAL(si_resourceUserRegistered(const QString&, Task*)), 
        SLOT(sl_onResourceUserRegistered(const QString&, Task*)));

    connect(AppContext::getResourceTracker(), 
        SIGNAL(si_resourceUserUnregistered(const QString&, Task*)), 
        SLOT(sl_onResourceUserUnregistered(const QString&, Task*)));


    foreach(Document* d, AppContext::getProject()->getDocuments()) {
        const QString resName = LoadUnloadedDocumentTask::getResourceName(d);
        QList<Task*> users = AppContext::getResourceTracker()->getResourceUsers(resName);
        foreach(Task* t, users) {
            sl_onResourceUserRegistered(resName, t);
        }
    }
}

void ProjectTreeController::connectModel() {
    Project* p = AppContext::getProject();
    const QList<Document*>& docs = p->getDocuments();
    foreach(Document* d, docs) {
        connectDocument(d);
    }
}

void ProjectTreeController::connectDocument(Document* d) {
    connect(d, SIGNAL(si_modifiedStateChanged()), SLOT(sl_onDocumentModifiedStateChanged()));
    connect(d, SIGNAL(si_loadedStateChanged()), SLOT(sl_onDocumentLoadedStateChanged()));
    connect(d, SIGNAL(si_objectAdded(GObject*)), SLOT(sl_onObjectAdded(GObject*)));
    connect(d, SIGNAL(si_objectRemoved(GObject*)), SLOT(sl_onObjectRemoved(GObject*)));
    connect(d, SIGNAL(si_lockedStateChanged()), SLOT(sl_onLockedStateChanged()));
    connect(d, SIGNAL(si_urlChanged()), SLOT(sl_onDocumentURLorNameChanged()));
    connect(d, SIGNAL(si_nameChanged()), SLOT(sl_onDocumentURLorNameChanged()));
    
    const QList<GObject*>& objects = d->getObjects();
    foreach(GObject* o, objects) {
        connectGObject(o);
    }
}

void ProjectTreeController::disconnectDocument(Document* d) {
    d->disconnect(this);
    const QList<GObject*>& objects = d->getObjects();
    foreach(GObject* o, objects) {
        o->disconnect(this);
    }
    Task* t = LoadUnloadedDocumentTask::findActiveLoadingTask(d);
    if (t) {
        t->disconnect(this);
        t->cancel();
    }
}

void ProjectTreeController::connectGObject(GObject* o) {
    connect(o, SIGNAL(si_modifiedStateChanged()), SLOT(sl_onObjectModifiedStateChanged()));
    connect(o, SIGNAL(si_nameChanged(const QString&)), SLOT(sl_onObjectNameChanged(const QString&)));
}

void ProjectTreeController::buildTree() {
    const QList<Document*>& docs = AppContext::getProject()->getDocuments();
    foreach(Document* d, docs) {
        buildDocumentTree(d);
    }
}

GObjectType ProjectTreeController::getLoadedObjectType(GObject* obj) const {
    GObjectType t = obj->getGObjectType();
    if (t == GObjectTypes::UNLOADED) {
        UnloadedObject* uo = qobject_cast<UnloadedObject*>(obj);
        t = uo->getLoadedObjectType();
        if (t == GObjectTypes::UNLOADED) {
            coreLog.details(tr("Unloaded object refers to unloaded type: %1").arg(obj->getGObjectName()));
            t = GObjectTypes::UNKNOWN;
        }
    }
    return t;
}

// if objects number in document < MAX_OBJECTS_TO_AUTOEXPAND - document content will be automatically expanded on loading
#define MAX_OBJECTS_TO_AUTOEXPAND 20
// if documents number in project < MAX_DOCUMENTS_TO_AUTOEXPAND - document content will be automatically expanded on loading
#define MAX_DOCUMENTS_TO_AUTOEXPAND 20

void ProjectTreeController::buildDocumentTree(Document* d) {
    TreeUpdateHelper h(itemsToUpdate);

    ProjViewItem* topItem =  NULL;
    bool docIsShown = mode.isDocumentShown(d);
    if (docIsShown) {
        topItem = findDocumentItem(d, true);
    } 
    
    const QList<GObject*>& objs = d->getObjects();
    foreach(GObject* obj, objs) {
        assert(obj->getDocument() == d);
        if (mode.isObjectShown(obj)) {
            if (mode.groupMode == ProjectTreeGroupMode_ByType) {
                topItem = findTypeItem(getLoadedObjectType(obj), true);
            }
            ProjViewObjectItem* oItem = findGObjectItem(topItem, obj);
            if (oItem == NULL) {
                oItem = new ProjViewObjectItem(obj, this);
                if (mode.groupMode != ProjectTreeGroupMode_ByDocument || topItem == NULL) {
                    insertTreeItemSorted(topItem, oItem);
                } else { //keep natural mode, as in file
                    topItem->addChild(oItem);
                }
                if (topItem!=NULL) {
                    itemsToUpdate.insert(topItem);
                }
            }
        } 
    }

    if (topItem!=NULL && topItem->childCount() > 0) {
        insertTreeItemSorted(NULL, topItem);
        if (docIsShown && d->isLoaded() && topItem->childCount() < MAX_OBJECTS_TO_AUTOEXPAND) { 
            //avoid system slowdown without user interaction -> do not auto expand docs with huge number of objects
            topItem->setExpanded(true);
        }
    }
}

ProjViewTypeItem* ProjectTreeController::findTypeItem(const GObjectType& t) const {
    assert(t!=GObjectTypes::UNLOADED);

    for (int i=0;i<tree->topLevelItemCount(); i++) {
        ProjViewItem* item =  static_cast<ProjViewItem*>(tree->topLevelItem(i));
        if (!item->isTypeItem()) {
            continue;
        }
        ProjViewTypeItem* typeItem = static_cast<ProjViewTypeItem*>(item);
        if (typeItem->otype == t) {
            return typeItem;
        }
    }
    return NULL;
}

ProjViewTypeItem* ProjectTreeController::findTypeItem(const GObjectType& t, bool create) {
    ProjViewTypeItem *res = findTypeItem(t);
    if (res == NULL && create) {
        res = new ProjViewTypeItem(t, this);
        tree->addTopLevelItem(res);
    }
    return res;
}

ProjViewDocumentItem* ProjectTreeController::findDocumentItem(Document* doc) const {
    for (int i=0;i<tree->topLevelItemCount(); i++) {
        ProjViewItem* item = static_cast<ProjViewItem*>(tree->topLevelItem(i));
        if (!item->isDocumentItem()) {
            continue;
        }
        ProjViewDocumentItem* docItem = static_cast<ProjViewDocumentItem*>(item);
        if (docItem->doc == doc) {
            return docItem;
        }
    }
    return NULL;
}

ProjViewDocumentItem* ProjectTreeController::findDocumentItem(Document* doc, bool create){
    ProjViewDocumentItem* res = findDocumentItem(doc);
    if (res == NULL && create) {
        res = new ProjViewDocumentItem(doc, this);
        tree->addTopLevelItem(res);
    }
    return res;
}

ProjViewObjectItem* ProjectTreeController::findGObjectItem(GObject* obj) const {
    return findGObjectItem(obj->getDocument(), obj);
}

ProjViewObjectItem* ProjectTreeController::findGObjectItem(Document* doc, GObject* obj) const {
    assert(doc == obj->getDocument() || obj->getDocument() == NULL);
    assert(doc != NULL);
    ProjViewItem* topItem = NULL;
    if (mode.groupMode == ProjectTreeGroupMode_ByDocument) {
        topItem = findDocumentItem(doc);
    } else if (mode.groupMode == ProjectTreeGroupMode_ByType) {
        topItem = findTypeItem(getLoadedObjectType(obj));
    }
    return findGObjectItem(topItem, obj);
}

ProjViewObjectItem* ProjectTreeController::findGObjectItem(ProjViewItem* topItem, GObject* obj) const {
    if (topItem!=NULL) {
        for (int i = 0 ; i < topItem->childCount(); i++) {
            ProjViewObjectItem* item = static_cast<ProjViewObjectItem*>(topItem->child(i));
            if (item->obj == obj) {
                return item;
            }
        }
    } else {
        for (int i=0; i < tree->topLevelItemCount(); i++) {
            ProjViewItem* item = static_cast<ProjViewItem*>(tree->topLevelItem(i));
            if (!item->isObjectItem()) {
                continue;
            }
            ProjViewObjectItem* objItem = static_cast<ProjViewObjectItem*>(item);
            if (objItem->obj == obj) {
                return objItem;
            }
        }
    }
    return NULL;
}

void ProjectTreeController::sl_onTreeSelectionChanged() {
    updateSelection();
    updateActions();
}

bool ProjectTreeController::eventFilter(QObject* o, QEvent* e) {
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *kEvent = (QKeyEvent*)e;
        int key = kEvent->key();
        bool hasSelection = !((QTreeWidget *)o)->selectedItems().isEmpty();
        if (key == Qt::Key_F2 && hasSelection) {
            sl_onRename();
            return true;
        } else if ( (key == Qt::Key_Return || key == Qt::Key_Enter) && hasSelection) {
            ProjViewItem *item = static_cast<ProjViewItem *>(((QTreeWidget *)o)->selectedItems().last());
            if (item->isObjectItem() && !item->flags().testFlag(Qt::ItemIsEditable)) {
                ProjViewObjectItem *objItem = static_cast<ProjViewObjectItem*>(item);
                SAFE_POINT(objItem, "ProjectTreeController::eventFilter::cannot cast to ProjViewObjectItem", true);
                SAFE_POINT(objItem->obj, "ProjectTreeController::eventFilter::no object", true);
                emit si_returnPressed(objItem->obj);
            }else if(item->isDocumentItem()) {
                ProjViewDocumentItem *docItem = static_cast<ProjViewDocumentItem*>(item);
                SAFE_POINT(docItem, "ProjectTreeController::eventFilter::cannot cast to ProjViewDocumentItem", true);
                SAFE_POINT(docItem->doc, "ProjectTreeController::eventFilter::no document", true);
                emit si_returnPressed(docItem->doc);
            }

        }
    }
    return false;
}

void ProjectTreeController::sl_onCloseEditor(QWidget*,QAbstractItemDelegate::EndEditHint) {
    QTreeWidgetItem* item = tree->currentItem();
    SAFE_POINT(item != NULL, "Unexpected current item on edit!",);
    ProjViewItem* pvi = static_cast<ProjViewItem*>(item);
    SAFE_POINT(pvi->isObjectItem(), "Not an object type item on edit!",);
    ProjViewObjectItem* objItem = static_cast<ProjViewObjectItem*>(pvi);
    if (!AppContext::getProject()->isStateLocked()) {
        int maxNameLength = 50;
        static QRegExp invalidCharactersRegExp("[\\\\|/+;=*]"); // \\\\ is a simple backslash    
        QString invalidCharacters = invalidCharactersRegExp.pattern();
        invalidCharacters.remove(0, 2).chop(1); // getting rid of the brackets and duplicate backslashes

        QString newName = item->text(0).trimmed();

        if (newName == objItem->obj->getGObjectName()) {
            if (objItem->isBeingEdited) {
                objItem->isBeingEdited = false;
                objItem->updateVisual();
            }
            return;
        } else if (newName.length() == 0  || newName.length() > maxNameLength) {
            
            QMessageBox::critical(0, "Error", tr("The name must be not empty and not longer than %1 characters").arg(maxNameLength));
            return;

        } else if (objItem->obj->getDocument()->findGObjectByName(newName) != NULL) {
            QMessageBox::critical(0, "Error", tr("Duplicate object names are not allowed"));
            return;

        } else if (newName.contains(invalidCharactersRegExp)) {
            QMessageBox::critical(0, "Error", tr("The name can't contain any of the following characters: %1").arg(invalidCharacters));
            return;
        }

        coreLog.trace(QString("Renaming object %1 to %2").arg(objItem->obj->getGObjectName()).arg(newName));
        objItem->obj->setGObjectName(newName);
    }
    objItem->setFlags(objItem->flags() & ~Qt::ItemIsEditable);
    objItem->updateVisual();
}

void ProjectTreeController::updateSelection() {
    SAFE_POINT(tree, "ProjectTreeController::no project tree", );
    QList<QTreeWidgetItem*> items = tree->selectedItems();
    if (items.isEmpty()) {
        objectSelection.clear();
        documentSelection.clear();
        return;
    } 
    QList<Document*> selectedDocs;
    QList<GObject*> selectedObjs;
    foreach(QTreeWidgetItem* item , items) {
        ProjViewItem* pvi = static_cast<ProjViewItem*>(item);
        SAFE_POINT(pvi, "ProjectTreeController::cannot cast to ProjViewItem", );
        if (pvi->isDocumentItem()) {
            ProjViewDocumentItem* di = static_cast<ProjViewDocumentItem*>(pvi);
            SAFE_POINT(di, "ProjectTreeController::cannot cast to ProjViewDocumentItem", );
            selectedDocs.push_back(di->doc);
        } else if (pvi->isObjectItem()) {
            ProjViewObjectItem* oi = static_cast<ProjViewObjectItem*>(pvi);
            SAFE_POINT(oi, "ProjectTreeController::cannot cast to ProjViewObjectItem", );
            selectedObjs.push_back(oi->obj);
        }
    }
    objectSelection.setSelection(selectedObjs);
    documentSelection.setSelection(selectedDocs);
}


void ProjectTreeController::sl_onItemDoubleClicked(QTreeWidgetItem * item, int) {
    ProjViewItem* pvi = static_cast<ProjViewItem*>(item);
    if (pvi->isObjectItem()) {
        emit si_doubleClicked((static_cast<ProjViewObjectItem*>(pvi))->obj);
    } else if (pvi->isDocumentItem()) {
        Document* d = (static_cast<ProjViewDocumentItem*>(pvi))->doc;
        SAFE_POINT(d, "ProjectTreeController::sl_onItemDoubleClicked::No document for an item", );
        if (!d->isLoaded() && pvi->childCount() == 0) { 
            assert(loadSelectedDocumentsAction->isEnabled());
            loadSelectedDocumentsAction->trigger();
        }else{
            //children > 0 -> expand action
            pvi->setExpanded(true);
            emit si_doubleClicked(d);
        }
    } else {
        assert(pvi->isTypeItem());
        bool isExapanded = pvi->isExpanded();
        pvi->setExpanded(!isExapanded);
    }
}

void ProjectTreeController::sl_onContextMenuRequested(const QPoint&) {
    QMenu m;

    m.addSeparator();

    ProjectView* pv = AppContext::getProjectView();
    ProjectLoader* pl = AppContext::getProjectLoader();
    if (pv != NULL) {
        QAction* addExistingDocumentAction = new QAction(pl->getAddExistingDocumentAction()->icon(), tr("Existing document"), &m);
        connect(addExistingDocumentAction, SIGNAL(triggered()), pl->getAddExistingDocumentAction(), SLOT(trigger()));

        QMenu* addMenu = m.addMenu(tr("Add"));
        addMenu->menuAction()->setObjectName( ACTION_PROJECT__ADD_MENU);
        addMenu->addAction(addExistingDocumentAction);
        addMenu->addAction(addObjectToDocumentAction);
    }

    QMenu* editMenu = m.addMenu(tr("Edit"));
    editMenu->menuAction()->setObjectName( ACTION_PROJECT__EDIT_MENU);
    if(pv != NULL){
        editMenu->addAction(renameAction);
    }
    if (addReadonlyFlagAction->isEnabled()) {
        editMenu->addAction(addReadonlyFlagAction);
    }
    if (removeReadonlyFlagAction->isEnabled()) {
        editMenu->addAction(removeReadonlyFlagAction);
    }

    if (removeSelectedDocumentsAction->isEnabled()) {
        removeSelectedDocumentsAction->setObjectName( ACTION_PROJECT__REMOVE_SELECTED);
        m.addAction(removeSelectedDocumentsAction);
    }
    if (removeSelectedObjectsAction->isEnabled()) {
        m.addAction(removeSelectedObjectsAction);
    }
    editMenu->setEnabled(!editMenu->actions().isEmpty());

    emit si_onPopupMenuRequested(m);

    if (loadSelectedDocumentsAction->isEnabled()) {
        m.addAction(loadSelectedDocumentsAction);
    }
    if (unloadSelectedDocumentsAction->isEnabled()) {
        m.addAction(unloadSelectedDocumentsAction);
        unloadSelectedDocumentsAction->setObjectName( ACTION_PROJECT__UNLOAD_SELECTED);
    }
    m.setObjectName("popMenu");
    m.exec(QCursor::pos());
}

QSet<Document*>  ProjectTreeController::getDocsInSelection(bool deriveFromObjects) {
    QSet<Document*> docsInSelection = documentSelection.getSelectedDocuments().toSet();
    if (deriveFromObjects) {
        foreach(GObject* o, objectSelection.getSelectedObjects()) {
            Document* doc = o->getDocument();
            assert(doc!=NULL);
            docsInSelection.insert(doc);
        }
    }
    return docsInSelection;
}

void ProjectTreeController::updateActions() {
    bool hasUnloadedDocumentInSelection = false;
    bool hasLoadedDocumentInSelection = false;
    QSet<Document*> docsItemsInSelection = getDocsInSelection(false);
    QSet<Document*> docsInSelection = getDocsInSelection(true);//mode.groupMode != ProjectTreeGroupMode_ByDocument);
    foreach(Document* d, docsInSelection) {
        if (!d->isLoaded()) {
            hasUnloadedDocumentInSelection = true;
            break;
        } else {
            hasLoadedDocumentInSelection = true;
        }
    }
    
    bool canAddObjectToDocument = true;
    foreach(Document* d, docsInSelection) {
        if (!DocumentUtils::canAddGObjectsToDocument(d,GObjectTypes::SEQUENCE)) {
            canAddObjectToDocument = false;
            break;
        }
    }
    addObjectToDocumentAction->setEnabled(canAddObjectToDocument && docsInSelection.size() == 1);
    
    bool canRemoveObjectFromDocument = true;
    QList<GObject*> selectedObjects = objectSelection.getSelectedObjects();
    foreach(GObject* obj, selectedObjects ) {
        if (!DocumentUtils::canRemoveGObjectFromDocument(obj)) {
            canRemoveObjectFromDocument = false;
            break;
        }
    }
    removeSelectedObjectsAction->setEnabled(canRemoveObjectFromDocument && !objectSelection.isEmpty());
    removeSelectedDocumentsAction->setEnabled(!docsItemsInSelection.isEmpty());

    loadSelectedDocumentsAction->setEnabled(hasUnloadedDocumentInSelection);
    unloadSelectedDocumentsAction->setEnabled(hasLoadedDocumentInSelection);

    addReadonlyFlagAction->setEnabled(docsInSelection.size() == 1 && !docsInSelection.toList().first()->hasUserModLock() && !docsInSelection.toList().first()->isStateLocked());
    removeReadonlyFlagAction->setEnabled(docsInSelection.size() == 1 && docsInSelection.toList().first()->isLoaded() && docsInSelection.toList().first()->hasUserModLock());

    if (!docsItemsInSelection.isEmpty()) {
        loadSelectedDocumentsAction->setText(tr("Load selected documents"));
        unloadSelectedDocumentsAction->setText(tr("Unload selected documents"));
    } else {
        loadSelectedDocumentsAction->setText(tr("Load selected objects"));
        unloadSelectedDocumentsAction->setText(tr("Unload selected objects"));
    }

    QList<QTreeWidgetItem*> selItems = tree->selectedItems();
    bool renameIsOk = false;
    if (selItems.size() == 1) {
        ProjViewItem *item = static_cast<ProjViewItem *>(selItems.last());
        if (item != NULL && item->isObjectItem() && !AppContext::getProject()->isStateLocked()) {
            renameIsOk = true;
        }
    } 
    renameAction->setEnabled(renameIsOk);
}

void ProjectTreeController::sl_onRemoveSelectedDocuments() {
    Project* p = AppContext::getProject();
    QSet<Document*> docsInSelection = getDocsInSelection(mode.groupMode != ProjectTreeGroupMode_ByDocument);
    if (!docsInSelection.isEmpty()) {
        AppContext::getTaskScheduler()->registerTopLevelTask(new RemoveMultipleDocumentsTask(p, docsInSelection.toList(), true, true));
    }
}

void ProjectTreeController::sl_onLoadSelectedDocuments() {
    QSet<Document*> docsInSelection = getDocsInSelection(true);//mode.groupMode != ProjectTreeGroupMode_ByDocument);
    QList<Document*> docsToLoad;
    foreach(Document* d, docsInSelection) {
        if (!d->isLoaded() && LoadUnloadedDocumentTask::findActiveLoadingTask(d)==NULL) {
            docsToLoad.append(d);
        }
    }
    runLoadDocumentTasks(docsToLoad);
}

void ProjectTreeController::sl_onUnloadSelectedDocuments() {
    QList<Document*> docsToUnload;
    QSet<Document*> docsInSelection = getDocsInSelection(true);//mode.groupMode != ProjectTreeGroupMode_ByDocument);
    foreach(Document* doc, docsInSelection) {
        if (doc->isLoaded()) {
            docsToUnload.append(doc);
        }
    }
    UnloadDocumentTask::runUnloadTaskHelper(docsToUnload, UnloadDocumentTask_SaveMode_Ask);
}


void ProjectTreeController::runLoadDocumentTasks(const QList<Document*>& docs) {
    QList<Task*> tasks;
    if (mode.loadTaskProvider!=NULL) {
        tasks = mode.loadTaskProvider->createLoadDocumentTasks(docs);
    } else {
        foreach(Document* d, docs) {
            tasks << new LoadUnloadedDocumentTask(d);
        }
    }
    foreach(Task* t, tasks) {
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    }

}

void ProjectTreeController::sl_onDocumentAddedToProject(Document* d) {
    //todo: sort?
    buildDocumentTree(d);
    connectDocument(d);

    updateActions();
}

void ProjectTreeController::sl_onDocumentRemovedFromProject(Document* d) {
    disconnectDocument(d);
    disconnect(tree, SIGNAL(itemSelectionChanged()), this, SLOT(sl_onTreeSelectionChanged()));

    if (mode.groupMode == ProjectTreeGroupMode_ByDocument) {
        ProjViewDocumentItem* di = findDocumentItem(d);
        delete di;
    } else {
        foreach (GObject* obj, d->getObjects()) {
             ProjViewObjectItem* oi = findGObjectItem(d,obj);
             if (mode.groupMode == ProjectTreeGroupMode_ByType) {
                ProjViewTypeItem* topItem = findTypeItem(getLoadedObjectType(obj), true);
                topItem->removeChild(oi);
                if (topItem->childCount() == 0) {
                    delete topItem;
                } else {
                    topItem->updateVisual();
                }
             } 
             delete oi;
        }
    }
  
    updateSelection();
    updateActions();
    connect(tree, SIGNAL(itemSelectionChanged()), SLOT(sl_onTreeSelectionChanged()));
}

void ProjectTreeController::sl_onToggleReadonly() {
    QSet<Document*> docsInSelection = getDocsInSelection(true);
    if (docsInSelection.size()!=1) {
        return;
    }
    Document* doc = docsInSelection.toList().first();
    if (!DocumentUtils::getPermissions(doc).testFlag(QFile::WriteUser)){
        QMessageBox::warning(QApplication::activeWindow(), tr("Warning"),
                             tr("This action requires changing file:\n%1\nYou don't have enough rights to change file").arg(doc->getURLString()),
                            QMessageBox::Ok);
        return;
    }
    if (doc->hasUserModLock()) {
        doc->setUserModLock(false);
    } else {
        doc->setUserModLock(true);
    }
}

void ProjectTreeController::sl_onLockedStateChanged() {
    QObject * who = sender();
    Document* doc = qobject_cast<Document*>(who);
    assert(doc!=NULL);

    ProjViewDocumentItem* di = findDocumentItem(doc);
    
    if (mode.readOnlyFilter == TriState_Unknown) {
        if (di!=NULL) {
            di->updateVisual(false);
        }
    } else {
        //remove/add document and its objects 
        bool remove = (doc->isStateLocked() && mode.readOnlyFilter == TriState_Yes) || (!doc->isStateLocked() && mode.readOnlyFilter == TriState_No);
        if (remove) {
            if (di!=NULL) {
                delete di;
            } else {
                foreach(GObject* obj, doc->getObjects()) {
                    ProjViewObjectItem* oi = findGObjectItem(di, obj);
                    if (oi!=NULL) {
                        delete oi;
                        oi = NULL;
                    }
                }
            }
            disconnectDocument(doc);
        } else { //add state locked document and its objects
            //ensure document
            assert(di == NULL);
            buildDocumentTree(doc);
            connectDocument(doc);
        }
        updateSelection();
    }
    updateActions();
}


void ProjectTreeController::sl_onRename() {
    QList<QTreeWidgetItem*> selItems = tree->selectedItems();
    if (selItems.size() != 1) {
        return;
    }
    
    ProjViewItem *item = static_cast<ProjViewItem *>(selItems.last());
    if (item != NULL && item->isObjectItem() && !AppContext::getProject()->isStateLocked()) {
        ProjViewObjectItem *objItem = static_cast<ProjViewObjectItem*>(item);
        
        objItem->setFlags(objItem->flags() | Qt::ItemIsEditable);
        objItem->setText(0, objItem->obj->getGObjectName());
        tree->editItem(objItem);
        objItem->isBeingEdited = true;
    }
}

void ProjectTreeController::flattenDocumentItem(ProjViewDocumentItem* docItem) {
    assert(mode.groupMode == ProjectTreeGroupMode_Flat);

    while (docItem->childCount()!=0) {
        ProjViewItem* item = static_cast<ProjViewItem*>(docItem->takeChild(0));
        assert(item->isObjectItem());
        ProjViewObjectItem* objItem = static_cast<ProjViewObjectItem*>(item);
        if (mode.isObjectShown(objItem->obj)) {
            tree->addTopLevelItem(objItem);
            objItem->updateVisual();
        } else {
            delete objItem;
        }
    }
}

void ProjectTreeController::sl_onDocumentLoadedStateChanged() {
    Document* d = qobject_cast<Document*>(sender());
    ProjViewDocumentItem* docItem = findDocumentItem(d);
    if (!mode.isDocumentShown(d)) { 
        // document item can be automatically removed from the view after its loaded due to filters
        if (docItem != NULL) {
            if (mode.groupMode == ProjectTreeGroupMode_Flat) {
                flattenDocumentItem(docItem);
            }
            delete docItem;
            docItem = NULL;
        }
    }
    if (docItem != NULL && d->getObjects().size() < MAX_OBJECTS_TO_AUTOEXPAND && AppContext::getProject()->getDocuments().size() < MAX_DOCUMENTS_TO_AUTOEXPAND) {
        docItem->setExpanded(d->isLoaded());
    }
    updateActions();
}


void ProjectTreeController::sl_onDocumentModifiedStateChanged() {
    Document* d = qobject_cast<Document*>(sender());
    assert(d!=NULL);
    if (!mode.isDocumentShown(d)) {
        return;
    }
    updateActions();
    ProjViewDocumentItem* di = findDocumentItem(d);
    if (di!=NULL) {
        di->updateVisual();
    }
}

void ProjectTreeController::updateObjectVisual(GObject* obj) {
    SAFE_POINT(obj != NULL, "Object is NULL in updateObjectVisual()", );
    if (!mode.isObjectShown(obj)) {
        return;
    }
    ProjViewObjectItem* item = findGObjectItem(obj->getDocument(), obj);
    SAFE_POINT(item != NULL, QString("item not found for object %1").arg(obj->getGObjectName()), );
    item->updateVisual();
}

void ProjectTreeController::sl_onObjectModifiedStateChanged() {
    updateActions();
    updateObjectVisual(qobject_cast<GObject*>(sender()));
}

void ProjectTreeController::sl_onObjectNameChanged(const QString&) {
    updateActions();
    updateObjectVisual(qobject_cast<GObject*>(sender()));
}

void ProjectTreeController::sl_onObjectAdded(GObject* obj) {
    if (!mode.isObjectShown(obj)) {
        return;
    }
    Document* doc = obj->getDocument();
    assert(doc!=NULL);
    ProjViewItem* topItem = NULL;
    if (mode.groupMode == ProjectTreeGroupMode_ByType) {
        topItem = findTypeItem(getLoadedObjectType(obj), true);
    } else if (mode.isDocumentShown(doc)) {
        topItem = findDocumentItem(doc);
        if (topItem == NULL) { //doc is not really shown yet -> add it using general routine
            buildDocumentTree(doc);
            connectDocument(doc);
            return;
        }
    }
    connectGObject(obj);

    ProjViewObjectItem* objItem = new ProjViewObjectItem(obj, this);
    
    if (mode.groupMode != ProjectTreeGroupMode_ByDocument || topItem == NULL) {
        insertTreeItemSorted(topItem, objItem);
    } else { //keep natural mode, as in file
        topItem->addChild(objItem);
    }
    if (topItem != NULL && topItem->childCount() == 1) {
        topItem->updateVisual();
    }
    updateActions();
}

void ProjectTreeController::sl_onObjectRemoved(GObject* obj) {
    Document* doc = qobject_cast<Document*>(sender());
    assert(doc!=NULL);
    ProjViewObjectItem* objItem = findGObjectItem(doc, obj);
    if (objItem == NULL) {
        assert(!objectSelection.contains(obj));
        return;
    }
    ProjViewItem* pi = static_cast<ProjViewItem*>(objItem->parent());
    delete objItem;
    if (pi!=NULL && pi->isTypeItem()) {
        pi->updateVisual();
    }
    objectSelection.removeFromSelection(obj);
    updateActions();
}

void ProjectTreeController::sl_onResourceUserRegistered(const QString& res, Task* t) {
    Q_UNUSED(res);
    LoadUnloadedDocumentTask* lut = qobject_cast<LoadUnloadedDocumentTask*>(t);
    if (lut == NULL) {
        return;
    }
    connect(lut, SIGNAL(si_progressChanged()), SLOT(sl_onLoadingDocumentProgressChanged()));
}


#define MAX_OBJS_TO_SHOW_LOAD_PROGRESS 100
void ProjectTreeController::updateLoadingState(Document* doc) {
    if (mode.isDocumentShown(doc)) {
        ProjViewDocumentItem* di = findDocumentItem(doc);
        if (di) {
            di->updateVisual();
        }
    }
    if (doc->getObjects().size() < MAX_OBJS_TO_SHOW_LOAD_PROGRESS) {
        foreach(GObject* obj, doc->getObjects()) {
            ProjViewObjectItem* oi;
            if (mode.isObjectShown(obj) && (oi = findGObjectItem(doc, obj))) {
                oi->updateVisual();
            }
        }
    }
}

void ProjectTreeController::sl_onResourceUserUnregistered(const QString& res, Task* t) {
    Q_UNUSED(res);
    LoadUnloadedDocumentTask* lut = qobject_cast<LoadUnloadedDocumentTask*>(t);
    if (lut == NULL) {
        return;
    }
    lut->disconnect(this);

    Document* doc = lut->getDocument();
    if (doc!=NULL) {
        updateLoadingState(doc);
    }
}

void ProjectTreeController::sl_onLoadingDocumentProgressChanged() {
    LoadUnloadedDocumentTask* lut = qobject_cast<LoadUnloadedDocumentTask*>(sender());
    assert(lut!=NULL);
    Document* doc = lut->getDocument();
    updateLoadingState(doc);
}

void ProjectTreeController::updateSettings(const ProjectTreeControllerModeSettings& _mode) {
    // try to keep GObject selection while reseting view
    QList<GObject*> objects = getGObjectSelection()->getSelectedObjects();
    bool groupModeChanged = _mode.groupMode != mode.groupMode;
    //bool filtersChanged = _mode.objectFilter!= mode.objectFilter || _mode.documentFilter != mode.documentFilter;
    mode = _mode;
    if (groupModeChanged) {
        tree->clear();
    } else {
        filterItemsRecursive(NULL);
    }
    buildTree();

    updateActions();

    bool madeVisible = false;
    foreach(GObject* obj, objects) {
        ProjViewObjectItem* oi = findGObjectItem(obj->getDocument(), obj);
        if (oi!=NULL) {
            oi->setSelected(true);
            if (!madeVisible) {
                madeVisible = true;
                tree->scrollToItem(oi);
            }
        }
    }
}

void ProjectTreeController::filterItemsRecursive(ProjViewItem* pi) {
    TreeUpdateHelper h(itemsToUpdate);
    if (pi == NULL) {
        for(int i=0; i < tree->topLevelItemCount(); i++) {
            ProjViewItem* ci = static_cast<ProjViewItem*>(tree->topLevelItem(i));
            if (ci->isDocumentItem()) {
                ProjViewDocumentItem*  di = static_cast<ProjViewDocumentItem*>(ci);
                if (!mode.isDocumentShown(di->doc)) {
                    delete di;
                    i--;
                } else {
                    filterItemsRecursive(di);
                }
            } else if (ci->isTypeItem()) {
                filterItemsRecursive(ci);
                if (ci->childCount() == 0) {
                    itemsToUpdate.remove(ci);
                    delete ci;
                    i--;
                }
            } else {            
                assert(ci->isObjectItem());
                ProjViewObjectItem* oi = static_cast<ProjViewObjectItem*>(ci);
                bool filterObject = !mode.isObjectShown(oi->obj) 
                    || (mode.isDocumentShown(oi->obj->getDocument()) && oi->parent() == NULL);
                if (filterObject) {
                    delete oi;
                    i--;
                }
            }
        }
    } else {
        for(int i=0; i<pi->childCount(); i++) {
            ProjViewItem* ci = static_cast<ProjViewItem*>(pi->child(i));
            assert(ci->isObjectItem());
            ProjViewObjectItem* oi = static_cast<ProjViewObjectItem*>(ci);
            if (!mode.isObjectShown(oi->obj)) {
                delete oi;
                i--;
                itemsToUpdate.insert(pi);
            }
        }
    }
}

void ProjectTreeController::sl_onGroupByDocument() {
    if (mode.groupMode == ProjectTreeGroupMode_ByDocument) {
        return;
    }
    ProjectTreeControllerModeSettings newMode = mode;
    newMode.groupMode= ProjectTreeGroupMode_ByDocument;
    updateSettings(newMode);
}

void ProjectTreeController::sl_onGroupByType() {
    if (mode.groupMode == ProjectTreeGroupMode_ByType) {
        return;
    }
    ProjectTreeControllerModeSettings newMode = mode;
    newMode.groupMode= ProjectTreeGroupMode_ByType;
    updateSettings(newMode);
}

void ProjectTreeController::sl_onGroupFlat() {
    if (mode.groupMode == ProjectTreeGroupMode_Flat) {
        return;
    }
    ProjectTreeControllerModeSettings newMode = mode;
    newMode.groupMode= ProjectTreeGroupMode_Flat;
    updateSettings(newMode);
}

void ProjectTreeController::insertTreeItemSorted(ProjViewItem* p, ProjViewItem* item) {
    if (p == NULL) { //top level
        for (int i=0, n = tree->topLevelItemCount(); i < n; i++) {
            ProjViewItem* otherItem = static_cast<ProjViewItem*>(tree->topLevelItem(i));
            if (*item < *otherItem) {
                tree->insertTopLevelItem(i, item);
                return;
            }
        }
        tree->addTopLevelItem(item);
    } else {
        for (int i = 0, n = p->childCount(); i < n; i++) {
            ProjViewItem* otherItem = static_cast<ProjViewItem*>(p->child(i));
            if (*item < *otherItem) {
                p->insertChild(i, item);
                return;
            }
        }
        p->addChild(item);
    }
}


void ProjectTreeController::sl_onDocumentURLorNameChanged() {
    Document* doc = qobject_cast<Document*>(sender());
    ProjViewDocumentItem* di = findDocumentItem(doc);
    if (!mode.isDocumentShown(doc)) {
        delete di;
        return;
    }
    if (di != NULL) {
        di->updateVisual();
        return;
    }
    di = new ProjViewDocumentItem(doc, this);
    tree->addTopLevelItem(di);
}

void ProjectTreeController::highlightItem(Document *doc){
    assert(doc);
    ProjViewDocumentItem *item = findDocumentItem(doc);
    if (item) { //item can be NULL here if custom filter is used
        item->setSelected(true);
    }
}

void ProjectTreeController::sl_windowActivated(MWMDIWindow* w) {
    if (!mode.markActive) {
        return;
    }
    if (mode.groupMode ==  ProjectTreeGroupMode_ByType) {
        // only second level objects has 'active' info
        for (int i=0; i < tree->topLevelItemCount(); i++) {
            ProjViewItem* item1 = static_cast<ProjViewItem*>(tree->topLevelItem(i));
            for (int j=0; j < item1->childCount(); j++) {
                ProjViewItem* item2 = static_cast<ProjViewItem*>(item1->child(j));
                item2->updateActive();
            }
        }

    } else { // first level objects has 'active' info
        for (int i=0; i < tree->topLevelItemCount(); i++) {
            ProjViewItem* item = static_cast<ProjViewItem*>(tree->topLevelItem(i));
            item->updateActive();
        }
    }

    // listen all add/remove to view events
    if (markActiveView != NULL) {
        markActiveView->disconnect(this);
        markActiveView = NULL;
    }
    GObjectViewWindow* ow = qobject_cast<GObjectViewWindow*>(w); 
    if (ow != NULL) {
        uiLog.trace(QString("Project view now listens object events in '%1' view").arg(ow->windowTitle()));
        markActiveView = ow->getObjectView();
        connect(markActiveView, SIGNAL(si_objectAdded(GObjectView*, GObject*)), SLOT(sl_objectAddedToActiveView(GObjectView*, GObject*)));
        connect(markActiveView, SIGNAL(si_objectRemoved(GObjectView*, GObject*)), SLOT(sl_objectRemovedFromActiveView(GObjectView*, GObject*)));
    }
}

void ProjectTreeController::updateObjectActiveStateVisual(GObject* o) {
    SAFE_POINT(o!= NULL, "ProjectTreeController::updateObjectActiveStateVisual. Object is NULL", );
    if (mode.groupMode == ProjectTreeGroupMode_ByDocument) {
        ProjViewDocumentItem* di = findDocumentItem(o->getDocument());
        if (di!=NULL) {
            di->updateActive();
        }
    } else {
         ProjViewObjectItem* oi = findGObjectItem(o);
         if (oi!=NULL) {
            oi->updateActive();
         }
    }
}

void ProjectTreeController::sl_objectAddedToActiveView(GObjectView*, GObject* o) {
    SAFE_POINT(o != NULL, tr("No object to add to view"), );
    uiLog.trace(QString("Processing object add to active view in project tree: %1").arg(o->getGObjectName()));
    updateObjectActiveStateVisual(o);
}

void ProjectTreeController::sl_objectRemovedFromActiveView(GObjectView*, GObject* o) {
    SAFE_POINT(o != NULL, tr("No object to remove from view"), );
    uiLog.trace(QString("Processing object remove form active view in project tree: %1").arg(o->getGObjectName()));
    updateObjectActiveStateVisual(o);
}

void ProjectTreeController::sl_onAddObjectToSelectedDocument() {
    QSet<Document*> selectedDocuments = getDocsInSelection(true);
    assert(selectedDocuments.size() == 1);
    Document* doc = selectedDocuments.values().first();
    
    ProjectTreeControllerModeSettings settings;
    
    // do not show objects from the selected document
    QList<GObject*> docObjects = doc->getObjects();
    foreach (GObject* obj, docObjects) {
        settings.excludeObjectList.append(obj);
    }
    
    QSet<GObjectType> types = doc->getDocumentFormat()->getSupportedObjectTypes();
    foreach(const GObjectType& type, types) {
        settings.objectTypesToShow.append(type);
    }
    
    QList<GObject*> objects = ProjectTreeItemSelectorDialog::selectObjects(settings, tree);
    U2OpStatus2Log os;
    if (!objects.isEmpty()) {
        foreach(GObject* obj, objects) {
            if (obj->isUnloaded()) {
                continue;
            }
            doc->addObject(obj->clone(doc->getDbiRef(), os));
            CHECK_OP(os, );
        }
    }
}

void ProjectTreeController::sl_onRemoveSelectedObjects() {
    QList<GObject*> objs = getGObjectSelection()->getSelectedObjects();

    if (objs.isEmpty()) {
        return;
    }
    objectSelection.clear();

    foreach (GObject* obj, objs) {
        Document* doc = obj->getDocument();
        assert(doc != NULL);
        doc->removeObject(obj);
    }
}

void ProjectTreeController::sl_updateAfterItemchange( QTreeWidgetItem * item, int column ){
    Q_UNUSED(item);
    Q_UNUSED(column);
    updateActions();
}


//////////////////////////////////////////////////////////////////////////
/// Tree Items

void ProjViewItem::updateActive() {
    if (!markedAsActive && !isActive()) {
        return;
    }
    uiLog.trace(QString("Updating active status for %1").arg(text(0)));
    for (int i = 0, n = childCount(); i < n ; i++ ) {
        ProjViewItem* item = static_cast<ProjViewItem*>(child(i));
        item->updateActive();
    }

    updateVisual(false);
}

ProjViewDocumentItem::ProjViewDocumentItem(Document* _doc, ProjectTreeController* c) 
: ProjViewItem(c), doc(_doc) 
{
    updateVisual();
}

bool ProjViewDocumentItem::operator< ( const QTreeWidgetItem & other ) const {
    const ProjViewItem& pi = (const ProjViewItem&)other;
    if (pi.isDocumentItem()) {
        const ProjViewDocumentItem& di = (const ProjViewDocumentItem&)other;
        return doc->getName() < di.doc->getName();
    } else {
        assert(pi.isObjectItem());
        const ProjViewObjectItem& oi = (const ProjViewObjectItem&)other;
        return doc->getName() < oi.obj->getGObjectName();
    }
}

#define MODIFIED_ITEM_COLOR "#0032a0"
void ProjViewDocumentItem::updateVisual(bool recursive) {
    if (recursive) {
        for (int i = 0, n = childCount(); i < n ; i++ ) {
            ProjViewObjectItem* oi = static_cast<ProjViewObjectItem*>(child(i));
            oi->updateVisual(recursive);
        }
    }

    //update text
    QString text;
    if (doc->isModified()) {
        setData(0, Qt::TextColorRole, QColor(MODIFIED_ITEM_COLOR));
    } else {
        setData(0, Qt::TextColorRole, QVariant());
    }
    
    markedAsActive = controller->getModeSettings().markActive && isActive();
    if (markedAsActive) {
        setData(0, Qt::FontRole, controller->getModeSettings().activeFont);
        
    } else {
        setData(0, Qt::FontRole, QVariant());
    }

    if (!doc->isLoaded()) {
        LoadUnloadedDocumentTask* t = LoadUnloadedDocumentTask::findActiveLoadingTask(doc);
        if (t == NULL) {
            text+="[unloaded]";
        } else {
            text+=ProjectTreeController::tr("[loading %1%]").arg(t->getProgress());
        }
    }
    text+=doc->getName();
    setData(0, Qt::DisplayRole, text);
    
    //update icon
    bool showLockedIcon = doc->isStateLocked();
    if (!doc->isLoaded() && doc->getStateLocks().size() == 1 && doc->getDocumentModLock(DocumentModLock_UNLOADED_STATE)!=NULL) {
        showLockedIcon = false;
    }
    setIcon(0, showLockedIcon ? controller->roDocumentIcon : controller->documentIcon);

    //update tooltip
    QString tooltip = doc->getURLString();
    if  (doc->isStateLocked()) {
        tooltip.append("<br><br>").append(ProjectTreeController::tr("Locks:"));
        StateLockableItem* docContext = doc->getParentStateLockItem();
        if (docContext != NULL && docContext->isStateLocked()) {
            tooltip.append("<br>&nbsp;*&nbsp;").append(ProjectTreeController::tr("Project is locked"));
        } 
        foreach(StateLock* lock, doc->getStateLocks()) {
            if (!doc->isLoaded() && lock == doc->getDocumentModLock(DocumentModLock_FORMAT_AS_INSTANCE)) {
                continue; //do not visualize some locks for unloaded document
            }
            tooltip.append("<br>&nbsp;*&nbsp;").append(lock->getUserDesc());
        }
    }
    setData(0, Qt::ToolTipRole, tooltip);
}

bool ProjViewDocumentItem::isActive() const {
    GObjectViewWindow* w = GObjectViewUtils::getActiveObjectViewWindow();
    if (w == NULL) {
        return false;
    }
    return w->getObjectView()->containsDocumentObjects(doc);
}


ProjViewObjectItem::ProjViewObjectItem(GObject* _obj, ProjectTreeController* c) 
: ProjViewItem(c), obj(_obj), isBeingEdited(false)
{
    updateVisual();
}

bool ProjViewObjectItem::operator< ( const QTreeWidgetItem & other ) const {
    const ProjViewItem& pi = (const ProjViewItem&)other;
    if (pi.isObjectItem()) {
        const ProjViewObjectItem& oi = (const ProjViewObjectItem&)other;
        return obj->getGObjectName() < oi.obj->getGObjectName();
    } else {
        assert(pi.isDocumentItem());
        const ProjViewDocumentItem& di = (const ProjViewDocumentItem&)other;
        return obj->getGObjectName() < di.doc->getName();
    }
}

void ProjViewObjectItem::updateVisual(bool ) {
    if (isBeingEdited) {
        isBeingEdited = false;
        return;
    }
    QString text;
    GObjectType t = obj->getGObjectType();
    bool unloaded = t == GObjectTypes::UNLOADED;
    bool allowSelectUnloaded = controller->getModeSettings().allowSelectUnloaded;
    if (unloaded) {
        if(!allowSelectUnloaded){
            setDisabled(true);
        }
        t = qobject_cast<UnloadedObject*>(obj)->getLoadedObjectType();        
    }
    const GObjectTypeInfo& ti = GObjectTypes::getTypeInfo(t);
    text+="[" + ti.treeSign + "] ";
    
    if (unloaded && obj->getDocument()->getObjects().size() < MAX_OBJS_TO_SHOW_LOAD_PROGRESS) {
        LoadUnloadedDocumentTask* t = LoadUnloadedDocumentTask::findActiveLoadingTask(obj->getDocument());
        if (t != NULL) {
            text+=ProjectTreeController::tr("[loading %1%]").arg(t->getProgress());
        }
    }

    if (obj->isItemModified()) {
        setData(0, Qt::TextColorRole, QColor(MODIFIED_ITEM_COLOR));
    } else {
        setData(0, Qt::TextColorRole, QVariant());
    }

    markedAsActive = controller->getModeSettings().markActive && isActive();
    if (markedAsActive) {
        setData(0, Qt::FontRole, controller->getModeSettings().activeFont);
    } else {
        setData(0, Qt::FontRole, QVariant());
    }


    text+=obj->getGObjectName();

    ProjectTreeGroupMode groupMode = controller->getModeSettings().groupMode;
    if (groupMode == ProjectTreeGroupMode_ByType || groupMode == ProjectTreeGroupMode_Flat) {
        text+=" ["+obj->getDocument()->getName()+"]";
    }


    setData(0, Qt::DisplayRole, text);
    setIcon(0, ti.icon);


    QString tooltip;
    //todo: make tooltip for object items
   
    if (groupMode == ProjectTreeGroupMode_Flat) {
        tooltip.append(obj->getDocument()->getURLString());
    }
    setToolTip(0, tooltip);
}

bool ProjViewObjectItem::isActive() const {
    GObjectViewWindow* w = GObjectViewUtils::getActiveObjectViewWindow();
    if (w == NULL) {
        return false;
    }
    return w->getObjectView()->containsObject(obj);
}

ProjViewTypeItem::ProjViewTypeItem(const GObjectType& t, ProjectTreeController* c) 
: ProjViewItem(c), otype(t)
{
    typePName = GObjectTypes::getTypeInfo(t).pluralName;
    updateVisual(false);
}

bool ProjViewTypeItem::operator< ( const QTreeWidgetItem & other ) const {
    assert( ((const ProjViewItem&)other).isTypeItem() );
    const ProjViewTypeItem& ti = (const ProjViewTypeItem&)other; 
    return otype < ti.otype;
}

void ProjViewTypeItem::updateVisual(bool recursive) {
    static QIcon groupIcon(":/core/images/group_green_active.png");

    assert(!recursive); Q_UNUSED(recursive); //TODO: remove this param
    setText(0, typePName + " (" +QString::number(childCount())+")");
    setIcon(0, groupIcon);
}

ProjItemDelegate::ProjItemDelegate(QObject *parent) : QItemDelegate(parent) {

}

QWidget * ProjItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    QLineEdit *editor = new QLineEdit(parent);
    return editor;
}

void ProjItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    const QString value = index.model()->data(index).toString();
    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    lineEdit->setText(value);
}

void ProjItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
    const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    const QString value = lineEdit->text();
    model->setData(index, value, Qt::DisplayRole);
}

const float TEXT_EDITOR_HEIGHT_ADDITION = 1.1f;
const float TEXT_EDITOR_WIDTH_ADDITION = 2.0f;

void ProjItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    QRect textRect = option.rect;
    textRect.setHeight(option.rect.height() * TEXT_EDITOR_HEIGHT_ADDITION);
    const QFontMetrics fontMetrics(editor->font());
    textRect.setWidth(qMin(static_cast<int>(fontMetrics.width(index.data().toString())
        * TEXT_EDITOR_WIDTH_ADDITION), option.rect.width()));
    editor->setGeometry(textRect);
}

//////////////////////////////////////////////////////////////////////////
// settings

bool ProjectTreeControllerModeSettings::isDocumentShown(Document* doc) const {
    if (groupMode != ProjectTreeGroupMode_ByDocument && groupMode != ProjectTreeGroupMode_Flat) {
        return false;
    }
    if (groupMode == ProjectTreeGroupMode_Flat && (doc->isLoaded() || !doc->getObjects().isEmpty())) {
        return false; // only unloaded docs without cached object info are shown in flat mode
    }
    
    //filter by readonly state
    //TODO: revise readonly filters;
    //if the only lock is unloaded state lock -> not show it
    bool isReadonly = ! (doc->getStateLocks().size() == 1 && doc->getDocumentModLock(DocumentModLock_UNLOADED_STATE)!=NULL);
    bool res = readOnlyFilter == TriState_Unknown ? true : 
        (readOnlyFilter == TriState_Yes && !isReadonly) || (readOnlyFilter == TriState_No && isReadonly);
    if (!res) {
        return false;
    }

    //filter by object types
    if (!objectTypesToShow.isEmpty()) { 
        const QList<GObject*>& docObjs = doc->getObjects();
        if (!docObjs.isEmpty()) { //ok we have mapping about document objects -> apply filter to the objects
            bool found = false;
            foreach(GObject* o, docObjs) {
                found = isObjectShown(o);
                if (found) {
                    break;
                }
            }
            if (!found) {
                return false;
            }
        } else {
            if (!doc->isLoaded()) {
                DocumentFormatConstraints c;
                c.supportedObjectTypes += objectTypesToShow.toSet();
                res = doc->getDocumentFormat()->checkConstraints(c);
            } else {
                res = false;
            }
            if (!res) {
                return false;
            }
        }
    }

    //filter by name
    foreach(const QString& token, tokensToShow) {
        if (!doc->getURLString().contains(token)) {
            return false;
        }
    }

    // check custom filter
    if (documentFilter!= NULL && documentFilter->filter(doc)) {
        return false;
    }

    //TODO: make document visible in GroupByDoc mode if any of its objects is visible and avoid flattening?

    return true;

}

bool ProjectTreeControllerModeSettings::isObjectShown(GObject* o) const  {
    //filter by type
    GObjectType t = o->isUnloaded() ? qobject_cast<UnloadedObject*>(o)->getLoadedObjectType() : o->getGObjectType();
    bool res = isTypeShown(t);
    if (!res) {
        return false;
    }
    //filter by readonly flag
    Document* doc = o->getDocument();
    //TODO: revise readonly filters -> use isStateLocked or hasReadonlyLock ?
    res = readOnlyFilter == TriState_Unknown ? true : 
         (readOnlyFilter == TriState_Yes && !doc->isStateLocked()) || (readOnlyFilter == TriState_No && doc->isStateLocked());
    if (!res) {
        return false;
    }

    //filter by exclude list
    foreach(const QPointer<GObject>& p, excludeObjectList) {
        if (p.isNull()) {
            continue;
        }
        if (o == p.data()) {
            return false;
        }
    }

    //filter by internal obj properties
    if (!objectConstraints.isEmpty()) {
        res = true;
        foreach(const GObjectConstraints* c, objectConstraints) {
            if (o->getGObjectType() != c->objectType) {
                continue;
            }
            res = o->checkConstraints(c);
            if (!res) {
                return false;
            }
        }
    }
    
    //filter by name
    foreach(const QString& token, tokensToShow) {
        if (!o->getGObjectName().contains(token)) {
            return false;
        }
    }

    // check custom filter
    if (objectFilter != NULL && objectFilter->filter(o)) {
        return false;
    }

    return true;
}

bool ProjectTreeControllerModeSettings::isTypeShown(GObjectType t) const {
    if (objectTypesToShow.isEmpty()) {
        return true;
    }
    return objectTypesToShow.contains(t);
}



}//namespace
