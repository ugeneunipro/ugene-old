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

#ifndef _U2_PROJECT_SUPPORT_H_
#define _U2_PROJECT_SUPPORT_H_

#include "ui/ui_CreateNewProjectWidget.h"
#include "ui/ui_SaveProjectDialog.h"

#include <U2Core/ProjectModel.h>
#include <U2Core/ProjectService.h>
#include <assert.h>

#include <QtGui/QtGui>

namespace U2 {

#define SETTINGS_DIR QString("project_loader/")
#define RECENT_ITEMS_SETTINGS_NAME "recentItems"
#define RECENT_PROJECTS_SETTINGS_NAME "recentProjects"

class DocumentProviderTask;
class FormatDetectionResult;

class ProjectLoaderImpl : public ProjectLoader {
    Q_OBJECT
public:
    ProjectLoaderImpl();
	
    virtual Task* openWithProjectTask(const QList<GUrl>& urls, const QVariantMap& hints = QVariantMap());

    virtual Task* createNewProjectTask(const GUrl& url = GUrl());

    virtual Task* createProjectLoadingTask(const GUrl& url, const QVariantMap& hints = QVariantMap());
    
    virtual Project* createProject(const QString& name, const QString& url, QList<Document*>& documents, QList<GObjectViewState*>& states);

    virtual QAction* getAddExistingDocumentAction(){return addExistingDocumentAction;};
    
    static QString getLastProjectURL();

private:

    void updateState();
	void updateRecentProjectsMenu();
    void prependToRecentProjects(const QString& pFile);
    void updateRecentItemsMenu();
    void prependToRecentItems(const QString& url);
    void rememberProjectURL();
    bool processHints(FormatDetectionResult& dr);

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
    void sl_onAddExistingDocument();
    
	void sl_downloadRemoteFile();
    void sl_searchGenbankEntry();


// QT 4.5.0 bug workaround
    void sl_updateRecentItemsMenu();
    
private:
    QAction* addExistingDocumentAction;
	QAction* newProjectAction;
	QAction* openProjectAction;
    QAction* downloadRemoteFileAction;
    QAction* searchGenbankEntryAction;
    QAction* newDocumentFromtext;
    QAction* separatorAction1;
	QAction* separatorAction2;

	QMenu* recentProjectsMenu;
    QMenu* recentItemsMenu;
};


//////////////////////////////////////////////////////////////////////////
/// Dialogs

//TODO: merge project dir & project name fields

class SaveProjectDialogController : public QDialog, public Ui::SaveProjectDialog {
    Q_OBJECT
public:
    SaveProjectDialogController(QWidget *p);
public slots:
    void sl_clicked(QAbstractButton *button);
};

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
    QPushButton* createButton;
};


class AD2P_DocumentInfo {
public:
    AD2P_DocumentInfo() : iof (NULL), openView(false), loadDocuments(false) {}
    GUrl                    url;
    DocumentFormatId        formatId;
    IOAdapterFactory*       iof;
    QVariantMap             hints;
    bool                    openView;
    bool                    loadDocuments;
};

class AD2P_ProviderInfo {
public:
    AD2P_ProviderInfo() : dp (NULL), openView(false){}
    DocumentProviderTask*   dp;
    bool                    openView;
};

class AddDocumentsToProjectTask: public Task {
    Q_OBJECT
public:
    AddDocumentsToProjectTask(const QList<AD2P_DocumentInfo>& docsInfo, const QList<AD2P_ProviderInfo>& providersInfo);
    ~AddDocumentsToProjectTask();

    virtual QList<Task*> onSubTaskFinished(Task* subTask);
private:
    QList<Task*> prepareLoadTasks();

    QList<AD2P_DocumentInfo> docsInfo;
    QList<AD2P_ProviderInfo> providersInfo;
    bool loadTasksAdded;
};

class OpenWithProjectTask : public Task {
    Q_OBJECT
public:
    OpenWithProjectTask(const QStringList& urls);
    void prepare();
private:
    QList<GUrl> urls;
};


}//namespace
#endif
