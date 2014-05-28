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

#ifndef _U2_PROJECTTREECONTROLLER_H_
#define _U2_PROJECTTREECONTROLLER_H_

#include <QtGui/QAction>

#include <U2Core/DocumentSelection.h>
#include <U2Core/FolderSelection.h>
#include <U2Core/GObjectSelection.h>

#include <U2Gui/ProjectViewModel.h>

class QTreeView;

namespace U2 {

class GObjectView;
class MWMDIWindow;
class ProjectUpdater;
class Task;

class U2GUI_EXPORT ProjectTreeController : public QObject {
    Q_OBJECT
public:
    ProjectTreeController(QTreeView *tree, const ProjectTreeControllerModeSettings &settings, QObject *parent);
    ~ProjectTreeController();

    const DocumentSelection * getDocumentSelection() const;
    const GObjectSelection * getGObjectSelection() const;

    bool isObjectInRecycleBin(GObject *obj) const;
    const ProjectTreeControllerModeSettings & getModeSettings() const;
    void highlightItem(Document *doc);
    QAction * getLoadSeletectedDocumentsAction() const;
    void updateSettings(const ProjectTreeControllerModeSettings &settings);

private slots:
    void sl_onDocumentAdded(Document *doc);
    void sl_onDocumentRemoved(Document *doc);
    void sl_mergeData();
    void sl_updateSelection();
    void sl_updateActions();
    void sl_doubleClicked(const QModelIndex &index);
    void sl_documentContentChanged(Document *doc);

    void sl_onCreateFolder();
    void sl_onAddObjectToSelectedDocument();
    void sl_onLoadSelectedDocuments();
    void sl_onUnloadSelectedDocuments();
    void sl_onContextMenuRequested(const QPoint &pos);
    void sl_onDocumentLoadedStateChanged();
    void sl_onToggleReadonly();
    void sl_onRemoveSelectedItems();
    void sl_onLockedStateChanged();
    void sl_onImportToDatabase();
    void sl_windowActivated(MWMDIWindow *w);
    void sl_objectAddedToActiveView(GObjectView *w, GObject *obj);
    void sl_objectRemovedFromActiveView(GObjectView *w, GObject *obj);
    void sl_onResourceUserRegistered(const QString &res, Task *t);
    void sl_onResourceUserUnregistered(const QString &res, Task *t);
    void sl_onLoadingDocumentProgressChanged();
    void sl_onRename();
    void sl_onRestoreSelectedItems();
    void sl_onEmptyRecycleBin();
    void sl_onProjectItemRenamed(const QModelIndex &index);

signals:
    void si_onPopupMenuRequested(QMenu &popup);
    void si_doubleClicked(GObject *obj);
    void si_doubleClicked(Document *doc);
    void si_returnPressed(GObject *obj);
    void si_returnPressed(Document *doc);

private:
    // QObject
    bool eventFilter(QObject *o, QEvent *e);

    void setupActions();
    void connectDocument(Document *doc);
    void disconnectDocument(Document *doc);
    void connectToResourceTracker();
    void updateLoadingState(Document *doc);
    void runLoadDocumentTasks(const QList<Document*> &docs) const;
    QSet<Document*> getDocsInSelection(bool deriveFromObjects) const;
    QList<Folder> getSelectedFolders() const;
    void removeItems(const QList<Document*> &docs, const QList<Folder> &folders, const QList<GObject*> &objs);
    bool removeObjects(const QList<GObject*> &objs, const QList<Document*> &excludedDocs, const QList<Folder> &excludedFolders, bool removeFromDbi);
    bool removeFolders(const QList<Folder> &folders, const QList<Document*> &excludedDocs);
    void removeDocuments(const QList<Document*> &docs);
    void updateObjectActiveStateVisual(GObject *obj);
    bool canCreateSubFolder() const;
    bool canRenameFolder() const;
    void restoreSelectedObjects();
    void restoreSelectedFolders();

    static bool isSubFolder(const QList<Folder> &folders, const Folder &expectedSubFolder, bool trueIfSamePath);

private:
    QTreeView *tree;
    ProjectTreeControllerModeSettings settings;
    ProjectUpdater *updater;
    ProjectViewModel *model;

    // Actions
    QAction *createFolderAction;
    QAction *addObjectToDocumentAction;
    QAction *loadSelectedDocumentsAction;
    QAction *unloadSelectedDocumentsAction;
    QAction *addReadonlyFlagAction;
    QAction *renameAction;
    QAction *removeReadonlyFlagAction;
    QAction *removeSelectedItemsAction;
    QAction *importToDatabaseAction;
    QAction *restoreSelectedItemsAction;
    QAction *emptyRecycleBinAction;

    DocumentSelection documentSelection;
    FolderSelection folderSelection;
    GObjectSelection objectSelection;
    GObjectView *markActiveView;
};

} // U2

#endif // _U2_PROJECTTREECONTROLLER_H_
