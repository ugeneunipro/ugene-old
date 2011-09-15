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

#include <U2Core/Log.h>
#include <U2Core/Timer.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/ResourceTracker.h>
#include <U2Core/DocumentFormatConfigurators.h>
#include <U2Core/DBXRefRegistry.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Core/DataBaseRegistry.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/CMDLineUtils.h>
#include <U2Core/CMDLineCoreOptions.h>
#include <U2Core/DNAAlphabetRegistryImpl.h>
#include <U2Core/AnnotationSettings.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/TaskStarter.h>
#include <U2Core/LoadRemoteDocumentTask.h>
#include <U2Core/LogCache.h>
#include <U2Core/VirtualFileSystem.h>

#include <U2Formats/DocumentFormatUtils.h>

#include <U2Algorithm/SecStructPredictAlgRegistry.h>
#include <U2Algorithm/CudaGpuRegistry.h>
#include <U2Algorithm/OpenCLGpuRegistry.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>
#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include <U2Algorithm/PWMConversionAlgorithmRegistry.h>
#include <U2Algorithm/SubstMatrixRegistry.h>
#include <U2Algorithm/SWResultFilterRegistry.h>
#include <U2Algorithm/SmithWatermanTaskFactoryRegistry.h>
#include <U2Algorithm/DnaAssemblyAlgRegistry.h>
#include <U2Algorithm/MSAAlignAlgRegistry.h>
#include <U2Algorithm/RepeatFinderTaskFactoryRegistry.h>
#include <U2Algorithm/MolecularSurfaceFactoryRegistry.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>
#include <U2Algorithm/StructuralAlignmentAlgorithmRegistry.h>
#include <U2Algorithm/CDSearchTaskFactoryRegistry.h>

#include <U2Gui/ObjectViewModel.h>
#include <U2Gui/BaseDocumentFormatConfigurators.h>
#include <U2Gui/LogView.h>

#include <U2View/MSAColorScheme.h>
#include <U2View/UIndexSupport.h>
#include <U2View/DnaAssemblyUtils.h>

#include <U2Test/GTestFrameworkComponents.h>
#include <U2Test/XMLTestFormat.h>
#include <U2Test/GUITestService.h>
#include <U2Test/GUITestBase.h>

#include <U2Lang/WorkflowEnvImpl.h>
#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowSettings.h>
#include <U2Lang/WorkflowRunTask.h>
#include <U2Lang/QueryDesignerRegistry.h>

#include <U2Remote/DistributedComputingUtil.h>

//U2Private imports
#include <AppContextImpl.h>
#include <SettingsImpl.h>
#include <DocumentFormatRegistryImpl.h>
#include <IOAdapterRegistryImpl.h>
#include <PluginSupportImpl.h>
#include <ServiceRegistryImpl.h>
#include <TaskSchedulerImpl.h>
#include <AppSettingsImpl.h>
#include <CrashHandler.h>


#include "app_settings/AppSettingsGUIImpl.h"
#include "app_settings/logview_settings/LogSettingsGUIController.h"

#include "plugin_viewer/PluginViewerImpl.h"
#include "project_support/ProjectLoaderImpl.h"
#include "main_window/MainWindowImpl.h"
#include "main_window/CheckUpdatesTask.h"
#include "project_view/ProjectViewImpl.h"

#include "task_view/TaskViewController.h"
#include "shtirlitz/Shtirlitz.h"
#include <project_support/ProjectTasksGui.h>


#include <QtGui/QApplication>
#include <QtGui/QIcon>



/* TRANSLATOR U2::AppContextImpl */

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
QT_BEGIN_NAMESPACE
extern Q_GUI_EXPORT bool qt_use_native_dialogs;
QT_END_NAMESPACE
#endif

#ifdef _DEBUG //force no crash handler mode in debug build
#undef USE_CRASHHANDLER
#endif



namespace U2 {

static void registerCoreServices() {
    ServiceRegistry* sr = AppContext::getServiceRegistry();
    TaskScheduler* ts = AppContext::getTaskScheduler();
    ts->registerTopLevelTask(sr->registerServiceTask(new PluginViewerImpl()));
    ts->registerTopLevelTask(sr->registerServiceTask(new ProjectViewImpl()));
//    ts->registerTopLevelTask(sr->registerServiceTask(new ScriptRegistryService()));
}

}//namespace

using namespace U2;

static void updateStaticTranslations() {
    GObjectTypes::initTypeTranslations();
    GObjectTypes::initTypeIcons();
}

static void setDataSearchPaths() {
    //set search paths for data files
    QStringList dataSearchPaths;
    const static char * RELATIVE_DATA_DIR = "/data";
    const static char * RELATIVE_DEV_DATA_DIR = "/../../data";
    //on windows data is normally located in the application directory
    QString appDirPath = QCoreApplication::applicationDirPath();
    if( QDir(appDirPath+RELATIVE_DATA_DIR).exists() ) {
        dataSearchPaths.push_back( appDirPath+RELATIVE_DATA_DIR );
    } else if( QDir(appDirPath+RELATIVE_DEV_DATA_DIR).exists() ) {          //data location for developers
        dataSearchPaths.push_back( appDirPath+RELATIVE_DEV_DATA_DIR );
    }
    if( dataSearchPaths.empty() ) {
        dataSearchPaths.push_back("/");
    }

#if (defined(Q_OS_LINUX) || defined(Q_OS_UNIX)) && defined( UGENE_DATA_DIR )
    //using directory which is set during installation process on Linux
    QString ugene_data_dir( UGENE_DATA_DIR );
    if( QDir(ugene_data_dir).exists() ) {
        dataSearchPaths.push_back( QString(UGENE_DATA_DIR) );
    }
#endif

    QDir::setSearchPaths( PATH_PREFIX_DATA, dataSearchPaths );
    //now data files may be opened using QFile( "data:some_data_file" )
} 

static void setSearchPaths() {
    setDataSearchPaths();
}


static void initLogsCache(LogCacheExt& logsCache, const QStringList& ) {
#ifdef _DEBUG
    logsCache.setConsoleOutputEnabled(true);
#else
    QString consoleOutVal = qgetenv("UGENE_PRINT_TO_CONSOLE");
    logsCache.setConsoleOutputEnabled(consoleOutVal == "true" || consoleOutVal == "1");
#endif
    QString file = qgetenv("UGENE_PRINT_TO_FILE");
    if (!file.isEmpty()) {
        logsCache.setFileOutputEnabled(file);
    }
}

class GApplication: public QApplication {
public:
    GApplication(int & argc, char ** argv): QApplication(argc, argv) {}
    virtual bool notify(QObject * receiver, QEvent * event ) {
        bool res = false;
#if defined(USE_CRASHHANDLER)
        try {
            res = QApplication::notify(receiver, event);
        } catch(...) {
            if(CrashHandler::buffer) {
                CrashHandler::releaseReserve();
            }

            CrashHandler::runMonitorProcess("C++ exception|Unhandled exception");
        }
#else
        res = QApplication::notify(receiver, event);
#endif
        return res;
    }
};

int main(int argc, char **argv) 
{
#if defined(USE_CRASHHANDLER)
    CrashHandler::setupHandler();
    CrashHandler::preallocateReservedSpace();
#endif

    QT_REQUIRE_VERSION( argc, argv, QT_VERSION_STR );

    GTIMER(c1, t1, "main()->QApp::exec");
    
    //QApplication app(argc, argv);
    GApplication app(argc, argv);
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

// add some extra paths used during development
#ifdef Q_OS_WIN
#ifdef _DEBUG
    QString devPluginsPath = QDir(QCoreApplication::applicationDirPath()+"/../../extras/windows/dotnet_style/_debug").absolutePath();
#else 
    QString devPluginsPath = QDir(QCoreApplication::applicationDirPath()+"/../../extras/windows/dotnet_style/_release").absolutePath();
#endif
    QCoreApplication::addLibraryPath(devPluginsPath); //dev version
#endif

    setSearchPaths();

    AppContextImpl* appContext = AppContextImpl::getApplicationContext();
    appContext->setGUIMode(true);

    // parse all cmdline arguments
    CMDLineRegistry* cmdLineRegistry = new CMDLineRegistry(app.arguments()); 
    appContext->setCMDLineRegistry(cmdLineRegistry);
    
    //1 create settings
    SettingsImpl* globalSettings = new SettingsImpl(QSettings::SystemScope);
    appContext->setGlobalSettings(globalSettings);

    SettingsImpl* settings = new SettingsImpl(QSettings::UserScope);
    appContext->setSettings(settings);

    AppSettings* appSettings = new AppSettingsImpl();
    appContext->setAppSettings(appSettings);

    UserAppsSettings* userAppSettings = AppContext::getAppSettings()->getUserAppsSettings();
    
    // set translations
    QTranslator translator;
    QString transFile[] = {
        userAppSettings->getTranslationFile(),
        "transl_" + QLocale::system().name(),
        "transl_en"
    };
    bool trOK = false;
    for (int i = transFile[0].isEmpty() ? 1 : 0; i < 3; ++i) {
        if (!translator.load(transFile[i], QCoreApplication::applicationDirPath())) {
            fprintf(stderr, "Translation not found: %s\n", transFile[i].toAscii().constData());
        } else {
            trOK = true;
            break;
        }
    }
    if (!trOK) {
        fprintf(stderr, "No translations found, exiting\n");
        return 1;   
    }
    
    app.installTranslator(&translator);
    updateStaticTranslations();
    
    // 3 create functional components of ugene
    QStringList envList = QProcess::systemEnvironment();

    LogCacheExt logsCache;
    initLogsCache(logsCache, envList);
    LogCache::setAppGlobalInstance(&logsCache);
    coreLog.details(AppContextImpl::tr("UGENE initialization started"));

    coreLog.trace(QString("UGENE run at dir %1 with parameters %2").arg(app.applicationDirPath()).arg(app.arguments().join(" ")));

    //print some settings info, can't do it earlier than logging is initialized
    coreLog.trace(QString("Active UGENE.ini file : %1").arg(AppContext::getSettings()->fileName()));
//    logCat.trace(QString("INI-var: %1").arg(AppContext::getCMDLineRegistry()->getParameterValue( CMDLineCoreOptions::INI_FILE)));


#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
    bool useNative = envList.indexOf("UGENE_USE_NATIVE_DIALOGS=false") < 0 && envList.indexOf("UGENE_USE_NATIVE_DIALOGS=0") < 0;
    qt_use_native_dialogs = useNative; //using local here for debugger
#endif

    QString style = userAppSettings->getVisualStyle();
    if (!style.isEmpty()) {
        QStyle* qstyle = QStyleFactory::create(style);
        if (qstyle!=NULL) {
            app.setStyle(qstyle);
        } else {
            uiLog.details(AppContextImpl::tr("Style not available %1").arg(style));
        }
    } 
    
    ResourceTracker* resTrack = new ResourceTracker();
    appContext->setResourceTracker(resTrack);

    TaskSchedulerImpl* ts = new TaskSchedulerImpl(appSettings->getAppResourcePool());
    appContext->setTaskScheduler(ts);

    AnnotationSettingsRegistry* asr = new AnnotationSettingsRegistry(DocumentFormatUtils::predefinedSettings());
    appContext->setAnnotationSettingsRegistry(asr);

    TestFramework* tf = new TestFramework();
    appContext->setTestFramework(tf);

    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    QList<XMLTestFactory*> fs = ProjectTests::createTestFactories();
    foreach(XMLTestFactory* f, fs) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        assert(res); Q_UNUSED(res);
    }

    MainWindowImpl* mw = new MainWindowImpl();
    appContext->setMainWindow(mw);
    mw->show();
   
    AppSettingsGUI* appSettingsGUI = new AppSettingsGUIImpl();
    appContext->setAppSettingsGUI(appSettingsGUI);

    AppContext::getMainWindow()->getDockManager()->registerDock(MWDockArea_Bottom, new TaskViewDockWidget(), QKeySequence(Qt::ALT | Qt::Key_2));
    
    // Initialize logged log view
    LogViewWidget* logView = new LogViewWidget(&logsCache);
    logView->setObjectName(DOCK_LOG_VIEW);
    AppContext::getAppSettingsGUI()->registerPage(new LogSettingsPageController(logView));
    AppContext::getMainWindow()->getDockManager()->registerDock(MWDockArea_Bottom, logView, QKeySequence(Qt::ALT | Qt::Key_3));
    
    
    GObjectViewFactoryRegistry* ovfr = new GObjectViewFactoryRegistry();
    appContext->setObjectViewFactoryRegistry(ovfr);

    PluginSupportImpl* psp = new PluginSupportImpl();
    appContext->setPluginSupport(psp);

    ServiceRegistryImpl* sreg = new ServiceRegistryImpl() ;
    appContext->setServiceRegistry(sreg);

    DocumentFormatRegistryImpl* dfr = new DocumentFormatRegistryImpl();
    appContext->setDocumentFormatRegistry(dfr);

    DocumentFormatConfigurators* dfc = new DocumentFormatConfigurators();
    appContext->setDocumentFormatConfigurators(dfc);
    BaseDocumentFormatConfigurators::initBuiltInConfigurators();
    
    IOAdapterRegistryImpl* io = new IOAdapterRegistryImpl();
    appContext->setIOAdapterRegistry(io);

    DNATranslationRegistry* dtr = new DNATranslationRegistry();
    appContext->setDNATranslationRegistry(dtr);

    DNAAlphabetRegistry* dal = new DNAAlphabetRegistryImpl(dtr);
    appContext->setDNAAlphabetRegistry(dal);

    ProjectLoaderImpl* pli = new ProjectLoaderImpl();
    appContext->setProjectLoader(pli);

    DBXRefRegistry* dbxr = new DBXRefRegistry();
    appContext->setDBXRefRegistry(dbxr);

    MSAColorSchemeRegistry* mcsr = new MSAColorSchemeRegistry();
    appContext->setMSAColorSchemeRegistry(mcsr);

    MSAConsensusAlgorithmRegistry* msaConsReg = new MSAConsensusAlgorithmRegistry();
    appContext->setMSAConsensusAlgorithmRegistry(msaConsReg);

    MSADistanceAlgorithmRegistry* msaDistReg = new MSADistanceAlgorithmRegistry();
    appContext->setMSADistanceAlgorithmRegistry(msaDistReg);

    PWMConversionAlgorithmRegistry* pwmConvReg = new PWMConversionAlgorithmRegistry();
    appContext->setPWMConversionAlgorithmRegistry(pwmConvReg);

    SubstMatrixRegistry* smr = new SubstMatrixRegistry();
    appContext->setSubstMatrixRegistry(smr);

    SmithWatermanTaskFactoryRegistry* swar = new SmithWatermanTaskFactoryRegistry();
    appContext->setSmithWatermanTaskFactoryRegistry(swar);

    RepeatFinderTaskFactoryRegistry* rfr = new RepeatFinderTaskFactoryRegistry();
    appContext->setRepeatFinderTaskFactoryRegistry(rfr);

    ExternalToolRegistry* etr = new ExternalToolRegistry();
    appContext->setExternalToolRegistry(etr);

    U2DbiRegistry *dbiRegistry = new U2DbiRegistry();
    appContext->setDbiRegistry(dbiRegistry);

    QDActorPrototypeRegistry* qpr = new QDActorPrototypeRegistry();
    appContext->setQDActorFactoryRegistry(qpr);

    CDSearchFactoryRegistry* cdsfr = new CDSearchFactoryRegistry();
    appContext->setCDSearchFactoryRegistry(cdsfr);

    StructuralAlignmentAlgorithmRegistry *saar = new StructuralAlignmentAlgorithmRegistry();
    appContext->setStructuralAlignmentAlgorithmRegistry(saar);

    CMDLineUtils::init();
    
    PhyTreeGeneratorRegistry* genRegistry = new PhyTreeGeneratorRegistry();
    appContext->setPhyTreeGeneratorRegistry(genRegistry);

    MolecularSurfaceFactoryRegistry* msfr = new MolecularSurfaceFactoryRegistry();
    appContext->setMolecularSurfaceFactoryRegistry(msfr);

    SWResultFilterRegistry* swrfr = new SWResultFilterRegistry();
    appContext->setSWResultFilterRegistry(swrfr);

    SecStructPredictAlgRegistry* sspar = new SecStructPredictAlgRegistry();
    appContext->setSecStructPedictAlgRegistry(sspar);

    CudaGpuRegistry * cgr = new CudaGpuRegistry();
    appContext->setCudaGpuRegistry(cgr); 

    OpenCLGpuRegistry * oclgr = new OpenCLGpuRegistry();
    appContext->setOpenCLGpuRegistry( oclgr );

    RecentlyDownloadedCache* rdc = new RecentlyDownloadedCache();
    appContext->setRecentlyDownloadedCache(rdc);

    DistributedComputingUtil * dcu = new DistributedComputingUtil();
    
    VirtualFileSystemRegistry * vfsReg = new VirtualFileSystemRegistry();
    appContext->setVirtualFileSystemRegistry( vfsReg );
    
    Workflow::WorkflowEnv::init(new Workflow::WorkflowEnvImpl());
    Workflow::WorkflowEnv::getDomainRegistry()->registerEntry(new LocalWorkflow::LocalDomainFactory());
    
    UIndexSupport indexSupport;

    DnaAssemblyAlgRegistry* assemblyReg = new DnaAssemblyAlgRegistry();
    appContext->setDnaAssemblyAlgRegistry(assemblyReg);
    DnaAssemblySupport assemblySupport;

    MSAAlignAlgRegistry* alignReg = new MSAAlignAlgRegistry();
    appContext->setMSAAlignAlgRegistry(alignReg);

    DataBaseRegistry *dbr = new DataBaseRegistry();
    appContext->setDataBaseRegistry(dbr);

    AutoAnnotationsSupport* aaSupport = new AutoAnnotationsSupport();
    appContext->setAutoAnnotationsSupport(aaSupport);

    GUITestBase *tb = new GUITestBase();
    appContext->setGUITestBase(tb);
    
#ifndef RUN_WORKFLOW_IN_THREADS
    CheckCmdlineUgeneUtils::setCmdlineUgenePath();
#endif // RUN_WORKFLOW_IN_THREADS
    
    if(!cmdLineRegistry->hasParameter(CMDLineCoreOptions::LAUNCH_TEST)) {
    QStringList urls = CMDLineRegistryUtils::getPureValues();
    if( !urls.isEmpty() ) {
        // defer loading until all plugins/services loaded
        QObject::connect( AppContext::getPluginSupport(), SIGNAL( si_allStartUpPluginsLoaded() ), 
            new TaskStarter( new OpenWithProjectTask(urls) ), SLOT( registerTask() ) );
        
    } else if (AppContext::getAppSettings()->getUserAppsSettings()->openLastProjectAtStartup()) {
        QString lastProject = ProjectLoaderImpl::getLastProjectURL();
        if (!lastProject.isEmpty()) {
            Task* t = AppContext::getProjectLoader()->openWithProjectTask(lastProject);
            // defer project start until all plugins/services loaded
            QObject::connect(AppContext::getPluginSupport(), SIGNAL(si_allStartUpPluginsLoaded()), 
                new TaskStarter(t), SLOT(registerTask()));
        }
    }
    }
    
    registerCoreServices();
#ifdef _DEBUG 
    GUITestService *guiTestService = new GUITestService();
    Q_UNUSED(guiTestService);
#endif
    
    if ( !envList.contains(ENV_UGENE_DEV+QString("=1")) ) {
        Shtirlitz::wakeup();
    }

    GCOUNTER(cvar, tvar, "ugeneui launch");

    //3 run QT GUI
    t1.stop();
    coreLog.info(AppContextImpl::tr("UGENE started"));
    if(AppContext::getSettings()->getValue(ASK_VESRION_SETTING, true).toBool()) {
        ts->registerTopLevelTask(new CheckUpdatesTask(true));
    }
    int rc = app.exec();

    //4 deallocate resources
    if ( !envList.contains(ENV_UGENE_DEV+QString("=1")) ) {
        Shtirlitz::saveGatheredInfo();
    }

    Workflow::WorkflowEnv::shutdown();
    
    delete dcu;

    appContext->setGUITestBase(NULL);
    delete tb;
    
    appContext->setVirtualFileSystemRegistry( NULL );
    delete vfsReg;
    
    appContext->setRecentlyDownloadedCache(NULL);
    delete rdc;

    appContext->setSWResultFilterRegistry(NULL);
    delete swrfr;

    appContext->setSmithWatermanTaskFactoryRegistry(NULL);
    delete swar;

    appContext->setMolecularSurfaceFactoryRegistry(NULL);
    delete msfr;

    appContext->setSubstMatrixRegistry(NULL);
    delete smr;

    appContext->setMSAColorSchemeRegistry(NULL);
    delete mcsr;

    appContext->setDBXRefRegistry(NULL);
    delete dbxr;

    appContext->setProjectLoader(NULL);
    delete pli;

    appContext->setServiceRegistry(NULL);
    delete sreg;

    appContext->setPluginSupport(NULL);
    delete psp;

    appContext->setExternalToolRegistry(NULL);
    delete etr;

    appContext->setMainWindow(NULL);
    delete mw;

    appContext->setTestFramework(0);
    delete tf;

    appContext->setObjectViewFactoryRegistry(NULL);
    delete ovfr;

    appContext->setDNAAlphabetRegistry(NULL);
    delete dal;

    appContext->setDNATranslationRegistry(NULL);
    delete dtr;

    appContext->setIOAdapterRegistry(NULL);
    delete io;

    appContext->setDocumentFormatConfigurators(NULL);
    delete dfc;

    appContext->setDocumentFormatRegistry(NULL);
    delete dfr;

    appContext->setTaskScheduler(NULL);
    delete ts;

    appContext->setAnnotationSettingsRegistry(NULL);
    delete asr;

    appContext->setResourceTracker(NULL);
    delete resTrack;

    appContext->setCudaGpuRegistry(NULL);
    delete cgr;

    delete oclgr;
    appContext->setOpenCLGpuRegistry(NULL);

    delete cmdLineRegistry;
    appContext->setCMDLineRegistry(NULL);

    appContext->setSecStructPedictAlgRegistry(NULL);
    delete sspar;

    appContext->setMSAConsensusAlgorithmRegistry(NULL);
    delete msaConsReg;
    
    appContext->setMSADistanceAlgorithmRegistry(NULL);
    delete msaDistReg;

    appContext->setPWMConversionAlgorithmRegistry(NULL);
    delete pwmConvReg;

    appContext->setDnaAssemblyAlgRegistry(NULL);
    delete assemblyReg;

    appContext->setMSAAlignAlgRegistry(NULL);
    delete alignReg;
    
    appContext->setDataBaseRegistry(NULL);
    delete dbr;

    appContext->setAutoAnnotationsSupport(NULL);
    delete aaSupport;

    appContext->setQDActorFactoryRegistry(NULL);
    delete qpr;

    appContext->setCDSearchFactoryRegistry(NULL);
    delete cdsfr;

    appContext->setStructuralAlignmentAlgorithmRegistry(NULL);
    delete saar;

    appContext->setAppSettingsGUI(NULL);
    delete appSettingsGUI;

    appContext->setAppSettings(NULL);
    delete appSettings;

    appContext->setSettings(NULL);
    delete settings;

    appContext->setGlobalSettings(NULL);
    delete globalSettings;

    return rc;   
}

