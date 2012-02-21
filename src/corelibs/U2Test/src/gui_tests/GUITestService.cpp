/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "GUITestService.h"
#include "GUITestBase.h"
#include "GUITestTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/TaskStarter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObject.h>
#include <U2Core/CMDLineCoreOptions.h>

namespace U2 {

GUITestService::GUITestService(QObject *) : Service(Service_GUITesting, tr("GUI test viewer"), tr("Service to support UGENE GUI testing")),
runTestsAction(NULL), testLauncher(NULL) {
    connect(AppContext::getPluginSupport(), SIGNAL(si_allStartUpPluginsLoaded()), SLOT(sl_registerService()));
}

GUITestService::~GUITestService() {

    delete runTestsAction;
}

void GUITestService::sl_registerService() {

    LaunchOptions launchedFor = getLaunchOptions(AppContext::getCMDLineRegistry());

    switch (launchedFor) {
        case RUN_ONE_TEST:
            registerGUITestTask();
            break;

        case RUN_ALL_TESTS:
            registerAllTestsTask();
            break;

        default:
        case NONE:
            registerServiceTask();
            break;
    }
}

GUITestService::LaunchOptions GUITestService::getLaunchOptions(CMDLineRegistry* cmdLine) const {

    if (launchedToTestGUI(cmdLine)) {
        QString paramValue = cmdLine->getParameterValue(CMDLineCoreOptions::LAUNCH_GUI_TEST);
        if (!paramValue.isEmpty()) {
            return RUN_ONE_TEST;
        }
        return RUN_ALL_TESTS;
    }

    return NONE;
}

bool GUITestService::launchedToTestGUI(CMDLineRegistry* cmdLine) const {

    return cmdLine && cmdLine->hasParameter(CMDLineCoreOptions::LAUNCH_GUI_TEST);
}

void GUITestService::registerAllTestsTask() {

    testLauncher = createTestLauncherTask();
    AppContext::getTaskScheduler()->registerTopLevelTask(testLauncher);

    connect(AppContext::getTaskScheduler(), SIGNAL(si_stateChanged(Task*)), SLOT(sl_taskStateChanged(Task*)));
}

Task* GUITestService::createTestLauncherTask() const {

    Q_ASSERT(!testLauncher);

    Task *task = new GUITestLauncher();
    Q_ASSERT(task);

    return task;
}

void GUITestService::registerGUITestTask() const {

    GUITest *t = getTest();
    Q_ASSERT(t);

    if (t) {
        GUITestTask *task = new GUITestTask(t);
        Q_ASSERT(task);

        AppContext::getTaskScheduler()->registerTopLevelTask(task);
    }
}

void GUITestService::registerServiceTask() {

    Task *registerServiceTask = AppContext::getServiceRegistry()->registerServiceTask(this);
    Q_ASSERT(registerServiceTask);

    AppContext::getTaskScheduler()->registerTopLevelTask(registerServiceTask);
}

GUITest* GUITestService::getTest() const {

    CMDLineRegistry* cmdLine = AppContext::getCMDLineRegistry();
    Q_ASSERT(cmdLine);

    GUITestBase *tb = AppContext::getGUITestBase();
    Q_ASSERT(tb);

    QString testName = cmdLine->getParameterValue(CMDLineCoreOptions::LAUNCH_GUI_TEST);
    GUITest *t = tb->getTest(testName);

    return t;
}

void GUITestService::serviceStateChangedCallback(ServiceState, bool enabledStateChanged) {

    if (!enabledStateChanged) {
        return;
    }

    if (isEnabled()) {
        addServiceMenuItem();
    } else {
        deleteServiceMenuItem();
    }
}

void GUITestService::deleteServiceMenuItem() {

    delete runTestsAction; runTestsAction = NULL;
}

void GUITestService::addServiceMenuItem() {

    deleteServiceMenuItem();
    runTestsAction = new QAction(tr("GUI testing"), this);
    Q_ASSERT(runTestsAction);
    runTestsAction->setObjectName("action_guitest");

    connect(runTestsAction, SIGNAL(triggered()), SLOT(sl_registerTestLauncherTask()));
    AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS)->addAction(runTestsAction);
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
    if (launchedFor == RUN_ALL_TESTS) {
        AppContext::getTaskScheduler()->cancelAllTasks();
        AppContext::getMainWindow()->getQMainWindow()->close();
    }
}

}
