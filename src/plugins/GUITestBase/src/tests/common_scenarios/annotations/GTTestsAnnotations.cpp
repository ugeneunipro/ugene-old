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

#include <QTreeWidget>

#include "GTTestsAnnotations.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsSequenceView.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/ugene/corelibs/U2Gui/CreateObjectRelationDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportAnnotationsDialogFiller.h"
#include "runnables/ugene/ugeneui/DocumentFormatSelectorDialogFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"

#include <U2View/ADVConstants.h>

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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann1", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Create annotation using keyboard shortcut Ctrl+N
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann2", "complement(1.. 20)"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

// 5. Press right mouse button on sequence area, use context menu item {Add->New Annotation} to create annotation
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann3", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);

// Expected state: there is three new annotations on sequence created by threee different ways
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "ann1");
    GTUtilsAnnotationsTreeView::findItem(os, "ann2");
    GTUtilsAnnotationsTreeView::findItem(os, "ann3");
}

GUI_TEST_CLASS_DEFINITION(test_0001_1) {
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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann1", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Create annotation using keyboard shortcut Ctrl+N
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann2", "complement(1.. 20)"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

// 5. Press right mouse button on sequence area, use context menu item {Add->New Annotation} to create annotation
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann3", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);

// Expected state: there is three new annotations on sequence created by threee different ways
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "ann1");
    GTUtilsAnnotationsTreeView::findItem(os, "ann2");
    GTUtilsAnnotationsTreeView::findItem(os, "ann3");
}

GUI_TEST_CLASS_DEFINITION(test_0001_2) {
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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann1", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Create annotation using keyboard shortcut Ctrl+N
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann2", "complement(1.. 20)"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

// 5. Press right mouse button on sequence area, use context menu item {Add->New Annotation} to create annotation
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann3", "complement(1.. 20)"));
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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "DDD", "D", "join(10..16,18..20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// Expected state: annotation with 2 segments has been created
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "D");
}

GUI_TEST_CLASS_DEFINITION(test_0002_1) {
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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "DDD", "D", "join(10..16,18..20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// Expected state: annotation with 2 segments has been created
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "D");
}

GUI_TEST_CLASS_DEFINITION(test_0002_2) {
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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "DDD", "D", "join(10..16,18..20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// Expected state: annotation with 2 segments has been created
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "D");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann1", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Create annotation using keyboard shortcut Ctrl+N
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann2", "complement(1.. 20)"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

// 5. Press right mouse button on sequence area, use context menu item {Add->New Annotation} to create annotation
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann3", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);

// Expected state: there is three new annotations on sequence created by threee different ways
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "ann1");
    GTUtilsAnnotationsTreeView::findItem(os, "ann2");
    GTUtilsAnnotationsTreeView::findItem(os, "ann3");
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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "a1_group", "a1", "10..16"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

//     2) a1 in group a2
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "a2_group", "a1", "18..20"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Toggle highlight for a1.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggle_HL_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "a1"));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: both annotations (a1) and groups (a1, a2) looks muted (grayed out)
}

GUI_TEST_CLASS_DEFINITION(test_0004_1) {
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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "a1_group", "a1", "10..16"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

//     2) a1 in group a2
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "a2_group", "a1", "18..20"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Toggle highlight for a1.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggle_HL_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "a1"));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: both annotations (a1) and groups (a1, a2) looks muted (grayed out)
}

GUI_TEST_CLASS_DEFINITION(test_0004_2) {
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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "a1_group", "a1", "10..16"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

//     2) a1 in group a2
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "a2_group", "a1", "18..20"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Toggle highlight for a1.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggle_HL_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "a1"));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: both annotations (a1) and groups (a1, a2) looks muted (grayed out)
}

GUI_TEST_CLASS_DEFINITION(test_0005) {
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

    // 5. Press right mouse button on sequence area, use context menu item {Add->New Annotation} to create annotation
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann3", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);

// 3. Create annotation using menu {Actions->Add->New Annotation}
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann1", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Create annotation using keyboard shortcut Ctrl+N
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann2", "complement(1.. 20)"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

// Expected state: there is three new annotations on sequence created by threee different ways
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "ann1");
    GTUtilsAnnotationsTreeView::findItem(os, "ann2");
    GTUtilsAnnotationsTreeView::findItem(os, "ann3");
}

GUI_TEST_CLASS_DEFINITION(test_0006) {
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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "DDD", "D", "join(10..16,18..20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// Expected state: annotation with 2 segments has been created
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "D");
}

GUI_TEST_CLASS_DEFINITION(test_0007) {
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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "a1_group", "a1", "10..16"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

//     2) a1 in group a2
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "a2_group", "a1", "18..20"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Toggle highlight for a1.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggle_HL_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "a1"));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: both annotations (a1) and groups (a1, a2) looks muted (grayed out)
}

GUI_TEST_CLASS_DEFINITION(test_0008) {
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
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "DDD", "D", "join(10..16,18..20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// Expected state: annotation with 2 segments has been created
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "D");
}

GUI_TEST_CLASS_DEFINITION(test_0009) {
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

// 5. Press right mouse button on sequence area, use context menu item {Add->New Annotation} to create annotation
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann3", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);

// 3. Create annotation using menu {Actions->Add->New Annotation}
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann1", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Create annotation using keyboard shortcut Ctrl+N
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann2", "complement(1.. 20)"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

// Expected state: there is three new annotations on sequence created by threee different ways
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "ann1");
    GTUtilsAnnotationsTreeView::findItem(os, "ann2");
    GTUtilsAnnotationsTreeView::findItem(os, "ann3");
}

GUI_TEST_CLASS_DEFINITION(test_0010_1) {
    //    BED

    //    Export annotations from different annotation table objects
    //    The following sceanrio should work with BED, GFF and GTF formats.
    //    1. Open '_common_data/fasta/DNA.fa' as separate sequences
    //    2. Select a few annotations from different annotation table objects
    //    3. {Export -> Export annotations...}, select on of the following formats: BED, GFF, GTF
    //    Expected state: 'Export annotations' dialog appeared
    //    4. Open exported file
    //    Expected state: there are two annotation table objects

    GTUtilsDialog::waitForDialog( os, new SequenceReadingModeSelectorDialogFiller( os,
        SequenceReadingModeSelectorDialogFiller::Separate ) );
    GTFileDialog::openFile(os, testDir + "_common_data/fasta", "DNA.fa");
    CHECK_SET_ERR( GTUtilsProjectTreeView::checkItem(os, "GXL_141619"), "No GXL_141619 object!");
    CHECK_SET_ERR( GTUtilsProjectTreeView::checkItem(os, "GXL_141618"), "No GXL_141618 object!");

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "ann_1", "200..300",
                                                                      sandBoxDir + "ann_test_0010_1_19.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "ann_2", "100..200",
                                                                      sandBoxDir + "ann_test_0010_1_18.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_1"), Qt::RightButton);

    QStringList annList;
    annList << "ann_1" << "ann_2";
    GTUtilsAnnotationsTreeView::selectItems(os, annList);

    GTUtilsDialog::waitForDialog(os, new ExportAnnotationsFiller(sandBoxDir + "ann_export_test_0010_1.bed",
                                                                 ExportAnnotationsFiller::bed, os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EXPORT << "action_export_annotations"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTUtilsDocument::removeDocument(os, "DNA.fa");
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "No"));
    GTUtilsDocument::removeDocument(os, "ann_test_0010_1_18.gb");
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "No"));
    GTUtilsDocument::removeDocument(os, "ann_test_0010_1_19.gb");
    GTGlobals::sleep();

    GTFileDialog::openFile(os, sandBoxDir, "ann_export_test_0010_1.bed");
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, "GXL_141619 features"), "No GXL_141619 features object!");
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, "GXL_141618 features"), "No GXL_141618 features object!");
}

GUI_TEST_CLASS_DEFINITION(test_0010_2) {
    //    GFF

    //    Export annotations from different annotation table objects
    //    The following sceanrio should work with BED, GFF and GTF formats.
    //    1. Open '_common_data/fasta/DNA.fa' as separate sequences
    //    2. Select a few annotations from different annotation table objects
    //    3. {Export -> Export annotations...}, select on of the following formats: BED, GFF, GTF
    //    Expected state: 'Export annotations' dialog appeared
    //    4. Open exported file
    //    Expected state: there are two annotation table objects

    GTUtilsDialog::waitForDialog( os, new SequenceReadingModeSelectorDialogFiller( os,
        SequenceReadingModeSelectorDialogFiller::Separate ) );
    GTFileDialog::openFile(os, testDir + "_common_data/fasta", "DNA.fa");
    CHECK_SET_ERR( GTUtilsProjectTreeView::checkItem(os, "GXL_141619"), "No GXL_141619 object!");
    CHECK_SET_ERR( GTUtilsProjectTreeView::checkItem(os, "GXL_141618"), "No GXL_141618 object!");

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "ann_1", "200..300",
                                                                      sandBoxDir + "ann_test_0010_2_19.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "ann_2", "100..200",
                                                                      sandBoxDir + "ann_test_0010_2_18.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_1"), Qt::RightButton);

    QStringList annList;
    annList << "ann_1" << "ann_2";
    GTUtilsAnnotationsTreeView::selectItems(os, annList);

    GTUtilsDialog::waitForDialog(os, new ExportAnnotationsFiller(sandBoxDir + "ann_export_test_0010_2.gff",
                                                                 ExportAnnotationsFiller::gff, os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EXPORT << "action_export_annotations"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTUtilsDocument::removeDocument(os, "DNA.fa");
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "No"));
    GTUtilsDocument::removeDocument(os, "ann_test_0010_2_18.gb");
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "No"));
    GTUtilsDocument::removeDocument(os, "ann_test_0010_2_19.gb");
    GTGlobals::sleep();

    GTFileDialog::openFile(os, sandBoxDir, "ann_export_test_0010_2.gff");
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, "GXL_141619 features"), "No GXL_141619 features object!");
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, "GXL_141618 features"), "No GXL_141618 features object!");
}

GUI_TEST_CLASS_DEFINITION(test_0010_3) {
    //    GTF

    //    Export annotations from different annotation table objects
    //    The following sceanrio should work with BED, GFF and GTF formats.
    //    1. Open '_common_data/fasta/DNA.fa' as separate sequences
    //    2. Select a few annotations from different annotation table objects
    //    3. {Export -> Export annotations...}, select on of the following formats: BED, GFF, GTF
    //    Expected state: 'Export annotations' dialog appeared
    //    4. Open exported file
    //    Expected state: there are two annotation table objects

    GTUtilsDialog::waitForDialog( os, new SequenceReadingModeSelectorDialogFiller( os,
        SequenceReadingModeSelectorDialogFiller::Separate ) );
    GTFileDialog::openFile(os, testDir + "_common_data/fasta", "DNA.fa");
    CHECK_SET_ERR( GTUtilsProjectTreeView::checkItem(os, "GXL_141619"), "No GXL_141619 object!");
    CHECK_SET_ERR( GTUtilsProjectTreeView::checkItem(os, "GXL_141618"), "No GXL_141618 object!");

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "ann_1", "200..300",
                                                                      sandBoxDir + "ann_test_0010_3_19.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);


    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "ann_2", "100..200",
                                                                      sandBoxDir + "ann_test_0010_3_18.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_1"), Qt::RightButton);


    QStringList annList;
    annList << "ann_1" << "ann_2";
    GTUtilsAnnotationsTreeView::selectItems(os, annList);
    GTUtilsAnnotationsTreeView::createQualifier(os, "gene_id", "YT483", "ann_1");
    GTUtilsAnnotationsTreeView::createQualifier(os, "transcript_id", "001T", "ann_1");
    GTUtilsAnnotationsTreeView::createQualifier(os, "gene_id", "YT496", "ann_2");
    GTUtilsAnnotationsTreeView::createQualifier(os, "transcript_id", "0012", "ann_2");
    GTUtilsAnnotationsTreeView::selectItems(os, annList);

    GTUtilsDialog::waitForDialog(os, new ExportAnnotationsFiller(sandBoxDir + "ann_export_test_0010_3.gtf",
                                                                 ExportAnnotationsFiller::gtf, os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EXPORT << "action_export_annotations"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTUtilsDocument::removeDocument(os, "DNA.fa");
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "No"));
    GTUtilsDocument::removeDocument(os, "ann_test_0010_3_18.gb");
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "No"));
    GTUtilsDocument::removeDocument(os, "ann_test_0010_3_19.gb");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DocumentFormatSelectorDialogFiller(os, "gtf"));
    GTFileDialog::openFile(os, sandBoxDir, "ann_export_test_0010_3.gtf");
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, "GXL_141619 features"), "No GXL_141619 features object!");
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, "GXL_141618 features"), "No GXL_141618 features object!");
}

GUI_TEST_CLASS_DEFINITION(test_0011_1) {
    // gene_id and transcript_id attributes are required for GTF format!

    //    1. Open "human_T1.fa"
    //    2. Create an annotation
    //    3. Try to export it to GTF format
    //    Expected state: error message is in the log(GTF annotation MUST have gene_id and transcript_id attribute!)

    GTLogTracer l;

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "exon", "annotation", "200..300",
                                                                      sandBoxDir + "ann_test_0011_1.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);
    GTWidget::click(os, GTUtilsAnnotationsTreeView::getTreeWidget(os));
    GTUtilsAnnotationsTreeView::createQualifier(os, "transcript_id", "TR321", "annotation");

    GTUtilsAnnotationsTreeView::selectItems(os, QStringList() << "annotation");

    GTUtilsDialog::waitForDialog(os, new ExportAnnotationsFiller(sandBoxDir + "ann_export_test_0011_1.gtf",
                                                                 ExportAnnotationsFiller::gtf, os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EXPORT << "action_export_annotations"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    CHECK_SET_ERR(l.hasError(), "No error in the log");
}

GUI_TEST_CLASS_DEFINITION(test_0011_2) {
    // gene_id and transcript_id attributes are required for GTF format!

    //    1. Open "human_T1.fa"
    //    2. Create an annotation
    //    3. Try to export it to GTF format
    //    Expected state: error message is in the log(GTF annotation MUST have gene_id and transcript_id attribute!)

    GTLogTracer l;

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "ann", "200..300",
                                                                      sandBoxDir + "ann_test_0011_1.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);
    GTWidget::click(os, GTUtilsAnnotationsTreeView::getTreeWidget(os));
    GTUtilsAnnotationsTreeView::createQualifier(os, "gene_id", "XCV", "ann");

    GTUtilsAnnotationsTreeView::selectItems(os, QStringList() << "ann");

    GTUtilsDialog::waitForDialog(os, new ExportAnnotationsFiller(sandBoxDir + "ann_export_test_0011_1.gtf",
                                                                 ExportAnnotationsFiller::gtf, os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EXPORT << "action_export_annotations"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    CHECK_SET_ERR(l.hasError(), "No erro in the log");
}

GUI_TEST_CLASS_DEFINITION(test_0011_3) {
    // gene_id and transcript_id attributes are required for GTF format!

    //    1. Open "human_T1.fa"
    //    2. Create an annotation
    //    4. Add 'gene_id' and 'transcript_id' quilifiers
    //    3. Export it to GTF format
    //    Expected state: no errors in the log

    GTLogTracer l;

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "ann", "200..300",
                                                                      sandBoxDir + "ann_test_0011_1.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);
    GTWidget::click(os, GTUtilsAnnotationsTreeView::getTreeWidget(os));
    GTUtilsAnnotationsTreeView::createQualifier(os, "gene_id", "XCV", "ann");
    GTUtilsAnnotationsTreeView::createQualifier(os, "transcript_id", "TR321", "ann");

    GTUtilsAnnotationsTreeView::selectItems(os, QStringList() << "ann");

    GTUtilsDialog::waitForDialog(os, new ExportAnnotationsFiller(sandBoxDir + "ann_export_test_0011_1.gtf",
                                                                 ExportAnnotationsFiller::gtf, os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EXPORT << "action_export_annotations"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_0012_1) {
    // BED

    //    1. Open sequence
    //    2. Open annotation file
    //    3. Create a relation between the first annotation object and sequence
    //    4. Open another annotation file
    //    5. Create a relation between the second annotation object and sequence
    //    Expected state: there are 2 ann.objects in annotation tree view
    //    6. Select a few annotation from different annotation table objects
    //    7. Export them  to BED, GTF ot GFF format
    //    8. Open the result file
    //    Expected state: annotation table object name contain sequence name

    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "sars.gb");
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, "NC_004718 features"), "Object not found");

    GTFileDialog::openFile(os, testDir + "_common_data/gff/", "scaffold_90.gff");
    GTUtilsProjectTreeView::checkItem(os, "scaffold_90 features");

    QModelIndex idx = GTUtilsProjectTreeView::findIndex(os, "scaffold_90 features");
    QWidget* sequence = GTUtilsSequenceView::getSeqWidgetByNumber(os);
    CHECK_SET_ERR(sequence != NULL, "Sequence widget not found");

    GTUtilsDialog::waitForDialog(os, new CreateObjectRelationDialogFiller(os));
    GTUtilsProjectTreeView::dragAndDrop(os, idx, sequence);

    QStringList annList;
    annList << "5'UTR" << "exon";

    GTUtilsAnnotationsTreeView::selectItems(os, annList);

    GTUtilsDialog::waitForDialog(os, new ExportAnnotationsFiller(sandBoxDir + "ann_export_test_0012_1.bed",
                                                                 ExportAnnotationsFiller::bed, os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EXPORT << "action_export_annotations"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTUtilsDocument::removeDocument(os, "sars.gb");
    GTUtilsDocument::removeDocument(os, "scaffold_90.gff");
    GTGlobals::sleep();

    GTFileDialog::openFile(os, sandBoxDir, "ann_export_test_0012_1.bed");
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, "NC_004718 features"), "Object not found");
    CHECK_SET_ERR(!GTUtilsProjectTreeView::checkItem(os, "scaffold_90 features"), "Object shound not be in the project");
}

GUI_TEST_CLASS_DEFINITION(test_0012_2) {
    // GFF

    //    1. Open sequence
    //    2. Open annotation file
    //    3. Create a relation between the first annotation object and sequence
    //    4. Open another annotation file
    //    5. Create a relation between the second annotation object and sequence
    //    Expected state: there are 2 ann.objects in annotation tree view
    //    6. Select a few annotation from different annotation table objects
    //    7. Export them  to BED, GTF ot GFF format
    //    8. Open the result file
    //    Expected state: annotation table object name contain sequence name

    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "sars.gb");
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, "NC_004718 features"), "Object not found");

    GTFileDialog::openFile(os, testDir + "_common_data/gff/", "scaffold_90.gff");
    GTUtilsProjectTreeView::checkItem(os, "scaffold_90 features");

    QModelIndex idx = GTUtilsProjectTreeView::findIndex(os, "scaffold_90 features");
    QWidget* sequence = GTUtilsSequenceView::getSeqWidgetByNumber(os);
    CHECK_SET_ERR(sequence != NULL, "Sequence widget not found");

    GTUtilsDialog::waitForDialog(os, new CreateObjectRelationDialogFiller(os));
    GTUtilsProjectTreeView::dragAndDrop(os, idx, sequence);

    QStringList annList;
    annList << "5'UTR" << "exon";

    GTUtilsAnnotationsTreeView::selectItems(os, annList);

    GTUtilsDialog::waitForDialog(os, new ExportAnnotationsFiller(sandBoxDir + "ann_export_test_0012_2.gff",
                                                                 ExportAnnotationsFiller::gff, os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EXPORT << "action_export_annotations"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTUtilsDocument::removeDocument(os, "sars.gb");
    GTUtilsDocument::removeDocument(os, "scaffold_90.gff");
    GTGlobals::sleep();

    GTFileDialog::openFile(os, sandBoxDir, "ann_export_test_0012_2.gff");
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, "NC_004718 features"), "Object not found");
    CHECK_SET_ERR(!GTUtilsProjectTreeView::checkItem(os, "scaffold_90 features"), "Object shound not be in the project");

}

GUI_TEST_CLASS_DEFINITION(test_0012_3) {
    // GTF - gtf annotations MUST have gene_id and transcript_id attributes

    //    1. Open sequence
    //    2. Open annotation file
    //    3. Create a relation between the first annotation object and sequence
    //    4. Open another annotation file
    //    5. Create a relation between the second annotation object and sequence
    //    Expected state: there are 2 ann.objects in annotation tree view
    //    6. Select a few annotation from different annotation table objects
    //    7. Export them  to BED, GTF ot GFF format
    //    8. Open the result file
    //    Expected state: annotation table object name contain sequence name

    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "sars.gb");
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, "NC_004718 features"), "Object not found");

    GTFileDialog::openFile(os, testDir + "_common_data/gff/", "scaffold_90.gff");
    GTUtilsProjectTreeView::checkItem(os, "scaffold_90 features");

    QModelIndex idx = GTUtilsProjectTreeView::findIndex(os, "scaffold_90 features");
    QWidget* sequence = GTUtilsSequenceView::getSeqWidgetByNumber(os);
    CHECK_SET_ERR(sequence != NULL, "Sequence widget not found");

    GTUtilsDialog::waitForDialog(os, new CreateObjectRelationDialogFiller(os));
    GTUtilsProjectTreeView::dragAndDrop(os, idx, sequence);

    GTUtilsAnnotationsTreeView::createQualifier(os, "gene_id", "XCV", "exon");
    GTUtilsAnnotationsTreeView::createQualifier(os, "transcript_id", "TR321", "exon");
    GTUtilsAnnotationsTreeView::createQualifier(os, "gene_id", "XCV", "5'UTR");
    GTUtilsAnnotationsTreeView::createQualifier(os, "transcript_id", "TR321", "5'UTR");

    QStringList annList;
    annList << "5'UTR" << "exon";

    GTUtilsAnnotationsTreeView::selectItems(os, annList);

    GTUtilsDialog::waitForDialog(os, new ExportAnnotationsFiller(sandBoxDir + "ann_export_test_0012_3.gtf",
                                                                 ExportAnnotationsFiller::gtf, os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EXPORT << "action_export_annotations"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "No"));
    GTUtilsDocument::removeDocument(os, "sars.gb");
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "No"));
    GTUtilsDocument::removeDocument(os, "scaffold_90.gff");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DocumentFormatSelectorDialogFiller(os, "gtf"));
    GTFileDialog::openFile(os, sandBoxDir, "ann_export_test_0012_3.gtf");
    CHECK_SET_ERR(GTUtilsProjectTreeView::checkItem(os, "NC_004718 features"), "Object not found");
    CHECK_SET_ERR(!GTUtilsProjectTreeView::checkItem(os, "scaffold_90 features"), "Object shound not be in the project");
}

} // namespace GUITest_common_scenarios_annotations
} // namespace U2
