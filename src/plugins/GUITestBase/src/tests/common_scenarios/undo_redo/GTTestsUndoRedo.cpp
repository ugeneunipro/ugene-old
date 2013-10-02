/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
k
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

#include "GTTestsUndoRedo.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTAction.h"
#include "api/GTTreeWidget.h"
#include "api/GTGlobals.h"
#include "api/GTClipboard.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsProjectTreeView.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/DeleteGapsDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ConsensusSelectorDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ExtractSelectedAsMSADialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/util/RenameSequenceFiller.h"
#include "runnables/ugene/plugins_3rdparty/umuscle/MuscleDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/kalign/KalignDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/clustalw/ClustalWDialogFiller.h"
#include <U2View/MSAEditor.h>

namespace U2{

namespace GUITest_common_scenarios_undo_redo{

GUI_TEST_CLASS_DEFINITION(test_0001){//DIFFERENCE: lock document is checked
//Check Undo/Redo functional
//1. Open document COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//2. Insert seversl spaces somewhere
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(0,0));
    for(int i=0; i<6; i++){
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
        GTGlobals::sleep(200);
    }

    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");
//3. Undo this
    for (int i=0; i<3; i++){
        GTWidget::click(os,undo);
        GTGlobals::sleep(200);
    }
//4. lock document
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"COI.aln"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<ACTION_PROJECT__EDIT_MENU<<ACTION_DOCUMENT__LOCK));
    GTMouseDriver::click(os, Qt::RightButton);

//Expected state: Undo and redo buttons are disabled
    CHECK_SET_ERR(!undo->isEnabled(),"Undo button is enebled after locking document");
    CHECK_SET_ERR(!redo->isEnabled(),"Redo button is enebled after locking document");

//5. Unlock document
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"COI.aln"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<ACTION_PROJECT__EDIT_MENU<<ACTION_DOCUMENT__UNLOCK));
    GTMouseDriver::click(os, Qt::RightButton);

//Expected state: undo and redo buttons are enebled and work properly
    CHECK_SET_ERR(undo->isEnabled(),"Undo button is disabled after unlocking document");
    CHECK_SET_ERR(redo->isEnabled(),"Redo button is disabled after unlocking document");

    //check undo
    GTWidget::click(os,GTUtilsMdi::activeWindow(os));
    GTWidget::click(os, undo);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(9,0));
    GTKeyboardDriver::keyClick(os,'c',GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText=="--TAAGACTT","Undo works wrong. Found text is: " + clipboardText);

    //check redo
    GTWidget::click(os,GTUtilsMdi::activeWindow(os));
    GTWidget::click(os, redo);
    GTGlobals::sleep(200);
    GTWidget::click(os, redo);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(9,0));
    GTKeyboardDriver::keyClick(os,'c',GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText=="----TAAGAC", "Redo works wrong. Found text is: " + clipboardText);
}

GUI_TEST_CLASS_DEFINITION(test_0002){//DIFFERENCE: delete sequence is checked
//Check Undo/Redo functional
//1. Open document COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//2. Delete 4-th sequence
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Roeseliana_roeseli", "Roeseliana_roeseli"));
    GTMouseDriver::doubleClick(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);


// Expected state: sequence deleted
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Montana_montana", "Montana_montana"));
    GTMouseDriver::doubleClick(os);

//3. undo deletion
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");

    GTWidget::click(os, undo);

//Expected state: delition undone
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Roeseliana_roeseli", "Roeseliana_roeseli"));
    GTMouseDriver::doubleClick(os);

//4. Redo delition
    GTWidget::click(os, redo);

//Expected state: delition is redone
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,3));
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "Montana_montana", "Montana_montana"));
    GTMouseDriver::doubleClick(os);
}

GUI_TEST_CLASS_DEFINITION(test_0003){//DIFFERENCE: add sequence is checked
    //Check Undo/Redo functional
//1. Open document COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//2. add sequence to alignment
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_LOAD<<"Sequence from file"));
    GTFileDialogUtils *ob = new GTFileDialogUtils(os, dataDir + "/samples/Raw/", "raw.seq");
    GTUtilsDialog::waitForDialog(os, ob);
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));

// Expected state: raw sequence appeared in alignment
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getNameList(os).contains("raw"), "raw is not added");
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,18));
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "raw", "raw"));
    GTMouseDriver::doubleClick(os);

//3. undo adding
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");

    GTWidget::click(os, undo);

//Expected state: raw doesn't present in namelist
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(!nameList.contains("raw"), "adding raw is not undone");

//4. Redo delition
    GTWidget::click(os, redo);

//Expected state: delition is redone
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getNameList(os).contains("raw"), "Adding raw is not redone");
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-10,18));
    GTUtilsDialog::waitForDialog(os, new RenameSequenceFiller(os, "raw", "raw"));
    GTMouseDriver::doubleClick(os);
}

GUI_TEST_CLASS_DEFINITION(test_0004){//DIFFERENCE: add sequence is checked
//Check Undo/Redo functional
//1. Open document COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//2. insert gap->undo->insert gap->undo->redo
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");

    GTUtilsMSAEditorSequenceArea::click(os, QPoint(0,0));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTWidget::click(os, undo);

    GTUtilsMSAEditorSequenceArea::click(os, QPoint(0,0));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    GTWidget::click(os, undo);

    GTWidget::click(os, redo);

// Expected state: redo button is disabled
    CHECK_SET_ERR(!redo->isEnabled(), "Redo button is enebled");
}

GUI_TEST_CLASS_DEFINITION(test_0005){//undo remove selection
    //open file
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    //remove selection
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(3,1));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    //Expected state: selection removed
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(3,1));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString clipdoardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipdoardText=="ACTT\nCTTA", QString("Expected ACTT\nCTTA, found: %1").arg(clipdoardText));

    //undo
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    GTWidget::click(os, undo);

    //Expected state: delition undone
    GTWidget::click(os,GTUtilsMdi::activeWindow(os));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(3,1));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    clipdoardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipdoardText=="TAAG\nTAAG", QString("Expected TAAG\nTAAG, found: %1").arg(clipdoardText));

    //redo
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");
    GTWidget::click(os, redo);

    //Expected state: delition redone
    GTWidget::click(os,GTUtilsMdi::activeWindow(os));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(3,1));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    clipdoardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipdoardText=="ACTT\nCTTA", QString("Expected ACTT\nCTTA, found: %1").arg(clipdoardText));
}

GUI_TEST_CLASS_DEFINITION(test_0006){//undo replace_selected_rows_with_reverse-complement
// In-place reverse complement replace in MSA Editor (0002425)

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");

// 2. Select first sequence and do context menu {Edit->Replace selected rows with reverce complement}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement"));
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( -1, 2 ) );
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: sequence changed from TTG -> CAA
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "CAA\nTGA\nATC",
        "Clipboard string and expected MSA string differs");

//  sequence name changed from L -> L|revcompl
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR( nameList.size( ) >= 6, "nameList doesn't contain enough strings" );
    CHECK_SET_ERR( ( nameList[0] == "L" ) && ( nameList[1] == "revcompl" )
        && ( nameList[2] == "S" ) && ( nameList[3] == "revcompl" )
        && ( nameList[4] == "D" ) && ( nameList[5] == "revcompl" ), "Unexpected sequence names" );

// 3. Undo
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    GTWidget::click(os, undo);

// Expected state: sequence changed from CAA -> TTG
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR( clipboardText == "TTG\nTCA\nGAT",
        "Clipboard string and expected MSA string differs" );

//  sequence name changed from L|revcompl ->
    nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR( nameList.size( ) >= 3, "nameList doesn't contain enough strings" );
    CHECK_SET_ERR( ( nameList[0] == "L" ) && ( nameList[1] == "S" ) && ( nameList[2] == "D" ),
        "There are unexpected names in nameList" );

    GTGlobals::sleep(500);

// 4. Redo
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");
    GTWidget::click(os, redo);

// Expected state: sequence changed from TTG -> CAA
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "CAA\nTGA\nATC",
        "Clipboard string and expected MSA string differs");

//  sequence name changed from L -> L|revcompl
    nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR( nameList.size( ) >= 6, "nameList doesn't contain enough strings" );
    CHECK_SET_ERR( ( nameList[0] == "L" ) && ( nameList[1] == "revcompl" )
        && ( nameList[2] == "S" ) && ( nameList[3] == "revcompl" )
        && ( nameList[4] == "D" ) && ( nameList[5] == "revcompl" ), "Unexpected sequence names" );
}

GUI_TEST_CLASS_DEFINITION(test_0006_1){//undo replace_selected_rows_with_reverse
// In-place reverse complement replace in MSA Editor (0002425)

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln");

// 2. Select first sequence and do context menu {Edit->Replace selected rows with reverce complement}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse"));
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( -1, 2 ) );
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: sequence changed from TTG -> GTT
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR( clipboardText == "GTT\nACT\nTAG",
        "Clipboard string and expected MSA string differs");

// sequence name  changed from L -> L|revcompl
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR( nameList.size( ) >= 6, "nameList doesn't contain enough strings" );
    CHECK_SET_ERR( ( nameList[0] == "L" ) && ( nameList[1] == "rev" )
        && ( nameList[2] == "S" ) && ( nameList[3] == "rev" )
        && ( nameList[4] == "D" ) && ( nameList[5] == "rev" ), "Unexpected sequence names" );

// 3. Undo
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    GTWidget::click(os, undo);

// Expected state: sequence changed from GTT -> TTG
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR( clipboardText == "TTG\nTCA\nGAT",
        "Clipboard string and expected MSA string differs" );

//  sequence name changed from L|rev ->
    nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR( nameList.size( ) >= 3, "nameList doesn't contain enough strings" );
    CHECK_SET_ERR( ( nameList[0] == "L" ) && ( nameList[1] == "S" ) && ( nameList[2] == "D" ),
        "There are unexpected names in nameList" );

    GTGlobals::sleep(500);

// 4. Redo
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");
    GTWidget::click(os, redo);

// Expected state: sequence changed from TTG -> GTT
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR( clipboardText == "GTT\nACT\nTAG",
        "Clipboard string and expected MSA string differs");

//  sequence name changed from L -> L|revcompl
    nameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR( nameList.size( ) >= 6, "nameList doesn't contain enough strings" );
    CHECK_SET_ERR( ( nameList[0] == "L" ) && ( nameList[1] == "rev" )
        && ( nameList[2] == "S" ) && ( nameList[3] == "rev" )
        && ( nameList[4] == "D" ) && ( nameList[5] == "rev" ), "Unexpected sequence names" );
}

GUI_TEST_CLASS_DEFINITION( test_0006_2 )
{
//undo replace_selected_rows_with_complement
// In-place reverse complement replace in MSA Editor (0002425)

// 1. Open file _common_data\scenarios\msa\translations_nucl.aln
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa/", "translations_nucl.aln" );

// 2. Select first sequence and do context menu {Edit->Replace selected rows with reverce complement}
    GTUtilsDialog::waitForDialog( os, new PopupChooser( os, QStringList( ) << MSAE_MENU_EDIT
        << "replace_selected_rows_with_complement" ) );
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( -1, 2 ) );
    GTMouseDriver::click( os, Qt::RightButton );

// Expected state: sequence changed from TTG -> AAC
    GTGlobals::sleep( 500 );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );

    GTGlobals::sleep( 500 );
    QString clipboardText = GTClipboard::text( os );
    CHECK_SET_ERR( clipboardText == "AAC\nAGT\nCTA",
        "Clipboard string and expected MSA string differs. Clipboard string is: "
        + clipboardText );

//  sequence name  changed from L -> L|compl
    QStringList nameList = GTUtilsMSAEditorSequenceArea::getNameList( os );
    CHECK_SET_ERR( nameList.size( ) >= 6, "nameList doesn't contain enough strings" );
    CHECK_SET_ERR( ( nameList[0] == "L" ) && ( nameList[1] == "compl" )
        && ( nameList[2] == "S" ) && ( nameList[3] == "compl" )
        && ( nameList[4] == "D" ) && ( nameList[5] == "compl" ), "Unexpected sequence names" );

// 3. Undo
    QAbstractButton *undo = GTAction::button( os, "msa_action_undo" );
    GTWidget::click( os, undo );

// Expected state: sequence changed from AAC -> TTG
    GTGlobals::sleep( 500 );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );

    GTGlobals::sleep( 500 );
    clipboardText = GTClipboard::text( os );
    CHECK_SET_ERR( clipboardText == "TTG\nTCA\nGAT",
        "Clipboard string and expected MSA string differs" );

//  sequence name changed from L|rev ->
    nameList = GTUtilsMSAEditorSequenceArea::getNameList( os );
    CHECK_SET_ERR( nameList.size( ) >= 3, "nameList doesn't contain enough strings" );
    CHECK_SET_ERR( ( nameList[0] == "L" ) && ( nameList[1] == "S" ) && ( nameList[2] == "D" ),
        "There are unexpected names in nameList" );

    GTGlobals::sleep( 500 );

// 4. Redo
    QAbstractButton *redo = GTAction::button( os, "msa_action_redo" );
    GTWidget::click( os, redo );

// Expected state: sequence changed from TTG -> AAC
    GTGlobals::sleep( 500 );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );

    GTGlobals::sleep( 500 );
    clipboardText = GTClipboard::text( os );
    CHECK_SET_ERR( clipboardText == "AAC\nAGT\nCTA",
        "Clipboard string and expected MSA string differs" );

//  sequence name  changed from L -> L|revcompl
    nameList = GTUtilsMSAEditorSequenceArea::getNameList( os );
    CHECK_SET_ERR( nameList.size( ) >= 6, "nameList doesn't contain enough strings" );
    CHECK_SET_ERR( ( nameList[0] == "L" ) && ( nameList[1] == "compl" )
        && ( nameList[2] == "S" ) && ( nameList[3] == "compl" )
        && ( nameList[4] == "D" ) && ( nameList[5] == "compl" ),
       "There are unexpected names in nameList" );
}

GUI_TEST_CLASS_DEFINITION(test_0007){//remove columns with 3 or more gaps
    //Open file
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gap_col.aln");

    //save initial state
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString initAln = GTClipboard::text(os);
    QString expectedChangedAln = "AAGCTTCTTT\nAAGTTACTAA\nTAG---TTAT\nAAGC---TAT\n"
                                 "TAGTTATTAA\nTAGTTATTAA\nTAGTTATTAA\nAAGCTTT---\n"
                                 "A--AGAATAA\nAAGCTTTTAA";

    //fill remove columns of gaps dialog
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "remove_columns_of_gaps",GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new RemoveGapColsDialogFiller(os, RemoveGapColsDialogFiller::Number,3));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString changedAln = GTClipboard::text(os);
    CHECK_SET_ERR(changedAln == expectedChangedAln,"remove gaps option works wrong");

    //undo
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    GTWidget::click(os, undo);

    GTWidget::click(os, GTUtilsMdi::activeWindow(os));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText==initAln, "undo works wrong");

    //redo
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");
    GTWidget::click(os, redo);

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText==expectedChangedAln, "redo works wrong");
}

GUI_TEST_CLASS_DEFINITION(test_0007_1){//remove columns with 15 percents of gaps
    //Open file
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gap_col.aln");

    //save initial state
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString initAln = GTClipboard::text(os);
    QString expectedChangedAln = "AAGCCTTT\nAAGTCTAA\nTAG-TTAT\nAAGC-TAT\nTAGTTTAA\nTAGTTTAA\n"
                                 "TAGTTTAA\nAAGCT---\nA--AATAA\nAAGCTTAA";

    //fill remove columns of gaps dialog
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "remove_columns_of_gaps",GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new RemoveGapColsDialogFiller(os, RemoveGapColsDialogFiller::Percent,15));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString changedAln = GTClipboard::text(os);
    CHECK_SET_ERR(changedAln == expectedChangedAln,"remove gaps option works wrong\n" + changedAln + '\n' + expectedChangedAln);

    //undo
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    GTWidget::click(os, undo);

    GTWidget::click(os, GTUtilsMdi::activeWindow(os));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText==initAln, "undo works wrong");

    //redo
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");
    GTWidget::click(os, redo);

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    clipboardText=GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText==expectedChangedAln, "redo works wrong");
}

GUI_TEST_CLASS_DEFINITION(test_0007_2){//remove columns of gaps is tested
    //Open file
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gap_col.aln");

    //save initial state
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString initAln = GTClipboard::text(os);
    QString expectedChangedAln = "AAGCTTCTTTTAA\nAAGTTACTAA---\nTAG---TTATTAA\nAAGC---TATTAA\nTAGTTATTAA---\n"
                                 "TAGTTATTAA---\nTAGTTATTAA---\nAAGCTTT---TAA\nA--AGAATAATTA\nAAGCTTTTAA---";

    //fill remove columns of gaps dialog
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "remove_columns_of_gaps",GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new RemoveGapColsDialogFiller(os, RemoveGapColsDialogFiller::Column));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString changedAln = GTClipboard::text(os);
    CHECK_SET_ERR(changedAln == expectedChangedAln,"remove gaps option works wrong\n" + changedAln + '\n' + expectedChangedAln);

    //undo
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    GTWidget::click(os, undo);

    GTWidget::click(os, GTUtilsMdi::activeWindow(os));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText==initAln, "undo works wrong");

    //redo
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");
    GTWidget::click(os, redo);

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    clipboardText=GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText==expectedChangedAln, "redo works wrong\n" + clipboardText);
}

GUI_TEST_CLASS_DEFINITION(test_0008){//remove all gaps is tested
    //Open file
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gap_col.aln");

    //save initial state
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString initAln = GTClipboard::text(os);
    QString expectedChangedAln = "AAGCTTCTTTTAA\nAAGTTACTAA---\nTAGTTATTAA---\nAAGCTATTAA---\nTAGTTATTAA---\n"
                                 "TAGTTATTAA---\nTAGTTATTAA---\nAAGCTTTTAA---\nAAGAATAATTA--\nAAGCTTTTAA---";

    //fill remove columns of gaps dialog
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "Remove all gaps",GTGlobals::UseMouse));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString changedAln = GTClipboard::text(os);
    CHECK_SET_ERR(changedAln == expectedChangedAln,"remove gaps option works wrong\n" + changedAln + '\n' + expectedChangedAln);

    //undo
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    GTWidget::click(os, undo);

    GTWidget::click(os, GTUtilsMdi::activeWindow(os));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText==initAln, "undo works wrong");

    //redo
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");
    GTWidget::click(os, redo);

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    clipboardText=GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText==expectedChangedAln, "redo works wrong\n" + clipboardText);
}

GUI_TEST_CLASS_DEFINITION(test_0009){//rename msa is tested
    //Open file
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gap_col.aln");

    //rename msa
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ACTION_PROJECT__EDIT_MENU<<"Rename"));
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os, "ma2_gap_col"));
    GTMouseDriver::click(os,Qt::RightButton);
    GTKeyboardDriver::keySequence(os, "some_name");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);

    //Expected state: msa renamed
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os, "some_name"));

    //undo
    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");
    GTWidget::click(os, undo);

    //Expected state: rename undone
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os, "ma2_gap_col"));

    //redo
    GTWidget::click(os, redo);

    //Expected state: rename redone
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os, "some_name"));
}

GUI_TEST_CLASS_DEFINITION(test_0010){//MUSCLE aligner undo test
    //Open file
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString initAln = GTClipboard::text(os);
    QString expectedAln("AAG---AATAATTA\n"
                        "AAG---TCTATTAA\n"
                        "AAGACTTCTTTTAA\n"
                        "AAG---TCTTTTAA\n"
                        "AAG---CCTTTTAA\n"
                        "AAG---CTTACTAA\n"
                        "TAG---TTTATTAA\n"
                        "TAG---CTTATTAA\n"
                        "TAG---CTTATTAA\n"
                        "TAG---CTTATTAA");

    //Use context {Edit->Align with MUSCLE}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_ALIGN<<"Align with muscle", GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new MuscleDialogFiller(os,MuscleDialogFiller::Default,false));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
#ifdef Q_OS_MAC
    GTGlobals::sleep(10000);
#endif
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString changedAln = GTClipboard::text(os);
    CHECK_SET_ERR(changedAln==expectedAln, "Unexpected alignment" + changedAln);

    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");

    //undo
    GTWidget::click(os,undo);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    changedAln = GTClipboard::text(os);

    CHECK_SET_ERR(changedAln==initAln, "Undo works wrong\n" + changedAln);

    //redo
    GTWidget::click(os,redo);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    changedAln = GTClipboard::text(os);

    CHECK_SET_ERR(changedAln==expectedAln, "Undo works wrong\n" + changedAln);

}

GUI_TEST_CLASS_DEFINITION(test_0011){//Kalign undo test
    //Open file
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString initAln = GTClipboard::text(os);

    QString expectedAln("AAGACTTCTTTTAA\n"
                        "AAGCTTACT---AA\n"
                        "TAGTTTATT---AA\n"
                        "AAGTCTATT---AA\n"
                        "TAGCTTATT---AA\n"
                        "TAGCTTATT---AA\n"
                        "TAGCTTATT---AA\n"
                        "AAGTCTTTT---AA\n"
                        "AAGAATAAT---TA\n"
                        "AAGCCTTTT---AA");

    //Use context {Edit->Align with Kalign}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_ALIGN<<"align_with_kalign", GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
#ifdef Q_OS_MAC
    GTGlobals::sleep(10000);
#endif
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString changedAln = GTClipboard::text(os);
    CHECK_SET_ERR(changedAln==expectedAln, "Unexpected alignment" + changedAln);

    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");

    //undo
    GTWidget::click(os,undo);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    changedAln = GTClipboard::text(os);

    CHECK_SET_ERR(changedAln==initAln, "Undo works wrong\n" + changedAln);

    //redo
    GTWidget::click(os,redo);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    changedAln = GTClipboard::text(os);

    CHECK_SET_ERR(changedAln==expectedAln, "Undo works wrong\n" + changedAln);
}

GUI_TEST_CLASS_DEFINITION(test_0011_1){//Kalign undo test
    //Open file
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString initAln = GTClipboard::text(os);

    QString expectedAln("AAGACTTCTTTTAA\n"
                        "AAG-CTTACT--AA\n"
                        "TAG-TTTATT--AA\n"
                        "AAG-TCTATT--AA\n"
                        "TAG-CTTATT--AA\n"
                        "TAG-CTTATT--AA\n"
                        "TAG-CTTATT--AA\n"
                        "AAG-TCTTTT--AA\n"
                        "AAG-AATAAT--TA\n"
                        "AAG-CCTTTT--AA");

    //Use context {Edit->Align with Kalign}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_ALIGN<<"align_with_kalign", GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os,100));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
#ifdef Q_OS_MAC
    GTGlobals::sleep(10000);
#endif
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString changedAln = GTClipboard::text(os);
    CHECK_SET_ERR(changedAln==expectedAln, "Unexpected alignment" + changedAln);

    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");

    //undo
    GTWidget::click(os,undo);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    changedAln = GTClipboard::text(os);

    CHECK_SET_ERR(changedAln==initAln, "Undo works wrong\n" + changedAln);

    //redo
    GTWidget::click(os,redo);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    changedAln = GTClipboard::text(os);

    CHECK_SET_ERR(changedAln==expectedAln, "Undo works wrong\n" + changedAln);
}

GUI_TEST_CLASS_DEFINITION(test_0012){//ClustalW aligner undo test
    //Open file
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped.aln");

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(14,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString initAln = GTClipboard::text(os);

    QString expectedAln("---AAGACTTCTTTTAA---\n"
                        "---AAGCTT---ACTAA---\n"
                        "---TAGT---TTATTAA---\n"
                        "---AAGTC---TATTAA---\n"
                        "---TAGCTT---ATTAA---\n"
                        "---TAGCTT---ATTAA---\n"
                        "---TAGCTT---ATTAA---\n"
                        "---AAGTCTTT---TAA---\n"
                        "A---AGAAT--AATTA----\n"
                        "---AAGCCT---TTTAA---");

    //Use context {Edit->Align with Kalign}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_ALIGN<<"Align with ClustalW", GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new ClustalWDialogFiller(os));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));

    GTWidget::click(os, GTUtilsMdi::activeWindow(os));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(20,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString changedAln = GTClipboard::text(os);
    CHECK_SET_ERR(changedAln==expectedAln, "Unexpected alignment\n" + changedAln);

    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");
    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");

    //undo
    GTWidget::click(os,undo);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    changedAln = GTClipboard::text(os);

    CHECK_SET_ERR(changedAln==initAln, "Undo works wrong\n" + changedAln);

    //redo
    GTWidget::click(os,redo);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(20,10));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    changedAln = GTClipboard::text(os);

    CHECK_SET_ERR(changedAln==expectedAln, "Redo works wrong\n" + changedAln);
}

}//namespace GUITest_common_scenarios_undo_redo

}//namespace U2
