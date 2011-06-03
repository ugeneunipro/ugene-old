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

#include "ProjectTasksGui.h"

#include "ProjectServiceImpl.h"
#include "ProjectImpl.h"
#include "ProjectLoaderImpl.h"

#include <AppContextImpl.h>

#include <U2Core/SaveDocumentTask.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/RemoveDocumentTask.h>
#include <U2Gui/OpenViewTask.h>
#include <U2Gui/UnloadDocumentTask.h>

#include <U2Core/DocumentUtils.h>

#include <U2Core/IOAdapter.h>
#include <U2Core/Log.h>
#include <U2Core/GUrlUtils.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/GHints.h>
#include <U2Gui/ProjectView.h>
#include <U2Core/GObject.h>
#include <U2Core/AppSettings.h>
#include <U2Core/L10n.h>

#include <U2Core/CMDLineUtils.h>

#include <U2Core/UnloadedObject.h>
#include <U2Core/GObjectUtils.h>

#include <U2Gui/ProjectParsing.h>

#include <QtXml/QDomDocument>
#include <QtGui/QMessageBox>

#include <QtCore/QMutex>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
///Close project
CloseProjectTask::CloseProjectTask() : Task(tr("close_project_task_name"), TaskFlags_NR_FOSCOE)
{
}

void CloseProjectTask::prepare() {
    if (AppContext::getProject()==NULL) {
        stateInfo.setError(  tr("error_no_active_project") );
        return;
    }
    /* TODO: this is done by project view. Need to cleanup this part! 
    addSubTask(new SaveProjectTask(SaveProjectTaskKind_SaveProjectAndDocumentsAskEach));
    */
    QList<Task*> tasks;
    /**/
    Project *pp = AppContext::getProject();
    if (pp->isTreeItemModified()) {
        tasks.append(AppContext::getProjectService()->saveProjectTask(SaveProjectTaskKind_SaveProjectAndDocumentsAskEach));
    }
    /**/
    ServiceRegistry* sr = AppContext::getServiceRegistry();
    QList<Service*> services = sr->findServices(Service_Project);
    assert(services.size() == 1);
    Service* projectService = services.first();
    tasks.append(sr->unregisterServiceTask(projectService));
    addSubTask(new MultiTask(tr("Save and close project"), tasks));
}


//////////////////////////////////////////////////////////////////////////
/// OpenProjectTask
OpenProjectTask::OpenProjectTask(const QString& _url, bool _closeActiveProject, const QString& _name) 
    : Task(tr("open_project_task_name"), TaskFlags_NR_FOSCOE), url(_url), name(_name), loadProjectTask(NULL), closeActiveProject(_closeActiveProject)
{
}

OpenProjectTask::OpenProjectTask(const QList<GUrl>& list, bool _closeActiveProject) 
    : Task(tr("open_project_task_name"), TaskFlags_NR_FOSCOE), urlList(list), loadProjectTask(NULL), closeActiveProject(_closeActiveProject) 
{
    if (urlList.isEmpty()) {
        return;
    }
    int howManyProjFiles = 0;
    foreach(const GUrl & url, urlList) {
        if(url.lastFileSuffix() == PROJECT_FILE_PURE_EXT) {
            howManyProjFiles++;
        }
    }
    if(howManyProjFiles == urlList.size()) { // only project files are given -> open first project, ignore other
        url = urlList.takeFirst().getURLString();
        foreach( const GUrl & url, urlList ) {
            coreLog.info(tr("Project file '%1' ignored").arg(url.getURLString()));
        }
    }
}

void OpenProjectTask::prepare() {
    if (url.endsWith(PROJECTFILE_EXT)) { // open a project
        QFileInfo f(url);
        if (f.exists() && !(f.isFile() && f.isReadable())) {
            stateInfo.setError(  tr("invalid_url%1").arg(url) );
            return;
        }
        
        // close current
        if (AppContext::getProject()!=NULL) {
            if(url == AppContext::getProject()->getProjectURL()) {
                coreLog.info(tr("Project already opened"));
                QMessageBox::critical(AppContext::getMainWindow()->getQMainWindow(),"UGENE", tr("Project already opened"));
                return ;
            }

            QMessageBox msgBox(AppContext::getMainWindow()->getQMainWindow());
            msgBox.setWindowTitle(tr("UGENE"));
            msgBox.setText(tr("New project can either be opened in a new window or replace the project in the existing. How would you like to open the project?"));
            QPushButton *newWindow = msgBox.addButton(tr("New Window"), QMessageBox::ActionRole);
            QPushButton *oldWindow = msgBox.addButton(tr("This Window"), QMessageBox::ActionRole);
            /*QPushButton *abort =*/ msgBox.addButton(QMessageBox::Abort);
            msgBox.exec();

            if(msgBox.clickedButton() == newWindow) {
                QStringList params =  CMDLineRegistryUtils::getPureValues(0);
                bool b = QProcess::startDetached(params.first(), QStringList() << url);
                if(!b) {
                    coreLog.error(tr("Failed to open new instance of UGENE"));
                }
                return;
            } else if(msgBox.clickedButton() == oldWindow) {
                if (!closeActiveProject) {
                    stateInfo.cancelFlag = true;
                    coreLog.info(tr("Stopped loading project: %1. Reason: active project found").arg(url));
                    return;
                }
                addSubTask(new CloseProjectTask());
            } else {
                return;
            }        
        }

        if (f.exists()) {
            loadProjectTask = new LoadProjectTask(url);
            addSubTask(loadProjectTask);
        } else {
            ProjectImpl* p =  new ProjectImpl(name, url);
            //addSubTask(new SaveProjectTask(SaveProjectTaskKind_SaveProjectOnly, p));
            addSubTask(new RegisterProjectServiceTask(p));
        }
    } else { // load a (bunch of) documents
        if (!url.isEmpty()) {
            urlList << GUrl(url);
        }
        Project* p = AppContext::getProject();
        if (!p) {
            // create anonymous project
            coreLog.info(tr("Creating new project"));
            p = new ProjectImpl("", "");
            Task* rpt = new RegisterProjectServiceTask(p);
            rpt->setSubtaskProgressWeight(0);
            addSubTask(rpt);
        }
#define MAX_DOCS_TO_OPEN_VIEWS 5
        bool openView = urlList.size() < MAX_DOCS_TO_OPEN_VIEWS;
        foreach(const GUrl& _url, urlList) {
            if (_url.lastFileSuffix() == PROJECT_FILE_PURE_EXT) {
                // skip extra project files
                coreLog.info(tr("Project file '%1' ignored").arg(_url.getURLString()));
                continue;
            }
            Document * doc = p->findDocumentByURL(_url);
            if (!doc) {
                QFileInfo fi(_url.getURLString());
                if(!fi.exists()){
                    stateInfo.setError(L10N::errorFileNotFound(_url));
                    continue;
                }
                QList<DocumentFormat*> fs = DocumentUtils::detectFormat(_url);
                if (fs.isEmpty()) {
                    stateInfo.setError(  tr("unsupported_document%1").arg(_url.fileName()) );
                    continue;
                }

                DocumentFormat* format = fs.first();
                assert(format);
                IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(_url));
                doc = new Document(format, iof, _url);
                GObjectType t = format->getSupportedObjectTypes().toList().first();
                if(GObjectTypes::getTypeInfo(t).type != GObjectTypes::UNKNOWN) {
                    Task * adt = new AddDocumentTask(doc);
                    adt->setSubtaskProgressWeight(0);
                    addSubTask(adt);
                }
                else {
                    coreLog.info(tr("Document wasn't added to project: unsupported format"));
                }
            } else{
                QWidget *p = AppContextImpl::getMainWindow()->getQMainWindow();
                coreLog.details("The document already in the project");
                QMessageBox::warning(p, tr("warning"), tr("The document already in the project"));
                if (doc->isLoaded()) {
                    const QList<GObject*>& docObjects = doc->getObjects();
                    QList<GObjectViewWindow*> viewsList = GObjectViewUtils::findViewsWithAnyOfObjects(docObjects);
                    if (!viewsList.isEmpty()) {
                        AppContext::getMainWindow()->getMDIManager()->activateWindow(viewsList.first());
                    }else{
                        AppContext::getProjectView()->highlightItem(doc);
                    }
                    coreLog.info(tr("The document is already opened: %1").arg(_url.fileName()));
                    continue;
                }else if(!doc->isLoaded() && AppContext::getProjectView()){
                    AppContext::getProjectView()->highlightItem(doc);
                    continue;
                }
            }
            if (openView) { // view is opened for first document only
                addSubTask(new LoadUnloadedDocumentAndOpenViewTask(doc));
            }
        }
    }
}

QList<Task*> OpenProjectTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if(subTask->hasError()) {
        if (subTask == loadProjectTask) {
            return res;
        }
        Project *p = AppContext::getProject();
        if(p == NULL) {
            return res;
        }
        Document * doc = p->findDocumentByURL(url);
        if(doc) {
            p->removeDocument(doc);
        }
    } 
    if (!isCanceled() && subTask == loadProjectTask && !loadProjectTask->hasError()) {
        Project* p =  loadProjectTask->detachProject();
        res.append(new RegisterProjectServiceTask(p));
    }
    return res;
}

//////////////////////////////////////////////////////////////////////////
/// Save project
SaveProjectTask::SaveProjectTask(SaveProjectTaskKind _k, Project* p, const QString& _url) 
    : Task(tr("save_project_task_name"), TaskFlag_NoRun), k(_k), proj(p), url(_url)
{
}

SaveProjectTask::~SaveProjectTask () {
}

void SaveProjectTask::prepare() {
    if (proj == NULL) {
        proj = AppContext::getProject();
    }
    assert(proj!=NULL);
    if (url.isEmpty()) {
        url = proj->getProjectURL();
    }
    QList<Task *> ssTasks;
    if (url.isEmpty() && (!proj->getGObjectViewStates().isEmpty() || proj->getDocuments().size() > 0)) {
        //ask if to save project
        QWidget* w  = AppContext::getMainWindow()->getQMainWindow();
        int code = QMessageBox::question(w, tr("UGENE"), tr("Save current project?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
        if (code == QMessageBox::Yes) {
            ProjectDialogController d(ProjectDialogController::Save_Project, w);
            int rc = d.exec();
            if (rc == QDialog::Accepted) {
                AppContext::getProject()->setProjectName(d.projectNameEdit->text());
                url = d.projectFolderEdit->text() + "/" + d.projectFileEdit->text();
                if (!url.endsWith(PROJECTFILE_EXT)) {
                    url.append(PROJECTFILE_EXT);
                }
                AppContext::getProject()->setProjectURL(url);
            } else {
                cancel();
                return;
            }
        } else if (code == QMessageBox::Cancel) {
            cancel();
            return;
        }
    }

    if (k!=SaveProjectTaskKind_SaveProjectOnly) {
        QList<Document*> modifiedDocs = SaveMiltipleDocuments::findModifiedDocuments(AppContext::getProject()->getDocuments());
        if (!modifiedDocs.isEmpty()) {
            ssTasks.append(new SaveMiltipleDocuments(modifiedDocs, k == SaveProjectTaskKind_SaveProjectAndDocumentsAskEach));               
        }
    }
    if (!url.isEmpty()) {
        ssTasks.append(new SaveOnlyProjectTask(proj, url));
    }
    if (!ssTasks.isEmpty()) {
        addSubTask(new MultiTask(tr("Save documents, remove phantom docs, save  project"), ssTasks));
    }
}

Task::ReportResult SaveProjectTask::report() {
    return Task::ReportResult_Finished;
}



//////////////////////////////////////////////////////////////////////////
/// SaveOnlyProjectTask

SaveOnlyProjectTask::SaveOnlyProjectTask(Project* p, const QString& _url)
    :Task(tr("Save project"), TaskFlag_NoRun), sub(NULL), proj(p), url(_url){
        lock = NULL;
        //assert(!url.isEmpty());
}

SaveOnlyProjectTask::~SaveOnlyProjectTask(){
    assert(lock == NULL);
}

void SaveOnlyProjectTask::prepare(){
    if (proj == NULL) {
        proj = AppContext::getProject();
    }
    assert(proj!=NULL);
    if (url.isEmpty()) {
        url = proj->getProjectURL();
    }

    foreach(Document *d, proj->getDocuments()){
        if (d->getURL().isLocalFile()) {
            QFile pathToDoc(d->getURLString());
            if(!pathToDoc.exists()){
                phantomDocs.append(d);
            }
        }
    }
    if (!phantomDocs.isEmpty()){   
        sub = new RemoveMultipleDocumentsTask(proj, phantomDocs, false, false);
        addSubTask(sub);
    }else{
        _run();
    }
}

QList<Task*> SaveOnlyProjectTask::onSubTaskFinished(Task* subTask){
    QList<Task*> ret;
    if (!isCanceled() && subTask == sub){
        _run();
    }
    return ret;
}

void SaveOnlyProjectTask::_run(){
    lock = new StateLock(getTaskName(), StateLockFlag_LiveLock);
    proj->lockState(lock);

    coreLog.info(tr("Saving project %1").arg(url));
    ProjectFileUtils::saveProjectFile(stateInfo, proj, url);
}

Task::ReportResult SaveOnlyProjectTask::report(){
    if (!stateInfo.hasError() && url == proj->getProjectURL()) {
        proj->setModified(false);
    }
    proj->unlockState(lock);
    delete lock;
    lock = NULL;
    return Task::ReportResult_Finished;
}

//////////////////////////////////////////////////////////////////////////
/// LoadProjectTask
LoadProjectTask::LoadProjectTask(const QString& _url) 
    : Task(tr("load_project_task_name"), TaskFlag_None), proj(NULL), url(_url)
{
    xmlDoc = new QDomDocument();
}

LoadProjectTask::~LoadProjectTask() {
    delete proj;
    delete xmlDoc;
}

void LoadProjectTask::run() {
    coreLog.details(tr("Loading project from: %1").arg(url));
    ProjectFileUtils::loadXMLProjectModel(url, stateInfo, *xmlDoc, version);
}

Task::ReportResult LoadProjectTask::report() {
    if (!stateInfo.hasError()) {
        ProjectParserRegistry *ppr = ProjectParserRegistry::instance();
        ProjectParser *parser = ppr->getProjectParserByVersion(version);
        if(parser == NULL){
            coreLog.info(tr("Unable to detect parser for project"));
            setError(tr("failed to parse project file %1").arg(url));
        }else{
            proj = parser->createProjectFromXMLModel(url, *xmlDoc, stateInfo);
        }
        if (proj!=NULL) {
            coreLog.info(tr("Project loaded: %1").arg(url));
        } 
    }
    return Task::ReportResult_Finished;
}




RegisterProjectServiceTask::RegisterProjectServiceTask(Project* _proj) 
    : Task(tr("Register project"), TaskFlag_NoRun), proj(_proj)
{
}

void RegisterProjectServiceTask::prepare() {
    ProjectServiceImpl* ps = new ProjectServiceImpl(proj);
    addSubTask(AppContext::getServiceRegistry()->registerServiceTask(ps));
}

ExportProjectTask::ExportProjectTask(const QString& _destinationDir, const QString& _file, bool _compress):
Task(tr("Export project task"), TaskFlags_NR_FOSCOE), compress(_compress), destinationDir(_destinationDir), projectFile(_file)
{
    assert(!destinationDir.isEmpty());
    setVerboseLogMode(true);
}

void ExportProjectTask::prepare(){
    Project *pr = AppContext::getProject();

    QList<Document*> docList = pr->getDocuments();
    foreach(Document* doc, docList){
        if(doc->getURL().isEmpty()){
            coreLog.error(tr("One of the documents has empty URL"));
            return;
        }
    }

    QString error;
    QDir dDir(GUrlUtils::prepareDirLocation(destinationDir, error));
    if (!error.isEmpty()){
        coreLog.error(error);
        setError(error);
        return;
    }

    QMap<QString, QString> urlRemap;
    foreach(Document* doc, docList){
        QString origPath = doc->getURLString();
        IOAdapterId id = doc->getIOAdapterFactory()->getAdapterId();
        if (id == BaseIOAdapters::LOCAL_FILE || id == BaseIOAdapters::GZIPPED_LOCAL_FILE){
            QFile f(origPath);
            QFileInfo fi(f);
            QString resultPath = destinationDir + "/" + fi.fileName();
            if (resultPath != origPath && !f.copy(resultPath)){
                if(QFile::exists(resultPath)) {
                    coreLog.error(tr("Error during coping documents: file already exist"));
                } else {
                    coreLog.error(tr("Error during coping documents"));
                }
                return;
            }
            urlRemap[origPath] = resultPath;
        }
    }
    if(pr->getProjectName().isEmpty()){
        QFileInfo fi(projectFile);
        pr->setProjectName(fi.baseName());
    }
 
    ProjectFileUtils::saveProjectFile(stateInfo, pr, destinationDir + "/" + projectFile, urlRemap);
}

void GTest_LoadProject::init(XMLTestFormat*, const QDomElement& el) {
    addTaskResource(TaskResourceUsage( RESOURCE_PROJECT, 1, true));
    projContextName = el.attribute("index");
    if(!el.attribute("load_from_temp").isEmpty()){
        url = env->getVar("TEMP_DATA_DIR") + "/" + el.attribute("url");
    }else{
        url = env->getVar("COMMON_DATA_DIR") + "/" + el.attribute("url");
    }
}

void GTest_LoadProject::prepare(){
    QList<Task*> tasks;
    Project *previousProject = AppContext::getProject();
    Task *tt;
    ServiceRegistry* sr = AppContext::getServiceRegistry();
    QList<Service*> services = sr->findServices(Service_Project);
    int servSize = services.size();
    assert(servSize <= 1);
    if(servSize == 1){
        if(previousProject->metaObject() != NULL){
            tt  = new CloseProjectTask();
            tasks.append(tt);
        }
    } else if(previousProject != NULL) {
        AppContextImpl::getApplicationContext()->setProject(NULL);
    }
    contextAdded = false;

    loadTask = new LoadProjectTask(url);
    tasks.append(loadTask);
    mt = new MultiTask(tr("Unload previous project, then load new"), tasks);
    addSubTask(mt);
}

Task::ReportResult GTest_LoadProject::report() {
    if (loadTask!=NULL && loadTask->hasError()) {
        stateInfo.setError( loadTask->getError());
    } else if (!projContextName.isEmpty()) {
        addContext(projContextName, loadTask->getProject());
        contextAdded = true;
    }
    return ReportResult_Finished;
}

QList<Task*> GTest_LoadProject::onSubTaskFinished( Task* subTask ){
    QList<Task*> subTasks;
    if (subTask->hasError()) {
        return subTasks;
    }
    if (subTask == mt) {
        Project *p = loadTask->getProject();
        if(p != NULL){
            AppContextImpl::getApplicationContext()->setProject(p);
        }
    }
    return subTasks;
}

void GTest_LoadProject::cleanup(){
    AppContextImpl::getApplicationContext()->setProject(NULL);
}


QList<XMLTestFactory*> ProjectTests::createTestFactories(){
    QList<XMLTestFactory*> res;
    res.append(GTest_LoadProject::createFactory());
    res.append(GTest_ExportProject::createFactory());
    res.append(GTest_UnloadProject::createFactory());
    res.append(GTest_LoadDocumentFromProject::createFactory());
    return res;
}

void GTest_ExportProject::init(XMLTestFormat* tf, const QDomElement& el) {
    Q_UNUSED(tf);
    
    exportTask = NULL;
    url = env->getVar("TEMP_DATA_DIR") + el.attribute("url");
}

void GTest_ExportProject::prepare(){
    QDir qd(url);
    if(qd.exists(url)){
        if(!removeDir(qd)){
            stateInfo.setError(tr("GTest_ExportProject::prepare unable to clean directory for exporting"));
            return;
        }
    }   
    if(!qd.mkpath(url)){
        stateInfo.setError(tr("GTest_ExportProject::prepare unable to create directory for exporting"));
        return;
    }
    exportTask = new ExportProjectTask(url, "");
    addSubTask(exportTask);
}

Task::ReportResult GTest_ExportProject::report(){
    if(exportTask!=NULL){
        if (exportTask->hasError()) {
            stateInfo.setError(exportTask->getError());
        }
    }
    return ReportResult_Finished;
}

void GTest_ExportProject::cleanup(){
    QDir qd(url);
    if(qd.exists(url)){
        if(!removeDir(QDir(url))){
            coreLog.info(tr("GTest_ExportProject::cleanup unable to delete exported files"));
        }
    }
}

bool GTest_ExportProject::removeDir( const QDir &aDir ){
    bool has_err = false;
    if (aDir.exists())//QDir::NoDotAndDotDot
    {
        QFileInfoList entries = aDir.entryInfoList(QDir::NoDotAndDotDot | 
            QDir::Dirs | QDir::Files);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.canonicalFilePath();
            if (entryInfo.isDir()){
                QDir aaa(path);
                has_err = removeDir( aaa );
            }else{
                QFile file(path);
                if (!file.remove())
                    has_err = true;
            }
        }
        if (!aDir.rmdir(aDir.absolutePath()))
            has_err = true;
    }
    return(has_err);
}
void GTest_UnloadProject::init(XMLTestFormat* tf, const QDomElement& el) {
    Q_UNUSED(tf);

    QString packedList = el.attribute("documents");
    if(packedList.isEmpty()){
        //document list can be empty!
        return;
    }
    unloadDocList = packedList.split(",");
}

void GTest_UnloadProject::prepare(){
    Project *p = AppContext::getProject();
    if(p != NULL){
        foreach(QString doc, unloadDocList){
            removeContext(doc);
        }
        AppContextImpl::getApplicationContext()->setProject(NULL);
    }
}

void GTest_LoadDocumentFromProject::init(XMLTestFormat* tf, const QDomElement& el) {
    Q_UNUSED(tf);

    documentFileName = el.attribute("document");
    contextAdded = false;
}

void GTest_LoadDocumentFromProject::prepare(){
    Project *pr = AppContext::getProject();
    if(pr == NULL){
        stateInfo.setError(tr("No project loaded"));
        return;
    }
    if(documentFileName.isEmpty()){
        stateInfo.setError(tr("Document name to load is empty"));
        return;
    }
    foreach(Document *doc, pr->getDocuments()){
        QFileInfo fi(doc->getURLString());
        QString tmp  = fi.fileName();
        if (fi.fileName() == documentFileName){
            if(!doc->isLoaded()){
                loadTask = new LoadUnloadedDocumentTask(doc);
                addSubTask(loadTask);
            }
            loadedDoc = doc;
        }
    }
    if (loadedDoc == NULL){
        stateInfo.setError(tr("File %1 not found in project").arg(documentFileName));
        return;
    }
}

void GTest_LoadDocumentFromProject::cleanup(){
    QObject *o = getContext(documentFileName);
    if(contextAdded && o != NULL){
        removeContext(documentFileName);
        if(loadedDoc->isLoaded()){
            AppContext::getTaskScheduler()->registerTopLevelTask(new UnloadDocumentTask(loadedDoc, false));
        }
    }
}

Task::ReportResult GTest_LoadDocumentFromProject::report(){
    if(loadTask!=NULL){
        if(!loadTask->hasError()){
            addContext(documentFileName, loadedDoc);
            contextAdded = true;
        }
    }
    return ReportResult_Finished;
}

} //namespace
