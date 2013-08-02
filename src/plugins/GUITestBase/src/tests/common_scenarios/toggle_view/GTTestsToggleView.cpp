/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "GTTestsToggleView.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTMenu.h"
#include "api/GTAction.h"
#include "api/GTFileDialog.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsProject.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsApp.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsMdi.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsMdi.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/RangeSelectorFiller.h"

#include <U2Core/DocumentModel.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>
#include <U2View/ADVConstants.h>
#include <QtGui/QClipboard>
#include <ApplicationServices/ApplicationServices.h>

namespace U2 {

namespace GUITest_common_scenarios_toggle_view {

GUI_TEST_CLASS_DEFINITION(test_0001) {

// 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

// 2. Click on toolbar button Toggle view for sequence se2. Click menu item Remove sequence.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "remove_sequence"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

// Expected state: views for se2 sequence has been closed
    GTGlobals::sleep();
    QWidget *sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 == NULL, "sequenceWidget is present");

    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2", NULL, false);
    QWidget *DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    QWidget *zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2", NULL, false);
    QWidget *toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2", NULL, false);
    CHECK_SET_ERR(overViewSe2 == NULL && DetailsViewSe2 == NULL && zoomViewSe2 == NULL && toolBarSe2 == NULL, "there are widgets of ADV_single_sequence_widget");
}

GUI_TEST_CLASS_DEFINITION(test_0001_1) {

// 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

// 2. Click on toolbar button Toggle view for sequence se1. Click menu item Remove sequence.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "remove_sequence"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();

// Expected state: views for se1 sequence has been closed
    GTGlobals::sleep();
    QWidget *sequenceWidget1 = GTWidget::findWidget(os, "ADV_single_sequence_widget_0", NULL, false);
    CHECK_SET_ERR(sequenceWidget1 == NULL, "sequenceWidget is present");

    QWidget *overViewSe1 = GTWidget::findWidget(os, "overview_se1", NULL, false);
    QWidget *DetailsViewSe1 = GTWidget::findWidget(os, "det_view_se1", NULL, false);
    QWidget *zoomViewSe1 = GTWidget::findWidget(os, "pan_view_se1", NULL, false);
    QWidget *toolBarSe1 = GTWidget::findWidget(os, "tool_bar_se1", NULL, false);
    CHECK_SET_ERR(overViewSe1 == NULL && DetailsViewSe1 == NULL && zoomViewSe1 == NULL && toolBarSe1 == NULL, "there are widgets of ADV_single_sequence_widget");
}

GUI_TEST_CLASS_DEFINITION(test_0001_2) {

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "remove_sequence"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *sequenceWidget1 = GTWidget::findWidget(os, "ADV_single_sequence_widget_0", NULL, false);
    CHECK_SET_ERR(sequenceWidget1 == NULL, "sequenceWidget is present");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "remove_sequence"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 == NULL, "sequenceWidget is present");

    QWidget* activeMDIWindow = GTUtilsMdi::activeWindow(os, false);
    CHECK_SET_ERR(activeMDIWindow == NULL, "there is active MDI window");
}

GUI_TEST_CLASS_DEFINITION(test_0001_3) {

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "remove_sequence"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 == NULL, "sequenceWidget is present");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "remove_sequence"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *sequenceWidget1 = GTWidget::findWidget(os, "ADV_single_sequence_widget_0", NULL, false);
    CHECK_SET_ERR(sequenceWidget1 == NULL, "sequenceWidget is present");

    QWidget* activeMDIWindow = GTUtilsMdi::activeWindow(os, false);
    CHECK_SET_ERR(activeMDIWindow == NULL, "there is active MDI window");
}

GUI_TEST_CLASS_DEFINITION(test_0001_4) {

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "remove_sequence"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 == NULL, "sequenceWidget is present");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "remove_sequence"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *sequenceWidget1 = GTWidget::findWidget(os, "ADV_single_sequence_widget_0", NULL, false);
    CHECK_SET_ERR(sequenceWidget1 == NULL, "sequenceWidget is present");

    QWidget* activeMDIWindow = GTUtilsMdi::activeWindow(os, false);
    CHECK_SET_ERR(activeMDIWindow == NULL, "there is active MDI window");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "se2"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "remove_sequence"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    GTGlobals::sleep();
    sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 == NULL, "sequenceWidget is present");

    activeMDIWindow = GTUtilsMdi::activeWindow(os, false);
    CHECK_SET_ERR(activeMDIWindow == NULL, "there is active MDI window");
}



#include <QCursor>

GUI_TEST_CLASS_DEFINITION(test_0002) {

    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide all.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_all_views"));
    //GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    QWidget *w = GTWidget::findWidget(os, "toggle_view_button_se2");
    QPoint p = w->parentWidget()->mapToGlobal(w->geometry().center());
    GTMouseDriver::moveTo(os, p);

    Qt::MouseButton button = Qt::LeftButton;
    QPoint mousePos = QCursor::pos();

    CGEventType eventType = button == Qt::LeftButton ? kCGEventLeftMouseDown :
                                button == Qt::RightButton ? kCGEventRightMouseDown:
                                button == Qt::MidButton ? kCGEventOtherMouseDown : kCGEventNull;
    CGEventRef event = CGEventCreateMouseEvent(NULL, eventType, CGPointMake(mousePos.x(), mousePos.y()), 0 /*ignored*/);
    //GT_CHECK(event != NULL, "Can't create event");

    CGEventPost(kCGSessionEventTap, event);
    GTGlobals::sleep(0); // don't touch, it's Mac's magic
    CFRelease(event);


    mousePos = QCursor::pos();
    eventType = button == Qt::LeftButton ? kCGEventLeftMouseUp :
                                button == Qt::RightButton ? kCGEventRightMouseUp:
                                button == Qt::MidButton ? kCGEventOtherMouseUp : kCGEventNull;
    event = CGEventCreateMouseEvent(NULL, eventType, CGPointMake(mousePos.x(), mousePos.y()), 0 /*ignored*/);
    //GT_CHECK(event != NULL, "Can't create event");

    CGEventPost(kCGSessionEventTap, event);
    GTGlobals::sleep(0); // don't touch, it's Mac's magic
    CFRelease(event);

    GTGlobals::sleep(100);
    GTGlobals::sleep();

    //Expected state: views for se2 sequence has been closed, but toolbar still present.
    GTGlobals::sleep();
    QWidget *sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 != NULL, "sequenceWidget is not present");

    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2", NULL, false);
    QWidget *DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    QWidget *zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2", NULL, false);
    QWidget *toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2", NULL, false);
    CHECK_SET_ERR(overViewSe2->isVisible() == false && 
        DetailsViewSe2->isVisible() == false && 
        zoomViewSe2->isVisible() == false
        , "there are widgets not hidden widgets of ADV_single_sequence_widget");
    CHECK_SET_ERR(toolBarSe2->isVisible() == true, "toolbar is hidden");

    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show all.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_all_views"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: views for se2 sequence has been appeared
    GTGlobals::sleep();
    sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 != NULL, "sequenceWidget is present");

    overViewSe2 = GTWidget::findWidget(os, "overview_se2", NULL, false);
    DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2", NULL, false);
    toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2", NULL, false);
    CHECK_SET_ERR(overViewSe2->isVisible() == true && 
        DetailsViewSe2->isVisible() == true && 
        zoomViewSe2->isVisible() == true
        , "there are widgets hidden widgets of ADV_single_sequence_widget");
    CHECK_SET_ERR(toolBarSe2->isVisible() == true, "toolbar is hidden");
}

GUI_TEST_CLASS_DEFINITION(test_0002_1) {
    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide all.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_all_views"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: views for se2 sequence has been closed, but toolbar still present.
    GTGlobals::sleep();
    QWidget *sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 != NULL, "sequenceWidget is not present");

    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2", NULL, false);
    QWidget *DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    QWidget *zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2", NULL, false);
    QWidget *toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2", NULL, false);
    CHECK_SET_ERR(overViewSe2->isVisible() == false && 
        DetailsViewSe2->isVisible() == false && 
        zoomViewSe2->isVisible() == false
        , "there are widgets not hidden widgets of ADV_single_sequence_widget");
    CHECK_SET_ERR(toolBarSe2->isVisible() == true, "toolbar is hidden");

    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show all. (CHANGES: instead click 'Show all', click for show each view)
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_zoom_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_details_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_overview"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: views for se2 sequence has been appeared
    GTGlobals::sleep();
    sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 != NULL, "sequenceWidget is present");

    overViewSe2 = GTWidget::findWidget(os, "overview_se2", NULL, false);
    DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2", NULL, false);
    toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2", NULL, false);
    CHECK_SET_ERR(overViewSe2->isVisible() == true && 
        DetailsViewSe2->isVisible() == true && 
        zoomViewSe2->isVisible() == true
        , "there are widgets hidden widgets of ADV_single_sequence_widget");
    CHECK_SET_ERR(toolBarSe2->isVisible() == true, "toolbar is hidden");
}

GUI_TEST_CLASS_DEFINITION(test_0002_2) {
    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide all. (CHANGES: instead click 'Hide all', click for hide each view)
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_zoom_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_details_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_overview"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: views for se2 sequence has been closed, but toolbar still present.
    GTGlobals::sleep();
    QWidget *sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 != NULL, "sequenceWidget is not present");

    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2", NULL, false);
    QWidget *DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    QWidget *zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2", NULL, false);
    QWidget *toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2", NULL, false);
    CHECK_SET_ERR(overViewSe2->isVisible() == false && 
        DetailsViewSe2->isVisible() == false && 
        zoomViewSe2->isVisible() == false
        , "there are widgets not hidden widgets of ADV_single_sequence_widget");
    CHECK_SET_ERR(toolBarSe2->isVisible() == true, "toolbar is hidden");

    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show all.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_all_views"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: views for se2 sequence has been appeared
    GTGlobals::sleep();
    sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 != NULL, "sequenceWidget is present");

    overViewSe2 = GTWidget::findWidget(os, "overview_se2", NULL, false);
    DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2", NULL, false);
    toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2", NULL, false);
    CHECK_SET_ERR(overViewSe2->isVisible() == true && 
        DetailsViewSe2->isVisible() == true && 
        zoomViewSe2->isVisible() == true
        , "there are widgets hidden widgets of ADV_single_sequence_widget");
    CHECK_SET_ERR(toolBarSe2->isVisible() == true, "toolbar is hidden");
}

GUI_TEST_CLASS_DEFINITION(test_0002_3) {
    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide all. (CHANGES: hiding zoom view before pressing 'Hide all')
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_zoom_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_all_views"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: views for se2 sequence has been closed, but toolbar still present.
    GTGlobals::sleep();
    QWidget *sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 != NULL, "sequenceWidget is not present");

    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2", NULL, false);
    QWidget *DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    QWidget *zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2", NULL, false);
    QWidget *toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2", NULL, false);
    CHECK_SET_ERR(overViewSe2->isVisible() == false && 
        DetailsViewSe2->isVisible() == false && 
        zoomViewSe2->isVisible() == false
        , "there are widgets not hidden widgets of ADV_single_sequence_widget");
    CHECK_SET_ERR(toolBarSe2->isVisible() == true, "toolbar is hidden");

    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show all.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_all_views"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: views for se2 sequence has been appeared
    GTGlobals::sleep();
    sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 != NULL, "sequenceWidget is present");

    overViewSe2 = GTWidget::findWidget(os, "overview_se2", NULL, false);
    DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2", NULL, false);
    toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2", NULL, false);
    CHECK_SET_ERR(overViewSe2->isVisible() == true && 
        DetailsViewSe2->isVisible() == true && 
        zoomViewSe2->isVisible() == true
        , "there are widgets hidden widgets of ADV_single_sequence_widget");
    CHECK_SET_ERR(toolBarSe2->isVisible() == true, "toolbar is hidden");
}

GUI_TEST_CLASS_DEFINITION(test_0002_4) {
    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide all. CHANGES: using 'Toggle views' insetead 'Toggle view'
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleAllSequenceViews"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    //Expected state: views for se2 sequence has been closed, but toolbar still present.
    GTGlobals::sleep();
    QWidget *sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 != NULL, "sequenceWidget is not present");

    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2", NULL, false);
    QWidget *DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    QWidget *zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2", NULL, false);
    QWidget *toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2", NULL, false);
    CHECK_SET_ERR(overViewSe2->isVisible() == false && 
        DetailsViewSe2->isVisible() == false && 
        zoomViewSe2->isVisible() == false
        , "there are widgets not hidden widgets of ADV_single_sequence_widget");
    CHECK_SET_ERR(toolBarSe2->isVisible() == true, "toolbar is hidden");

    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show all. CHANGES: using 'Toggle views' insetead 'Toggle view'
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleAllSequenceViews"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    //Expected state: views for se2 sequence has been appeared
    GTGlobals::sleep();
    sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 != NULL, "sequenceWidget is present");

    overViewSe2 = GTWidget::findWidget(os, "overview_se2", NULL, false);
    DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2", NULL, false);
    toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2", NULL, false);
    CHECK_SET_ERR(overViewSe2->isVisible() == true && 
        DetailsViewSe2->isVisible() == true && 
        zoomViewSe2->isVisible() == true
        , "there are widgets hidden widgets of ADV_single_sequence_widget");
    CHECK_SET_ERR(toolBarSe2->isVisible() == true, "toolbar is hidden");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    //1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide details.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_details_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: detailed view for se2 sequence has been closed.
    QWidget* DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    CHECK_SET_ERR(DetailsViewSe2->isVisible() == false, "details view isn't hidden");

    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show details.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_details_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: detailed view for se2 sequence has been appeared
    DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    CHECK_SET_ERR(DetailsViewSe2->isVisible() == true, "details view is hidden");
}

GUI_TEST_CLASS_DEFINITION(test_0003_1) {
    //1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide details. CHANGES: using 'Toggle views' instead 'Toggle view'
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleDetailsView"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    //Expected state: detailed view for se2 sequence has been closed.
    QWidget* DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    CHECK_SET_ERR(DetailsViewSe2->isVisible() == false, "details view isn't hidden");

    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show details.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_details_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: detailed view for se2 sequence has been appeared
    DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    CHECK_SET_ERR(DetailsViewSe2->isVisible() == true, "details view is hidden");
}

GUI_TEST_CLASS_DEFINITION(test_0003_2) {
    //1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide details.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_details_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: detailed view for se2 sequence has been closed.
    QWidget* DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    CHECK_SET_ERR(DetailsViewSe2->isVisible() == false, "details view isn't hidden");

    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show details. CHANGES: using 'Toggle views' instead 'Toggle view', clicking twice to avoid missunderstanding
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleDetailsView"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();
    
    //checking hiding all detviews
    DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    QWidget *DetailsViewSe1 = GTWidget::findWidget(os, "det_view_se1", NULL, false);
    CHECK_SET_ERR(DetailsViewSe2->isVisible() == false &&
        DetailsViewSe1->isVisible() == false, "details view isn't hidden");


    //clicking 2nd time
    //toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleDetailsView"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    //Expected state: detailed view for se2 sequence has been appeared
    DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    CHECK_SET_ERR(DetailsViewSe2->isVisible() == true, "details view is hidden");
}

GUI_TEST_CLASS_DEFINITION(test_0003_3) {
    //1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide details. CHANGES: hiding all views, instead hidding only deatails
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_all_views"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: detailed view for se2 sequence has been closed.
    QWidget* DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    CHECK_SET_ERR(DetailsViewSe2->isVisible() == false, "details view isn't hidden");

    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show details.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_details_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: detailed view for se2 sequence has been appeared
    DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    CHECK_SET_ERR(DetailsViewSe2->isVisible() == true, "details view is hidden");
}

GUI_TEST_CLASS_DEFINITION(test_0003_4) {
    //1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide details. CHANGES: hiding all views for all sequences, instead hidding only deatails
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleAllSequenceViews"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    //Expected state: detailed view for se2 sequence has been closed.
    QWidget* DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    CHECK_SET_ERR(DetailsViewSe2->isVisible() == false, "details view isn't hidden");

    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show details.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_details_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: detailed view for se2 sequence has been appeared
    DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2", NULL, false);
    CHECK_SET_ERR(DetailsViewSe2->isVisible() == true, "details view is hidden");
}

GUI_TEST_CLASS_DEFINITION(test_0004) {
    //1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide overview.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_overview"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: over view for se2 sequence has been closed, all zoom buttons at this toolbar becomes disabled
    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2");
    CHECK_SET_ERR(overViewSe2->isVisible() == false, "overview is visible")


    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show overview.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_overview"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: over view for se2 sequence has been appeared
    CHECK_SET_ERR(overViewSe2->isVisible() == true, "overview is hidden")
}

GUI_TEST_CLASS_DEFINITION(test_0004_1) {
    //1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide overview. CHANGES: using 'Toggle views' instead 'Toggle view'
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleOverview"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    //Expected state: over view for se2 sequence has been closed, all zoom buttons at this toolbar becomes disabled
    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2");
    CHECK_SET_ERR(overViewSe2->isVisible() == false, "overview is visible");


    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show overview.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_overview"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: over view for se2 sequence has been appeared
    CHECK_SET_ERR(overViewSe2->isVisible() == true, "overview is hidden")
}

GUI_TEST_CLASS_DEFINITION(test_0004_2) {
    //1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide overview.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_overview"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: over view for se2 sequence has been closed, all zoom buttons at this toolbar becomes disabled
    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2");
    CHECK_SET_ERR(overViewSe2->isVisible() == false, "overview is visible");


    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show overview. CHANGES: using 'Toggle views' instead 'Toggle view'
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleOverview"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleOverview"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    //Expected state: over view for se2 sequence has been appeared
    CHECK_SET_ERR(overViewSe2->isVisible() == true, "overview is hidden")
}

GUI_TEST_CLASS_DEFINITION(test_0004_3) {
    //1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide overview. CHANGES: using sequence se1 instead se2
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_overview"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();

    //Expected state: over view for se2 sequence has been closed, all zoom buttons at this toolbar becomes disabled
    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se1");
    CHECK_SET_ERR(overViewSe2->isVisible() == false, "overview is visible");


    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show overview.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_overview"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();

    //Expected state: over view for se2 sequence has been appeared
    CHECK_SET_ERR(overViewSe2->isVisible() == true, "overview is hidden")
}

GUI_TEST_CLASS_DEFINITION(test_0004_4) {
    //1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //2. Click on toolbar button Toggle view for sequence se2. Click menu item Hide overview. CHANGES: using 'hide all' instead hide overview
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_all_views"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: over view for se2 sequence has been closed, all zoom buttons at this toolbar becomes disabled
    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2");
    CHECK_SET_ERR(overViewSe2->isVisible() == false, "overview is visible")

    //3. Click on toolbar button Toggle view for sequence se2. Click menu item Show overview.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_overview"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    //Expected state: over view for se2 sequence has been appeared
    CHECK_SET_ERR(overViewSe2->isVisible() == true, "overview is hidden")
}

GUI_TEST_CLASS_DEFINITION(test_0005) {

// 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa, open file as separate sequences
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *overViewSe1 = GTWidget::findWidget(os, "overview_se1");
    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2");
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");

// 2. Click on toolbar button Toggle views. Click menu item Hide all overviews.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleOverview"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

// Expected state: panoramical views for both sequences has been closed, all zoom buttons at sequence views becomes disabled
    GTGlobals::sleep();
    CHECK_SET_ERR(overViewSe1->isVisible() == false && overViewSe2->isVisible() == false,
        "panoramical views for both sequences has not been closed");

// 3. Click on toolbar button Toggle views. Click menu item Show all overviews.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleOverview"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    //QWidget *_overViewSe1 = GTWidget::findWidget(os, "overview_se1");
    //QWidget *_overViewSe2 = GTWidget::findWidget(os, "overview_se2");

// Expected state: panoramical view for both sequences has been appeared
    GTGlobals::sleep();
    CHECK_SET_ERR(overViewSe1->isVisible() == true && overViewSe2->isVisible() == true, 
        "panoramical view for both sequences has not been shown");
}

GUI_TEST_CLASS_DEFINITION(test_0006) {
// 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *detailsViewSe1 = GTWidget::findWidget(os, "det_view_se1");
    QWidget *detailsViewSe2 = GTWidget::findWidget(os, "det_view_se2");
    QWidget *toolBarSe1 = GTWidget::findWidget(os, "tool_bar_se1");
    QWidget *toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2");
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");

// 2. Click on toolbar button Toggle views. Click menu item Hide all details.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleDetailsView"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

// Expected state: views for both sequences has been closed, but toolbars still present.
    GTGlobals::sleep();
    CHECK_SET_ERR(detailsViewSe1->isVisible() == false && detailsViewSe2->isVisible() == false, 
        "panoramical view for both sequences has not been shown");
    CHECK_SET_ERR(toolBarSe1->isVisible() == true && toolBarSe2->isVisible() == true, 
        "panoramical view for both sequences has not been shown");

// 3. Click on toolbar button Toggle views. Click menu item Show all details.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleDetailsView"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

// Expected state: view for both sequences has been appeared
    GTGlobals::sleep();
    CHECK_SET_ERR(detailsViewSe1->isVisible() == true && detailsViewSe2->isVisible() == true, 
        "panoramical view for both sequences has not been shown");
}

GUI_TEST_CLASS_DEFINITION(test_0007) {

// 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *overViewSe1 = GTWidget::findWidget(os, "overview_se1");
    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2");
    QWidget *DetailsViewSe1 = GTWidget::findWidget(os, "det_view_se1");
    QWidget *DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2");
    QWidget *zoomViewSe1 = GTWidget::findWidget(os, "pan_view_se1");
    QWidget *zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2");
    QWidget *toolBarSe1 = GTWidget::findWidget(os, "tool_bar_se1");
    QWidget *toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2");
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");

// 2. Click on toolbar button Toggle views. Click menu item Hide all sequences.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleAllSequenceViews"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

// Expected state: all views for both sequences has been closed, but toolbars still present.
    GTGlobals::sleep();
    CHECK_SET_ERR(overViewSe1->isHidden() == true && overViewSe2->isHidden() == true,
        "panoramical views for both sequences has not been closed");
    CHECK_SET_ERR(DetailsViewSe1->isHidden() == true && DetailsViewSe2->isHidden() == true,
        "details views for both sequences has not been closed");
    CHECK_SET_ERR(zoomViewSe1->isHidden() == true && zoomViewSe2->isHidden() == true,
        "zoom views for both sequences has not been closed");
    CHECK_SET_ERR(toolBarSe1->isHidden() == false && toolBarSe2->isHidden() == false, 
        "toolbars view for both sequences has not been shown");

// 3. Click on toolbar button Toggle views. Click menu item Show all sequences.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleAllSequenceViews"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

// Expected state: all views for both sequences has been appeared
    GTGlobals::sleep();
    CHECK_SET_ERR(overViewSe1->isHidden() == false && overViewSe2->isHidden() == false, 
        "panoramical view for both sequences has not been shown");
    CHECK_SET_ERR(DetailsViewSe1->isHidden() == false && DetailsViewSe2->isHidden() == false, 
        "details view for both sequences has not been shown");
    CHECK_SET_ERR(zoomViewSe1->isHidden() == false && zoomViewSe2->isHidden() == false, 
        "zoom view for both sequences has not been shown");
}

GUI_TEST_CLASS_DEFINITION(test_0008) {
// 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    QToolBar *toolBarSe2 = mainWindow->findChild<QToolBar*>("tool_bar_se2");
    CHECK_SET_ERR(mainWindow != NULL, "Main Window not found");
    CHECK_SET_ERR(toolBarSe2 != NULL, "Tool bar not found");

    QAbstractButton* zoomButton = GTAction::button(os, "zoom_to_range_se2");
    CHECK_SET_ERR(zoomButton != NULL, "Zoom button not found");
    QAbstractButton* zoomInButton = GTAction::button(os, "action_zoom_in_se2");
    CHECK_SET_ERR(zoomInButton != NULL, "Zoom In button not found");
    QAbstractButton* zoomOutButton = GTAction::button(os, "action_zoom_out_se2");
    CHECK_SET_ERR(zoomOutButton != NULL, "Zoom Out button not found");

// 2. Click on toolbar button Zoom to range for sequence se2. Select region 20..50, then click OK.
    GTUtilsDialog::waitForDialog(os, new ZoomToRangeDialogFiller(os, 20, 50));
    GTWidget::click(os, zoomButton);
    GTGlobals::sleep();

// Expected state: sequence overview show 20..50 sequence region, all zoom buttons at this toolbar becomes enabled
    GTGlobals::sleep();
    //  TODO: Expected state: sequence overview showh 20..50 sequence region
    CHECK_SET_ERR(zoomInButton->isEnabled(), "Zoom In button is not enabled");
    CHECK_SET_ERR(zoomOutButton->isEnabled(), "Zoom Out button is not enabled");

// 3. Click on toolbar button Toggle view for sequence se2. Click menu item Hide zoom view.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_zoom_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

// Expected state: panoramical view for se2 sequence has been closed, all zoom buttons at this toolbar becomes disabled
    GTGlobals::sleep();
    QWidget *zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2");
    CHECK_SET_ERR(zoomViewSe2 != NULL, "Zoom View widget not found");
    CHECK_SET_ERR(zoomViewSe2->isHidden(), "panoramical view for se2 sequence has been not closed");
    CHECK_SET_ERR(!zoomInButton->isEnabled(), "Zoom In button is enabled");
    CHECK_SET_ERR(!zoomOutButton->isEnabled(), "Zoom Out button is enabled");

// 4. Click on toolbar button Toggle view for sequence se2. Click menu item Show zoom view.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_zoom_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

// Expected state: panoramical view for se2 sequence has been appeared, all zoom buttons at this toolbar becomes enabled
    GTGlobals::sleep();
    CHECK_SET_ERR(!zoomViewSe2->isHidden(), "panoramical view for se2 sequence has been not appeared");
    CHECK_SET_ERR(zoomInButton->isEnabled(), "Zoom In button is not enabled");
    CHECK_SET_ERR(zoomOutButton->isEnabled(), "Zoom Out button is not enabled");
}

GUI_TEST_CLASS_DEFINITION(test_0009)
{
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep(1000);


    QWidget *overViewSe1 = GTWidget::findWidget(os, "overview_se1");
    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2");

    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    QWidget *toggleViewButtonSe2 = GTWidget::findWidget(os, "toggle_view_button_se2");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_overview"));
    GTWidget::click(os, toggleViewButtonSe2);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(overViewSe2->isHidden(), "panoramical view for se2 sequence has been not closed");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleOverview"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(overViewSe1->isHidden() && overViewSe2->isHidden(),
                  "panoramical views for both sequences has been not closed");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_overview"));
    GTWidget::click(os, toggleViewButtonSe2);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(!overViewSe2->isHidden(), "panoramical view for se2 sequence has been not shown");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleOverview"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(10000);
    CHECK_SET_ERR(overViewSe1->isHidden() && overViewSe2->isHidden(),
                  "panoramical view for both sequences has been not closed");
}

GUI_TEST_CLASS_DEFINITION(test_0011)
{
// 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    QToolBar *toolBarSe1 = mainWindow->findChild<QToolBar*>("tool_bar_se1");
    CHECK_SET_ERR(mainWindow != NULL, "Main Window not found");
    CHECK_SET_ERR(toolBarSe1 != NULL, "Tool bar not found");


    QAbstractButton* zoomButton = GTAction::button(os,"zoom_to_range_se1");
    CHECK_SET_ERR(zoomButton != NULL, "Zoom button not found");

    QAbstractButton* zoomInButton = GTAction::button(os,"action_zoom_in_se1");
    QAbstractButton* zoomOutButton = GTAction::button(os,"action_zoom_out_se1");

    CHECK_SET_ERR(zoomInButton != NULL, "Zoom In button not found");
    CHECK_SET_ERR(zoomOutButton != NULL, "Zoom Out button not found");

// 2. Press 'Zoom in' button for seq1.
    GTWidget::click(os, zoomInButton);
    GTGlobals::sleep();
    QWidget *zoomViewSe1 = GTWidget::findWidget(os, "pan_view_se1");
    CHECK_SET_ERR(!zoomViewSe1->isHidden(), "panoramical view for se1 sequence has been not appeared");

// 3. Close zoom view for seq1
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_zoom_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();

// Expected state: Zoom area (grey rectangle) has disappered from overview for seq1
    GTGlobals::sleep();
    CHECK_SET_ERR(zoomViewSe1 != NULL, "Zoom View widget not found");
    CHECK_SET_ERR(zoomViewSe1->isHidden(), "panoramical view for se1 sequence has been not closed");
}

GUI_TEST_CLASS_DEFINITION(test_0011_1)
{
// 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    QToolBar *toolBarSe1 = mainWindow->findChild<QToolBar*>("tool_bar_se1");
    QToolBar *toolBarSe2 = mainWindow->findChild<QToolBar*>("tool_bar_se2");
    CHECK_SET_ERR(mainWindow != NULL, "Main Window not found");
    CHECK_SET_ERR(toolBarSe1 != NULL, "Tool bar not found");
    CHECK_SET_ERR(toolBarSe2 != NULL, "Tool bar not found");


    QAbstractButton* zoomButton1 = GTAction::button(os,"action_zoom_in_se1");
    CHECK_SET_ERR(zoomButton1 != NULL, "Zoom button not found");

    QAbstractButton* zoomButton2 = GTAction::button(os,"action_zoom_in_se2");
    CHECK_SET_ERR(zoomButton2 != NULL, "Zoom button not found");

    QAbstractButton* zoomInButton1 = GTAction::button(os,"action_zoom_in_se1");
    QAbstractButton* zoomOutButton1 = GTAction::button(os,"action_zoom_out_se1");

    QAbstractButton* zoomInButton2 = GTAction::button(os,"action_zoom_in_se2");
    QAbstractButton* zoomOutButton2 = GTAction::button(os,"action_zoom_out_se2");

    CHECK_SET_ERR(zoomInButton1 != NULL, "Zoom In button not found");
    CHECK_SET_ERR(zoomOutButton1 != NULL, "Zoom Out button not found");

    CHECK_SET_ERR(zoomInButton2 != NULL, "Zoom In button not found");
    CHECK_SET_ERR(zoomOutButton2 != NULL, "Zoom Out button not found");

// 2. Press 'Zoom in' button for both. CHANGES: for both instead of seq1
    GTWidget::click(os, zoomInButton1);
    GTGlobals::sleep();
    QWidget *zoomViewSe1 = GTWidget::findWidget(os, "pan_view_se1");
    CHECK_SET_ERR(!zoomViewSe1->isHidden(), "panoramical view for se1 sequence has been not appeared");

    GTWidget::click(os, zoomInButton2);
    GTGlobals::sleep();
    QWidget *zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2");
    CHECK_SET_ERR(!zoomViewSe2->isHidden(), "panoramical view for se1 sequence has been not appeared");

// 3. Close zoom view for both. CHANGES: for both instead of seq1
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_zoom_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "show_hide_zoom_view"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

// Expected state: Zoom area (grey rectangle) has disappered from overview for both. CHANGES: for both instead of seq1
    GTGlobals::sleep();
    CHECK_SET_ERR(zoomViewSe1 != NULL, "Zoom View widget not found");
    CHECK_SET_ERR(zoomViewSe1->isHidden(), "panoramical view for se1 sequence has been not closed");

    GTGlobals::sleep();
    CHECK_SET_ERR(zoomViewSe2 != NULL, "Zoom View widget not found");
    CHECK_SET_ERR(zoomViewSe2->isHidden(), "panoramical view for se2 sequence has been not closed");

}

GUI_TEST_CLASS_DEFINITION(test_0011_2)
{
    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    QToolBar *toolBarSe1 = mainWindow->findChild<QToolBar*>("tool_bar_se1");
    QToolBar *toolBarSe2 = mainWindow->findChild<QToolBar*>("tool_bar_se2");
    CHECK_SET_ERR(mainWindow != NULL, "Main Window not found");
    CHECK_SET_ERR(toolBarSe1 != NULL, "Tool bar not found");
    CHECK_SET_ERR(toolBarSe2 != NULL, "Tool bar not found");

    QAbstractButton* zoomButton1 = GTAction::button(os,"zoom_to_range_se1");
    CHECK_SET_ERR(zoomButton1 != NULL, "Zoom button not found");
    QAbstractButton* zoomButton2 = GTAction::button(os,"zoom_to_range_se2");
    CHECK_SET_ERR(zoomButton2 != NULL, "Zoom button not found");

    QAbstractButton* zoomInButton1 = GTAction::button(os,"action_zoom_in_se1");
    QAbstractButton* zoomOutButton1 = GTAction::button(os,"action_zoom_out_se1");

    QAbstractButton* zoomInButton2 = GTAction::button(os,"action_zoom_in_se2");
    QAbstractButton* zoomOutButton2 = GTAction::button(os,"action_zoom_out_se2");

    CHECK_SET_ERR(zoomInButton1 != NULL, "Zoom In button not found");
    CHECK_SET_ERR(zoomOutButton1 != NULL, "Zoom Out button not found");

    CHECK_SET_ERR(zoomInButton2 != NULL, "Zoom In button not found");
    CHECK_SET_ERR(zoomOutButton2 != NULL, "Zoom Out button not found");


    // 2. Press 'Zoom in' button for both. CHANGES: for both instead of seq1
    GTWidget::click(os, zoomInButton1);
    GTGlobals::sleep();
    QWidget *zoomViewSe1 = GTWidget::findWidget(os, "pan_view_se1");
    CHECK_SET_ERR(!zoomViewSe1->isHidden(), "panoramical view for se1 sequence has been not appeared");

    GTWidget::click(os, zoomInButton2);
    GTGlobals::sleep();
    QWidget *zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2");
    CHECK_SET_ERR(!zoomViewSe2->isHidden(), "panoramical view for se1 sequence has been not appeared");

    // 3. Close zoom views by global Toogle View Button. CHANGES: for both instead of seq1 + global Toggle View Button instead of the seq1 and buttons

    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "toggleZoomView"));
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    // Expected state: Zoom area (grey rectangle) has disappered from overview for both. 
    GTGlobals::sleep();
    CHECK_SET_ERR(zoomViewSe1 != NULL, "Zoom View widget not found");
    CHECK_SET_ERR(zoomViewSe1->isHidden(), "panoramical view for se1 sequence has been not closed");

    GTGlobals::sleep();
    CHECK_SET_ERR(zoomViewSe2 != NULL, "Zoom View widget not found");
    CHECK_SET_ERR(zoomViewSe2->isHidden(), "panoramical view for se2 sequence has been not closed");
}

GUI_TEST_CLASS_DEFINITION(test_0012)
    {
// 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    Runnable *dialog = new SequenceReadingModeSelectorDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, dialog);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

// 2. Close detailed view
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    Runnable *chooser = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    QWidget *detailsViewSe1 = GTWidget::findWidget(os, "det_view_se1");
    QWidget *detailsViewSe2 = GTWidget::findWidget(os, "det_view_se2");

    GTGlobals::sleep();
    CHECK_SET_ERR(detailsViewSe1->isHidden() && detailsViewSe1->isHidden(),
        "details views for both sequences has not been closed");

// TODO: Expected state: position indicator(Yellow triangle) on overview has disappered from overview
    }

GUI_TEST_CLASS_DEFINITION(test_0012_1)
    {
    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    // 2. Close detailed view CHANGES: for 1 and 2 instead of global button

    Runnable *chooser1 = new PopupChooser(os, QStringList() << "show_hide_details_view");
    GTUtilsDialog::waitForDialog(os, chooser1);
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();

    Runnable *chooser2 = new PopupChooser(os, QStringList() << "show_hide_details_view");
    GTUtilsDialog::waitForDialog(os, chooser2);
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    QWidget *detailsViewSe1 = GTWidget::findWidget(os, "det_view_se1");
    QWidget *detailsViewSe2 = GTWidget::findWidget(os, "det_view_se2");

    GTGlobals::sleep();
    GTGlobals::sleep();
    CHECK_SET_ERR(detailsViewSe1->isHidden() && detailsViewSe1->isHidden(),
        "details views for both sequences has not been closed");

    // TODO: Expected state: position indicator(Yellow triangle) on overview has disappered from overview
    }

GUI_TEST_CLASS_DEFINITION(test_0012_2)
    {
    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    Runnable *dialog = new SequenceReadingModeSelectorDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, dialog);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    // 2. Close detailed view
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    Runnable *chooser0 = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser0);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    QWidget *detailsViewSe1 = GTWidget::findWidget(os, "det_view_se1");
    QWidget *detailsViewSe2 = GTWidget::findWidget(os, "det_view_se2");

    GTGlobals::sleep();
    CHECK_SET_ERR(detailsViewSe1->isHidden() && detailsViewSe1->isHidden(),
        "details views for both sequences has not been closed");

    Runnable *chooser1 = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser1);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(!detailsViewSe1->isHidden() && !detailsViewSe1->isHidden(),
        "details views for both sequences has not been show");

    Runnable *chooser2 = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser2);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(detailsViewSe1->isHidden() && detailsViewSe1->isHidden(),
        "details views for both sequences has not been closed");


    // TODO: Expected state: position indicator(Yellow triangle) on overview has disappered from overview
    }

GUI_TEST_CLASS_DEFINITION(test_0013)
    {
// 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    Runnable *dialog = new SequenceReadingModeSelectorDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, dialog);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

// 2. Close detailed view
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    Runnable *chooser = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    QWidget *detailsViewSe1 = GTWidget::findWidget(os, "det_view_se1");
    QWidget *detailsViewSe2 = GTWidget::findWidget(os, "det_view_se2");

    GTGlobals::sleep();
    CHECK_SET_ERR(detailsViewSe1->isHidden() && detailsViewSe1->isHidden(),
        "details views for both sequences has not been closed");

// TODO: Expected state: position indicator (Grey dotted line rectangle)  has disappered from zoom view

    }
GUI_TEST_CLASS_DEFINITION(test_0013_1)
    {
    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    Runnable *dialog = new SequenceReadingModeSelectorDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, dialog);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    // 2. Close detailed view CHANGES: for 1 and 2 instead of global button

    Runnable *chooser1 = new PopupChooser(os, QStringList() << "show_hide_details_view");
    GTUtilsDialog::waitForDialog(os, chooser1);
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();

    Runnable *chooser2 = new PopupChooser(os, QStringList() << "show_hide_details_view");
    GTUtilsDialog::waitForDialog(os, chooser2);
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();

    QWidget *detailsViewSe1 = GTWidget::findWidget(os, "det_view_se1");
    QWidget *detailsViewSe2 = GTWidget::findWidget(os, "det_view_se2");

    GTGlobals::sleep();
    CHECK_SET_ERR(detailsViewSe1->isHidden() && detailsViewSe1->isHidden(),
        "details views for both sequences has not been closed");

    // TODO: Expected state: position indicator (Grey dotted line rectangle)  has disappered from zoom view

    }

GUI_TEST_CLASS_DEFINITION(test_0013_2)
    {
    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    Runnable *dialog = new SequenceReadingModeSelectorDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, dialog);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    // 2. Close detailed view
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    Runnable *chooser0 = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser0);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    QWidget *detailsViewSe1 = GTWidget::findWidget(os, "det_view_se1");
    QWidget *detailsViewSe2 = GTWidget::findWidget(os, "det_view_se2");

    GTGlobals::sleep();
    CHECK_SET_ERR(detailsViewSe1->isHidden() && detailsViewSe1->isHidden(),
        "details views for both sequences has not been closed");

    Runnable *chooser1 = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser1);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(!detailsViewSe1->isHidden() && !detailsViewSe1->isHidden(),
        "details views for both sequences has not been show");

    Runnable *chooser2 = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser2);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(detailsViewSe1->isHidden() && detailsViewSe1->isHidden(),
        "details views for both sequences has not been closed");

    // TODO: Expected state: position indicator (Grey dotted line rectangle)  has disappered from zoom view

    }
GUI_TEST_CLASS_DEFINITION(test_0014)
    {
    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    Runnable *dialog = new SequenceReadingModeSelectorDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, dialog);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

//  2. Open graph view {Graphs->CG% content}
    GTGlobals::sleep();
    QWidget *sequenceWidget1 = GTWidget::findWidget(os, "ADV_single_sequence_widget_0", NULL, false);
    CHECK_SET_ERR(sequenceWidget1 != NULL, "sequenceWidget is not present");
    QWidget *circularViewSe1 = GTWidget::findWidget(os, "GraphMenuAction", sequenceWidget1, false);
    Runnable *chooser = new PopupChooser(os, QStringList() << "GC Content (%)");
    GTUtilsDialog::waitForDialog(os, chooser);
    GTWidget::click(os, circularViewSe1);
    GTGlobals::sleep();

//  3. Close detailed view
    Runnable *chooser1 = new PopupChooser(os, QStringList() << "show_hide_details_view");
    GTUtilsDialog::waitForDialog(os, chooser1);
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();
//  TODO: Expected state: position indicator (Grey dotted line rectangle)  has disappered from graph view

    }
GUI_TEST_CLASS_DEFINITION(test_0014_1)
    {
    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    Runnable *dialog = new SequenceReadingModeSelectorDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, dialog);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //  2. Open graph view {Graphs->CG% content}. Changes: for both sequences
    GTGlobals::sleep();
    QWidget *sequenceWidget1 = GTWidget::findWidget(os, "ADV_single_sequence_widget_0", NULL, false);
    CHECK_SET_ERR(sequenceWidget1 != NULL, "sequenceWidget is not present");
    QWidget *circularViewSe1 = GTWidget::findWidget(os, "GraphMenuAction", sequenceWidget1, false);
    Runnable *chooser1 = new PopupChooser(os, QStringList() << "GC Content (%)");
    GTUtilsDialog::waitForDialog(os, chooser1);
    GTWidget::click(os, circularViewSe1);
    GTGlobals::sleep();

    GTGlobals::sleep();
    QWidget *sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 != NULL, "sequenceWidget is not present");
    QWidget *circularViewSe2 = GTWidget::findWidget(os, "GraphMenuAction", sequenceWidget2, false);
    Runnable *chooser2 = new PopupChooser(os, QStringList() << "GC Content (%)");
    GTUtilsDialog::waitForDialog(os, chooser2);
    GTWidget::click(os, circularViewSe2);
    GTGlobals::sleep();

    //  3. Close detailed view. Changes: for both sequences
    Runnable *chooser3 = new PopupChooser(os, QStringList() << "show_hide_details_view");
    GTUtilsDialog::waitForDialog(os, chooser3);
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();

    Runnable *chooser4 = new PopupChooser(os, QStringList() << "show_hide_details_view");
    GTUtilsDialog::waitForDialog(os, chooser4);
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();
    //  TODO: Expected state: position indicator (Grey dotted line rectangle)  has disappered from graph view for both sequences

    }
GUI_TEST_CLASS_DEFINITION(test_0014_2)
    {
    // 1. Use menu {File->Open}. Open file _common_data/scenarios/project/multiple.fa
    Runnable *dialog = new SequenceReadingModeSelectorDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, dialog);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep();

    //  2. Open graph view {Graphs->CG% content}. Changes: for 1 sequences
    GTGlobals::sleep();
    QWidget *sequenceWidget1 = GTWidget::findWidget(os, "ADV_single_sequence_widget_0", NULL, false);
    CHECK_SET_ERR(sequenceWidget1 != NULL, "sequenceWidget is not present");
    QWidget *circularViewSe1 = GTWidget::findWidget(os, "GraphMenuAction", sequenceWidget1, false);
    Runnable *chooser1 = new PopupChooser(os, QStringList() << "GC Content (%)");
    GTUtilsDialog::waitForDialog(os, chooser1);
    GTWidget::click(os, circularViewSe1);
    GTGlobals::sleep();

    //  3. Close detailed view. Changes: for 1 sequences
    Runnable *chooser3 = new PopupChooser(os, QStringList() << "show_hide_details_view");
    GTUtilsDialog::waitForDialog(os, chooser3);
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se1"));
    GTGlobals::sleep();

    //  2. Open graph view {Graphs->CG% content}. Changes: for 2 sequences
    GTGlobals::sleep();
    QWidget *sequenceWidget2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1", NULL, false);
    CHECK_SET_ERR(sequenceWidget2 != NULL, "sequenceWidget is not present");
    QWidget *circularViewSe2 = GTWidget::findWidget(os, "GraphMenuAction", sequenceWidget2, false);
    Runnable *chooser2 = new PopupChooser(os, QStringList() << "GC Content (%)");
    GTUtilsDialog::waitForDialog(os, chooser2);
    GTWidget::click(os, circularViewSe2);
    GTGlobals::sleep();

    //  3. Close detailed view. Changes: for 2 sequences
    Runnable *chooser4 = new PopupChooser(os, QStringList() << "show_hide_details_view");
    GTUtilsDialog::waitForDialog(os, chooser4);
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_se2"));
    GTGlobals::sleep();
    //  TODO: Expected state: position indicator (Grey dotted line rectangle)  has disappered from graph view for both sequences
    }
} // namespace
} // namespace U2
