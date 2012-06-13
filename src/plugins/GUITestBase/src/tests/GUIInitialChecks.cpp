/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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
#include "api/GTGlobals.h"
#include "GTUtilsProject.h"
#include "GTUtilsDialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

namespace GUITest_initial_checks {

GUI_TEST_CLASS_DEFINITION(test_0000) {
    Q_UNUSED(os);
    GTUtilsDialog::cleanup();
}

GUI_TEST_CLASS_DEFINITION(test_0001) {
    QString activeWindowName = AppContext::getActiveWindowName();
    CHECK_SET_ERR(activeWindowName.isEmpty(), "Active window name is not empty");

    QMainWindow *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    CHECK_SET_ERR(mainWindow->isActiveWindow(), "MainWindow is not active");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    CHECK_SET_ERR(AppContext::getProjectView() == NULL && AppContext::getProject() == NULL, "There is a project");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    Q_UNUSED(os);
    AppContext::getMainWindow()->getQMainWindow()->showMaximized();
    GTGlobals::sleep(1000);
}

} // GUITest_initial_checks namespace

} // U2 namespace
