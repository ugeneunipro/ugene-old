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

#include "GTTestsAnnotations.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsSequenceView.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"

namespace U2 {

namespace GUITest_common_scenarios_annotations {

GUI_TEST_CLASS_DEFINITION(test_0001) {
// Creating annotations by different ways
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

// 3. Create annotation using menu {Actions->Add->New Annotation}
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "<auto>", "ann1", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Create annotation using keyboard shortcut Ctrl+N
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "<auto>", "ann2", "complement(1.. 20)"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

// 5. Press right mouse button on sequence area, use context menu item {Add->New Annotation} to create annotation
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "<auto>", "ann3", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);

// Expected state: there is three new annotations on sequence created by threee different ways
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "ann1");
    GTUtilsAnnotationsTreeView::findItem(os, "ann2");
    GTUtilsAnnotationsTreeView::findItem(os, "ann3");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
// Creating joined annotation
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
//     
// 2. Open view for "1.gb"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
// 
// 3. Do menu {Actions->Add->New Annotation}
// Expected state: "Create annotation" dialog has appeared
// 
// 3. Fill the next field in dialog:
//     {Group Name} DDD
//     {Annotation Name} D
//     {Location} join(10..16,18..20)
// 
// 4. Click Create button
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "DDD", "D", "join(10..16,18..20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// Expected state: annotation with 2 segments has been created
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "D");
}

GUI_TEST_CLASS_DEFINITION(test_0004) {
// Annotation editor: update annotations incorrect behavior (0001585)
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

// 3. Create 2 annotations:
//     1) a1 in group a1
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "a1", "a1", "10..16"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

//     2) a1 in group a2
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "a2", "a1", "18..20"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Toggle highlight for a1. 
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggle_HL_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "a1"));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: both annotations (a1) and groups (a1, a2) looks muted (grayed out)
}

} // namespace GUITest_common_scenarios_annotations
} // namespace U2
