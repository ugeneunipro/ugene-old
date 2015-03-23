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

#include <QtCore/QMap>

#include <U2Core/AppContext.h>
#include <U2Core/CMDLineCoreOptions.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/Timer.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/MainWindow.h>

#include "GUITestBase.h"
#include "GUITestLauncher.h"
#include "GUITestService.h"
#include "GUITestTeamcityLogger.h"

#define TIMEOUT 480000

#ifdef Q_OS_LINUX
#define NUMBER_OF_TESTS_IN_SUTIE 450
#endif
#ifdef Q_OS_MAC
#define NUMBER_OF_TESTS_IN_SUTIE 600
#endif
#ifdef Q_OS_WIN
#define NUMBER_OF_TESTS_IN_SUTIE 850
#endif
#define GUITESTING_REPORT_PREFIX "GUITesting"

namespace U2 {

GUITestLauncher::GUITestLauncher(int _suiteNumber, bool _noIgnored)
    : Task("gui_test_launcher", TaskFlags(TaskFlag_ReportingIsSupported) | TaskFlag_ReportingIsEnabled),
      suiteNumber(_suiteNumber), noIgnored(_noIgnored), pathToSuite("") {

    tpm = Task::Progress_Manual;
    testOutDir = getTestOutDir();
}

GUITestLauncher::GUITestLauncher(QString _pathToSuite, bool _noIgnored)
    : Task("gui_test_launcher", TaskFlags(TaskFlag_ReportingIsSupported) | TaskFlag_ReportingIsEnabled),
      suiteNumber(0), noIgnored(_noIgnored), pathToSuite(_pathToSuite) {

    tpm = Task::Progress_Manual;
    testOutDir = getTestOutDir();
}

bool GUITestLauncher::renameTestLog(const QString& testName) {
    QString outFileName = testOutFile(testName);
    QString outFilePath = testOutDir + QString("/logs/");

    QFile outLog(outFilePath + outFileName);
    return outLog.rename(outFilePath + "failed_" + outFileName);
}

void GUITestLauncher::run() {

    if (!initGUITestBase()) {
        return;
    }

    int finishedCount = 0;
    foreach(GUITest* t, tests) {
        if (isCanceled()) {
            return;
        }

        Q_ASSERT(t);
        if (t) {
            QString testName = t->getName();
            results[testName] = "";

            firstTestRunCheck(testName);

            if (!t->isIgnored()) {
                qint64 startTime = GTimer::currentTimeMicros();
                GUITestTeamcityLogger::testStarted(testName);

                QString testResult = performTest(testName);
                results[testName] = testResult;
                if (GUITestTeamcityLogger::testFailed(testResult)) {
                    renameTestLog(testName);
                }

                qint64 finishTime = GTimer::currentTimeMicros();
                GUITestTeamcityLogger::teamCityLogResult(testName, testResult, GTimer::millisBetween(startTime, finishTime));
            }
            else if(t->getReason() == GUITest::Bug){
                GUITestTeamcityLogger::testIgnored(testName, t->getIgnoreMessage());
            }
        }

        updateProgress(finishedCount++);
    }
}

void GUITestLauncher::firstTestRunCheck(const QString& testName) {

    QString testResult = results[testName];
    Q_ASSERT(testResult.isEmpty());
}

bool GUITestLauncher::initGUITestBase() {
    GUITestBase* b = AppContext::getGUITestBase();
    SAFE_POINT(NULL != b, "Test base is NULL", false);
    QList<GUITest *> list = b->getTests();
    if (list.isEmpty()) {
        setError(tr("No tests to run"));
        return false;
    }

    QList<QList<GUITest *> > suiteList;
    if(suiteNumber){
        for(int i=0; i<(list.length()/NUMBER_OF_TESTS_IN_SUTIE + 1);i++){
            suiteList << list.mid(i*NUMBER_OF_TESTS_IN_SUTIE,NUMBER_OF_TESTS_IN_SUTIE);
        }
        if(suiteNumber<0 || suiteNumber>suiteList.size()){
            setError(tr("Invalid suite number: %1. There are %2 suites").arg(suiteNumber).arg(suiteList.size()));
            return false;
        }
        tests = suiteList.takeAt(suiteNumber - 1);
    }else if(!pathToSuite.isEmpty()){
        QString absPath = QDir().absoluteFilePath(pathToSuite);
        QFile suite(absPath);
        SAFE_POINT(suite.exists(), "file " + absPath + " does not exists", false);
        if(suite.open(QFile::ReadOnly)){
            char buf[1024];
            while(suite.readLine(buf, sizeof(buf)) != -1){
                QString testName(buf);
                testName.remove('\n');
                testName.remove('\t');
                testName.remove(' ');
                bool added = false;
                foreach (GUITest* t, list) {
                    if(t->getName() == testName){
                        tests<<t;
                        added = true;
                        break;
                    }
                }
                SAFE_POINT(added, "test " + testName + " not found", false)
            }
        }else{
            SAFE_POINT(0, "can not open file " + absPath, false);
        }
    }
    else{
        tests = list;
    }

    if(noIgnored){
        foreach(GUITest* test, tests){
            test->setIgnored(false);
        }
    }
    return true;
}

void GUITestLauncher::updateProgress(int finishedCount) {

    int testsSize = tests.size();
    if (testsSize) {
        stateInfo.progress = finishedCount*100/testsSize;
    }
}

QString GUITestLauncher::testOutFile(const QString &testName) {
    return "ugene_"+testName+".out";
}

QString GUITestLauncher::getTestOutDir(){
    QString date = QDate::currentDate().toString("dd.MM.yyyy");
    QString initPath = QDir::homePath() + "/gui_testing_output/" + date;
    QDir d(initPath);
    int i = 1;
    while(d.exists()){
        d = QDir(initPath + QString("_%1").arg(i));
        i++;
    }
    return d.absolutePath();
}

QProcessEnvironment GUITestLauncher::getProcessEnvironment(const QString &testName) {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    QDir().mkpath(testOutDir + "/logs");
    env.insert("UGENE_DEV", "1");
    env.insert("UGENE_GUI_TEST", "1");
    env.insert("UGENE_USE_NATIVE_DIALOGS", "0");
    env.insert("UGENE_PRINT_TO_FILE", testOutDir + "/logs/" + testOutFile(testName));
    env.insert("UGENE_USER_INI", testOutDir + "/inis/" + testName + "_UGENE.ini");

    return env;
}

QString GUITestLauncher::performTest(const QString& testName) {

    QString path = QCoreApplication::applicationFilePath();

    // ~QProcess is killing the process, will not return until the process is terminated.
    QProcess process;
    process.setProcessEnvironment(getProcessEnvironment(testName));
    process.start(path, getTestProcessArguments(testName));

    bool started = process.waitForStarted();
    if (!started) {
        return tr("An error occurred while starting UGENE: ") + process.errorString();
    }
    bool finished;
    finished = process.waitForFinished(TIMEOUT);
    QProcess::ExitStatus exitStatus = process.exitStatus();

#ifdef Q_OS_WIN
    QProcess::execute("closeErrorReport.exe"); //this exe file, compiled Autoit script
#endif

    if (finished && (exitStatus == QProcess::NormalExit)) {
        return readTestResult(process.readAllStandardOutput());
    }
#ifdef Q_OS_WIN
    process.kill();
#endif
    if (finished) {
        return tr("An error occurred while finishing UGENE: ") + process.errorString() + '\n' + readTestResult(process.readAllStandardOutput());
    } else {
        return tr("Test fails because of timeout.");
    }
}

QStringList GUITestLauncher::getTestProcessArguments(const QString &testName) {

    return QStringList() << QString("--") + CMDLineCoreOptions::LAUNCH_GUI_TEST + "=" + testName;
}

QString GUITestLauncher::readTestResult(const QByteArray& output) {

    QString msg;
    QTextStream stream(output, QIODevice::ReadOnly);

    while(!stream.atEnd()) {
        QString str = stream.readLine();

        if (str.contains(GUITESTING_REPORT_PREFIX)) {
            msg =str.remove(0, str.indexOf(':')+1);
            if (!msg.isEmpty()) {
                break;
            }
        }
    }

    return msg;
}

QString GUITestLauncher::generateReport() const {

    QString res;
    res += "<table width=\"100%\">";
    res += QString("<tr><th>%1</th><th>%2</th></tr>").arg(tr("Test name")).arg(tr("Status"));

    QMap<QString, QString>::const_iterator i;
    for (i = results.begin(); i != results.end(); ++i) {
        QString color = "green";
        if (GUITestTeamcityLogger::testFailed(i.value())) {
            color = "red";
        }
        res += QString("<tr><th><font color='%3'>%1</font></th><th><font color='%3'>%2</font></th></tr>").arg(i.key()).arg(i.value()).arg(color);
    }
    res+="</table>";

    return res;
}

}
