/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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
#include "GTUtilsProject.h"
#include "GTUtilsDialog.h"
#include "GTUtilsProjectTreeView.h"
#include "runnables/qt/MessageBoxFiller.h"

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

namespace GUITest_initial_checks {

GUI_TEST_CLASS_DEFINITION(test_0000) {
    GTUtilsDialog::cleanup(os);
}

GUI_TEST_CLASS_DEFINITION(test_0001) {
// fail on MacOS
#ifndef Q_OS_MAC
//    QString activeWindowName = AppContext::getActiveWindowName();
//    CHECK_SET_ERR(activeWindowName.isEmpty(), "Active window name is not empty, it is " + activeWindowName);

    QMainWindow *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    CHECK_SET_ERR(mainWindow->isActiveWindow(), "MainWindow is not active");
#endif
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    CHECK_SET_ERR(AppContext::getProjectView() == NULL && AppContext::getProject() == NULL, "There is a project");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    Q_UNUSED(os);
    QMainWindow *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    mainWindow->showMaximized();

#ifdef Q_OS_MAC
    mainWindow->setWindowFlags(mainWindow->windowFlags() | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    mainWindow->show();
    mainWindow->setFocus();
#endif

    GTGlobals::sleep(1000);
}

GUI_TEST_CLASS_DEFINITION(test_0004) {
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj1.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj2-1.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj2.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj3.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj4.uprj");
    GTFile::backup(os, testDir + "_common_data/scenarios/project/proj5.uprj");
}

GUI_TEST_CLASS_DEFINITION(post_test_0000) {
    GTUtilsDialog::cleanup(os);
}

GUI_TEST_CLASS_DEFINITION(post_test_0001) {

    GTGlobals::sleep(1000);
    // close project
    if (AppContext::getProject() != NULL) {

        GTGlobals::sleep();
        GTWidget::click(os, GTUtilsProjectTreeView::getTreeWidget(os));
        GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
        GTGlobals::sleep(100);

        GTUtilsDialog::waitForDialog(os, new MessageBoxNoToAllOrNo(os));
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
        GTGlobals::sleep(100);

        GTKeyboardDriver::keyClick(os, 'q', GTKeyboardDriver::key["ctrl"]);
        GTGlobals::sleep(100);
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
}

} // GUITest_initial_checks namespace

} // U2 namespace
