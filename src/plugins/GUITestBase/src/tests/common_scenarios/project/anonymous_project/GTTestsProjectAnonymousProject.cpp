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
#include "GTUtilsProject.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include <U2View/AnnotatedDNAViewFactory.h>
#include "GTUtilsToolTip.h"
#include "GTUtilsDialogRunnables.h"
#include "api/GTFileDialog.h"
#include "GTUtilsProject.h"

namespace U2{

namespace GUITest_common_scenarios_project_anonymous_project{

GUI_TEST_CLASS_DEFINITION(test_0002) {

	GTFileDialog::openFile(os, dataDir+"samples/PDB/", "1CF7.PDB");
	GTGlobals::sleep(5000);
	GTUtilsDocument::checkDocument(os, "1CF7.PDB");
	GTUtilsProject::exportProject(os, testDir + "_common_data/scenarios/sandbox", "proj2.uprj");
	GTUtilsDialogRunnables::MessageBoxDialogFiller filler(os, QMessageBox::No);
	GTUtilsDialog::waitForDialog(os, &filler, GUIDialogWaiter::Modal, false);
	GTUtilsProject::closeProject(os);
	GTGlobals::sleep(2000);

	GTFileDialog::openFile(os, testDir+"_common_data/scenarios/sandbox/", "proj2.uprj");
	GTUtilsDocument::checkDocument(os, "1CF7.PDB");
	GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");
	GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1CF7.PDB"));
	GTGlobals::sleep(2000);
	GTUtilsToolTip::checkExistingToolTip(os, "_common_data/scenarios/sandbox/1CF7.PDB");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {

	GTFileDialog::openFile(os, dataDir+"samples/PDB/", "1CF7.PDB");
	GTUtilsDocument::checkDocument(os, "1CF7.PDB");
	GTUtilsProject::saveProjectAs(os, "proj2", testDir+"_common_data/scenarios/sandbox", "proj2");
	GTUtilsProject::closeProject(os);
	
	GTFileDialog::openFile(os, testDir+"_common_data/scenarios/sandbox/", "proj2.uprj");
	GTUtilsDocument::checkDocument(os, "1CF7.PDB");
	GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");
	GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1CF7.PDB"));
	GTGlobals::sleep(2000);
	GTUtilsToolTip::checkExistingToolTip(os, "samples/PDB/1CF7.PDB");
}

}

}
