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

#include "GTTestsAnnotationsEdit.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsSequenceView.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditAnnotationDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditGroupAnnotationsDialogFiller.h"


namespace U2 {

namespace GUITest_common_scenarios_annotations_edit {


GUI_TEST_CLASS_DEFINITION(test_0001) {
// Rename annotation
// 
// Steps:
// 
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Select annotation C in annotation tree. Click F2. Change name to BB.

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "B_group");

    Runnable *filler = new EditGroupAnnotationsFiller(os, "BB");
    GTUtilsDialog::waitForDialog(os, filler);

    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "BB") != NULL, "Item BB not found in tree widget");


}

GUI_TEST_CLASS_DEFINITION(test_0001_1) {
// Rename annotation
// 
// Steps:
// 
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Select annotation B in annotation tree. Click F2. Change name to BB.

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "B_group");

    Runnable *filler = new EditGroupAnnotationsFiller(os, "BB");
    GTUtilsDialog::waitForDialog(os, filler);

    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "BB") != NULL, "Item BB not found in tree widget");

    QTreeWidgetItem *item1 = GTUtilsAnnotationsTreeView::findItem(os, "BB");

    Runnable *filler1 = new EditGroupAnnotationsFiller(os, "B_group");
    GTUtilsDialog::waitForDialog(os, filler1);

    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item1));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "B_group") != NULL, "Item B_group not found in tree widget");

}


GUI_TEST_CLASS_DEFINITION(test_0001_2) {
// Rename annotation
// 
// Steps:
// 
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Select annotation C in annotation tree. Click F2. Change name to BB.

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "B_group");

    Runnable *filler = new EditGroupAnnotationsFiller(os, "BB");
    GTUtilsDialog::waitForDialog(os, filler);

    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "BB") != NULL, "Item BB not found in tree widget");


    QTreeWidgetItem *item1 = GTUtilsAnnotationsTreeView::findItem(os, "C_group");

    Runnable *filler1 = new EditGroupAnnotationsFiller(os, "CC");
    GTUtilsDialog::waitForDialog(os, filler1);

    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item1));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "CC") != NULL, "Item BB not found in tree widget");

}

GUI_TEST_CLASS_DEFINITION(test_0002) {
// Rename annotation
// 
// Steps:
// 
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Select annotation C in annotation tree. Click F2. Change name to CC.

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "C");
    Runnable *filler = new EditAnnotationFiller(os, "CC", "80 ..90");
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "CC") != NULL, "Item CC not found in tree widget");

}

GUI_TEST_CLASS_DEFINITION(test_0002_1) {
// Rename annotation
// 
// Steps:
// 
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Select annotation C in annotation tree. Click F2. Change name to CC.

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "C");
    Runnable *filler = new EditAnnotationFiller(os, "CC", "80 ..90");
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "CC") != NULL, "Item CC not found in tree widget");

    QTreeWidgetItem *item1 = GTUtilsAnnotationsTreeView::findItem(os, "CC");
    Runnable *filler1 = new EditAnnotationFiller(os, "C", "80 ..90");
    GTUtilsDialog::waitForDialog(os, filler1);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item1));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "C") != NULL, "Item C not found in tree widget");
}


GUI_TEST_CLASS_DEFINITION(test_0002_2) {
// Rename annotation
// 
// Steps:
// 
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Select annotation C in annotation tree. Click F2. Change name to CC.

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "C");
    Runnable *filler = new EditAnnotationFiller(os, "CC", "80 ..90");
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "CC") != NULL, "Item CC not found in tree widget");

    QTreeWidgetItem *item1 = GTUtilsAnnotationsTreeView::findItem(os, "B");
    Runnable *filler1 = new EditAnnotationFiller(os, "BB", "30 ..120");
    GTUtilsDialog::waitForDialog(os, filler1);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item1));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "BB") != NULL, "Item BB not found in tree widget");
}


GUI_TEST_CLASS_DEFINITION(test_0003) {
// Rename annotation
// 
// Steps:
// 
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Select annotation C in annotation tree. Click F2. Change name to CC.

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "C");
    Runnable *filler = new EditAnnotationFiller(os, "C", "20 ..40");
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    bool found = GTUtilsAnnotationsTreeView::findRegion(os, "C", U2Region(20, 40));
    CHECK_SET_ERR(found == true, "There is no {20 ..40} region in annotation");

}

GUI_TEST_CLASS_DEFINITION(test_0003_1) {
// Rename annotation
// 
// Steps:
// 
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Select annotation C in annotation tree. Click F2. Change name to CC.

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "C");
    Runnable *filler = new EditAnnotationFiller(os, "C", "20 ..40");
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    bool found = GTUtilsAnnotationsTreeView::findRegion(os, "C", U2Region(20, 40));
    CHECK_SET_ERR(found == true, "There is no {20 ..40} region in annotation");
// 4. Select annotation C in annotation tree. Click F2. Change name to CC.

    QTreeWidgetItem *item1 = GTUtilsAnnotationsTreeView::findItem(os, "C");
    Runnable *filler1 = new EditAnnotationFiller(os, "C", "10 ..90");
    GTUtilsDialog::waitForDialog(os, filler1);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item1));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    bool found1 = GTUtilsAnnotationsTreeView::findRegion(os, "C", U2Region(10, 90));
    CHECK_SET_ERR(found1 == true, "There is no {10 ..90} region in annotation");
}

GUI_TEST_CLASS_DEFINITION(test_0003_2) {
// Rename annotation
// 
// Steps:
// 
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Select annotation C in annotation tree. Click F2. Change name to CC.
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "C");
    Runnable *filler = new EditAnnotationFiller(os, "CC", "20 ..40");
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    bool found = GTUtilsAnnotationsTreeView::findRegion(os, "CC", U2Region(20, 40));
    CHECK_SET_ERR(found == true, "There is no {20 ..40} region in annotation");

    QTreeWidgetItem *item1 = GTUtilsAnnotationsTreeView::findItem(os, "B");
    Runnable *filler1 = new EditAnnotationFiller(os, "BB", "20 ..40");
    GTUtilsDialog::waitForDialog(os, filler1);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item1));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    bool found1 = GTUtilsAnnotationsTreeView::findRegion(os, "BB", U2Region(20, 40));
    CHECK_SET_ERR(found1 == true, "There is no {20 ..40} region in annotation");

}

GUI_TEST_CLASS_DEFINITION(test_0004) {
//Rename annotation

// Steps:
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Select annotation C in annotation tree. Click F2. Change name to CC.

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "C");
    Runnable *filler = new EditAnnotationFiller(os, "C", "20 ..40", true);
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    Runnable *checker = new EditAnnotationChecker(os, "", "complement(20..40)");
    GTUtilsDialog::waitForDialog(os, checker);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();
    }
GUI_TEST_CLASS_DEFINITION(test_0004_1) {
//Rename annotation

// Steps:
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Select annotation C in annotation tree. Click F2. Change name to CC.

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "C");
    Runnable *filler = new EditAnnotationFiller(os, "CC", "20 ..40", true);
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    Runnable *checker = new EditAnnotationChecker(os, "CC", "complement(20..40)");
    GTUtilsDialog::waitForDialog(os, checker);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();
    }
GUI_TEST_CLASS_DEFINITION(test_0004_2) {
//Rename annotation

// Steps:
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window titled with text "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// 3. Select annotation C in annotation tree. Click F2. Change name to CC.

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "B");
    Runnable *filler = new EditAnnotationFiller(os, "B", "20 ..40", true);
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    Runnable *checker = new EditAnnotationChecker(os, "", "complement(20..40)");
    GTUtilsDialog::waitForDialog(os, checker);
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();
    }

GUI_TEST_CLASS_DEFINITION(test_0005) {
    //Rename annotation
    // Steps:

    // 1. Open data/samples/FASTA/human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    //2. Press <Ctrl>+N
        //2.1 CHECK if dialog box titled with "Create annotation" appeared
        //2.2 CHECK if "Create new table" radio button is checked
    //3. Specify the annotation with the next data and the press "Create":
    //Group name: group
    //Annotation name: misc_feature
    //Location: 1..1000
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "group", "misc_feature", "1..1000"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    //4. CHECK if new gb-format document is loaded into the annotation editor
        //4.1 CHECK if it contains group "group" with annotation "misc_feature" in it
        //TODO: 4.2 CHECK if "group" subgroups/annotations counter displays (0,1)
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "group");
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature");

    //5. Select misc_feature annotation and press <DEL>
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Delete"]);
    GTGlobals::sleep();
    //6. CHECK if misc_feature annotation is removed
        //TODO: 6.1 CHECK if "group" subgroups/annotations counter displays (0,0)
    GTUtilsAnnotationsTreeView::findItem(os, "misc_feature", false);
    GTGlobals::sleep();

    }

GUI_TEST_CLASS_DEFINITION(test_0005_1) {
    //Rename annotation
    // Steps:

    // 1. Open data/samples/FASTA/human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    //2. Press <Ctrl>+N
        //2.1 CHECK if dialog box titled with "Create annotation" appeared
        //2.2 CHECK if "Create new table" radio button is checked
    //3. Specify the annotation with the next data and the press "Create":
    //Group name: group
    //Annotation name: misc_feature
    //Location: 1..1000
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "group", "misc_feature", "1..1000"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    //4. CHECK if new gb-format document is loaded into the annotation editor
        //4.1 CHECK if it contains group "group" with annotation "misc_feature" in it
        //TODO: 4.2 CHECK if "group" subgroups/annotations counter displays (0,1)
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "group");
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature");

    //5. Select misc_feature annotation and press <DEL>
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Delete"]);
    GTGlobals::sleep();
    //6. CHECK if misc_feature annotation is removed
        //TODO: 6.1 CHECK if "group" subgroups/annotations counter displays (0,0)
    GTUtilsAnnotationsTreeView::findItem(os, "misc_feature", false);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "group_new", "misc_feature_1", "1..500"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "group_new");
    GTUtilsAnnotationsTreeView::findItem(os, "group");
    QTreeWidgetItem *item1 = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature_1");

    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item1));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Delete"]);

    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "misc_feature_1", false);
    GTGlobals::sleep();

    }
GUI_TEST_CLASS_DEFINITION(test_0005_2) {

    //Rename annotation
    // Steps:

    // 1. Open data/samples/FASTA/human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    //2. Press <Ctrl>+N
        //2.1 CHECK if dialog box titled with "Create annotation" appeared
        //2.2 CHECK if "Create new table" radio button is checked
    //3. Specify the annotation with the next data and the press "Create":
    //Group name: group
    //Annotation name: misc_feature
    //Location: 1..1000
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "group", "misc_feature", "1..1000"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "group_new", "misc_feature_1", "1..500"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    //4. CHECK if new gb-format document is loaded into the annotation editor
        //4.1 CHECK if it contains group "group" with annotation "misc_feature" in it
        //TODO: 4.2 CHECK if "group" subgroups/annotations counter displays (0,1)
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "group");
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature");

    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "group_new");
    QTreeWidgetItem *item1 = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature_1");

    //5. Select misc_feature annotation and press <DEL>
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Delete"]);
    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item1));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Delete"]);
    GTGlobals::sleep();

    //6. CHECK if misc_feature annotation is removed
        //TODO: 6.1 CHECK if "group" subgroups/annotations counter displays (0,0)
    GTUtilsAnnotationsTreeView::findItem(os, "misc_feature", false);
    GTUtilsAnnotationsTreeView::findItem(os, "misc_feature_1", false);
    GTGlobals::sleep();

    }

} // namespace GUITest_common_scenarios_annotations_edit
} // namespace U2
