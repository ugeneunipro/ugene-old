#include "GUITestLauncher.h"
#include "GUITestBase.h"

#include <U2Core/AppContext.h>
#include <U2Core/CMDLineCoreOptions.h>
#include <U2Core/Timer.h>
#include <U2Gui/MainWindow.h>
#include <QtCore/QMap>

#define TIMEOUT 60000
#define GUITESTING_REPORT_PREFIX "GUITesting"

#define ULOG_CAT_TEAMCITY "Teamcity Integration"

namespace U2 {

static Logger teamcityLog(ULOG_CAT_TEAMCITY);

GUITestLauncher::GUITestLauncher()
: Task("gui_test_launcher", TaskFlags(TaskFlag_ReportingIsSupported) | TaskFlag_ReportingIsEnabled) {

    tpm = Task::Progress_Manual;
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
            teamcityLog.trace(QString("##teamcity[testStarted name='%1 : %2']").arg(testName, testName));

            firstTestRunCheck(testName);

            qint64 startTime = GTimer::currentTimeMicros();
            QString testResult = performTest(testName);
            results[testName] = testResult;
            qint64 finishTime = GTimer::currentTimeMicros();

            teamCityLogResult(testName, testResult, GTimer::millisBetween(startTime, finishTime));
        }

        updateProgress(finishedCount++);
    }
}

void GUITestLauncher::teamCityLogResult(const QString &testName, const QString &testResult, qint64 testTimeMicros) const {

    if (testFailed(testResult)) {
        teamcityLog.trace(QString("##teamcity[testFailed name='%1 : %2' message='%3' details='%3' duration='%4']").arg(testName, testName, testResult, QString::number(testTimeMicros)));
    }

    teamcityLog.trace(QString("##teamcity[testFinished name='%1 : %2' duration='%3']").arg(testName, testName, QString::number(testTimeMicros)));
}

bool GUITestLauncher::testFailed(const QString &testResult) const {

    if (!testResult.contains("Success")) {
        return true;
    }

    return false;
}

void GUITestLauncher::firstTestRunCheck(const QString& testName) {

    QString testResult = results[testName];
    Q_ASSERT(testResult.isEmpty());
}

bool GUITestLauncher::initGUITestBase() {

    tests = AppContext::getGUITestBase()->getTests();
    if (tests.isEmpty()) {
        setError(tr("No tests to run"));
        return false;
    }

    return true;
}

void GUITestLauncher::updateProgress(int finishedCount) {

    int testsSize = tests.size();
    if (testsSize) {
        stateInfo.progress = finishedCount*100/testsSize;
    }
}

QString GUITestLauncher::performTest(const QString& testName) const {

    QString path = QCoreApplication::applicationFilePath();

    // ~QProcess is killing the process, will not return until the process is terminated.
    QProcess process;
    process.start(path, getTestProcessArguments(testName));

    bool started = process.waitForStarted();
    if (!started) {
        return tr("An error occurred while starting UGENE: ") + process.errorString();
    }

    bool finished = process.waitForFinished(TIMEOUT);
    QProcess::ExitStatus exitStatus = process.exitStatus();

    if (finished && (exitStatus == QProcess::NormalExit)) {
        return readTestResult(process.readAllStandardOutput());
    }

    return tr("An error occurred while finishing UGENE: ") + process.errorString();
}

QStringList GUITestLauncher::getTestProcessArguments(const QString &testName) const {

    return QStringList() << QString("--") + CMDLineCoreOptions::LAUNCH_GUI_TEST + "=" + testName;
}

QString GUITestLauncher::readTestResult(const QByteArray& output) const {

    QString msg;
    QTextStream stream(output, QIODevice::ReadOnly);

    while(!stream.atEnd()) {
        QString str = stream.readLine();

        if (str.contains(GUITESTING_REPORT_PREFIX)) {
            msg = str.split(":").last();
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
        if (testFailed(i.value())) {
            color = "red";
        }
        res += QString("<tr><th><font color='%3'>%1</font></th><th><font color='%3'>%2</font></th></tr>").arg(i.key()).arg(i.value()).arg(color);
    }
    res+="</table>";

    return res;
}

}
