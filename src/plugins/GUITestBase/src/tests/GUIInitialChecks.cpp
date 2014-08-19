/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "GUIInitialChecks.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWidget.h"
#include "api/GTGlobals.h"
#include "api/GTFile.h"
#include "api/GTMenu.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsProject.h"
#include "GTUtilsDialog.h"
#include "GTUtilsProjectTreeView.h"
#include "runnables/ugene/ugeneui/SaveProjectDialogFiller.h"
#include "runnables/qt/MessageBoxFiller.h"

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Gui/ObjectViewModel.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include <QtCore/QDir>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#endif

namespace U2 {

namespace GUITest_initial_checks {

GUI_TEST_CLASS_DEFINITION(test_0000) {
    GTUtilsDialog::cleanup(os);
}

GUI_TEST_CLASS_DEFINITION(test_0001) {
    CHECK_SET_ERR(AppContext::getProjectView() == NULL && AppContext::getProject() == NULL, "There is a project");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    Q_UNUSED(os);
    QMainWindow *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    CHECK_SET_ERR(mainWindow != NULL, "main window is NULL");

#ifdef Q_OS_WIN
    mainWindow->setWindowFlags(mainWindow->windowFlags() | Qt::WindowStaysOnTopHint);
#endif

    mainWindow->showMaximized();

#ifdef Q_OS_MAC
    mainWindow->setWindowFlags(mainWindow->windowFlags() | Qt::CustomizeWindowHint);
    mainWindow->show();
    mainWindow->setFocus();
#endif

    GTGlobals::sleep(1000);
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj1.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj2-1.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj2.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj3.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj4.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj5.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/assembly/example-alignment.ugenedb");
#ifndef Q_OS_MAC
    //GTFile::backupDir(os, dataDir + "workflow_samples");
#endif
}

GUI_TEST_CLASS_DEFINITION(test_0004){
    Q_UNUSED(os);
    QDir dir(QDir().absoluteFilePath(screenshotDir));
    if(!dir.exists(dir.absoluteFilePath(screenshotDir))){
        dir.mkpath(dir.path());
    }
}
GUI_TEST_CLASS_DEFINITION(test_0005){
    GTUtilsTaskTreeView::waitTaskFinished(os);
}

GUI_TEST_CLASS_DEFINITION(test_0006) {
    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    CHECK_SET_ERR(mw != NULL, "main window is NULL");
#ifdef Q_OS_MAC
    GTWidget::click(os, mw, Qt::LeftButton, QPoint(1500,300));
#endif
    GTGlobals::sleep(200);
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    uiLog.trace(QString("modifiers: %1").arg(mods));
    if(mods.testFlag(Qt::ShiftModifier)){
        uiLog.trace("shift pressed");
        GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
    }
    if(mods.testFlag(Qt::ControlModifier)){
        uiLog.trace("control pressed");
        GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["ctrl"]);
    }
    if(mods.testFlag(Qt::MetaModifier)){
        uiLog.trace("meta pressed");
    }
    if(mods.testFlag(Qt::AltModifier)){
        uiLog.trace("alt pressed");
        GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["alt"]);
    }

    //GTKeyboardDriver::releaseAllKeys(os);
}

GUI_TEST_CLASS_DEFINITION(post_test_0000) {
    GTUtilsDialog::cleanup(os);
}

GUI_TEST_CLASS_DEFINITION(post_test_0001) {

    GTGlobals::sleep(1000);
    // close project
    if (AppContext::getProject() != NULL) {
        int key;
#ifdef Q_OS_MAC
        key = GTKeyboardDriver::key["cmd"];
#else
        key = GTKeyboardDriver::key["ctrl"];
#endif

        GTGlobals::sleep();
        GTWidget::click(os, GTUtilsProjectTreeView::getTreeView(os));
        GTKeyboardDriver::keyClick(os, 'a', key);
        GTGlobals::sleep(100);

        GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
        GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
        GTGlobals::sleep(500);
#ifdef Q_OS_MAC
        QMenu *menu = GTMenu::showMainMenu(os, MWMENU_FILE);
        GTMenu::clickMenuItem(os, menu, ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
#else
        GTKeyboardDriver::keyClick(os, 'q', key);
        GTGlobals::sleep(100);
#endif
        GTGlobals::sleep(500);

        GTUtilsDialog::cleanup(os, GTUtilsDialog::NoFailOnUnfinished);
        GTGlobals::sleep();
    }
}

GUI_TEST_CLASS_DEFINITION(post_test_0002) {
    GTFile::restore(os, testDir + "_common_data/scenarios/project/proj1.uprj");
    GTFile::restore(os, testDir + "_common_data/scenarios/project/proj2-1.uprj");
    GTFile::restore(os, testDir + "_common_data/scenarios/project/proj2.uprj");
    GTFile::restore(os, testDir + "_common_data/scenarios/project/proj3.uprj");
    GTFile::restore(os, testDir + "_common_data/scenarios/project/proj4.uprj");
    GTFile::restore(os, testDir + "_common_data/scenarios/project/proj5.uprj");
    GTFile::restore(os, testDir + "_common_data/scenarios/assembly/example-alignment.ugenedb");
#ifndef Q_OS_MAC
    //GTFile::restoreDir(os, dataDir + "workflow_samples");
#endif
    GTFile::removeDir(QDir().currentPath() + "/MSA_schemes");
}

GUI_TEST_CLASS_DEFINITION(post_test_0003) {     //if this post test detect any problems, use test_0004 and post_test_0002 for backup and restore corrupted files
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
