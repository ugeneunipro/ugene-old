/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QComboBox>
#include <QSpinBox>
#include <QApplication>
#include <QCheckBox>
#include <QTableWidget>

#include "GTTestsMSAEditorReplaceCharacter.h"
#include "drivers/GTMouseDriver.h"
#include "drivers/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTMenu.h"
#include "GTGlobals.h"
#include "api/GTClipboard.h"
#include "GTUtilsDialog.h"
#include "GTUtilsNotifications.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditor.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsProjectTreeView.h"
#include "runnables/qt/PopupChooser.h"

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>

namespace U2 {

namespace GUITest_common_scenarios_msa_editor_replace_character {
using namespace HI;

GUI_TEST_CLASS_DEFINITION(test_0001){
    //1. Open an alignment in the Alignment Editor.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select one character.
    //Expected result : the character is selected in the normal mode(i.e.borders of the character are drawn using a dashed line).
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(9, 9), QPoint(9, 9));

    //3. Press Shift + R keys on the keyboard.
    //Expected result : the character is selected in the replacement mode(i.e.the border of the character are drawn using another color and / or bold).
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    //4. Press a key on the keyboard with another character of the same alphabet(e.g C key).
    //Expected result : the original character of the alignment was replaced with the new one(e.g 'A' was replaced with 'C').Selection is in normal mode.
    GTKeyboardDriver::keyClick(os, 'c');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "C", QString("Incorrect selection content: expected - %1, received - %2").arg("C").arg(selectionContent));
}

GUI_TEST_CLASS_DEFINITION(test_0002){
    //1. Open an alignment in the Alignment Editor.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select one character.
    //Expected result : the character is selected in the normal mode(i.e.borders of the character are drawn using a dashed line).
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(9, 9), QPoint(9, 9));

    //3. Open the context menu in the sequence area.
    //Expected result : the menu contains an item "Edit > Replace character".The item is enabled.A hotkey Shift + R is shown nearby.
    //4. Select the item.
    //Expected result : the character is selected in the replacement mode.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_character"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_editor_sequence_area"));

    //5. Press a key on the keyboard with another character of the same alphabet(e.g C key).
    //Expected result : the original character of the alignment was replaced with the new one(e.g 'A' was replaced with 'C').Selection is in normal mode.
    GTKeyboardDriver::keyClick(os, 'a');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "A", QString("Incorrect selection content: expected - %1, received - %2").arg("A").arg(selectionContent));
}


GUI_TEST_CLASS_DEFINITION(test_0003){
    //1. Open an alignment in the Alignment Editor.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select one character.
    //Expected result : the character is selected in the normal mode(i.e.borders of the character are drawn using a dashed line).
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(9, 9), QPoint(9, 9));

    //3. Open the main menu in the sequence area.
    //Expected result : the menu contains an item "Actions > Edit > Replace character".The item is enabled.A hotkey Shift + R is shown nearby.
    //4. Select the item.
    //Expected result : the character is selected in the replacement mode.
    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Edit" << "Replace selected character", GTGlobals::UseMouse);

    //5. Press a key on the keyboard with another character of the same alphabet(e.g C key).
    //Expected result : the original character of the alignment was replaced with the new one(e.g 'A' was replaced with 'C').Selection is in normal mode.
    GTKeyboardDriver::keyClick(os, 'g');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "G", QString("Incorrect selection content: expected - %1, received - %2").arg("G").arg(selectionContent));
}

GUI_TEST_CLASS_DEFINITION(test_0004){
    //1. Open an alignment in the Alignment Editor.

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select a region with more than one character.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(9, 9), QPoint(10, 10));

    //3. Open the context menu in the sequence area.
    //Expected result : the "Edit > Replace character" item is disabled.Selection is in normal mode.
    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_character", PopupChecker::IsDisabled));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_editor_sequence_area"));
}

GUI_TEST_CLASS_DEFINITION(test_0005){
    //Gui test will be added after commit UGENE-4804
}

GUI_TEST_CLASS_DEFINITION(test_0006){
    //Gui test will be added after commit UGENE-4804
}

GUI_TEST_CLASS_DEFINITION(test_0007){
    //1. Open an alignment.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select a character and make active the replace mode for it.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(9, 9), QPoint(9, 9));

    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    //3. Click another character in the alignment.
    //Expected result : the first character is no more in the replacement mode.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(9, 8), QPoint(9, 8));

    GTKeyboardDriver::keyClick(os, 'c');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "T", QString("Incorrect selection content: expected - %1, received - %2").arg("T").arg(selectionContent));
}
GUI_TEST_CLASS_DEFINITION(test_0008){
    //1. Open an alignment.

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select a gap after sequence character.
    GTUtilsMSAEditorSequenceArea::clickToPosition(os, QPoint(41, 9));

    //3. Replace the gap
    //Expected result : the gap is succesfully replaced.
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    GTKeyboardDriver::keyClick(os, 'c');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "C", QString("Incorrect selection content: expected - %1, received - %2").arg("C").arg(selectionContent));
}
GUI_TEST_CLASS_DEFINITION(test_0009){
    //1. Open an alignment.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select a gap before sequence character.
    GTUtilsMSAEditorSequenceArea::clickToPosition(os, QPoint(43, 9));

    //3. Replace the gap
    //Expected result : the gap is succesfully replaced.
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    GTKeyboardDriver::keyClick(os, 'c');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "C", QString("Incorrect selection content: expected - %1, received - %2").arg("C").arg(selectionContent));
}
GUI_TEST_CLASS_DEFINITION(test_0010){
    //1. Open an alignment.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select a gap between other gaps.
    GTUtilsMSAEditorSequenceArea::clickToPosition(os, QPoint(42, 9));

    //3. Replace the gap
    //Expected result : the gap is succesfully replaced.
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    GTKeyboardDriver::keyClick(os, 'c');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "C", QString("Incorrect selection content: expected - %1, received - %2").arg("C").arg(selectionContent));
}
GUI_TEST_CLASS_DEFINITION(test_0011){
    //1. Open an alignment.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select a gap after the last sequence character.
    GTUtilsMSAEditorSequenceArea::clickToPosition(os, QPoint(603, 9));

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["up"]);
    GTGlobals::sleep(500);

    //3. Replace the gap
    //Expected result : the gap is succesfully replaced.
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep(1000);
    GTKeyboardDriver::keyClick(os, 'c');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "C", QString("Incorrect selection content: expected - %1, received - %2").arg("C").arg(selectionContent));
}
GUI_TEST_CLASS_DEFINITION(test_0012){
    //1. Open an alignment.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select a gap before the first sequence character.
    GTUtilsMSAEditorSequenceArea::selectColumnInConsensus(os, 0);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 9), QPoint(0, 9));

    //3. Replace the gap
    //Expected result : the gap is succesfully replaced.
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    GTKeyboardDriver::keyClick(os, 'c');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "C", QString("Incorrect selection content: expected - %1, received - %2").arg("C").arg(selectionContent));
}
GUI_TEST_CLASS_DEFINITION(test_0013){
    //1. Open an alignment.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select a first character in sequence.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 9), QPoint(0, 9));

    //3. Replace the character
    //Expected result : the character is succesfully replaced.
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    GTKeyboardDriver::keyClick(os, 'c');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "C", QString("Incorrect selection content: expected - %1, received - %2").arg("C").arg(selectionContent));
}

GUI_TEST_CLASS_DEFINITION(test_0014){
    //1. Open an alignment.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select a last character in sequence.
    GTUtilsMSAEditorSequenceArea::clickToPosition(os, QPoint(603, 9));

    //3. Replace the character
    //Expected result : the character is succesfully replaced.
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    GTKeyboardDriver::keyClick(os, 'c');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "C", QString("Incorrect selection content: expected - %1, received - %2").arg("C").arg(selectionContent));
}

GUI_TEST_CLASS_DEFINITION(test_0015){
    //1. Open an alignment.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    GTUtilsMsaEditor::toggleCollapsingMode(os);

    //2. Enable collapsing mode
    GTUtilsMSAEditorSequenceArea::clickCollapceTriangle(os, "Mecopoda_elongata__Ishigaki__J");

    //3. Select a character in sequence inside of the collapsing group.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(9, 14), QPoint(9, 14));

    //4. Replace the character
    //Expected result : the character is replaced in all sequences of the group.
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    GTKeyboardDriver::keyClick(os, 'c');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "C", QString("Incorrect selection content: expected - %1, received - %2").arg("C").arg(selectionContent));

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(9, 13), QPoint(9, 13));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "C", QString("Incorrect selection content: expected - %1, received - %2").arg("C").arg(selectionContent));
}

GUI_TEST_CLASS_DEFINITION(test_0016){
    //1. Open an alignment.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select a character in sequence.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 9), QPoint(0, 9));

    //3. Replace the character to gap by space key
    //Expected result : the character is succesfully replaced.
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "-", QString("Incorrect selection content: expected - %1, received - %2").arg("-").arg(selectionContent));

    //3. Select a character in sequence.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 10), QPoint(0, 10));

    //4. Replace the character to gap by '-' key
    //Expected result : the character is succesfully replaced.
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    GTKeyboardDriver::keyClick(os, '-');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "-", QString("Incorrect selection content: expected - %1, received - %2").arg("-").arg(selectionContent));
}

GUI_TEST_CLASS_DEFINITION(test_0017){
    //1. Open an alignment.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select a first character in sequence.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 9), QPoint(0, 9));

    //3. Press unsupported key
    //Expected result : the error notification appeared.
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    GTUtilsNotifications::waitForNotification(os, false, "It is not possible to insert the character into the alignment");
    GTKeyboardDriver::keyClick(os, ']');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "T", QString("Incorrect selection content: expected - %1, received - %2").arg("T").arg(selectionContent));
}

GUI_TEST_CLASS_DEFINITION(test_0018){
    //1. Open an alignment.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //2. Select a character in sequence.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 9), QPoint(0, 9));

    //3. Press 'Escape'
    //Expected result : edit character mode is ended.
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["shift"]);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);

    GTKeyboardDriver::keyClick(os, 'c');

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text(os);
    CHECK_SET_ERR(selectionContent == "T", QString("Incorrect selection content: expected - %1, received - %2").arg("T").arg(selectionContent));
}

} // namespace
} // namespace U2

