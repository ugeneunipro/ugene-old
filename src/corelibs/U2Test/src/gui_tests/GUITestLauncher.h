#ifndef _U2_GUI_TEST_LAUNCHER_H_
#define _U2_GUI_TEST_LAUNCHER_H_

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/MultiTask.h>
#include <U2Gui/MainWindow.h>
#include <QtCore/QProcessEnvironment>

namespace U2 {

class GUITest;

class GUITestLauncher: public Task {
    Q_OBJECT
public:
    GUITestLauncher();

    virtual void run();
    virtual QString generateReport() const;

private:
    QList<GUITest *> tests;
    QMap<QString, QString> results;

    static QStringList getTestProcessArguments(const QString &testName);
    static QProcessEnvironment getProcessEnvironment(const QString &testName);
    static QString testOutFile(const QString &testName);

    void firstTestRunCheck(const QString& testName);
    static QString performTest(const QString& testName);
    static QString readTestResult(const QByteArray& output);
    static bool renameTestLog(const QString& testName);

    bool initGUITestBase();
    void updateProgress(int finishedCount);

    void teamCityLogResult(const QString &testName, const QString &testResult, qint64 testTimeMicros) const;
    bool testFailed(const QString &testResult) const;
};

} // namespace

#endif
