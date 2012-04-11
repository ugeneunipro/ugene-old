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

#include "GTTestsProjectUserLocking.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "GTUtilsProject.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

namespace GUITest_common_scenarios_project_user_locking {

GUI_TEST_CLASS_DEFINITION(test_0001) {

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj2.uprj");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");
    GTUtilsDocument::checkDocument(os, "1.gb");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);

    GTGlobals::sleep(2000);

    GTKeyboardDriver::keyClick(os, 'N', GTKeyboardDriver::key["ctrl"]);
    GTUtilsDialog::CreateAnnotationDialogChecker checker(os);
    GTUtilsDialog::waitForDialog(os, &checker);

    QString s = os.getError();
}

GUI_TEST_CLASS_DEFINITION(test_0003) {

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);

    ProjViewItem* item = (ProjViewItem*)GTUtilsProjectTreeView::findItem(os, "1.gb");
    CHECK_SET_ERR(item->controller != NULL, "Item controller is NULL");
    CHECK_SET_ERR(item->icon(0).cacheKey() == item->controller->documentIcon.cacheKey(), "Icon is locked");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1.gb"));
    GTUtilsDialog::PopupChooser lockPopupChooser(os, QStringList() << ACTION_DOCUMENT__LOCK);
    GTUtilsDialog::preWaitForDialog(os, &lockPopupChooser, GUIDialogWaiter::Popup);
    GTMouseDriver::click(os, Qt::RightButton);

    item = (ProjViewItem*)GTUtilsProjectTreeView::findItem(os, "1.gb");
    CHECK_SET_ERR(item->controller != NULL, "Item controller is NULL");
    CHECK_SET_ERR(item->icon(0).cacheKey() == item->controller->roDocumentIcon.cacheKey(), "Icon is unlocked");

    GTUtilsProject::saveProjectAs(os, "proj2", testDir+"_common_data/scenarios/sandbox", "proj2");

    GTUtilsProject::closeProject(os);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/sandbox/proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");

    item = (ProjViewItem*)GTUtilsProjectTreeView::findItem(os, "1.gb");
    CHECK_SET_ERR(item->controller != NULL, "Item controller is NULL");
    CHECK_SET_ERR(item->icon(0).cacheKey() == item->controller->roDocumentIcon.cacheKey(), "Icon is unlocked");
}

GUI_TEST_CLASS_DEFINITION(test_0005) {
	GTUtilsProject::openFiles(os, dataDir + "samples/ABIF/A01.abi");
	GTUtilsProject::openFiles(os, dataDir + "samples/Genbank/sars.gb");
	Document* d = GTUtilsDocument::getDocument(os,"A01.abi");
	CHECK_SET_ERR(!d->isModificationAllowed(StateLockModType_AddChild), QString("Enable to perform locking/unlocking for : %1").arg(d->getName()));

	d = GTUtilsDocument::getDocument(os,"sars.gb");
	// Needs to retest, so modification is allowed for sars.gb
	//CHECK_SET_ERR(!d->isModificationAllowed(StateLockModType_AddChild), QString("Enable to perform locking/unlocking for : %1").arg(d->getName()));
}
} // GUITest_common_scenarios_project_user_locking namespace

} // U2 namespace
