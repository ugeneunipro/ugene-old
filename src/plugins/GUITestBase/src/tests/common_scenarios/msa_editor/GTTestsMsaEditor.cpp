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
#include "api/GTKeyboardDriver.h"
#include "api/GTMenu.h"
#include "api/GTFileDialog.h"
#include "GTUtilsDialogRunnables.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditorSequenceArea.h"

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

    GTUtilsDialogRunnables::PopupChooser chooser(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");
    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();
    GTGlobals::sleep();

    bool offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == false, "Offsets are visible");


    GTUtilsDialogRunnables::PopupChooser chooser2(os, QStringList() << MSAE_MENU_VIEW << "show_offsets");
    GTUtilsDialog::preWaitForDialog(os, &chooser2, GUIDialogWaiter::Popup);

    mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");
    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();
    GTGlobals::sleep();

    offsetsVisible = GTUtilsMSAEditorSequenceArea::offsetsVisible(os);
    CHECK_SET_ERR(offsetsVisible == true, "Offsets are not visible");
}


GUI_TEST_CLASS_DEFINITION(test_0004)
{
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/" , "ma2_gapped.aln");
    GTGlobals::sleep(1000);

    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(mdiWindow != NULL, "MDI window == NULL");

    GTUtilsDialogRunnables::PopupChooser chooser(os, QStringList() << "action_go_to_position");
    GTUtilsDialogRunnables::GoToDialogFiller filler(os, 6);
    GTUtilsDialog::preWaitForDialog(os, &filler);
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);

    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep();
    GTGlobals::sleep();

    QRect expectedRect(5, 0, 1, 1);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, expectedRect);

    GTUtilsDialogRunnables::GoToDialogFiller filler1(os, 6);
    GTUtilsDialog::preWaitForDialog(os, &filler1);
    GTKeyboardDriver::keyClick(os, 'g', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, expectedRect);
}

} // namespace GUITest_common_scenarios_msa_editor
} // namespace U2
