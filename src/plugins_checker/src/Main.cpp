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

#include <U2Core/CMDLineRegistry.h>
#include <U2Core/ResourceTracker.h>
#include <U2Core/Timer.h>
#include <U2Core/ConsoleShutdownTask.h>

#include <U2Algorithm/OpenCLGpuRegistry.h>

#include <U2Lang/LocalDomain.h>

//U2Private
#include <AppContextImpl.h>
#include <SettingsImpl.h>
#include <DocumentFormatRegistryImpl.h>
#include <IOAdapterRegistryImpl.h>
#include <PluginSupportImpl.h>
#include <ServiceRegistryImpl.h>
#include <TaskSchedulerImpl.h>
#include <AppSettingsImpl.h>
#include <CrashHandler.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QLibraryInfo>

#define TR_SETTINGS_ROOT QString("test_runner/")

/* TRANSLATOR U2::AppContextImpl */

using namespace U2;

static void registerCoreServices() {
    ServiceRegistry* sr = AppContext::getServiceRegistry();
    TaskScheduler* ts = AppContext::getTaskScheduler();
    Q_UNUSED(sr); Q_UNUSED(ts);
    // unlike ugene's UI Main.cpp we don't create PluginViewerImpl, ProjectViewImpl
//    ts->registerTopLevelTask(sr->registerServiceTask(new ScriptRegistryService()));
}

class GApplication: public QCoreApplication { //Move to the core? same code in ugenecl and ugeneui
public:
    GApplication(int & argc, char ** argv): QCoreApplication(argc, argv) {}
    virtual bool notify(QObject * receiver, QEvent * event ) {
        bool res = false;

        try {
            res = QCoreApplication::notify(receiver, event);
        } catch(...) {
            if(CrashHandler::buffer) {
                CrashHandler::releaseReserve();
            }

            CrashHandler::runMonitorProcess("C++ exception|Unhandled exception");
        }
        return res;
    }

};

int main(int argc, char **argv) 
{
    CrashHandler::setupHandler();
    CrashHandler::preallocateReservedSpace();
    CrashHandler::sendCrashReports = false;

    const char* build = QT_VERSION_STR, *runtime = qVersion();
    if (strcmp(build, runtime) > 0){
        printf("Installed Qt version must be %s or greater \r\n", QT_VERSION_STR);
        return -1;
    }

    GTIMER(c1, t1, "main()->QApp::exec");

    GApplication app(argc, argv);

#ifdef Q_OS_MAC
    // A workaround to avoid using non-bundled plugins
    QCoreApplication::removeLibraryPath(QLibraryInfo::location(QLibraryInfo::PluginsPath));
    QCoreApplication::addLibraryPath("../../PlugIns");
#endif

    AppContextImpl* appContext = AppContextImpl::getApplicationContext();
    appContext->setWorkingDirectoryPath(QCoreApplication::applicationDirPath());

    QCoreApplication::addLibraryPath(AppContext::getWorkingDirectoryPath());
    QString devPluginsPath = QDir(AppContext::getWorkingDirectoryPath()+"/../../installer/windows").absolutePath();
    QCoreApplication::addLibraryPath(devPluginsPath); //dev version

    // parse all cmdline arguments
    CMDLineRegistry* cmdLineRegistry = new CMDLineRegistry(app.arguments()); 
    appContext->setCMDLineRegistry(cmdLineRegistry);
    
    //1 create settings
    SettingsImpl* globalSettings = new SettingsImpl(QSettings::SystemScope);
    appContext->setGlobalSettings(globalSettings);

    SettingsImpl * settings = new SettingsImpl( QSettings::UserScope );
    appContext->setSettings( settings );
    
    
    AppSettings* appSettings = new AppSettingsImpl();
    appContext->setAppSettings(appSettings);

    // 2 create functional components of ugene

    ResourceTracker* resTrack = new ResourceTracker();
    appContext->setResourceTracker(resTrack);

    TaskSchedulerImpl* ts = new TaskSchedulerImpl(appSettings->getAppResourcePool());
    appContext->setTaskScheduler(ts);

    PluginSupportImpl* psp = new PluginSupportImpl(true);
    appContext->setPluginSupport(psp);

    ServiceRegistryImpl* sreg = new ServiceRegistryImpl() ;
    appContext->setServiceRegistry(sreg);

#ifdef OPENCL_SUPPORT
    OpenCLGpuRegistry * oclgr = new OpenCLGpuRegistry();
    appContext->setOpenCLGpuRegistry( oclgr );
#endif

    registerCoreServices();

    //3 run QT 
    t1.stop();
    ConsoleShutdownTask watchQuit(&app);
    int rc = app.exec();
    //int rc = 0;
    //4 deallocate resources
    //Workflow::WorkflowEnv::shutdown();

    delete psp;
    appContext->setPluginSupport(NULL);

    delete ts;
    appContext->setTaskScheduler(NULL);

    delete resTrack;
    appContext->setResourceTracker(NULL);

    appContext->setAppSettings(NULL);
    delete appSettings;

    delete settings;
    appContext->setSettings(NULL);

    delete globalSettings;
    appContext->setGlobalSettings(NULL);

    delete cmdLineRegistry;
    appContext->setCMDLineRegistry(NULL);

    return rc;
}
