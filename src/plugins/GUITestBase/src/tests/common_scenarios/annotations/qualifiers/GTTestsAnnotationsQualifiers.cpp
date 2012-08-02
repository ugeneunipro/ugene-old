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

#include "GTTestsAnnotationsQualifiers.h"
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

namespace GUITest_common_scenarios_annotations_qualifiers {

GUI_TEST_CLASS_DEFINITION(test_0001) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "<auto>", "ann1", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "<auto>", "ann2", "complement(1.. 20)"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "<auto>", "ann3", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);

    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "ann1");
    GTUtilsAnnotationsTreeView::findItem(os, "ann2");
    GTUtilsAnnotationsTreeView::findItem(os, "ann3");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "DDD", "D", "join(10..16,18..20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "D");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "DDD", "D", "join(10..16,18..20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "D");
}

GUI_TEST_CLASS_DEFINITION(test_0004) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "a1_group", "a1", "10..16"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "a2_group", "a1", "18..20"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggle_HL_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "a1"));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_0005) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "a1_group", "a1", "10..16"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "a2_group", "a1", "18..20"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggle_HL_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "a1"));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_0006) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "<auto>", "ann1", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "<auto>", "ann2", "complement(1.. 20)"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "<auto>", "ann3", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);

    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "ann1");
    GTUtilsAnnotationsTreeView::findItem(os, "ann2");
    GTUtilsAnnotationsTreeView::findItem(os, "ann3");
}

GUI_TEST_CLASS_DEFINITION(test_0007) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj2.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "<auto>", "ann1", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "<auto>", "ann2", "complement(1.. 20)"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, "<auto>", "ann3", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);

    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "ann1");
    GTUtilsAnnotationsTreeView::findItem(os, "ann2");
    GTUtilsAnnotationsTreeView::findItem(os, "ann3");
}

} // namespace GUITest_common_scenarios_annotations_qualifiers
} // namespace U2
