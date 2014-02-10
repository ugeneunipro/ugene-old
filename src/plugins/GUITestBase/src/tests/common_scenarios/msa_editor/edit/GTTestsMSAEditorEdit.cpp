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
#include "api/GTAction.h"
#include "api/GTTreeWidget.h"
#include "api/GTComboBox.h"
#include "api/GTLineEdit.h"
#include "api/GTCheckBox.h"
#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsProjectTreeView.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/DeleteGapsDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ExtractSelectedAsMSADialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/util/RenameSequenceFiller.h"
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>
#include <QComboBox>
#include <QSpinBox>
#include <QApplication>
#include <QCheckBox>
#include <QTableWidget>
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

GUI_TEST_CLASS_DEFINITION(test_0007){
//    Check remove all gaps
//    1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Use msa editor context menu {Edit->Remove all gaps}.
    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_EDIT" << "Remove all gaps"));
    GTMenu::showContextMenu(os,seq);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(13, 9));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardTest = GTClipboard::text(os);
    QString expectedSeq=QString(    "AAGACTTCTTTTAA\n"
                                    "AAGCTTACTAA---\n"
                                    "TAGTTTATTAA---\n"
                                    "AAGTCTATTAA---\n"
                                    "TAGCTTATTAA---\n"
                                    "TAGCTTATTAA---\n"
                                    "TAGCTTATTAA---\n"
                                    "AAGTCTTTTAA---\n"
                                    "AAGAATAATTA---\n"
                                    "AAGCCTTTTAA---");

    CHECK_SET_ERR(clipboardTest==expectedSeq,"\n Expected: \n"+ expectedSeq +"\nFound:\n" + clipboardTest);
/*    Expected state: length 14, right offsets 14
*    Phaneroptera_falcata               AAGACTTCTTTTAA
*    Isophya_altaica_EF540820           AAGCTTACTAA---
*    Bicolorana_bicolor_EF540830        TAGTTTATTAA---
*    Tettigonia_viridissima             AAGTCTATTAA---
*    Conocephalus_discolor              TAGCTTATTAA---
*    Conocephalus_sp.                   TAGCTTATTAA---
*    Conocephalus_percaudata            TAGCTTATTAA---
*    Mecopoda_elongata__Ishigaki__J     AAGTCTTTTAA---
*    Podisma_sapporensis                AAGAATAATTA---
*    Hetrodes_pupus_EF540832            AAGCCTTTTAA--- */

}

GUI_TEST_CLASS_DEFINITION(test_0007_1){
//    Check remove all gaps
//DIFFERENSE:    1. Open document _common_data\scenarios\msa\ma2_gap_col.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gap_col.aln");
//    2. Use msa editor context menu {Edit->Remove all gaps}.
    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_EDIT" << "Remove all gaps"));
    GTMenu::showContextMenu(os,seq);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(12, 9));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardTest = GTClipboard::text(os);
    QString expectedSeq=QString("AAGCTTCTTTTAA\n"
                                "AAGTTACTAA---\n"
                                "TAGTTATTAA---\n"
                                "AAGCTATTAA---\n"
                                "TAGTTATTAA---\n"
                                "TAGTTATTAA---\n"
                                "TAGTTATTAA---\n"
                                "AAGCTTTTAA---\n"
                                "AAGAATAATTA--\n"
                                "AAGCTTTTAA---");

    CHECK_SET_ERR(clipboardTest==expectedSeq,"\n Expected: \n"+ expectedSeq +"\nFound:\n"+clipboardTest);
/*    Expected state: length 14, right offsets 14
*    Phaneroptera_falcata               AAGCTTCTTTTAA
*    Isophya_altaica_EF540820           AAGTTACTAA---
*    Bicolorana_bicolor_EF540830        TAGTTATTAA---
*    Tettigonia_viridissima             AAGCTATTAA---
*    Conocephalus_discolor              TAGTTATTAA---
*    Conocephalus_sp.                   TAGTTATTAA---
*    Conocephalus_percaudata            TAGTTATTAA---
*    Mecopoda_elongata__Ishigaki__J     AAGCTTTTAA---
*    Podisma_sapporensis                AAGAATAATTA--
*    Hetrodes_pupus_EF540832            AAGCTTTTAA--- */

}
GUI_TEST_CLASS_DEFINITION(test_0008){
//Check remove columns with gaps
//1. Open document _common_data\scenarios\msa\ma2_gap_col.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gap_col.aln");
//2. Place cursor on 4th column of alignment. Use msa editor context menu (at the column with gaps) {Edit->Remove column of gaps}.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(3,3));

    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_EDIT" << "remove_columns_of_gaps"));
    GTUtilsDialog::waitForDialog(os,new DeleteGapsDialogFiller(os,1));
    GTMenu::showContextMenu(os,seq);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(12, 9));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardTest = GTClipboard::text(os);
    QString expectedSeq=QString("AAGCTTCTTTTAA\n"
                                "AAGTTACTAA---\n"
                                "TAG---TTATTAA\n"
                                "AAGC---TATTAA\n"
                                "TAGTTATTAA---\n"
                                "TAGTTATTAA---\n"
                                "TAGTTATTAA---\n"
                                "AAGCTTT---TAA\n"
                                "A--AGAATAATTA\n"
                                "AAGCTTTTAA---");

    CHECK_SET_ERR(clipboardTest==expectedSeq,"\n Expected: \n"+ expectedSeq  +"\nFound:\n" + clipboardTest);
//Expected state: "Remove column of gaps" dialog appears

//3. Choose option "Delete all columns of gaps" and click "Delete" button
/*Expected state: length 13, right offsets 13
Phaneroptera_falcata               AAGCTTCTTTTAA
Isophya_altaica_EF540820           AAGTTACTAA---
Bicolorana_bicolor_EF540830        TAG---TTATTAA
Tettigonia_viridissima             AAGC---TATTAA
Conocephalus_discolor              TAGTTATTAA---
Conocephalus_sp.                   TAGTTATTAA---
Conocephalus_percaudata            TAGTTATTAA---
Mecopoda_elongata__Ishigaki__J     AAGCTTT---TAA
Podisma_sapporensis                A--AGAATAATTA
Hetrodes_pupus_EF540832            AAGCTTTTAA---
*/
}

GUI_TEST_CLASS_DEFINITION(test_0008_1){
//Check remove columns with gaps
//1. Open document _common_data\scenarios\msa\ma2_gap_col.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gap_col.aln");
//2. Place cursor on 4th column of alignment. Use msa editor context menu (at the column with gaps) {Edit->Remove column of gaps}.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(3,3));

    // TODO: shift+delete doesn't work under windows

//     QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");
//     GTUtilsDialog::waitForDialog(os,new DeleteGapsDialogFiller(os,1));
//     GTWidget::click(os,seq);
//    GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["delete"],GTKeyboardDriver::key["shift"]);
    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_EDIT" << "remove_columns_of_gaps"));
    GTUtilsDialog::waitForDialog(os,new DeleteGapsDialogFiller(os,1));
    GTMenu::showContextMenu(os,seq);

    GTGlobals::sleep(1000);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(12, 9));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardTest = GTClipboard::text(os);
    QString expectedSeq=QString("AAGCTTCTTTTAA\n"
                                "AAGTTACTAA---\n"
                                "TAG---TTATTAA\n"
                                "AAGC---TATTAA\n"
                                "TAGTTATTAA---\n"
                                "TAGTTATTAA---\n"
                                "TAGTTATTAA---\n"
                                "AAGCTTT---TAA\n"
                                "A--AGAATAATTA\n"
                                "AAGCTTTTAA---");

    CHECK_SET_ERR(clipboardTest==expectedSeq,"\n Expected: \n"+ expectedSeq +"\nFound:\n"+clipboardTest);
//Expected state: "Remove column of gaps" dialog appears

//3. Choose option "Delete all columns of gaps" and click "Delete" button
/*Expected state: length 13, right offsets 13
Phaneroptera_falcata               AAGCTTCTTTTAA
Isophya_altaica_EF540820           AAGTTACTAA---
Bicolorana_bicolor_EF540830        TAG---TTATTAA
Tettigonia_viridissima             AAGC---TATTAA
Conocephalus_discolor              TAGTTATTAA---
Conocephalus_sp.                   TAGTTATTAA---
Conocephalus_percaudata            TAGTTATTAA---
Mecopoda_elongata__Ishigaki__J     AAGCTTT---TAA
Podisma_sapporensis                A--AGAATAATTA
Hetrodes_pupus_EF540832            AAGCTTTTAA---
*/
}


void test_9(U2OpStatus &os, int i=0){
    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");
    QString gaps;
    QString expectedSeq;

    if(i){
        GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(i,0), QPoint(i, 9));
    }
    else{
        GTUtilsMSAEditorSequenceArea::click(os,QPoint(13,0));
    }

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep(100);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTGlobals::sleep(100);
    GTWidget::click(os,seq);

    if(i){
        GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(i,0), QPoint(i+1, 9));
        GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

        gaps=QString("--\n--\n--\n--\n--\n--\n--\n--\n--\n--");
    }
    else{
        GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(14,0), QPoint(15, 9));
        GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);
        gaps=QString("-A\n--\n--\n--\n--\n--\n--\n--\n--\n--");
    }

    GTGlobals::sleep(500);
    QString clipboardTest = GTClipboard::text(os);


    CHECK_SET_ERR(clipboardTest==gaps,"\n Expected: \n" + gaps + "\nFound:\n" + clipboardTest);
//Expected state: two columns with gaps added to the end of sequence.
//3. Move cursor at 15th symbol in first sequence. Use msa editor context menu {Edit->Delete column of gaps}.

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_EDIT" << "remove_columns_of_gaps"));
    GTUtilsDialog::waitForDialog(os,new DeleteGapsDialogFiller(os,1));
    GTMenu::showContextMenu(os,seq);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14, 9));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    clipboardTest = GTClipboard::text(os);
    if(i){
        expectedSeq=QString("AAGACTTCTTTTAA\n"
                            "AAGCTTACTAA---\n"
                            "TAGT---TTATTAA\n"
                            "AAGTC---TATTAA\n"
                            "TAGCTTATTAA---\n"
                            "TAGCTTATTAA---\n"
                            "TAGCTTATTAA---\n"
                            "AAGTCTTT---TAA\n"
                            "A---AGAATAATTA\n"
                            "AAGCCTTTTAA---");
    }
    else{
        expectedSeq=QString("AAGACTTCTTTTA-A\n"
                            "AAGCTTACTAA----\n"
                            "TAGT---TTATTAA-\n"
                            "AAGTC---TATTAA-\n"
                            "TAGCTTATTAA----\n"
                            "TAGCTTATTAA----\n"
                            "TAGCTTATTAA----\n"
                            "AAGTCTTT---TAA-\n"
                            "A---AGAATAATTA-\n"
                            "AAGCCTTTTAA----");
    }
    CHECK_SET_ERR(clipboardTest==expectedSeq,"\n Expected: \n"+ expectedSeq  +"\nFound:\n" + clipboardTest);
/*Expected state:
Phaneroptera_falcata               AAGACTTCTTTTA-A
Isophya_altaica_EF540820           AAGCTTACTAA----
Bicolorana_bicolor_EF540830        TAGT---TTATTAA-
Tettigonia_viridissima             AAGTC---TATTAA-
Conocephalus_discolor              TAGCTTATTAA----
Conocephalus_sp.                   TAGCTTATTAA----
Conocephalus_percaudata            TAGCTTATTAA----
Mecopoda_elongata__Ishigaki__J     AAGTCTTT---TAA-
Podisma_sapporensis                A---AGAATAATTA-
Hetrodes_pupus_EF540832            AAGCCTTTTAA----
*/
}
GUI_TEST_CLASS_DEFINITION(test_0009){
//Check remove columns with gaps
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//2. Put cursor at last symbol in first sequence, click "Space" two times.
    test_9(os);
}

GUI_TEST_CLASS_DEFINITION(test_0009_1){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//DIFFERENCE: 2. Select column 3, click "Space" two times.
    test_9(os,2);

}

GUI_TEST_CLASS_DEFINITION(test_0009_2){
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//DIFFERENCE: 2. Select column 9, click "Space" two times.
    test_9(os,8);
}

GUI_TEST_CLASS_DEFINITION(test_0010){
//UGENE crashes when edit multiply alignment (0001744)
//1. Open file samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//2. Open at least 2 views for multiple alignment
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gap_col.aln");
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma.aln");
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

//3. Try to edit MSA(insert spaces or something
    GTUtilsMSAEditorSequenceArea::click(os);
    for(int i = 0; i<7; i++){
        GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["space"]);
        GTGlobals::sleep(200);
    }
//Expected state: UGENE not crash
}

GUI_TEST_CLASS_DEFINITION(test_0011){
//Check Undo/Redo functional
//1. Open document _common_data\scenarios\msa\ma.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma.aln");
//2. Select region 4..11 from Zychia_baranovi sequence. Press "Delete" button.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(3,8), QPoint(10,8));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(200);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,8), QPoint(11,8));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    QString clipboardText = GTClipboard::text(os);
    GTWidget::click(os,GTWidget::findWidget(os, "msa_editor_sequence_area"));
    CHECK_SET_ERR(clipboardText=="TTAA--------","\nExpected: TTAA--------\nFound:\n"+clipboardText);
//Expected state: Zychia_baranovi TTAA

//3. Click Undo button on toolbar panel.
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    GTWidget::click(os,undo);
//Expected state: Zychia_baranovi TTAGATTATTAA
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,8), QPoint(11,8));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    clipboardText = GTClipboard::text(os);
    GTWidget::click(os,GTWidget::findWidget(os, "msa_editor_sequence_area"));
    CHECK_SET_ERR(clipboardText=="TTAGATTATTAA","\nExpected: TTAGATTATTAA\nFound:\n"+clipboardText);

//4. Click Redo button on toolbar panel.msa_action_redo
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");
    GTWidget::click(os,redo);
//Expected state: Zychia_baranovi TTAA
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,8), QPoint(11,8));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText=="TTAA--------","\nExpected: TTAA--------\nFound:\n"+clipboardText);
}
GUI_TEST_CLASS_DEFINITION(test_0011_1){
//Check Undo/Redo functional
//1. Open document _common_data\scenarios\msa\ma.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma.aln");
//2. Select region 4..11 from Zychia_baranovi sequence. Press "Delete" button.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(3,8), QPoint(10,8));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(200);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,8), QPoint(11,8));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    QString clipboardText = GTClipboard::text(os);
    GTWidget::click(os,GTWidget::findWidget(os, "msa_editor_sequence_area"));
    CHECK_SET_ERR(clipboardText=="TTAA--------","\nExpected: TTAA--------\nFound:\n"+clipboardText);
//Expected state: Zychia_baranovi TTAA

//DIFFERENCE: 3. Click ctrl+z
    GTKeyboardDriver::keyClick(os, 'z', GTKeyboardDriver::key["ctrl"]);
//Expected state: Zychia_baranovi TTAGATTATTAA
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,8), QPoint(11,8));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    clipboardText = GTClipboard::text(os);
    GTWidget::click(os,GTWidget::findWidget(os, "msa_editor_sequence_area"));
    CHECK_SET_ERR(clipboardText=="TTAGATTATTAA","\nExpected: TTAGATTATTAA\nFound:\n"+clipboardText);

//4.DIFFERENCE: 3. Click ctrl+y or ctrl+shift+z
#ifdef Q_OS_WIN
    GTKeyboardDriver::keyClick(os, 'y', GTKeyboardDriver::key["ctrl"]);
#else
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);
    GTKeyboardDriver::keyClick(os, 'z', GTKeyboardDriver::key["ctrl"]);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
#endif
//Expected state: Zychia_baranovi TTAA
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,8), QPoint(11,8));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText=="TTAA--------","\nExpected: TTAA--------\nFound:\n"+clipboardText);
}

GUI_TEST_CLASS_DEFINITION(test_0011_2){
//Check Undo/Redo functional
//1. Open document _common_data\scenarios\msa\ma.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//    2. Use msa editor context menu {Edit->Remove all gaps}.
    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_EDIT" << "Remove all gaps"));
    GTMenu::showContextMenu(os,seq);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(13, 9));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardTest = GTClipboard::text(os);
    QString  expectedSeq=QString("AAGACTTCTTTTAA\n"
                                 "AAGCTTACTAA---\n"
                                 "TAGTTTATTAA---\n"
                                 "AAGTCTATTAA---\n"
                                 "TAGCTTATTAA---\n"
                                 "TAGCTTATTAA---\n"
                                 "TAGCTTATTAA---\n"
                                 "AAGTCTTTTAA---\n"
                                 "AAGAATAATTA---\n"
                                 "AAGCCTTTTAA---");

    CHECK_SET_ERR(clipboardTest==expectedSeq,"\n Expected: \n"+ expectedSeq +"\nFound:\n"+clipboardTest);
//3. Click Undo button on toolbar panel.
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    GTWidget::click(os,undo);
    GTWidget::click(os,seq);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(13, 9));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    clipboardTest = GTClipboard::text(os);
    QString modyfiedSeq=QString("AAGACTTCTTTTAA\n"
                        "AAGCTTACTAA---\n"
                        "TAGT---TTATTAA\n"
                        "AAGTC---TATTAA\n"
                        "TAGCTTATTAA---\n"
                        "TAGCTTATTAA---\n"
                        "TAGCTTATTAA---\n"
                        "AAGTCTTT---TAA\n"
                        "A---AGAATAATTA\n"
                        "AAGCCTTTTAA---");

    CHECK_SET_ERR(clipboardTest==modyfiedSeq,"\n Expected: \n"+ expectedSeq +"\nFound:\n"+clipboardTest);
//4. Click Redo button on toolbar panel.
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");
    GTWidget::click(os,redo);
    GTWidget::click(os,seq);

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(13, 9));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    clipboardTest = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardTest==expectedSeq,"\n Expected: \n"+ expectedSeq +"\nFound:\n"+clipboardTest);
}

GUI_TEST_CLASS_DEFINITION(test_0011_3){
//Check Undo/Redo functional
//1. Open document _common_data\scenarios\msa\ma.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//2. Insert seversl spaces somewhere
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(0,0));
    for(int i=0; i<6; i++){
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
        GTGlobals::sleep(200);
    }
//3. Undo this
    for (int i=0; i<6; i++){
        GTKeyboardDriver::keyClick(os, 'z', GTKeyboardDriver::key["ctrl"]);
        GTGlobals::sleep(200);
    }
//Expected state: First sequwnce is AAGACTTCTTTTAA
    GTWidget::click(os,GTWidget::findWidget(os, "msa_editor_sequence_area"));

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(13, 0));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardTest = GTClipboard::text(os);
    QString expectedSeq=QString("AAGACTTCTTTTAA");
    CHECK_SET_ERR(clipboardTest==expectedSeq,"\n Expected: \n"+ expectedSeq +"\nFound:\n"+clipboardTest);

}

GUI_TEST_CLASS_DEFINITION(test_0012){
//Check copy
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");
//2. Select region 3..8 for Conocephalus_discolor sequence. Press Ctrl+C.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(2,4), QPoint(7, 4));
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
//Expected state: GCTTAT has copied to clipboard
    QString clipboardTest = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardTest=="GCTTAT","\n Expected: \nGCTTAT\nFound:\n"+clipboardTest);
//3. Select region 6..12 for Conocephalus_discolor sequence. Use context menu {Copy->Copy selection}.
    GTWidget::click(os,GTWidget::findWidget(os, "msa_editor_sequence_area"));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(5,4), QPoint(11, 4));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_COPY" << "copy_selection"));
    GTMenu::showContextMenu(os,GTWidget::findWidget(os, "msa_editor_sequence_area"));
    GTGlobals::sleep(500);
//Expected state: TATTAA- has copied to clipboard
    clipboardTest = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardTest=="TATTAA-","\n Expected: \nTATTAA-\nFound:\n"+clipboardTest);
}

void test_13(U2OpStatus &os, int comboVal, int SpinVal, QString ExpectedCons){
    QWidget* seq=GTWidget::findWidget(os, "msa_editor_sequence_area");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Consensus mode",GTGlobals::UseMouse));
    GTMenu::showContextMenu(os,seq);

    QComboBox *consensusCombo=qobject_cast<QComboBox*>(GTWidget::findWidget(os,"consensusType"));
    CHECK_SET_ERR(consensusCombo!=NULL, "consensusCombo is NULL");
    GTComboBox::setCurrentIndex(os,consensusCombo, comboVal);

    QSpinBox *thresholdSpinBox=qobject_cast<QSpinBox*>(GTWidget::findWidget(os,"thresholdSpinBox"));
    CHECK_SET_ERR(thresholdSpinBox!=NULL, "consensusCombo is NULL");
    GTSpinBox::setValue(os,thresholdSpinBox,SpinVal,GTGlobals::UseKeyBoard);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"MSAE_MENU_COPY" << "Copy consensus",GTGlobals::UseMouse));
    GTMenu::showContextMenu(os,seq);

    QString clipboardText=GTClipboard::text(os);
    GTGlobals::sleep(500);
    CHECK_SET_ERR(clipboardText==ExpectedCons,"\n Expected: \n"+ ExpectedCons +"\nFound:\n"+clipboardText);
}

GUI_TEST_CLASS_DEFINITION(test_0013){
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_13(os,1,100,"aagc+tattaataa");
}

GUI_TEST_CLASS_DEFINITION(test_0013_1){
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_13(os,1,1,"AAGC+TATTAATAA");
}

GUI_TEST_CLASS_DEFINITION(test_0013_2){
//1. Open document _common_data\scenarios\msa\ma2_gapped.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    test_13(os,2,75,"WAGYYTWYTAW");
}

GUI_TEST_CLASS_DEFINITION(test_0014){
//Create subaligniment
//1. Open document samples\CLUSTALW\COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//2. Right click on aligniment view. Open context menu {Export->save subalignment}
    GTUtilsMSAEditorSequenceArea::selectArea(os,QPoint(0,5),QPoint(10,5));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_EXPORT<<"Save subalignment"));
    GTUtilsDialog::waitForDialog(os,new ExtractSelectedAsMSADialogFiller(os,testDir+"_common_data/scenarios/sandbox/result.aln",QStringList()<<"Zychia_baranovi"<<"Montana_montana",6,16));
    GTMenu::showContextMenu(os,GTWidget::findWidget(os,"msa_editor_sequence_area"));
//Expected state: Create subaligniment dialog has appeared

//3. Fill the next fields in dialog:
//    {Subaligniment start:} 6
//    {End:} 16
//    {Selected sequences:} set checked: Montana_montana, Zychia_baranovi
//    {Path:} _common_data/sandbox/result.aln
//    {Add to project:} set checked

//4. Click OK
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"result.aln"));
    GTMouseDriver::doubleClick(os);
//Expected result: subaligniment created and added to project

//5. Open result.aln in aligniment viewer
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0),QPoint(10,1));
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);

    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText=="TTATTAATTCG\nTTATTAATCCG", "clipboardText is:\n"+clipboardText);
//Expected result: aligniment with 2 sequences appear in editor:
//Montana_montana TTATTAATTCG
//Zychia_baranovi TTATTAATCCG
}
class SpecialExtractSelectedAsMSADialogFiller : public Filler {
public:
    SpecialExtractSelectedAsMSADialogFiller(U2OpStatus &os) : Filler(os, "CreateSubalignimentDialog"){}
    virtual void run(){
        QWidget* dialog = QApplication::activeModalWidget();
        CHECK_SET_ERR(dialog, "activeModalWidget is NULL");

        QTableWidget *table=dialog->findChild<QTableWidget*>("sequencesTableWidget");
        CHECK_SET_ERR(table!=NULL, "tableWidget is NULL");

        for(int i=0; i<table->rowCount() ;i++){//check all selected
            QCheckBox *box = qobject_cast<QCheckBox*>(table->cellWidget(i,0));
            CHECK_SET_ERR(box->isChecked(),"box" + box->text() + "is not checked");
        }

        QWidget *noneButton = dialog->findChild<QWidget*>("noneButton");//clear button
        CHECK_SET_ERR(noneButton!=NULL, "noneButton is NULL");
        GTWidget::click(os,noneButton);

        for(int i=0; i<table->rowCount() ;i++){//check all unselected
            QCheckBox *box = qobject_cast<QCheckBox*>(table->cellWidget(i,0));
            CHECK_SET_ERR(!box->isChecked(),"box " + box->text() + " is checked");
        }

        QPoint p=table->geometry().topRight();
        p.setX(p.x()-2);
        p.setY(p.y()+2);
        p=dialog->mapToGlobal(p);
        GTMouseDriver::moveTo(os,p);
        GTMouseDriver::doubleClick(os);

        QStringList list;
        list<<"Zychia_baranovi"<<"Montana_montana";//select these
        for(int i=0; i<table->rowCount() ;i++){
            GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["down"]);
            GTGlobals::sleep(100);
            foreach(QString s, list){
                QCheckBox *box = qobject_cast<QCheckBox*>(table->cellWidget(i,0));
                if (s==box->text()){
                    GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["space"]);
                }
            }
        }

        QWidget *invertButton = dialog->findChild<QWidget*>("invertButton");//click invert button
        CHECK_SET_ERR(invertButton!=NULL, "invertButton is NULL");
        GTWidget::click(os,invertButton);

        for(int i=4; i<table->rowCount() ;i++){//check invertion result
            bool nameFound=false;
            QCheckBox *box = qobject_cast<QCheckBox*>(table->cellWidget(i,0));

            foreach(QString s, list){
                if (s==box->text()){
                    CHECK_SET_ERR(!box->isChecked(), "box "+box->text()+" is checked. It must be uncheked");
                    nameFound=true;
                }
            }

            if(!nameFound){
                CHECK_SET_ERR(box->isChecked(), "box "+box->text()+" is unchecked. It must be cheked");
            }
        }

        QWidget *allButton = dialog->findChild<QWidget*>("allButton");//click SelectAll button
        CHECK_SET_ERR(allButton!=NULL, "allButton is NULL");
        GTWidget::click(os,allButton);

        for(int i=0; i<table->rowCount() ;i++){//check all selected
            QCheckBox *box = qobject_cast<QCheckBox*>(table->cellWidget(i,0));
            CHECK_SET_ERR(box->isChecked(),"box " + box->text() + " is not checked");
        }

        GTWidget::click(os, GTWidget::findWidget(os,"cancelButton"));
    }
};
GUI_TEST_CLASS_DEFINITION(test_0015){
//Create subaligniment
//1. Open document samples\CLUSTALW\COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//2. Right click on aligniment view. Open context menu {Export->save subalignment}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_EXPORT<<"Save subalignment"));
    GTUtilsDialog::waitForDialog(os,new SpecialExtractSelectedAsMSADialogFiller(os));
    GTMenu::showContextMenu(os,GTWidget::findWidget(os,"msa_editor_sequence_area"));
//Expected state: Create subaligniment dialog has appeared, all sequences are checked

//3. Click button Clean selection
//Expected result: all sequences unchecked

//4. Check sequences Montana_montana, Zychia_baranovi

//5. Click button Invert selection
//Expected result: all sequences except Montana_montana, Zychia_baranovi are checked

//6. Click button Select all
//Expected result: all sequences are checked
}

GUI_TEST_CLASS_DEFINITION( test_0016 )
{
    // Perform scribbling
    // 1. Open file with MSA
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma.aln" );

    // 2. Select some region in msa
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 7 ), QPoint( 7, 14 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text( os );

    // 3. Shift the selection to the right
    QPoint mouseClickPosition( 7, 7 );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseClickPosition );
    GTMouseDriver::click( os );
    GTGlobals::sleep( 200 );
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os,
        QRect( QPoint( 4, 7 ), QPoint( 11, 14 ) ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    QString selectionAfterScribbling = GTClipboard::text( os );
    CHECK_SET_ERR( selectionContent == selectionAfterScribbling, "Unexpected selection content" );

    // 4. Shift the selection to the left
    mouseClickPosition = QPoint( 4, 7 );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseClickPosition );
    GTMouseDriver::click( os );
    GTGlobals::sleep( 200 );
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os,
        QRect( QPoint( 1, 7 ), QPoint( 8, 14 ) ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    selectionAfterScribbling = GTClipboard::text( os );
    CHECK_SET_ERR( selectionContent == selectionAfterScribbling, "Unexpected selection content" );

    // 5. Shift the selection to the 1st base
    mouseClickPosition = QPoint( 1, 7 );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseClickPosition );
    GTMouseDriver::click( os );
    GTGlobals::sleep( 200 );
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os,
        QRect( QPoint( 0, 7 ), QPoint( 7, 14 ) ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    selectionAfterScribbling = GTClipboard::text( os );
    CHECK_SET_ERR( selectionContent == selectionAfterScribbling, "Unexpected selection content" );
}

} // namespace
} // namespace U2

