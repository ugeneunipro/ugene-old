/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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
#include "primitives/GTAction.h"
#include <primitives/GTComboBox.h>
#include <primitives/GTCheckBox.h>
#include <drivers/GTMouseDriver.h>
#include <drivers/GTKeyboardDriver.h>
#include "primitives/GTMenu.h"
#include "system/GTFile.h"
#include <base_dialogs/GTFileDialog.h>
#include "system/GTClipboard.h"
#include <primitives/GTToolbar.h>
#include <primitives/GTLineEdit.h>
#include <primitives/GTSpinBox.h>
#include <primitives/GTRadioButton.h>
#include "api/GTSequenceReadingModeDialogUtils.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsLog.h"
#include "GTUtilsBookmarksTreeView.h"
#include "GTUtilsNotifications.h"
#include "GTUtilsProject.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsOptionPanelMSA.h"
#include <base_dialogs/DefaultDialogFiller.h>
#include <base_dialogs/ColorDialogFiller.h>
#include "primitives/PopupChooser.h"
#include <base_dialogs/MessageBoxFiller.h>
#include <base_dialogs/FontDialogFiller.h>
#include "runnables/ugene/corelibs/U2Gui/PositionSelectorFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportMSA2MSADialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportMSA2SequencesDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSelectedSequenceFromAlignmentDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSequences2MSADialogFiller.h"
#include "runnables/ugene/plugins/weight_matrix/PwmBuildDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/kalign/KalignDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/umuscle/MuscleDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/util/RenameSequenceFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ExportDocumentDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ExportImageDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ProjectTreeItemSelectorDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/RangeSelectionDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/BuildTreeDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/DeleteGapsDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/DistanceMatrixDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ExportHighlightedDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ExtractSelectedAsMSADialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/GenerateAlignmentProfileDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/WizardFiller.h"
#include "utils/GTThread.h"

#include <U2Gui/ToolsMenu.h>

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorNameList.h>
#include <U2View/ADVConstants.h>
#include <U2Test/UGUITest.h>


namespace U2 {

namespace GUITest_common_scenarios_msa_editor {
using namespace HI;

GUI_TEST_CLASS_DEFINITION(test_0001) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
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
    GTUtilsTaskTreeView::waitTaskFinished(os);
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
    GTUtilsTaskTreeView::waitTaskFinished(os);
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
    GTUtilsTaskTreeView::waitTaskFinished(os);
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

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "translations_nucl.aln", GTFileDialog::Cancel);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "translations_nucl.aln", GTFileDialog::Open);
    GTUtilsTaskTreeView::waitTaskFinished(os);
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
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets"));

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    bool offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == false, "Offsets are visible");


    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets"));

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == true, "Offsets are not visible");

}

GUI_TEST_CLASS_DEFINITION(test_0002_1) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets"));

    GTUtilsMdi::click(os, GTGlobals::Maximize);
    GTGlobals::sleep();

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    bool offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == false, "Offsets are visible");


    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets"));

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == true, "Offsets are not visible");
}

GUI_TEST_CLASS_DEFINITION(test_0002_2) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gap_col.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    GTUtilsMdi::click(os, GTGlobals::Maximize);
    GTGlobals::sleep();

    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "View" << "Show offsets");
    GTGlobals::sleep();
    GTGlobals::sleep();

    bool offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == false, "Offsets are visible");


    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets"));

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == true, "Offsets are not visible");
}

GUI_TEST_CLASS_DEFINITION(test_0002_3) {
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "revcompl.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "View" << "Show offsets");
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

    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "View" << "Show offsets");
    GTGlobals::sleep();
    GTGlobals::sleep();

    offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == true, "Offsets are not visible");
}

GUI_TEST_CLASS_DEFINITION(test_0002_4) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "revcompl.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets"));

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

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets"));

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == true, "Offsets are not visible");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "action_sort_by_name"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os);
}

GUI_TEST_CLASS_DEFINITION(test_0003_1) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "View" << "Sort sequences by name");
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os);
}

GUI_TEST_CLASS_DEFINITION(test_0003_2) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "revcompl.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "action_sort_by_name"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "action_sort_by_name"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os);
}

GUI_TEST_CLASS_DEFINITION(test_0003_3) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gap_col.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os, false);

    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "View" << "Sort sequences by name");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets"));

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Maximize);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0003_4) {
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSorted(os, false);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "action_sort_by_name"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep(1000);
#ifdef Q_OS_MAC
    GTMouseDriver::click(os);
    GTGlobals::sleep(1000);
#endif
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
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 6));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_go_to_position"));

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();
    GTGlobals::sleep();

    QRect expectedRect(5, 0, 1, 1);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, expectedRect);

    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 6));
    GTKeyboardDriver::keyClick(os, 'g', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, expectedRect);
}

GUI_TEST_CLASS_DEFINITION(test_0004_1)
{
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 6));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_go_to_position"));

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();
    GTGlobals::sleep();

    QRect expectedRect(5, 0, 1, 1);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, expectedRect);

    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 6));
    GTKeyboardDriver::keyClick(os, 'g', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, expectedRect);
}

GUI_TEST_CLASS_DEFINITION(test_0004_2)
{
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 6));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_go_to_position"));

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();
    GTGlobals::sleep();

    QRect expectedRect(5, 0, 1, 1);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, expectedRect);

    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 6));
    GTKeyboardDriver::keyClick(os, 'g', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, expectedRect);
}

GUI_TEST_CLASS_DEFINITION(test_0005) {
// Check maligniment view status bar coordinates

// 1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

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
    GTUtilsTaskTreeView::waitTaskFinished(os);

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
    GTUtilsTaskTreeView::waitTaskFinished(os);

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
    GTUtilsTaskTreeView::waitTaskFinished(os);

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
    GTUtilsTaskTreeView::waitTaskFinished(os);

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
    GTUtilsTaskTreeView::waitTaskFinished(os);

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
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    //Expected state: Aligniment length 14, left offset 1, right offset 14

    //2. Do double click on Tettigonia_viridissima sequence name.
    //Expected state: Rename dialog appears
    //3. Put "Sequence_a" into text field. Click OK.

    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Sequence_a", "Tettigonia_viridissima"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed to Sequence_a
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Sequence_a", "Sequence_a"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //4. Rlick Undo button.
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    GTWidget::click(os,undo);
    //GTKeyboardDriver::keyClick(os, 'z', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed back
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Tettigonia_viridissima", "Tettigonia_viridissima"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0007_1) {
    //1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    //Expected state: Aligniment length 14, left offset 1, right offset 14

    //2. Do double click on Tettigonia_viridissima sequence name.
    //Expected state: Rename dialog appears
    //3. Put "Sequence_a" into text field. Click OK.

    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Sequence_a", "Tettigonia_viridissima"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed to Sequence_a

    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Sequence_a", "Sequence_a"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //4. Rlick Undo button. CHANGES: clicking undo by mouse
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "msa_action_undo"));
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed back
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Tettigonia_viridissima", "Tettigonia_viridissima"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0007_2) {
    //1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    //Expected state: Aligniment length 14, left offset 1, right offset 14

    //2. Do double click on Tettigonia_viridissima sequence name. CHANGES: another sequence renamed
    //Expected state: Rename dialog appears
    //3. Put "Sequence_a" into text field. Click OK.

    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Sequence_a", "Bicolorana_bicolor_EF540830"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,2));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed to Sequence_a
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Sequence_a", "Sequence_a"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,2));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //4. Rlick Undo button.
    GTKeyboardDriver::keyClick(os, 'z', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed back
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Bicolorana_bicolor_EF540830", "Bicolorana_bicolor_EF540830"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,2));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0007_3) {
    //1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    //Expected state: Aligniment length 14, left offset 1, right offset 14

    //2. Do double click on Tettigonia_viridissima sequence name. CHANGES: another sequence renamed
    //Expected state: Rename dialog appears
    //3. Put "Sequence_a" into text field. Click OK.

    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Sequence_a", "Phaneroptera_falcata"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,0));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed to Sequence_a
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Sequence_a", "Sequence_a"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,0));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //4. Rlick Undo button.
    GTKeyboardDriver::keyClick(os, 'z', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed back
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Phaneroptera_falcata", "Phaneroptera_falcata"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,0));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0007_4) {
    //1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    //Expected state: Aligniment length 14, left offset 1, right offset 14

    //2. Do double click on Tettigonia_viridissima sequence name. CHANGES: another sequence renamed
    //Expected state: Rename dialog appears
    //3. Put "Sequence_a" into text field. Click OK.

    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Sequence_a", "Conocephalus_sp."));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,5));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed to Sequence_a
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Sequence_a", "Sequence_a"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,5));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    //4. Rlick Undo button.
    GTKeyboardDriver::keyClick(os, 'z', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    //Expected state: Tettigonia_viridissima renamed back
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Conocephalus_sp.", "Conocephalus_sp."));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,5));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0008) {

//     1. Open document samples\CLUSTALW\COI.aln

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

//     2. Create bookmark. Rename "New bookmark" to "start bookmark"
    QPoint p = GTUtilsBookmarksTreeView::getItemCenter(os, "COI [m] COI");
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseKey));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keySequence(os, "start bookmark");
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep();

    int startRO = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    int startLO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);

//     3. Scroll msa to the middle.
    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 300));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_go_to_position"));

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();

//     4. Create bookmark. Rename "New bookmark" to "middle bookmark"
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseKey));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keySequence(os, "middle bookmark");
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep();

    int midRO = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    int midLO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
//
//     5. Scroll msa to the end.
    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 550));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_go_to_position"));

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();
//     6. Create bookmark. Rename "New bookmark" to "end bookmark"
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseKey));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keySequence(os, "end bookmark");
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep();

    int endRO = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    int endLO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);

//     Expected state: clicking on each bookmark will recall corresponding MSA position
    p = GTUtilsBookmarksTreeView::getItemCenter(os, "start bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    int RO = GTUtilsMSAEditorSequenceArea::getRightOffset(os), LO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(startRO == RO && startLO == LO, "start bookmark offsets doesnt equal");

    p = GTUtilsBookmarksTreeView::getItemCenter(os, "middle bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    RO = GTUtilsMSAEditorSequenceArea::getRightOffset(os), LO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(midRO == RO && midLO == LO, "mid bookmark offsets doesnt equal");

    p = GTUtilsBookmarksTreeView::getItemCenter(os, "end bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    RO = GTUtilsMSAEditorSequenceArea::getRightOffset(os), LO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(endRO == RO && endLO == LO, "end bookmark offsets doesnt equal");

//     7. Delete Start bookmark
    p = GTUtilsBookmarksTreeView::getItemCenter(os, "start bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(500);

//     Expected state: start bookmark doesn't present
    QTreeWidgetItem* start = GTUtilsBookmarksTreeView::findItem(os,"start bookmark", GTGlobals::FindOptions(false));
    CHECK_SET_ERR(start==NULL, "Start bookmark not deleted");
}

GUI_TEST_CLASS_DEFINITION(test_0008_1) {  //CHANGES: default names used

    //     1. Open document samples\CLUSTALW\COI.aln

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    //     2. Create bookmark. Rename "New bookmark" to "start bookmark"
    QPoint p = GTUtilsBookmarksTreeView::getItemCenter(os, "COI [m] COI");
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseKey));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep();

    int startRO = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    int startLO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);

    //     3. Scroll msa to the middle.
    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 300));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_go_to_position"));

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();

    //     4. Create bookmark. Rename "New bookmark" to "middle bookmark"
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseKey));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep();

    int midRO = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    int midLO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    //
    //     5. Scroll msa to the end.
    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 550));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_go_to_position"));

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();
    //     6. Create bookmark. Rename "New bookmark" to "end bookmark"
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseKey));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep();

    int endRO = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    int endLO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);

    //     Expected state: clicking on each bookmark will recall corresponding MSA position
    p = GTUtilsBookmarksTreeView::getItemCenter(os, "New bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    int RO = GTUtilsMSAEditorSequenceArea::getRightOffset(os), LO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(startRO == RO && startLO == LO, "start bookmark offsets doesnt equal");

    p = GTUtilsBookmarksTreeView::getItemCenter(os, "New bookmark 2");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    RO = GTUtilsMSAEditorSequenceArea::getRightOffset(os), LO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(midRO == RO && midLO == LO, "mid bookmark offsets doesnt equal");

    p = GTUtilsBookmarksTreeView::getItemCenter(os, "New bookmark 3");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    RO = GTUtilsMSAEditorSequenceArea::getRightOffset(os), LO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(endRO == RO && endLO == LO, "end bookmark offsets doesnt equal");

//     7. Delete Start bookmark
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ACTION_REMOVE_BOOKMARK));
    p = GTUtilsBookmarksTreeView::getItemCenter(os, "New bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::click(os,Qt::RightButton);
    GTGlobals::sleep(500);

//     Expected state: start bookmark doesn't present
    QTreeWidgetItem* start = GTUtilsBookmarksTreeView::findItem(os,"New bookmark", GTGlobals::FindOptions(false));
    CHECK_SET_ERR(start==NULL, "bookmark not deleted");
}

GUI_TEST_CLASS_DEFINITION(test_0008_2) { //CHANGES: mid and end coordinates changed

    //     1. Open document samples\CLUSTALW\COI.aln

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    //     2. Create bookmark. Rename "New bookmark" to "start bookmark"
    QPoint p = GTUtilsBookmarksTreeView::getItemCenter(os, "COI [m] COI");
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseKey));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keySequence(os, "start bookmark");
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep();

    int startRO = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    int startLO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);

    //     3. Scroll msa to the middle.
    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 200));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_go_to_position"));

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();

    //     4. Create bookmark. Rename "New bookmark" to "middle bookmark"
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseKey));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keySequence(os, "middle bookmark");
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep();

    int midRO = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    int midLO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    //
    //     5. Scroll msa to the end.
    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 510));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_go_to_position"));

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();
    //     6. Create bookmark. Rename "New bookmark" to "end bookmark"
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseKey));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keySequence(os, "end bookmark");
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep();

    int endRO = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    int endLO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);

    //     Expected state: clicking on each bookmark will recall corresponding MSA position
    p = GTUtilsBookmarksTreeView::getItemCenter(os, "start bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    int RO = GTUtilsMSAEditorSequenceArea::getRightOffset(os), LO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(startRO == RO && startLO == LO, "start bookmark offsets doesnt equal");

    p = GTUtilsBookmarksTreeView::getItemCenter(os, "middle bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    RO = GTUtilsMSAEditorSequenceArea::getRightOffset(os), LO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(midRO == RO && midLO == LO, "mid bookmark offsets doesnt equal");

    p = GTUtilsBookmarksTreeView::getItemCenter(os, "end bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    RO = GTUtilsMSAEditorSequenceArea::getRightOffset(os), LO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(endRO == RO && endLO == LO, "end bookmark offsets doesnt equal");
}

GUI_TEST_CLASS_DEFINITION(test_0008_3) { //CHANGES: mid and end coordinates changed, used another file

    //     1. Open document samples\CLUSTALW\COI.aln

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "HIV-1.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    //     2. Create bookmark. Rename "New bookmark" to "start bookmark"
    QPoint p = GTUtilsBookmarksTreeView::getItemCenter(os, "HIV-1 [m] HIV-1");
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseKey));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keySequence(os, "start bookmark");
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep();

    int startRO = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    int startLO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);

    //     3. Scroll msa to the middle.
    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 600));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_go_to_position"));

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();

    //     4. Create bookmark. Rename "New bookmark" to "middle bookmark"
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseKey));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keySequence(os, "middle bookmark");
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep();

    int midRO = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    int midLO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    //
    //     5. Scroll msa to the end.
    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 1000));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_go_to_position"));

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();
    //     6. Create bookmark. Rename "New bookmark" to "end bookmark"
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseKey));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();
    GTKeyboardDriver::keySequence(os, "end bookmark");
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep();

    int endRO = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    int endLO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);

    //     Expected state: clicking on each bookmark will recall corresponding MSA position
    p = GTUtilsBookmarksTreeView::getItemCenter(os, "start bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    int RO = GTUtilsMSAEditorSequenceArea::getRightOffset(os), LO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(startRO == RO && startLO == LO, "start bookmark offsets doesnt equal");

    p = GTUtilsBookmarksTreeView::getItemCenter(os, "middle bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    RO = GTUtilsMSAEditorSequenceArea::getRightOffset(os), LO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(midRO == RO && midLO == LO, "mid bookmark offsets doesnt equal");

    p = GTUtilsBookmarksTreeView::getItemCenter(os, "end bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    RO = GTUtilsMSAEditorSequenceArea::getRightOffset(os), LO = GTUtilsMSAEditorSequenceArea::getLeftOffset(os);
    CHECK_SET_ERR(endRO == RO && endLO == LO, "end bookmark offsets doesnt equal");
}

GUI_TEST_CLASS_DEFINITION(test_0009) {

    //1. Open ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    //2. Select a trailing region length=3 (all gaps) for Isophia_altiacaEF540820
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(11, 1), QPoint(13, 1));
    GTGlobals::sleep();

    //3. Do context menu {Align-> Align with MUSCLE}  use "column range"
    GTUtilsDialog::waitForDialog(os, new MuscleDialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with muscle", GTGlobals::UseKey));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();
    //GTGlobals::sleep();

    //Expected state: Column range = 12-14
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(11, 0), QPoint(13, 9));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();


    QString clipboardText = GTClipboard::text(os);
    QString expectedMSA = "TAA\n---\nTAA\nTAA\n---\n---\n---\nTAA\nTTA\n---";


    CHECK_SET_ERR(clipboardText == expectedMSA, "Clipboard string and expected MSA string differs\n"+clipboardText);

    GTGlobals::sleep();

    //4. Press Align
    //Expected state: After aligning with 'stable' option the order must not change
}

GUI_TEST_CLASS_DEFINITION(test_0009_1) {

    //1. Open ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    //2. Select a trailing region length=3 (all gaps) for Isophia_altiacaEF540820
    //CHANGES: selection from right to left
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(13, 1), QPoint(11, 1));
    GTGlobals::sleep();

    //3. Do context menu {Align-> Align with MUSCLE}  use "column range"
    GTUtilsDialog::waitForDialog(os, new MuscleDialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with muscle", GTGlobals::UseKey));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();
    //GTGlobals::sleep();

    //Expected state: Column range = 12-14
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(11, 0), QPoint(13, 9));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    QString clipboardText = GTClipboard::text(os);
    QString expectedMSA = "TAA\n---\nTAA\nTAA\n---\n---\n---\nTAA\nTTA\n---";

    CHECK_SET_ERR(clipboardText == expectedMSA, "Clipboard string and expected MSA string differs");

    GTGlobals::sleep();

    //4. Press Align
    //Expected state: After aligning with 'stable' option the order must not change
}

GUI_TEST_CLASS_DEFINITION(test_0009_2) {

    //1. Open ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    //2. Select a trailing region length=3 (all gaps) for Isophia_altiacaEF540820
    //CHANGES: another region selected
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(11, 4), QPoint(13, 4));
    GTGlobals::sleep();

    //3. Do context menu {Align-> Align with MUSCLE}  use "column range"
    GTUtilsDialog::waitForDialog(os, new MuscleDialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with muscle", GTGlobals::UseKey));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();
    //GTGlobals::sleep();

    //Expected state: Column range = 12-14
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(11, 0), QPoint(13, 9));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    QString clipboardText = GTClipboard::text(os);
    QString expectedMSA = "TAA\n---\nTAA\nTAA\n---\n---\n---\nTAA\nTTA\n---";

    CHECK_SET_ERR(clipboardText == expectedMSA, "Clipboard string and expected MSA string differs");

    GTGlobals::sleep();

    //4. Press Align
    //Expected state: After aligning with 'stable' option the order must not change
}

GUI_TEST_CLASS_DEFINITION(test_0010) {

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Do document context menu {Export->Export aligniment to amino format}
// 3. Translate with default settings
    GTUtilsDialog::waitForDialog(os, new ExportMSA2MSADialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows"));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::selectArea(os);
    GTGlobals::sleep();
// copy to clipboard
//    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_COPY" << "copy_selection"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: every sequense name the same as it amino translation
    QString clipboardText = GTClipboard::text(os);
    QString expectedMSA = "L\nS\nD\nS\nP\nK";

    CHECK_SET_ERR(clipboardText == expectedMSA, clipboardText);

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0010_1) {

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Do document context menu {Export->Export aligniment to amino format}
// 3. Translate with default settings
    GTUtilsDialog::waitForDialog(os, new ExportMSA2MSADialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows"));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::selectArea(os);
// copy to clipboard
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection"));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: every sequense name the same as it amino translation
    QString clipboardText = GTClipboard::text(os);
    QString expectedMSA = "L\nS\nD\nS\nP\nK";

    CHECK_SET_ERR(clipboardText == expectedMSA, "Clipboard string and expected MSA string differs");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0010_2) {

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Do document context menu {Export->Export aligniment to amino format}
// 3. Translate with default settings
    GTUtilsDialog::waitForDialog(os, new ExportMSA2MSADialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows"));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::selectArea(os);
// copy to clipboard
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection"));
    GTMouseDriver::click(os, Qt::RightButton);

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
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Select first sequence and do context menu {Edit->Replace selected rows with reverce complement}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement"));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(-1, 0));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: sequence changed from TTG -> CAA
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep();
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "CAA", "Clipboard string and expected MSA string differs");

//                 sequence name  changed from L -> L|revcompl
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() >= 2, "nameList doesn't contain enough strings");
    CHECK_SET_ERR(nameList[0] == "L|revcompl", "There are no 'L|revcompl' in nameList");

// 3. Do step 2 again
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement"));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: sequence changed from CAA -> TTG
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep();
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "TTG", "Clipboard string and expected MSA string differs");

//                 sequence name changed from L|revcompl ->
    nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() >= 2, "nameList doesn't contain enough strings");
    CHECK_SET_ERR(!nameList.contains("L|revcompl"), "There are 'L|revcompl' in nameList");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0011_1) {
// In-place reverse complement replace in MSA Editor (0002425)

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Select first sequence and do context menu {Edit->Replace selected rows with reverce complement}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement"));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(-1, 0));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: sequence changed from TTG -> CAA
// CHANGES: copy by context menu
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "CAA", "Clipboard string and expected MSA string differs");

//                 sequence name  changed from L -> L|revcompl
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() >= 2, "nameList doesn't contain enough strings");
    CHECK_SET_ERR(nameList[0] == "L|revcompl" , "There are no 'L|revcompl' in nameList");

// 3. Do step 2 again
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement"));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: sequence changed from CAA -> TTG
    GTGlobals::sleep();
// CHANGES: copy by context menu
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "TTG", "Clipboard string and expected MSA string differs");

//                 sequence name changed from L|revcompl ->
    nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() >= 2, "nameList doesn't contain enough strings");
    CHECK_SET_ERR(!nameList.contains("L|revcompl"), "There are 'L|revcompl' in nameList");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0011_2) {
// In-place reverse complement replace in MSA Editor (0002425)

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Select first sequence and do context menu {Edit->Replace selected rows with reverce complement}
// CHANGES: using main menu
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(-1, 0));
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Edit" << "Replace selected rows with reverse-complement");
    GTGlobals::sleep();
    //GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(-1, 0));
// Expected state: sequence changed from TTG -> CAA
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "CAA", "Clipboard string and expected MSA string differs"+clipboardText);

//                 sequence name  changed from L -> L|revcompl
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() >= 2, "nameList doesn't contain enough strings");
    CHECK_SET_ERR(nameList[0] == "L|revcompl" , "There are no 'L|revcompl' in nameList");

// 3. Do step 2 again
// CHANGES: using main menu
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Edit" << "Replace selected rows with reverse-complement");
    GTGlobals::sleep();

// Expected state: sequence changed from CAA -> TTG
    //GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(-1, 0));
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep();
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "TTG", "Clipboard string and expected MSA string differs");

//                 sequence name changed from L|revcompl ->
    nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() >= 2, "nameList doesn't contain enough strings");
    CHECK_SET_ERR(!nameList.contains("L|revcompl"), "There are 'L|revcompl' in nameList");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0012) {
// Add tests on alignment translation features (0002432)

// 1. Open file _common_data\scenarios\msa\revcompl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "revcompl.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Select all sequences and do context menu {Edit->Replace selected rows with reverce complement}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement"));

    GTUtilsMSAEditorSequenceArea::selectArea(os);
    GTMouseDriver::click(os, Qt::RightButton);
    GTWidget::click(os,GTUtilsMdi::activeWindow(os));

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
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Select all sequences and do context menu {Edit->Replace selected rows with reverce complement}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement"));

    GTUtilsMSAEditorSequenceArea::selectArea(os);
    GTMouseDriver::click(os, Qt::RightButton);
    GTWidget::click(os,GTUtilsMdi::activeWindow(os));

// Expected state: result alignement must be
// CAA---
// --TGA-
// ---ATC

    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, -1), QPoint(-1, 0));
// copy to clipboard. CHANGES: copy by context menu
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection"));
    GTMouseDriver::click(os, Qt::RightButton);

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
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Select all sequences and do context menu {Edit->Replace selected rows with reverce complement}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement"));

    GTUtilsMSAEditorSequenceArea::selectArea(os);
    GTMouseDriver::click(os, Qt::RightButton);

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
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "copy_selection"));
    GTMouseDriver::click(os, Qt::RightButton);

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
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Convert alignment to amino. Use context menu {Export->Amino translation of alignment rows}
    GTUtilsDialog::waitForDialog(os, new ExportMSA2MSADialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows"));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();

// 3. Open converted alignment. Use context menu {Align->Align with Kalign}
    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign"));
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
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Convert alignment to amino. Use context menu {Export->Amino translation of alignment rows}
    GTUtilsDialog::waitForDialog(os, new ExportMSA2MSADialogFiller(os, "CLUSTALW", testDir + "_common_data/scenarios/sandbox/COI_transl.aln"));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows"));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();

// CHANGES: close and open MDI window
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI_transl.aln"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();


// 3. Open converted alignment. Use context menu {Align->Align with Kalign}
    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign"));
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
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Convert alignment to amino. Use context menu {Export->Amino translation of alignment rows}
    GTUtilsDialog::waitForDialog(os, new ExportMSA2MSADialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows"));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTGlobals::sleep();

// 3. Open converted alignment. Use context menu {Align->Align with Kalign}
    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));

// CHANGES: using main menu
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Align" << "Align with Kalign...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

// Expected state: UGENE not crash
}

GUI_TEST_CLASS_DEFINITION(test_0014) {
// UGENE crashes in malignment editor after aligning (UGENE-6)

// 1. Do menu tools->multiple alignment->kalign, set input alignment "data/samples/CLUSTALW/COI.aln" and press Align button
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign"));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();
    GTGlobals::sleep();

// 2. after kalign finishes and msa opens insert gaps and click in alignment
    GTGlobals::sleep(5000);

    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep();

    GTMouseDriver::click(os);

// Expected state: UGENE not crash
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0014_1) {
// UGENE crashes in malignment editor after aligning (UGENE-6)

// 1. Do menu tools->multiple alignment->kalign, set input alignment "data/samples/CLUSTALW/COI.aln" and press Align button
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));

// CHANGES: using main menu
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Align" << "Align with Kalign...");
    GTGlobals::sleep();

// 2. after kalign finishes and msa opens insert gaps and click in alignment
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep();

    GTMouseDriver::click(os);

// Expected state: UGENE not crash
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0014_2) {
// UGENE crashes in malignment editor after aligning (UGENE-6)

// 1. Do menu tools->multiple alignment->kalign, set input alignment "data/samples/CLUSTALW/COI.aln" and press Align button
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

// CHANGES: close and open MDI window, close Project tree view
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep();
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
    GTUtilsProjectTreeView::toggleView(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign"));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();
    GTGlobals::sleep();

// 2. after kalign finishes and msa opens insert gaps and click in alignment
    GTGlobals::sleep(5000);

    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep();

    GTMouseDriver::click(os);

// Expected state: UGENE not crash
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0015) {
// ugene crashes when removing document after kalign (UGENE-36)
//
// 1. create empty project
// 2. do menu {tools->multiple alignment->kalign}, set aligned document samples/CLUSTALW/COI.aln

    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/CLUSTALW/", "COI.aln"));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Multiple sequence alignment" << "Align with Kalign...");
    GTGlobals::sleep();

// 3. aligned document opens
    GTGlobals::sleep(5000);
    GTUtilsMdi::activeWindow(os);

// 4. select document in project and press del
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI.aln"));
    GTMouseDriver::click(os);
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
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();

// 3. aligned document opens
    GTGlobals::sleep(5000);
    GTUtilsMdi::activeWindow(os);

// 4. select document in project and press del
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI.aln"));
    GTMouseDriver::click(os);
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

    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/CLUSTALW/", "COI.aln"));
    GTMenu::clickMainMenuItem(os, QStringList() << "Tools" << "Multiple sequence alignment" << "Align with Kalign...");
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
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

// Expected state: UGENE not crash
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0016) {

// 1. Run Ugene. Open file _common_data\scenarios\msa\ma2_gapped.aln
    GTFile::copy(os, testDir + "_common_data/scenarios/msa/ma2_gapped.aln", sandBoxDir + "ma2_gapped.aln");
    GTFile::copy(os, testDir + "_common_data/scenarios/msa/ma2_gapped_edited.aln", sandBoxDir + "ma2_gapped_edited.aln");
    GTFileDialog::openFile(os, sandBoxDir, "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

// 2. Open same file in text editor. Change first 3 bases of 'Phaneroptera_falcata'
//    from 'AAG' to 'CTT' and save file.
//CHANGES: backup old file, copy changed file
//    GTFile::backup(os, testDir + "_common_data/scenarios/msa/ma2_gapped.aln");
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
    GTFile::copy(os, sandBoxDir + "ma2_gapped.aln", sandBoxDir + "ma2_gapped_old.aln");
    GTFile::copy(os, sandBoxDir + "ma2_gapped_edited.aln", sandBoxDir + "ma2_gapped.aln");

//    Expected state: Dialog suggesting to reload modified document has appeared.
// 3. Press 'Yes'.
    GTGlobals::sleep(10000);

//    Expected state: document was reloaded, view activated.
//    'Phaneroptera_falcata' starts with CTT.
    GTGlobals::sleep();
    GTUtilsMdi::activeWindow(os);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(2, 0));
// copy to clipboard
//    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTUtilsDialog::waitForDialog(os,new PopupChooser(os,QStringList()<<MSAE_MENU_COPY<<"copy_selection"));
    GTMouseDriver::click(os,Qt::RightButton);
    GTGlobals::sleep();

    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "CTT", "MSA part differs from expected");
    GTGlobals::sleep(3000);
}

GUI_TEST_CLASS_DEFINITION(test_0016_1) {
// 1. Run Ugene. Open file _common_data\scenarios\msa\ma2_gapped.aln
    GTFile::copy(os, testDir + "_common_data/scenarios/msa/ma2_gapped.aln", sandBoxDir + "ma2_gapped.aln");
    GTFile::copy(os, testDir + "_common_data/scenarios/msa/ma2_gapped_edited.aln", sandBoxDir + "ma2_gapped_edited.aln");
    GTFileDialog::openFile(os, sandBoxDir, "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

// CHANGES: insert gaps in the beginning
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(0, 0));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep(200);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep(200);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep();

// 2. Open same file in text editor. Change first 3 bases of 'Phaneroptera_falcata'
//    from 'AAG' to 'CTT' and save file.
//CHANGES: backup old file, copy changed file
    //GTFile::backup(os, testDir + "_common_data/scenarios/msa/ma2_gapped.aln");
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
    GTFile::copy(os, sandBoxDir + "ma2_gapped.aln", sandBoxDir + "ma2_gapped_old.aln");
    GTFile::copy(os, sandBoxDir + "ma2_gapped_edited.aln", sandBoxDir + "ma2_gapped.aln");
//    Expected state: Dialog suggesting to reload modified document has appeared.
// 3. Press 'Yes'.
    GTGlobals::sleep(10000);

//    Expected state: document was reloaded, view activated.
//    'Phaneroptera_falcata' starts with CTT.
    GTUtilsMdi::activeWindow(os);

    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(2, 0));
    GTGlobals::sleep();
// copy to clipboard
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    QString clipboardText = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardText == "CTT", "MSA part differs from expected. Expected: CTT, actual: " + clipboardText);
    GTGlobals::sleep(3000);
}

GUI_TEST_CLASS_DEFINITION(test_0016_2) {
// 1. Run Ugene. Open file _common_data\scenarios\msa\ma2_gapped.aln
    GTFile::copy(os, testDir + "_common_data/scenarios/msa/ma2_gapped.aln", sandBoxDir + "ma2_gapped.aln");
    GTFile::copy(os, testDir + "_common_data/scenarios/msa/ma2_gapped_edited.aln", sandBoxDir + "ma2_gapped_edited.aln");
    GTFileDialog::openFile(os, sandBoxDir, "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

// 2. Open same file in text editor. Change first 3 bases of 'Phaneroptera_falcata'
//    from 'AAG' to 'CTT' and save file.
//CHANGES: backup old file, copy changed file
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
    GTFile::copy(os, sandBoxDir + "ma2_gapped.aln", sandBoxDir + "ma2_gapped_old.aln");
    GTFile::copy(os, sandBoxDir + "ma2_gapped_edited.aln", sandBoxDir + "ma2_gapped.aln");

//    Expected state: Dialog suggesting to reload modified document has appeared.
// 3. Press 'Yes'.
    GTGlobals::sleep(10000);

//    Expected state: document was reloaded, view activated.
//    'Phaneroptera_falcata' starts with CTT.
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
}

GUI_TEST_CLASS_DEFINITION(test_0017) {
// Add a molecule from project  (UGENE-288)
//
// 1. Open file data/samples/Genbank/murine.gb
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

// 2. Open file data/samples/MSF/HMA.msf
    GTFileDialog::openFile(os, dataDir + "samples/MSF/", "HMA.msf");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

// 3. On MSA editor {Context Menu->Add->Sequence from current project}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_LOAD << "Sequence from current project"));

// 4. Select item dialog appeared
// Expected state: loaded sequences present in list
    GTUtilsDialog::waitForDialog(os, new ProjectTreeItemSelectorDialogFiller(os, "murine.gb", "NC_001363"));

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0017_1) {
// Add a molecule from project  (UGENE-288)
//
// 1. Open file data/samples/Genbank/murine.gb
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

// 2. Open file data/samples/MSF/HMA.msf
    GTFileDialog::openFile(os, dataDir + "samples/MSF/", "HMA.msf");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

// 3. On MSA editor {Context Menu->Add->Sequence from current project}
// 4. Select item dialog appeared
// Expected state: loaded sequences present in list
    GTUtilsDialog::waitForDialog(os, new ProjectTreeItemSelectorDialogFiller(os, "murine.gb", "NC_001363"));

// CHANGES: using main menu instead of popup
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Add" << "Sequence from current project...");
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0017_2) {
// Add a molecule from project  (UGENE-288)
//
// 1. Open file data/samples/Genbank/murine.gb
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

// CHANGES: close MDI window of murine.gb
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep();

// 2. Open file data/samples/MSF/HMA.msf
    GTFileDialog::openFile(os, dataDir + "samples/MSF/", "HMA.msf");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

// 3. On MSA editor {Context Menu->Add->Sequence from current project}
// 4. Select item dialog appeared
// Expected state: loaded sequences present in list
    GTUtilsDialog::waitForDialog(os, new ProjectTreeItemSelectorDialogFiller(os, "murine.gb", "NC_001363"));

// CHANGES: using main menu instead of popup
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Add" << "Sequence from current project...");
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0018) {
// Shifting sequences in the Alignment Editor (UGENE-238)
//
// 1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Click on some row in sequence names area
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-10, 2));
// Expected state: row became selected
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 2, 604, 1));

// 3. Click & drag selected row in sequence names area
    QStringList list1 = GTUtilsMSAEditorSequenceArea::getNameList(os);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(-10, 2), QPoint(-10, 3));
// Expected state: row order changes respectively
    QStringList list2 = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(list1 != list2, "Name list wasn't changed");

// 4. Click & drag on unselected area
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(-10, 0), QPoint(-9, 1));
// Expected state: multiple rows selected
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 0, 604, 2));

// 5. Click & drag selected block
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(-10, 0), QPoint(-9, 1));
// Expected state: whole selected block shifted
    QStringList list3 = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(list2 != list3, "Name list wasn't changed");

// 6. Click on some row in selected block
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-9, 1));
// Expected state: selection falls back to one row
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 1, 604, 1));
}

GUI_TEST_CLASS_DEFINITION(test_0018_1) {
// Shifting sequences in the Alignment Editor (UGENE-238)
//
// 1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Click on some row in sequence names area
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-10, 2));
// Expected state: row became selected
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 2, 604, 1));

// 3. Click & drag selected row in sequence names area
    QStringList list1 = GTUtilsMSAEditorSequenceArea::getNameList(os);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(-10, 2), QPoint(-10, 3));
// Expected state: row order changes respectively
    QStringList list2 = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(list1 != list2, "Name list wasn't changed");

// 4. Click & drag on unselected area
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(-10, 0), QPoint(-9, 1));
// Expected state: multiple rows selected
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 0, 604, 2));

// 5. Click & drag selected block
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(-10, 0), QPoint(-9, 1));
// Expected state: whole selected block shifted
    QStringList list3 = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(list2 != list3, "Name list wasn't changed");

// 6. Click on some row in selected block
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-9, 1));
// Expected state: selection falls back to one row
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 1, 604, 1));
}

GUI_TEST_CLASS_DEFINITION(test_0018_2) {
// Shifting sequences in the Alignment Editor (UGENE-238)
//
// 1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Click on some row in sequence names area
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-10, 2));
// Expected state: row became selected
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 2, 604, 1));

// 3. Click & drag selected row in sequence names area
    QStringList list1 = GTUtilsMSAEditorSequenceArea::getNameList(os);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(-10, 2), QPoint(-10, 3));
// Expected state: row order changes respectively
    QStringList list2 = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(list1 != list2, "Name list wasn't changed");

// 4. Click & drag on unselected area
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(-10, 0), QPoint(-9, 1));
// Expected state: multiple rows selected
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 0, 604, 2));

// 5. Click & drag selected block
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(-10, 0), QPoint(-9, 1));
// Expected state: whole selected block shifted
    QStringList list3 = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(list2 != list3, "Name list wasn't changed");

// 6. Click on some row in selected block
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-9, 1));
// Expected state: selection falls back to one row
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 1, 604, 1));
}

GUI_TEST_CLASS_DEFINITION(test_0019) {
// UGENE-79 In MSA editor support rows collapsing mode
//
// 1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QStringList preList = GTUtilsMSAEditorSequenceArea::getVisibaleNames(os);
// 2. Press button Enable collapsing
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Enable collapsing"));

// Expected state: Mecopoda_elongata__Ishigaki__J and Mecopoda_elongata__Sumatra_ folded together
    QStringList postList = GTUtilsMSAEditorSequenceArea::getVisibaleNames(os);
    CHECK_SET_ERR(preList.size() == postList.size() + 1, "Name lists differs not by 1");
}

GUI_TEST_CLASS_DEFINITION(test_0019_1) {
// UGENE-79 In MSA editor support rows collapsing mode
//
// 1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QStringList preList = GTUtilsMSAEditorSequenceArea::getVisibaleNames(os);
// 2. Press button Enable collapsing
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Enable collapsing"));

// Expected state: Mecopoda_elongata__Ishigaki__J and Mecopoda_elongata__Sumatra_ folded together
    QStringList postList = GTUtilsMSAEditorSequenceArea::getVisibaleNames(os);
    CHECK_SET_ERR(preList.size() == postList.size() + 1, "Name lists differs not by 1");
}

GUI_TEST_CLASS_DEFINITION(test_0019_2) {
// UGENE-79 In MSA editor support rows collapsing mode
//
// 1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QStringList preList = GTUtilsMSAEditorSequenceArea::getVisibaleNames(os);
// 2. Press button Enable collapsing
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Enable collapsing"));

// Expected state: Mecopoda_elongata__Ishigaki__J and Mecopoda_elongata__Sumatra_ folded together
    QStringList postList = GTUtilsMSAEditorSequenceArea::getVisibaleNames(os);
    CHECK_SET_ERR(preList.size() == postList.size() + 1, "Name lists differs not by 1");
}

GUI_TEST_CLASS_DEFINITION(test_0020) {
// UGENE crashes when all columns in MSAEditor are deleted (UGENE-329)
//
// 1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
// 2. Select Edit -> remove columns of gaps -> remove columns with number of gaps 1.
// 3. Click OK
    GTUtilsDialog::waitForDialog(os, new DeleteGapsDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_EDIT" << "remove_columns_of_gaps"));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: UGENE not crashes, deletion is not performed
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0),QPoint(0,9));
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os,'c',GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString text = GTClipboard::text(os);
    QString expected = "A\nA\nT\nA\nT\nT\nT\nA\nA\nA";
    CHECK_SET_ERR(text == expected, "expected: " + expected + "found: " + text);
}

GUI_TEST_CLASS_DEFINITION(test_0020_1) {
// UGENE crashes when all columns in MSAEditor are deleted (UGENE-329)
//
// 1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

// 2. Insert some gaps to the first column. Ensure, that every column has a gap
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(0, 0));
    GTGlobals::sleep();
    for (int i=0; i<6; i++) {
        GTKeyboardDriver::keyClick(os, ' ');
        GTGlobals::sleep(100);
    }
// 3. Select Edit -> remove columns of gaps -> remove columns with number of gaps 1.
    GTWidget::click(os,GTWidget::findWidget(os, "msa_editor_sequence_area"));
    GTUtilsMSAEditorSequenceArea::selectArea(os,QPoint(0,0),QPoint(19,9));
    GTKeyboardDriver::keyClick(os,'c',GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    QString initial = GTClipboard::text(os);
// 4. Click OK
    GTUtilsDialog::waitForDialog(os, new DeleteGapsDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_EDIT" << "remove_columns_of_gaps"));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: UGENE not crashes, deletion is not performed
    GTWidget::click(os,GTWidget::findWidget(os, "msa_editor_sequence_area"));
    GTGlobals::sleep(500);

    GTUtilsMSAEditorSequenceArea::selectArea(os,QPoint(0,0),QPoint(19,9));
    GTKeyboardDriver::keyClick(os,'c',GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    QString final = GTClipboard::text(os);

    CHECK_SET_ERR(initial == final, "msa area was changed");
}

GUI_TEST_CLASS_DEFINITION(test_0021) {
// MSA editor zoom bug (UGENE-520)
//
// 1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

// 2. zoom MSA to maximum
    for (int i=0; i<8; i++) {
        GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Zoom In"));
    }

// Expected state: top sequence not overlaps with ruler
    GTGlobals::sleep();

    for (int i=0; i<8; i++) {
        GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Zoom Out"));
    }
}

GUI_TEST_CLASS_DEFINITION(test_0021_1) {
// MSA editor zoom bug (UGENE-520)
//
// 1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

// 2. zoom MSA to maximum
    for (int i=0; i<8; i++) {
        GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Zoom In"));
    }

// Expected state: top sequence not overlaps with ruler
    GTGlobals::sleep();

    for (int i=0; i<8; i++) {
        GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Zoom Out"));
    }
}

GUI_TEST_CLASS_DEFINITION(test_0021_2) {
// MSA editor zoom bug (UGENE-520)
//
// 1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

// 2. zoom MSA to maximum
    for (int i=0; i<8; i++) {
        GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Zoom In"));
    }

// Expected state: top sequence not overlaps with ruler
    GTGlobals::sleep();

    for (int i=0; i<8; i++) {
        GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Zoom Out"));
    }
}

GUI_TEST_CLASS_DEFINITION(test_0022){
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Select character â„–3 in "Phaneroptera_falcata"(G)
    GTUtilsMSAEditorSequenceArea::click(os,QPoint(2,0));
    QLabel* posLabel = qobject_cast<QLabel*>(GTWidget::findWidget(os,"Position"));
    CHECK_SET_ERR(posLabel, "Position label not found");
    CHECK_SET_ERR(posLabel->text()=="Pos 3 / 14", "Expected text: Pos 3/14. Found: " + posLabel->text());
//Expected state: Statistics "Pos" in right bottom is "Pos 3/14"

//3. Insert 3 gaps to first three positoons in "Phaneroptera_falcata"
    GTUtilsMSAEditorSequenceArea::click(os,QPoint(0,0));
    for(int i=0;i<3;i++){
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
        GTGlobals::sleep(200);
    }
//4. Select char at 4 position in "Phaneroptera_falcata"(A)
    GTUtilsMSAEditorSequenceArea::click(os,QPoint(3,0));
    CHECK_SET_ERR(posLabel->text()=="Pos 1 / 14", "Expected text: Pos 1/14. Found: " + posLabel->text());
//Expected state: Gaps are inserted, statistics "Pos" in right bottom is "Pos 1/14"
}

GUI_TEST_CLASS_DEFINITION(test_0022_1){//DIFFERENCE: Column label is tested
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Select character â„–3 in "Phaneroptera_falcata"(G)
    GTUtilsMSAEditorSequenceArea::click(os,QPoint(2,0));
    QLabel* colLabel = qobject_cast<QLabel*>(GTWidget::findWidget(os,"Column"));
    CHECK_SET_ERR(colLabel, "Column label not found");
    CHECK_SET_ERR(colLabel->text()=="Col 3 / 14", "Expected text: Col 3/14. Found: " + colLabel->text());
//Expected state: Statistics "Pos" in right bottom is "Pos 3/14"

//3. Insert 3 gaps to first three positoons in "Phaneroptera_falcata"
    GTUtilsMSAEditorSequenceArea::click(os,QPoint(0,0));
    for(int i=0;i<3;i++){
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
        GTGlobals::sleep(200);
    }
//4. Select char at 4 position in "Phaneroptera_falcata"(A)
    GTUtilsMSAEditorSequenceArea::click(os,QPoint(3,0));
    CHECK_SET_ERR(colLabel->text()=="Col 4 / 17", "Expected text: Col 4 / 17. Found: " + colLabel->text());
//Expected state: Gaps are inserted, statistics "Pos" in right bottom is "Pos 1/14"
}

GUI_TEST_CLASS_DEFINITION(test_0022_2){//DIFFERENCE: Line label is tested
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Select character â„–3 in "Phaneroptera_falcata"(G)
    GTUtilsMSAEditorSequenceArea::click(os,QPoint(2,0));
    QLabel* lineLabel = qobject_cast<QLabel*>(GTWidget::findWidget(os,"Line"));
    CHECK_SET_ERR(lineLabel, "Line label not found");
    CHECK_SET_ERR(lineLabel->text()=="Ln 1 / 10", "Expected text: Ln 1 / 10. Found: " + lineLabel->text());
//Expected state: Statistics "Pos" in right bottom is "Pos 3/14"

//3. Insert 3 gaps to first three positoons in "Phaneroptera_falcata"
    GTUtilsMSAEditorSequenceArea::selectArea(os,QPoint(-5,0),QPoint(-5,4));
    GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["delete"]);

//4. Select char at 4 position in "Phaneroptera_falcata"(A)
    GTUtilsMSAEditorSequenceArea::click(os,QPoint(3,0));
    CHECK_SET_ERR(lineLabel->text()=="Ln 1 / 5", "Expected text: Ln 1 / 5. Found: " + lineLabel->text());
//Expected state: Gaps are inserted, statistics "Pos" in right bottom is "Pos 1/14"
}

GUI_TEST_CLASS_DEFINITION(test_0023){
//    1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(500);
//    2. Do context menu->Add->sequence from file
    GTFileDialogUtils *ob = new GTFileDialogUtils(os,dataDir + "samples/Genbank/", "CVU55762.gb");
    GTUtilsDialog::waitForDialog(os, ob);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_LOAD<<"Sequence from file"));
    GTMenu::showContextMenu(os,GTWidget::findWidget(os, "msa_editor_sequence_area"));
//    3. Select data/samples/GENBANK/CVU55762_new.fa
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "CVU55762", "CVU55762"));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,18));
    GTMouseDriver::doubleClick(os);
//    Expected state: CVU55762 presents in list
}

GUI_TEST_CLASS_DEFINITION(test_0024){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. select first symbol of first sequence
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0,0));
    GTMouseDriver::click(os);
//3. press toolbar button "zoom to selection"
    int initOffset  = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    //offsets are used to check zooming
    QAbstractButton* zoom_to_sel = GTAction::button(os,"Zoom To Selection");
    GTWidget::click(os,zoom_to_sel);

    int finOffset  = GTUtilsMSAEditorSequenceArea::getRightOffset(os);
    CHECK_SET_ERR(initOffset>=(finOffset*2-8), "inital offset: " + QString().setNum(initOffset) + " final offset: " + QString().setNum(finOffset));
//Expected state: MSA is zoomed

//4. press toolbar button "Reset zoom"
    GTGlobals::sleep();
    QAbstractButton* reset_zoom = GTAction::button(os,"Reset Zoom");
    GTWidget::click(os,reset_zoom);
    GTGlobals::sleep(500);
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getRightOffset(os)==initOffset, "MSA is not zoomed back");
//Expected state: MSA is zoomed back

}

// linux test
GUI_TEST_CLASS_DEFINITION(test_0025){
//    1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. press "change font button" on toolbar
    GTUtilsDialog::waitForDialog(os, new FontDialogFiller(os));

    QAbstractButton* change_font = GTAction::button(os,"Change Font");
    GTWidget::click(os,change_font);
    GTGlobals::sleep(500);

    QWidget* nameListWidget = GTWidget::findWidget(os,"msa_editor_COI");
    MSAEditorUI* ui = qobject_cast<MSAEditorUI*>(nameListWidget);

    QFont f = ui->getEditor()->getFont();
    QString expectedFont = "Sans Serif,10,-1,5,50,0,0,0,0,0";

    CHECK_SET_ERR(f.toString() == expectedFont,"Expected: " + expectedFont + " found: " + f.toString())
    ;
//    Expected state: change font dialog appeared

//    3. choose some font, press OK
//    Expected state: font is changed
}

// windows test
GUI_TEST_CLASS_DEFINITION(test_0025_1){
//    1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. press "change font button" on toolbar
    GTUtilsDialog::waitForDialog(os, new FontDialogFiller(os));

    QAbstractButton* change_font = GTAction::button(os,"Change Font");
    GTWidget::click(os,change_font);
    GTGlobals::sleep(500);

    QWidget* nameListWidget = GTWidget::findWidget(os,"msa_editor_COI");
    MSAEditorUI* ui = qobject_cast<MSAEditorUI*>(nameListWidget);

    QFont f = ui->getEditor()->getFont();
    QString expectedFont = "Verdana,10,-1,5,50,0,0,0,0,0";

    CHECK_SET_ERR(f.toString() == expectedFont,"Expected: " + expectedFont + "found: " + f.toString())
    ;
//    Expected state: change font dialog appeared

//    3. choose some font, press OK
//    Expected state: font is changed
}

GUI_TEST_CLASS_DEFINITION(test_0026){
//    1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. press "export as image" on toolbar
    GTUtilsDialog::waitForDialog(os, new ExportImage(os,testDir + "_common_data/scenarios/sandbox/image.bmp"));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));

    QAbstractButton* saveImage = GTAction::button(os,"Export as image");
    CHECK_SET_ERR(saveImage, "Save as image button not found");

    GTWidget::click(os,saveImage);
//    Expected state: export dialog appeared

//    3. fill dialog:
//    file name: test/_common_data/scenarios/sandbox/image.bmp
//    press OK
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/sandbox/","image.bmp");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Expected state: image is exported
}

GUI_TEST_CLASS_DEFINITION(test_0026_1){//DIFFERENCE: context menu is used
//    1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. press "export as image" on toolbar
    GTUtilsDialog::waitForDialog(os, new ExportImage(os,testDir + "_common_data/scenarios/sandbox/image.bmp"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_EXPORT<<"Export as image"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
//    Expected state: export dialog appeared

//    3. fill dialog:
//    file name: test/_common_data/scenarios/sandbox/image.bmp
//    press OK
//    Expected state: image is exported
}

GUI_TEST_CLASS_DEFINITION(test_0026_2_linux){
//    1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. press "export as image" on toolbar
    GTUtilsDialog::waitForDialog(os, new ExportImage(os,testDir + "_common_data/scenarios/sandbox/bigImage.bmp", "jpeg" ,100));
    //GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));

    QAbstractButton* saveImage = GTAction::button(os,"Export as image");
    CHECK_SET_ERR(saveImage, "Save as image button not found");

    GTWidget::click(os,saveImage);
//    Expected state: export dialog appeared
    GTUtilsDialog::waitForDialog(os, new ExportImage(os,testDir + "_common_data/scenarios/sandbox/smallImage.bmp","jpeg",50));
    GTWidget::click(os,saveImage);
    GTGlobals::sleep(500);
//    3. fill dialog:
//    file name: test/_common_data/scenarios/sandbox/image.bmp
//    press OK
    qint64 bigSize = GTFile::getSize(os,testDir + "_common_data/scenarios/sandbox/bigImage.jpeg");
    qint64 smallSize = GTFile::getSize(os,testDir + "_common_data/scenarios/sandbox/smallImage.jpeg");

    CHECK_SET_ERR(bigSize==4800933 && smallSize==917068, QString().setNum(bigSize) + "  " + QString().setNum(smallSize));
//    Expected state: image is exported
}

GUI_TEST_CLASS_DEFINITION(test_0026_2_windows){
//    1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. press "export as image" on toolbar
    GTUtilsDialog::waitForDialog(os, new ExportImage(os,testDir + "_common_data/scenarios/sandbox/bigImage.bmp","jpeg",100));
    //GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));

    QAbstractButton* saveImage = GTAction::button(os,"Export as image");
    CHECK_SET_ERR(saveImage, "Save as image button not found");

    GTWidget::click(os,saveImage);
//    Expected state: export dialog appeared
    GTUtilsDialog::waitForDialog(os, new ExportImage(os,testDir + "_common_data/scenarios/sandbox/smallImage.bmp","jpeg",50));
    GTWidget::click(os,saveImage);
//    3. fill dialog:
//    file name: test/_common_data/scenarios/sandbox/image.bmp
//    press OK
    qint64 bigSize = GTFile::getSize(os,testDir + "_common_data/scenarios/sandbox/bigImage.jpeg");
    qint64 smallSize = GTFile::getSize(os,testDir + "_common_data/scenarios/sandbox/smallImage.jpeg");

    CHECK_SET_ERR(bigSize > 4000000 && bigSize < 5500000  && smallSize > 800000 && smallSize < 1500000, QString().setNum(bigSize) + "  " + QString().setNum(smallSize));
//    Expected state: image is exported
}

GUI_TEST_CLASS_DEFINITION(test_0027){
//    1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. select element 4 in sequence 3
    GTUtilsMSAEditorSequenceArea::click(os,QPoint(3,2));
//    3. Move selected left using mouse by 6
    GTUtilsMSAEditorSequenceArea::selectArea(os,QPoint(3,2),QPoint(9,2));
    GTGlobals::sleep(500);

    GTUtilsMSAEditorSequenceArea::selectArea(os,QPoint(3,2),QPoint(8,2));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "------", "Expected: ------ Found: " + clipboardText);
//    Expected stste: area is moved,position 4-9 filled with gaps
}

GUI_TEST_CLASS_DEFINITION(test_0027_1){
//    1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. select element 4 in sequence 3
    GTUtilsMSAEditorSequenceArea::selectArea(os,QPoint(3,2),QPoint(3,3));
//    3. Move selected left using mouse by 6
    GTUtilsMSAEditorSequenceArea::selectArea(os,QPoint(3,2),QPoint(9,2));
    GTGlobals::sleep(500);

    GTUtilsMSAEditorSequenceArea::selectArea(os,QPoint(3,2),QPoint(8,3));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "------\n------", "Expected: ------\n------ Found: " + clipboardText);
//    Expected stste: area is moved,position 4-9 filled with gaps
}

GUI_TEST_CLASS_DEFINITION(test_0028_linux){
//    1. Open document "samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    2. Context menu -- "Export as image"
    GTUtilsDialog::waitForDialog(os,new ExportMsaImage(os, testDir + "_common_data/scenarios/sandbox/test.svg", QString("svg")));
    GTUtilsDialog::waitForDialog( os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Export as image"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));

//    3. Fill dialog: svg format, output file
    qint64 fileSize = GTFile::getSize(os,testDir + "_common_data/scenarios/sandbox/test.svg");
    CHECK_SET_ERR(fileSize > 7500000 && fileSize < 8000000, "Current size: " + QString().setNum(fileSize));
//    Expected state:  SVG is exported
}

GUI_TEST_CLASS_DEFINITION(test_0028_windows){
//    1. Open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    2. Context menu -- "Export as image"
    GTUtilsDialog::waitForDialog(os,new ExportMsaImage(os, testDir + "_common_data/scenarios/sandbox/test.svg", QString("svg")));
    GTUtilsDialog::waitForDialog( os, new PopupChooser( os, QStringList() << MSAE_MENU_EXPORT << "Export as image"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));

//    3. Fill dialog: svg format, output file
    qint64 fileSize = GTFile::getSize(os,testDir + "_common_data/scenarios/sandbox/test.svg");
    CHECK_SET_ERR(fileSize > 6500000 && fileSize < 9800000, "Current size: " + QString().setNum(fileSize));
//    Expected state:  SVG is exported

}

GUI_TEST_CLASS_DEFINITION(test_0029){
    //    1. open document samples/CLUSTALW/COI.aln
        GTUtilsProject::openFiles(os, dataDir + "samples/CLUSTALW/COI.aln");
    //    2. Select first sequence
        GTUtilsMSAEditorSequenceArea::click(os,QPoint(0,0));
        GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_EXPORT<<"Save sequence",GTGlobals::UseKey));
        Runnable* r = new ExportSelectedSequenceFromAlignment(os,testDir + "_common_data/scenarios/sandbox/export.fasta",ExportSelectedSequenceFromAlignment::FASTA,true);
        GTUtilsDialog::waitForDialog(os, r);

        GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
        GTGlobals::sleep();
        GTUtilsTaskTreeView::waitTaskFinished(os);
        //GTMouseDriver::click(os,Qt::RightButton);

        GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"export.fasta"));
        GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"Phaneroptera_falcata"));
        GTGlobals::sleep();

        GTUtilsDialog::waitForDialog(os, new SelectSequenceRegionDialogFiller(os,42,44));
        GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Select"<< "Sequence region"));
        GTMenu::showContextMenu(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));
        GTGlobals::sleep();


        GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ADV_MENU_COPY<< "Copy sequence"));
        GTMenu::showContextMenu(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));
        GTGlobals::sleep();

        QString clipboardText = GTClipboard::text(os);
        CHECK_SET_ERR(clipboardText == "---", "Expected: TAGTTTATTAA, Found: " + clipboardText);
    //    3. use MSA area context menu->export->save sequence
    //    Exptcted state: Export sequence dialog appeared

    //    4. fill dialog:
    //    Export to file: test/_common_data/scenarios/sandbox/sequence.fa(use other extensions is branches)
    //    Add to project: checked
    //    Gap characters: keep
    //    Expectes state: sequence added to project
}

GUI_TEST_CLASS_DEFINITION(test_0029_1){//DIFFERENCE:gaps are trimmed, FASTQ format is used
//    1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Select first sequence
    GTUtilsMSAEditorSequenceArea::click(os,QPoint(0,2));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_EXPORT<<"Save sequence",GTGlobals::UseKey));
    Runnable* r = new ExportSelectedSequenceFromAlignment(os,testDir + "_common_data/scenarios/sandbox/export.fasta",ExportSelectedSequenceFromAlignment::FASTQ,false);
    GTUtilsDialog::waitForDialog(os, r);

    GTMouseDriver::click(os,Qt::RightButton);

    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"export.fastq"));
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"Bicolorana_bicolor_EF540830"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new SelectSequenceRegionDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Select"<< "Sequence region",GTGlobals::UseKey));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ADV_MENU_COPY<< "Copy sequence",GTGlobals::UseKey));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));

    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "TAGTTTATTAA", "Expected: TAGTTTATTAA, Found: " + clipboardText);
//    3. use MSA area context menu->export->save sequence
//    Exptcted state: Export sequence dialog appeared

//    4. fill dialog:
//    Export to file: test/_common_data/scenarios/sandbox/sequence.fa(use other extensions is branches)
//    Add to project: checked
//    Gap characters: keep
//    Expectes state: sequence added to project
}

GUI_TEST_CLASS_DEFINITION(test_0029_2){
//    1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Select first sequence
    GTUtilsMSAEditorSequenceArea::click(os,QPoint(0,2));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_EXPORT<<"Save sequence",GTGlobals::UseKey));
    Runnable* r = new ExportSelectedSequenceFromAlignment(os,testDir + "_common_data/scenarios/sandbox/export.fasta",ExportSelectedSequenceFromAlignment::Genbank,true,false);
    GTUtilsDialog::waitForDialog(os, r);

    GTMouseDriver::click(os,Qt::RightButton);

    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/sandbox/","export.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    3. use MSA area context menu->export->save sequence
//    Exptcted state: Export sequence dialog appeared

//    4. fill dialog:
//    Export to file: test/_common_data/scenarios/sandbox/sequence.fa(use other extensions is branches)
//    Add to project: checked
//    Gap characters: keep
//    Expectes state: sequence added to project
}

GUI_TEST_CLASS_DEFINITION(test_0031){//TODO: check statistic result
//    1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Do MSA area context menu->Statistics->generate grid profile
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate grid profile"));
    GTUtilsDialog::waitForDialog(os, new GenerateAlignmentProfileDialogFiller(os));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Exptcted state: generate alignment profile dialog appeared

//    3. Fill dialog: Profile mode:Counts. Click "Generate"
    QWidget* profile = GTWidget::findWidget(os, "Alignment profile for ma2_gapped");
    CHECK_SET_ERR(profile, "Alignment profile widget not found");
//    Expected state: Alignment profile generated

}

GUI_TEST_CLASS_DEFINITION(test_0031_1){//DIFFERENCE: Percentage is used
//    1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Do MSA area context menu->Statistics->generate grid profile
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate grid profile"));
    GTUtilsDialog::waitForDialog(os, new GenerateAlignmentProfileDialogFiller(os,false));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(1500);
//    Exptcted state: generate alignment profile dialog appeared

//    3. Fill dialog: Profile mode:Counts. Click "Generate"
    QWidget* profile = GTWidget::findWidget(os, "Alignment profile for ma2_gapped");
    CHECK_SET_ERR(profile, "Alignment profile widget not found");
//    Expected state: Alignment profile generated

}

GUI_TEST_CLASS_DEFINITION(test_0031_2){//TODO: check statistic result
//    1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Do MSA area context menu->Statistics->generate grid profile
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate grid profile",GTGlobals::UseKey));
    GTUtilsDialog::waitForDialog(os, new GenerateAlignmentProfileDialogFiller(os,true,false,false));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(1500);
//    Exptcted state: generate alignment profile dialog appeared

//    3. Fill dialog: Profile mode:Counts. Click "Generate"
    QWidget* profile = GTWidget::findWidget(os, "Alignment profile for ma2_gapped");
    CHECK_SET_ERR(profile, "Alignment profile widget not found");
//    Expected state: Alignment profile generated

}

GUI_TEST_CLASS_DEFINITION(test_0031_3){//TODO: check statistic result
//    1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Do MSA area context menu->Statistics->generate grid profile
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate grid profile",GTGlobals::UseKey));
    GTUtilsDialog::waitForDialog(os, new GenerateAlignmentProfileDialogFiller(os,false,true,false));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(1500);
//    Exptcted state: generate alignment profile dialog appeared

//    3. Fill dialog: Profile mode:Counts. Click "Generate"
    QWidget* profile = GTWidget::findWidget(os, "Alignment profile for ma2_gapped");
    CHECK_SET_ERR(profile, "Alignment profile widget not found");
//    Expected state: Alignment profile generated

}

GUI_TEST_CLASS_DEFINITION(test_0031_4){//TODO: check statistic result
//    1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Do MSA area context menu->Statistics->generate grid profile
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate grid profile",GTGlobals::UseKey));
    GTUtilsDialog::waitForDialog(os, new GenerateAlignmentProfileDialogFiller(os,false,false,true));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(1500);
//    Exptcted state: generate alignment profile dialog appeared

//    3. Fill dialog: Profile mode:Counts. Click "Generate"
    QWidget* profile = GTWidget::findWidget(os, "Alignment profile for ma2_gapped");
    CHECK_SET_ERR(profile, "Alignment profile widget not found");
//    Expected state: Alignment profile generated

}

GUI_TEST_CLASS_DEFINITION(test_0032){
//    1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Do MSA area context menu->Statistics->generate grid profile
//    Exptcted state: generata alignment profile dialog appeared
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate grid profile",GTGlobals::UseKey));
    GTUtilsDialog::waitForDialog(os, new GenerateAlignmentProfileDialogFiller(os,true,GenerateAlignmentProfileDialogFiller::HTML,
                                                                              testDir + "_common_data/scenarios/sandbox/stat.html"));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(500);

    qint64 size = 0;
    size = GTFile::getSize(os, testDir + "_common_data/scenarios/sandbox/stat.html");
    CHECK_SET_ERR(size > 0, "file not found");
//    3. Fill dialog: Profile mode:Counts
//            Save profile to file: checked
//            file path: test/_common_data/scenarios/sandbox/stat.html(stat.csv)
//            Click "Generate"
//    Expected state: Alignment profile file created
}

GUI_TEST_CLASS_DEFINITION(test_0032_1){//DIFFERENCE: csv format is used
//    1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Do MSA area context menu->Statistics->generate grid profile
//    Exptcted state: generata alignment profile dialog appeared
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate grid profile",GTGlobals::UseKey));
    GTUtilsDialog::waitForDialog(os, new GenerateAlignmentProfileDialogFiller(os,true,GenerateAlignmentProfileDialogFiller::CSV,
                                                                              testDir + "_common_data/scenarios/sandbox/stat.html"));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(500);

    qint64 size = 0;
    size = GTFile::getSize(os, testDir + "_common_data/scenarios/sandbox/stat.csv");
    CHECK_SET_ERR(size > 0, "file not found");
//    3. Fill dialog:Profile mode:Counts
//            Save profile to file: checked
//            file path: test/_common_data/scenarios/sandbox/stat.html(stat.csv)
//            Click "Generate"
//    Expected state: Alignment profile file created
}

GUI_TEST_CLASS_DEFINITION(test_0033){
//1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Do MSA area context menu->Statistics->generate distance matrix
    GTUtilsDialog::waitForDialog(os, new DistanceMatrixDialogFiller(os,true,true,true));
    Runnable* pop = new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate distance matrix",GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, pop);
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(500);
//Exptcted state: generata distance matrix dialog appeared

//3. Fill dialog: Distance Algorithm: Hamming dissimilarity(Simple similiraty)
//        Profile mode: Counts
//        Exclude gakls: checked
//        Click "Generate"
    QWidget* profile = GTWidget::findWidget(os, "Distance matrix for ma2_gapped");
    CHECK_SET_ERR(profile, "Alignment profile widget not found");
//Expected state: Alignment profile file created
}

GUI_TEST_CLASS_DEFINITION(test_0033_1){
//1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Do MSA area context menu->Statistics->generate distance matrix
    GTUtilsDialog::waitForDialog(os, new DistanceMatrixDialogFiller(os,false,true,true));
    Runnable* pop = new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate distance matrix",GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, pop);
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(500);
//Exptcted state: generata distance matrix dialog appeared

//3. Fill dialog: Distance Algorithm: Hamming dissimilarity(Simple similiraty)
//        Profile mode: Counts
//        Exclude gakls: checked
//        Click "Generate"
    QWidget* profile = GTWidget::findWidget(os, "Distance matrix for ma2_gapped");
    CHECK_SET_ERR(profile, "Alignment profile widget not found");
//Expected state: Alignment profile file created
}

GUI_TEST_CLASS_DEFINITION(test_0034){
//1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Do MSA area context menu->Statistics->generate distance matrix
    GTUtilsDialog::waitForDialog(os, new DistanceMatrixDialogFiller(os,true,false,true));
    Runnable* pop = new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate distance matrix",GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, pop);
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(500);
//Exptcted state: generata distance matrix dialog appeared

//3. Fill dialog: Distance Algorithm: Hamming dissimilarity
//        Profile mode: Counts(Percents)
//        Exclude gakls: checked(unchecked)
//        Click "Generate"
    QWidget* profile = GTWidget::findWidget(os, "Distance matrix for ma2_gapped");
    CHECK_SET_ERR(profile, "Alignment profile widget not found");
//Expected state: Alignment profile file created

}

GUI_TEST_CLASS_DEFINITION(test_0034_1){
//1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Do MSA area context menu->Statistics->generate distance matrix
    GTUtilsDialog::waitForDialog(os, new DistanceMatrixDialogFiller(os,true,true,false));
    Runnable* pop = new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate distance matrix",GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, pop);
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(500);
//Exptcted state: generata distance matrix dialog appeared

//3. Fill dialog: Distance Algorithm: Hamming dissimilarity
//        Profile mode: Counts(Percents)
//        Exclude gakls: checked(unchecked)
//        Click "Generate"
    QWidget* profile = GTWidget::findWidget(os, "Distance matrix for ma2_gapped");
    CHECK_SET_ERR(profile, "Alignment profile widget not found");
//Expected state: Alignment profile file created

}

GUI_TEST_CLASS_DEFINITION(test_0035){
//1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Do MSA area context menu->Statistics->generate distance matrix
    Runnable* dis = new DistanceMatrixDialogFiller(os,DistanceMatrixDialogFiller::HTML,testDir + "_common_data/scenarios/sandbox/matrix.html");
    GTUtilsDialog::waitForDialog(os, dis);
    Runnable* pop = new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate distance matrix",GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, pop);
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(500);
//Exptcted state: generata distance matrix dialog appeared

//3. Fill dialog: Distance Algorithm: Hamming dissimilarity
//        Profile mode: Counts
//        Exclude gakls: checked
//        Save profile to file: checked
//        File path: test/_common_data/scenarios/sandbox/matrix.html(matrix.csv)
//        Click "Generate"
    qint64 size = GTFile::getSize(os,testDir + "_common_data/scenarios/sandbox/matrix.html");
    CHECK_SET_ERR(size!=0,"file not created");
//Expected state: Alignment profile file created
}

GUI_TEST_CLASS_DEFINITION(test_0035_1){
//1. Open document test/_common_data/scenarios/msa/ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/","ma2_gapped.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Do MSA area context menu->Statistics->generate distance matrix
    Runnable* dis = new DistanceMatrixDialogFiller(os,DistanceMatrixDialogFiller::CSV,testDir + "_common_data/scenarios/sandbox/matrix.html");
    GTUtilsDialog::waitForDialog(os, dis);
    Runnable* pop = new PopupChooser(os,QStringList()<<MSAE_MENU_STATISTICS<<"Generate distance matrix",GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, pop);
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(500);
//Exptcted state: generata distance matrix dialog appeared

//3. Fill dialog: Distance Algorithm: Hamming dissimilarity
//        Profile mode: Counts
//        Exclude gakls: checked
//        Save profile to file: checked
//        File path: test/_common_data/scenarios/sandbox/matrix.html(matrix.csv)
//        Click "Generate"
    qint64 size = GTFile::getSize(os,testDir + "_common_data/scenarios/sandbox/matrix.csv");
    CHECK_SET_ERR(size!=0,"file not created");
//Expected state: Alignment profile file created
}

GUI_TEST_CLASS_DEFINITION(test_0036){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(500);
//2. Press "build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk",0));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTUtilsTaskTreeView::waitTaskFinished(os);
//Expected state: build tree dialog appeared

//3. Fill dialog:
//    Distanse matrix model: F84(Kimura/Jukes-Cantor/LogDet)
//    Press "Build"
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
//Expected state: tree appeared
}

GUI_TEST_CLASS_DEFINITION(test_0036_1){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(500);
//2. Press "build tree" button on toolbar
//Expected state: build tree dialog appeared
    //3. Fill dialog:
    //    Distanse matrix model: F84(Kimura/Jukes-Cantor/LogDet)
    //    Press "Build"
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk",1));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);

    GTUtilsTaskTreeView::waitTaskFinished(os);

//Expected state: tree appeared
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
}

GUI_TEST_CLASS_DEFINITION(test_0036_2){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(500);
//2. Press "build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk",2));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep(500);
//Expected state: build tree dialog appeared

//3. Fill dialog:
//    Distanse matrix model: F84(Kimura/Jukes-Cantor/LogDet)
//    Press "Build"
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
//Expected state: tree appeared
}

GUI_TEST_CLASS_DEFINITION(test_0036_3){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(500);
//2. Press "build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk",3));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep(500);
//Expected state: build tree dialog appeared

//3. Fill dialog:
//    Distanse matrix model: F84(Kimura/Jukes-Cantor/LogDet)
//    Press "Build"
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
//Expected state: tree appeared
}

GUI_TEST_CLASS_DEFINITION(test_0037){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Press "build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk",0,0.5));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep(500);
//Expected state: build tree dialog appeared

//3. Fill dialog:
//    Distanse matrix model: F84
//    Gamma distributed rates across sites: checked
//    Coefficient of variation: 0.50(50.00/99.00)
//    Press "Build"
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
//Expected state: tree appeared
}

GUI_TEST_CLASS_DEFINITION(test_0037_1){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Press "build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk",0,50));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep();
//Expected state: build tree dialog appeared

//3. Fill dialog:
//    Distanse matrix model: F84
//    Gamma distributed rates across sites: checked
//    Coefficient of variation: 0.50(50.00/99.00)
//    Press "Build"
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
//Expected state: tree appeared
}

GUI_TEST_CLASS_DEFINITION(test_0037_2){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Press "build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk",0,99));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep();
//Expected state: build tree dialog appeared

//3. Fill dialog:
//    Distanse matrix model: F84
//    Gamma distributed rates across sites: checked
//    Coefficient of variation: 0.50(50.00/99.00)
//    Press "Build"
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
//Expected state: tree appeared
}

GUI_TEST_CLASS_DEFINITION(test_0038){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Press "build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os,100,testDir + "_common_data/scenarios/sandbox/COI.nwk",
                                                               5,BuildTreeDialogFiller::MAJORITYEXT));

    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTUtilsTaskTreeView::waitTaskFinished(os);//some time is needed to build tree
//Expected state: build tree dialog appeared

//3. Fill dialog:
//    Distanse matrix model: F84
//    Gamma distributed rates across sites: unchecked
//    Bootatraping and consensus tree: checked
//    Number of replications: 100
//    Seed: 5
//    Consensus type: Majority Rule extended(Strict/Majority Rule/M1)
//    Press "Build"
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
//Expected state: tree appeared
}

GUI_TEST_CLASS_DEFINITION(test_0038_1){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Press "build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os,100,testDir + "_common_data/scenarios/sandbox/COI.nwk",
                                                               5,BuildTreeDialogFiller::STRICTCONSENSUS));

    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTUtilsTaskTreeView::waitTaskFinished(os);//some time is needed to build tree
//Expected state: build tree dialog appeared

//3. Fill dialog:
//    Distanse matrix model: F84
//    Gamma distributed rates across sites: unchecked
//    Bootatraping and consensus tree: checked
//    Number of replications: 100
//    Seed: 5
//    Consensus type: Majority Rule extended(Strict/Majority Rule/M1)
//    Press "Build"
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
//Expected state: tree appeared
}

GUI_TEST_CLASS_DEFINITION(test_0038_2){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Press "build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os,100,testDir + "_common_data/scenarios/sandbox/COI.nwk",
                                                               5,BuildTreeDialogFiller::MAJORITY));

    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
//Expected state: build tree dialog appeared

//3. Fill dialog:
//    Distanse matrix model: F84
//    Gamma distributed rates across sites: unchecked
//    Bootatraping and consensus tree: checked
//    Number of replications: 100
//    Seed: 5
//    Consensus type: Majority Rule extended(Strict/Majority Rule/M1)
//    Press "Build"

    GTUtilsTaskTreeView::waitTaskFinished(os);

    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
//Expected state: tree appeared
}

GUI_TEST_CLASS_DEFINITION(test_0038_3){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Press "build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os,100,testDir + "_common_data/scenarios/sandbox/COI.nwk",
                                                               5,BuildTreeDialogFiller::M1));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
//Expected state: build tree dialog appeared

//3. Fill dialog:
//    Distanse matrix model: F84
//    Gamma distributed rates across sites: unchecked
//    Bootatraping and consensus tree: checked
//    Number of replications: 100
//    Seed: 5
//    Consensus type: Majority Rule extended(Strict/Majority Rule/M1)
//    Press "Build"
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
//Expected state: tree appeared
}

GUI_TEST_CLASS_DEFINITION(test_0038_4){
//1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//2. Press "build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os,100,testDir + "_common_data/scenarios/sandbox/COI.nwk",
                                                               5,BuildTreeDialogFiller::M1,1));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
//Expected state: build tree dialog appeared

//3. Fill dialog:
//    Distanse matrix model: F84
//    Gamma distributed rates across sites: unchecked
//    Bootatraping and consensus tree: checked
//    Number of replications: 100
//    Seed: 5
//    Consensus type: Majority Rule extended(Strict/Majority Rule/M1)
//    Press "Build"

    GTUtilsTaskTreeView::waitTaskFinished(os);

    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
//Expected state: tree appeared
}

void test_0039_function(HI::GUITestOpStatus &os, const QString &comboText, QString extention){
    //1. open document samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, UGUITest::dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Use project tree context menu->Export/Import->Export Nucleic Alignment to Amino Translation
    //Expected state: Export Nucleic Alignment to Amino Translation dialog appeared
    //3.Fill dialog:
    //    File name: test/_common_data/scenarios/sandbox/transl.aln
    //    File format: CLUSTALW(use other formats too, check extension change)
    //    Amino translation: Standart genetic code
    //    Add document to project: checked
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"COI"));
    GTUtilsDialog::waitForDialog(os, new ExportMSA2MSADialogFiller(os, comboText, UGUITest::sandBoxDir + "COI_transl.aln"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION<<ACTION_PROJECT__EXPORT_TO_AMINO_ACTION));
    GTMouseDriver::click(os,Qt::RightButton);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: transl.aln appeared in project
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI_transl." + extention));
}

GUI_TEST_CLASS_DEFINITION(test_0039) {
    test_0039_function(os, "CLUSTALW", "aln");
}

GUI_TEST_CLASS_DEFINITION(test_0039_1) {
    test_0039_function(os, "FASTA", "fa");
}

GUI_TEST_CLASS_DEFINITION(test_0039_2) {
    test_0039_function(os, "MSF", "msf");
}

GUI_TEST_CLASS_DEFINITION(test_0039_3) {
    test_0039_function(os, "Mega", "meg");
}

GUI_TEST_CLASS_DEFINITION(test_0039_4) {
    test_0039_function(os, "NEXUS", "nex");
}

GUI_TEST_CLASS_DEFINITION(test_0039_5) {
    test_0039_function(os, "PHYLIP Interleaved", "phy");
}

GUI_TEST_CLASS_DEFINITION(test_0039_6) {
    test_0039_function(os, "PHYLIP Sequential", "phy");
}

GUI_TEST_CLASS_DEFINITION(test_0039_7) {
    test_0039_function(os, "Stockholm", "sto");
}

GUI_TEST_CLASS_DEFINITION(test_0040){//UGENE crashes when opening several files
    QFile human_T1(dataDir + "/samples/FASTA/human_T1.fa");
    human_T1.copy(dataDir + "/samples/CLUSTALW/human_T1.fa");
    GTFileDialog::openFileList(os,dataDir + "samples/CLUSTALW/",QStringList()<<"COI.aln"<<"human_T1.fa");

    //GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os,QMessageBox::No));
    GTUtilsProjectTreeView::findIndex(os,"human_T1.fa");//checks inside
    GTUtilsProjectTreeView::findIndex(os,"COI.aln");

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTGlobals::sleep(500);
    QFile(dataDir + "/samples/CLUSTALW/human_T1.fa").remove();
    GTGlobals::sleep(5000);

}

GUI_TEST_CLASS_DEFINITION(test_0041) {
    // Shifting region in the Alignment Editor (UGENE-2127)
    //
    // 1. Open file data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    int oldLength = GTUtilsMSAEditorSequenceArea::getLength(os);
    // 2. Select the first column.
    GTUtilsMSAEditorSequenceArea::selectColumnInConsensus(os, 0);
    // Expected state: column became selected
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(0, 0, 1, 18));

    // 3. Drag the selection with mouse to 5 bases to the right.
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0,0));
    GTMouseDriver::press(os);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(5,0));
    GTMouseDriver::release(os);
    // Expected state: alignment moved to 5 bases to the right.

    // 4. Drag the selection with mouse to one base to the left.
    GTMouseDriver::press(os);
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(4,0));
    GTMouseDriver::release(os);
    // Expected state: alignment  moved to one bases to the left.

    // Check results
    int newLength = GTUtilsMSAEditorSequenceArea::getLength(os);
    CHECK_SET_ERR(4 == newLength - oldLength, QString("Wrong length of changed alignment"));
}

GUI_TEST_CLASS_DEFINITION(test_0042) {
    // default msa export
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< MSAE_MENU_EXPORT << "Export as image"));
    GTUtilsDialog::waitForDialog(os, new ExportMsaImage(os, testDir + "_common_data/scenarios/sandbox/test_0042.png"));

    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"msa_editor_sequence_area"));
}

GUI_TEST_CLASS_DEFINITION(test_0042_1) {
    // "all included" export
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Export as image"));
    GTUtilsDialog::waitForDialog(os, new ExportMsaImage(os,
                                                        testDir + "_common_data/scenarios/sandbox/test_0042_1.png",
                                                        ExportMsaImage::Settings(true, true, true)/*include all*/));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_editor_sequence_area"));
}

GUI_TEST_CLASS_DEFINITION(test_0042_2) {
    // slightly modified export
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Export as image"));
    GTUtilsDialog::waitForDialog(os, new ExportMsaImage(os,
                                                        testDir + "_common_data/scenarios/sandbox/test_0042_1",
                                                        ExportMsaImage::Settings(true, false, true)/*include all*/,
                                                        true,
                                                        false, RegionMsa(),
                                                        "bmp"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_editor_sequence_area"));
}

GUI_TEST_CLASS_DEFINITION(test_0043) {
    // select a few sequences
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QStringList sequences;
    sequences << "Montana_montana"
              << "Conocephalus_percaudata"
              << "Podisma_sapporensis";

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< MSAE_MENU_EXPORT << "Export as image"));
    GTUtilsDialog::waitForDialog(os, new ExportMsaImage(os, testDir + "_common_data/scenarios/sandbox/test_0043.png",
                                                        ExportMsaImage::Settings(), false, false,
                                                        RegionMsa(U2Region(1, 594), sequences)));

    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"msa_editor_sequence_area"));
}

GUI_TEST_CLASS_DEFINITION(test_0044) {
    // export selected region
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(5, 2), QPoint(25, 8));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< MSAE_MENU_EXPORT << "Export as image"));
    GTUtilsDialog::waitForDialog(os, new ExportMsaImage(os, testDir + "_common_data/scenarios/sandbox/test_0044.png",
                                                        ExportMsaImage::Settings(true, true, true),
                                                        false, true));

    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"msa_editor_sequence_area"));
}

GUI_TEST_CLASS_DEFINITION(test_0045) {
    // check the connection between export comboBox and selectRegion dialog
    // there should be no selection
    class ExportDialogChecker : public Filler {
    public:
        ExportDialogChecker(HI::GUITestOpStatus &os)
            : Filler(os, "ImageExportForm") {}
        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "activeModalWidget is NULL");

            GTUtilsDialog::waitForDialog(os, new DefaultDialogFiller(os, "SelectSubalignmentDialog", QDialogButtonBox::Cancel));
            QComboBox* exportType = dialog->findChild<QComboBox*>("comboBox");
            GTComboBox::setIndexWithText(os, exportType, "Custom region", false, GTGlobals::UseKey);

            GTGlobals::sleep();
            CHECK_SET_ERR(exportType->currentText() == "Whole alignment", "Wrong combo box text!");
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Export as image"));
    GTUtilsDialog::waitForDialog(os, new ExportDialogChecker(os));

    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"msa_editor_sequence_area"));
}

GUI_TEST_CLASS_DEFINITION(test_0045_1) {
    // check the connection between export comboBox and selectRegion dialog
    // there should be no selection

    class ExportChecker : public Filler {
    public:
        ExportChecker(HI::GUITestOpStatus &os)
            : Filler(os, "ImageExportForm") {}
        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "activeModalWidget is NULL");

            QComboBox* exportType = dialog->findChild<QComboBox*>("comboBox");
            CHECK_SET_ERR(exportType->currentText() == "Whole alignment", "Wrong combo box text!");

            GTUtilsDialog::waitForDialog(os,
                                         new SelectSubalignmentFiller(os,
                                                                      RegionMsa(U2Region(1, 593),
                                                                                QStringList() << "Montana_montana"
                                                                                << "Conocephalus_percaudata")));
            QPushButton* select = dialog->findChild<QPushButton*>("selectRegionButton");
            GTWidget::click(os, select);

            GTGlobals::sleep();
            CHECK_SET_ERR(exportType->currentText() == "Custom region", "Wrong combo box text!");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Export as image"));
    GTUtilsDialog::waitForDialog(os, new ExportChecker(os));

    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"msa_editor_sequence_area"));
}

GUI_TEST_CLASS_DEFINITION(test_0046) {
    // check quality
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTWidget::click(os, GTWidget::findWidget(os, "msa_editor_sequence_area"));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Export as image"));
    GTUtilsDialog::waitForDialog(os, new ExportMsaImage(os,
                                                        testDir + "_common_data/scenarios/sandbox/test_0046",
                                                        "jpg", 50));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_editor_sequence_area"));
}

GUI_TEST_CLASS_DEFINITION(test_0047) {
    // check select subalignment dialog

    class SelectSubalignmentChecker : public Filler {
    public:
        SelectSubalignmentChecker(HI::GUITestOpStatus &os)
            : Filler(os, "SelectSubalignmentDialog") {}

        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "activeModalWidget is NULL");
            QDialogButtonBox* box = dialog->findChild<QDialogButtonBox*>("buttonBox");
            CHECK_SET_ERR(box != NULL, "buttonBox is NULL");
            QPushButton* ok = box->button(QDialogButtonBox::Ok);
            CHECK_SET_ERR(ok !=NULL, "ok button is NULL");

            QSpinBox* startPosBox = dialog->findChild<QSpinBox*>("startPosBox");
            CHECK_SET_ERR(startPosBox != NULL, "startPosBox is NULL");
            GTSpinBox::setValue(os, startPosBox, 10);

            QSpinBox* endPosBox = dialog->findChild<QSpinBox*>("endPosBox");
            CHECK_SET_ERR(endPosBox != NULL, "endPoxBox is NULL");
            GTSpinBox::setValue(os, endPosBox, 5);

            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
            GTWidget::click(os, ok);

            GTSpinBox::setValue(os, endPosBox, 15);

            QWidget *noneButton = dialog->findChild<QWidget*>("noneButton");
            CHECK_SET_ERR(noneButton != NULL, "noneButton is NULL");
            GTWidget::click(os, noneButton);

            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
            GTWidget::click(os, ok);

            GTGlobals::sleep();
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    class ExportChecker : public Filler {
    public:
        ExportChecker(HI::GUITestOpStatus &os)
            : Filler(os, "ImageExportForm") {}
        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "activeModalWidget is NULL");

            GTUtilsDialog::waitForDialog(os, new SelectSubalignmentChecker(os));
            QPushButton* select = dialog->findChild<QPushButton*>("selectRegionButton");
            GTWidget::click(os, select);

            GTGlobals::sleep();
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "align.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(1,1), QPoint(1,1));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Export as image"));
    GTUtilsDialog::waitForDialog(os, new ExportChecker(os));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_editor_sequence_area"));
}

GUI_TEST_CLASS_DEFINITION(test_0048) {
    // fail to export big alignment
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa", "big.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    class CustomFiller_0048 : public Filler {
    public:
        CustomFiller_0048(HI::GUITestOpStatus &os)
            : Filler(os, "ImageExportForm") {}
        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "activeModalWidget is NULL");

            QComboBox* exportType = dialog->findChild<QComboBox*>("comboBox");
            CHECK_SET_ERR( exportType != NULL, "Cannot find comboBox");
            CHECK_SET_ERR( exportType->currentText() == "Whole alignment", "Wrong combo box text!");

            QLabel* hintLabel = dialog->findChild<QLabel*>("hintLabel");
            CHECK_SET_ERR( hintLabel != NULL, "Cannot find hintLabel");
            CHECK_SET_ERR( hintLabel->isVisible(), "Warning message is hidden!");

            QDialogButtonBox* buttonBox = dialog->findChild<QDialogButtonBox*>("buttonBox");
            CHECK_SET_ERR(buttonBox != NULL, "Cannot find buttonBox");
            QPushButton* exportButton = buttonBox->button(QDialogButtonBox::Ok);
            CHECK_SET_ERR( exportButton != NULL, "Cannot find Export button");
            CHECK_SET_ERR( !exportButton->isEnabled(), "Export button is enabled");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };


    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< MSAE_MENU_EXPORT << "Export as image"));
    GTUtilsDialog::waitForDialog(os, new CustomFiller_0048(os));

    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"msa_editor_sequence_area"));
}

GUI_TEST_CLASS_DEFINITION(test_0049){
    //save alignment buttons test
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsDialog::waitForDialog(os, new ExportDocumentDialogFiller(os, sandBoxDir, "COI_test_0049.aln",
                                                                    ExportDocumentDialogFiller::CLUSTALW));\
    GTWidget::click(os, GTAction::button(os, "Save alignment as"));
    GTUtilsProjectTreeView::click(os, "COI.aln");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    GTFileDialog::openFile(os, sandBoxDir, "COI_test_0049.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(10,10));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    GTWidget::click(os, GTAction::button(os, "Save alignment"));
    GTUtilsProjectTreeView::click(os, "COI_test_0049.aln");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    GTFileDialog::openFile(os, sandBoxDir, "COI_test_0049.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsMSAEditorSequenceArea::checkSelection(os, QPoint(0,0), QPoint(10,0), "ATTCGAGCCGA");
}

GUI_TEST_CLASS_DEFINITION(test_0050){
    //    1. Open "COI.aln"
    //    2. Set any reference sequence
    //    3. Open context menu, open the "Highlighting" submenu, set the "Agreements" type
    //    4. Open context menu again, open the "Export" submenu, choose the "Export highlighted" menu item
    //    Expected state: the "Export highlighted to file" dialog appears - there is a checkbox 'transpose output'
    //    5. Click "Export"
    //    Expected state: result file contain columns of sequences
    //    6. Repeat 3-4
    //    7. Deselect 'Transpose output' and click 'Export'
    //    Expected state: result file contain rowa of sequences

        GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
        GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Set this sequence as reference"));
        GTWidget::click(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os), Qt::RightButton, QPoint(10, 10));

        GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Highlighting" << "Agreements"));
        GTWidget::click(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os), Qt::RightButton);

        GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Export" << "Export highlighted"));
        GTUtilsDialog::waitForDialog(os, new ExportHighlightedDialogFiller(os, sandBoxDir + "common_msa_test_0050_1.txt"));
        GTWidget::click(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os), Qt::RightButton);
        GTGlobals::sleep();

        CHECK_SET_ERR( GTFile::equals(os, sandBoxDir + "common_msa_test_0050_1.txt",
                                      testDir + "_common_data/clustal/COI_highlighted_1"),
                       "Transposed export is incorrect");

        GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Export" << "Export highlighted"));
        GTUtilsDialog::waitForDialog(os, new ExportHighlightedDialogFiller(os, sandBoxDir + "common_msa_test_0050_2.txt", false));
        GTWidget::click(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os), Qt::RightButton);
        GTGlobals::sleep();

        CHECK_SET_ERR( GTFile::equals(os, sandBoxDir + "common_msa_test_0050_2.txt",
                                      testDir + "_common_data/clustal/COI_highlighted_2"),
                       "Export is incorrect");
}

GUI_TEST_CLASS_DEFINITION(test_0051){
/* 1. Open samples/CLUSTALW/ty3.aln.gz
 * 2. Open context menu, open "View" and choose "Sort sequences by name"
 *   Expected state: Sequences sorted case insensitive by name
*/
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa", "ma_unsorted.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_VIEW << "action_sort_by_name"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(200);

    QStringList names = GTUtilsMSAEditorSequenceArea::getNameList(os);

    CHECK_SET_ERR( names.length() == 4, "Count of sequences in MSA incorrect");
    CHECK_SET_ERR( names.at(0) == "a", "At positoin 0 not showed 'a' name");
    CHECK_SET_ERR( names.at(1) == "C", "At positoin 1 not showed 'C' name");
    CHECK_SET_ERR( names.at(2) == "D", "At positoin 2 not showed 'D' name");
    CHECK_SET_ERR( names.at(3) == "d", "At positoin 3 not showed 'd' name");
}

GUI_TEST_CLASS_DEFINITION(test_0052){
//    1. Open "_common_data/clustal/3000_sequences.aln"
//    2. Context menu -- Export as ImageExport
//    Expected state: export dialog appeared, there is a warning message and Export button is disabled
//    3. Select smalle region
//    Expected state: warning is gone, export is enabled
    GTFileDialog::openFile(os, testDir + "_common_data/clustal/3000_sequences.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    class CustomFiller_0052 : public Filler {
    public:
        CustomFiller_0052(HI::GUITestOpStatus &os)
            : Filler(os, "ImageExportForm") {}
        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "activeModalWidget is NULL");

            QComboBox* exportType = dialog->findChild<QComboBox*>("comboBox");
            CHECK_SET_ERR( exportType != NULL, "Cannot find comboBox");
            CHECK_SET_ERR( exportType->currentText() == "Whole alignment", "Wrong combo box text!");

            QLabel* hintLabel = dialog->findChild<QLabel*>("hintLabel");
            CHECK_SET_ERR( hintLabel != NULL, "Cannot find hintLabel");
            CHECK_SET_ERR( hintLabel->isVisible(), "Warning message is hidden!");

            QDialogButtonBox* buttonBox = dialog->findChild<QDialogButtonBox*>("buttonBox");
            CHECK_SET_ERR(buttonBox != NULL, "Cannot find buttonBox");
            QPushButton* exportButton = buttonBox->button(QDialogButtonBox::Ok);
            CHECK_SET_ERR( exportButton != NULL, "Cannot find Export button");
            CHECK_SET_ERR( !exportButton->isEnabled(), "Export button is enabled");

            GTUtilsDialog::waitForDialog(os,
                                         new SelectSubalignmentFiller(os,
                                                                      RegionMsa(U2Region(1, 593),
                                                                                QStringList() << "Sequence__1"
                                                                                << "Sequence__2" << "Sequnce__3" << "Sequence__4")));

            QPushButton* select = dialog->findChild<QPushButton*>("selectRegionButton");
            GTWidget::click(os, select);

            GTGlobals::sleep();
            CHECK_SET_ERR(exportType->currentText() == "Custom region", "Wrong combo box text!");

            CHECK_SET_ERR( !hintLabel->isVisible(), "Warning is visible");
            CHECK_SET_ERR( exportButton->isEnabled(), "Export button is disabled");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    GTUtilsDialog::waitForDialog(os, new CustomFiller_0052(os));
    GTUtilsDialog::waitForDialog( os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Export as image"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
}

GUI_TEST_CLASS_DEFINITION(test_0053){
        //Copied formatted (context menu)
        //1. Open amples\CLUSTALW\COI.aln
        //2. Select the first three letters TAA
        //3. Context menue {Copy-><<Copy formatted}
        //Expected state: the buffer contatin the sequence in CLUSTALW format
        GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
        GTGlobals::sleep();

        GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(2, 0));

        GTUtilsDialog::waitForDialog(os,new PopupChooser(os,QStringList()<<MSAE_MENU_COPY<<"copy_formatted"));
        GTMouseDriver::click(os,Qt::RightButton);
        GTGlobals::sleep(3000);

        QString clipboardText = GTClipboard::text(os);

        CHECK_SET_ERR(clipboardText.contains("TAA"), clipboardText);
        GTGlobals::sleep(3000);
}

GUI_TEST_CLASS_DEFINITION(test_0053_1){
    //Copied formatted (context menu), the format is changable
    //1. Open samples\CLUSTALW\COI.aln
    //2. Select the first three letters TAA
    //3. In the general tab of the options panel find the Copy Type combobox and select the Mega format
    //4. Context menu {Copy->Copy formatted}
    //Expected state: the buffer contatin the sequence in Mega format
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::General);
    GTGlobals::sleep(200);

    QComboBox* copyType = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "copyType"));
    CHECK_SET_ERR(copyType != NULL, "copy combobox not found");

    GTComboBox::setIndexWithText(os, copyType, "Mega");

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(2, 0));

    GTUtilsDialog::waitForDialog(os,new PopupChooser(os,QStringList()<<MSAE_MENU_COPY<<"copy_formatted"));
    GTMouseDriver::click(os,Qt::RightButton);
    GTGlobals::sleep(3000);

    QString clipboardText = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardText.contains("mega"), clipboardText);
    CHECK_SET_ERR(clipboardText.contains("TAA"), clipboardText);

    GTGlobals::sleep(3000);
}

GUI_TEST_CLASS_DEFINITION(test_0053_2){
    //Copied formatted (toolbar), the format is changable
    //1. Open samples\CLUSTALW\COI.aln
    //2. Select the first three letters TAA
    //3. In the general tab of the options panel find the Copy Type combobox and select the CLUSTALW format
    //4. Toolbar {Copy->Copy formatted}
    //Expected state: the buffer contatin the sequence in CLUSTALW format
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::General);
    GTGlobals::sleep(200);

    QComboBox* copyType = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "copyType"));
    CHECK_SET_ERR(copyType != NULL, "copy combobox not found");

    GTComboBox::setIndexWithText(os, copyType, "CLUSTALW");

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(2, 0));

    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "copy_formatted"));
    GTGlobals::sleep(3000);

    QString clipboardText = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardText.contains("CLUSTAL W 2.0 multiple sequence alignment"), clipboardText);
    CHECK_SET_ERR(clipboardText.contains("TAA"), clipboardText);

    GTGlobals::sleep(3000);
}

GUI_TEST_CLASS_DEFINITION(test_0053_3){
        //Copied formatted (context menu) for a big alignment
        //1. Open _common_data/clustal/100_sequences.aln
        //2. Select the whole alignment
        //3. Context menue {Copy->Copy formatted}
        //Expected state: the buffer contatin the sequences in CLUSTALW format
        GTFileDialog::openFile(os, testDir + "_common_data/clustal/100_sequences.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
        GTGlobals::sleep();

        QStringList names = GTUtilsMSAEditorSequenceArea::getNameList(os);
        CHECK_SET_ERR(!names.isEmpty(), "the alignment is empty");
        GTUtilsMSAEditorSequenceArea::selectSequence(os, names.first());

        GTUtilsDialog::waitForDialog(os,new PopupChooser(os,QStringList()<<MSAE_MENU_COPY<<"copy_formatted"));
        GTMouseDriver::click(os,Qt::RightButton);
        GTGlobals::sleep(3000);

        QString clipboardText = GTClipboard::text(os);

        CHECK_SET_ERR(clipboardText.contains("ACCAGGCTTGGCAATGCGTATC"), clipboardText);
        GTGlobals::sleep(3000);
}

GUI_TEST_CLASS_DEFINITION(test_0053_4){
        //Copied formatted (action is disabled when no selection
        //1. Open samples\CLUSTALW\COI.aln
        //2. Try context menue {Copy->Copy formatted}
        //Expected state: the action is disabled
        GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
        GTGlobals::sleep();

        QWidget* w = GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "copy_formatted");
        CHECK_SET_ERR(w!=NULL, "no copy action on the toolbar");
        CHECK_SET_ERR(w->isEnabled() == false, "selection is empty but the action is enabled");
}

GUI_TEST_CLASS_DEFINITION(test_0053_5){
    //Copied formatted (toolbar), the format is changable to RTF
    //1. Open samples\CLUSTALW\COI.aln
    //2. Select the first three letters TAA
    //3. In the general tab of the options panel find the Copy Type combobox and select the RTF format
    //4. Toolbar {Copy->Copy formatted}
    //Expected state: the buffer contatin the sequence in RTF format
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::General);
    GTGlobals::sleep(200);

    QComboBox* copyType = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "copyType"));
    CHECK_SET_ERR(copyType != NULL, "copy combobox not found");

    GTComboBox::setIndexWithText(os, copyType, "Rich text (HTML)");

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(2, 0));

    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "copy_formatted"));
    GTGlobals::sleep(3000);

    QString clipboardText = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardText.contains("<span style=\"font-size:10pt; font-family:Verdana;\">"), clipboardText);
    CHECK_SET_ERR(clipboardText.contains("<p><span style=\"background-color:#ff99b1;\">T</span><span style=\"background-color:#fcff92;\">A</span><span style=\"background-color:#fcff92;\">A</span></p>"), clipboardText);



    GTGlobals::sleep(3000);
}

/** These tests are created according to test plan: https://ugene.unipro.ru/wiki/display/PD/MSA**/

GUI_TEST_CLASS_DEFINITION(test_0054){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Use context menu:
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with muscle"));
    GTUtilsDialog::waitForDialog(os, new MuscleDialogFiller(os, MuscleDialogFiller::Default, true, true));
    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));
    GTUtilsTaskTreeView::waitTaskFinished(os);
    QString actual = GTUtilsMSAEditorSequenceArea::getSequenceData(os, "Phaneroptera_falcata");
    CHECK_SET_ERR(actual.startsWith("TAAGACTTCTAATTCGAGCCGAATTAGGTCAACCAGGATACC---TAATTGGAGATGATCAAATTTATAATGTAATTGT"), "unexpected sequence: " + actual);

//    {Align->Align with MUSCLE}
//    Check "Translate to amino when aligning" checkbox
//    Align
}

GUI_TEST_CLASS_DEFINITION(test_0054_1){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Use context menu:
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign"));
    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os, 0, true));
    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));
    GTUtilsTaskTreeView::waitTaskFinished(os);
    QString actual = GTUtilsMSAEditorSequenceArea::getSequenceData(os, "Phaneroptera_falcata");
    CHECK_SET_ERR(actual.startsWith("TAAGACTTCTAATTCGAGCCGAATTAGGTCAAC---CAGGATACCTAATTGGAGATGATCAAATTTATAATG"), "unexpected sequence: " + actual);

//    {Align->Align with MUSCLE}
//    Check "Translate to amino when aligning" checkbox
//    Align
}

GUI_TEST_CLASS_DEFINITION(test_0055){
//    Open data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Select some area
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(2, 2), QPoint(8, 8));
//    Use context menu:
//    {Export->Export subalignment}
    class custom: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();

            QLineEdit* filepathEdit = GTWidget::findExactWidget<QLineEdit*>(os, "filepathEdit", dialog);
            GTLineEdit::setText(os, filepathEdit, dataDir + "samples/CLUSTALW/COI.aln");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Save subalignment"));

    GTUtilsDialog::waitForDialog(os, new ExtractSelectedAsMSADialogFiller(os, new custom()));
    GTUtilsNotifications::waitForNotification(os, true, "Document is locked:");
    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));
//    In export dialog set output file to
//    "data/samples/CLUSTALW/COI.aln"
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0056){
//    Open murine.gb
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Export sequence as alignment. In export dialog check

    class custom: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();

            QLineEdit* fileNameEdit = GTWidget::findExactWidget<QLineEdit*>(os, "fileNameEdit", dialog);
            GTLineEdit::setText(os, fileNameEdit, sandBoxDir + "murine.aln");

            QCheckBox* genbankBox = GTWidget::findExactWidget<QCheckBox*>(os, "genbankBox", dialog);
            GTCheckBox::setChecked(os, genbankBox, true);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTUtilsDialog::waitForDialog(os, new ExportSequenceAsAlignmentFiller(os, new custom()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_project__export_import_menu_action"
                                                  << "export sequences as alignment"));
    GTUtilsProjectTreeView::click(os, "murine.gb", Qt::RightButton);
    GTGlobals::sleep();
//    "Use Genbank "SOURCE" tags..." checkbox
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(nameList.size() == 1, QString("unexpected number of names: %1").arg(nameList.size()));
    CHECK_SET_ERR(nameList.first() == "Murine_sarcoma_virus.", "unexpected sequence name: " + nameList.first());
}

GUI_TEST_CLASS_DEFINITION(test_0057){


    class custom: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            GTGlobals::sleep();

            QRadioButton* join2alignmentMode = GTWidget::findExactWidget<QRadioButton*>(os, "join2alignmentMode", dialog);
            GTRadioButton::click(os, join2alignmentMode);
            GTGlobals::sleep();

            QLineEdit* newDocUrl = GTWidget::findExactWidget<QLineEdit*>(os, "newDocUrl", dialog);
            GTLineEdit::setText(os, newDocUrl, sandBoxDir + "test_0057.aln");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };
    GTUtilsDialog::waitForDialog(os, new GTSequenceReadingModeDialogUtils(os, new custom()));
    GTFileDialog::openFileList(os, dataDir + "samples/Genbank", QStringList() << "murine.gb" << "sars.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSelection(os, QPoint(0,0), QPoint(10, 1), "AAATGAAAGAC\nATATTAGGTTT");

}

GUI_TEST_CLASS_DEFINITION(test_0058){

    class custom: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            GTGlobals::sleep(500);

            QWidget* logoWidget = GTWidget::findWidget(os, "logoWidget", dialog);
            int initHeight = logoWidget->geometry().height();
            CHECK_SET_ERR(initHeight==0, QString("logoWidget has too big height: %1").arg(initHeight));

            GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/clustal/test_1393.aln"));
            GTWidget::click(os, GTWidget::findWidget(os, "inputButton", dialog));
            GTGlobals::sleep(500);

            int finalHeight = logoWidget->geometry().height();
            CHECK_SET_ERR(finalHeight==150, QString("logoWidget has wrong height after choosing file: %1").arg(finalHeight));

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };
    GTUtilsDialog::waitForDialog(os, new PwmBuildDialogFiller(os, new custom()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "TFBS_MENU" << "TFBS_WEIGHT"));
    GTMenu::showMainMenu(os, MWMENU_TOOLS);
//    Use main menu {Tools->Search for TFBS->Build weigth mantix}
//    In "Weight matrix" dialog set input amino alignment
//    shorter then 50.
//    Expected state: weight matrix logo appeared in dialog
//    Change input file
//    Expected state: logo updated
}

GUI_TEST_CLASS_DEFINITION(test_0059){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Create new color scheme. Set some new color for some

//    character.
//    Press "Clear" button. check state

    class customColorSelector: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            GTGlobals::sleep(500);
            QWidget *dialog = QApplication::activeModalWidget();

            QWidget* alphabetColorsFrame = GTWidget::findWidget(os, "alphabetColorsFrame", dialog);

            int cellWidth = alphabetColorsFrame->geometry().width()/6;
            QStringList initialColors;
            initialColors << "#ffffff" << "#fcff92" << "#70f970" << "#4eade1" << "#fcfcfc" << "#ff99b1";
            QString finalColor =  "#ffffff";

            GTWidget::click(os, GTWidget::findWidget(os, "clearButton", dialog));
            GTGlobals::sleep(200);
            for(double i=0; i<6; i++){
                QPoint p = QPoint((i+0.5)*cellWidth, 10);
                QColor c = GTWidget::getColor(os, dialog, alphabetColorsFrame->mapTo(dialog, p));
                CHECK_SET_ERR(c.name() == finalColor, QString("unexpected color at cell %1 after clearing: %2").arg(i).arg(c.name()));
                uiLog.trace(c.name());
            }

            GTWidget::click(os, GTWidget::findWidget(os, "restoreButton", dialog));
            GTGlobals::sleep(200);
            for(double i=0; i<6; i++){
                QPoint p = QPoint((i+0.5)*cellWidth, 10);
                QColor c = GTWidget::getColor(os, dialog, alphabetColorsFrame->mapTo(dialog, p));
                CHECK_SET_ERR(c.name() == initialColors[i], QString("unexpected color at cell %1 after clearing: %2, expected: %3").
                              arg(i).arg(c.name()).arg(initialColors[i]));
                uiLog.trace(c.name());
            }

            GTUtilsDialog::waitForDialog(os, new ColorDialogFiller(os, 255, 0, 0));
            QPoint cell2 = QPoint(1.5*cellWidth, 10);
            GTMouseDriver::moveTo(os, alphabetColorsFrame->mapToGlobal(cell2));
            GTMouseDriver::click(os);
            GTGlobals::sleep(500);
            QColor cell2Color = GTWidget::getColor(os, dialog, alphabetColorsFrame->mapTo(dialog, cell2));
            CHECK_SET_ERR(cell2Color.name() == "#ff0000", "color was chanded wrong: " + cell2Color.name());

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    class customColorSchemeCreator: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            GTGlobals::sleep(500);
            QWidget *dialog = QApplication::activeModalWidget();

            QLineEdit* schemeName = GTWidget::findExactWidget<QLineEdit*>(os, "schemeName",dialog);
            GTLineEdit::setText(os, schemeName, "GUITest_common_scenarios_msa_editor_test_0059_scheme");

            QComboBox *alphabetComboBox = (GTWidget::findExactWidget<QComboBox*>(os,"alphabetComboBox",dialog));
            GTComboBox::setIndexWithText(os, alphabetComboBox, "Nucleotide");

            GTUtilsDialog::waitForDialog(os, new ColorSchemeDialogFiller(os, new customColorSelector()));

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
            GTGlobals::sleep(500);
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    class customAppSettingsFiller: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            GTGlobals::sleep(500);

            GTUtilsDialog::waitForDialog(os, new CreateAlignmentColorSchemeDialogFiller(os, new customColorSchemeCreator()));

            GTWidget::click(os, GTWidget::findWidget(os, "addSchemaButton", dialog));
            GTGlobals::sleep(500);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new customAppSettingsFiller()));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Colors" << "Custom schemes" << "Create new color scheme"));
    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));

    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_0060){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Open "Color schemes" dialog.
    class customAppSettingsFiller: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            GTGlobals::sleep(500);

            GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, sandBoxDir, "", GTFileDialogUtils::Choose));
            GTWidget::click(os, GTWidget::findWidget(os, "colorsDirButton", dialog));

            GTGlobals::sleep(500);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new customAppSettingsFiller()));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Colors" << "Custom schemes" << "Create new color scheme"));
//    Select some color scheme directory. Check state
    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));

    GTUtilsDialog::waitForDialog(os, new NewColorSchemeCreator(os, "GUITest_common_scenarios_msa_editor_test_0060", NewColorSchemeCreator::nucl));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Colors" << "Custom schemes" << "Create new color scheme"));
    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));

    GTGlobals::sleep(500);

    GTFile::check(os, sandBoxDir + "GUITest_common_scenarios_msa_editor_test_0060.csmsa");



    class customAppSettingsFiller1: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            GTGlobals::sleep(500);

            QLineEdit* colorsDirEdit = GTWidget::findExactWidget<QLineEdit*>(os, "colorsDirEdit", dialog);
            QString path = colorsDirEdit->text();
            CHECK_SET_ERR(path.contains("_common_data/scenarios/sandbox"), "unexpected color directory: " + path);

            GTGlobals::sleep(500);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new customAppSettingsFiller1()));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Colors" << "Custom schemes" << "Create new color scheme"));
//    Select some color scheme directory. Check state
    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));
}

GUI_TEST_CLASS_DEFINITION(test_0061){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Open "Color schemes" dialog.
//    Open "Create color scheme" dialog.
//    Set wrong scheme names: space only, empty, with forbidden
//    characters, duplicating existing scnemes.
//    Check error hint in dialog

    GTUtilsDialog::waitForDialog(os, new NewColorSchemeCreator(os, "GUITest_common_scenarios_msa_editor_test_0061", NewColorSchemeCreator::nucl));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Colors" << "Custom schemes" << "Create new color scheme"));
    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));


    class customColorSchemeCreator: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            GTGlobals::sleep(500);
            QWidget *dialog = QApplication::activeModalWidget();

            QLabel* validLabel = GTWidget::findExactWidget<QLabel*>(os, "validLabel", dialog);
            QLineEdit* schemeName = GTWidget::findExactWidget<QLineEdit*>(os, "schemeName",dialog);

            GTLineEdit::setText(os, schemeName, "   ");
            CHECK_SET_ERR(validLabel->text() == "Warning: Name can't contain only spaces.", "unexpected hint: " + validLabel->text());
            GTLineEdit::setText(os, schemeName, "");
            CHECK_SET_ERR(validLabel->text() == "Warning: Name of scheme is empty.", "unexpected hint: " + validLabel->text());
            GTLineEdit::setText(os, schemeName, "name*");
            CHECK_SET_ERR(validLabel->text() == "Warning: Name has to consist of letters, digits, spaces<br>or underscore symbols only.", "unexpected hint: " + validLabel->text());
            GTLineEdit::setText(os, schemeName, "GUITest_common_scenarios_msa_editor_test_0061");
            CHECK_SET_ERR(validLabel->text() == "Warning: Color scheme with the same name already exists.", "unexpected hint: " + validLabel->text());

            QComboBox *alphabetComboBox = (GTWidget::findExactWidget<QComboBox*>(os,"alphabetComboBox",dialog));
            GTComboBox::setIndexWithText(os, alphabetComboBox, "Nucleotide");

            GTGlobals::sleep(500);
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    class customAppSettingsFiller: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            GTGlobals::sleep(500);

            GTUtilsDialog::waitForDialog(os, new CreateAlignmentColorSchemeDialogFiller(os, new customColorSchemeCreator()));

            GTWidget::click(os, GTWidget::findWidget(os, "addSchemaButton", dialog));
            GTGlobals::sleep(500);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new customAppSettingsFiller()));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Colors" << "Custom schemes" << "Create new color scheme"));
    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));

    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_0062){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    QDir().mkpath(sandBoxDir + "read_only_dir");
    GTFile::setReadOnly(os, sandBoxDir + "read_only_dir");
//    Open "Export subalignment" dialog
    class custom: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            GTGlobals::sleep(500);
            QLineEdit* filepathEdit = GTWidget::findExactWidget<QLineEdit*>(os, "filepathEdit", dialog);
//    Check wrong parameters:
//    Dir to save does not exists
            GTLineEdit::setText(os, filepathEdit, sandBoxDir + "some_dir/subalignment.aln");
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "Directory to save does not exist"));
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
            GTGlobals::sleep(500);
//    No permission  to write to directory
            GTLineEdit::setText(os, filepathEdit, sandBoxDir + "read_only_dir/subalignment.aln");
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "No write permission to "));
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
            GTGlobals::sleep(500);
//    Empty file path
            GTLineEdit::setText(os, filepathEdit, "");
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "No path specified"));
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
            GTGlobals::sleep(500);
//    Filename is empty
            GTLineEdit::setText(os, filepathEdit, sandBoxDir);
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "Filename to save is empty"));
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
            GTGlobals::sleep(500);
//    Select 0 sequences
            GTLineEdit::setText(os, filepathEdit, sandBoxDir + "subalignment.aln");

            GTWidget::click(os, GTWidget::findWidget(os, "noneButton", dialog));
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "You must select at least one sequence"));
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
//    Start pos > end pos
            QSpinBox* startPosBox = GTWidget::findExactWidget<QSpinBox*>(os, "startPosBox", dialog);
            GTSpinBox::setValue(os, startPosBox, 50, GTGlobals::UseKeyBoard);
            QSpinBox* endPosBox = GTWidget::findExactWidget<QSpinBox*>(os, "endPosBox", dialog);
            GTSpinBox::setValue(os, endPosBox, 40, GTGlobals::UseKeyBoard);


            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "Start position must be less than end position!"));
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
            GTGlobals::sleep(500);


            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    GTUtilsDialog::waitForDialog(os, new ExtractSelectedAsMSADialogFiller(os, new custom()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Save subalignment"));

    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));

    GTGlobals::sleep(500);

}

GUI_TEST_CLASS_DEFINITION(test_0063){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Press "align" button on toolbar. Check state

    class custom: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            QStringList expectedActions = QStringList() << "Align with muscle"
                                                   << "Align sequences to profile with MUSCLE"
                                                   << "Align profile to profile with MUSCLE"
                                                   << "Align with ClustalW"
                                                   << "Align with ClustalO"
                                                   << "Align with MAFFT"
                                                   << "Align with T-Coffee"
                                                   << "align_with_kalign";
            QMenu* m = qobject_cast<QMenu*>(QApplication::activePopupWidget());
            CHECK_SET_ERR(m != NULL, "menu not found");
            QList<QAction*> menuActions = m->actions();
            CHECK_SET_ERR(menuActions.size() == 8, QString("unexpected number of actions: %1").arg(menuActions.size()));
            foreach (QAction* act, menuActions) {
                CHECK_SET_ERR(expectedActions.contains(act->objectName()), act->objectName() + " unexpectidly found in menu");
            }

            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);
        }
    };

    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, new custom()));
    GTWidget::click(os, GTAction::button(os, "Align"));

    GTGlobals::sleep(500);

}

GUI_TEST_CLASS_DEFINITION(test_0064){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Open "Statistics" OP tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::Statistics);
//    Set some reference sequence
    GTUtilsOptionPanelMsa::addReference(os, "Phaneroptera_falcata");

//    Click "Show distance column". Check state
    QCheckBox* showDistancesColumnCheck = GTWidget::findExactWidget<QCheckBox*>(os, "showDistancesColumnCheck");
    GTCheckBox::setChecked(os, showDistancesColumnCheck, true);
    QString val1 = GTUtilsMSAEditorSequenceArea::getSimilarityValue(os, 0);
    QString val2 = GTUtilsMSAEditorSequenceArea::getSimilarityValue(os, 2);
    CHECK_SET_ERR(val1 == "0%", "1: unexpected valeu1: " + val1);
    CHECK_SET_ERR(val2 == "19%", "1: unexpected valeu2: " + val2);
//    Click "Show distance column". Check state
    GTCheckBox::setChecked(os, showDistancesColumnCheck, false);
    QWidget* column = GTWidget::findWidget(os, "msa_editor_similarity_column");
    CHECK_SET_ERR(!column->isVisible(), "similarity column unexpectidly found");
//    Click "Show distance column". Check state
    GTCheckBox::setChecked(os, showDistancesColumnCheck, true);
    val1 = GTUtilsMSAEditorSequenceArea::getSimilarityValue(os, 0);
    val2 = GTUtilsMSAEditorSequenceArea::getSimilarityValue(os, 2);
    CHECK_SET_ERR(val1 == "0%", "2: unexpected valeu1: " + val1);
    CHECK_SET_ERR(val2 == "19%", "2: unexpected valeu2: " + val2);
}

GUI_TEST_CLASS_DEFINITION(test_0065){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Use context menu: {Copy->Copy consensus with gaps}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "Copy consensus with gaps"));

    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));
//    Check clipboard
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText.startsWith("TaAGttTatTaATtCGagCtGAAtTagG+CAaCCaGGtTat---+TaATT"), "unexpected consensus was exported: " + clipboardText);
}

GUI_TEST_CLASS_DEFINITION(test_0066){
//    Open COI.aln consArea
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Select some area on consensus with mouse
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(1, -3), QPoint(10, -3));
//    Check selection on consensus and alignment
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(QPoint(1, 0), QPoint(10, 17)));
}

GUI_TEST_CLASS_DEFINITION(test_0067){
//TODO: write this test when UGENE-4803 is fixed
//    Open COI.aln
//    Build tree displayed with msa
//    Use context menu on tree tab(in tabWidget)
//    Check all actions in popup menu
}

GUI_TEST_CLASS_DEFINITION(test_0068){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Press on some sequence in nameList with right button
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-5, 2));
//    Use context menu: {Copy->Copy current sequence}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_COPY << "Copy current sequence"));
    GTMouseDriver::click(os, Qt::RightButton);
//    Check the clipboard
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText.startsWith("TTAGTTTATTAATTCGAGCTGAACTAGGTCAACCAGGCTAT---TTAATTGGTGACGATCAAATTTACAAT"), "unexpected clipboard text: " + clipboardText);
}

GUI_TEST_CLASS_DEFINITION(test_0069){
//    Open COI.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa", "Chikungunya_E1.fasta");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Press on some sequence in nameList
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-5, 2));
    QScrollBar* hscroll = GTWidget::findExactWidget<QScrollBar*>(os, "horizontal_names_scroll");
    QScrollBar* vscroll = GTWidget::findExactWidget<QScrollBar*>(os, "vertical_sequence_scroll");
//    Check keys:
//    right,
    for(int i = 0; i<3; i++){
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["right"]);
        GTGlobals::sleep(100);
    }
    CHECK_SET_ERR(hscroll->value() == 3, QString("right key works wrong. Scrollbar has value: %1").arg(hscroll->value()));
//    left
    for(int i = 0; i<2; i++){
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["left"]);
        GTGlobals::sleep(100);
    }
    CHECK_SET_ERR(hscroll->value() == 1, QString("left key works wrong. Scrollbar has value: %1").arg(hscroll->value()));\
//    page down
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["pagedown"]);
    GTGlobals::sleep(500);
    CHECK_SET_ERR(vscroll->value() > 20, QString("page down key works wrong: %1").arg(vscroll->value()));
//    page up
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["pageup"]);
    GTGlobals::sleep(500);
    CHECK_SET_ERR(vscroll->value() == 0, QString("page up key works wrong: %1").arg(vscroll->value()));
//    end
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["end"]);
    GTGlobals::sleep(500);
    CHECK_SET_ERR(vscroll->value() > 1650, QString("end key works wrong: %1").arg(vscroll->value()));
//    home
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["home"]);
    GTGlobals::sleep(500);
    CHECK_SET_ERR(vscroll->value() == 0, QString("end key works wrong: %1").arg(vscroll->value()));
//    mouse wheel
    for(int i = 0; i<3; i++){
        GTMouseDriver::scroll(os, -1);
        GTGlobals::sleep(100);
    }
    CHECK_SET_ERR(vscroll->value() == 3, QString("scroll down works wrong. Scrollbar has value: %1").arg(vscroll->value()));
    GTGlobals::sleep(500);

    for(int i = 0; i<2; i++){
        GTMouseDriver::scroll(os, 1);
        GTGlobals::sleep(100);
    }
    CHECK_SET_ERR(vscroll->value() == 1, QString("scroll up works wrong. Scrollbar has value: %1").arg(vscroll->value()));
}
GUI_TEST_CLASS_DEFINITION(test_0070){
//    Open empty alignment
    GTFileDialog::openFile(os, testDir + "_common_data/fasta", "empty.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Press on nameList area
    GTWidget::click(os, GTWidget::findWidget(os, "msa_editor_name_list"));
//    Check state
}

GUI_TEST_CLASS_DEFINITION(test_0071){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Click on some character on sequence area
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(2, 2));
//    Press on other character with shift modifier
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(8, 8));
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
//    Expected state: selection is created on these characters
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(QPoint(2, 2), QPoint(8, 8)));
}

GUI_TEST_CLASS_DEFINITION(test_0072){
//    Open COI.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa", "Chikungunya_E1.fasta");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(5, 5));
//    Check keys: arrows
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["up"]);
    GTGlobals::sleep(300);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(QPoint(5,4), QPoint(5,4)));

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["left"]);
    GTGlobals::sleep(300);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(QPoint(4,4), QPoint(4,4)));

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
    GTGlobals::sleep(300);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(QPoint(4,5), QPoint(4,5)));

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["right"]);
    GTGlobals::sleep(300);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(QPoint(5,5), QPoint(5,5)));

//    shift + arrows
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["up"]);
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(QPoint(5,4), QPoint(5,5)));

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["left"]);
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(QPoint(4,4), QPoint(5,5)));

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(QPoint(4,5), QPoint(5,5)));

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["right"]);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep(300);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, QRect(QPoint(5,5), QPoint(5,5)));
//    end
    QScrollBar* hbar = GTWidget::findExactWidget<QScrollBar*>(os, "horizontal_sequence_scroll");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["end"]);
    GTGlobals::sleep(300);
    CHECK_SET_ERR(hbar->value() == hbar->maximum(), QString("end key scrollbar value: %1").arg(hbar->value()))
//    home
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["home"]);
    GTGlobals::sleep(300);
    CHECK_SET_ERR(hbar->value() == 0, QString("home key works wrong. Scrollbar value: %1").arg(hbar->value()))
//    page down
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["pagedown"]);
    GTGlobals::sleep(300);
    CHECK_SET_ERR(hbar->value() > 20, QString("page down key works wrong. Scrollbar value: %1").arg(hbar->value()))
//    page up
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["pageup"]);
    GTGlobals::sleep(300);
    CHECK_SET_ERR(hbar->value() == 0, QString("page down key works wrong. Scrollbar value: %1").arg(hbar->value()))
//  end+shift
    QScrollBar* vbar = GTWidget::findExactWidget<QScrollBar*>(os, "vertical_sequence_scroll");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["end"], GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep(300);
    CHECK_SET_ERR(vbar->value() == vbar->maximum(), QString("shift + end key works wrong. Scrollbar value: %1").arg(vbar->value()))
//  home+shift
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["home"], GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep(300);
    CHECK_SET_ERR(vbar->value() == 0, QString("shift + home key works wrong. Scrollbar value: %1").arg(vbar->value()))
//  page down+shift
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["pagedown"], GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep(300);
    CHECK_SET_ERR(vbar->value() > 20, QString("shift + page down key works wrong. Scrollbar value: %1").arg(vbar->value()))
//  page up + shift
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["pageup"], GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep(300);
    CHECK_SET_ERR(vbar->value() == 0, QString("shift + page down key works wrong. Scrollbar value: %1").arg(vbar->value()))
//  wheel event
    for(int i = 0; i<3; i++){
        GTMouseDriver::scroll(os, -1);
        GTGlobals::sleep(100);
    }
    CHECK_SET_ERR(hbar->value() == 3, QString("scroll down works wrong. Scrollbar has value: %1").arg(hbar->value()));
    GTGlobals::sleep(500);

    for(int i = 0; i<2; i++){
        GTMouseDriver::scroll(os, 1);
        GTGlobals::sleep(100);
    }
    CHECK_SET_ERR(hbar->value() == 1, QString("scroll up works wrong. Scrollbar has value: %1").arg(hbar->value()));
}

GUI_TEST_CLASS_DEFINITION(test_0073){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Unload document
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"action_project__unload_selected_action"));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
    GTUtilsProjectTreeView::click(os, "COI.aln", Qt::RightButton);
//    Use context menu on object: {Open view -> Open new view: Alignment editor}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Open View" << "action_open_view"));
    GTUtilsProjectTreeView::click(os, "COI.aln", Qt::RightButton);
//    Expected: view is opened, document is loaded
    GTUtilsMdi::findWindow(os, "COI [m] COI");
}

GUI_TEST_CLASS_DEFINITION(test_0074){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(0, 5));
//    Open "Export subalignment" dialog
    class custom: public CustomScenario{
    public:
        virtual void run(HI::GUITestOpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            GTGlobals::sleep(500);

            QStringList list = ExtractSelectedAsMSADialogFiller::getSequences(os, true);
            CHECK_SET_ERR(list.first() == "Phaneroptera_falcata", "unexpected first sequence: " + list.first());
            CHECK_SET_ERR(list.last() == "Metrioptera_japonica_EF540831", "unexpected last sequence: " + list.last());
            CHECK_SET_ERR(list.size() == 6, QString("Unexpected initial list size: %1").arg(list.size()));
//    Press "Invert selection" button. Expected: selection is inverted
            GTWidget::click(os, GTWidget::findWidget(os, "invertButton", dialog));
            list = ExtractSelectedAsMSADialogFiller::getSequences(os, true);
            CHECK_SET_ERR(list.first() == "Gampsocleis_sedakovii_EF540828", "unexpected first sequence(inverted): " + list.first());
            CHECK_SET_ERR(list.last() == "Hetrodes_pupus_EF540832", "unexpected last sequence(inverted): " + list.last());
            CHECK_SET_ERR(list.size() == 12, QString("Unexpected initial list size: %1").arg(list.size()));
//    Press "Select all" button. Expected: all sequences selected
            GTWidget::click(os, GTWidget::findWidget(os, "allButton", dialog));
            list = ExtractSelectedAsMSADialogFiller::getSequences(os, true);
            CHECK_SET_ERR(list.first() == "Phaneroptera_falcata", "unexpected first sequence(all): " + list.first());
            CHECK_SET_ERR(list.last() == "Hetrodes_pupus_EF540832", "unexpected last sequence(all): " + list.last());
            CHECK_SET_ERR(list.size() == 18, QString("Unexpected initial list size: %1").arg(list.size()));

            GTWidget::click(os, GTWidget::findWidget(os, "noneButton", dialog));
            list = ExtractSelectedAsMSADialogFiller::getSequences(os, true);
            CHECK_SET_ERR(list.size() == 0, QString("list is not cleared: %1").arg(list.size()));

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    GTUtilsDialog::waitForDialog(os, new ExtractSelectedAsMSADialogFiller(os, new custom()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Save subalignment"));

    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));

    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_0075){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    QWidget* overview = GTWidget::findWidget(os, "msa_overview_area_graph");
    QImage init = GTWidget::getImage(os, overview);
//    Use context menu on overview: {Calculation method->Clustal}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Calculation method" << "Clustal"));
    GTMenu::showContextMenu(os, overview);
//    Check state
    QImage clustal = GTWidget::getImage(os, overview);
    CHECK_SET_ERR(init != clustal, "overview was not changed(clustal)");
//    Use context menu on overview: {Display settings...->Graph type->Histogram}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Display settings" << "Graph type" << "Histogram"));
    GTMenu::showContextMenu(os, overview);
//    Check state
    QImage histogram = GTWidget::getImage(os, overview);
    CHECK_SET_ERR(histogram != clustal, "overview was not changed(histogram)");

}

GUI_TEST_CLASS_DEFINITION(test_0076){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    QWidget* overview = GTWidget::findWidget(os, "msa_overview_area_graph");
//    Show simple overview
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show simple overview"));
    GTMenu::showContextMenu(os, overview);
    GTGlobals::sleep(200);
    QWidget* simple = GTWidget::findWidget(os, "msa_overview_area_simple");
    QColor initColor = GTWidget::getColor(os, simple, simple->geometry().center());
    QString initColorS = initColor.name();
//    Press on overview with mouse left button

    GTWidget::click(os, overview);
    QColor finalColor = GTWidget::getColor(os, simple, simple->geometry().center());
    QString finalColorS = finalColor.name();
    CHECK_SET_ERR(initColorS != finalColorS, "color was not changed(1)");
//    Expected state: visible range moved
//    Drag visible range with mouse
    QColor initColor1 = GTWidget::getColor(os, simple, simple->geometry().topLeft() + QPoint(5, 5));
    QString initColorS1 = initColor1.name();
    GTMouseDriver::press(os);
    GTMouseDriver::moveTo(os, QPoint(10, GTMouseDriver::getMousePosition().y()));
    GTMouseDriver::release(os);
//    Expected state: visible range dragged
    QColor finalColor1 = GTWidget::getColor(os, simple, simple->geometry().topLeft() + QPoint(5, 5));
    QString finalColorS1 = finalColor1.name();
    CHECK_SET_ERR(initColorS1 != finalColorS1, "color was not changed(2)")

}

GUI_TEST_CLASS_DEFINITION(test_0077){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Open tree with msa
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk", 0, 0, true));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep(500);
//    Press "refresh tree" button on tree view toolbar
    QWidget* button = GTAction::button(os, "Refresh tree");
    bool vis = button->isVisible();
    if(vis){
        GTWidget::click(os, button);
    }else{
        QWidget* extButton = GTWidget::findWidget(os, "qt_toolbar_ext_button", GTWidget::findWidget(os, "msa_editor_tree_view_container_widget"));
        GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Refresh tree"));
        GTWidget::click(os, extButton);
    }
    GTGlobals::sleep(1000);
//    Expected state: tree refreshed
}

GUI_TEST_CLASS_DEFINITION(test_0078){
//    Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Open tree with msa
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk", 0, 0, true));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep();
//    Shrink tree view to show horizontal scrollbar
//    Move wheel
    QWidget* parent = GTWidget::findWidget(os, "qt_scrollarea_hcontainer", GTWidget::findWidget(os, "treeView"));
    QScrollBar* hbar = parent->findChild<QScrollBar*>();
    int val = hbar->value();
    GTGlobals::sleep();

    GTWidget::click(os, GTWidget::findWidget(os, "treeView"));
    for(int i = 0; i<2; i++){
        GTMouseDriver::scroll(os, 1);
        GTGlobals::sleep(100);
    }
    int val1 = hbar->value();
    CHECK_SET_ERR(val1 < val, QString("unexpected scroll value: %1").arg(val1));
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0079) {
    // Open an alignment with some alphabet.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    // Do the action for a sequence (or sequences) of the same alphabet.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(7, 3), QPoint(12, 7));

    GTClipboard::setText(os, ">human_T1\r\nACGTACG\r\n");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_COPY" << "paste"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();
    GTUtilsTaskTreeView::waitTaskFinished(os);
    const QStringList sequencesNameList = GTUtilsMSAEditorSequenceArea::getNameList(os);

    // The sequence was added to the bottom of the alignment.
    CHECK_SET_ERR(sequencesNameList.length() > 0, "No sequences");
    CHECK_SET_ERR(sequencesNameList.last() == "human_T1", "No pasted sequences");
}

GUI_TEST_CLASS_DEFINITION(test_0080) {
    // Open an alignment with some alphabet.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    // Use a sequence of another alphabet.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(7, 3), QPoint(12, 7));

    GTClipboard::setText(os, ">human_T1\r\nACGTACS\r\n");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_COPY" << "paste"));

    // A warning notification appears:
    GTUtilsNotifications::waitForNotification(os, true, "from \"Standard DNA\" to \"Extended DNA\"");

    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    const QStringList sequencesNameList = GTUtilsMSAEditorSequenceArea::getNameList(os);

    // The sequence was added to the bottom of the alignment.
    CHECK_SET_ERR(sequencesNameList.length() > 0, "No sequences");
    CHECK_SET_ERR(sequencesNameList.last() == "human_T1", "No pasted sequences");
}

GUI_TEST_CLASS_DEFINITION(test_0081) {
    // Open an alignment with some alphabet.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    // Use a sequence of another alphabet.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(7, 3), QPoint(12, 7));

    GTClipboard::setText(os, ">human_T1\r\nACGTACS\r\n>human_T2\r\nACGTAC\r\n");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_COPY" << "paste"));

    // A warning notification appears:
    GTUtilsNotifications::waitForNotification(os, true, "from \"Standard DNA\" to \"Extended DNA\"");

    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    const QStringList sequencesNameList = GTUtilsMSAEditorSequenceArea::getNameList(os);

    // The sequence was added to the bottom of the alignment.
    CHECK_SET_ERR(sequencesNameList.length() > 0, "No sequences");
    CHECK_SET_ERR(sequencesNameList.last() == "human_T2", "No pasted sequences");
}

GUI_TEST_CLASS_DEFINITION(test_0082) {
    // Open an alignment with some alphabet.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    // Use a sequence of another alphabet.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(7, 3), QPoint(12, 7));

    GTClipboard::setText(os, ">human_T1\r\nACGTAC\r\n>human_T2\r\nACGTACS\r\n>human_T3\r\nACGTAC\r\n");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_COPY" << "paste"));

    // A warning notification appears:
    GTUtilsNotifications::waitForNotification(os, true, "from \"Standard DNA\" to \"Extended DNA\"");

    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    const QStringList sequencesNameList = GTUtilsMSAEditorSequenceArea::getNameList(os);

    // The sequence was added to the bottom of the alignment.
    CHECK_SET_ERR(sequencesNameList.length() > 0, "No sequences");
    CHECK_SET_ERR(sequencesNameList.last() == "human_T3", "No pasted sequences");
}

GUI_TEST_CLASS_DEFINITION(test_0083) {
    // Open an alignment with some alphabet.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    // Use a sequence of another alphabet.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(7, 3), QPoint(12, 7));

    GTClipboard::setText(os, ">human_T1\r\nACGTAC\r\n>human_T2\r\nACGTACS\r\n>human_T3\r\nQQ\r\n");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_COPY" << "paste"));

    // A warning notification appears:
    GTUtilsNotifications::waitForNotification(os, true, "from \"Standard DNA\" to \"Raw\"");

    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    const QStringList sequencesNameList = GTUtilsMSAEditorSequenceArea::getNameList(os);

    // The sequence was added to the bottom of the alignment.
    CHECK_SET_ERR(sequencesNameList.length() > 0, "No sequences");
    CHECK_SET_ERR(sequencesNameList.last() == "human_T3", "No pasted sequences");
}


GUI_TEST_CLASS_DEFINITION(test_fake) {
    Q_UNUSED(os);
}

} // namespace GUITest_common_scenarios_msa_editor
} // namespace U2
