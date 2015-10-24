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
#include <QDir>
#include <QMainWindow>
#include <QTreeView>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/ObjectViewModel.h>

#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GUIInitialChecks.h"
#include "api/GTClipboard.h"
#include "api/GTFile.h"
#include "api/GTFileDialog.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"
#include "api/GTWidget.h"
#include "runnables/qt/EscapeClicker.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/ugeneui/SaveProjectDialogFiller.h"

namespace U2 {

namespace GUITest_initial_checks {

GUI_TEST_CLASS_DEFINITION(test_0000) {
    GTUtilsTaskTreeView::waitTaskFinished(os);
#ifdef Q_OS_WIN
    QProcess::execute("closeAllErrors.exe"); //this exe file, compiled Autoit script
#endif
    GTUtilsDialog::cleanup(os);
#ifndef Q_OS_WIN
    GTMouseDriver::release(os, Qt::RightButton);
    GTMouseDriver::release(os);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["ctrl"]);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["alt"]);
#endif
#ifdef Q_OS_MAC
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["cmd"]);
#endif

    GTUtilsDialog::startHangChecking(os);
}

GUI_TEST_CLASS_DEFINITION(test_0001) {
    CHECK_SET_ERR(AppContext::getProjectView() == NULL && AppContext::getProject() == NULL, "There is a project");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    Q_UNUSED(os);
    QMainWindow *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    CHECK_SET_ERR(mainWindow != NULL, "main window is NULL");

    if (!mainWindow->isMaximized()) {
        GTWidget::showMaximized(os, mainWindow);
        GTGlobals::sleep(1000);
    }

}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj1.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj2-1.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj2.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj3.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj4.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj5.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/assembly/example-alignment.ugenedb");
}

GUI_TEST_CLASS_DEFINITION(test_0004){
    Q_UNUSED(os);
    QDir dir(QDir().absoluteFilePath(screenshotDir));
    if(!dir.exists(dir.absoluteFilePath(screenshotDir))){
        dir.mkpath(dir.path());
    }
}
GUI_TEST_CLASS_DEFINITION(test_0005){
    PermissionsSetter::setReadWrite(os, sandBoxDir);
    GTGlobals::sleep();
    QDir sandBox = QDir(sandBoxDir);
    foreach (QString path, sandBox.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden)) {
        GTFile::removeDir(sandBox.absolutePath() + "/" + path);
    }
}

GUI_TEST_CLASS_DEFINITION(test_0006){
    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    CHECK_SET_ERR(mw != NULL, "main window is NULL");
#ifdef Q_OS_MAC
    GTWidget::click(os, mw, Qt::LeftButton, QPoint(200,200));
#endif
}

GUI_TEST_CLASS_DEFINITION(test_0007){
    ///temporary ignored
    //GTFile::removeDir(AppContext::getAppSettings()->getUserAppsSettings()->getDefaultDataDirPath());
}

GUI_TEST_CLASS_DEFINITION(post_test_0000){
    GTUtilsDialog::cleanup(os);
}

GUI_TEST_CLASS_DEFINITION(post_test_0001) {
    QWidget* widget = QApplication::activeModalWidget();
    while (widget != NULL) {
        GTWidget::close(os, widget);
        widget = QApplication::activeModalWidget();
    }

    GTClipboard::text(os);
//#ifdef Q_OS_WIN
    TaskScheduler* scheduller = AppContext::getTaskScheduler();
    QString s;
    foreach (Task* t, scheduller->getTopLevelTasks()) {
        s.append(t->getTaskName() + '\n');
    }
    if(!s.isEmpty()){
        QFile f(testDir + QDateTime::currentDateTime().toString() + ".txt");
        f.open(QFile::ReadWrite);
        f.write(s.toUtf8());
        f.close();
    }

//#endif

}

GUI_TEST_CLASS_DEFINITION(post_test_0002) {
    GTGlobals::sleep(1000);
    // close project
    if (AppContext::getProject() != NULL) {
        GTGlobals::sleep();
        GTWidget::click(os, GTUtilsProjectTreeView::getTreeView(os));
        GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
        GTGlobals::sleep(100);

        GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
        GTUtilsDialog::waitForDialog(os, new AppCloseMessageBoxDialogFiller(os));
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
        GTGlobals::sleep(500);
#ifdef Q_OS_MAC
        GTMenu::clickMainMenuItem(os, QStringList() << "File" << "Close project");
#else
        GTKeyboardDriver::keyClick(os, 'q', GTKeyboardDriver::key["ctrl"]);
        GTGlobals::sleep(100);
#endif
        GTGlobals::sleep(500);

        GTUtilsDialog::cleanup(os, GTUtilsDialog::NoFailOnUnfinished);
        GTGlobals::sleep();
    }

    // close all Workflow Designer windows
    GTGlobals::FindOptions options;
    options.failIfNull = false;
    options.matchPolicy = Qt::MatchContains;

    GTUtilsMdi::closeAllWindows(os);

    //cancel all tasks
    AppContext::getTaskScheduler()->cancelAllTasks();
    GTUtilsTaskTreeView::waitTaskFinished(os, 60000);
}

GUI_TEST_CLASS_DEFINITION(post_test_0003) {
    GTFile::restore(os, testDir + "_common_data/scenarios/project/proj1.uprj");
    GTFile::restore(os, testDir + "_common_data/scenarios/project/proj2-1.uprj");
    GTFile::restore(os, testDir + "_common_data/scenarios/project/proj2.uprj");
    GTFile::restore(os, testDir + "_common_data/scenarios/project/proj3.uprj");
    GTFile::restore(os, testDir + "_common_data/scenarios/project/proj4.uprj");
    GTFile::restore(os, testDir + "_common_data/scenarios/project/proj5.uprj");
    GTFile::restore(os, testDir + "_common_data/scenarios/assembly/example-alignment.ugenedb");

    PermissionsSetter::setReadWrite(os, sandBoxDir);
    GTGlobals::sleep();
    QDir sandBox = QDir(sandBoxDir);
    foreach (QString path, sandBox.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden)) {
        GTFile::removeDir(sandBox.absolutePath() + "/" + path);
    }
}

GUI_TEST_CLASS_DEFINITION(post_test_0004) {     //if this post test detect any problems, use test_0004 and post_test_0002 for backup and restore corrupted files
    Q_UNUSED(os);
#ifdef Q_OS_WIN
    QProcess *svnProcess = new QProcess();

    QStringList dirs;
    dirs.append(testDir + "_common_data/");
    dirs.append(dataDir + "samples/");
    bool SVNCorrupted = false;
    foreach(QString workingDir, dirs){
        QDir d;
        svnProcess->setWorkingDirectory(d.absoluteFilePath(workingDir));
        svnProcess->start("svn", QStringList()<<"st");
        if ( !svnProcess->waitForStarted( 2000 ) ) {
            coreLog.info( "SVN process hasn't start!" );
            continue;
        }
        while (!svnProcess->waitForFinished(30000));
        //CHECK_SET_ERR(svnProcess->exitCode() != EXIT_FAILURE, "SVN process finished wrong");
        QStringList output = QString(svnProcess->readAllStandardOutput()).split('\n');
        bool needUpdate = false;
        foreach(QString str, output){
            QStringList byWords = str.split(QRegExp("\\s+"));
            if (byWords[0][0] == '?' || byWords[0][0] == 'M'){
                if(byWords[0][0] == 'M'){
                    SVNCorrupted = true;
                    needUpdate = true;
                }
                QFile::remove(workingDir + QDir::separator() + byWords[1]);
            }
        }
        if (needUpdate){
            svnProcess->start("svn", QStringList()<<"up");
            while(!svnProcess->waitForFinished(30000));
        }
    }
    CHECK_SET_ERR(!SVNCorrupted, "SVN corrupted by this test");
#endif
}

} // GUITest_initial_checks namespace

} // U2 namespace
