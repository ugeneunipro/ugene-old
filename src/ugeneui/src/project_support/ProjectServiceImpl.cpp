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

#include "ProjectServiceImpl.h"

#include "ProjectImpl.h"
#include "ProjectLoaderImpl.h"
#include "ProjectTasksGui.h"

#include <project_support/ExportProjectDialogController.h>

#include <AppContextImpl.h>

#include <U2Core/L10n.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/MainWindow.h>
#include <U2Core/Settings.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/DialogUtils.h>

#include <QtGui/QMenu>
#include <QtGui/QToolBar>

namespace U2 {

#define SETTINGS_DIR QString("project_loader/")

ProjectServiceImpl::ProjectServiceImpl(Project* _pr) 
: ProjectService(tr("project_sname"), tr("project_sdesc"))
{
    saveAction = NULL;
    saveAsAction = NULL;
    closeProjectAction = NULL;
    exportProjectAction = NULL;

    pr = _pr;
    assert(pr!=NULL);
}

ProjectServiceImpl::~ProjectServiceImpl() {
    delete pr;
}

Task* ProjectServiceImpl::saveProjectTask(SaveProjectTaskKind k) {
    return new SaveProjectTask(k);
}

Task* ProjectServiceImpl::closeProjectTask() {
    return new CloseProjectTask();
}

void ProjectServiceImpl::enableSaveAction(bool e) {
    if(saveAction) {
        saveAction->setEnabled(e);
    }
}

Task* ProjectServiceImpl::createServiceEnablingTask() {
    return new ProjectServiceEnableTask(this);
}

Task* ProjectServiceImpl::createServiceDisablingTask() {
    return new ProjectServiceDisableTask(this);
}

void ProjectServiceImpl::sl_closeProject() {
    AppContext::getTaskScheduler()->registerTopLevelTask(closeProjectTask());
}

void ProjectServiceImpl::sl_save() {
    AppContext::getTaskScheduler()->registerTopLevelTask(saveProjectTask(SaveProjectTaskKind_SaveProjectAndDocuments));
}

void ProjectServiceImpl::sl_saveAs() {
    QWidget *p = qobject_cast<QWidget*>(AppContext::getMainWindow()->getQMainWindow());
    ProjectDialogController d(ProjectDialogController::Save_Project, p);
    int rc = d.exec();
    if (rc == QDialog::Rejected) {
        return;
    }

    QDir dir(d.projectFolderEdit->text());

    U2OpStatus2Log os;
    QString fullPath = GUrlUtils::prepareDirLocation(d.projectFolderEdit->text(), os);

    if (fullPath.isEmpty()) {
        QMessageBox::critical(0, L10N::errorTitle(), os.getError());
        return;
    }

    AppContext::getSettings()->setValue(SETTINGS_DIR + "last_dir", fullPath, true);
    
    AppContext::getProject()->setProjectName(d.projectNameEdit->text());
    
    QString fileName = fullPath + "/" + d.projectFileEdit->text();
    if (!fileName.endsWith(PROJECTFILE_EXT)) {
        fileName.append(PROJECTFILE_EXT);
    }
    AppContext::getProject()->setProjectURL(fileName);

    AppContext::getTaskScheduler()->registerTopLevelTask(saveProjectTask(SaveProjectTaskKind_SaveProjectAndDocuments));
}

void ProjectServiceImpl::sl_exportProject(){
    Project* p = getProject();
    QString pUrl = p->getProjectURL();
    QString projectFileName = pUrl.isEmpty() ? QString() : QFileInfo(pUrl).fileName();
    ExportProjectDialogController dialog(AppContext::getMainWindow()->getQMainWindow(), projectFileName);
    dialog.exec();
    if (dialog.result() == QDialog::Accepted){
        Task *t = new ExportProjectTask(dialog.getDirToSave(), dialog.getProjectFile(), dialog.useCompression());
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
    }
}
//////////////////////////////////////////////////////////////////////////
/// Service tasks

ProjectServiceEnableTask::ProjectServiceEnableTask(ProjectServiceImpl* _psi) 
: Task(tr("project_enable_task"), TaskFlag_NoRun), psi(_psi)
{}

Task::ReportResult ProjectServiceEnableTask::report() {
    AppContextImpl::getApplicationContext()->setProjectService(psi);
    AppContextImpl::getApplicationContext()->setProject(psi->getProject());

    assert(psi->saveAction == NULL && psi->closeProjectAction == NULL);

    psi->saveAction = new QAction(QIcon(":ugene/images/project_save.png"), tr("&Save all"), psi);
    psi->saveAction->setObjectName(ACTION_PROJECTSUPPORT__SAVE_PROJECT);
    psi->saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    psi->saveAction->setShortcutContext(Qt::WindowShortcut);
    connect(psi->saveAction, SIGNAL(triggered()), psi, SLOT(sl_save()));

    psi->saveAsAction = new QAction(tr("Save project &as..."), psi);
	psi->saveAsAction->setObjectName(ACTION_PROJECTSUPPORT__SAVE_AS_PROJECT);
    connect(psi->saveAsAction, SIGNAL(triggered()), psi, SLOT(sl_saveAs()));

    psi->closeProjectAction = new QAction(tr("&Close project"), psi);
    psi->closeProjectAction->setObjectName(ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    psi->closeProjectAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    psi->closeProjectAction->setShortcutContext(Qt::WindowShortcut);
    connect(psi->closeProjectAction, SIGNAL(triggered()), psi, SLOT(sl_closeProject()));

    psi->exportProjectAction = new QAction(tr("Export project..."), psi);
    psi->exportProjectAction->setObjectName(ACTION_PROJECTSUPPORT__EXPORT_PROJECT);
    connect(psi->exportProjectAction, SIGNAL(triggered()), psi, SLOT(sl_exportProject()));

    MainWindow* mw = AppContext::getMainWindow();
    QMenu* fileMenu = mw->getTopLevelMenu(MWMENU_FILE);
    QAction* beforeAction= GUIUtils::findActionAfter(fileMenu->actions(), ACTION_PROJECTSUPPORT__OPEN_AS);
    fileMenu->insertAction(beforeAction,  psi->saveAction);
    fileMenu->insertAction(beforeAction,  psi->saveAsAction);
    fileMenu->insertAction(beforeAction,  psi->exportProjectAction);
        fileMenu->insertAction(beforeAction,  psi->closeProjectAction);
    
    QToolBar* tb = mw->getToolbar(MWTOOLBAR_MAIN);
    beforeAction= GUIUtils::findActionAfter(tb->actions(), ACTION_PROJECTSUPPORT__OPEN_PROJECT);
    tb->insertAction(beforeAction,  psi->saveAction);
    
    return ReportResult_Finished;
}


ProjectServiceDisableTask::ProjectServiceDisableTask(ProjectServiceImpl* _psi) 
: Task(tr("project_disable_task"), TaskFlag_NoRun), psi(_psi)
{}

Task::ReportResult ProjectServiceDisableTask::report() {
    AppContextImpl::getApplicationContext()->setProject(NULL);
    AppContextImpl::getApplicationContext()->setProjectService(NULL);

    delete psi->saveAction;
    psi->saveAction = NULL;
    
    delete psi->closeProjectAction;
    psi->closeProjectAction = NULL;

    return ReportResult_Finished;
}

} //namespace
