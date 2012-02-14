#ifndef _U2_GUI_TEST_LAUNCHER_H_
#define _U2_GUI_TEST_LAUNCHER_H_

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/MultiTask.h>
#include <U2Gui/MainWindow.h>

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

	QStringList getTestProcessArguments(const QString &testName) const;

	void firstTestRunCheck(const QString& testName);
	QString performTest(const QString& testName) const;

	QString readTestResult(const QByteArray& output) const;

	bool initGUITestBase();
	void updateProgress(int finishedCount);

	void teamCityLogResult(const QString &testName, const QString &testResult) const;
	bool testFailed(const QString &testResult) const;
};

} // namespace

#endif
