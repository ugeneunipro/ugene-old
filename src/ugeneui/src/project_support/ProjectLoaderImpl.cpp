#include "ProjectLoaderImpl.h"

#include <U2Gui/MainWindow.h>
#include <U2Core/Settings.h>
#include <U2Core/ServiceTypes.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>

#include <U2Misc/DialogUtils.h>

#include <U2Gui/CreateDocumentFromTextDialogController.h>
#include <U2Gui/DownloadRemoteFileDialog.h>

#include "ProjectTasksGui.h"
#include "ProjectImpl.h"

#include <QtGui/QAction>

namespace U2 {

/* TRANSLATOR U2::ProjectLoaderImpl */

//////////////////////////////////////////////////////////////////////////
/// ProjectLoaderImpl
//////////////////////////////////////////////////////////////////////////

ProjectLoaderImpl::ProjectLoaderImpl() {
    openProjectAction = newProjectAction = separatorAction1 = separatorAction2 = NULL;
	recentProjectsMenu = NULL;

	assert(AppContext::getProject() == NULL);
	assert(AppContext::getProjectLoader() == NULL);

	ServiceRegistry* sr = AppContext::getServiceRegistry();
	connect(sr, SIGNAL(si_serviceStateChanged(Service*, ServiceState)), SLOT(sl_serviceStateChanged(Service*, ServiceState)));
    
    newProjectAction = new QAction(QIcon(":ugene/images/project_new.png"), tr("&New project..."), this);
	newProjectAction->setObjectName(ACTION_PROJECTSUPPORT__NEW_PROJECT);
//    newProjectAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    newProjectAction->setShortcutContext(Qt::WindowShortcut);
    connect(newProjectAction, SIGNAL(triggered()), SLOT(sl_newProject()));

    newDocumentFromtext = new QAction(QIcon(), tr("New document from text..."), this);
    newDocumentFromtext->setObjectName("NewDocumentFromText");
    newDocumentFromtext->setShortcutContext(Qt::WindowShortcut);
    connect(newDocumentFromtext, SIGNAL(triggered()), SLOT(sl_newDocumentFromText()));

    openProjectAction = new QAction(QIcon(":ugene/images/project_open.png"), tr("Open"), this);
	openProjectAction->setObjectName(ACTION_PROJECTSUPPORT__OPEN_PROJECT);
    openProjectAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    openProjectAction->setShortcutContext(Qt::WindowShortcut);
    connect(openProjectAction, SIGNAL(triggered()), SLOT(sl_openProject()));
    
    downloadRemoteFileAction = new QAction(tr("Access remote database..."), this);
    downloadRemoteFileAction->setIcon(QIcon(":ugene/images/world_go.png"));
    connect(downloadRemoteFileAction, SIGNAL(triggered()), SLOT(sl_downloadRemoteFile()));

    separatorAction1  = new QAction("-", this);
    separatorAction1->setSeparator(true);

	separatorAction2 = new QAction("-", this);
	separatorAction2->setSeparator(true);

    //add load/close actions to menu and toolbar
    MainWindow* mw = AppContext::getMainWindow();
    QMenu* fileMenu = mw->getTopLevelMenu(MWMENU_FILE);

	recentProjectsMenu = new QMenu(tr("Recent Projects"));
	recentProjectsMenu->menuAction()->setObjectName(ACTION_PROJECTSUPPORT__RECENT_PROJECTS_MENU);
	updateRecentProjectsMenu();

    recentItemsMenu = new QMenu(tr("Recent Files"));
    recentItemsMenu->menuAction()->setObjectName("recent_docs_menu_action");
    updateRecentItemsMenu();

	QList<QAction*> actions;
    actions << newProjectAction << newDocumentFromtext << downloadRemoteFileAction 
        << openProjectAction << separatorAction1 <<  recentItemsMenu->menuAction() 
        << recentProjectsMenu->menuAction() << separatorAction2;
	
	fileMenu->insertActions(fileMenu->actions().first(), actions);

	QToolBar* tb = mw->getToolbar(MWTOOLBAR_MAIN);
	tb->addAction(newProjectAction);
	tb->addAction(openProjectAction);

	updateState();
}

ProjectLoaderImpl::~ProjectLoaderImpl() {
    assert(AppContext::getProject() == NULL);

	delete separatorAction1;
	delete separatorAction2;
    delete openProjectAction;
    delete newProjectAction;
	delete recentProjectsMenu;
	delete recentItemsMenu;

    openProjectAction = newProjectAction = separatorAction1 = separatorAction2 = NULL;
}


void ProjectLoaderImpl::updateState() {
	recentProjectsMenu->setDisabled(recentProjectsMenu->isEmpty());
}

#define MAX_RECENT_FILES 7


void ProjectLoaderImpl::sl_newProject() {
    QWidget *p = (QWidget*)AppContext::getMainWindow()->getQMainWindow();
    ProjectDialogController d(ProjectDialogController::New_Project,p);
	int rc = d.exec();
	AppContext::getSettings()->setValue(SETTINGS_DIR + "last_dir",d.projectFolderEdit->text());

	if (rc == QDialog::Rejected) {
		updateState();
		return;
	}

	QString fileName = d.projectFolderEdit->text() + "/" + d.projectFileEdit->text();
	if (!fileName.endsWith(PROJECTFILE_EXT)) {
		fileName.append(PROJECTFILE_EXT);
	}
	QFileInfo fi(fileName);
	if (fi.exists()) {
		QFile::remove(fileName);
	}

	QString projectName = d.projectNameEdit->text();
	AppContext::getTaskScheduler()->registerTopLevelTask(new OpenProjectTask(fileName, true, projectName));
}

void ProjectLoaderImpl::sl_openProject() {
    LastOpenDirHelper h;
    QString filter = DialogUtils::prepareDocumentsFileFilter(true);

    filter.append("\n"+tr("UGENE project file") + " (*" + PROJECTFILE_EXT + ")");

    QStringList files = QFileDialog::getOpenFileNames(QApplication::activeWindow(), tr("Select files to open..."), h.dir,  filter);

	if (files.isEmpty()) {
        return;
    }

    if (QFileInfo(files.first()).exists()) {
        h.url = files.first();
    }
    QList<GUrl> urls;
    foreach(QString file, files) {
        urls << GUrl(file, GUrl_File);
    }
    //updateRecentItemsMenu();
    Task* openTask = new OpenProjectTask(urls, true);
    connect(openTask, SIGNAL(si_stateChanged()), SLOT(sl_projectOpened()));
	AppContext::getTaskScheduler()->registerTopLevelTask(openTask);	
}

void ProjectLoaderImpl::sl_projectOpened() {
    Task *t = static_cast<Task*>(sender());
    if(t->isFinished()) {
        Project *p = AppContext::getProject();
        if(p->getDocuments().isEmpty()) {
            Task * cls = new CloseProjectTask();
            AppContext::getTaskScheduler()->registerTopLevelTask(cls);
        }
    }
}

void ProjectLoaderImpl::sl_openRecentProject()
{
    QAction *action = qobject_cast<QAction *>(sender());
    assert(action);
    QString url = action->data().toString();
    AppContext::getTaskScheduler()->registerTopLevelTask(new OpenProjectTask(url, true));	
}

void ProjectLoaderImpl::sl_openRecentFile() {
	QAction *action = qobject_cast<QAction *>(sender());
	assert(action);
	QString url = action->data().toString();
    AppContext::getTaskScheduler()->registerTopLevelTask(new OpenProjectTask(url, false));	
    prependToRecentItems(url);
#ifdef Q_OS_LINUX
    if(QString("4.5.0") == qVersion())
    {
        QTimer::singleShot(0,this,SLOT(sl_updateRecentItemsMenu()));
    }
    else
    {
#endif // Q_OS_LINUX
        updateRecentItemsMenu();
#ifdef Q_OS_LINUX
    }
#endif // Q_OS_LINUX
 }

void ProjectLoaderImpl::prependToRecentProjects(const QString& url) {
    assert(!url.isEmpty());
	QStringList recentFiles = AppContext::getSettings()->getValue(SETTINGS_DIR + RECENT_PROJECTS_SETTINGS_NAME).toStringList();
    recentFiles.removeAll(QString()); //remove all empty tokens if fount (a kind of cleanup)
	recentFiles.removeAll(url); // remove URL from the old position
	recentFiles.prepend(url); // make URL first
	while(recentFiles.size() > MAX_RECENT_FILES) {
		recentFiles.pop_back();
	}
	AppContext::getSettings()->setValue(SETTINGS_DIR + RECENT_PROJECTS_SETTINGS_NAME, recentFiles);
}

void ProjectLoaderImpl::updateRecentProjectsMenu() {
    assert(recentProjectsMenu!=NULL);
    recentProjectsMenu->clear();
    QStringList recentFiles =AppContext::getSettings()->getValue(SETTINGS_DIR + RECENT_PROJECTS_SETTINGS_NAME).toStringList();
    Project* p = AppContext::getProject();
    foreach (QString f, recentFiles) {
        if ((p==NULL || f != p->getProjectURL()) && !f.isEmpty()) {
            QAction* a = recentProjectsMenu->addAction(f, this, SLOT(sl_openRecentProject()));
            a->setData(f);
            a->setDisabled(!QFile::exists(f));
        }
    }
}


Task* ProjectLoaderImpl::openProjectTask(const QString& file, bool closeActiveProject) {
	return new OpenProjectTask(file, closeActiveProject);
}

Task* ProjectLoaderImpl::openProjectTask(const QList<GUrl>& urls, bool closeActiveProject) {
    return new OpenProjectTask(urls, closeActiveProject);
}

void ProjectLoaderImpl::sl_projectURLChanged(const QString& oldURL) {
    if (!oldURL.isEmpty()) {
        prependToRecentProjects(oldURL);
    }
    rememberProjectURL();
}

void ProjectLoaderImpl::rememberProjectURL() {
    Project* p = AppContext::getProject();
    QString url = p == NULL ? QString() : p->getProjectURL();
    if (!url.isEmpty()) {
        prependToRecentProjects(url);
    }
    updateRecentProjectsMenu();
}

void ProjectLoaderImpl::sl_serviceStateChanged(Service* s, ServiceState prevState) {
    Q_UNUSED(prevState);

	if (s->getType()!=Service_Project) {
		return;
	}
    if (s->isEnabled()) {
        Project* p = AppContext::getProject();
        connect(p, SIGNAL(si_projectURLChanged(const QString&)), SLOT(sl_projectURLChanged(const QString&)));
        connect(p, SIGNAL(si_documentAdded(Document*)), SLOT(sl_documentAdded(Document*)));
    } 
	rememberProjectURL();
    updateState();
}


QString ProjectLoaderImpl::getLastProjectURL() {
    QStringList recentFiles =AppContext::getSettings()->getValue(SETTINGS_DIR + RECENT_PROJECTS_SETTINGS_NAME).toStringList();
    if (!recentFiles.isEmpty()) {
        return recentFiles.first();
    }
    return QString();
}

void ProjectLoaderImpl::prependToRecentItems( const QString& url )
{
    assert(!url.isEmpty());
    QStringList recentFiles =AppContext::getSettings()->getValue(SETTINGS_DIR + RECENT_ITEMS_SETTINGS_NAME).toStringList();
    recentFiles.removeAll(url);
    recentFiles.prepend(url);
    while(recentFiles.size() > MAX_RECENT_FILES) {
        recentFiles.pop_back();
    }
    AppContext::getSettings()->setValue(SETTINGS_DIR + RECENT_ITEMS_SETTINGS_NAME, recentFiles);
    
}

// QT 4.5.0 bug workaround
void ProjectLoaderImpl::sl_updateRecentItemsMenu()
{
    updateRecentItemsMenu();
}

void ProjectLoaderImpl::updateRecentItemsMenu()
{
    assert(recentItemsMenu!=NULL);
    recentItemsMenu->clear();
    QStringList recentFiles =AppContext::getSettings()->getValue(SETTINGS_DIR + RECENT_ITEMS_SETTINGS_NAME).toStringList();
    recentItemsMenu->menuAction()->setEnabled(!recentFiles.isEmpty());
    Project* p = AppContext::getProject();
    foreach (QString f, recentFiles) {
        if ((p==NULL || f != p->getProjectURL()) && !f.isEmpty()) {
            QAction* a = recentItemsMenu->addAction(f, this, SLOT(sl_openRecentFile()));
            a->setData(f);
            a->setDisabled(!QFile::exists(f));
        }
    }

}

void ProjectLoaderImpl::sl_documentAdded( Document* doc )
{
    if (!doc->isModified()) {
        prependToRecentItems(doc->getURLString());
        updateRecentItemsMenu();
    } else {
        connect(doc, SIGNAL(si_modifiedStateChanged()), SLOT(sl_documentStateChanged()));
    }
}

void ProjectLoaderImpl::sl_documentStateChanged()
{
    Document* doc = qobject_cast<Document*>( QObject::sender() );
    if (doc != NULL) {
        if (!doc->isModified()) {
            prependToRecentItems(doc->getURLString());
            updateRecentItemsMenu();
        }
    }
}

void ProjectLoaderImpl::sl_newDocumentFromText(){
    QWidget *p = (QWidget*)AppContext::getMainWindow()->getQMainWindow();
    CreateDocumentFromTextDialogController *dialog = new CreateDocumentFromTextDialogController(p);
    dialog->exec();
    delete dialog;
}

void ProjectLoaderImpl::sl_downloadRemoteFile()
{
    QWidget *p = (QWidget*)(AppContext::getMainWindow()->getQMainWindow());
    DownloadRemoteFileDialog dlg(p);
    dlg.exec();
}


//////////////////////////////////////////////////////////////////////////
//ProjectDialogController
//////////////////////////////////////////////////////////////////////////
ProjectDialogController::ProjectDialogController(ProjectDialogController::Mode m, QWidget *p):QDialog(p) {
    setupUi(this);
    setModal(true);
    fileEditIsEmpty = false;
    QString lastDir =AppContext::getSettings()->getValue(SETTINGS_DIR + "last_dir", QString("")).toString();
    projectFolderEdit->setText(lastDir);

    if (m == Save_Project) {
        setWindowTitle(ProjectLoaderImpl::tr("Save project as"));
        createButton->setText(ProjectLoaderImpl::tr("Save"));
        titleLabel->setVisible(false);
        resize(width(), height() - titleLabel->pixmap()->height());
        frame->setFrameShape(QFrame::NoFrame);
        projectNameEdit->setText(AppContext::getProject()->getProjectName());
        QString url = AppContext::getProject()->getProjectURL();
        if (!url.isEmpty()) {
            QFileInfo fi(url);
            projectFileEdit->setText(fi.completeBaseName());
            projectFolderEdit->setText(fi.absolutePath());
        }
    } else {
        projectNameEdit->setText(ProjectLoaderImpl::tr("new_project_default_name"));
        projectFileEdit->setText(ProjectLoaderImpl::tr("new_project_default_file"));
    }
    //projectFolderEdit->setReadOnly(true);
    if (projectFileEdit->text().isEmpty()) {
        fileEditIsEmpty = true;
    }
    connect(folderSelectButton, SIGNAL(clicked()), SLOT(sl_folderSelectClicked()));
    connect(projectFileEdit, SIGNAL(textEdited(const QString&)), SLOT(sl_fileNameEdited(const QString&)));
    connect(projectNameEdit, SIGNAL(textEdited(const QString&)), SLOT(sl_projectNameEdited(const QString&)));
    updateState();
}

void ProjectDialogController::updateState() { 
    bool ready = true;
    
    const QString& folder = projectFolderEdit->text();
    const QString& file = projectFileEdit->text();
    const QString& name = projectNameEdit->text();
    
    //todo: improve check
    if (folder.isEmpty() || file.isEmpty() || name.isEmpty()) {
        ready = false;
    }
    createButton->setEnabled(ready);
}

void ProjectDialogController::keyPressEvent(QKeyEvent* event) {
    int key = event->key();
    if (event->modifiers() == Qt::NoModifier && (key == Qt::Key_Enter || key == Qt::Key_Return)) {
        createButton->animateClick();
    }
    QDialog::keyPressEvent(event);
    
}

void ProjectDialogController::sl_folderSelectClicked() {
    QString folder = QFileDialog::getExistingDirectory(this, tr("Choose directory"), projectFolderEdit->text());
    if (folder.isEmpty()) return;
    projectFolderEdit->setText(folder);
    updateState();
}

void ProjectDialogController::sl_fileNameEdited(const QString&) {
	//TODO: warn about overwrite
    fileEditIsEmpty = false;
    updateState();
}

void ProjectDialogController::sl_projectNameEdited(const QString& text) {
    if (fileEditIsEmpty) {
        projectFileEdit->setText(text);
    }
    updateState();
    
}

Project* ProjectLoaderImpl::createProject(const QString& name, const QString& url, QList<Document*>& documents, QList<GObjectViewState*>& states) {
    ProjectImpl* pi = new ProjectImpl(name, url, documents, states);
    return pi;
}


}//namespace

