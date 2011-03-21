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

#ifndef _U2_PROJECT_SUPPORT_H_
#define _U2_PROJECT_SUPPORT_H_

#include "ui/ui_CreateNewProjectWidget.h"

#include <U2Core/ProjectModel.h>
#include <U2Core/ProjectService.h>
#include <assert.h>

#include <QtGui/QtGui>

namespace U2 {

#define SETTINGS_DIR QString("project_loader/")
#define RECENT_ITEMS_SETTINGS_NAME "recentItems"
#define RECENT_PROJECTS_SETTINGS_NAME "recentProjects"

class ProjectLoaderImpl : public ProjectLoader {
    Q_OBJECT
public:
    ProjectLoaderImpl();
	~ProjectLoaderImpl();

    virtual Task* openProjectTask(const QString& file, bool closeActiveProject);
    virtual Task* openProjectTask(const QList<GUrl>& urls, bool closeActiveProject);
    virtual Project* createProject(const QString& name, const QString& url, QList<Document*>& documents, QList<GObjectViewState*>& states);

    static QString getLastProjectURL();

private:
    void updateState();
	void updateRecentProjectsMenu();
    void prependToRecentProjects(const QString& pFile);
    void updateRecentItemsMenu();
    void prependToRecentItems(const QString& url);
    void rememberProjectURL();

private slots:
    void sl_newProject();
    void sl_newDocumentFromText();
    void sl_openProject();
	void sl_openRecentFile();
    void sl_openRecentProject();
    void sl_serviceStateChanged(Service* s, ServiceState prevState);
    void sl_documentAdded(Document* doc);
    void sl_documentStateChanged();
    void sl_projectURLChanged(const QString& oldURL);
    void sl_projectOpened();

	void sl_downloadRemoteFile();


// QT 4.5.0 bug workaround
    void sl_updateRecentItemsMenu();
    
private:
    
	QAction* newProjectAction;
	QAction* openProjectAction;
    QAction* downloadRemoteFileAction;
    QAction* newDocumentFromtext;
    QAction* separatorAction1;
	QAction* separatorAction2;

	QMenu* recentProjectsMenu;
    QMenu* recentItemsMenu;
};


//////////////////////////////////////////////////////////////////////////
/// Dialogs

//TODO: merge project dir & project name fields

class ProjectDialogController : public QDialog, public Ui::CreateNewProjectDialog {
    Q_OBJECT
public:
    enum Mode {New_Project, Save_Project};
    ProjectDialogController(Mode m, QWidget *p);

    void accept();
    void updateState();

protected:
    void keyPressEvent ( QKeyEvent * event );
    
private slots:
    void sl_folderSelectClicked();
    void sl_fileNameEdited(const QString&);
    void sl_projectNameEdited(const QString&);
private:
    void setupDefaults();
    bool fileEditIsEmpty;
};


}//namespace
#endif
