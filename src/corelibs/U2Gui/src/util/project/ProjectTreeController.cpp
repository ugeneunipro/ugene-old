/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <QtCore/QTimer>

#include <QtGui/QKeyEvent>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QTreeView>
#else
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QTreeView>
#endif

#include <U2Core/AppContext.h>
#include <U2Core/DeleteObjectsTask.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/RemoveDocumentTask.h>
#include <U2Core/ResourceTracker.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/ImportToDatabaseDialog.h>
#include <U2Gui/LoadDocumentTaskProvider.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>
#include <U2Gui/UnloadDocumentTask.h>

#include "FolderNameDialog.h"
#include "ProjectUpdater.h"
#include "ProjectUtils.h"

#include "ProjectTreeController.h"

namespace U2 {

ProjectTreeController::ProjectTreeController(QTreeView *tree, const ProjectTreeControllerModeSettings &settings, QObject *parent)
    : QObject(parent), tree(tree), settings(settings), updater(NULL), model(NULL), markActiveView(NULL), objectIsBeingRecycled(NULL)
{
    Project *project = AppContext::getProject();
    SAFE_POINT(NULL != project, "NULL project", );

    model = new ProjectViewModel(settings, this);

    updater = new ProjectUpdater();

    QTimer *timer = new QTimer(this);
    timer->setInterval(U2ObjectDbi::OBJECT_ACCESS_UPDATE_INTERVAL);
    connect(timer, SIGNAL(timeout()), SLOT(sl_mergeData()));

    connect(project, SIGNAL(si_documentAdded(Document*)), SLOT(sl_onDocumentAdded(Document*)));
    connect(project, SIGNAL(si_documentRemoved(Document*)), SLOT(sl_onDocumentRemoved(Document*)));

    tree->setModel(model);
    updater->start();
    timer->start();

    tree->setSelectionMode(settings.allowMultipleSelection ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
    tree->setEditTriggers(tree->editTriggers() & ~QAbstractItemView::DoubleClicked);
    connect(tree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), SLOT(sl_updateSelection()));
    connect(tree, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(sl_doubleClicked(const QModelIndex &)));
    connect(tree, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sl_onContextMenuRequested(const QPoint &)));
    tree->installEventFilter(this);

    connect(model, SIGNAL(si_modelChanged()), SLOT(sl_updateActions()));
    connect(model, SIGNAL(si_documentContentChanged(Document*)), SLOT(sl_documentContentChanged(Document*)));
    connect(model, SIGNAL(si_projectItemRenamed(const QModelIndex &)), SLOT(sl_onProjectItemRenamed(const QModelIndex &)));

    setupActions();

    foreach (Document *doc, project->getDocuments()) {
        sl_onDocumentAdded(doc);
    }

    MWMDIManager* mdi = AppContext::getMainWindow()->getMDIManager();
    connect(mdi, SIGNAL(si_windowActivated(MWMDIWindow*)), SLOT(sl_windowActivated(MWMDIWindow*)));
    connect(mdi, SIGNAL(si_windowDeactivated(MWMDIWindow*)), SLOT(sl_windowDeactivated(MWMDIWindow*)));
    connect(mdi, SIGNAL(si_windowClosing(MWMDIWindow*)), SLOT(sl_windowDeactivated(MWMDIWindow*)));
    sl_windowActivated(mdi->getActiveWindow()); // if any window is active - check it content

    sl_updateSelection();
}

ProjectTreeController::~ProjectTreeController() {
    if (NULL != updater) {
        updater->stop();
        // TODO
        //delete updater;
    }
}

const DocumentSelection * ProjectTreeController::getDocumentSelection() const {
    return &documentSelection;
}

const GObjectSelection * ProjectTreeController::getGObjectSelection() const {
    return &objectSelection;
}

bool ProjectTreeController::isObjectInRecycleBin(GObject *obj) const {
    Document *doc = obj->getDocument();
    CHECK(NULL != doc && ProjectUtils::isConnectedDatabaseDoc(doc), false);

    const QString objectPath = model->getObjectFolder(doc, obj);
    return ProjectUtils::isFolderInRecycleBin(objectPath);
}

const ProjectTreeControllerModeSettings & ProjectTreeController::getModeSettings() const {
    return settings;
}

void ProjectTreeController::highlightItem(Document *doc) {
    QModelIndex idx = model->getIndexForDoc(doc);
    CHECK(idx.isValid(), );
    tree->selectionModel()->select(idx, QItemSelectionModel::Select);
}

QAction * ProjectTreeController::getLoadSeletectedDocumentsAction() const {
    return loadSelectedDocumentsAction;
}

void ProjectTreeController::updateSettings(const ProjectTreeControllerModeSettings &newSettings) {
    // try to keep GObject selection while reseting view
    QList<GObject*> objects = getGObjectSelection()->getSelectedObjects();
    bool filterChanged = (settings.tokensToShow != newSettings.tokensToShow);
    settings = newSettings;
    CHECK(filterChanged, );

    model->updateSettings(newSettings);

    tree->selectionModel()->clear();
    bool scrolled = false;
    foreach(GObject *obj, objects) {
        QModelIndex index = model->getIndexForObject(obj);
        if (!index.isValid()) {
            continue;
        }
        tree->selectionModel()->select(index, QItemSelectionModel::Select);
        if (!scrolled) {
            tree->scrollTo(index);
        }
    }
    sl_updateActions();
}

void ProjectTreeController::sl_onDocumentAdded(Document *doc) {
    tree->setSortingEnabled(false);
    model->addDocument(doc);
    updater->addDocument(doc);
    tree->setSortingEnabled(true);
    connectDocument(doc);
    sl_updateActions();

    const QModelIndex idx = model->getIndexForDoc(doc);
    CHECK(idx.isValid(), );
    tree->setExpanded(idx, doc->isLoaded());
}

void ProjectTreeController::sl_onDocumentRemoved(Document *doc) {
    disconnectDocument(doc);
    model->removeDocument(doc);
    updater->removeDocument(doc);
    sl_updateActions();
}

void ProjectTreeController::sl_mergeData() {
    const QList<Document*> &docs = AppContext::getProject()->getDocuments();
    foreach(Document *doc, docs) {
        if (!ProjectUtils::isConnectedDatabaseDoc(doc) || doc->isStateLocked()) {
            continue;
        }
        DocumentFoldersUpdate update;
        if (updater->takeData(doc, update)) {
            tree->setSortingEnabled(false);
            model->merge(doc, update);
            tree->setSortingEnabled(true);
        }
    }
    sl_updateActions();
}

void ProjectTreeController::sl_updateSelection() {
    QList<Document*> selectedDocs;
    QList<Folder> selectedFolders;
    QList<GObject*> selectedObjs;

    QModelIndexList selection = tree->selectionModel()->selectedRows();
    foreach (const QModelIndex &index, selection) {
        switch (ProjectViewModel::itemType(index)) {
            case ProjectViewModel::DOCUMENT:
                selectedDocs << ProjectViewModel::toDocument(index);
                break;
            case ProjectViewModel::FOLDER:
                selectedFolders << Folder(*ProjectViewModel::toFolder(index));
                break;
            case ProjectViewModel::OBJECT:
                selectedObjs << ProjectViewModel::toObject(index);
                break;
            default:
                FAIL("Unexpected item type", );
        }
    }

    documentSelection.setSelection(selectedDocs);
    folderSelection.setSelection(selectedFolders);
    objectSelection.setSelection(selectedObjs);

    sl_updateActions();
}

void ProjectTreeController::updateAddObjectAction() {
    QSet<Document*> docsItemsInSelection = getDocsInSelection(false);
    const bool singleDocumentIsChosen = (1 == docsItemsInSelection.size());
    const bool isDatabaseDocument = (docsItemsInSelection.size() > 0 && ProjectUtils::isConnectedDatabaseDoc(docsItemsInSelection.values().first()));
    bool canAddObjectToDocument = true;
    foreach(Document* d, docsItemsInSelection) {
        if (!DocumentUtils::canAddGObjectsToDocument(d, GObjectTypes::SEQUENCE)) {
            canAddObjectToDocument = false;
            break;
        }
    }
    addObjectToDocumentAction->setEnabled(canAddObjectToDocument && singleDocumentIsChosen && !isDatabaseDocument);
}

void ProjectTreeController::updateImportToDbAction() {
    bool isImportActionEnabled = false;
    QList<Folder> folders = getSelectedFolders();
    if (1 == folders.size() && !folders.first().getDocument()->isStateLocked()) {
        const QString actionText = (U2ObjectDbi::ROOT_FOLDER == folders.first().getFolderPath())
            ? tr("Import to the database...")
            : tr("Import to the folder...");
        importToDatabaseAction->setText(actionText);
        isImportActionEnabled = true;
    }
    importToDatabaseAction->setEnabled(isImportActionEnabled);
}

void ProjectTreeController::sl_updateActions() {
    updateAddObjectAction();
    updateImportToDbAction();

    bool canRemoveObjectFromDocument = true;
    bool allObjectsAreInRecycleBin = true;
    QList<GObject*> selectedObjects = objectSelection.getSelectedObjects();
    bool selectedModifiableObjectsExist = !selectedObjects.isEmpty();
    foreach (GObject *obj, selectedObjects) {
        if (!DocumentUtils::canRemoveGObjectFromDocument(obj) && canRemoveObjectFromDocument) {
            canRemoveObjectFromDocument = false;
        }
        if (!isObjectInRecycleBin(obj) && allObjectsAreInRecycleBin) {
            allObjectsAreInRecycleBin = false;
        }
        selectedModifiableObjectsExist &= !obj->getDocument()->isStateLocked();
        if (!canRemoveObjectFromDocument && !allObjectsAreInRecycleBin && !selectedModifiableObjectsExist) {
            break;
        }
    }

    bool allSelectedFoldersAreInRecycleBin = true;
    const QList<Folder> selectedFolders = getSelectedFolders();
    bool selectedModifiableFoldersExist = !selectedFolders.isEmpty();
    bool modifiableRecycleBinSelected = false;
    foreach (const Folder &f, selectedFolders) {
        allSelectedFoldersAreInRecycleBin = ProjectUtils::isFolderInRecycleBin(f.getFolderPath(), false);
        modifiableRecycleBinSelected = ProjectUtils::RECYCLE_BIN_FOLDER_PATH == f.getFolderPath() && !f.getDocument()->isStateLocked();
        selectedModifiableFoldersExist &= !f.getDocument()->isStateLocked();

        if (!allSelectedFoldersAreInRecycleBin && modifiableRecycleBinSelected && !selectedModifiableFoldersExist) {
            break;
        }
    }

    const bool canRestore = selectedModifiableFoldersExist && allSelectedFoldersAreInRecycleBin
        || selectedModifiableObjectsExist && allObjectsAreInRecycleBin;
    restoreSelectedItemsAction->setEnabled(canRestore);
    createFolderAction->setEnabled(canCreateSubFolder());

    const bool selectedDocsExist = !getDocsInSelection(false).isEmpty();

    const bool canRemoveItems = selectedModifiableObjectsExist && canRemoveObjectFromDocument
        || selectedDocsExist || selectedModifiableFoldersExist && !modifiableRecycleBinSelected;
    removeSelectedItemsAction->setEnabled(canRemoveItems);

    const bool canEmptyRecycleBin = modifiableRecycleBinSelected && 1 == selectedFolders.size()
        && !selectedModifiableObjectsExist && !selectedDocsExist;
    emptyRecycleBinAction->setEnabled(canEmptyRecycleBin);

    updateLoadDocumentActions();
    updateReadOnlyFlagActions();
    updateRenameAction();
}

void ProjectTreeController::updateLoadDocumentActions() {
    bool hasUnloadedDocumentInSelection = false;
    bool hasLoadedDocumentInSelection = false;
    foreach(Document *doc, getDocsInSelection(false)) {
        if (!doc->isLoaded()) {
            hasUnloadedDocumentInSelection = true;
            break;
        } else if (!ProjectUtils::isDatabaseDoc(doc)) {
            hasLoadedDocumentInSelection = true;
            break;
        }
    }

    loadSelectedDocumentsAction->setEnabled(hasUnloadedDocumentInSelection);
    unloadSelectedDocumentsAction->setEnabled(hasLoadedDocumentInSelection);
}

void ProjectTreeController::updateReadOnlyFlagActions() {
    const QSet<Document *> docsItemsInSelection = getDocsInSelection(false);
    const bool singleDocumentIsChosen = (1 == docsItemsInSelection.size());

    if (singleDocumentIsChosen) {
        Document *doc = docsItemsInSelection.toList().first();
        const bool docHasUserModLock = doc->hasUserModLock();
        addReadonlyFlagAction->setEnabled(!docHasUserModLock && !doc->isStateLocked());
        removeReadonlyFlagAction->setEnabled(doc->isLoaded() && docHasUserModLock);
    } else {
        addReadonlyFlagAction->setEnabled(false);
        removeReadonlyFlagAction->setEnabled(false);
    }
}

void ProjectTreeController::updateRenameAction() {
    const QModelIndexList selItems = tree->selectionModel()->selectedIndexes();
    bool renameIsOk = false;
    if (selItems.size() == 1 && !AppContext::getProject()->isStateLocked()) {
        if (!objectSelection.isEmpty()) {
            GObject *selectedObj = objectSelection.getSelectedObjects().first();
            const bool parentDocLocked = NULL != selectedObj->getDocument() && selectedObj->getDocument()->isStateLocked();
            renameIsOk = !isObjectInRecycleBin(selectedObj) && !parentDocLocked;
        } else {
            renameIsOk = canRenameFolder();
        }
    }
    renameAction->setEnabled(renameIsOk);
}

void ProjectTreeController::sl_doubleClicked(const QModelIndex &index) {
    switch (ProjectViewModel::itemType(index)) {
        case ProjectViewModel::DOCUMENT: {
            Document *doc = ProjectViewModel::toDocument(index);
            if (!doc->isLoaded() && !doc->getObjects().isEmpty()) {
                SAFE_POINT(loadSelectedDocumentsAction->isEnabled(), "Action is not enabled", );
                loadSelectedDocumentsAction->trigger();
            } else {
                ////children > 0 -> expand action
                tree->setExpanded(index, false); // Magic: false
                emit si_doubleClicked(doc);
            }
            break;
        }
        case ProjectViewModel::FOLDER:
            break;
        case ProjectViewModel::OBJECT:
            emit si_doubleClicked(ProjectViewModel::toObject(index));
            break;
        default:
            FAIL("Unexpected item type", );
    }
}

void ProjectTreeController::sl_documentContentChanged(Document *doc) {
    updater->invalidate(doc);
}

bool ProjectTreeController::canCreateSubFolder() const {
    CHECK(objectSelection.isEmpty(), false);
    const QList<Document *> docs = documentSelection.getSelectedDocuments();
    const QList<Folder> folders = folderSelection.getSelection();
    CHECK((folders.isEmpty() && (1 == docs.size())) ||
        (docs.isEmpty() && (1 == folders.size())), false);

    const QList<Folder> selection = getSelectedFolders();
    CHECK(1 == selection.size(), false);
    const Folder &selectedFolder = selection.first();
    return !ProjectUtils::isFolderInRecycleBin(selectedFolder.getFolderPath()) && !selectedFolder.getDocument()->isStateLocked();
}

void ProjectTreeController::sl_onAddObjectToSelectedDocument() {
    QSet<Document*> selectedDocuments = getDocsInSelection(true);
    SAFE_POINT(1 == selectedDocuments.size(), "No document selected", );
    Document *doc = selectedDocuments.values().first();

    ProjectTreeControllerModeSettings settings;

    // do not show objects from the selected document
    QList<GObject*> docObjects = doc->getObjects();
    foreach (GObject *obj, docObjects) {
        settings.excludeObjectList.append(obj);
    }

    QSet<GObjectType> types = doc->getDocumentFormat()->getSupportedObjectTypes();
    foreach(const GObjectType &type, types) {
        settings.objectTypesToShow.insert(type);
    }

    QList<GObject*> objects = ProjectTreeItemSelectorDialog::selectObjects(settings, tree);
    CHECK(!objects.isEmpty(), );

    foreach(GObject *obj, objects) {
        if (obj->isUnloaded()) {
            continue;
        }

        U2OpStatus2Log os;
        GObject *newObj = obj->clone(doc->getDbiRef(), os);
        CHECK_OP(os, );
        doc->addObject(newObj);
    }
}

void ProjectTreeController::sl_onLoadSelectedDocuments() {
    QSet<Document*> docsInSelection = getDocsInSelection(true);
    QList<Document*> docsToLoad;
    foreach(Document *doc, docsInSelection) {
        if (!doc->isLoaded() && (NULL == LoadUnloadedDocumentTask::findActiveLoadingTask(doc))) {
            docsToLoad << doc;
        }
    }
    runLoadDocumentTasks(docsToLoad);
}

void ProjectTreeController::sl_onUnloadSelectedDocuments() {
    QList<Document*> docsToUnload;
    QSet<Document*> docsInSelection = getDocsInSelection(true);
    foreach(Document *doc, docsInSelection) {
        if (doc->isLoaded() && !ProjectUtils::isDatabaseDoc(doc)) {
            docsToUnload.append(doc);
        }
    }
    UnloadDocumentTask::runUnloadTaskHelper(docsToUnload, UnloadDocumentTask_SaveMode_Ask);
}

void ProjectTreeController::sl_onContextMenuRequested(const QPoint &) {
    QMenu m;
    m.addSeparator();

    ProjectView *pv = AppContext::getProjectView();

    const bool addActionsExist = addObjectToDocumentAction->isEnabled() || createFolderAction->isEnabled();


    if (NULL != pv && addActionsExist) {
        QMenu *addMenu = m.addMenu(tr("Add"));
        addMenu->menuAction()->setObjectName(ACTION_PROJECT__ADD_MENU);
        if (addObjectToDocumentAction->isEnabled()) {
            addMenu->addAction(addObjectToDocumentAction);
        }

         if (importToDatabaseAction->isEnabled()) {
             addMenu->addAction(importToDatabaseAction);
         }

         if (createFolderAction->isEnabled()) {
             addMenu->addAction(createFolderAction);
         }
    }

    QMenu *editMenu = new QMenu(tr("Edit"), &m);
    editMenu->menuAction()->setObjectName(ACTION_PROJECT__EDIT_MENU);
     if(pv != NULL && renameAction->isEnabled()){
         editMenu->addAction(renameAction);
     }
    if (addReadonlyFlagAction->isEnabled()) {
        editMenu->addAction(addReadonlyFlagAction);
    }
    if (removeReadonlyFlagAction->isEnabled()) {
        editMenu->addAction(removeReadonlyFlagAction);
    }

    if (!editMenu->actions().isEmpty()) {
        m.addMenu(editMenu);
    }

    if (emptyRecycleBinAction->isEnabled()) {
        m.addAction(emptyRecycleBinAction);
    }

    if (restoreSelectedItemsAction->isEnabled()) {
        m.addAction(restoreSelectedItemsAction);
    }
    if (removeSelectedItemsAction->isEnabled()) {
        removeSelectedItemsAction->setObjectName(ACTION_PROJECT__REMOVE_SELECTED);
        m.addAction(removeSelectedItemsAction);
    }

    emit si_onPopupMenuRequested(m);

    if (loadSelectedDocumentsAction->isEnabled()) {
        m.addAction(loadSelectedDocumentsAction);
    }
    if (unloadSelectedDocumentsAction->isEnabled()) {
        m.addAction(unloadSelectedDocumentsAction);
        unloadSelectedDocumentsAction->setObjectName(ACTION_PROJECT__UNLOAD_SELECTED);
    }
    m.setObjectName("popMenu");
    m.exec(QCursor::pos());
}

namespace {
    // if objects number in document < MAX_OBJECTS_TO_AUTOEXPAND - document content will be automatically expanded on loading
    const int MAX_OBJECTS_TO_AUTOEXPAND = 20;
    // if documents number in project < MAX_DOCUMENTS_TO_AUTOEXPAND - document content will be automatically expanded on loading
    const int MAX_DOCUMENTS_TO_AUTOEXPAND = 20;
}

void ProjectTreeController::sl_onDocumentLoadedStateChanged() {
    Document *doc = qobject_cast<Document*>(sender());
    SAFE_POINT(NULL != doc, "NULL document", );

    if (doc->isLoaded()) {
        updater->addDocument(doc);
        connectDocument(doc);
    } else {
        updater->removeDocument(doc);
        disconnectDocument(doc);
        connect(doc, SIGNAL(si_loadedStateChanged()), SLOT(sl_onDocumentLoadedStateChanged()));
    }

    if (AppContext::getProject()->getDocuments().size() < MAX_DOCUMENTS_TO_AUTOEXPAND) {
        QModelIndex idx = model->getIndexForDoc(doc);
        CHECK(idx.isValid(), );
        tree->setExpanded(idx, doc->isLoaded());
    }
}

void ProjectTreeController::sl_onRename() {
    CHECK(!AppContext::getProject()->isStateLocked(), );

    const QModelIndexList selection = tree->selectionModel()->selectedIndexes();
    CHECK(selection.size() == 1, );

    const QModelIndex &selectedIndex = selection.first();
    ProjectViewModel::Type indexType = model->itemType(selectedIndex);
    CHECK(ProjectViewModel::DOCUMENT != indexType, );

    Document *doc = NULL;
    switch (indexType) {
    case ProjectViewModel::OBJECT :
        doc = model->toObject(selectedIndex)->getDocument();
        break;
    case ProjectViewModel::FOLDER :
        doc = model->toFolder(selectedIndex)->getDocument();
        break;
    default:
        FAIL("Unexpected project view item type", );
    }
    tree->edit(selectedIndex);
}

void ProjectTreeController::sl_onProjectItemRenamed(const QModelIndex &index) {
    tree->selectionModel()->clear();
    Document *doc = NULL;
    switch (model->itemType(index)) {
    case ProjectViewModel::OBJECT :
        doc = model->toObject(index)->getDocument();
        break;
    case ProjectViewModel::FOLDER :
        doc = model->toFolder(index)->getDocument();
        break;
    case ProjectViewModel::DOCUMENT :
        doc = model->toDocument(index);
        break;
    default:
        FAIL("Unexpected project view item type", );
    }
    updater->invalidate(doc);
}

void ProjectTreeController::sl_onRestoreSelectedItems() {
    restoreSelectedObjects();
    restoreSelectedFolders();
}

void ProjectTreeController::sl_onEmptyRecycleBin() {
    const QList<Folder> selectedFolders = getSelectedFolders();
    SAFE_POINT(!selectedFolders.isEmpty(), "No selected folders found!", );
    Document *doc = selectedFolders.first().getDocument();
    SAFE_POINT(NULL != doc, "Invalid document detected!", );

    QModelIndex rbIndex = model->getIndexForPath(doc, ProjectUtils::RECYCLE_BIN_FOLDER_PATH);
    CHECK(rbIndex.isValid(), );

    QList<Folder> removedFolders;
    QList<GObject *> removedObjects;

    int childCount = model->rowCount(rbIndex);
    for (int i=0; i<childCount; i++) {
        QModelIndex index = model->index(i, 0, rbIndex);
        switch (ProjectViewModel::itemType(index)) {
            case ProjectViewModel::OBJECT:
                removedObjects << ProjectViewModel::toObject(index);
                break;
            case ProjectViewModel::FOLDER:
                removedFolders << *ProjectViewModel::toFolder(index);
                break;
            default:
                FAIL("Unexpected item encountered in Recycle bin!", );
        }
    }

    removeItems(QList<Document *>(), removedFolders, removedObjects);
}

bool ProjectTreeController::canRenameFolder() const {
    CHECK(objectSelection.isEmpty(), false);
    CHECK(documentSelection.isEmpty(), false);
    const QList<Folder> selection = getSelectedFolders();
    CHECK(1 == selection.size(), false);
    const Folder &selectedFolder = selection.first();
    return !ProjectUtils::isFolderInRecycleBin(selectedFolder.getFolderPath()) && !selectedFolder.getDocument()->isStateLocked();
}

void ProjectTreeController::restoreSelectedObjects() {
    const QList<GObject*> objs = objectSelection.getSelectedObjects();

    bool restoreFailed = false;

    QSet<Document *> docs;
    foreach (GObject *obj, objs) {
        Document *doc = obj->getDocument();
        SAFE_POINT(NULL != doc, "Invalid parent document detected!", );
        SAFE_POINT(isObjectInRecycleBin(obj), "Restoring is requested for non removed object!", );
        if (model->restoreObjectItemFromRecycleBin(doc, obj)) {
            docs.insert(doc);
        } else {
            restoreFailed = true;
        }
    }

    foreach (Document *doc, docs) {
        updater->invalidate(doc);
    }

    if (restoreFailed) {
        QMessageBox::warning(QApplication::activeWindow(), tr("Unable to Restore"),
            tr("UGENE is unable to restore some object from Recycle Bin because its original location does not exist. "
            "You can still restore the objects by dragging them with mouse from Recycle Bin."), QMessageBox::Ok);
    }
}

void ProjectTreeController::restoreSelectedFolders() {
    const QList<Folder> folders = folderSelection.getSelection();

    bool restoreFailed = false;

    QSet<Document *> docs;
    foreach (const Folder &folder, folders) {
        Document *doc = folder.getDocument();
        SAFE_POINT(NULL != doc, "Invalid parent document detected!", );

        const QString oldFolderPath = folder.getFolderPath();
        SAFE_POINT(ProjectUtils::isFolderInRecycleBin(oldFolderPath), "Restoring is requested for non removed folder!", );

        if (model->restoreFolderItemFromRecycleBin(doc, oldFolderPath)) {
            docs.insert(doc);
        } else {
            restoreFailed = true;
        }
    }

    foreach (Document *doc, docs) {
        updater->invalidate(doc);
    }

    if (restoreFailed) {
        QMessageBox::warning(QApplication::activeWindow(), tr("Unable to Restore"),
            tr("UGENE is unable to restore some folder from Recycle Bin because its original location does not exist. "
            "You can still restore the folders by dragging them with mouse from Recycle Bin."), QMessageBox::Ok);
    }
}

void ProjectTreeController::sl_onToggleReadonly() {
    QSet<Document*> docsInSelection = getDocsInSelection(true);
    if (docsInSelection.size()!=1) {
        return;
    }
    Document *doc = docsInSelection.toList().first();
    if (!DocumentUtils::getPermissions(doc).testFlag(QFile::WriteUser)){
        QMessageBox::warning(QApplication::activeWindow(), tr("Unable to Change Access Mode"),
            tr("This action requires the modification of the file:\n%1\nYou do not have enough permissions to do this").arg(doc->getURLString()),
            QMessageBox::Ok);
        return;
    }
    if (doc->hasUserModLock()) {
        doc->setUserModLock(false);
    } else {
        doc->setUserModLock(true);
    }
}

void ProjectTreeController::sl_onCreateFolder() {
    const QList<Folder> folders = getSelectedFolders();
    CHECK(1 == folders.size(), );
    const Folder folder = folders.first();
    const QString folderPath = folder.getFolderPath();
    CHECK(!ProjectUtils::isFolderInRecycleBin(folderPath), );

    FolderNameDialog d("", tree);
    if (QDialog::Accepted == d.exec()) {
        QString path = Folder::createPath(folderPath, d.getResult());
        Document *doc = folder.getDocument();
        model->createFolder(doc, path);
        updater->invalidate(doc);
    }
}

void ProjectTreeController::sl_onRemoveSelectedItems() {
    const bool deriveDocsFromObjs = (settings.groupMode != ProjectTreeGroupMode_ByDocument);

    const QList<Document *> selectedDocs = getDocsInSelection(deriveDocsFromObjs).values();
    const QList<Folder> selectedFolders = getSelectedFolders();
    const QList<GObject*> selectedObjects = objectSelection.getSelectedObjects();

    removeItems(selectedDocs, selectedFolders, selectedObjects);
}

void ProjectTreeController::sl_onLockedStateChanged() {
    Document *doc = qobject_cast<Document*>(sender());
    SAFE_POINT(NULL != doc, "NULL document", );

    if (settings.readOnlyFilter != TriState_Unknown) {
        bool remove = (doc->isStateLocked() && settings.readOnlyFilter == TriState_Yes) || (!doc->isStateLocked() && settings.readOnlyFilter == TriState_No);
        if (remove) {
            disconnectDocument(doc);
        } else {
            connectDocument(doc);
        }
    }
}

void ProjectTreeController::sl_onImportToDatabase() {
    QSet<Document*> selectedDocuments = getDocsInSelection(true);
    QList<Folder> selectedFolders = getSelectedFolders();
    bool folderIsSelected = (1 == selectedFolders.size());

    Document* doc = NULL;
    if (folderIsSelected) {
        doc = selectedFolders.first().getDocument();
    } else if (1 == selectedDocuments.size()) {
        doc = selectedDocuments.values().first();
    }
    SAFE_POINT(doc != NULL, tr("Select a database to import anything"), );

    QWidget* mainWindow = qobject_cast<QWidget*>(AppContext::getMainWindow()->getQMainWindow());
    ImportToDatabaseDialog importDialog(doc, selectedFolders.first().getFolderPath(), mainWindow);
    importDialog.exec();
}

void ProjectTreeController::sl_windowActivated(MWMDIWindow *w) {
    if (!settings.markActive) {
        return;
    }

    // listen all add/remove to view events
    if (NULL != markActiveView) {
        markActiveView->disconnect(this);
        markActiveView = NULL;
    }

    GObjectViewWindow *ow = qobject_cast<GObjectViewWindow*>(w);
    CHECK(NULL != ow, );
    uiLog.trace(QString("Project view now listens object events in '%1' view").arg(ow->windowTitle()));
    markActiveView = ow->getObjectView();
    connect(markActiveView, SIGNAL(si_objectAdded(GObjectView*, GObject*)), SLOT(sl_objectAddedToActiveView(GObjectView*, GObject*)));
    connect(markActiveView, SIGNAL(si_objectRemoved(GObjectView*, GObject*)), SLOT(sl_objectRemovedFromActiveView(GObjectView*, GObject*)));
    foreach (GObject *obj, ow->getObjects()) {
        updateObjectActiveStateVisual(obj);
    }
}

void ProjectTreeController::sl_windowDeactivated(MWMDIWindow *w) {
    GObjectViewWindow *ow = qobject_cast<GObjectViewWindow*>(w);
    CHECK(NULL != ow, );
    foreach (GObject *obj, ow->getObjects()) {
        updateObjectActiveStateVisual(obj);
    }
}

void ProjectTreeController::sl_objectAddedToActiveView(GObjectView *, GObject *obj) {
    SAFE_POINT(NULL != obj, tr("No object to add to view"), );
    uiLog.trace(QString("Processing object add to active view in project tree: %1").arg(obj->getGObjectName()));
    updateObjectActiveStateVisual(obj);
}

void ProjectTreeController::sl_objectRemovedFromActiveView(GObjectView *, GObject *obj) {
    SAFE_POINT(NULL != obj, tr("No object to remove from view"), );
    uiLog.trace(QString("Processing object remove form active view in project tree: %1").arg(obj->getGObjectName()));
    updateObjectActiveStateVisual(obj);
}

void ProjectTreeController::sl_onResourceUserRegistered(const QString & /*res*/, Task *t) {
    LoadUnloadedDocumentTask *lut = qobject_cast<LoadUnloadedDocumentTask*>(t);
    CHECK(NULL != lut, );
    connect(lut, SIGNAL(si_progressChanged()), SLOT(sl_onLoadingDocumentProgressChanged()));
}

void ProjectTreeController::sl_onResourceUserUnregistered(const QString & /*res*/, Task *t) {
    LoadUnloadedDocumentTask *lut = qobject_cast<LoadUnloadedDocumentTask*>(t);
    CHECK(NULL != lut, );
    lut->disconnect(this);

    Document *doc = lut->getDocument();
    CHECK(NULL != doc, );
    updateLoadingState(doc);
}

void ProjectTreeController::sl_onLoadingDocumentProgressChanged() {
    LoadUnloadedDocumentTask *lut = qobject_cast<LoadUnloadedDocumentTask*>(sender());
    CHECK(NULL != lut, );
    Document *doc = lut->getDocument();
    CHECK(NULL != doc, );
    updateLoadingState(doc);
}

bool ProjectTreeController::eventFilter(QObject *o, QEvent *e) {
    QTreeView *tree = dynamic_cast<QTreeView*>(o);
    CHECK(NULL != tree, false);

    if (QEvent::KeyPress == e->type()) {
        QKeyEvent *kEvent = dynamic_cast<QKeyEvent*>(e);
        CHECK(NULL != kEvent, false);
        int key = kEvent->key();
        bool hasSelection = !documentSelection.isEmpty() || !objectSelection.isEmpty() || !folderSelection.isEmpty();
        if (key == Qt::Key_F2 && hasSelection) {
            if (renameAction->isEnabled()) {
                sl_onRename();
            }
            return true;
        } else if ((key == Qt::Key_Return || key == Qt::Key_Enter) && hasSelection) {
            if (!objectSelection.isEmpty()) {
                GObject *obj = objectSelection.getSelectedObjects().last();
                QModelIndex idx = model->getIndexForObject(obj);
                CHECK(idx.isValid(), false);
                if (!model->flags(idx).testFlag(Qt::ItemIsEditable)) {
                    emit si_returnPressed(obj);
                    return true;
                }
            }
            if (!documentSelection.isEmpty()) {
                Document *doc = documentSelection.getSelectedDocuments().last();
                emit si_returnPressed(doc);
            }
            return true;
        }
    }

    return false;
}

void ProjectTreeController::setupActions() {
    addObjectToDocumentAction = new QAction(QIcon(":core/images/add_gobject.png"), tr("Add object to document..."), this);
    addObjectToDocumentAction->setObjectName(ACTION_PROJECT__ADD_OBJECT);
    tree->addAction(addObjectToDocumentAction);
    connect(addObjectToDocumentAction, SIGNAL(triggered()), SLOT(sl_onAddObjectToSelectedDocument()));

    importToDatabaseAction = new QAction(QIcon(":core/images/db/database_copy.png"), tr("Import..."), this);
    importToDatabaseAction->setObjectName(ACTION_PROJECT__IMPORT_TO_DATABASE);
    tree->addAction(importToDatabaseAction);
    connect(importToDatabaseAction, SIGNAL(triggered()), SLOT(sl_onImportToDatabase()));

    loadSelectedDocumentsAction = new QAction(QIcon(":core/images/load_selected_documents.png"), tr("Load selected documents"), this);
    loadSelectedDocumentsAction->setObjectName("action_load_selected_documents");
    loadSelectedDocumentsAction->setShortcuts(QList<QKeySequence>() << Qt::Key_Enter << Qt::Key_Return);
    loadSelectedDocumentsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    tree->addAction(loadSelectedDocumentsAction);
    connect(loadSelectedDocumentsAction, SIGNAL(triggered()), SLOT(sl_onLoadSelectedDocuments()));

    unloadSelectedDocumentsAction = new QAction( QIcon(":core/images/unload_document.png"), tr("Unload selected document"), this);
    connect(unloadSelectedDocumentsAction, SIGNAL(triggered()), SLOT(sl_onUnloadSelectedDocuments()));

    addReadonlyFlagAction = new QAction(tr("Lock document for editing"), this);
    addReadonlyFlagAction->setObjectName(ACTION_DOCUMENT__LOCK);
    connect(addReadonlyFlagAction, SIGNAL(triggered()), SLOT(sl_onToggleReadonly()));

    removeReadonlyFlagAction = new QAction(tr("Unlock document for editing"), this);
    removeReadonlyFlagAction->setObjectName(ACTION_DOCUMENT__UNLOCK);
    connect(removeReadonlyFlagAction, SIGNAL(triggered()), SLOT(sl_onToggleReadonly()));

    renameAction = new QAction(tr("Rename..."), this);
    connect(renameAction, SIGNAL(triggered()), SLOT(sl_onRename()));
    renameAction->setObjectName("Rename");

    removeSelectedItemsAction = new QAction(QIcon(":core/images/remove_selected_documents.png"), tr("Remove selected items"), this);
    removeSelectedItemsAction->setShortcut(QKeySequence::Delete);
    removeSelectedItemsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    tree->addAction(removeSelectedItemsAction);
    connect(removeSelectedItemsAction, SIGNAL(triggered()), SLOT(sl_onRemoveSelectedItems()));

    createFolderAction = new QAction(QIcon(":U2Designer/images/add_directory.png"), tr("Add folder..."), this);
    createFolderAction->setObjectName(ACTION_PROJECT__CREATE_FOLDER);
    connect(createFolderAction, SIGNAL(triggered()), SLOT(sl_onCreateFolder()));
    tree->addAction(createFolderAction);

    restoreSelectedItemsAction = new QAction(QIcon(":core/images/bin_restore2.png"), tr("Restore selected items"), this);
    connect(restoreSelectedItemsAction, SIGNAL(triggered()), SLOT(sl_onRestoreSelectedItems()));
    tree->addAction(restoreSelectedItemsAction);

    emptyRecycleBinAction = new QAction(QIcon(":core/images/clean_full_bin.png"), tr("Empty recycle bin"), this);
    tree->addAction(emptyRecycleBinAction);
    connect(emptyRecycleBinAction, SIGNAL(triggered()), SLOT(sl_onEmptyRecycleBin()));
    emptyRecycleBinAction->setObjectName("empty_rb");
}

void ProjectTreeController::connectDocument(Document *doc) {
    connect(doc, SIGNAL(si_loadedStateChanged()), SLOT(sl_onDocumentLoadedStateChanged()));
    connect(doc, SIGNAL(si_lockedStateChanged()), SLOT(sl_onLockedStateChanged()));
}

void ProjectTreeController::disconnectDocument(Document *doc) {
    doc->disconnect(this);
}

void ProjectTreeController::connectToResourceTracker() {
    connect(AppContext::getResourceTracker(),
        SIGNAL(si_resourceUserRegistered(const QString&, Task*)),
        SLOT(sl_onResourceUserRegistered(const QString&, Task*)));

    connect(AppContext::getResourceTracker(),
        SIGNAL(si_resourceUserUnregistered(const QString&, Task*)),
        SLOT(sl_onResourceUserUnregistered(const QString&, Task*)));


    foreach(Document *doc, AppContext::getProject()->getDocuments()) {
        const QString resName = LoadUnloadedDocumentTask::getResourceName(doc);
        QList<Task*> users = AppContext::getResourceTracker()->getResourceUsers(resName);
        foreach(Task *t, users) {
            sl_onResourceUserRegistered(resName, t);
        }
    }
}

void ProjectTreeController::updateLoadingState(Document *doc) {
    if (settings.isDocumentShown(doc)) {
        QModelIndex idx = model->getIndexForDoc(doc);
        if (idx.isValid()) {
            model->updateData(idx);
        }
    }
    if (doc->getObjects().size() < ProjectUtils::MAX_OBJS_TO_SHOW_LOAD_PROGRESS) {
        foreach(GObject *obj, doc->getObjects()) {
            if (settings.isObjectShown(obj)) {
                QModelIndex idx = model->getIndexForObject(obj);
                if (!idx.isValid()) {
                    continue;
                }
                model->updateData(idx);
            }
        }
    }
}

void ProjectTreeController::runLoadDocumentTasks(const QList<Document*> &docs) const {
    QList<Task*> tasks;
    if (NULL != settings.loadTaskProvider) {
        tasks = settings.loadTaskProvider->createLoadDocumentTasks(docs);
    } else {
        foreach(Document *doc, docs) {
            tasks << new LoadUnloadedDocumentTask(doc);
        }
    }
    foreach(Task *t, tasks) {
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    }
}

QSet<Document*> ProjectTreeController::getDocsInSelection(bool deriveFromObjects) const {
    QSet<Document*> result = documentSelection.getSelectedDocuments().toSet();

    if (deriveFromObjects) {
        foreach(GObject *obj, objectSelection.getSelectedObjects()) {
            Document *doc = obj->getDocument();
            SAFE_POINT(NULL != doc, "NULL document", result);
            result << doc;
        }
    }

    return result;
}

QList<Folder> ProjectTreeController::getSelectedFolders() const {
    QList<Folder> result;
    foreach (Document *doc, documentSelection.getSelectedDocuments()) {
        if (ProjectUtils::isConnectedDatabaseDoc(doc)) {
            result << Folder(doc, U2ObjectDbi::ROOT_FOLDER);
        }
    }
    result << folderSelection.getSelection();

    return result;
}

void ProjectTreeController::removeItems(const QList<Document*> &docs, const QList<Folder> &folders, const QList<GObject*> &objs) {
    bool itemsInRecycleBin = false;
    foreach (GObject *obj, objs) {
        itemsInRecycleBin = isObjectInRecycleBin(obj);
        if (itemsInRecycleBin) {
            break;
        }
    }
    if (!itemsInRecycleBin) {
        foreach (const Folder &f, folders) {
            itemsInRecycleBin = ProjectUtils::isFolderInRecycleBin(f.getFolderPath());
            if (itemsInRecycleBin) {
                break;
            }
        }
    }
    if (itemsInRecycleBin) {
        QMessageBox::StandardButton choice = QMessageBox::warning(QApplication::activeWindow(),
            QObject::tr("Confirm Deletion"),
            QObject::tr("Are you sure you want to delete items in Recycle Bin?\n"
            "The items cannot be recovered once deleted."), QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

        if (QMessageBox::No == choice) {
            return;
        }
    }

    bool objectsRemoved = removeObjects(objs, docs, folders, true);
    bool foldersRemoved = removeFolders(folders, docs);
    removeDocuments(docs);

    if (!foldersRemoved || !objectsRemoved) {
        QMessageBox::warning(QApplication::activeWindow(), QObject::tr("Unable to Remove"),
            QObject::tr("Some of selected objects are being used by the other users of the database. "
            "Try to remove them later."));
    }
}

bool ProjectTreeController::isSubFolder(const QList<Folder> &folders, const Folder &expectedSubFolder, bool trueIfSamePath) {
    foreach (const Folder &folder, folders) {
        if (folder.getDocument() != expectedSubFolder.getDocument()) {
            continue;
        }
        if (Folder::isSubFolder(folder.getFolderPath(), expectedSubFolder.getFolderPath())) {
            return true;
        }
    }

    if (trueIfSamePath) {
        return folders.contains(expectedSubFolder);
    }
    return false;
}

bool ProjectTreeController::isObjectInFolder(GObject *obj, const Folder &folder) const {
    Document *objDoc = obj->getDocument();
    SAFE_POINT(NULL != objDoc, "Invalid parent document", false);
    Document *folderDoc = folder.getDocument();
    SAFE_POINT(NULL != folderDoc, "Invalid parent document", false);
    if (objDoc != folderDoc) {
        return false;
    }

    const Folder objFolder(objDoc, model->getObjectFolder(objDoc, obj));
    return isSubFolder(QList<Folder>() << folder, objFolder, true);
}

bool ProjectTreeController::removeObjects(const QList<GObject*> &objs, const QList<Document*> &excludedDocs, const QList<Folder> &excludedFolders, bool removeFromDbi) {
    bool deletedSuccessfully = true;
    QHash<GObject *, Document *> objects2Doc;

    foreach (GObject *obj, objs) {
        Document *doc = obj->getDocument();
        SAFE_POINT(NULL != doc, "Invalid parent document detected!", false);

        const Folder curFolder(doc, model->getObjectFolder(doc, obj));
        bool parentFolderSelected = isSubFolder(excludedFolders, curFolder, true);
        bool parentDocSelected = excludedDocs.contains(doc);
        if (parentDocSelected || parentFolderSelected) {
            continue;
        } else if (!ProjectUtils::isDatabaseDoc(doc) || isObjectInRecycleBin(obj)) {
            objectSelection.removeFromSelection(obj);
            if (doc->removeObject(obj, DocumentObjectRemovalMode_Release)) {
                objects2Doc.insert(obj, doc);
                if (removeFromDbi) {
                    model->addToIgnoreObjFilter(doc, obj->getEntityRef().entityId);
                }
            } else {
                deletedSuccessfully = false;
            }
        } else if (!isObjectInRecycleBin(obj)) {
            objectIsBeingRecycled = obj;
            model->moveObject(doc, obj, ProjectUtils::RECYCLE_BIN_FOLDER_PATH);
            QCoreApplication::processEvents();
            objectIsBeingRecycled = NULL;
        }
        updater->invalidate(doc);
    }

    if (removeFromDbi && !objects2Doc.isEmpty()) {
        Task *t = new DeleteObjectsTask(objects2Doc.keys());
        startTrackingRemovedObjects(t, objects2Doc);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_onObjRemovalTaskFinished()));
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    } else {
        foreach (GObject *obj, objects2Doc.keys()) {
            delete obj;
        }
    }

    return deletedSuccessfully;
}

bool ProjectTreeController::removeFolders(const QList<Folder> &folders, const QList<Document*> &excludedDocs) {
    QList<Folder> folders2Delete;

    bool deletedSuccessfully = true;
    QSet<Document *> relatedDocs;
    foreach (const Folder &folder, folders) {
        Document *doc = folder.getDocument();
        SAFE_POINT(NULL != doc, "Invalid parent document detected!", false);
        bool parentFolderSelected = isSubFolder(folders, folder, false);
        bool parentDocSelected = excludedDocs.contains(doc);

        const QString &folderPath = folder.getFolderPath();
        if (parentDocSelected || parentFolderSelected || !ProjectUtils::isFolderRemovable(folderPath)) {
            continue;
        } else if (ProjectUtils::isFolderInRecycleBin(folderPath)) {
            QList<GObject*> objects = model->getFolderContent(doc, folderPath);
            deletedSuccessfully &= removeObjects(objects, excludedDocs, QList<Folder>(), false);
            if (!deletedSuccessfully) {
                continue;
            }
            model->removeFolder(doc, folderPath);
            folders2Delete << folder;
            model->addToIgnoreFolderFilter(doc, folderPath);
        } else {
            const QString dstPath = ProjectUtils::RECYCLE_BIN_FOLDER_PATH + folderPath;
            model->renameFolder(doc, folderPath, dstPath);
        }
        relatedDocs.insert(doc);
    }
    if (!folders2Delete.isEmpty()) {
        Task *t = new DeleteFoldersTask(folders2Delete);
        startTrackingRemovedFolders(t, folders2Delete);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_onFolderRemovalTaskFinished()));
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    }
    foreach (Document *doc, relatedDocs) {
        updater->invalidate(doc);
    }
    return deletedSuccessfully;
}

void ProjectTreeController::sl_onObjRemovalTaskFinished() {
    Task *removalTask = qobject_cast<Task *>(sender());
    if (NULL != removalTask && removalTask->isFinished()) {
        SAFE_POINT(task2ObjectsBeingDeleted.contains(removalTask), "Invalid object removal task detected", );
        QHash<Document *, QSet<U2DataId> > &doc2ObjIds = task2ObjectsBeingDeleted[removalTask];
        foreach (Document *doc, doc2ObjIds.keys()) {
            if (model->hasDocument(doc)) {
                model->excludeFromObjIgnoreFilter(doc, doc2ObjIds[doc]);
                updater->invalidate(doc);
            }
        }
        task2ObjectsBeingDeleted.remove(removalTask);
    }
}

void ProjectTreeController::sl_onFolderRemovalTaskFinished() {
    Task *removalTask = qobject_cast<Task *>(sender());
    if (NULL != removalTask && removalTask->isFinished()) {
        SAFE_POINT(task2FoldersBeingDeleted.contains(removalTask), "Invalid folder removal task detected", );
        QHash<Document *, QSet<QString> > &doc2Paths = task2FoldersBeingDeleted[removalTask];
        foreach (Document *doc, doc2Paths.keys()) {
            model->excludeFromFolderIgnoreFilter(doc, doc2Paths[doc]);
            updater->invalidate(doc);
        }
        task2FoldersBeingDeleted.remove(removalTask);
    }
}

void ProjectTreeController::startTrackingRemovedObjects(Task *deleteTask, const QHash<GObject *, Document *> &objs2Docs) {
    SAFE_POINT(NULL != deleteTask && !objs2Docs.isEmpty(), "Incorrect objects removal", );

    task2ObjectsBeingDeleted.insert(deleteTask, QHash<Document *, QSet<U2DataId> >());
    QHash<Document *, QSet<U2DataId> > &doc2ObjIds = task2ObjectsBeingDeleted[deleteTask];
    foreach (GObject *o, objs2Docs.keys()) {
        Document *parentDoc = objs2Docs[o];
        SAFE_POINT(NULL != parentDoc, "Invalid parent document detected", );
        if (!doc2ObjIds.contains(parentDoc)) {
            doc2ObjIds.insert(parentDoc, QSet<U2DataId>());
        }
        doc2ObjIds[parentDoc].insert(o->getEntityRef().entityId);
    }
}

void ProjectTreeController::startTrackingRemovedFolders(Task *deleteTask, const QList<Folder> &folders) {
    SAFE_POINT(NULL != deleteTask && !folders.isEmpty(), "Incorrect folders removal", );

    task2FoldersBeingDeleted.insert(deleteTask, QHash<Document *, QSet<QString> >());
    QHash<Document *, QSet<QString> > &doc2Folders = task2FoldersBeingDeleted[deleteTask];
    foreach (const Folder &f, folders) {
        Document *parentDoc = f.getDocument();
        SAFE_POINT(NULL != parentDoc, "Invalid parent document detected", );
        if (!doc2Folders.contains(parentDoc)) {
            doc2Folders.insert(parentDoc, QSet<QString>());
        }
        doc2Folders[parentDoc].insert(f.getFolderPath());
    }
}

void ProjectTreeController::removeDocuments(const QList<Document*> &docs) {
    QSet<Document*> docsInSelection = getDocsInSelection(settings.groupMode != ProjectTreeGroupMode_ByDocument);
    if (!docs.isEmpty()) {
        AppContext::getTaskScheduler()->registerTopLevelTask(new RemoveMultipleDocumentsTask(AppContext::getProject(), docs, true, true));
    }
}

void ProjectTreeController::updateObjectActiveStateVisual(GObject *obj) {
    SAFE_POINT(NULL != obj, "ProjectTreeController::updateObjectActiveStateVisual. Object is NULL", );
    CHECK(objectIsBeingRecycled != obj, );
    if (ProjectTreeGroupMode_ByDocument == settings.groupMode) {
        Document *parentDoc = obj->getDocument();
        CHECK(model->hasDocument(parentDoc), );
        QModelIndex docIdx = model->getIndexForDoc(parentDoc);
        CHECK(docIdx.isValid(), );
        model->updateData(docIdx);

        CHECK(model->hasObject(parentDoc, obj), );
        QModelIndex objIdx = model->getIndexForObject(obj);
        CHECK(objIdx.isValid(), );
        model->updateData(objIdx);
    } else {
        QModelIndex idx = model->getIndexForObject(obj);
        CHECK(idx.isValid(), );
        model->updateData(idx);
    }
}

} // U2
