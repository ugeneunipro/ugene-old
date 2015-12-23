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

#include <QMainWindow>
#include <QScreen>

#include <U2Core/AppContext.h>
#include <U2Core/CMDLineCoreOptions.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/GObject.h>
#include <U2Core/Log.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/TaskStarter.h>
#include <U2Core/Timer.h>
#include <U2Core/U2SafePoints.h>

#include <core/MainThreadRunnable.h>
#include "UGUITestBase.h"
#include <core/GUITestOpStatus.h>
#include "GUITestService.h"
#include "GUITestTeamcityLogger.h"
#include "GUITestThread.h"
#include "GUITestWindow.h"
#include "UGUITest.h"

/**************************************************** to use qt file dialog *************************************************************/
#ifdef Q_OS_LINUX
#if (QT_VERSION < 0x050000) //Qt 5
typedef QStringList(*_qt_filedialog_open_filenames_hook)(QWidget * parent, const QString &caption, const QString &dir,
                                                          const QString &filter, QString *selectedFilter, QFileDialog::Options options);
typedef QString(*_qt_filedialog_open_filename_hook)     (QWidget * parent, const QString &caption, const QString &dir,
                                                          const QString &filter, QString *selectedFilter, QFileDialog::Options options);
typedef QString(*_qt_filedialog_save_filename_hook)     (QWidget * parent, const QString &caption, const QString &dir,
                                                          const QString &filter, QString *selectedFilter, QFileDialog::Options options);
typedef QString(*_qt_filedialog_existing_directory_hook)(QWidget *parent, const QString &caption, const QString &dir,
                                                          QFileDialog::Options options);

extern Q_GUI_EXPORT _qt_filedialog_open_filename_hook qt_filedialog_open_filename_hook;
extern Q_GUI_EXPORT _qt_filedialog_open_filenames_hook qt_filedialog_open_filenames_hook;
extern Q_GUI_EXPORT _qt_filedialog_save_filename_hook qt_filedialog_save_filename_hook;
extern Q_GUI_EXPORT _qt_filedialog_existing_directory_hook qt_filedialog_existing_directory_hook;
#endif
#endif
/******************************************************************************************************************************************/

namespace U2 {

#define ULOG_CAT_TEAMCITY "Teamcity Log"
static Logger log(ULOG_CAT_TEAMCITY);
const QString GUITestService::GUITESTING_REPORT_PREFIX = "GUITesting";
const qint64 GUITestService::TIMER_INTERVAL = 100;

GUITestService::GUITestService(QObject *) :
    Service(Service_GUITesting, tr("GUI test viewer"), tr("Service to support UGENE GUI testing")),
    runTestsAction(NULL),
    testLauncher(NULL)
{
    connect(AppContext::getPluginSupport(), SIGNAL(si_allStartUpPluginsLoaded()), SLOT(sl_allStartUpPluginsLoaded()));
    setQtFileDialogView();
}

GUITestService::~GUITestService() {
    delete runTestsAction;
}

GUITestService *GUITestService::getGuiTestService() {
    QList<Service *> services = AppContext::getServiceRegistry()->findServices(Service_GUITesting);
    return services.isEmpty() ? NULL : qobject_cast<GUITestService *>(services.first());
}

void GUITestService::sl_registerService() {
    registerServiceTask();
}

void GUITestService::sl_serviceRegistered() {
    const LaunchOptions launchedFor = getLaunchOptions(AppContext::getCMDLineRegistry());

    switch (launchedFor) {
        case RUN_ONE_TEST:
            QTimer::singleShot(1000, this, SLOT(runGUITest()));
            break;

        case RUN_ALL_TESTS:
            registerAllTestsTask();
            break;

        case RUN_TEST_SUITE:
            registerTestSuiteTask();
            break;

        case RUN_ALL_TESTS_BATCH:
            QTimer::singleShot(1000, this, SLOT(runAllGUITests()));
            break;

        case RUN_CRAZY_USER_MODE:
            QTimer::singleShot(1000, this, SLOT(runGUICrazyUserTest()));
            break;

        case CREATE_GUI_TEST:
            new GUITestingWindow();
            break;

        case RUN_ALL_TESTS_NO_IGNORED:
            registerAllTestsTaskNoIgnored();
            break;

        case NONE:
        default:
            break;
    }
}

GUITestService::LaunchOptions GUITestService::getLaunchOptions(CMDLineRegistry* cmdLine) const {
    CHECK(cmdLine, NONE);

    if(cmdLine->hasParameter(CMDLineCoreOptions::CREATE_GUI_TEST)){
        return CREATE_GUI_TEST;
    }


    if (cmdLine->hasParameter(CMDLineCoreOptions::LAUNCH_GUI_TEST)) {
        QString paramValue = cmdLine->getParameterValue(CMDLineCoreOptions::LAUNCH_GUI_TEST);
        if (!paramValue.isEmpty()) {
            return RUN_ONE_TEST;
        }
        return RUN_ALL_TESTS;
    }

    if (cmdLine->hasParameter(CMDLineCoreOptions::LAUNCH_GUI_TEST_BATCH)) {
        return RUN_ALL_TESTS_BATCH;
    }

    if (cmdLine->hasParameter(CMDLineCoreOptions::LAUNCH_GUI_TEST_SUITE)) {
        return RUN_TEST_SUITE;
    }

    if (cmdLine->hasParameter(CMDLineCoreOptions::LAUNCH_GUI_TEST_NO_IGNORED)) {
        return RUN_ALL_TESTS_NO_IGNORED;
    }

    if (cmdLine->hasParameter(CMDLineCoreOptions::LAUNCH_GUI_TEST_CRAZY_USER)) {
        return RUN_CRAZY_USER_MODE;
    }

    return NONE;
}

void GUITestService::registerAllTestsTask() {

    testLauncher = createTestLauncherTask();
    AppContext::getTaskScheduler()->registerTopLevelTask(testLauncher);

    connect(AppContext::getTaskScheduler(), SIGNAL(si_stateChanged(Task*)), SLOT(sl_taskStateChanged(Task*)));
}

void GUITestService::registerAllTestsTaskNoIgnored() {

    testLauncher = createTestLauncherTask(0, true);
    AppContext::getTaskScheduler()->registerTopLevelTask(testLauncher);

    connect(AppContext::getTaskScheduler(), SIGNAL(si_stateChanged(Task*)), SLOT(sl_taskStateChanged(Task*)));
}

Task* GUITestService::createTestLauncherTask(int suiteNumber, bool noIgnored) const {
    SAFE_POINT(NULL == testLauncher,"",NULL);

    Task *task = new GUITestLauncher(suiteNumber, noIgnored);
    return task;
}

void GUITestService::registerTestSuiteTask(){
    testLauncher = createTestSuiteLauncherTask();
    AppContext::getTaskScheduler()->registerTopLevelTask(testLauncher);

    connect(AppContext::getTaskScheduler(), SIGNAL(si_stateChanged(Task*)), this, SLOT(sl_taskStateChanged(Task*)));
}

Task* GUITestService::createTestSuiteLauncherTask() const {

    Q_ASSERT(!testLauncher);

    CMDLineRegistry* cmdLine = AppContext::getCMDLineRegistry();
    Q_ASSERT(cmdLine);

    bool ok;
    int suiteNumber = cmdLine->getParameterValue(CMDLineCoreOptions::LAUNCH_GUI_TEST_SUITE).toInt(&ok);
    if(!ok){
        QString pathToSuite = cmdLine->getParameterValue(CMDLineCoreOptions::LAUNCH_GUI_TEST_SUITE);
        Task *task = new GUITestLauncher(pathToSuite);
        Q_ASSERT(task);
        return task;
    }

    Task *task = new GUITestLauncher(suiteNumber);
    Q_ASSERT(task);

    return task;
}

GUITests GUITestService::preChecks() {

    UGUITestBase* tb = AppContext::getGUITestBase();
    SAFE_POINT(NULL != tb,"",GUITests());

    GUITests additionalChecks = tb->takeTests(UGUITestBase::PreAdditional);
    SAFE_POINT(additionalChecks.size()>0,"",GUITests());

    return additionalChecks;
}

GUITests GUITestService::postChecks() {

    UGUITestBase* tb = AppContext::getGUITestBase();
    SAFE_POINT(NULL != tb,"",GUITests());

    GUITests additionalChecks = tb->takeTests(UGUITestBase::PostAdditionalChecks);
    SAFE_POINT(additionalChecks.size()>0,"",GUITests());

    return additionalChecks;
}

GUITests GUITestService::postActions() {

    UGUITestBase* tb = AppContext::getGUITestBase();
    SAFE_POINT(NULL != tb,"",GUITests());

    GUITests additionalChecks = tb->takeTests(UGUITestBase::PostAdditionalActions);
    SAFE_POINT(additionalChecks.size()>0,"",GUITests());

    return additionalChecks;
}

void GUITestService::sl_allStartUpPluginsLoaded() {
    if (!connect(AppContext::getExternalToolRegistry()->getManager(), SIGNAL(si_startupChecksFinish()), SLOT(sl_registerService()))) {
        coreLog.error(tr("Can't connect external tool manager signal"));
        sl_registerService();
    }
}

void GUITestService::runAllGUITests() {

    GUITests initTests = preChecks();
    GUITests postChecksTests = postChecks();
    GUITests postActiosTests = postActions();

    GUITests tests = AppContext::getGUITestBase()->takeTests();
    SAFE_POINT(false == tests.isEmpty(),"",);

    foreach(HI::GUITest* t, tests) {
        SAFE_POINT(NULL != t,"",);
        if (!t) {
            continue;
        }
        QString testName = t->getFullName();
        QString testNameForTeamCity = t->getSuite() +"_"+ t->getName();

        if (t->isIgnored()) {
            GUITestTeamcityLogger::testIgnored(testNameForTeamCity, t->getIgnoreMessage());
            continue;
        }

        qint64 startTime = GTimer::currentTimeMicros();
        GUITestTeamcityLogger::testStarted(testNameForTeamCity);

        HI::GUITestOpStatus os;
        log.trace("GTRUNNER - runAllGUITests - going to run initial checks before " + testName);
        foreach(HI::GUITest* t, initTests) {
            if (t) {
                t->run(os);
            }
        }

        clearSandbox();
        log.trace("GTRUNNER - runAllGUITests - going to run test " + testName);
        t->run(os);
        log.trace("GTRUNNER - runAllGUITests - finished running test " + testName);

        foreach(HI::GUITest* t, postChecksTests) {
            if (t) {
                t->run(os);
            }
        }

        HI::GUITestOpStatus os2;
        foreach(HI::GUITest* t, postActiosTests) {
            if (t) {
                t->run(os2);
            }
        }

        QString testResult = os.hasError() ? os.getError() : GUITestTeamcityLogger::successResult;

        qint64 finishTime = GTimer::currentTimeMicros();
        GUITestTeamcityLogger::teamCityLogResult(testNameForTeamCity, testResult, GTimer::millisBetween(startTime, finishTime));
    }

    log.trace("GTRUNNER - runAllGUITests - shutting down UGENE");
    AppContext::getTaskScheduler()->cancelAllTasks();
    AppContext::getMainWindow()->getQMainWindow()->close();
}

void GUITestService::runGUITest() {

    CMDLineRegistry* cmdLine = AppContext::getCMDLineRegistry();
    SAFE_POINT(NULL != cmdLine,"",);
    QString testName = cmdLine->getParameterValue(CMDLineCoreOptions::LAUNCH_GUI_TEST);

    UGUITestBase *tb = AppContext::getGUITestBase();
    SAFE_POINT(NULL != tb,"",);
    HI::GUITest *t = tb->takeTest(testName.split(":").first(), testName.split(":").last());

    runGUITest(t);
}

void GUITestService::runGUICrazyUserTest() {
    UGUITestBase *tb = AppContext::getGUITestBase();
    SAFE_POINT(tb,"",);
    HI::GUITest *t = tb->takeTest("","simple_crazy_user");

    runGUITest(t);
}

void GUITestService::runGUITest(HI::GUITest *test) {
    SAFE_POINT(NULL != test, "GUITest is NULL", );
    GUITestThread *testThread = new GUITestThread(test, log);
    connect(testThread, SIGNAL(finished()), SLOT(sl_testThreadFinish()));
    testThread->start();
}

void GUITestService::registerServiceTask() {
    Task *registerServiceTask = AppContext::getServiceRegistry()->registerServiceTask(this);
    SAFE_POINT(NULL != registerServiceTask, "registerServiceTask is NULL", );
    connect(new TaskSignalMapper(registerServiceTask), SIGNAL(si_taskFinished(Task *)), SLOT(sl_serviceRegistered()));

    AppContext::getTaskScheduler()->registerTopLevelTask(registerServiceTask);
}

void GUITestService::serviceStateChangedCallback(ServiceState, bool enabledStateChanged) {

    if (!enabledStateChanged) {
        return;
    }
}

void GUITestService::sl_registerTestLauncherTask() {

    registerAllTestsTask();
}

void GUITestService::sl_taskStateChanged(Task* t) {

    if (t != testLauncher) {
        return;
    }
    if (!t->isFinished()) {
        return;
    }

    testLauncher = NULL;
    AppContext::getTaskScheduler()->disconnect(this);

    LaunchOptions launchedFor = getLaunchOptions(AppContext::getCMDLineRegistry());
    if (launchedFor == RUN_ALL_TESTS || RUN_TEST_SUITE) {
        AppContext::getTaskScheduler()->cancelAllTasks();
        AppContext::getMainWindow()->getQMainWindow()->close();
    }
}

void GUITestService::writeTestResult(const QString& result) {
    printf("%s\n", (QString(GUITESTING_REPORT_PREFIX) + ": " + result).toUtf8().data());
}

void GUITestService::setQtFileDialogView()
{
#ifdef Q_OS_LINUX
#if (QT_VERSION < 0x050000) //Qt 5
    if (!qgetenv("UGENE_USE_NATIVE_DIALOGS").isEmpty()) {//this condition does not controls native dialogs
    //if (qgetenv("UGENE_GUI_TEST").toInt() == 1 && qgetenv("UGENE_USE_NATIVE_DIALOGS").toInt() == 0) {
        qt_filedialog_open_filename_hook = 0;
        qt_filedialog_open_filenames_hook = 0;
        qt_filedialog_save_filename_hook = 0;
        qt_filedialog_existing_directory_hook = 0;
    }
#endif
#endif
}

void GUITestService::clearSandbox()
{
    log.trace("GUITestService __ clearSandbox");

    QString pathToSandbox = UGUITest::testDir + "_common_data/scenarios/sandbox/";
    QDir sandbox(pathToSandbox);

    foreach (QString fileName, sandbox.entryList()) {
        if (fileName != "." && fileName != "..") {
            if(QFile::remove(pathToSandbox + fileName))
                continue;
            else{
                QDir dir(pathToSandbox + fileName);
                removeDir(dir.absolutePath());
            }
        }
    }
}

void GUITestService::removeDir(QString dirName)
{
    QDir dir(dirName);


    foreach (QFileInfo fileInfo, dir.entryInfoList()) {
        QString fileName = fileInfo.fileName();
        QString filePath = fileInfo.filePath();
        if (fileName != "." && fileName != "..") {
            if(QFile::remove(filePath))
                continue;
            else{
                QDir dir(filePath);
                if(dir.rmdir(filePath))
                    continue;
                else
                    removeDir(filePath);
            }

        }
    }dir.rmdir(dir.absoluteFilePath(dirName));
}

void GUITestService::sl_testThreadFinish() {
    sender()->deleteLater();
    AppContext::getMainWindow()->getQMainWindow()->close();
}

}
