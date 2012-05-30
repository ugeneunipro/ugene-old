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

#include "GTTestsProjectMultipleDocs.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "GTUtilsProject.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsTaskTreeView.h"
#include "runnables/ugene/ugeneui/CreateNewProjectWidgetFiller.h"

#include <U2View/AnnotatedDNAViewFactory.h>

namespace U2 {

namespace GUITest_common_scenarios_project_multiple_docs {

GUI_TEST_CLASS_DEFINITION(test_0001) {

// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/project/", "proj2.uprj");

// Expected state: 
// 	1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
// 	2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Use menu {File->Save Project As}
// Expected state: "Save project as" dialog has appeared
// 
// 3. Fill the next field in dialog:
// 	{Project name:} proj2
// 	{Project Folder:} _common_data/scenarios/sandbox
// 	{Project file} proj2
// 4. Click Save button
    Runnable *filler = new SaveProjectAsDialogFiller(os, "proj2", testDir+"_common_data/scenarios/sandbox", "proj2");
    GTUtilsDialog::waitForDialog(os, filler);
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__SAVE_AS_PROJECT);
    GTGlobals::sleep();

// 5. Use menu {File->Open}. Open file samples/PDB/1CF7.PDB
    GTFileDialog::openFile(os, dataDir+"samples/PDB/", "1CF7.PDB");

// Expected state: 
// 	1) Project view with documents "1CF7.PDB", "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");
    GTUtilsDocument::checkDocument(os, "1.gb");

// 6. Close project
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTGlobals::sleep();

// 7. Open project from the location used in item 3
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/sandbox/", "proj2.uprj");

// Expected state:
// Project has 2 documents: 1CF7.PDB and 1.gb
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");
}

}

}
