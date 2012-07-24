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

#include "GTTestsSequenceEdit.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTMouseDriver.h"
#include "api/GTMenu.h"
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
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/RangeSelectionDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/RemovePartFromSequenceDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditSequenceDialogFiller.h"

#include <U2Core/DocumentModel.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>
#include <U2View/ADVConstants.h>
#include <QClipboard>

namespace U2{

namespace GUITest_common_scenarios_sequence_edit {

GUI_TEST_CLASS_DEFINITION(test_0001) {

    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 1, 50));
    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse));

    Runnable *removeDialog = new RemovePartFromSequenceDialogFiller(os,
        RemovePartFromSequenceDialogFiller::Remove,
        true,
        testDir+"_common_data/scenarios/sandbox/result.fa",
        RemovePartFromSequenceDialogFiller::FASTA
    );
    GTUtilsDialog::waitForDialog(os, removeDialog);

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(1000);

    GTUtilsSequenceView::openSequenceView(os, "result.fa");

    int sequenceLength = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(sequenceLength == 199900, "Sequence length is " + QString::number(sequenceLength) + ", expected 199900");

    QString sequenceBegin = GTUtilsSequenceView::getBeginOfSequenceAsString(os, 7);
    CHECK_SET_ERR(sequenceBegin == "AGAGAGA", "Sequence starts with <" + sequenceBegin + ">, expected AGAGAGA");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
// Removing part from sequence
// 
// Steps:
// 
// 1. Use menu {File->Open}. Open file samples/FASTA/human_T1.fa
    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");

// 2. Click Ctrl+A. 
// Expected state: Select range dialog appears
// 
// 3. Fill the next field in dialog:
//     {Range:} 1..50
//     
    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 1, 50));
    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);

// 4. Click OK. Right click on sequence area. Use context menu {Edit sequence->Remove selected sequence}.
// Expected state: Remove subsequence dialog appears
// 
// 5. Fill the next field in dialog:
//     {Save resulted document to a new file} set checked
//     {Document format} Genbank
//     {Document location} _common_data/scenarios/sandbox/result.gb
// 6. Click Remove Button.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse));
    Runnable *removeDialog = new RemovePartFromSequenceDialogFiller(os,
        RemovePartFromSequenceDialogFiller::Remove,
        true,
        testDir+"_common_data/scenarios/sandbox/result.gb",
        RemovePartFromSequenceDialogFiller::Genbank
    );
    GTUtilsDialog::waitForDialog(os, removeDialog);
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);

// Expected state: 
//     document with edited sequence must appear in project view, 
//     sequence length in new document must be 199900
//     sequence must starts with "AGAGAGA"
    GTUtilsSequenceView::openSequenceView(os, "result.gb");
    CHECK_SET_ERR(GTUtilsSequenceView::getLengthOfSequence(os) == 199900, "Expected length differs");
    CHECK_SET_ERR(GTUtilsSequenceView::getBeginOfSequenceAsString(os, 7) == "AGAGAGA", "Expected sequence beginning differs");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    Runnable *filler = new InsertSequenceFiller(os,
        "AAAAAA", InsertSequenceFiller::Resize, 1,
        testDir + "_common_data/scenarios/sandbox/result.fa",
        InsertSequenceFiller::FASTA,
        true,
        false
    );
    GTUtilsDialog::waitForDialog(os, filler);
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_ACTIONS), QStringList() <<  ADV_MENU_EDIT << ACTION_EDIT_INSERT_SUBSEQUENCE, GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.fa");
    GTGlobals::sleep();

    GTUtilsSequenceView::openSequenceView(os, "result.fa");	

    int sequenceLength = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(sequenceLength == 199956, "Sequence length is " + QString::number(sequenceLength) + ", expected 199956");

    QString sequenceBegin = GTUtilsSequenceView::getBeginOfSequenceAsString(os, 6);
    CHECK_SET_ERR(sequenceBegin == "AAAAAA", "Sequence starts with <" + sequenceBegin + ">, expected AAAAAA");

}
GUI_TEST_CLASS_DEFINITION(test_0004) {

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    Runnable *filler = new InsertSequenceFiller(os,
        "AAAAAA", InsertSequenceFiller::Resize, 1,
        testDir + "_common_data/scenarios/sandbox/result.gb",
        InsertSequenceFiller::Genbank,
        true,
        false
    );
    GTUtilsDialog::waitForDialog(os, filler);
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_ACTIONS), QStringList() <<  ADV_MENU_EDIT << ACTION_EDIT_INSERT_SUBSEQUENCE, GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "result.gb");
    GTGlobals::sleep();

    GTUtilsSequenceView::openSequenceView(os, "result.gb");	

    int sequenceLength = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(sequenceLength == 199956, "Sequence length is " + QString::number(sequenceLength) + ", expected 199956");

    QString sequenceBegin = GTUtilsSequenceView::getBeginOfSequenceAsString(os, 6);
    CHECK_SET_ERR(sequenceBegin == "AAAAAA", "Sequence starts with <" + sequenceBegin + ">, expected AAAAAA");

}
GUI_TEST_CLASS_DEFINITION(test_0005) {

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 1, 50));
    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse));
    Runnable *removeDialog = new RemovePartFromSequenceDialogFiller(os,
        RemovePartFromSequenceDialogFiller::Remove,
        true,
        testDir+"_common_data/scenarios/sandbox/result.fa",
        RemovePartFromSequenceDialogFiller::FASTA
    );
    GTUtilsDialog::waitForDialog(os, removeDialog);
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(1000);
    GTUtilsDocument::checkDocument(os, "result.fa");
    GTGlobals::sleep(1000);
    GTUtilsSequenceView::openSequenceView(os, "result.fa");	

    int sequenceLength = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(sequenceLength == 199900, "Sequence length is " + QString::number(sequenceLength) + ", expected 199900");

    QString sequenceBegin = GTUtilsSequenceView::getBeginOfSequenceAsString(os, 7);
    CHECK_SET_ERR(sequenceBegin == "AGAGAGA", "Sequence starts with <" + sequenceBegin + ">, expected AGAGAGA");


}
GUI_TEST_CLASS_DEFINITION(test_0006) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/dp_view/", "NC_014267.gb");
    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_RESERVE_COMPLEMENT_SEQUENCE, GTGlobals::UseKey));
    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep(1000);

    QString expectedSequenceBegin = "ATCAGATT";
    QString sequenceBegin = GTUtilsSequenceView::getBeginOfSequenceAsString(os, 8);
    if (expectedSequenceBegin != sequenceBegin && !os.hasError()) {
        os.setError("Bad sequence");
    }
}

GUI_TEST_CLASS_DEFINITION(test_0007) {
    GTUtilsProject::openFiles(os, testDir + "_common_data/edit_sequence/test.gb");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse));

    GTUtilsDialog::waitForDialog(os, new RemovePartFromSequenceDialogFiller(os, "2..2"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(1000);

    QString sequenceBegin = GTUtilsSequenceView::getBeginOfSequenceAsString(os, 3);
    CHECK_SET_ERR(sequenceBegin == "AAT", "Sequence starts with " + sequenceBegin + ", expected AAT");

    int length = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(length == 29, "Sequence length is " + QString::number(length) + ", expected 29");

    bool found = GTUtilsAnnotationsTreeView::findRegion(os, "DUMMY_1", U2Region(2, 5));
    CHECK_SET_ERR(found == true, "There is no {2..5} region in annotation");
}

GUI_TEST_CLASS_DEFINITION(test_0008) {

    GTFileDialog::openFile(os, testDir + "_common_data/edit_sequence/", "test.gb");

    GTGlobals::sleep();
    QTreeWidgetItem *dummyTest = GTUtilsAnnotationsTreeView::findItem(os, "DUMMY_1", false);
    CHECK_SET_ERR(dummyTest != NULL, "There is no annotation DUMMY_1");

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 2, 2));
    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse));

    Runnable *removeDialog = new RemovePartFromSequenceDialogFiller(os,
        RemovePartFromSequenceDialogFiller::Remove,
        false,
        testDir+"_common_data/scenarios/sandbox/result.fa",
        RemovePartFromSequenceDialogFiller::FASTA
        );

    GTUtilsDialog::waitForDialog(os, removeDialog);
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    int sequenceLength = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(sequenceLength == 29, "Sequence length is " + QString::number(sequenceLength) + ", expected 29");

    QString sequenceBegin = GTUtilsSequenceView::getBeginOfSequenceAsString(os, 3);
    CHECK_SET_ERR(sequenceBegin == "AAT", "Sequence starts with <" + sequenceBegin + ">, expected AAT");

    QTreeWidgetItem *dummy1 = GTUtilsAnnotationsTreeView::findItem(os, "DUMMY_1", false);
    CHECK_SET_ERR(dummy1 == NULL, "There is annotation DUMMY_1, expected state there is no annotation DUMMY_1");
}

GUI_TEST_CLASS_DEFINITION(test_0009) {
    GTUtilsProject::openFiles(os, testDir + "_common_data/fasta/AMINO.fa");
    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 10, 13));
    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);
    GTKeyboardUtils::copy(os);
    GTGlobals::sleep(1000);
    QString sequence = QApplication::clipboard()->text();
    CHECK_SET_ERR("ACCC" == sequence, "Incorrect sequence is copied");
}

GUI_TEST_CLASS_DEFINITION(test_0010) {
    GTFileDialog::openFile(os, testDir + "_common_data/edit_sequence/", "test.gb");
    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 1, 11));
    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_COPY << ADV_COPY_TRANSLATION_ACTION, GTGlobals::UseKey));
    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep(1000);
    QString text = QApplication::clipboard()->text();

    CHECK_SET_ERR(text == "K*K", "Sequcence part translated to <" + text + ">, expected K*K");
}

GUI_TEST_CLASS_DEFINITION(test_0011) {

    GTFileDialog::openFile(os, testDir + "_common_data/edit_sequence/", "test.gb");
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_COPY" << "action_copy_annotation_sequence"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "DUMMY_1"));
    GTGlobals::sleep(1000);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(1000);

    const QString expectedSequence = "AATGA";

    QString realSequence = QApplication::clipboard()->text();
    CHECK_SET_ERR(expectedSequence == realSequence, "Sequence is not pasted");

}

GUI_TEST_CLASS_DEFINITION(test_0012) {
    GTUtilsProject::openFiles(os, testDir + "_common_data/edit_sequence/test.gb");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new RemovePartFromSequenceDialogFiller(os, "2..2"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(1000);

    QString sequenceBegin = GTUtilsSequenceView::getBeginOfSequenceAsString(os, 3);
    CHECK_SET_ERR(sequenceBegin == "AAT", "Sequence starts with " + sequenceBegin + ", expected AAT");

    int length = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(length == 29, "Sequence length is " + QString::number(length) + ", expected 29");

    bool found = GTUtilsAnnotationsTreeView::findRegion(os, "DUMMY_1", U2Region(2, 5));
    CHECK_SET_ERR(found == true, "There is no {2..5} region in annotation");
}

} // namespace
} // namespace U2
