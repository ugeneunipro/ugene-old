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
#include "api/GTFile.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsProject.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsApp.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsMdi.h"
#include "GTSequenceViewUtils.h"
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>
#include <U2Core/DocumentModel.h>
#include <U2View/ADVConstants.h>
#include <U2Core/AppContext.h>

namespace U2{

namespace GUITest_common_scenarios_project_sequence_exporting {

GUI_TEST_CLASS_DEFINITION(test_0001) {
	GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj4.uprj");

	CHECK_SET_ERR(!GTUtilsDocument::getDocument(os, "1.gb")->isLoaded(), "1.gb is loaded");
	CHECK_SET_ERR(!GTUtilsDocument::getDocument(os, "2.gb")->isLoaded(), "2.gb is loaded");
	GTUtilsApp::checkUGENETitle(os, "proj4 UGENE");

	GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 sequence"));
	GTMouseDriver::doubleClick(os);
	GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);
	GTUtilsDialog::PopupChooser popupChooser(os, QStringList() << "ADV_MENU_EXPORT" << "action_export_selected_sequence_region", GTGlobals::UseMouse);
	GTUtilsDialog::ExportSelectedRegionFiller filler(os, "_common_data/scenarios/sandbox/", "exp.fasta", GTGlobals::UseMouse);
	GTUtilsDialog::preWaitForDialog(os, &popupChooser, GUIDialogWaiter::Popup);
	GTUtilsMdi::selectRandomRegion(os, "1 [s] NC_001363 sequence");
	GTUtilsDialog::preWaitForDialog(os, &filler, GUIDialogWaiter::Modal);
	GTMouseDriver::click(os, Qt::RightButton);
	GTGlobals::sleep(100);
	GTUtilsDocument::checkDocument(os, "exp.fasta");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
	GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj4.uprj");

	GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 sequence"));
	GTMouseDriver::doubleClick(os);
	GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);
	GTGlobals::sleep(100);
	QTreeWidgetItem* item = GTUtilsAnnotationsTreeView::findItem(os, "C");
	CHECK_SET_ERR(item != NULL, "AnnotationsTreeView is NULL");
	GTUtilsDialog::PopupChooser popupChooser(os, QStringList() << "ADV_MENU_EXPORT" << "action_export_sequence_of_selected_annotations", GTGlobals::UseMouse);
	GTUtilsDialog::ExportSequenceOfSelectedAnnotationsFiller filler(os, 
		"_common_data/scenarios/sandbox/exp.fasta",
		GTUtilsDialog::ExportSequenceOfSelectedAnnotationsFiller::Fasta,
		GTUtilsDialog::ExportSequenceOfSelectedAnnotationsFiller::SaveAsSeparate,
		0
	);
	GTUtilsDialog::preWaitForDialog(os, &popupChooser, GUIDialogWaiter::Popup);
	GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "C"));
	GTUtilsDialog::preWaitForDialog(os, &filler, GUIDialogWaiter::Modal);
	GTMouseDriver::click(os, Qt::RightButton);
	GTGlobals::sleep(1000);
}

GUI_TEST_CLASS_DEFINITION(test_0003)
{
    const QString doc1("1.gb"), doc2("2.gb"), expectedSequences("ACCCCACCCGTAGGTGGCAAGCTAGCTTAAG");

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj4.uprj");
    GTGlobals::sleep(1000);

    QTreeWidgetItem *item1 = GTUtilsProjectTreeView::findItem(os, doc1);
    QTreeWidgetItem *item2 = GTUtilsProjectTreeView::findItem(os, doc2);
    if (item1 == NULL || item2 == NULL) {
        os.setError("Project view with document \"1.gb\" and \"2.gb\" is not opened");
        return;
    }

    GTUtilsApp::checkUGENETitle(os, "proj4 UGENE");

    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, "Annotations");
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep(1000);

    GTUtilsDocument::checkDocument(os, doc2, AnnotatedDNAViewFactory::ID);

    GTUtilsProject::exportSequenceOfSelectedAnnotations(os, "B_joined", testDir + "_common_data/scenarios/sandbox/exp.fasta",
                                                        GTUtilsDialog::ExportSequenceOfSelectedAnnotationsFiller::Fasta,
                                                        GTUtilsDialog::ExportSequenceOfSelectedAnnotationsFiller::Merge, 5);

    GTGlobals::sleep(100);

    GTSequenceViewUtils::checkSequence(os, expectedSequences);
}

GUI_TEST_CLASS_DEFINITION(test_0004) {

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj4.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsDocument::checkDocument(os, "2.gb");
    GTUtilsApp::checkUGENETitle(os, "proj4 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    GTMouseDriver::doubleClick(os);
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);

    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B_joined"));
    GTUtilsDialog::PopupChooser popupChooser(os, QStringList() << ADV_MENU_EXPORT << ACTION_EXPORT_ANNOTATIONS);
    GTUtilsDialog::preWaitForDialog(os, &popupChooser, GUIDialogWaiter::Popup);

    GTUtilsDialog::ExportAnnotationsDialogFiller filler(os, testDir+"_common_data/scenarios/sandbox/1.csv");
    GTUtilsDialog::preWaitForDialog(os, &filler, GUIDialogWaiter::Modal);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(100);

    GTGlobals::sleep(1000);
    bool equals = GTFile::equals(os, testDir+"_common_data/scenarios/sandbox/1.csv", testDir+"_common_data/scenarios/project/test_0004.csv");
    CHECK_SET_ERR(equals == true, "Exported file differs from the test file");
}

}

}
