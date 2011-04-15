#include "GUITestService.h"
#include "GUITestBase.h"

#include <U2Core/AppContext.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/TaskStarter.h>
#include <U2Gui/ProjectTreeController.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObject.h>


namespace U2 {

GUITestService::GUITestService(QObject *parent): Service(Service_GUITesting, tr("GUI test viewer"), tr("Service to support UGENE GUI testing")) {
    testLauncher = NULL;
    connect(AppContext::getPluginSupport(), SIGNAL(si_allStartUpPluginsLoaded()), SLOT(sl_registerSevice()));
    GUITestBase *tb = AppContext::getGUITestBase();
    TestProjectView *test1 = new TestProjectView("E:/Files/_1.003.fa", "E:/Files/_1.002.fa", "3INS chain 3 sequence", "Add object to view test");
    TestTaskView *test2 = new TestTaskView("E:/Files/ecoli.gbk", "Cancel task test");
    ComplexTest *test3 = new ComplexTest("E:/Files/_1.002.fa", "Lock-unlock test");
    Test1AboutDialog* test1AboutDialog=new Test1AboutDialog("AboutDialog_test1");
    Test2AboutDialog* test2AboutDialog=new Test2AboutDialog("AboutDialog_test2");
    Test3AboutDialog* test3AboutDialog=new Test3AboutDialog("AboutDialog_test3");
    Test4AboutDialog* test4AboutDialog=new Test4AboutDialog("AboutDialog_test4");
    Test5AboutDialog* test5AboutDialog=new Test5AboutDialog("AboutDialog_test5");
//    tb->registerTest(test1);
//    tb->registerTest(test2);
//    tb->registerTest(test3);
    tb->registerTest(test1AboutDialog);
    tb->registerTest(test2AboutDialog);
    tb->registerTest(test3AboutDialog);
    tb->registerTest(test4AboutDialog);
    tb->registerTest(test5AboutDialog);
//    tb->registerTest(new LongTest());
}

void GUITestService::sl_registerSevice() {
    CMDLineRegistry* cmdLine = AppContext::getCMDLineRegistry();
    if(cmdLine && cmdLine->hasParameter("gui-test")) {
        testLauncher = new TestLauncher();
        GUITestBase *tb = AppContext::getGUITestBase();
        GUITest *t = tb->findTestByName(cmdLine->getParameterValue("gui-test"));
        if(t) {
            //testLauncher->launchWithinProcess(t);
            LaunchTestTask *task = new LaunchTestTask(t);
            AppContext::getTaskScheduler()->registerTopLevelTask(task);
        }
    } else {
        AppContext::getTaskScheduler()->registerTopLevelTask(AppContext::getServiceRegistry()->registerServiceTask(this));
    }
}

void GUITestService::sl_showWindow() {
    AppContext::getTaskScheduler()->registerTopLevelTask(testLauncher);
    //view->show();
}

void GUITestService::serviceStateChangedCallback(ServiceState oldState, bool enabledStateChanged) {
    if (!enabledStateChanged) {
        return;
    }

    if (isEnabled()) {
        testLauncher = new TestLauncher();
        runTestsAction = new QAction(tr("GUI testing"), this);
        runTestsAction->setObjectName("action_guitest");
        connect(runTestsAction, SIGNAL(triggered()), new TaskStarter(testLauncher), SLOT(registerTask()));
        AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS)->addAction(runTestsAction);
    } else {
        assert(runTestsAction!=NULL);
        delete runTestsAction;
        runTestsAction = NULL;

        //delete testLauncher;
        testLauncher = NULL;
    }
}

//Test examples
void TestProjectView::execute() {
    openFile(path1);
    openFile(path2);

    sleep(2000);
    if(!isWidgetExists(projectViewName)) {
        keyClick("left_dock_bar", Qt::Key_1, Qt::AltModifier);
    }
    sleep(1000);
    addObjectToView("[s] " + seqName);
}

void TestProjectView::checkResult() {
    QWidget *w1 = findWidgetByName("ADV_single_sequence_widget_0", "_1 3INS chain 2 sequence");
    QWidget *w2 = findWidgetByName("ADV_single_sequence_widget_1", "_1 3INS chain 2 sequence");

    /*ADVSingleSequenceWidget * sw1 = qobject_cast<ADVSingleSequenceWidget*>(w1);
    ADVSingleSequenceWidget * sw2 = qobject_cast<ADVSingleSequenceWidget*>(w2);
    QString str1 = sw1->getSequenceObject()->getGObjectName();
    QString str2 = sw2->getSequenceObject()->getGObjectName();
    if(!(str1 == "3INS chain 2 sequence" && str2 == "3INS chain 3 sequence")) {
        throw TestException(tr("Not expected result"));
    }*/
}

void TestTaskView::execute() {
    OpenDocumentTest t(path, "tttt");
    //t.moveToThread(QApplication::instance()->thread());
    t.launch();
    if(!isWidgetExists(taskViewWidgetName)) {
        keyClick("bottom_dock_bar", Qt::Key_2, Qt::AltModifier);
    }
    sleep(1000);
    cancelTask("Open project/document");
    sleep(500);
}

void TestTaskView::checkResult() {
    if(isItemExists("Open project/document", taskViewWidgetName)) {
        throw TestException(tr("Not expected result"));
    }
}

void LockDocumentTest::execute(){
    QPoint pos = getItemPosition(document, projectViewName);
    moveTo(projectViewName, pos);
    mouseClickOnItem(projectViewName, Qt::LeftButton, pos);
    contextMenuOnItem(projectViewName, pos);
    sleep(1000);
    clickMenu("Lock document for editing","", true);
    sleep(1000);
}

void LockDocumentTest::checkResult() {
    QTreeWidget * projectTree = static_cast<QTreeWidget*>(findWidgetByName(projectViewName));
    QList<QTreeWidgetItem*> items = projectTree->findItems(document, Qt::MatchRecursive | Qt::MatchExactly);
    if(!items.isEmpty()) {
        ProjViewDocumentItem *docItem = static_cast<ProjViewDocumentItem*>(items.first());
        if(!docItem->doc->isStateLocked()) {
            throw TestException(tr("Document %1 not locked").arg(document));
        }
    } else {
        throw TestException(tr("Not expected result"));
    }
}

void UnlockDocumentTest::execute(){
    QPoint pos = getItemPosition(document, projectViewName);
    moveTo(projectViewName, pos);
    mouseClickOnItem(projectViewName, Qt::LeftButton, pos);
    contextMenuOnItem(projectViewName, pos);
    sleep(1000);
    clickMenu("Unlock document for editing","", true);
    sleep(1000);
}

void UnlockDocumentTest::checkResult() {
    QTreeWidget * projectTree = static_cast<QTreeWidget*>(findWidgetByName(projectViewName));
    QList<QTreeWidgetItem*> items = projectTree->findItems(document, Qt::MatchRecursive | Qt::MatchExactly);
    if(!items.isEmpty()) {
        ProjViewDocumentItem *docItem = static_cast<ProjViewDocumentItem*>(items.first());
        if(docItem->doc->isStateLocked()) {
            throw TestException(tr("Document %1 not locked").arg(document));
        }
    } else {
        throw TestException(tr("Not expected result"));
    }
}

void ComplexTest::execute() {
    openFile(path);
    sleep(1000);
    if(!isWidgetExists(projectViewName)) {
        keyClick("left_dock_bar", Qt::Key_1, Qt::AltModifier);
    }
    LockDocumentTest lock(path.split("/").last(), "lock");
    UnlockDocumentTest unlock(path.split("/").last(), "unlock");
    lock.launch();
    unlock.launch();
}

}
