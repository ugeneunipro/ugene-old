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

#include "GTTestsProjectAnonymousProject.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "GTUtilsProject.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsProject.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/ugeneui/ExportProjectDialogFiller.h"
#include "runnables/ugene/ugeneui/CreateNewProjectWidgetFiller.h"

#include <U2View/AnnotatedDNAViewFactory.h>

namespace U2{

namespace GUITest_common_scenarios_project_anonymous_project{

GUI_TEST_CLASS_DEFINITION(test_0002) {

// 1. Use menu {File->Open}. Open file data/samples/PDB/1CF7.pdb
    GTFileDialog::openFile(os, dataDir+"samples/PDB/", "1CF7.PDB");

// Expected state: 
//     1) Project view with document "1CF7.PDB" is opened
    GTGlobals::sleep(5000);
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");

// 2. Use menu {File->Export Project}
// Expected state: "Export Project" dialog has appeared
// 3. Fill the next field in dialog:
//     {Destination Directory} _common_data/scenarios/sandbox
//     {Project file name} proj2.uprj
// 4. Click OK button
// 5. Click NO in opened messagebox
    GTUtilsDialog::waitForDialog(os, new ExportProjectDialogFiller(os, testDir + "_common_data/scenarios/sandbox", "proj2.uprj"));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__EXPORT_PROJECT);
    GTGlobals::sleep();

// 6. Use menu {File->Close project}
// 7. Click NO in opened messagebox
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTGlobals::sleep();

// 8. Use menu {File->Open}. Open project _common_data/scenarios/sandbox/proj2.uprj
    GTGlobals::sleep();
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/sandbox/", "proj2.uprj");

// Expected state: 
//     1) project view with document "1CF7.PDB" has been opened, 
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");

//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

//     3) File path at tooltip for "1CF7.PDB" must be "_common_data/scenarios/sandbox/1CF7.PDB"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1CF7.PDB"));
    GTGlobals::sleep();
    GTUtilsToolTip::checkExistingToolTip(os, "_common_data/scenarios/sandbox/1CF7.PDB");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {

// 1. Use menu {File->Open}. Open file samples/PDB/1CF7.pdb
    GTFileDialog::openFile(os, dataDir+"samples/PDB/", "1CF7.PDB");

// Expected state: 
//     1) Project view with document "1CF7.PDB" is opened
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");

// 2. Use menu {File->Save Project As}
// Expected state: "Save project as" dialog has appeared
// 
// 3. Fill the next field in dialog:
//     {Project name:} proj2
//     {Project Folder:} _common_data/scenarios/sandbox
//     {Project file} proj2
// 4. Click Save button
    GTUtilsDialog::waitForDialog(os, new SaveProjectAsDialogFiller(os, "proj2", testDir+"_common_data/scenarios/sandbox", "proj2"));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__SAVE_AS_PROJECT);
    GTGlobals::sleep();

// 5. Use menu {File->Close project}
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTGlobals::sleep();

// Expected state: project and sequence view closed
    GTUtilsProject::checkProject(os, GTUtilsProject::NotExists);

// 6. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/sandbox/", "proj2.uprj");

// Expected state: 
//     1) project view with document "1CF7.PDB" has been opened, 
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");
//     2) UGENE window titled with text "proj2"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");
//     3) File path at tooltip for "1CF7.PDB" must be "samples/PDB/1CF7.PDB"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1CF7.PDB"));
    GTGlobals::sleep(2000);
    GTUtilsToolTip::checkExistingToolTip(os, "samples/PDB/1CF7.PDB");
}

}

}
