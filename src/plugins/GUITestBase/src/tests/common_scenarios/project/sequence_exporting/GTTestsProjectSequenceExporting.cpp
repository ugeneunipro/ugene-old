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

#include "GTTestsProjectSequenceExporting.h"
#include "api/GTGlobals.h"
#include <api/GTFileDialog.h>
#include <api/GTKeyboardDriver.h>
#include "api/GTMenu.h"
#include <api/GTMouseDriver.h>
#include "GTUtilsProject.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsApp.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsMdi.h"
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>
#include <U2Core/DocumentModel.h>

namespace U2{

namespace GUITests_sequence_exporting_scenarios_project {

GUI_TEST_CLASS_DEFINITION(test_0001) {
	GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj4.uprj");

	CHECK_SET_ERR(!GTUtilsDocument::getDocument(os, "1.gb")->isLoaded(), "1.gb is loaded");
	CHECK_SET_ERR(!GTUtilsDocument::getDocument(os, "2.gb")->isLoaded(), "2.gb is loaded");
	GTUtilsApp::checkUGENETitle(os, "proj4 UGENE");

	GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 sequence"));
	GTMouseDriver::doubleClick(os);
	GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);
	GTUtilsMdi::selectRandomRegion(os, "1 [s] NC_001363 sequence");
	GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
	GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj4.uprj");

	GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 sequence"));
	GTMouseDriver::doubleClick(os);
	GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);
	GTMouseDriver::moveTo(os, GTUtilsMdi::getMdiItemPosition(os, "1 [s] NC_001363 sequence"));
}

}

}
