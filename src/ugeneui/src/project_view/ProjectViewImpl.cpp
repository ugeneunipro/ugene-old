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

#include "ProjectViewImpl.h"

#include <AppContextImpl.h>

#include <U2Core/ProjectService.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/Settings.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/Log.h>
#include <U2Core/GObject.h>
#include <U2Core/GUrl.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DbiDocumentFormat.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/MultiTask.h>
#include <U2Core/CopyDataTask.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/SelectionUtils.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/OpenViewTask.h>
#include <U2Gui/UnloadDocumentTask.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/CopyDocumentDialogController.h>

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceWidget.h>
#include <U2View/ADVSingleSequenceWidget.h>

#include <QtGui/QtGui>
#include <memory>

namespace U2 {


/* TRANSLATOR U2::ProjectViewImpl */
/* TRANSLATOR U2::ProjectTreeController */

#define SETTINGS_ROOT QString("projecview/")

#define UPDATER_TIMEOUT 3000

DocumentUpdater::DocumentUpdater(QObject* p) : QObject(p) {
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(sl_update()));
    timer->start(UPDATER_TIMEOUT);
    recursion = false;
    updateTask = NULL;
}

void DocumentUpdater::sl_update() {
    if (recursion || updateTask != NULL) {
        return;
    }
    recursion = true;
    update();
    recursion = false;
}

static bool hasActiveDialogs(QObject* o) {
    const QObjectList& childObjects = o->children();

    foreach(QObject* o, childObjects) {
        if (hasActiveDialogs(o)) {
            return true;
        }
    }
    QDialog* d = qobject_cast<QDialog*>(o);
    if ( d != NULL && d->isVisible() ) {
        //coreLog.trace(QString("Rejecting dialog %1").arg(o->metaObject()->className()));
        return true;
    }
    return false;
}

void DocumentUpdater::update() {
    Project* prj = AppContext::getProject();
    assert(prj);

    // don't check documents currently used by save/load tasks
    QList<Document*> docs2check = prj->getDocuments();
    excludeDocumentsInTasks(AppContext::getTaskScheduler()->getTopLevelTasks(), docs2check);

    // build list of documents which files were modified between calls to sl_update()
    QList<Document*> outdatedDocs;
    foreach(Document* doc, docs2check) {
        if (!doc->isLoaded()) {
            continue;
        }

        DbiDocumentFormat* dbiFormat = qobject_cast<DbiDocumentFormat*>(doc->getDocumentFormat());
        if (dbiFormat) {
            continue;
        }

        QFileInfo fi(doc->getURLString());
        QDateTime updTime = doc->getLastUpdateTime();

        // last update time is updated by save/load tasks
        // if it's a null the document was not loaded or saved => reload is pointless
        // if it's not a null and file not exists => file was deleted (don't reload)
        if (updTime.isNull() || !fi.exists()) {
            continue;
        }
        if (fi.lastModified() != updTime) { // file was modified
            outdatedDocs.append(doc);
        }
    }

    if (outdatedDocs.isEmpty()) {
        return;
    }
    
    coreLog.trace(QString("Found %1 outdated docs!").arg(outdatedDocs.size()));
    foreach(GObjectViewWindow*  vw, GObjectViewUtils::getAllActiveViews()) {
        if (hasActiveDialogs(vw)) {
            coreLog.trace(QString("View: '%1' has active dialogs, skipping reload").arg(vw->windowTitle()));
            return;
        }
    }

    
    // query user what documents he wants to reload
    // reloaded document modification time will be updated in load task
    QList<Document*> docs2Reload;
    QListIterator<Document*> iter(outdatedDocs);
    while (iter.hasNext()) {
        Document* doc = iter.next();
        QMessageBox::StandardButton btn = QMessageBox::question(
            QApplication::activeWindow(),
            U2_APP_TITLE,
            tr("Document '%1' was modified. Do you wish to reload it?").arg(doc->getName()),
            QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll);

        switch (btn) {
            case QMessageBox::Yes:
                docs2Reload.append(doc);
                break;
            case QMessageBox::YesToAll:
                docs2Reload.append(doc);
                while (iter.hasNext()) {
                    doc = iter.next();
                    docs2Reload.append(doc);
                }
                break;
            case QMessageBox::No:
                doc->setLastUpdateTime();
                break;
            case QMessageBox::NoToAll:
                doc->setLastUpdateTime();
                while (iter.hasNext()) {
                    doc = iter.next();
                    doc->setLastUpdateTime();
                }
                break;
                default:;
        }
    }

    if (docs2Reload.isEmpty()) {
        return;
    }

    // setup multi task : reload documents + open views

    ReloadDocuments(docs2Reload);

}

void DocumentUpdater::sl_updateTaskStateChanged() {
    SAFE_POINT(updateTask != NULL, "updateTask is NULL?", );
    if (updateTask->isFinished()) {
        updateTask = NULL;
    }
}

void DocumentUpdater::excludeDocumentsInTasks(const QList<Task*>& tasks, QList<Document*>& documents) {
    foreach(Task* task, tasks) {
        excludeDocumentsInTasks(task->getSubtasks(), documents);
        SaveDocumentTask* saveTask = qobject_cast<SaveDocumentTask*>(task);
        if (saveTask) {
            documents.removeAll(saveTask->getDocument());
        } else {
            LoadDocumentTask* loadTask = qobject_cast<LoadDocumentTask*>(task);
            if (loadTask) {
                documents.removeAll(loadTask->getDocument());
            }
        }
    }
}

void DocumentUpdater::ReloadDocuments( QList<Document*> docs2Reload ){
    Task* reloadTask = new Task(tr("Reload documents task"), TaskFlag_NoRun);

    QList<GObjectViewState*> states;
    QList<GObjectViewWindow*> viewWindows;

    foreach(Document* doc, docs2Reload) {
        QList<GObjectViewWindow*> viewWnds = GObjectViewUtils::findViewsWithAnyOfObjects(doc->getObjects());
        foreach(GObjectViewWindow* vw, viewWnds) {
            if (viewWindows.contains(vw)) {
                continue;
            }
            viewWindows.append(vw);

            GObjectViewFactoryId id = vw->getViewFactoryId();
            QVariantMap stateData = vw->getObjectView()->saveState();
            if (stateData.isEmpty()) {
                continue;
            }
            states << new GObjectViewState(id, vw->getViewName(), "", stateData);

            vw->closeView();
        }

        QString unloadErr = UnloadDocumentTask::checkSafeUnload(doc);
        if (!unloadErr.isEmpty()) {
            QMessageBox::warning(QApplication::activeWindow(),
                U2_APP_TITLE,
                tr("Unable to unload '%1'").arg(doc->getName())
                );
            doc->setLastUpdateTime();
            continue;
        }

        QList<Task*> subs;
        subs << new UnloadDocumentTask(doc, false);
        subs << new LoadUnloadedDocumentTask(doc);
        reloadTask->addSubTask(new MultiTask(tr("Reload '%1' task").arg(doc->getName()), subs));
    }

    Task* updateViewTask = new Task(tr("Restore state task"), TaskFlag_NoRun);

    foreach(GObjectViewState* state, states) {
        GObjectViewWindow* view = GObjectViewUtils::findViewByName(state->getViewName());
        if (view!=NULL) {
            assert(view->isPersistent());
            AppContext::getMainWindow()->getMDIManager()->activateWindow(view);
            updateViewTask->addSubTask(view->getObjectView()->updateViewTask(state->getStateName(), state->getStateData()));
        } else {
            GObjectViewFactory* f = AppContext::getObjectViewFactoryRegistry()->getFactoryById(state->getViewFactoryId());
            assert(f!=NULL);
            updateViewTask->addSubTask(f->createViewTask(state->getViewName(), state->getStateData()));
        }
        delete state;
    }

    QList<Task*> subs;
    subs << reloadTask << updateViewTask;
    updateTask = new MultiTask(tr("Reload documents and restore view state task"), subs);
    connect(updateTask, SIGNAL(si_stateChanged()), SLOT(sl_updateTaskStateChanged()));
    AppContext::getTaskScheduler()->registerTopLevelTask(updateTask);

}

ProjectViewWidget::ProjectViewWidget() {
    setupUi(this);
    groupModeMenu = new QMenu(tr("Group mode"), this);
    groupModeMenu->setObjectName("group_Mode_Menu");
    groupModeButton->setMenu(groupModeMenu);
    groupModeButton->setPopupMode(QToolButton::InstantPopup);
    groupModeButton->setIcon(QIcon(":ugene/images/project_filter.png"));
    
    setObjectName(DOCK_PROJECT_VIEW);
    setWindowTitle(tr("Project"));
    setWindowIcon(QIcon(":ugene/images/project.png"));

    updater = new DocumentUpdater(this);
}

static ProjectTreeGroupMode getLastGroupMode() {
    int n = AppContext::getSettings()->getValue(SETTINGS_ROOT + "groupMode", ProjectTreeGroupMode_ByDocument).toInt();
    n = qBound((int)ProjectTreeGroupMode_Min, n, (int)ProjectTreeGroupMode_Max);
    return (ProjectTreeGroupMode)n;
}

static void saveGroupMode(ProjectTreeGroupMode m) {
    AppContext::getSettings()->setValue(SETTINGS_ROOT + "groupMode", (int)m);
}


//////////////////////////////////////////////////////////////////////////
// ProjectViewImpl
ProjectViewImpl::ProjectViewImpl()
: ProjectView(tr("projectview_sname"), tr("projectview_sdesc"))
{
    w = NULL;
    projectTreeController = NULL;
    objectViewController = NULL;
    addExistingDocumentAction = NULL;
//    addNewDocumentAction = NULL;
    saveSelectedDocsAction = NULL;
    relocateDocumentAction = NULL;
    saveProjectOnClose = false;

    //todo: move it somewhere else -> object views could be openend without project view service active
    registerBuiltInObjectViews();
}

ProjectViewImpl::~ProjectViewImpl() {
    unregisterBuiltInObjectViews();
}

/// returns NULL if no actions are required to enable service
Task* ProjectViewImpl::createServiceEnablingTask() {
    return new EnableProjectViewTask(this);
}

/// returns NULL if no actions are required to disable service
Task* ProjectViewImpl::createServiceDisablingTask() {
    return new DisableProjectViewTask(this, saveProjectOnClose);
}


void ProjectViewImpl::enable() {
    Project* pr = AppContext::getProject();
    connect(pr, SIGNAL(si_documentAdded(Document*)), SLOT(sl_onDocumentAdded(Document*)));
    connect(pr, SIGNAL(si_documentRemoved(Document*)), SLOT(sl_onDocumentRemoved(Document*)));
    connect(pr, SIGNAL(si_modifiedStateChanged()), SLOT(sl_onProjectModifiedStateChanged()));
    
    pr->setMainThreadModificationOnly(true);
    
    MWMDIManager* mdi = AppContext::getMainWindow()->getMDIManager();
    connect(mdi, SIGNAL(si_windowAdded(MWMDIWindow*)), SLOT(sl_onMDIWindowAdded(MWMDIWindow*)));
        
    assert(w == NULL);
    w = new ProjectViewWidget();
    
    assert(addExistingDocumentAction == NULL);
    addExistingDocumentAction = new QAction(QIcon(":ugene/images/advanced_open.png"), tr("Open as..."), w);
    addExistingDocumentAction->setObjectName("Open as");
    addExistingDocumentAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    addExistingDocumentAction->setShortcutContext(Qt::ApplicationShortcut);
    connect(addExistingDocumentAction, SIGNAL(triggered()), SLOT(sl_onAddExistingDocument()));

    saveSelectedDocsAction = new QAction(QIcon(":ugene/images/save_selected_documents.png"), tr("save_selected_modified_docs_action"), w);
    connect(saveSelectedDocsAction, SIGNAL(triggered()), SLOT(sl_onSaveSelectedDocs()));

    relocateDocumentAction = new QAction(tr("Relocate.."), w);
    relocateDocumentAction->setIcon(QIcon(":ugene/images/relocate.png"));
    connect(relocateDocumentAction, SIGNAL(triggered()), SLOT(sl_relocate()));

    saveCopyAction = new QAction(tr("Save a copy.."), w);
	saveCopyAction->setObjectName("Save a copy..");
    saveCopyAction->setIcon(QIcon(":ugene/images/save_copy.png"));
    connect(saveCopyAction, SIGNAL(triggered()), SLOT(sl_saveCopy()));

    initView();

    MainWindow* mw = AppContext::getMainWindow();
    MWDockManager* dm = mw->getDockManager();
    w->setObjectName("project_view");
    dm->registerDock(MWDockArea_Left, w, QKeySequence(Qt::ALT | Qt::Key_1));
    if (AppContext::getSettings()->getValue(SETTINGS_ROOT + "firstShow", true).toBool()) {
        dm->activateDock(w->objectName());
        AppContext::getSettings()->setValue(SETTINGS_ROOT + "firstShow", false);
    }
    
    QMenu* fm = mw->getTopLevelMenu(MWMENU_FILE);
    fm->insertAction(GUIUtils::findActionAfter(fm->actions(), ACTION_PROJECTSUPPORT__OPEN_PROJECT), addExistingDocumentAction);

    AppContextImpl::getApplicationContext()->setProjectView(this);

    updateMWTitle();
    const QList<Document*>& docs = AppContext::getProject()->getDocuments();
    foreach(Document* d, docs) {
        sl_onDocumentAdded(d);
    }
    foreach (MWMDIWindow* w, mdi->getWindows()) {
        sl_onMDIWindowAdded(w);
    }
}

void ProjectViewImpl::disable() {
    MainWindow* mw = AppContext::getMainWindow();
    if (w!=NULL) {
        saveWidgetState(w);
        saveGroupMode(projectTreeController->getModeSettings().groupMode);
    }	

    Project* pr = AppContext::getProject();
    pr->disconnect(this);

    //All these QObjects are autodeleted when 'w' is deleted;
    projectTreeController = NULL;
    objectViewController = NULL;
    addExistingDocumentAction = NULL;
//    addNewDocumentAction = NULL;

    AppContextImpl::getApplicationContext()->setProjectView(NULL);

    // close all views;
    MWMDIManager* mdiManager = mw->getMDIManager();
    const QList<GObjectViewWindow*> views = GObjectViewUtils::getAllActiveViews();
    foreach(GObjectViewWindow* view, views) {
        mdiManager->closeMDIWindow(view);
    }
    mw->setWindowTitle("");

    delete w;
    w = NULL;
}


void ProjectViewImpl::saveWidgetState(ProjectViewWidget* w) {
    QByteArray splitState = w->splitter->saveState();
    AppContext::getSettings()->setValue(SETTINGS_ROOT + "splitterState", splitState);
}

void ProjectViewImpl::restoreWidgetState(ProjectViewWidget* w) {
    QByteArray splitState = AppContext::getSettings()->getValue(SETTINGS_ROOT + "splitterState", QByteArray()).toByteArray();
    if (!splitState.isEmpty()) {
        w->splitter->restoreState(splitState);
    }
}

void ProjectViewImpl::initView() {
    //w->objectBox->setTitle(tr("documents_label"));
    //w->bookmarkBox->setTitle(tr("bookmarks_label"));
    
    assert(projectTreeController == NULL);
    ProjectTreeControllerModeSettings s;
    s.groupMode = getLastGroupMode();
    s.loadTaskProvider = this;
    s.markActive = true;
    s.activeFont.setWeight(QFont::Bold);
    projectTreeController = new ProjectTreeController(w, w->documentTreeWidget, s);
    projectTreeController->setObjectName("document_Filter_Tree_Controller");
    connect(projectTreeController, SIGNAL(si_onPopupMenuRequested(QMenu&)), SLOT(sl_onDocTreePopupMenuRequested(QMenu&)));
    connect(projectTreeController, SIGNAL(si_doubleClicked(GObject*)), SLOT(sl_onActivated(GObject*)));
    connect(projectTreeController, SIGNAL(si_returnPressed(GObject*)), SLOT(sl_onActivated(GObject*)));
    
    w->groupModeMenu->addAction(projectTreeController->getGroupByDocumentAction());
    w->groupModeMenu->addAction(projectTreeController->getGroupByTypeAction());
    w->groupModeMenu->addAction(projectTreeController->getGroupFlatAction());

    connect(w->nameFilterEdit, SIGNAL(textChanged(const QString&)), SLOT(sl_filterTextChanged(const QString&)));
    w->nameFilterEdit->installEventFilter(this);

    assert(objectViewController == NULL);
    objectViewController = new ObjectViewTreeController(w->viewTreeWidget);

    restoreWidgetState(w);	
}


bool ProjectViewImpl::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::Close) {
        GObjectViewWindow* ov = qobject_cast<GObjectViewWindow*>(obj);
        assert(ov);
        if (ov->isPersistent()) {
            saveViewState(ov, GObjectViewState::APP_CLOSING_STATE_NAME);
        }
    } else if (w!=NULL && w->nameFilterEdit == obj) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->modifiers() == Qt::NoModifier && keyEvent->key() == Qt::Key_Escape) {
                w->nameFilterEdit->clear();
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void ProjectViewImpl::saveViewState(GObjectViewWindow* v, const QString& stateName) {
    Project* p = AppContext::getProject();
    GObjectViewFactoryId id = v->getViewFactoryId();
    GObjectViewState* state = GObjectViewUtils::findStateInList(v->getViewName(), stateName, p->getGObjectViewStates());
    QVariantMap stateData = v->getObjectView()->saveState();
    if (stateData.isEmpty()) {
        return;
    }
    if (state == NULL) {
        state = new GObjectViewState(id, v->getViewName(), stateName, stateData);
        p->addGObjectViewState(state);
    } else {
        assert(state->getViewFactoryId() == id); //TODO: handle this error;
        state->setStateData(stateData);
    }
}

void ProjectViewImpl::sl_onMDIWindowAdded(MWMDIWindow* m) {
    GObjectViewWindow* v = qobject_cast<GObjectViewWindow*>(m);
    if (v) {
        v->installEventFilter(this);
        connect(v, SIGNAL(si_persistentStateChanged(GObjectViewWindow*)), SLOT(sl_onViewPersistentStateChanged(GObjectViewWindow*)));
    }
}

void ProjectViewImpl::sl_onDocumentRemoved(Document* doc) {
    doc->disconnect(this);
}

void ProjectViewImpl::sl_onSaveSelectedDocs() {
    const DocumentSelection* docSelection = getDocumentSelection();
    QList<Document*> modifiedDocs;
    foreach(Document* doc, docSelection->getSelectedDocuments()) {
        if (doc->isTreeItemModified()) {
            modifiedDocs.append(doc);
        }
    }
    if (!modifiedDocs.isEmpty()) {
        AppContext::getTaskScheduler()->registerTopLevelTask(new SaveMiltipleDocuments(modifiedDocs, false));
    }

}


void ProjectViewImpl::updateMWTitle() {
    Project* p  = AppContext::getProject();
    QString title = p->getProjectName();
    if (title.isEmpty()) {
        title = tr("unnamed_project_name");
    }
    if (p->isTreeItemModified()) {
        title+="*";
    }
    AppContext::getMainWindow()->setWindowTitle(title);
}

void ProjectViewImpl::sl_onProjectModifiedStateChanged() {
    updateMWTitle();
}

void ProjectViewImpl::sl_onViewPersistentStateChanged(GObjectViewWindow* v) {
    if (v->isPersistent()) {
        //add last saved state
        saveViewState(v, GObjectViewState::APP_CLOSING_STATE_NAME);
    } else {
        //remove all states
        QList<GObjectViewState*> states = GObjectViewUtils::findStatesByViewName(v->getViewName());
        foreach(GObjectViewState* s, states) {
            AppContext::getProject()->removeGObjectViewState(s);
        }
    }
}

void ProjectViewImpl::sl_onAddExistingDocument() {
    LastUsedDirHelper h;
    QString filter = DialogUtils::prepareDocumentsFileFilter(true);
    QString file = QFileDialog::getOpenFileName(QApplication::activeWindow(), tr("Select files to open..."), h.dir,  filter);
    if (file.isEmpty()) {
        return;
    }
    if (QFileInfo(file).exists()) {
        h.url = file;
    }
    QList<GUrl> urls; urls << GUrl(file, GUrl_File);
    QVariantMap hints;
    hints[ProjectLoaderHint_ForceFormatOptions] = true;
    Task* openTask = AppContext::getProjectLoader()->openWithProjectTask(urls, hints);
    if (openTask != NULL) {
        AppContext::getTaskScheduler()->registerTopLevelTask(openTask);	
    }

}

void ProjectViewImpl::sl_onDocTreePopupMenuRequested(QMenu& m) {
    buildViewMenu(m);
    emit si_onDocTreePopupMenuRequested(m);
}

#define MAX_DOCS_TO_OPEN_VIEWS 5
QList<Task*> ProjectViewImpl::createLoadDocumentTasks(const QList<Document*>& docs) const {
    bool openViews = docs.size() <= MAX_DOCS_TO_OPEN_VIEWS;
    QList<Task*> res;
    foreach(Document* doc, docs) {
        Task* t = NULL;
        if (openViews) {
            t = new LoadUnloadedDocumentAndOpenViewTask(doc);
        } else {
            t = new LoadUnloadedDocumentTask(doc);
        }
        res.append(t);
    }
    return res;
}

//TODO: selection could be modified before slot activation!
class OpenViewContext : public QObject {
public:
    OpenViewContext() : state(NULL), factory(NULL) {}
    OpenViewContext(QObject* p, const MultiGSelection& s, GObjectViewFactory* f) : QObject(p), selection(s), state(NULL), factory(f){}
    OpenViewContext(QObject* p, const GObjectViewState* s, GObjectViewFactory* f) : QObject(p), state(s), factory(f){}
    OpenViewContext(QObject* p, const QString& _viewName) : QObject(p), state(NULL), factory(NULL), viewName(_viewName){}

    MultiGSelection			selection;
    const GObjectViewState* state;
    GObjectViewFactory*		factory;
    QString					viewName;
};

class AddToViewContext : public QObject {
public:
    AddToViewContext(QObject* p, GObjectView* v, QList<GObject*> objs) : QObject(p), view(v) {
        foreach(GObject* o , objs) {
            objects.append(o);
        }
    }
    QPointer<GObjectView>		view;
    QList<QPointer<GObject> >	objects;
};

void ProjectViewImpl::sl_onActivated(GObject* o) {
    SAFE_POINT(o != NULL, "No double-clicked object found", );

    GObjectSelection os; os.addToSelection(o);
    MultiGSelection ms; ms.addSelection(&os);

    QMenu activeViewsMenu(tr("active_views_menu"), NULL);
    QList<QAction*> openActions;
    QList<GObjectViewFactory*> fs = AppContext::getObjectViewFactoryRegistry()->getAllFactories();
    foreach(GObjectViewFactory* f, fs) {
        QList<QAction*> tmp = selectOpenViewActions(f, ms, &activeViewsMenu);
        openActions<<tmp;
    }
    if (openActions.isEmpty()) { 
        if (o->isUnloaded()) {
            AppContext::getTaskScheduler()->registerTopLevelTask(new LoadUnloadedDocumentTask(o->getDocument()));
        }
        return;
    }
    if (openActions.size() == 1 ) {
        QAction* a = openActions.first();
        a->trigger();
        return;
    } 
    foreach(QAction* a, openActions) {
        activeViewsMenu.addAction(a);
    }
    activeViewsMenu.exec(QCursor::pos());
}


QList<QAction*> ProjectViewImpl::selectOpenViewActions(GObjectViewFactory* f, const MultiGSelection& ms, QObject* actionsParent) {
    QList<QAction*> res;

    //check if object is already displayed in some view.
    QList<GObjectViewWindow*> views;
    QList<MWMDIWindow*> windows = AppContext::getMainWindow()->getMDIManager()->getWindows();
    GObjectSelection* objectsSelection = (GObjectSelection*)ms.findSelectionByType(GSelectionTypes::GOBJECTS);
    if (objectsSelection!=NULL) {
        QSet<GObject*> objectsInSelection = objectsSelection->getSelectedObjects().toSet();
        foreach(MWMDIWindow* w, windows) {
            GObjectViewWindow* ov = qobject_cast<GObjectViewWindow*>(w);
            if (ov==NULL) {
                continue;
            }
            if (ov->getViewFactoryId() != f->getId()) {
                continue;
            }
            const QList<GObject*>& viewObjects = ov->getObjects();
            bool contains = false;
            foreach(GObject* o, viewObjects) {
                if (objectsInSelection.contains(o)) {
                    contains = true;
                    break;
                }
            }
            if (!contains) {
                continue;
            }
            QAction* action = new QAction(tr("activate_view_action_%1").arg(ov->getViewName()), actionsParent);		
            OpenViewContext* c = new OpenViewContext(action, ov->getViewName());
            action->setData(QVariant::fromValue((void*)c));
            connect(action, SIGNAL(triggered()), SLOT(sl_activateView()));
            res.append(action);
        }
    }

    //check if new view can be created
    if (f->canCreateView(ms)) {
        QAction* action = new QAction(tr("open_view_action_%1").arg(f->getName()), actionsParent);		
        OpenViewContext* c = new OpenViewContext(action, ms, f);
        action->setData(QVariant::fromValue((void*)c));
        connect(action, SIGNAL(triggered()), SLOT(sl_openNewView()));
        res.append(action);
    }
    //check saved state can be activated
    QList<GObjectViewState*> viewStates = GObjectViewUtils::selectStates(f, ms, AppContext::getProject()->getGObjectViewStates());
    foreach(GObjectViewState* s, viewStates) {
        QAction* action = new QAction(tr("open_state_%1_%2").arg(s->getViewName()).arg(s->getStateName()), actionsParent);		
        OpenViewContext* c = new OpenViewContext(action, s, f);
        action->setData(QVariant::fromValue((void*)c));
        connect(action, SIGNAL(triggered()), SLOT(sl_openStateView()));
        res.append(action);
    }

    return res;
}

void ProjectViewImpl::buildOpenViewMenu(const MultiGSelection& ms, QMenu* m) {
    QList<GObjectViewFactory*> fs = AppContext::getObjectViewFactoryRegistry()->getAllFactories();
    foreach(GObjectViewFactory* f, fs) {
        QList<QAction*> openActions = selectOpenViewActions(f, ms, m);
        if (openActions.isEmpty()) {
            continue;
        }
        if (openActions.size() == 1) {
            QAction *openAction = openActions.first();
            openAction->setObjectName("action_open_view");
            m->addAction(openAction);
            continue;
        }
        QMenu* submenu = new QMenu(tr("open_view_submenu_%1").arg(f->getName()), m);
        foreach (QAction* a, openActions) {
            submenu->addAction(a);
        }
        m->addAction(submenu->menuAction());
    }
}

void ProjectViewImpl::buildAddToViewMenu(const MultiGSelection& ms, QMenu* m) {
    MWMDIWindow* w = AppContext::getMainWindow()->getMDIManager()->getActiveWindow();
    if (w == NULL) {
        return;
    }
    GObjectViewWindow* ow = qobject_cast<GObjectViewWindow*>(w);
    if (ow == NULL) {
        return;
    }
    QList<GObject*> objects = SelectionUtils::getSelectedObjects(ms);
    if (objects.isEmpty()) {
        return;
    }
    foreach(GObject* obj, objects) {
        bool canBeAdded = ow->getObjectView()->canAddObject(obj);
        if (!canBeAdded) {
            return;
        }
    }
    QAction* action = new QAction(tr("add_to_view_action_%1").arg(ow->getViewName()), m);		
    AddToViewContext* ac = new AddToViewContext(action, ow->getObjectView(), objects);
    action->setData(QVariant::fromValue((void*)ac));
    action->setObjectName("action_add_view");
    connect(action, SIGNAL(triggered()), SLOT(sl_addToView()));
    m->addAction(action);
}

void ProjectViewImpl::buildRelocateMenu(QMenu* m) {
    const DocumentSelection* docSelection = getDocumentSelection();
    const GObjectSelection* objSelection = getGObjectSelection();
    if (!objSelection->isEmpty() || docSelection->getSelectedDocuments().size()!=1) {
        return;
    }
    Document* doc = docSelection->getSelectedDocuments().first();
    if (doc->isLoaded()) {
        DocumentFormatRegistry *dfr =  AppContext::getDocumentFormatRegistry();
        QList<DocumentFormatId> ids = dfr->getRegisteredFormats();
        QList<DocumentFormat*> allWritableFormats;
        foreach(DocumentFormatId id, ids){
            DocumentFormat *format = dfr->getFormatById(id);
            if (format->checkFlags(DocumentFormatFlag_SupportWriting)){
                allWritableFormats.append(format);
            }
        }
        foreach(DocumentFormat *f ,allWritableFormats){
            const QSet<GObjectType>& supportedObjectTypes = f->getSupportedObjectTypes();
            bool allObjectsWitable = true;
            foreach(GObject *gobj, doc->getObjects()){
                if (!supportedObjectTypes.contains(gobj->getGObjectType())){
                     allObjectsWitable = false;
                }
            }
            if(allObjectsWitable){
                m->addAction(saveCopyAction);
                return;
            }
        }
    } else {
        m->addAction(relocateDocumentAction);
    }
}

void ProjectViewImpl::buildViewMenu(QMenu& m) {
    QMenu* openViewMenu = new QMenu(tr("open_view_menu"), &m);
    QMenu* addToViewMenu= new QMenu(tr("add_to_view_menu"), &m);

    const DocumentSelection* docSelection = getDocumentSelection();
    MultiGSelection multiSelection;
    if (!getGObjectSelection()->isEmpty()) {
        multiSelection.addSelection(getGObjectSelection());
    }
    if (!docSelection->isEmpty()) {
        multiSelection.addSelection(getDocumentSelection());
    }

    buildOpenViewMenu(multiSelection, openViewMenu);
    openViewMenu->menuAction()->setObjectName("submenu_open_view");

    buildAddToViewMenu(multiSelection, addToViewMenu);
    addToViewMenu->menuAction()->setObjectName("submenu_add_view");

    addToViewMenu->setDisabled(addToViewMenu->isEmpty());
    m.insertMenu(m.actions().first(), addToViewMenu);

    openViewMenu->setDisabled(openViewMenu->isEmpty());
    m.insertMenu(m.actions().first(), openViewMenu);

    bool hasModifiedDocs = false;
    foreach(Document* doc, docSelection->getSelectedDocuments()) {
        if (doc->isTreeItemModified()) {
            hasModifiedDocs = true;
            break;
        }
    }

    buildRelocateMenu(&m);

    saveSelectedDocsAction->setEnabled(hasModifiedDocs);
    m.addAction(saveSelectedDocsAction);
}

void ProjectViewImpl::sl_activateView() {
    QAction* action  = (QAction*)sender();
    OpenViewContext* c = static_cast<OpenViewContext*>(action->data().value<void*>());
    assert(!c->viewName.isEmpty());
    GObjectViewWindow* ov = GObjectViewUtils::findViewByName(c->viewName);
    if (ov!=NULL) {
        AppContext::getMainWindow()->getMDIManager()->activateWindow(ov);
    }
}


void ProjectViewImpl::sl_openNewView() {
    QAction* action  = (QAction*)sender();
    OpenViewContext* c = static_cast<OpenViewContext*>(action->data().value<void*>());
    assert(c->factory->canCreateView(c->selection));
    AppContext::getTaskScheduler()->registerTopLevelTask(c->factory->createViewTask(c->selection));
}

void ProjectViewImpl::sl_addToView() {
    //TODO: create specialized action classes instead of using ->data().value<void*>() casts
    QAction* action  = (QAction*)sender();
    AddToViewContext* ac = static_cast<AddToViewContext*>(action->data().value<void*>());
    GObjectView* view = ac->view;
    if (view == NULL) {
        return;
    }
    foreach(GObject* o, ac->objects) {
        if (o!=NULL) {
            QString err  = view->addObject(o);
            if (!err.isEmpty()) {
                QMessageBox::critical(NULL, tr("error_adding_object_to_view_title"), err);
            }
        }
    }
}

void ProjectViewImpl::sl_openStateView() {
    QAction* action  = (QAction*)sender();
    OpenViewContext* c = static_cast<OpenViewContext*>(action->data().value<void*>());
    const GObjectViewState* state = c->state;
    assert(state);

    //todo: 70% of code duplication with ObjectViewTreeController::sl_activateView -> create util
    GObjectViewWindow* view = GObjectViewUtils::findViewByName(state->getViewName());
    if (view!=NULL) {
        assert(view->isPersistent());
        AppContext::getTaskScheduler()->registerTopLevelTask(view->getObjectView()->updateViewTask(state->getStateName(), state->getStateData()));
    } else {
        GObjectViewFactory* f = AppContext::getObjectViewFactoryRegistry()->getFactoryById(state->getViewFactoryId());
        assert(f!=NULL);
        AppContext::getTaskScheduler()->registerTopLevelTask(f->createViewTask(state->getViewName(), state->getStateData()));
    }
}


void ProjectViewImpl::sl_onDocumentAdded(Document* /*d*/) {
//    connect(d, SIGNAL(si_loadedStateChanged()), SLOT(sl_onDocumentLoadedStateChanged()));
}

void ProjectViewImpl::sl_filterTextChanged(const QString& t) {
    assert(projectTreeController!=NULL);
    ProjectTreeControllerModeSettings settings = projectTreeController->getModeSettings();
    settings.tokensToShow = t.split(' ', QString::SkipEmptyParts);
    projectTreeController->updateSettings(settings);
}

void ProjectViewImpl::sl_relocate() {
    const DocumentSelection* ds = getDocumentSelection();
    Document* d = ds->isEmpty() ? NULL : ds->getSelectedDocuments().first();
    if (d == NULL) {
        return;
    }
    if (d->isLoaded()) { //TODO: support loaded docs relocation?
        return;
    }
    LastUsedDirHelper h;
    h.url = QFileDialog::getOpenFileName(w, tr("Select new file location"), h.dir);
    if (h.url.isEmpty()) {
        return;
    }
	AppContext::getTaskScheduler()->registerTopLevelTask(new RelocateDocumentTask(d->getURL(), GUrl(h.url, GUrl_File)));    
}

void ProjectViewImpl::sl_saveCopy() {
    const DocumentSelection* ds = getDocumentSelection();
    Document* d = ds->isEmpty() ? NULL : ds->getSelectedDocuments().first();
    if (d == NULL) {
        return;
    }
    if (!d->isLoaded()) {
        return;
    }
    LastUsedDirHelper h;
    CopyDocumentDialogController dialog(d, w);
    int result = dialog.exec();
    if (result == QDialog::Accepted){
        h.url = dialog.getDocumentURL();
        if (h.url.isEmpty()) {
            return;
        }
        if (AppContext::getProject()->findDocumentByURL(h.url)) {
            QMessageBox::critical(w, tr("Error"), tr("Document with the same URL is added to the project. \n Remove it from the project first."));
            return;
        }
        bool addToProject = dialog.getAddToProjectFlag();
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(h.url));
        DocumentFormatRegistry *dfr =  AppContext::getDocumentFormatRegistry();
        DocumentFormat *df = dfr->getFormatById(dialog.getDocumentFormatId());
        if (iof == NULL) {
            return;
        }
        QList<GObject*> objs = d->getObjects();
        U2OpStatus2Log os;
        std::auto_ptr<Document> dp(df->createNewLoadedDocument(iof, h.url, os, d->getGHintsMap()));
        CHECK_OP(os, );
        foreach(GObject* go, objs){
            if(df->isObjectOpSupported(dp.get(), DocumentFormat::DocObjectOp_Add, go->getGObjectType())){
                GObject *cl = go->clone(dp->getDbiRef(), os);
                if (cl->getGObjectType() == GObjectTypes::MULTIPLE_ALIGNMENT){
                    QString name=QFileInfo(dialog.getDocumentURL()).baseName();
                    cl->setGObjectName(name);
                    cl->setModified(false);
                }
                dp->addObject(cl);
            }
        }
        foreach(GObject* o, dp->getObjects()) {
            GObjectUtils::updateRelationsURL(o, d->getURL(), h.url);
        }
        Document *d = dp.release();
        if (addToProject) {
            Project *p = AppContext::getProject();
            d->setModified(true);
            p->addDocument(d);
        }  
        SaveDocumentTask* t = new SaveDocumentTask(d, iof, h.url);
        if (!addToProject) {
            t->addFlag(SaveDoc_DestroyAfter);
        }
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    }
}


void ProjectViewImpl::highlightItem(Document* doc){
    assert(doc);
    projectTreeController->highlightItem(doc);
}
//////////////////////////////////////////////////////////////////////////
// Tasks

//EnableProjectViewTask

EnableProjectViewTask::EnableProjectViewTask(ProjectViewImpl* _pvi) 
: Task(tr("enable_project_view"), TaskFlag_NoRun), pvi(_pvi)
{
}

Task::ReportResult EnableProjectViewTask::report() {
    assert(AppContext::getProject()!=NULL);
    pvi->enable();
    return ReportResult_Finished;
}


DisableProjectViewTask::DisableProjectViewTask(ProjectViewImpl* _pvi, bool saveProjectOnClose) 
: Task(tr("disable_project_view"), TaskFlags_NR_FOSCOE), pvi(_pvi), saveProject(saveProjectOnClose)
{
}

void DisableProjectViewTask::prepare() {
    // TODO: this should be removed from here
    // because save project can be canceled by user.
    if (AppContext::getProject()->isTreeItemModified() && saveProject) {
        addSubTask(AppContext::getProjectService()->saveProjectTask(SaveProjectTaskKind_SaveProjectAndDocumentsAskEach));
    }
}


Task::ReportResult DisableProjectViewTask::report() {
    if (propagateSubtaskError()) {
        return ReportResult_Finished;
    }
    pvi->disable();
    return ReportResult_Finished;
}

}//namespace

