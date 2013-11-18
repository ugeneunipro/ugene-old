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

#include "WorkflowDesignerPlugin.h"
#include "WorkflowViewController.h"
#include "WorkflowDocument.h"
#include "WorkflowSettingsController.h"
#include "WorkflowSamples.h"
#include "tasks/ReadAssemblyTask.h"

#include "library/CoreLib.h"
#include "library/IncludedProtoFactoryImpl.h"

#include <util/SaveSchemaImageUtils.h>
#include <util/DatasetsCountValidator.h>

#include <U2Lang/IncludedProtoFactory.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowTasksRegistry.h>

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/Settings.h>
#include <U2Core/Task.h>
#include <U2Core/ServiceTypes.h>

#include <U2Core/CMDLineRegistry.h>
#include <U2Core/CMDLineHelpProvider.h>
#include <U2Core/CMDLineUtils.h>
#include <cmdline/WorkflowCMDLineTasks.h>
#include <cmdline/GalaxyConfigTask.h>

#include <U2Core/TaskStarter.h>
#include <U2Core/GAutoDeleteList.h>
#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

#include <QtGui/QMenu>

/* TRANSLATOR U2::LocalWorkflow::WorkflowView */
/* TRANSLATOR U2::LocalWorkflow::WorkflowDesignerPlugin */

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    WorkflowDesignerPlugin * plug = new WorkflowDesignerPlugin();
    return plug;
}

#define PLUGIN_SETTINGS QString("workflowview/")

const QString WorkflowDesignerPlugin::RUN_WORKFLOW              = "task";
const QString WorkflowDesignerPlugin::REMOTE_MACHINE            = "task-remote-machine";
const QString WorkflowDesignerPlugin::PRINT                     = "print";

WorkflowDesignerPlugin::WorkflowDesignerPlugin() 
: Plugin(tr("Workflow Designer"), tr("Workflow Designer allows to create complex computational workflows.")){
    if (AppContext::getMainWindow()) {
        services << new WorkflowDesignerService();
        AppContext::getAppSettingsGUI()->registerPage(new WorkflowSettingsPageController());
        AppContext::getObjectViewFactoryRegistry()->registerGObjectViewFactory(new WorkflowViewFactory(this));
    }
    IncludedProtoFactory::init(new IncludedProtoFactoryImpl());

    AppContext::getDocumentFormatRegistry()->registerFormat(new WorkflowDocFormat(this));
    
    // xml workflow tests removed. commented for future uses
    
    //GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    //XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    //assert(xmlTestFormat!=NULL);
    
    //GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    //l->qlist = WorkflowTests::createTestFactories();
    
    //foreach(XMLTestFactory* f, l->qlist) { 
    //    bool res = xmlTestFormat->registerTestFactory(f);
    //    assert(res);
    //    Q_UNUSED(res);
    //}
    
    registerCMDLineHelp();
    processCMDLineOptions();
    WorkflowEnv::getActorValidatorRegistry()->addValidator(DatasetsCountValidator::ID, new DatasetsCountValidator());

    CHECK(AppContext::getPluginSupport(), );
    connect(AppContext::getPluginSupport(), SIGNAL(si_allStartUpPluginsLoaded()), SLOT(sl_initWorkers()));
}

void WorkflowDesignerPlugin::processCMDLineOptions() {
    CMDLineRegistry * cmdlineReg = AppContext::getCMDLineRegistry();
    assert(cmdlineReg != NULL);
 
    bool consoleMode = !AppContext::isGUIMode(); // only in console mode we run workflows by default. Otherwise we show them
    if (cmdlineReg->hasParameter( RUN_WORKFLOW ) || (consoleMode && !CMDLineRegistryUtils::getPureValues().isEmpty()) ) {
        Task * t = NULL;
        if( cmdlineReg->hasParameter(REMOTE_MACHINE) ) {
            t = new WorkflowRemoteRunFromCMDLineTask();
        } else {
            t = new WorkflowRunFromCMDLineTask();
        }
        connect(AppContext::getPluginSupport(), SIGNAL(si_allStartUpPluginsLoaded()), new TaskStarter(t), SLOT(registerTask()));
    }
    else{
        if( cmdlineReg->hasParameter(GalaxyConfigTask::GALAXY_CONFIG_OPTION) && consoleMode ) {
            Task *t = NULL;
            const QString schemePath =  cmdlineReg->getParameterValue( GalaxyConfigTask::GALAXY_CONFIG_OPTION );
            const QString ugenePath = cmdlineReg->getParameterValue( GalaxyConfigTask::UGENE_PATH_OPTION );
            const QString galaxyPath = cmdlineReg->getParameterValue( GalaxyConfigTask::GALAXY_PATH_OPTION );
            const QString destinationPath = NULL;
            t = new GalaxyConfigTask( schemePath, ugenePath, galaxyPath, destinationPath );
            connect(AppContext::getPluginSupport(), SIGNAL(si_allStartUpPluginsLoaded()), new TaskStarter(t), SLOT(registerTask()));
        }
    }
}   

void WorkflowDesignerPlugin::sl_saveSchemaImageTaskFinished() {
    ProduceSchemaImageLinkTask * saveImgTask = qobject_cast<ProduceSchemaImageLinkTask*>(sender());
    assert(saveImgTask != NULL);
    if(saveImgTask->getState() != Task::State_Finished) {
        return;
    }
    
    QString imgUrl = saveImgTask->getImageLink();
    fprintf(stdout, "%s", imgUrl.toLocal8Bit().constData());
}

void WorkflowDesignerPlugin::registerWorkflowTasks() {
    WorkflowTasksRegistry *registry = WorkflowEnv::getWorkflowTasksRegistry();

    ReadDocumentTaskFactory *readAssemblyFactory = new ReadAssemblyTaskFactory();
    bool ok = registry->registerReadDocumentTaskFactory(readAssemblyFactory);
    if (!ok) {
        coreLog.error("Can not register read assembly task");
    }
}

void WorkflowDesignerPlugin::registerCMDLineHelp() {
    CMDLineRegistry * cmdLineRegistry = AppContext::getCMDLineRegistry();
    assert( NULL != cmdLineRegistry );

    CMDLineHelpProvider * taskSection = new CMDLineHelpProvider(
        RUN_WORKFLOW,
        tr("Runs the specified task."),
        tr("Runs the specified task. A path to a user-defined UGENE workflow"
           " be used as a task name."),
        tr("<task_name> [<task_parameter>=value ...]"));

    cmdLineRegistry->registerCMDLineHelpProvider( taskSection );

    CMDLineHelpProvider * printSection = new CMDLineHelpProvider(
        PRINT,
        tr("Prints the content of the specified slot."),
        tr("Prints the content of the specified slot. The incoming/outcoming content of"
        " specified slot is printed to the standart output."),
        tr("<actor_name>.<port_name>.<slot_name>"));
    Q_UNUSED(printSection);

    CMDLineHelpProvider * galaxyConfigSection = new CMDLineHelpProvider(
        GalaxyConfigTask::GALAXY_CONFIG_OPTION,
        tr("Creates new Galaxy tool config."),
        tr("Creates new Galaxy tool config from existing workflow. Paths to UGENE"
        " and Galaxy can be set"),
        tr("<uwl-file> [--ugene-path=value] [--galaxy-path=value]"));

    cmdLineRegistry->registerCMDLineHelpProvider( galaxyConfigSection );

    //CMDLineHelpProvider * remoteMachineSectionArguments = new CMDLineHelpProvider( REMOTE_MACHINE, "<path-to-machine-file>");
    //CMDLineHelpProvider * remoteMachineSection = new CMDLineHelpProvider( REMOTE_MACHINE, tr("run provided tasks on given remote machine") );        
    //TODO: bug UGENE-23
    //cmdLineRegistry->registerCMDLineHelpProvider( remoteMachineSectionArguments );
    //cmdLineRegistry->registerCMDLineHelpProvider( remoteMachineSection );
}

void WorkflowDesignerPlugin::sl_initWorkers() {
    Workflow::CoreLib::init();
    registerWorkflowTasks();
    Workflow::CoreLib::initIncludedWorkers();
}

class CloseDesignerTask : public Task {
public:
    CloseDesignerTask(WorkflowDesignerService* s) : 
      Task(U2::WorkflowDesignerPlugin::tr("Close Designer"), TaskFlag_NoRun),
          service(s) {}
    virtual void prepare();
private:
    WorkflowDesignerService* service;
};

void CloseDesignerTask::prepare() {
    if (!service->closeViews()) {
        stateInfo.setError(  U2::WorkflowDesignerPlugin::tr("Close Designer canceled") );
    }
}

Task* WorkflowDesignerService::createServiceDisablingTask(){
    return new CloseDesignerTask(this);
}

WorkflowDesignerService::WorkflowDesignerService() 
: Service(Service_WorkflowDesigner, tr("Workflow Designer"), ""),
designerAction(NULL), managerAction(NULL)
{
}

void WorkflowDesignerService::serviceStateChangedCallback(ServiceState , bool enabledStateChanged) {

    if (!enabledStateChanged) {
        return;
    }
    if (isEnabled()) {
        assert(designerAction == NULL);

        if(!AppContext::getPluginSupport()->isAllPluginsLoaded()) {
            connect( AppContext::getPluginSupport(), SIGNAL( si_allStartUpPluginsLoaded() ), SLOT(sl_startWorkflowPlugin())); 
        } else {
            sl_startWorkflowPlugin();
        }

        /*designerAction = new QAction(getName(), this);
        connect(designerAction, SIGNAL(triggered()), SLOT(sl_showDesignerWindow()));

        //managerAction = new QAction(tr("Workflow Manager"), this);
        //connect(designerAction, SIGNAL(triggered()), SLOT(sl_showManagerWindow()));

        AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS)->addAction(designerAction);
        //AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS)->addAction(managerAction);

        //FIXME refactor
        if (AppContext::getSettings()->getValue(PLUGIN_SETTINGS + "isVisible", false).toBool()) {
            sl_showDesignerWindow();
        }*/
    } else {
        delete designerAction;
        designerAction = NULL;
    }
}

void WorkflowDesignerService::sl_startWorkflowPlugin() {
    designerAction = new QAction( QIcon(":/workflow_designer/images/wd.png"), tr("Workflow Designer..."), this);
    designerAction->setObjectName("Workflow Designer");
#ifdef _DEBUG
    designerAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
#endif
    connect(designerAction, SIGNAL(triggered()), SLOT(sl_showDesignerWindow()));

    //managerAction = new QAction(tr("Workflow Manager"), this);
    //connect(designerAction, SIGNAL(triggered()), SLOT(sl_showManagerWindow()));

    AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS)->addAction(designerAction);
    //AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS)->addAction(managerAction);

    //FIXME refactor
    if (AppContext::getSettings()->getValue(PLUGIN_SETTINGS + "isVisible", false).toBool()) {
        sl_showDesignerWindow();
    }
}

bool WorkflowDesignerService::closeViews() {
    MWMDIManager* wm = AppContext::getMainWindow()->getMDIManager();
    assert(wm);
    foreach(MWMDIWindow* w, wm->getWindows()) {
        WorkflowView* view = qobject_cast<WorkflowView*>(w);
        if (view) {
            if (!AppContext::getMainWindow()->getMDIManager()->closeMDIWindow(view)) {
                return false;
            }
        }
    }
    return true;
}

void WorkflowDesignerService::sl_showDesignerWindow() {
    assert(isEnabled());
    WorkflowView* view = new WorkflowView(NULL); //FIXME
    view->setWindowIcon(QIcon(":/workflow_designer/images/wd.png"));
    AppContext::getMainWindow()->getMDIManager()->addMDIWindow(view);
    AppContext::getMainWindow()->getMDIManager()->activateWindow(view);
}

void WorkflowDesignerService::sl_showManagerWindow() {

}

Task* WorkflowDesignerService::createServiceEnablingTask()
{
    QString defaultDir = QDir::searchPaths( PATH_PREFIX_DATA ).first() + "/workflow_samples";

    return SampleRegistry::init(QStringList(defaultDir));
}
}//namespace
