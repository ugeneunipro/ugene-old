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

#include "GTTestsMsaEditor.h"
#include "api/GTAction.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMenu.h"
#include "api/GTFile.h"
#include "api/GTFileDialog.h"
#include "api/GTClipboard.h"
#include "api/GTToolbar.h"
#include "api/GTLineEdit.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsLog.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/corelibs/U2Gui/PositionSelectorFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportMSA2MSADialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/kalign/KalignDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/util/RenameSequenceFiller.h"

#include <U2View/MSAEditor.h>

namespace U2 {

namespace GUITest_common_scenarios_msa_editor {

GUI_TEST_CLASS_DEFINITION(test_0001) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTGlobals::sleep();

    int length = GTUtilsMSAEditorSequenceArea::getLength(os);
    CHECK_SET_ERR(length == 14, "Wrong length");

    int leftOffest = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(leftOffest == 1, "Wrong left offset");

    int rightOffest = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    CHECK_SET_ERR(rightOffest == 14, "Wrong right offset");
}

GUI_TEST_CLASS_DEFINITION(test_0001_1) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma.aln");
    GTGlobals::sleep();

    int length = GTUtilsMSAEditorSequenceArea::getLength(os);
    CHECK_SET_ERR(length == 12, "Wrong length");

    int leftOffest = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(leftOffest == 1, "Wrong left offset");

    int rightOffest = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    CHECK_SET_ERR(rightOffest == 12, "Wrong right offset");
}

GUI_TEST_CLASS_DEFINITION(test_0001_2) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gap_col.aln");
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Maximize);
    GTGlobals::sleep();

    int length = GTUtilsMSAEditorSequenceArea::getLength(os);
    CHECK_SET_ERR(length == 14, "Wrong length");

    int leftOffest = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(leftOffest == 1, "Wrong left offset");

    int rightOffest = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    CHECK_SET_ERR(rightOffest == 14, "Wrong right offset");
}

GUI_TEST_CLASS_DEFINITION(test_0001_3) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "revcompl.aln");
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Minimize);
    GTGlobals::sleep();

    int length = GTUtilsMSAEditorSequenceArea::getLength(os);
    CHECK_SET_ERR(length == 6, "Wrong length");

    int leftOffest = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(leftOffest == 1, "Wrong left offset");

    int rightOffest = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    CHECK_SET_ERR(rightOffest == 6, "Wrong right offset");
}

GUI_TEST_CLASS_DEFINITION(test_0001_4) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "translations_nucl.aln", "*.*", GTFileDialog::Cancel);
    GTGlobals::sleep();
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "translations_nucl.aln", "*.*", GTFileDialog::Open);
    GTGlobals::sleep();

    int length = GTUtilsMSAEditorSequenceArea::getLength(os);
    CHECK_SET_ERR(length == 3, "Wrong length");

    int leftOffest = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(leftOffest == 1, "Wrong left offset");

    int rightOffest = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    CHECK_SET_ERR(rightOffest == 3, "Wrong right offset");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTGlobals::sleep(1000);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    bool offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == false, "Offsets are visible");


    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == true, "Offsets are not visible");
}

GUI_TEST_CLASS_DEFINITION(test_0002_1) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma.aln");
    GTGlobals::sleep(1000);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    GTUtilsMdi::click(os, GTGlobals::Maximize);
    GTGlobals::sleep();

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    bool offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == false, "Offsets are visible");


    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == true, "Offsets are not visible");
}

GUI_TEST_CLASS_DEFINITION(test_0002_2) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gap_col.aln");
    GTGlobals::sleep(1000);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    GTUtilsMdi::click(os, GTGlobals::Maximize);
    GTGlobals::sleep();

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();
    GTGlobals::sleep();

    bool offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == false, "Offsets are visible");


    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == true, "Offsets are not visible");
}

GUI_TEST_CLASS_DEFINITION(test_0002_3) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "revcompl.aln");
    GTGlobals::sleep(1000);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();
    GTGlobals::sleep();

    bool offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == false, "Offsets are visible");

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep();

    mdiWindow = GTUtilsMdi::activeWindow(os, false);
    CHECK_SET_ERR(mdiWindow == NULL, "There is an MDI window");

    QPoint p = GTUtilsProjectTreeView::getItemCenter(os, "revcompl");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();
    GTGlobals::sleep();

    offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == true, "Offsets are not visible");
}

GUI_TEST_CLASS_DEFINITION(test_0002_4) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "revcompl.aln");
    GTGlobals::sleep(1000);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    bool offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == false, "Offsets are visible");

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep();

    QPoint p = GTUtilsProjectTreeView::getItemCenter(os, "revcompl");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Maximize);
    GTGlobals::sleep();

    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == true, "Offsets are not visible");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTGlobals::sleep();

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "action_sort_by_name");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os);
}

GUI_TEST_CLASS_DEFINITION(test_0003_1) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTGlobals::sleep();

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "action_sort_by_name");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os);
}

GUI_TEST_CLASS_DEFINITION(test_0003_2) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "revcompl.aln");
    GTGlobals::sleep();

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "action_sort_by_name");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();

    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "action_sort_by_name");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os);
}

GUI_TEST_CLASS_DEFINITION(test_0003_3) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gap_col.aln");
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os, false);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "action_sort_by_name");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Maximize);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os);
}

GUI_TEST_CLASS_DEFINITION(test_0003_4) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma.aln");
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os, false);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "action_sort_by_name");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep();

    QWidget* mdiWindow = GTUtilsMdi::activeWindow(os, false);
    CHECK_SET_ERR(mdiWindow == NULL, "There is an MDI window");

    QPoint p = GTUtilsProjectTreeView::getItemCenter(os, "ma");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os);
}

GUI_TEST_CLASS_DEFINITION(test_0004)
{
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    Runnable *chooser = new PopupChooser(os, QStringList() << "action_go_to_position");
    Runnable *filler = new GoToDialogFiller(os, 6);
    GTUtilsDialog::waitForDialog(os, filler);
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();
    GTGlobals::sleep();

    QRect expectedRect(5, 0, 1, 1);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, expectedRect);

    Runnable *filler1 = new GoToDialogFiller(os, 6);
    GTUtilsDialog::waitForDialog(os, filler1);
    GTKeyboardDriver::keyClick(os, 'g', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, expectedRect);
}

GUI_TEST_CLASS_DEFINITION(test_0005) {
// Check maligniment view status bar coordinates

// 1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");

// Expected state: Alignment length 14, left offset 1, right offset 14
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLength(os) == 14, "Wrong length");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLeftOffset(os) == 1, "Wrong left offset");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getRightOffset(os) == 14, "Wrong right offset");

    QWidget *msaEditorStatusBar = GTWidget::findWidget(os, "msa_editor_status_bar");
    CHECK_SET_ERR(msaEditorStatusBar != NULL, "MSAEditorStatusBar is NULL");

    QLabel* line = qobject_cast<QLabel*>(GTWidget::findWidget(os, "Line", msaEditorStatusBar));
    CHECK_SET_ERR(line != NULL, "Line of MSAEditorStatusBar is NULL");
    QLabel* column = qobject_cast<QLabel*>(GTWidget::findWidget(os, "Column", msaEditorStatusBar));
    CHECK_SET_ERR(column != NULL, "Column of MSAEditorStatusBar is NULL");

// 2. Put cursor in 5th symbol for Tettigonia_virdissima sequence.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(4, 3));
// Expected state: coordinates in status bar Ln 4/10 Col 5/14
    CHECK_SET_ERR(line->text() == "Ln 4 / 10", "Line is " + line->text());
    CHECK_SET_ERR(column->text() == "Col 5 / 14", "Column is " + column->text());

// 3. Put cursor in 2nd symbol for Podisma_sapporensis sequence.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(1, 8));
// Expected state: coordinates in status bar Ln 9/10 Col 2/14
    CHECK_SET_ERR(line->text() == "Ln 9 / 10", "Line is " + line->text());
    CHECK_SET_ERR(column->text() == "Col 2 / 14", "Column is " + column->text());

// 4. Select area from 8th symbol for Tettigonia_virdissima sequence(top left corner) to 13th symbol for Podisma_sapporensis sequence.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(7, 3), QPoint(12, 7));
// Expected state: coordinates in status bar Ln 4/10 Col 8/14
    CHECK_SET_ERR(line->text() == "Ln 4 / 10", "Line is " + line->text());
    CHECK_SET_ERR(column->text() == "Col 8 / 14", "Column is " + column->text());
}

GUI_TEST_CLASS_DEFINITION(test_0005_1) {
// Check maligniment view status bar coordinates

// 1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");

// Expected state: Alignment length 14, left offset 1, right offset 14
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLength(os) == 14, "Wrong length");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLeftOffset(os) == 1, "Wrong left offset");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getRightOffset(os) == 14, "Wrong right offset");

    QWidget *msaEditorStatusBar = GTWidget::findWidget(os, "msa_editor_status_bar");
    CHECK_SET_ERR(msaEditorStatusBar != NULL, "MSAEditorStatusBar is NULL");

    QLabel* line = qobject_cast<QLabel*>(GTWidget::findWidget(os, "Line", msaEditorStatusBar));
    CHECK_SET_ERR(line != NULL, "Line of MSAEditorStatusBar is NULL");
    QLabel* column = qobject_cast<QLabel*>(GTWidget::findWidget(os, "Column", msaEditorStatusBar));
    CHECK_SET_ERR(column != NULL, "Column of MSAEditorStatusBar is NULL");

// CHANGES: click order changed
// 3. Put cursor in 2nd symbol for Podisma_sapporensis sequence.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(1, 8));
// Expected state: coordinates in status bar Ln 9/10 Col 2/14
    CHECK_SET_ERR(line->text() == "Ln 9 / 10", "Line is " + line->text());
    CHECK_SET_ERR(column->text() == "Col 2 / 14", "Column is " + column->text());

// 2. Put cursor in 5th symbol for Tettigonia_virdissima sequence.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(4, 3));
// Expected state: coordinates in status bar Ln 4/10 Col 5/14
    CHECK_SET_ERR(line->text() == "Ln 4 / 10", "Line is " + line->text());
    CHECK_SET_ERR(column->text() == "Col 5 / 14", "Column is " + column->text());

// 4. Select area from 8th symbol for Tettigonia_virdissima sequence(top left corner) to 13th symbol for Podisma_sapporensis sequence.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(7, 3), QPoint(12, 7));
// Expected state: coordinates in status bar Ln 4/10 Col 8/14
    CHECK_SET_ERR(line->text() == "Ln 4 / 10", "Line is " + line->text());
    CHECK_SET_ERR(column->text() == "Col 8 / 14", "Column is " + column->text());
}

GUI_TEST_CLASS_DEFINITION(test_0005_2) {
// Check maligniment view status bar coordinates

// 1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");

// Expected state: Alignment length 14, left offset 1, right offset 14
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLength(os) == 14, "Wrong length");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLeftOffset(os) == 1, "Wrong left offset");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getRightOffset(os) == 14, "Wrong right offset");

    QWidget *msaEditorStatusBar = GTWidget::findWidget(os, "msa_editor_status_bar");
    CHECK_SET_ERR(msaEditorStatusBar != NULL, "MSAEditorStatusBar is NULL");

    QLabel* line = qobject_cast<QLabel*>(GTWidget::findWidget(os, "Line", msaEditorStatusBar));
    CHECK_SET_ERR(line != NULL, "Line of MSAEditorStatusBar is NULL");
    QLabel* column = qobject_cast<QLabel*>(GTWidget::findWidget(os, "Column", msaEditorStatusBar));
    CHECK_SET_ERR(column != NULL, "Column of MSAEditorStatusBar is NULL");

// 2. Put cursor in 5th symbol for Tettigonia_virdissima sequence.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(4, 3));
// Expected state: coordinates in status bar Ln 4/10 Col 5/14
    CHECK_SET_ERR(line->text() == "Ln 4 / 10", "Line is " + line->text());
    CHECK_SET_ERR(column->text() == "Col 5 / 14", "Column is " + column->text());

// CHANGES: close and open MDI window, hide projectTreeView
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "ma2_gapped"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsProjectTreeView::toggleView(os);
    GTGlobals::sleep();

    msaEditorStatusBar = GTWidget::findWidget(os, "msa_editor_status_bar");
    CHECK_SET_ERR(msaEditorStatusBar != NULL, "MSAEditorStatusBar is NULL");

    line = qobject_cast<QLabel*>(GTWidget::findWidget(os, "Line", msaEditorStatusBar));
    CHECK_SET_ERR(line != NULL, "Line of MSAEditorStatusBar is NULL");
    column = qobject_cast<QLabel*>(GTWidget::findWidget(os, "Column", msaEditorStatusBar));
    CHECK_SET_ERR(column != NULL, "Column of MSAEditorStatusBar is NULL");

// 3. Put cursor in 2nd symbol for Podisma_sapporensis sequence.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(1, 8));
// Expected state: coordinates in status bar Ln 9/10 Col 2/14
    CHECK_SET_ERR(line->text() == "Ln 9 / 10", "Line is " + line->text());
    CHECK_SET_ERR(column->text() == "Col 2 / 14", "Column is " + column->text());

// 4. Select area from 8th symbol for Tettigonia_virdissima sequence(top left corner) to 13th symbol for Podisma_sapporensis sequence.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(7, 3), QPoint(12, 7));
// Expected state: coordinates in status bar Ln 4/10 Col 8/14
    CHECK_SET_ERR(line->text() == "Ln 4 / 10", "Line is " + line->text());
    CHECK_SET_ERR(column->text() == "Col 8 / 14", "Column is " + column->text());
}


GUI_TEST_CLASS_DEFINITION(test_0006) {
    // Check find from status bar

    // 1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");

    // Expected state: Aligniment length 14, left offset 1, right offset 14
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLength(os) == 14, "Wrong length");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLeftOffset(os) == 1, "Wrong left offset");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getRightOffset(os) == 14, "Wrong right offset");

    QWidget *msaEditorStatusBar = GTWidget::findWidget(os, "msa_editor_status_bar");
    CHECK_SET_ERR(msaEditorStatusBar != NULL, "MSAEditorStatusBar is NULL");

    QLineEdit *searchEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "searchEdit", msaEditorStatusBar));
    QWidget *findForward = GTWidget::findWidget(os, "Find forward", msaEditorStatusBar);
    QWidget *findBackward = GTWidget::findWidget(os, "Find backward", msaEditorStatusBar);

    // 2. Put AAGT in text field at status bar. Click Find next button.
    GTLineEdit::setText(os, searchEdit, "AAGT");
    GTWidget::click(os, findForward);
    // Expected state: find result sequence Tettigonia_viridissima region 1..4
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 3, 4, 1));

    // 3. Click Find next button.
    GTWidget::click(os, findForward);
    // Expected state: find result sequence Mecopoda_elongata__Ishigaki__J region 1..4
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 7, 4, 1));

    // 4. Click Find previous button.
    GTWidget::click(os, findBackward);
    // Expected state: find result sequence Tettigonia_viridissima region 1..4
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 3, 4, 1));
}

GUI_TEST_CLASS_DEFINITION(test_0006_1) {
    // Check find from status bar

    // 1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");

    // Expected state: Aligniment length 14, left offset 1, right offset 14
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLength(os) == 14, "Wrong length");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLeftOffset(os) == 1, "Wrong left offset");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getRightOffset(os) == 14, "Wrong right offset");

    QWidget *msaEditorStatusBar = GTWidget::findWidget(os, "msa_editor_status_bar");
    CHECK_SET_ERR(msaEditorStatusBar != NULL, "MSAEditorStatusBar is NULL");

    QLineEdit *searchEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "searchEdit", msaEditorStatusBar));
    QWidget *findForward = GTWidget::findWidget(os, "Find forward", msaEditorStatusBar);
    QWidget *findBackward = GTWidget::findWidget(os, "Find backward", msaEditorStatusBar);

    // 2. Put AAGT in text field at status bar. Click Find next button.
    GTLineEdit::setText(os, searchEdit, "AAGT");
    GTWidget::click(os, findForward);
    // Expected state: find result sequence Tettigonia_viridissima region 1..4
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 3, 4, 1));

    // 3. Click Find next button. CHANGES: click 3 times
    GTWidget::click(os, findForward);
    GTWidget::click(os, findForward);
    GTWidget::click(os, findForward);
    // Expected state: find result sequence Mecopoda_elongata__Ishigaki__J region 1..4
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 7, 4, 1));

    // 4. Click Find previous button. CHANGES: click 3 times
    GTWidget::click(os, findBackward);
    GTWidget::click(os, findBackward);
    GTWidget::click(os, findBackward);
    // Expected state: find result sequence Tettigonia_viridissima region 1..4
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 3, 4, 1));
}

GUI_TEST_CLASS_DEFINITION(test_0006_2) {
    // Check find from status bar

    // 1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");

    // Expected state: Aligniment length 14, left offset 1, right offset 14
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLength(os) == 14, "Wrong length");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLeftOffset(os) == 1, "Wrong left offset");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getRightOffset(os) == 14, "Wrong right offset");

    QWidget *msaEditorStatusBar = GTWidget::findWidget(os, "msa_editor_status_bar");
    CHECK_SET_ERR(msaEditorStatusBar != NULL, "MSAEditorStatusBar is NULL");

    QLineEdit *searchEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "searchEdit", msaEditorStatusBar));
    QWidget *findForward = GTWidget::findWidget(os, "Find forward", msaEditorStatusBar);
    QWidget *findBackward = GTWidget::findWidget(os, "Find backward", msaEditorStatusBar);

    // 2. Put AAGT in text field at status bar. Click Find next button.
    GTLineEdit::setText(os, searchEdit, "AAGT");
    GTWidget::click(os, findForward);
    // Expected state: find result sequence Tettigonia_viridissima region 1..4
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 3, 4, 1));

    // CHANGES: clear search text, click Find forward and check that selection wasn't changed; set AAGT again
    GTLineEdit::setText(os, searchEdit, "");
    GTWidget::click(os, findForward);
    // Expected state: find result sequence Tettigonia_viridissima region 1..4
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 3, 4, 1));
    GTLineEdit::setText(os, searchEdit, "AAGT");
    GTGlobals::sleep();

    // 3. Click Find next button.
    GTWidget::click(os, findForward);
    // Expected state: find result sequence Mecopoda_elongata__Ishigaki__J region 1..4
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 7, 4, 1));

    // CHANGES: clear search text, click Find backward and check that selection wasn't changed; set AAGT again
    GTLineEdit::setText(os, searchEdit, "");
    GTWidget::click(os, findBackward);
    // Expected state: find result sequence Mecopoda_elongata__Ishigaki__J region 1..4
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 7, 4, 1));
    GTLineEdit::setText(os, searchEdit, "AAGT");
    GTGlobals::sleep();

    // 4. Click Find previous button.
    GTWidget::click(os, findBackward);
    // Expected state: find result sequence Tettigonia_viridissima region 1..4
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 3, 4, 1));
}

GUI_TEST_CLASS_DEFINITION(test_0007) {
    //1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");
    
    //Expected state: Aligniment length 14, left offset 1, right offset 14

    //2. Do double click on Tettigonia_viridissima sequence name.
    //Expected state: Rename dialog appears
    //3. Put "Sequence_a" into text field. Click OK.

    Runnable *filler = new RenameSequenceFiller(os, "Sequence_a", "Tettigonia_viridissima");
    GTUtilsDialog::waitForDialog(os, filler, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed to Sequence_a
    Runnable *filler2 = new RenameSequenceFiller(os, "Sequence_a", "Sequence_a");
    GTUtilsDialog::waitForDialog(os, filler2, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //4. Rlick Undo button.
    QWidget *mainWindow = AppContext::getMainWindow()->getQMainWindow();

    GTKeyboardDriver::keyClick(os, 'z', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    
    //Expected state: Tettigonia_viridissima renamed back
    Runnable *filler3 = new RenameSequenceFiller(os, "Tettigonia_viridissima", "Tettigonia_viridissima");
    GTUtilsDialog::waitForDialog(os, filler3, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();    
}

GUI_TEST_CLASS_DEFINITION(test_0007_1) {
    //1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    //Expected state: Aligniment length 14, left offset 1, right offset 14

    //2. Do double click on Tettigonia_viridissima sequence name.
    //Expected state: Rename dialog appears
    //3. Put "Sequence_a" into text field. Click OK.

    Runnable *filler = new RenameSequenceFiller(os, "Sequence_a", "Tettigonia_viridissima");
    GTUtilsDialog::waitForDialog(os, filler, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed to Sequence_a
    
    Runnable *filler2 = new RenameSequenceFiller(os, "Sequence_a", "Sequence_a");
    GTUtilsDialog::waitForDialog(os, filler2, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //4. Rlick Undo button. CHANGES: clicking undo by mouse
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "msa_action_undo"));
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed back
    Runnable *filler3 = new RenameSequenceFiller(os, "Tettigonia_viridissima", "Tettigonia_viridissima");
    GTUtilsDialog::waitForDialog(os, filler3, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0007_2) {
    //1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    //Expected state: Aligniment length 14, left offset 1, right offset 14

    //2. Do double click on Tettigonia_viridissima sequence name. CHANGES: another sequence renamed
    //Expected state: Rename dialog appears
    //3. Put "Sequence_a" into text field. Click OK.

    Runnable *filler = new RenameSequenceFiller(os, "Sequence_a", "Bicolorana_bicolor_EF540830");
    GTUtilsDialog::waitForDialog(os, filler, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,2));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed to Sequence_a
    Runnable *filler2 = new RenameSequenceFiller(os, "Sequence_a", "Sequence_a");
    GTUtilsDialog::waitForDialog(os, filler2, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,2));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //4. Rlick Undo button.
    QWidget *mainWindow = AppContext::getMainWindow()->getQMainWindow();

    GTKeyboardDriver::keyClick(os, 'z', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed back
    Runnable *filler3 = new RenameSequenceFiller(os, "Bicolorana_bicolor_EF540830", "Bicolorana_bicolor_EF540830");
    GTUtilsDialog::waitForDialog(os, filler3, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,2));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0007_3) {
    //1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    //Expected state: Aligniment length 14, left offset 1, right offset 14

    //2. Do double click on Tettigonia_viridissima sequence name. CHANGES: another sequence renamed
    //Expected state: Rename dialog appears
    //3. Put "Sequence_a" into text field. Click OK.

    Runnable *filler = new RenameSequenceFiller(os, "Sequence_a", "Phaneroptera_falcata");
    GTUtilsDialog::waitForDialog(os, filler, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,0));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed to Sequence_a
    Runnable *filler2 = new RenameSequenceFiller(os, "Sequence_a", "Sequence_a");
    GTUtilsDialog::waitForDialog(os, filler2, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,0));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //4. Rlick Undo button.
    QWidget *mainWindow = AppContext::getMainWindow()->getQMainWindow();

    GTKeyboardDriver::keyClick(os, 'z', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed back
    Runnable *filler3 = new RenameSequenceFiller(os, "Phaneroptera_falcata", "Phaneroptera_falcata");
    GTUtilsDialog::waitForDialog(os, filler3, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,0));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();    
}

GUI_TEST_CLASS_DEFINITION(test_0007_4) {
    //1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    //Expected state: Aligniment length 14, left offset 1, right offset 14

    //2. Do double click on Tettigonia_viridissima sequence name. CHANGES: another sequence renamed
    //Expected state: Rename dialog appears
    //3. Put "Sequence_a" into text field. Click OK.

    Runnable *filler = new RenameSequenceFiller(os, "Sequence_a", "Conocephalus_sp.");
    GTUtilsDialog::waitForDialog(os, filler, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,5));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed to Sequence_a
    Runnable *filler2 = new RenameSequenceFiller(os, "Sequence_a", "Sequence_a");
    GTUtilsDialog::waitForDialog(os, filler2, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,5));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //4. Rlick Undo button.
    QWidget *mainWindow = AppContext::getMainWindow()->getQMainWindow();

    GTKeyboardDriver::keyClick(os, 'z', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed back
    Runnable *filler3 = new RenameSequenceFiller(os, "Conocephalus_sp.", "Conocephalus_sp.");
    GTUtilsDialog::waitForDialog(os, filler3, GUIDialogWaiter::Modal);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,5));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();    
}

GUI_TEST_CLASS_DEFINITION(test_0010) {

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");

// 2. Do document context menu {Export->Export aligniment to amino format}
// 3. Translate with default settings
    Runnable *filler = new ExportMSA2MSADialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler, GUIDialogWaiter::Modal);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::selectArea(os);
// copy to clipboard
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

// Expected state: every sequense name the same as it amino translation
    QString clipboardText = GTClipboard::text(os);
    QString expectedMSA = "L\nS\nD\nS\nP\nK";

    CHECK_SET_ERR(clipboardText == expectedMSA, "Clipboard string and expected MSA string differs");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0010_1) {

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");

// 2. Do document context menu {Export->Export aligniment to amino format}
// 3. Translate with default settings
    Runnable *filler = new ExportMSA2MSADialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler, GUIDialogWaiter::Modal);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::selectArea(os);
// copy to clipboard
    Runnable* chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: every sequense name the same as it amino translation
    QString clipboardText = GTClipboard::text(os);
    QString expectedMSA = "L\nS\nD\nS\nP\nK";

    CHECK_SET_ERR(clipboardText == expectedMSA, "Clipboard string and expected MSA string differs");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0010_2) {

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");

// 2. Do document context menu {Export->Export aligniment to amino format}
// 3. Translate with default settings
    Runnable *filler = new ExportMSA2MSADialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler, GUIDialogWaiter::Modal);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::selectArea(os);
// copy to clipboard
    Runnable* chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: every sequense name the same as it amino translation
    QString clipboardText = GTClipboard::text(os);
    QString expectedMSA = "L\nS\nD\nS\nP\nK";
    CHECK_SET_ERR(clipboardText == expectedMSA, "Clipboard string and expected MSA string differs");

    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    QStringList expectedNameList = QStringList() << "L" << "S" << "D" << "S" << "P" << "K";

    CHECK_SET_ERR(nameList == expectedNameList, "name lists differ");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0011) {
// In-place reverse complement replace in MSA Editor (0002425)

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");

// 2. Select first sequence and do context menu {Edit->Replace selected rows with reverce complement}
    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(-1, 0));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: sequence changed from TTG -> CAA 
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep();
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "CAA", "Clipboard string and expected MSA string differs");

//                 sequence name  changed from L -> L|revcompl
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() >= 2, "nameList doesn't contain enough strings");
    CHECK_SET_ERR((nameList[0] == "L") && (nameList[1] == "revcompl"), "There are no 'L|revcompl' in nameList");

// 3. Do step 2 again
    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: sequence changed from CAA -> TTG 
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep();
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "TTG", "Clipboard string and expected MSA string differs");

//                 sequence name changed from L|revcompl ->    
    nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() >= 2, "nameList doesn't contain enough strings");
    CHECK_SET_ERR((nameList[0] == "L") && (nameList[1] != "revcompl"), "There are 'L|revcompl' in nameList");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0011_1) {
// In-place reverse complement replace in MSA Editor (0002425)

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");

// 2. Select first sequence and do context menu {Edit->Replace selected rows with reverce complement}
    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(-1, 0));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: sequence changed from TTG -> CAA 
// CHANGES: copy by context menu
    GTGlobals::sleep();
    Runnable* chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "CAA", "Clipboard string and expected MSA string differs");

//                 sequence name  changed from L -> L|revcompl
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() >= 2, "nameList doesn't contain enough strings");
    CHECK_SET_ERR((nameList[0] == "L") && (nameList[1] == "revcompl"), "There are no 'L|revcompl' in nameList");

// 3. Do step 2 again
    Runnable *chooser3 = new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement");
    GTUtilsDialog::waitForDialog(os, chooser3, GUIDialogWaiter::Popup);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: sequence changed from CAA -> TTG 
    GTGlobals::sleep();
// CHANGES: copy by context menu
    GTGlobals::sleep();
    Runnable* chooser4 = new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection");
    GTUtilsDialog::waitForDialog(os, chooser4, GUIDialogWaiter::Popup);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "TTG", "Clipboard string and expected MSA string differs");

//                 sequence name changed from L|revcompl ->    
    nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() >= 2, "nameList doesn't contain enough strings");
    CHECK_SET_ERR((nameList[0] == "L") && (nameList[1] != "revcompl"), "There are 'L|revcompl' in nameList");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0011_2) {
// In-place reverse complement replace in MSA Editor (0002425)

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");

// 2. Select first sequence and do context menu {Edit->Replace selected rows with reverce complement}
// CHANGES: using main menu
    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(-1, 0));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
//    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: sequence changed from TTG -> CAA 
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep();
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "CAA", "Clipboard string and expected MSA string differs");

//                 sequence name  changed from L -> L|revcompl
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() >= 2, "nameList doesn't contain enough strings");
    CHECK_SET_ERR((nameList[0] == "L") && (nameList[1] == "revcompl"), "There are no 'L|revcompl' in nameList");

// 3. Do step 2 again
// CHANGES: using main menu
    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
//    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: sequence changed from CAA -> TTG 
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep();
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "TTG", "Clipboard string and expected MSA string differs");

//                 sequence name changed from L|revcompl ->    
    nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() >= 2, "nameList doesn't contain enough strings");
    CHECK_SET_ERR((nameList[0] == "L") && (nameList[1] != "revcompl"), "There are 'L|revcompl' in nameList");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0012) {
// Add tests on alignment translation features (0002432)

// 1. Open file _common_data\scenarios\msa\revcompl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "revcompl.aln");

// 2. Select all sequences and do context menu {Edit->Replace selected rows with reverce complement}
    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    GTUtilsMSAEditorSequenceArea::selectArea(os);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: result alignement must be
// CAA---
// --TGA-
// ---ATC

    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, -1), QPoint(-1, 0));
// copy to clipboard
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

// Expected state: every sequense name the same as it amino translation
    QString clipboardText = GTClipboard::text(os);
    QString expectedMSA = "CAA---\n--TGA-\n---ATC";

    CHECK_SET_ERR(clipboardText == expectedMSA, "Clipboard string and expected MSA string differs");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0012_1) {
// Add tests on alignment translation features (0002432)

// 1. Open file _common_data\scenarios\msa\revcompl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "revcompl.aln");

// 2. Select all sequences and do context menu {Edit->Replace selected rows with reverce complement}
    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    GTUtilsMSAEditorSequenceArea::selectArea(os);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: result alignement must be
// CAA---
// --TGA-
// ---ATC

    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, -1), QPoint(-1, 0));
// copy to clipboard. CHANGES: copy by context menu
    Runnable* chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: every sequense name the same as it amino translation
    QString clipboardText = GTClipboard::text(os);
    QString expectedMSA = "CAA---\n--TGA-\n---ATC";

    CHECK_SET_ERR(clipboardText == expectedMSA, "Clipboard string and expected MSA string differs");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0012_2) {
// Add tests on alignment translation features (0002432)

// 1. Open file _common_data\scenarios\msa\revcompl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "revcompl.aln");

// 2. Select all sequences and do context menu {Edit->Replace selected rows with reverce complement}
    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    GTUtilsMSAEditorSequenceArea::selectArea(os);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// CHANGES: close and open MDI window
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "revcompl"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// Expected state: result alignement must be
// CAA---
// --TGA-
// ---ATC

    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, -1), QPoint(-1, 0));
// copy to clipboard. CHANGES: copy by context menu
    Runnable* chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: every sequense name the same as it amino translation
    QString clipboardText = GTClipboard::text(os);
    QString expectedMSA = "CAA---\n--TGA-\n---ATC";

    CHECK_SET_ERR(clipboardText == expectedMSA, "Clipboard string and expected MSA string differs");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0013) {
// Kalign crashes on amino alignment that was generated from nucleotide alignment (0002658)

// 1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

// 2. Convert alignment to amino. Use context menu {Export->Amino translation of alignment rows}
    Runnable *filler = new ExportMSA2MSADialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler, GUIDialogWaiter::Modal);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();

// 3. Open converted alignment. Use context menu {Align->Align with Kalign}
    Runnable *filler2 = new KalignDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler2, GUIDialogWaiter::Modal);

    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();
    GTGlobals::sleep();

// Expected state: UGENE not crash
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0013_1) {

// Kalign crashes on amino alignment that was generated from nucleotide alignment (0002658)

// 1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

// 2. Convert alignment to amino. Use context menu {Export->Amino translation of alignment rows}
    Runnable *filler = new ExportMSA2MSADialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler, GUIDialogWaiter::Modal);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();

// CHANGES: close and open MDI window
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Multiple alignment"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();


// 3. Open converted alignment. Use context menu {Align->Align with Kalign}
    Runnable *filler2 = new KalignDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler2, GUIDialogWaiter::Modal);

    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();
    GTGlobals::sleep();

// Expected state: UGENE not crash
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0013_2) {
// Kalign crashes on amino alignment that was generated from nucleotide alignment (0002658)

// 1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

// 2. Convert alignment to amino. Use context menu {Export->Amino translation of alignment rows}
    Runnable *filler = new ExportMSA2MSADialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler, GUIDialogWaiter::Modal);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();

// 3. Open converted alignment. Use context menu {Align->Align with Kalign}
    Runnable *filler2 = new KalignDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler2, GUIDialogWaiter::Modal);

// CHANGES: using main menu
    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();
    GTGlobals::sleep();

// Expected state: UGENE not crash
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0014) {
// UGENE crashes in malignment editor after aligning (UGENE-6)

// 1. Do menu tools->multiple alignment->kalign, set input alignment "data/samples/CLUSTALW/COI.aln" and press Align button
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    GTLogTracer logTracer;
    Runnable *filler2 = new KalignDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler2, GUIDialogWaiter::Modal);

    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();
    GTGlobals::sleep();

// 2. after kalign finishes and msa opens insert gaps and click in alignment
    GTGlobals::sleep(5000);
    GTUtilsLog::check(os, logTracer);

    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));
    GTMouseDriver::click(os);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep();

    GTMouseDriver::click(os);
    GTGlobals::sleep();

// Expected state: UGENE not crash
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0014_1) {
// UGENE crashes in malignment editor after aligning (UGENE-6)

// 1. Do menu tools->multiple alignment->kalign, set input alignment "data/samples/CLUSTALW/COI.aln" and press Align button
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    GTLogTracer logTracer;
    Runnable *filler2 = new KalignDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler2, GUIDialogWaiter::Modal);

// CHANGES: using main menu
    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();
    GTGlobals::sleep();

// 2. after kalign finishes and msa opens insert gaps and click in alignment
    GTGlobals::sleep(5000);
    GTUtilsLog::check(os, logTracer);

    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));
    GTMouseDriver::click(os);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep();

    GTMouseDriver::click(os);
    GTGlobals::sleep();

// Expected state: UGENE not crash
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0014_2) {
// UGENE crashes in malignment editor after aligning (UGENE-6)

// 1. Do menu tools->multiple alignment->kalign, set input alignment "data/samples/CLUSTALW/COI.aln" and press Align button
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

// CHANGES: close and open MDI window, close Project tree view
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep();
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
    GTUtilsProjectTreeView::toggleView(os);
    GTGlobals::sleep();

    GTLogTracer logTracer;
    Runnable *filler2 = new KalignDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler2, GUIDialogWaiter::Modal);

    Runnable *chooser2 = new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign");
    GTUtilsDialog::waitForDialog(os, chooser2, GUIDialogWaiter::Popup);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();
    GTGlobals::sleep();

// 2. after kalign finishes and msa opens insert gaps and click in alignment
    GTGlobals::sleep(5000);
    GTUtilsLog::check(os, logTracer);

    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));
    GTMouseDriver::click(os);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep();

    GTMouseDriver::click(os);
    GTGlobals::sleep();

// Expected state: UGENE not crash
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0015) {
// ugene crashes when removing document after kalign (UGENE-36)
// 
// 1. create empty project
// 2. do menu {tools->multiple alignment->kalign}, set aligned document samples/CLUSTALW/COI.aln

    Runnable *filler = new KalignDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler);
    Runnable *ob = new GTFileDialogUtils(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsDialog::waitForDialog(os, ob);
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Multiple alignment" << "Kalign");
    GTGlobals::sleep();

// 3. aligned document opens
    GTGlobals::sleep(5000);
    GTUtilsMdi::activeWindow(os);

// 4. select document in project and press del
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI.aln"));
    GTMouseDriver::click(os);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

// Expected state: UGENE not crash
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0015_1) {
// ugene crashes when removing document after kalign (UGENE-36)
// 
// 1. create empty project
// 2. do menu {tools->multiple alignment->kalign}, set aligned document samples/CLUSTALW/COI.aln

// CHANGES: opens file, Kalign by popup menu
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTGlobals::sleep();

    GTLogTracer logTracer;
    Runnable *filler = new KalignDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler);

    Runnable *chooser = new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();

// 3. aligned document opens
    GTGlobals::sleep(5000);
    GTUtilsMdi::activeWindow(os);
    GTUtilsLog::check(os, logTracer);

// 4. select document in project and press del
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI.aln"));
    GTMouseDriver::click(os);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

// Expected state: UGENE not crash
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0015_2) {
// ugene crashes when removing document after kalign (UGENE-36)
// 
// 1. create empty project
// 2. do menu {tools->multiple alignment->kalign}, set aligned document samples/CLUSTALW/COI.aln

    Runnable *filler = new KalignDialogFiller(os);
    GTUtilsDialog::waitForDialog(os, filler);
    Runnable *ob = new GTFileDialogUtils(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsDialog::waitForDialog(os, ob);
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Multiple alignment" << "Kalign");
    GTGlobals::sleep();

// 3. aligned document opens
    GTGlobals::sleep(5000);
    GTUtilsMdi::activeWindow(os);

// CHANGES: close MDI window
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep();

// 4. select document in project and press del
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI.aln"));
    GTMouseDriver::click(os);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

// Expected state: UGENE not crash
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0016) {

// 1. Run Ugene. Open file _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
    GTGlobals::sleep();

// 2. Open same file in text editor. Change first 3 bases of 'Phaneroptera_falcata'
//    from 'AAG' to 'CTT' and save file.
//CHANGES: backup old file, copy changed file
    GTFile::backup(os, testDir + "_common_data/scenarios/msa/ma2_gapped.aln");
    GTFile::copy(os, testDir + "_common_data/scenarios/msa/ma2_gapped_edited.aln", testDir + "_common_data/scenarios/msa/ma2_gapped.aln");

//    Expected state: Dialog suggesting to reload modified document has appeared.
// 3. Press 'Yes'.
    Runnable* messageBox = new MessageBoxDialogFiller(os, QMessageBox::Yes);
    GTUtilsDialog::waitForDialog(os, messageBox);
    GTGlobals::sleep(5000);

//    Expected state: document was reloaded, view activated.
//    'Phaneroptera_falcata' starts with CTT.
    GTGlobals::sleep();
    GTUtilsMdi::activeWindow(os);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(2, 0));
// copy to clipboard
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    QString clipboardText = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardText == "CTT", "MSA part differs from expected");
    GTFile::restore(os, testDir + "_common_data/scenarios/msa/ma2_gapped.aln");
}

GUI_TEST_CLASS_DEFINITION(test_0016_1) {
// 1. Run Ugene. Open file _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
    GTGlobals::sleep();

// CHANGES: insert gaps in the beginning
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(0, 0));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep();

// 2. Open same file in text editor. Change first 3 bases of 'Phaneroptera_falcata'
//    from 'AAG' to 'CTT' and save file.
//CHANGES: backup old file, copy changed file
    GTFile::backup(os, testDir + "_common_data/scenarios/msa/ma2_gapped.aln");
    GTFile::copy(os, testDir + "_common_data/scenarios/msa/ma2_gapped_edited.aln", testDir + "_common_data/scenarios/msa/ma2_gapped.aln");

//    Expected state: Dialog suggesting to reload modified document has appeared.
// 3. Press 'Yes'.
    Runnable* messageBox = new MessageBoxDialogFiller(os, QMessageBox::Yes);
    GTUtilsDialog::waitForDialog(os, messageBox);
    GTGlobals::sleep(5000);

//    Expected state: document was reloaded, view activated.
//    'Phaneroptera_falcata' starts with CTT.
    GTGlobals::sleep();
    GTUtilsMdi::activeWindow(os);

    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(2, 0));
// copy to clipboard
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    QString clipboardText = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardText == "CTT", "MSA part differs from expected");
    GTFile::restore(os, testDir + "_common_data/scenarios/msa/ma2_gapped.aln");
}

GUI_TEST_CLASS_DEFINITION(test_0016_2) {
// 1. Run Ugene. Open file _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
    GTGlobals::sleep();

// 2. Open same file in text editor. Change first 3 bases of 'Phaneroptera_falcata'
//    from 'AAG' to 'CTT' and save file.
//CHANGES: backup old file, copy changed file
    GTFile::backup(os, testDir + "_common_data/scenarios/msa/ma2_gapped.aln");
    GTFile::copy(os, testDir + "_common_data/scenarios/msa/ma2_gapped_edited.aln", testDir + "_common_data/scenarios/msa/ma2_gapped.aln");

//    Expected state: Dialog suggesting to reload modified document has appeared.
// 3. Press 'Yes'.
    Runnable* messageBox = new MessageBoxDialogFiller(os, QMessageBox::Yes);
    GTUtilsDialog::waitForDialog(os, messageBox);
    GTGlobals::sleep(5000);

//    Expected state: document was reloaded, view activated.
//    'Phaneroptera_falcata' starts with CTT.
    GTGlobals::sleep();
    GTUtilsMdi::activeWindow(os);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(2, 0));
// copy to clipboard
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "CTT", "MSA part differs from expected");

// CHANGES: select item in project tree view and press delete
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "ma2_gapped.aln"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(5000);

    GTFile::restore(os, testDir + "_common_data/scenarios/msa/ma2_gapped.aln");
}


} // namespace GUITest_common_scenarios_msa_editor
} // namespace U2
