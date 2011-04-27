#ifndef _U2_GUI_TESTS_VIEWER_H_
#define _U2_GUI_TESTS_VIEWER_H_

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/MultiTask.h>
#include <U2Gui/MainWindow.h>

#include <QtGui>

#include "GUITests.h"
#include "GUITestLauncher.h"
#include "ProjectViewTests.h"
#include "AboutDialogTests.h"


namespace U2 {

class TestLauncher;
class GUITestService;


class U2TEST_EXPORT GUITestService: public Service {
    Q_OBJECT
public:
    GUITestService(QObject *parent = NULL);
    void runTest(QList<GUITest*> testsToRun);

protected:
    virtual void serviceStateChangedCallback(ServiceState oldState, bool enabledStateChanged);
   
public slots:
    void sl_registerSevice();
    void sl_registerTask();

private:
    QAction *runTestsAction;
    TestLauncher *testLauncher;
    QList<GUITest*> testsToRun;
};

class TestProjectView: public ProjectViewTests {
public:
    TestProjectView(const QString &_path1, const QString &_path2, const QString &_seqName, const QString &name): 
      ProjectViewTests(name), path1(_path1), path2(_path2), seqName(_seqName) {}
      void execute();
      void checkResult();

private:
    QString path1;
    QString path2;
    QString seqName;
};

class LockDocumentTest: public ProjectViewTests {
public:
    LockDocumentTest(const QString &_document, const QString &name): ProjectViewTests(name), document(_document) {}
    void execute();
    void checkResult();

private:
    QString document;
};

class UnlockDocumentTest: public ProjectViewTests {
public:
    UnlockDocumentTest(const QString &_document, const QString &name): ProjectViewTests(name), document(_document) {}
    void execute();
    void checkResult();

private:
    QString document;
};

class ComplexTest: public ProjectViewTests {
public:
    ComplexTest(const QString& _path, const QString &name): ProjectViewTests(name), path(_path) {}
    void execute();
    void checkResult() {}

private:
    QString path;
};

class OpenDocumentTest: public ProjectViewTests {
public:
    OpenDocumentTest(const QString &_path, const QString &name): ProjectViewTests(name), path(_path){}
    void execute() {
        openFile(path);
    }
    void checkResult() {}
private:
    QString path;
};

class TestTaskView: public TaskViewTest{
public:
    TestTaskView(const QString &_path, const QString &_name): TaskViewTest(_name), path(_path) {}
    void execute();
    void checkResult();

private:
    QString path;
};


class LongTest:public GUITest {
public:
    LongTest(): GUITest("Long test") {}
    void execute() {while(true);}
    void checkResult() {}
};


}

#endif
