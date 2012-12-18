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
#include "GTTestsMSAEditorEdit.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTTreeWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTGlobals.h"
#include "api/GTClipboard.h"
#include "GTUtilsApp.h"
#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ConsensusSelectorDialogFiller.h"
#include "GTUtilsMdi.h"
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>

namespace U2 {
void test_1(U2OpStatus &os,int i, QString expectedSec, int j=0){

    GTUtilsMSAEditorSequenceArea::click(os, QPoint(j,i));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);

    GTGlobals::sleep(500);
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(0,1));

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,i), QPoint(14, i));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardTest = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardTest==expectedSec,clipboardTest);

}
namespace GUITest_common_scenarios_msa_editor_edit {


GUI_TEST_CLASS_DEFINITION(test_0001){
//Check insert gaps
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_1(os,0,"-AAGACTTCTTTTAA");
//2. Select first symbol for Phaneroptera_falcata sequence. Press "space".
//Expected state: Phaneroptera_falcata -AAGACTTCTTTTAA, sequence length 15, right offset 15
}
GUI_TEST_CLASS_DEFINITION(test_0001_1){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_1(os,3,"-AAGTC---TATTAA");
//DIFFERENCE:Select first symbol for Tettigonia_viridissima sequence. Press "space".
//Expected state: Tettigonia_viridissima --AAGTC---TATTAA, sequence length 15, right offset 15
}
GUI_TEST_CLASS_DEFINITION(test_0001_2){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_1(os,6,"TAGCT-TATTAA--",5);
//DIFFERENCE:Select first symbol for Conocephalus_percaudata sequence. Press "space".
//Expected state: Conocephalus_percaudata TAGCT-TATTAA--, sequence length 14, right offset 14
}

void test_3(U2OpStatus &os, int i=0, QString expectedSec=""){
//2. Select 6th symbol for Phaneroptera_falcata sequence. Use context menu {Edit->Insert gap}.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(13,i));

    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_EDIT" << "fill_selection_with_gaps"));
    GTMenu::showContextMenu(os,seq);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,i), QPoint(14, i));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardTest = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardTest==expectedSec,clipboardTest);
    //Expected state: Phaneroptera_falcata AAGAC-TTCTTTTAA, sequence length 15, right offset 14
}

GUI_TEST_CLASS_DEFINITION(test_0003){
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_3(os,0,"AAGACTTCTTTTA-A");
//Expected state: Phaneroptera_falcata AAGAC-TTCTTTTAA, sequence length 15, right offset 14
}

GUI_TEST_CLASS_DEFINITION(test_0003_1){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_3(os,3,"AAGTC---TATTA-A");
//Expected state: DIFFERENCE: Tettigonia_viridissima AAGTC---TATTA-A, sequence length 15, right offset 14
}

GUI_TEST_CLASS_DEFINITION(test_0003_2){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_3(os,7,"AAGTCTTT---TA-A");
//Expected state: DIFFERENCE: Mecopoda_elongata__Ishigaki__J AAGTCTTT---TA-A, sequence length 15, right offset 14
}

void test_4(U2OpStatus &os, int startPos, int endPos, QString expectedSeq, int i=0, int context=0){
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(startPos,i), QPoint(endPos, i));

    if(context){
        QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");
        GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_EDIT" << "Remove selection"));
        GTMenu::showContextMenu(os,seq);
    }
    else{
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    }

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,i), QPoint(14, i));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardTest = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardTest==expectedSeq,clipboardTest);
}

GUI_TEST_CLASS_DEFINITION(test_0004){
//    1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Select 1..5 region for Phaneroptera_falcata sequence. Press "delete".
    test_4(os,0,4,"TTCTTTTAA-----");
//    Expected state: Phaneroptera_falcata TTCTTTTAA-----, sequence length 14, right offset 9
}

GUI_TEST_CLASS_DEFINITION(test_0004_1){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_4(os,0,4,"---TATTAA-----",3);
//    Expected state: DIFFERENCE: Tettigonia_viridissima ---TATTAA-----, sequence length 14, right offset 9
}

GUI_TEST_CLASS_DEFINITION(test_0004_2){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_4(os,0,4,"TTT---TAA-----",7);
//    Expected state: DIFFERENCE: Mecopoda_elongata__Ishigaki__J TTT---TAA-----, sequence length 14, right offset 9
}

GUI_TEST_CLASS_DEFINITION(test_0005){
//    1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Select 6..9 symbol for Phaneroptera_falcata sequence. Use context menu {Edit->Delete selection}.
    test_4(os,5,8,"AAGACTTTAA----");
//    Expected state: Phaneroptera_falcata AAGACTTTAA----, sequence length 14, right offsets 10
}

GUI_TEST_CLASS_DEFINITION(test_0005_1){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_4(os,5,8,"AAGTCATTAA----",3);
//    Expected state: DIFFERENCE: Tettigonia_viridissima AAGTCATTAA----, sequence length 14, right offset 10
}

GUI_TEST_CLASS_DEFINITION(test_0005_2){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_4(os,5,8,"AAGTC--TAA----",7);
//    Expected state: DIFFERENCE: Mecopoda_elongata__Ishigaki__J AAGTC--TAA----, sequence length 14, right offset 10
}

GUI_TEST_CLASS_DEFINITION(test_0006){
//    1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//2. Select 13..14 symbol for Phaneroptera_falcata sequence. Use context menu {Edit->Delete selection}.
    test_4(os,12,13,"AAGACTTCTTTT--",0,1);
//    Expected state: Phaneroptera_falcata AAGACTTCTTTT--, sequence length 14, right offsets 12
}

GUI_TEST_CLASS_DEFINITION(test_0006_1){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_4(os,12,13,"AAGTC---TATT--",3,1);
//    Expected state: DIFFERENCE: Tettigonia_viridissima AAGTC---TATT--, sequence length 14, right offsets 12
}

GUI_TEST_CLASS_DEFINITION(test_0006_2){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_4(os,12,13,"AAGTCTTT---T--",7,1);
//    Expected state: DIFFERENCE: Mecopoda_elongata__Ishigaki__J AAGTCTTT---T--, sequence length 14, right offsets 12
}
} // namespace
} // namespace U2

