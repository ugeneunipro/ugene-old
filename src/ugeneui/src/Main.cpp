/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
#include <QIcon>

#include <U2Algorithm/AlignmentAlgorithmsRegistry.h>
#include <U2Algorithm/AssemblyConsensusAlgorithmRegistry.h>
#include <U2Algorithm/CDSearchTaskFactoryRegistry.h>
#include <U2Algorithm/CudaGpuRegistry.h>
#include <U2Algorithm/DnaAssemblyAlgRegistry.h>
#include <U2Algorithm/GenomeAssemblyRegistry.h>
#include <U2Algorithm/MSAColorScheme.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>
#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include <U2Algorithm/MolecularSurfaceFactoryRegistry.h>
#include <U2Algorithm/OpenCLGpuRegistry.h>
#include <U2Algorithm/PWMConversionAlgorithmRegistry.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>
#include <U2Algorithm/RepeatFinderTaskFactoryRegistry.h>
#include <U2Algorithm/SWMulAlignResultNamesTagsRegistry.h>
#include <U2Algorithm/SWResultFilterRegistry.h>
#include <U2Algorithm/SecStructPredictAlgRegistry.h>
#include <U2Algorithm/SequenceContentFilterTask.h>
#include <U2Algorithm/SmithWatermanTaskFactoryRegistry.h>
#include <U2Algorithm/SplicedAlignmentTaskRegistry.h>
#include <U2Algorithm/StructuralAlignmentAlgorithmRegistry.h>
#include <U2Algorithm/SubstMatrixRegistry.h>

#include <U2Core/AnnotationSettings.h>
#include <U2Core/AppFileStorage.h>
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Core/CMDLineCoreOptions.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/CMDLineUtils.h>
#include <U2Core/Counter.h>
#include <U2Core/DASSource.h>
#include <U2Core/DBXRefRegistry.h>
#include <U2Core/DNAAlphabetRegistryImpl.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DataBaseRegistry.h>
#include <U2Core/DataPathRegistry.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/LoadRemoteDocumentTask.h>
#include <U2Core/Log.h>
#include <U2Core/LogCache.h>
#include <U2Core/PasswordStorage.h>
#include <U2Core/ProjectFilterTaskRegistry.h>
#include <U2Core/ResourceTracker.h>
#include <U2Core/ScriptingToolRegistry.h>
#include <U2Core/TaskStarter.h>
#include <U2Core/Timer.h>
#include <U2Core/TmpDirChecker.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/UdrSchemaRegistry.h>
#include <U2Core/UserActionsWriter.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/VirtualFileSystem.h>

#include <U2Formats/ConvertFileTask.h>
#include <U2Formats/DocumentFormatUtils.h>

#include <U2Gui/CredentialsAskerGui.h>
#include <U2Gui/FeatureKeyFilterTask.h>
#include <U2Gui/ImportDialogsFactories.h>
#include <U2Gui/LogView.h>
#include <U2Gui/MsaContentFilterTask.h>
#include <U2Gui/MsaSeqNameFilterTask.h>
#include <U2Gui/OPWidgetFactoryRegistry.h>
#include <U2Gui/ObjectNameFilterTask.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Gui/SequenceAccFilterTask.h>
#include <U2Gui/TextContentFilterTask.h>
#include <U2Gui/ToolsMenu.h>

#include <U2Lang/LocalDomain.h>
#include <U2Lang/QueryDesignerRegistry.h>
#include <U2Lang/WorkflowEnvImpl.h>
#include <U2Lang/WorkflowSettings.h>

#include <U2Remote/DistributedComputingUtil.h>

#include <U2Test/GTestFrameworkComponents.h>
#include <U2Test/GUITestBase.h>
#include <U2Test/GUITestService.h>
#include <U2Test/XMLTestFormat.h>

#include <U2View/AnnotHighlightWidgetFactory.h>
#include <U2View/AssemblyInfoWidget.h>
#include <U2View/AssemblyNavigationWidget.h>
#include <U2View/AssemblySettingsWidget.h>
#include <U2View/ColorSchemaSettingsController.h>
#include <U2View/DasWidgetFactory.h>
#include <U2View/DnaAssemblyUtils.h>
#include <U2View/FindPatternWidgetFactory.h>
#include <U2View/MSAExportConsensusTabFactory.h>
#include <U2View/MSAGeneralTabFactory.h>
#include <U2View/MSAHighlightingTabFactory.h>
#include <U2View/PairAlignFactory.h>
#include <U2View/RefSeqCommonWidget.h>
#include <U2View/SeqStatisticsWidgetFactory.h>
#include <U2View/SequenceInfoFactory.h>
#include <U2View/TreeOptionsWidgetFactory.h>

//U2Private imports
#include <AppContextImpl.h>
#include <AppSettingsImpl.h>
#include <DocumentFormatRegistryImpl.h>
#include <IOAdapterRegistryImpl.h>
#include <PluginSupportImpl.h>
#include <ServiceRegistryImpl.h>
#include <SettingsImpl.h>
#include <TaskSchedulerImpl.h>
#include <crash_handler/CrashHandler.h>

#include "app_settings/AppSettingsGUIImpl.h"
#include "app_settings/logview_settings/LogSettingsGUIController.h"
#include "main_window/CheckUpdatesTask.h"
#include "main_window/MainWindowImpl.h"
#include "main_window/SplashScreen.h"
#include "plugin_viewer/PluginViewerImpl.h"
#include "project_support/ProjectLoaderImpl.h"
#include "project_support/ProjectTasksGui.h"
#include "project_view/ProjectViewImpl.h"
#include "shtirlitz/Shtirlitz.h"
#include "task_view/TaskViewController.h"
#include "welcome_page/WelcomePageController.h"

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
QT_BEGIN_NAMESPACE
extern Q_GUI_EXPORT bool qt_use_native_dialogs;
QT_END_NAMESPACE
#endif

using namespace U2;

static void registerCoreServices() {
    ServiceRegistry* sr = AppContext::getServiceRegistry();
    TaskScheduler* ts = AppContext::getTaskScheduler();
    ts->registerTopLevelTask(sr->registerServiceTask(new PluginViewerImpl()));
    ts->registerTopLevelTask(sr->registerServiceTask(new ProjectViewImpl()));
}

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
    QString appDirPath = AppContext::getWorkingDirectoryPath();
    if( QDir(appDirPath+RELATIVE_DATA_DIR).exists() ) {
        dataSearchPaths.push_back( appDirPath+RELATIVE_DATA_DIR );
    } else if( QDir(appDirPath+RELATIVE_DEV_DATA_DIR).exists() ) {          //data location for developers
        dataSearchPaths.push_back( appDirPath+RELATIVE_DEV_DATA_DIR );
    }

#if (defined(Q_OS_UNIX)) && defined( UGENE_DATA_DIR )
    //using directory which is set during installation process on Linux
    QString ugene_data_dir( UGENE_DATA_DIR );
    if( QDir(ugene_data_dir).exists() ) {
        dataSearchPaths.push_back( QString(UGENE_DATA_DIR) );
    }
#endif

    if( dataSearchPaths.empty() ) {
        dataSearchPaths.push_back("/");
    }

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
        return;
    }
    LogSettings ls;
    ls.reinitAll();
    if(ls.toFile){
        logsCache.setFileOutputEnabled(ls.outputFile);
    }
}

static void initOptionsPanels() {
    OPWidgetFactoryRegistry *opWidgetFactoryRegistry = AppContext::getOPWidgetFactoryRegistry();
    OPCommonWidgetFactoryRegistry *opCommonWidgetFactoryRegistry = AppContext::getOPCommonWidgetFactoryRegistry();

    // Sequence View groups
    opWidgetFactoryRegistry->registerFactory(new FindPatternWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new AnnotHighlightWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new SequenceInfoFactory());

    opWidgetFactoryRegistry->registerFactory(new DasWidgetFactory());

    // Assembly Browser groups
    opWidgetFactoryRegistry->registerFactory(new AssemblyNavigationWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new AssemblyInfoWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new AssemblySettingsWidgetFactory());

    //MSA groups
    MSAGeneralTabFactory *msaGeneralTabFactory = new MSAGeneralTabFactory();
    QString msaGeneralId = msaGeneralTabFactory->getOPGroupParameters().getGroupId();
    opWidgetFactoryRegistry->registerFactory(msaGeneralTabFactory);

    MSAHighlightingFactory *msaHighlightingFactory = new MSAHighlightingFactory();
    QString msaHighlightingId = msaHighlightingFactory->getOPGroupParameters().getGroupId();
    opWidgetFactoryRegistry->registerFactory(msaHighlightingFactory);

    opWidgetFactoryRegistry->registerFactory(new PairAlignFactory());
    opWidgetFactoryRegistry->registerFactory(new MSATreeOptionsWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new AddTreeWidgetFactory());
    opWidgetFactoryRegistry->registerFactory(new MSAExportConsensusFactoryTab());

    SeqStatisticsWidgetFactory *msaSeqStatisticvsFactory = new SeqStatisticsWidgetFactory();
    QString msaSeqStatisticsId = msaSeqStatisticvsFactory->getOPGroupParameters().getGroupId();
    opWidgetFactoryRegistry->registerFactory(msaSeqStatisticvsFactory);

    // MSA common widgets
    QList<QString> groupIds;
    groupIds << msaHighlightingId << msaSeqStatisticsId << msaGeneralId;
    RefSeqCommonWidgetFactory *refSeqCommonWidget = new RefSeqCommonWidgetFactory(groupIds);
    opCommonWidgetFactoryRegistry->registerFactory(refSeqCommonWidget);

    //Tree View groups
    opWidgetFactoryRegistry->registerFactory(new TreeOptionsWidgetFactory());
}

static void initProjectFilterTaskRegistry() {
    ProjectFilterTaskRegistry *registry = AppContext::getProjectFilterTaskRegistry();

    registry->registerTaskFactory(new ObjectNameFilterTaskFactory);
    registry->registerTaskFactory(new MsaSeqNameFilterTaskFactory);
    registry->registerTaskFactory(new SequenceContentFilterTaskFactory);
    registry->registerTaskFactory(new MsaContentFilterTaskFactory);
    registry->registerTaskFactory(new TextContentFilterTaskFactory);
    registry->registerTaskFactory(new SequenceAccFilterTaskFactory);
    registry->registerTaskFactory(new FeatureKeyFilterTaskFactory);
}

class GApplication: public QApplication {
public:
    GApplication(int & argc, char ** argv): QApplication(argc, argv) {}

    bool event(QEvent *event) {
        if (QEvent::FileOpen == event->type()) {
            QStringList urls(static_cast<QFileOpenEvent *>(event)->file());
            openAfterPluginsLoaded(urls);
            return true;
        }
        return QApplication::event(event);
    }

    void openAfterPluginsLoaded(const QStringList& urls, TaskStarter::StartCondition condition = TaskStarter::NoCondition) {
        OpenWithProjectTask * task = new OpenWithProjectTask(urls);

        TaskStarter* taskStarter = new TaskStarter(task, condition);
        if (AppContext::getMainWindow()->getQMainWindow()->isVisible()) {
            taskStarter->registerTask();
        } else {
            connect(AppContext::getMainWindow(), SIGNAL(si_show()), taskStarter, SLOT(registerTask()));
        }
    }
};

static QString findKey(const QStringList& envList, const QString& key) {
    QString prefix = key + "=";
    QString result;
    foreach(const QString& var, envList) {
        if (var.startsWith(prefix)) {
            result = var.mid(prefix.length());
            break;
        }
    }
    return result;
}

namespace {
#ifdef Q_OS_MACX
void fixMacFonts() {
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8) {
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        // the solution is taken from http://successfulsoftware.net/2013/10/23/fixing-qt-4-for-mac-os-x-10-9-mavericks/
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
}
#endif
}

int main(int argc, char **argv) 
{
    if (CrashHandler::isEnabled()) {
        CrashHandler::setupHandler();
    }

    QT_REQUIRE_VERSION( argc, argv, QT_VERSION_STR );

    GTIMER(c1, t1, "main()->QApp::exec");

#ifdef Q_OS_MACX
    fixMacFonts();
#endif
    
    //QApplication app(argc, argv);
    GApplication app(argc, argv);

    QMainWindow window;
    SplashScreen *splashScreen = new SplashScreen(&window);
    splashScreen->show(); //call show() method first, because you don't know actual size of splash screen
    QRect primaryDesktopRect = QApplication::desktop()->availableGeometry(QApplication::desktop()->primaryScreen());
    QRect splashScreenRect = splashScreen->rect();
    splashScreen->move(primaryDesktopRect.width()/2 - splashScreenRect.width()/2, primaryDesktopRect.height()/2 - splashScreenRect.height()/2);

    AppContextImpl* appContext = AppContextImpl::getApplicationContext();
    appContext->setGUIMode(true);
    appContext->setWorkingDirectoryPath(QCoreApplication::applicationDirPath());

    QCoreApplication::addLibraryPath(AppContext::getWorkingDirectoryPath());

    // add some extra paths used during development
#ifdef Q_OS_WIN
#ifdef _DEBUG
    QString devPluginsPath = QDir(AppContext::getWorkingDirectoryPath() + "/../../extras/windows/dotnet_style/_debug").absolutePath();
#else 
    QString devPluginsPath = QDir(AppContext::getWorkingDirectoryPath() + "/../../extras/windows/dotnet_style/_release").absolutePath();
#endif
    QCoreApplication::addLibraryPath(devPluginsPath); //dev version
#endif

    setSearchPaths();
    
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
    

    bool trOK = false;
    QTranslator translator;
    QStringList envList = QProcess::systemEnvironment();
    QString envTranslation = findKey(envList, "UGENE_TRANSLATION");
    if (!envTranslation.isEmpty()) {
        trOK = translator.load(QString("transl_") + envTranslation, AppContext::getWorkingDirectoryPath());
    }

    if (!trOK) {
        // set translations
        QString transFile[] = {
            userAppSettings->getTranslationFile(),
            "transl_" + QLocale::system().name().left(2),
            "transl_en"
        };
        for (int i = transFile[0].isEmpty() ? 1 : 0; i < 3; ++i) {
            if (!translator.load(transFile[i], AppContext::getWorkingDirectoryPath())) {
                fprintf(stderr, "Translation not found: %s\n", transFile[i].toLatin1().constData());
            } else {
                trOK = true;
                break;
            }
        }
        if (!trOK) {
            fprintf(stderr, "No translations found, exiting\n");
            return 1;   
        }
    }

    app.installTranslator(&translator);
    updateStaticTranslations();

    ToolsMenu::init();

    // 3 create functional components of ugene
    LogCacheExt logsCache;
    initLogsCache(logsCache, envList);
    LogCache::setAppGlobalInstance(&logsCache);
    app.installEventFilter(new UserActionsWriter());
    coreLog.details(UserAppsSettings::tr("UGENE initialization started"));
    GCOUNTER( cvar, tvar, "ugeneui" );

    coreLog.trace(QString("UGENE run at dir %1 with parameters %2").arg(AppContext::getWorkingDirectoryPath()).arg(app.arguments().join(" ")));

    //print some settings info, can't do it earlier than logging is initialized
    coreLog.trace(QString("Active UGENE.ini file : %1").arg(AppContext::getSettings()->fileName()));
//    logCat.trace(QString("INI-var: %1").arg(AppContext::getCMDLineRegistry()->getParameterValue( CMDLineCoreOptions::INI_FILE)));


#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
    bool useNative = envList.indexOf("UGENE_USE_NATIVE_DIALOGS=false") < 0 && envList.indexOf("UGENE_USE_NATIVE_DIALOGS=0") < 0;
    qt_use_native_dialogs = useNative; //using local here for debugger
#endif

#if defined(Q_OS_UNIX)
    if (envList.indexOf("UGENE_GUI_TEST=1") >= 0) {
        QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    }
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
    mw->prepare();
  
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

    UdrSchemaRegistry *schemaRegistry = new UdrSchemaRegistry();
    appContext->setUdrSchemaRegistry(schemaRegistry);

    U2DbiRegistry *dbiRegistry = new U2DbiRegistry();
    appContext->setDbiRegistry(dbiRegistry);
    
    DocumentFormatRegistryImpl* dfr = new DocumentFormatRegistryImpl();
    appContext->setDocumentFormatRegistry(dfr);
    ImportDialogFactories::registerFactories();
    
    IOAdapterRegistryImpl* io = new IOAdapterRegistryImpl();
    appContext->setIOAdapterRegistry(io);

    DNATranslationRegistry* dtr = new DNATranslationRegistry();
    appContext->setDNATranslationRegistry(dtr);

    DNAAlphabetRegistry* dal = new DNAAlphabetRegistryImpl(dtr);
    appContext->setDNAAlphabetRegistry(dal);

    DBXRefRegistry* dbxr = new DBXRefRegistry();
    appContext->setDBXRefRegistry(dbxr);

    MSAColorSchemeRegistry* mcsr = new MSAColorSchemeRegistry();
    appContext->setMSAColorSchemeRegistry(mcsr);

    AppContext::getAppSettingsGUI()->registerPage(new ColorSchemaSettingsPageController(mcsr));

    MSAHighlightingSchemeRegistry* mhsr = new MSAHighlightingSchemeRegistry();
    appContext->setMSAHighlightingSchemeRegistry(mhsr);

    MSAConsensusAlgorithmRegistry* msaConsReg = new MSAConsensusAlgorithmRegistry();
    appContext->setMSAConsensusAlgorithmRegistry(msaConsReg);

    AssemblyConsensusAlgorithmRegistry* assemblyConsReg = new AssemblyConsensusAlgorithmRegistry();
    appContext->setAssemblyConsensusAlgorithmRegistry(assemblyConsReg);

    MSADistanceAlgorithmRegistry* msaDistReg = new MSADistanceAlgorithmRegistry();
    appContext->setMSADistanceAlgorithmRegistry(msaDistReg);

    PWMConversionAlgorithmRegistry* pwmConvReg = new PWMConversionAlgorithmRegistry();
    appContext->setPWMConversionAlgorithmRegistry(pwmConvReg);

    SubstMatrixRegistry* smr = new SubstMatrixRegistry();
    appContext->setSubstMatrixRegistry(smr);

    SmithWatermanTaskFactoryRegistry* swar = new SmithWatermanTaskFactoryRegistry();
    appContext->setSmithWatermanTaskFactoryRegistry(swar);

    SWMulAlignResultNamesTagsRegistry* swmarntr = new SWMulAlignResultNamesTagsRegistry();
    appContext->setSWMulAlignResultNamesTagsRegistry(swmarntr);

    RepeatFinderTaskFactoryRegistry* rfr = new RepeatFinderTaskFactoryRegistry();
    appContext->setRepeatFinderTaskFactoryRegistry(rfr);

    ScriptingToolRegistry* str = new ScriptingToolRegistry();
    appContext->setScriptingToolRegistry(str);

    ExternalToolRegistry* etr = new ExternalToolRegistry();
    appContext->setExternalToolRegistry(etr);

    QDActorPrototypeRegistry* qpr = new QDActorPrototypeRegistry();
    appContext->setQDActorFactoryRegistry(qpr);

    CDSearchFactoryRegistry* cdsfr = new CDSearchFactoryRegistry();
    appContext->setCDSearchFactoryRegistry(cdsfr);

    StructuralAlignmentAlgorithmRegistry *saar = new StructuralAlignmentAlgorithmRegistry();
    appContext->setStructuralAlignmentAlgorithmRegistry(saar);

    PhyTreeGeneratorRegistry* genRegistry = new PhyTreeGeneratorRegistry();
    appContext->setPhyTreeGeneratorRegistry(genRegistry);

    MolecularSurfaceFactoryRegistry* msfr = new MolecularSurfaceFactoryRegistry();
    appContext->setMolecularSurfaceFactoryRegistry(msfr);

    SWResultFilterRegistry* swrfr = new SWResultFilterRegistry();
    appContext->setSWResultFilterRegistry(swrfr);

    SecStructPredictAlgRegistry* sspar = new SecStructPredictAlgRegistry();
    appContext->setSecStructPedictAlgRegistry(sspar);

    CudaGpuRegistry* cgr = new CudaGpuRegistry();
    appContext->setCudaGpuRegistry(cgr);

#ifdef OPENCL_SUPPORT
    OpenCLGpuRegistry* oclgr = new OpenCLGpuRegistry();
    appContext->setOpenCLGpuRegistry( oclgr );
#endif

    VirtualFileSystemRegistry* vfsReg = new VirtualFileSystemRegistry();
    appContext->setVirtualFileSystemRegistry( vfsReg );

    DnaAssemblyAlgRegistry* assemblyReg = new DnaAssemblyAlgRegistry();
    appContext->setDnaAssemblyAlgRegistry(assemblyReg);

    GenomeAssemblyAlgRegistry* genomeAssemblyReg = new GenomeAssemblyAlgRegistry();
    appContext->setGenomeAssemblyAlgRegistry(genomeAssemblyReg);

    DnaAssemblySupport assemblySupport;
    Q_UNUSED(assemblySupport);

    DataBaseRegistry *dbr = new DataBaseRegistry();
    appContext->setDataBaseRegistry(dbr);

    SplicedAlignmentTaskRegistry* splicedAlignmentTaskRegistiry = new SplicedAlignmentTaskRegistry();
    appContext->setSplicedAlignmentTaskRegistry(splicedAlignmentTaskRegistiry);

    OPWidgetFactoryRegistry* opWidgetFactoryRegistry = new OPWidgetFactoryRegistry();
    appContext->setOPWidgetFactoryRegistry(opWidgetFactoryRegistry);

    OPCommonWidgetFactoryRegistry *opCommonWidgetFactoryRegistry = new OPCommonWidgetFactoryRegistry();
    appContext->setOPCommonWidgetFactoryRegistry(opCommonWidgetFactoryRegistry);

    WorkflowScriptRegistry* workflowScriptRegistry = new WorkflowScriptRegistry();
    appContext->setWorkflowScriptRegistry(workflowScriptRegistry);

    AlignmentAlgorithmsRegistry* alignmentAlgorithmsRegistry = new AlignmentAlgorithmsRegistry();
    appContext->setAlignmentAlgorithmsRegistry(alignmentAlgorithmsRegistry);

    U2DataPathRegistry* dpr = new U2DataPathRegistry();
    appContext->setDataPathRegistry(dpr);

    DASSourceRegistry* dsr = new DASSourceRegistry();
    appContext->setDASSourceRegistry(dsr);

    CredentialsAsker* credentialsAsker = new CredentialsAskerGui();
    appContext->setCredentialsAsker(credentialsAsker);

    PasswordStorage* passwordStorage = new PasswordStorage();
    appContext->setPasswordStorage(passwordStorage);
    AppSettingsImpl::addPublicDbCredentials2Settings();

    ConvertFactoryRegistry* convertFactoryRegistry = new ConvertFactoryRegistry();
    appContext->setConvertFactoryRegistry(convertFactoryRegistry);

    IdRegistry<WelcomePageAction> *welcomePageActions = new IdRegistry<WelcomePageAction>();
    appContext->setWelcomePageActionRegistry(welcomePageActions);

    ProjectFilterTaskRegistry *projectFilterTaskRegistry = new ProjectFilterTaskRegistry;
    appContext->setProjectFilterTaskRegistry(projectFilterTaskRegistry);
    initProjectFilterTaskRegistry();

    Workflow::WorkflowEnv::init(new Workflow::WorkflowEnvImpl());
    Workflow::WorkflowEnv::getDomainRegistry()->registerEntry(new LocalWorkflow::LocalDomainFactory());

    ServiceRegistryImpl* sreg = new ServiceRegistryImpl() ;
    appContext->setServiceRegistry(sreg);

    PluginSupportImpl* psp = new PluginSupportImpl();
    appContext->setPluginSupport(psp);

    ProjectLoaderImpl* pli = new ProjectLoaderImpl();
    appContext->setProjectLoader(pli);

    CMDLineUtils::init();

    RecentlyDownloadedCache* rdc = new RecentlyDownloadedCache();
    appContext->setRecentlyDownloadedCache(rdc);

    DistributedComputingUtil * dcu = new DistributedComputingUtil();

    AutoAnnotationsSupport* aaSupport = new AutoAnnotationsSupport();
    appContext->setAutoAnnotationsSupport(aaSupport);

    GUITestBase *tb = new GUITestBase();
    appContext->setGUITestBase(tb);

    AppFileStorage *appFileStorage = new AppFileStorage();
    U2OpStatusImpl os;
    appFileStorage->init(os);
    if (os.hasError()) {
        coreLog.error(os.getError());
        delete appFileStorage;
    } else {
        appContext->setAppFileStorage(appFileStorage);
    }

    // Register all Options Panel groups on the required GObjectViews
    initOptionsPanels();

    if(!cmdLineRegistry->hasParameter(CMDLineCoreOptions::LAUNCH_GUI_TEST)) {
        QStringList urls = CMDLineRegistryUtils::getPureValues();

        if(urls.isEmpty() && AppContext::getAppSettings()->getUserAppsSettings()->openLastProjectAtStartup()) {
            QString lastProject = ProjectLoaderImpl::getLastProjectURL();
            if (!lastProject.isEmpty()) {
                urls << lastProject;
            }
        }

        if( !urls.isEmpty() ) {
            // defer loading until all plugins/services loaded
            app.openAfterPluginsLoaded(urls, TaskStarter::NoProject);
        }
    }

    registerCoreServices();

    if ( envList.contains(ENV_GUI_TEST+QString("=1")) ) {
        GUITestService *guiTestService = new GUITestService();
        Q_UNUSED(guiTestService);
    }

    GReportableCounter launchCounter("ugeneui launch", "", 1);
    ++launchCounter.totalCount;

    //3 run QT GUI
    t1.stop();

    //coreLog.info(AppContextImpl::tr("%1-bit version of UGENE started").arg(Version::appArchitecture));
    Version v = Version::appVersion();
    coreLog.info( QObject::tr( "UGENE started" ));
    coreLog.info( QObject::tr( "UGENE version: %1 %2-bit").arg( v.text ).arg( Version::appArchitecture ) );
    coreLog.info( QObject::tr( "UGENE distribution: %1").arg( v.distributionInfo ));
    
    QObject::connect(ts, SIGNAL(si_noTasksInScheduler()), splashScreen, SLOT(sl_close()));
    QObject::connect(ts, SIGNAL(si_noTasksInScheduler()), mw, SLOT(sl_show()));

    WelcomePageController *wpc = new WelcomePageController();
    QObject::connect(ts, SIGNAL(si_noTasksInScheduler()), wpc, SLOT(sl_showPage()));
    QObject::connect(mw, SIGNAL(si_showWelcomePage()), wpc, SLOT(sl_showPage()));
    QObject::connect(pli, SIGNAL(si_recentListChanged()), wpc, SLOT(sl_onRecentChanged()));

    QList<Task*> tasks;

    if(AppContext::getSettings()->getValue(ASK_VESRION_SETTING, true).toBool() && qgetenv("UGENE_GUI_TEST").toInt() != 1) {
        tasks << new CheckUpdatesTask(true);

        TmpDirChecker* tempDirChecker = new TmpDirChecker;
        tasks << tempDirChecker;
        QObject::connect(tempDirChecker, SIGNAL(si_checkFailed(QString)), mw, SLOT(sl_tempDirPathCheckFailed(QString)));
    }
    if ( !envList.contains(ENV_UGENE_DEV+QString("=1")) ) {
        tasks << new ShtirlitzStartupTask();
    }

    mw->registerStartupChecks(tasks);

    MemoryLocker l(160, AppResource::SystemMemory); // 100Mb on UGENE start, ~60Mb SQLite cache
    int rc = app.exec();
    l.release();

    //4 deallocate resources
    if ( !envList.contains(ENV_UGENE_DEV+QString("=1")) ) {
        Shtirlitz::saveGatheredInfo();
    }

    delete wpc;

    delete dcu;

    appContext->setProjectFilterTaskRegistry(NULL);
    delete projectFilterTaskRegistry;

    appContext->setGUITestBase(NULL);
    delete tb;
    
    appContext->setRecentlyDownloadedCache(NULL);
    delete rdc;

    appContext->setProjectLoader(NULL);
    delete pli;

    appContext->setPluginSupport(NULL);
    delete psp;

    appContext->setServiceRegistry(NULL);
    delete sreg;

    Workflow::WorkflowEnv::shutdown();

    appContext->setCredentialsAsker(NULL);
    delete credentialsAsker;

    appContext->setPasswordStorage(NULL);
    delete passwordStorage;

    appContext->setDASSourceRegistry( NULL );
    delete dsr;

    appContext->setDataPathRegistry( NULL );
    delete dpr;

    appContext->setAlignmentAlgorithmsRegistry(NULL);
    delete alignmentAlgorithmsRegistry;

    appContext->setWorkflowScriptRegistry(NULL);
    delete workflowScriptRegistry;

    appContext->setOPCommonWidgetFactoryRegistry(NULL);
    delete opCommonWidgetFactoryRegistry;

    appContext->setOPWidgetFactoryRegistry(NULL);
    delete opWidgetFactoryRegistry;

    appContext->setSplicedAlignmentTaskRegistry(NULL);
    delete splicedAlignmentTaskRegistiry;

    appContext->setDataBaseRegistry(NULL);
    delete dbr;

    appContext->setDnaAssemblyAlgRegistry(NULL);
    delete assemblyReg;

    appContext->setVirtualFileSystemRegistry( NULL );
    delete vfsReg;

#ifdef OPENCL_SUPPORT
    appContext->setOpenCLGpuRegistry(NULL);
    delete oclgr;
#endif

    appContext->setCudaGpuRegistry(NULL);
    delete cgr;

    appContext->setSecStructPedictAlgRegistry(NULL);
    delete sspar;

    appContext->setWelcomePageActionRegistry(NULL);
    delete welcomePageActions;

    appContext->setConvertFactoryRegistry(NULL);
    delete convertFactoryRegistry;

    appContext->setSWResultFilterRegistry(NULL);
    delete swrfr;

    appContext->setMolecularSurfaceFactoryRegistry(NULL);
    delete msfr;

    appContext->setPhyTreeGeneratorRegistry(NULL);
    delete genRegistry;

    appContext->setStructuralAlignmentAlgorithmRegistry(NULL);
    delete saar;

    appContext->setCDSearchFactoryRegistry(NULL);
    delete cdsfr;

    appContext->setQDActorFactoryRegistry(NULL);
    delete qpr;

    appContext->setExternalToolRegistry(NULL);
    delete etr;

    appContext->setScriptingToolRegistry(NULL);
    delete str;

    appContext->setRepeatFinderTaskFactoryRegistry(NULL);
    delete rfr;

    appContext->setSWMulAlignResultNamesTagsRegistry(NULL);
    delete swmarntr;

    appContext->setSmithWatermanTaskFactoryRegistry(NULL);
    delete swar;

    appContext->setSubstMatrixRegistry(NULL);
    delete smr;

    appContext->setPWMConversionAlgorithmRegistry(NULL);
    delete pwmConvReg;

    appContext->setMSADistanceAlgorithmRegistry(NULL);
    delete msaDistReg;

    appContext->setAssemblyConsensusAlgorithmRegistry(NULL);
    delete assemblyConsReg;

    appContext->setMSAConsensusAlgorithmRegistry(NULL);
    delete msaConsReg;

    appContext->setMSAHighlightingSchemeRegistry(NULL);
    delete mhsr;

    appContext->setMSAColorSchemeRegistry(NULL);
    delete mcsr;

    appContext->setDBXRefRegistry(NULL);
    delete dbxr;

    appContext->setDNAAlphabetRegistry(NULL);
    delete dal;

    appContext->setDNATranslationRegistry(NULL);
    delete dtr;

    appContext->setIOAdapterRegistry(NULL);
    delete io;

    appContext->setDocumentFormatRegistry(NULL);
    delete dfr;

    delete dbiRegistry;
    appContext->setDbiRegistry(NULL);

    appContext->setUdrSchemaRegistry(NULL);
    delete schemaRegistry;

    appContext->setObjectViewFactoryRegistry(NULL);
    delete ovfr;

    appContext->setAnnotationSettingsRegistry(NULL);
    delete asr;

    appContext->setCMDLineRegistry(NULL);
    delete cmdLineRegistry;

    appContext->setMainWindow(NULL);
    delete mw;

    appContext->setTestFramework(NULL);
    delete tf;

    appContext->setTaskScheduler(NULL);
    delete ts;

    appContext->setResourceTracker(NULL);
    delete resTrack;

    appContext->setAutoAnnotationsSupport(NULL);
    delete aaSupport;

    bool deleteSettingsFile = userAppSettings->resetSettings();
    QString iniFile = AppContext::getSettings()->fileName();
    
    appContext->setAppSettingsGUI(NULL);
    delete appSettingsGUI;

    appContext->setAppSettings(NULL);
    delete appSettings;

    appContext->setSettings(NULL);
    delete settings;

    appContext->setGlobalSettings(NULL);
    delete globalSettings;

    if (deleteSettingsFile){
        QFile ff;
        ff.remove(iniFile);
    }

    CrashHandler::shutdown();

    return rc;   
}

