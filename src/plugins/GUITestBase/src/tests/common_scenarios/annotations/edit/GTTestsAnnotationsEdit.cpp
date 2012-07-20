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

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "B");

    if ("B  (0, 2)" == item->parent()->text(0)) {
        GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item->parent()));
        GTMouseDriver::click(os);
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
        GTKeyboardUtils::selectAll(os);
        GTKeyboardDriver::keySequence(os, "BB");
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);
        GTGlobals::sleep(1000);
    }
    GTGlobals::sleep();

    QTreeWidgetItem *item1 = GTUtilsAnnotationsTreeView::findItem(os, "B");
    CHECK_SET_ERR("BB  (0, 2)" == item1->parent()->text(0), "Item BB not found in tree widget");

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

// 3. Select annotation C in annotation tree. Click F2. Change name to BB.

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "B");
    if ("B  (0, 2)" == item->parent()->text(0)) {
        GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item->parent()));
        GTMouseDriver::click(os);
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
        GTKeyboardUtils::selectAll(os);
        GTKeyboardDriver::keySequence(os, "BB");
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);
        GTGlobals::sleep(1000);
    }
    GTGlobals::sleep();

    QTreeWidgetItem *item1 = GTUtilsAnnotationsTreeView::findItem(os, "B");
    CHECK_SET_ERR("BB  (0, 2)" == item1->parent()->text(0), "Item BB not found in tree widget");

        GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item1->parent()));
        GTMouseDriver::click(os);
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
        GTKeyboardUtils::selectAll(os);
        GTKeyboardDriver::keySequence(os, "B");
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);
        GTGlobals::sleep(1000);

        QTreeWidgetItem *item2 = GTUtilsAnnotationsTreeView::findItem(os, "B");
        CHECK_SET_ERR("B  (0, 2)" == item2->parent()->text(0), "Item B not found in tree widget");
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

    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "B");
    if ("B  (0, 2)" == item->parent()->text(0)) {
        GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item->parent()));
        GTMouseDriver::click(os);
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
        GTKeyboardUtils::selectAll(os);
        GTKeyboardDriver::keySequence(os, "BB");
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);
        GTGlobals::sleep(1000);
    }
    GTGlobals::sleep();

    QTreeWidgetItem *item1 = GTUtilsAnnotationsTreeView::findItem(os, "B");
    CHECK_SET_ERR("BB  (0, 2)" == item1->parent()->text(0), "Item BB not found in tree widget");

        QTreeWidgetItem *item2 = GTUtilsAnnotationsTreeView::findItem(os, "C");
        if ("C  (0, 1)" == item2->parent()->text(0)) {

        GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item2->parent()));
        GTMouseDriver::click(os);
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
        GTKeyboardUtils::selectAll(os);
        GTKeyboardDriver::keySequence(os, "CC");
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);
        GTGlobals::sleep(1000);
            }

        QTreeWidgetItem *item3 = GTUtilsAnnotationsTreeView::findItem(os, "C");
        CHECK_SET_ERR("CC  (0, 1)" == item3->parent()->text(0), "Item CC not found in tree widget");
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

    GTUtilsAnnotationsTreeView::renameAndLocation(os, "C", "CC", "80 ..90", "80 ..90");
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

    GTUtilsAnnotationsTreeView::renameAndLocation(os, "C", "CC", "80 ..90", "80 ..90");
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "CC") != NULL, "Item CC not found in tree widget");

// 4. Select annotation C in annotation tree. Click F2. Change name to CC.

    GTUtilsAnnotationsTreeView::renameAndLocation(os, "CC", "C", "80 ..90", "80 ..90");
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

    GTUtilsAnnotationsTreeView::renameAndLocation(os, "C", "CC", "80 ..90", "80 ..90");
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "CC") != NULL, "Item CC not found in tree widget");

// 4. Select annotation B in annotation tree. Click F2. Change name to BB.

    GTUtilsAnnotationsTreeView::renameAndLocation(os, "B", "BB", "30 ..120", "30 ..120");
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

    GTUtilsAnnotationsTreeView::renameAndLocation(os, "C", "C", "80 ..90", "20 ..40");
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

    GTUtilsAnnotationsTreeView::renameAndLocation(os, "C", "C", "80 ..90", "20 ..40");
    GTGlobals::sleep();
    bool found = GTUtilsAnnotationsTreeView::findRegion(os, "C", U2Region(20, 40));
    CHECK_SET_ERR(found == true, "There is no {20 ..40} region in annotation");

// 4. Select annotation C in annotation tree. Click F2. Change name to CC.

    GTUtilsAnnotationsTreeView::renameAndLocation(os, "C", "C", "20 ..40", "10 ..90");
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

    GTUtilsAnnotationsTreeView::renameAndLocation(os, "C", "CC", "80 ..90", "20 ..40");
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "CC") != NULL, "Item CC not found in tree widget");

// 4. Select annotation B in annotation tree. Click F2. Change name to BB.

    GTUtilsAnnotationsTreeView::renameAndLocation(os, "B", "BB", "30 ..120", "20 ..40");
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "BB") != NULL, "Item BB not found in tree widget");

}

} // namespace GUITest_common_scenarios_annotations_edit
} // namespace U2
