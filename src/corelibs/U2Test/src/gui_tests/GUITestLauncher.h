#ifndef _U2_GUI_TESTS_LAUNCHER_H_
#define _U2_GUI_TESTS_LAUNCHER_H_

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/MultiTask.h>
#include <U2Gui/MainWindow.h>

#include <QtGui>

#include "GUITests.h"
#include "ProjectViewTests.h"


namespace U2 {

class LaunchTestTask: public Task {
    Q_OBJECT
public:
    LaunchTestTask(GUITest* _t):Task(tr("Launch test"), TaskFlags_FOSCOE), t(_t) {}
    void prepare();
    void run();
    ReportResult report();

private:
    GUITest *t;
};

class TestLauncher: public Task {
    Q_OBJECT
public:
    TestLauncher(): Task("gui_test_launcher", TaskFlags(TaskFlag_ReportingIsSupported) | TaskFlag_ReportingIsEnabled), testToLaunch(NULL) {}
    void run();
    QString generateReport() const;

private:
    QList<GUITest *> tests;
    GUITest* testToLaunch;
    QMap<QString, QString> results;
};


}

#endif
