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
#include "api/GTMouseDriver.h"
#include "api/GTMenu.h"
#include "api/GTFileDialog.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsProject.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsApp.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsDialogRunnables.h"
#include "GTUtilsMdi.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsMdi.h"

#include <U2Core/DocumentModel.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>
#include <U2View/ADVConstants.h>
#include <QClipboard>

namespace U2{

namespace GUITest_common_scenarios_sequence_edit {

GUI_TEST_CLASS_DEFINITION(test_0001) {

    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");

    Runnable *dialog = new GTUtilsDialogRunnables::selectSequenceRegionDialogFiller(os, 1, 50);
    GTUtilsDialog::waitForDialog(os, dialog, GUIDialogWaiter::Modal);
    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    Runnable *removeDialog = new GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller(os,
        GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller::Remove,
        true,
        testDir+"_common_data/scenarios/sandbox/result.fa",
        GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller::FASTA
    );

    GTUtilsDialog::waitForDialog(os, removeDialog, GUIDialogWaiter::Modal);
    GTGlobals::sleep(1000);

    GTUtilsSequenceView::openSequenceView(os, "result.fa");

    int sequenceLength = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(sequenceLength == 199900, "Sequence length is " + QString::number(sequenceLength) + ", expected 199900");

    QString sequenceBegin = GTUtilsSequenceView::getBeginOfSequenceAsString(os, 7);
    CHECK_SET_ERR(sequenceBegin == "AGAGAGA", "Sequence starts with <" + sequenceBegin + ">, expected AGAGAGA");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");
    Runnable *dialog = new GTUtilsDialogRunnables::selectSequenceRegionDialogFiller(os, 1, 50);
    GTUtilsDialog::waitForDialog(os, dialog, GUIDialogWaiter::Modal);
    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse);
    Runnable *removeDialog = new GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller(os,
        GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller::Remove,
        true,
        testDir+"_common_data/scenarios/sandbox/result.gb",
        GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller::Genbank
    );
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTUtilsDialog::waitForDialog(os, removeDialog, GUIDialogWaiter::Modal);
    GTGlobals::sleep(1000);
    GTUtilsSequenceView::openSequenceView(os, "result.gb");
/*	QString sequence = GTSequenceViewUtils::getSequenceAsString(os);
    if (sequence.length()!= 199900) {
        os.setError("incorrect sequence length");
    }*/
}
GUI_TEST_CLASS_DEFINITION(test_0003) {

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    Runnable *filler = new GTUtilsDialogRunnables::InsertSequenceFiller(os,
        "AAAAAA", GTUtilsDialogRunnables::InsertSequenceFiller::Resize, 1,
        testDir + "_common_data/scenarios/sandbox/result.fa",
        GTUtilsDialogRunnables::InsertSequenceFiller::FASTA,
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

    Runnable *filler = new GTUtilsDialogRunnables::InsertSequenceFiller(os,
        "AAAAAA", GTUtilsDialogRunnables::InsertSequenceFiller::Resize, 1,
        testDir + "_common_data/scenarios/sandbox/result.gb",
        GTUtilsDialogRunnables::InsertSequenceFiller::Genbank,
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
    Runnable *dialog = new GTUtilsDialogRunnables::selectSequenceRegionDialogFiller(os, 1, 50);
    GTUtilsDialog::waitForDialog(os, dialog, GUIDialogWaiter::Modal);
    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse);
    Runnable *removeDialog = new GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller(os,
        GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller::Remove,
        true,
        testDir+"_common_data/scenarios/sandbox/result.fa",
        GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller::FASTA
    );
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTUtilsDialog::waitForDialog(os, removeDialog, GUIDialogWaiter::Modal);
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

    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_RESERVE_COMPLEMENT_SEQUENCE, GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
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

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    Runnable *removeDialog = new GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller(os, "2..2");
    GTUtilsDialog::waitForDialog(os, removeDialog, GUIDialogWaiter::Modal);
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

    Runnable *dialog = new GTUtilsDialogRunnables::selectSequenceRegionDialogFiller(os, 2, 2);
    GTUtilsDialog::waitForDialog(os, dialog, GUIDialogWaiter::Modal);
    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    Runnable *removeDialog = new GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller(os,
        GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller::Remove,
        false,
        testDir+"_common_data/scenarios/sandbox/result.fa",
        GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller::FASTA
        );

    GTUtilsDialog::waitForDialog(os, removeDialog, GUIDialogWaiter::Modal);
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
    Runnable *dialog = new GTUtilsDialogRunnables::selectSequenceRegionDialogFiller(os, 10, 13);
    GTUtilsDialog::waitForDialog(os, dialog, GUIDialogWaiter::Modal);
    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);
    QString sequence = QApplication::clipboard()->text();
    CHECK_SET_ERR("ACCC" == sequence, "Incorrect sequence is copied");
}

GUI_TEST_CLASS_DEFINITION(test_0010) {
    GTFileDialog::openFile(os, testDir + "_common_data/edit_sequence/", "test.gb");
    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);

    Runnable *dialog = new GTUtilsDialogRunnables::selectSequenceRegionDialogFiller(os, 1, 11);
    GTUtilsDialog::waitForDialog(os, dialog, GUIDialogWaiter::Modal);
    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);
    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << ADV_MENU_COPY << ADV_COPY_TRANSLATION_ACTION, GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep(1000);
    QString text = QApplication::clipboard()->text();

    CHECK_SET_ERR(text == "K*K", "Sequcence part translated to <" + text + ">, expected K*K");
}

GUI_TEST_CLASS_DEFINITION(test_0011) {
    GTFileDialog::openFile(os, testDir + "_common_data/edit_sequence/", "test.gb");
    GTGlobals::sleep(1000);

    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "DUMMY_1"));

    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << "ADV_MENU_COPY" << "action_copy_annotation_sequence");
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTMouseDriver::click(os);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);

    const QString expectedSequence = "AATGA";
    QString realSequence = QApplication::clipboard()->text();

    CHECK_SET_ERR(expectedSequence == realSequence, "Sequence is not pasted");
}

GUI_TEST_CLASS_DEFINITION(test_0012) {
    GTUtilsProject::openFiles(os, testDir + "_common_data/edit_sequence/test.gb");

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);

    Runnable *removeDialog = new GTUtilsDialogRunnables::RemovePartFromSequenceDialogFiller(os, "2..2");
    GTUtilsDialog::waitForDialog(os, removeDialog, GUIDialogWaiter::Modal);
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
