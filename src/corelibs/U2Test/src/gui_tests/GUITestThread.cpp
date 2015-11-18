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
#include <QDesktopWidget>

#include <QGuiApplication>
#include <U2Core/AppContext.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Test/MainThreadRunnable.h>

#include "GUITest.h"
#include "GUITestBase.h"
#include "GUITestOpStatus.h"
#include "GUITestService.h"
#include "GUITestTeamcityLogger.h"
#include "GUITestThread.h"

namespace U2 {

GUITestThread::GUITestThread(GUITest *test, Logger &log, bool _needCleanup) :
    test(test),
    log(log),
    needCleanup(_needCleanup),
    testResult("Not runned")
{
    SAFE_POINT(NULL != test, "GUITest is NULL", );
}

void GUITestThread::run() {
    SAFE_POINT(NULL != test, "GUITest is NULL", );

    GUITests tests;
    tests << preChecks();
    tests << test;
    tests << postChecks();

    clearSandbox();

    const QString error = launchTest(tests);

    if(needCleanup){
        cleanup();
    }

    testResult = error.isEmpty() ? GUITestTeamcityLogger::successResult : error;
    writeTestResult();

    exit();
}

void GUITestThread::sl_testTimeOut() {
    saveScreenshot();
    cleanup();
    testResult = QString("test timed out");
    writeTestResult();
    exit();
}

QString GUITestThread::launchTest(const GUITests &tests) {
    QTimer::singleShot(test->getTimeout(), this, SLOT(sl_testTimeOut()));

    GUITestOpStatus os;
    try {
        foreach (GUITest *t, tests) {
            if (NULL != t) {
                t->run(os);
            }
        }
    } catch(GUITestOpStatus *) {

    }
    QString result = os.getError();

    //Run post checks if has error
    if (!result.isEmpty()){
        try {
            foreach (GUITest *t, postChecks()) {
                if (NULL != t) {
                    t->run(os);
                }
            }
        } catch(GUITestOpStatus *) {

        }
    }

    return result;
}

GUITests GUITestThread::preChecks() {
    GUITestBase *tb = AppContext::getGUITestBase();
    SAFE_POINT(NULL != tb, "GUITestBase is NULL", GUITests());

//    GUITests additionalChecks = tb->takeTests(GUITestBase::PreAdditional);
    GUITests additionalChecks = tb->getTests(GUITestBase::PreAdditional);
    SAFE_POINT(!additionalChecks.isEmpty(), "additionalChecks is empty", GUITests());

    return additionalChecks;
}

GUITests GUITestThread::postChecks() {
    GUITestBase *tb = AppContext::getGUITestBase();
    SAFE_POINT(NULL != tb, "GUITestBase is NULL", GUITests());

//    GUITests additionalChecks = tb->takeTests(GUITestBase::PostAdditionalChecks);
    GUITests additionalChecks = tb->getTests(GUITestBase::PostAdditionalChecks);
    SAFE_POINT(!additionalChecks.isEmpty(), "additionalChecks is empty", GUITests());

    return additionalChecks;
}

GUITests GUITestThread::postActions() {
    GUITestBase *tb = AppContext::getGUITestBase();
    SAFE_POINT(NULL != tb, "GUITestBase is NULL", GUITests());

//    GUITests additionalChecks = tb->takeTests(GUITestBase::PostAdditionalActions);
    GUITests additionalChecks = tb->getTests(GUITestBase::PostAdditionalActions);
    SAFE_POINT(!additionalChecks.isEmpty(), "additionalChecks is empty", GUITests());

    return additionalChecks;
}

void GUITestThread::clearSandbox() {
    log.trace("GUITestThread __ clearSandbox");

    const QString pathToSandbox = GUITest::testDir + "_common_data/scenarios/sandbox/";
    QDir sandbox(pathToSandbox);

    foreach (const QString &fileName, sandbox.entryList()) {
        if (fileName != "." && fileName != "..") {
            if (QFile::remove(pathToSandbox + fileName)) {
                continue;
            } else {
                QDir dir(pathToSandbox + fileName);
                removeDir(dir.absolutePath());
            }
        }
    }
}

void GUITestThread::removeDir(const QString &dirName) {
    QDir dir(dirName);

    foreach (const QFileInfo &fileInfo, dir.entryInfoList()) {
        const QString fileName = fileInfo.fileName();
        const QString filePath = fileInfo.filePath();
        if (fileName != "." && fileName != "..") {
            if (QFile::remove(filePath)) {
                continue;
            } else {
                QDir dir(filePath);
                if (dir.rmdir(filePath)) {
                    continue;
                } else {
                    removeDir(filePath);
                }
            }
        }
    }
    dir.rmdir(dir.absoluteFilePath(dirName));
}

void GUITestThread::saveScreenshot() {
    class Scenario : public CustomScenario {
    public:
        Scenario(GUITest *test) :
            test(test)
        {

        }

        void run(U2OpStatus &) {
            const QPixmap originalPixmap = QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());
            originalPixmap.save(GUITest::screenshotDir + test->getFullName() + ".jpg");
        }

    private:
        GUITest *test;
    };

    U2OpStatusImpl os;
    MainThreadRunnable::runInMainThread(os, new Scenario(test));
}

void GUITestThread::cleanup() {
    foreach (GUITest *postAction, postActions()) {
        TaskStateInfo os;
        postAction->run(os);
    }
}

void GUITestThread::writeTestResult() {
    printf("%s\n", (GUITestService::GUITESTING_REPORT_PREFIX + ": " + testResult).toUtf8().data());
}

}   // namespace U2
