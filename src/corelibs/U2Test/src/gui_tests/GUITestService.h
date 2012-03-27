#ifndef _U2_GUI_TESTS_VIEWER_H_
#define _U2_GUI_TESTS_VIEWER_H_

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/MultiTask.h>
#include <U2Gui/MainWindow.h>

#include <QtGui>

#include "GUITest.h"
#include "GUITestLauncher.h"


namespace U2 {

class GUITestLauncher;
class GUITestService;
class CMDLineRegistry;

class U2TEST_EXPORT GUITestService: public Service {
    Q_OBJECT
public:
    enum LaunchOptions {NONE, RUN_ONE_TEST, RUN_ALL_TESTS};
    static const QString successResult;

    GUITestService(QObject *parent = NULL);
    virtual ~GUITestService();

    void runTest(GUITests testsToRun);

protected:
    virtual void serviceStateChangedCallback(ServiceState oldState, bool enabledStateChanged);

    GUITest* getTest() const;
    GUITests addChecks() const;

protected slots:
    void sl_registerService();
    void sl_registerTestLauncherTask();
    void sl_taskStateChanged(Task*);
    void runGUITest();

private:
    QAction *runTestsAction;
    Task *testLauncher;
    GUITests testsToRun;

    LaunchOptions getLaunchOptions(CMDLineRegistry* cmdLine) const;
    bool launchedToTestGUI(CMDLineRegistry* cmdLine) const;

    void registerAllTestsTask();
    void registerServiceTask();

    void addServiceMenuItem();
    void deleteServiceMenuItem();

    Task* createTestLauncherTask() const;
    void writeTestResult(const QString &result) const;

    void setQtFileDialogView();

    TaskStateInfo os;
};

} // U2

#endif
