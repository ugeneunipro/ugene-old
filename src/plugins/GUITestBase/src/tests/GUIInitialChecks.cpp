/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "utils/GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GUIInitialChecks.h"
#include "system/GTClipboard.h"
#include "system/GTFile.h"
#include <base_dialogs/GTFileDialog.h>
#include "GTGlobals.h"
#include <drivers/GTKeyboardDriver.h>
#include "primitives/GTMenu.h"
#include <drivers/GTMouseDriver.h>
#include <primitives/GTWidget.h>
#include <primitives/GTWidget.h>
#include "runnables/qt/EscapeClicker.h"
#include <base_dialogs/MessageBoxFiller.h>
#include "runnables/ugene/ugeneui/SaveProjectDialogFiller.h"

namespace U2 {

namespace GUITest_initial_checks {
using namespace HI;

GUI_TEST_CLASS_DEFINITION(pre_action_0000) {
    GTUtilsTaskTreeView::waitTaskFinished(os);
#ifdef Q_OS_WIN
    QProcess::execute("closeAllErrors.exe"); //this exe file, compiled Autoit script
#endif
    GTUtilsDialog::cleanup(os, GTUtilsDialog::NoFailOnUnfinished);
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

GUI_TEST_CLASS_DEFINITION(pre_action_0001) {
    CHECK_SET_ERR(AppContext::getProjectView() == NULL && AppContext::getProject() == NULL, "There is a project");
}

GUI_TEST_CLASS_DEFINITION(pre_action_0002) {
    Q_UNUSED(os);
    QMainWindow *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    CHECK_SET_ERR(mainWindow != NULL, "main window is NULL");

    if (!mainWindow->isMaximized()) {
        GTWidget::showMaximized(os, mainWindow);
        GTGlobals::sleep(1000);
    }

}

GUI_TEST_CLASS_DEFINITION(pre_action_0003) {
    if(QDir(testDir).exists()){
        GTFile::backup(os, testDir + "_common_data/scenarios/project/proj1.uprj");
        GTFile::backup(os, testDir + "_common_data/scenarios/project/proj2-1.uprj");
        GTFile::backup(os, testDir + "_common_data/scenarios/project/proj2.uprj");
        GTFile::backup(os, testDir + "_common_data/scenarios/project/proj3.uprj");
        GTFile::backup(os, testDir + "_common_data/scenarios/project/proj4.uprj");
        GTFile::backup(os, testDir + "_common_data/scenarios/project/proj5.uprj");
    }
}

GUI_TEST_CLASS_DEFINITION(pre_action_0004) {
    Q_UNUSED(os);
    QDir dir(QDir().absoluteFilePath(screenshotDir));
    if (!dir.exists(dir.absoluteFilePath(screenshotDir))) {
        dir.mkpath(dir.path());
    }
}

GUI_TEST_CLASS_DEFINITION(pre_action_0005) {
    if(QDir(sandBoxDir).exists()){
        PermissionsSetter::setReadWrite(os, sandBoxDir);
        GTGlobals::sleep();
        QDir sandBox = QDir(sandBoxDir);
        foreach (QString path, sandBox.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden)) {
            GTFile::removeDir(sandBox.absolutePath() + "/" + path);
        }
    }
}

GUI_TEST_CLASS_DEFINITION(pre_action_0006) {
    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    CHECK_SET_ERR(mw != NULL, "main window is NULL");
#ifdef Q_OS_MAC
    GTWidget::click(os, mw, Qt::LeftButton, QPoint(200, 200));
#endif
}

GUI_TEST_CLASS_DEFINITION(post_check_0000) {
    GTUtilsDialog::cleanup(os);
}

GUI_TEST_CLASS_DEFINITION(post_check_0001) {
    QWidget *modalWidget = QApplication::activeModalWidget();
    if (NULL != modalWidget) {
        CHECK_SET_ERR(NULL == modalWidget, QString("There is a modal widget after test finish: %1").arg(modalWidget->windowTitle()));
    }

    QWidget *popupWidget = QApplication::activePopupWidget();
    CHECK_SET_ERR(NULL == popupWidget, "There is a popup widget after test finish");
}

GUI_TEST_CLASS_DEFINITION(post_action_0000) {
    const Qt::KeyboardModifiers modifiers = QGuiApplication::queryKeyboardModifiers();
    if (modifiers & Qt::ShiftModifier) {
        GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
    }

    if (modifiers & Qt::ControlModifier) {
        GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["ctrl"]);
    }

    if (modifiers & Qt::AltModifier) {
        GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["alt"]);
    }
}

GUI_TEST_CLASS_DEFINITION(post_action_0001) {
    QWidget* popupWidget = QApplication::activePopupWidget();
    while (popupWidget != NULL) {
        GTWidget::close(os, popupWidget);
        popupWidget = QApplication::activePopupWidget();
    }

    QWidget* modalWidget = QApplication::activeModalWidget();
    while (modalWidget != NULL) {
        GTWidget::close(os, modalWidget);
        modalWidget = QApplication::activeModalWidget();
    }

    GTClipboard::clear(os);
}

GUI_TEST_CLASS_DEFINITION(post_action_0002) {
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

    GTUtilsMdi::closeAllWindows(os);

    //cancel all tasks
    AppContext::getTaskScheduler()->cancelAllTasks();
    GTUtilsTaskTreeView::waitTaskFinished(os, 60000);
}

GUI_TEST_CLASS_DEFINITION(post_action_0003) {
    if(QDir(testDir).exists()){
        GTFile::restore(os, testDir + "_common_data/scenarios/project/proj1.uprj");
        GTFile::restore(os, testDir + "_common_data/scenarios/project/proj2-1.uprj");
        GTFile::restore(os, testDir + "_common_data/scenarios/project/proj2.uprj");
        GTFile::restore(os, testDir + "_common_data/scenarios/project/proj3.uprj");
        GTFile::restore(os, testDir + "_common_data/scenarios/project/proj4.uprj");
        GTFile::restore(os, testDir + "_common_data/scenarios/project/proj5.uprj");
    }
    if(QDir(sandBoxDir).exists()){
        PermissionsSetter::setReadWrite(os, sandBoxDir);
        GTGlobals::sleep();
        QDir sandBox = QDir(sandBoxDir);
        foreach (QString path, sandBox.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden)) {
            GTFile::removeDir(sandBox.absolutePath() + "/" + path);
        }
    }
}

} // GUITest_initial_checks namespace

} // U2 namespace
