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
#include "GTUtilsProject.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsApp.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTSequenceViewUtils.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsMdi.h"
#include <U2Core/DocumentModel.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>
#include <U2View/ADVConstants.h>

namespace U2{

namespace GUITest_common_scenarios_sequence_edit {

GUI_TEST_CLASS_DEFINITION(test_0002) {
	GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");
	GTUtilsDialog::selectSequenceRegionDialogFiller dialog(os, 1, 50);
	GTUtilsDialog::preWaitForDialog(os, &dialog, GUIDialogWaiter::Modal);
	GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
	GTGlobals::sleep(1000);
	GTMenu::showMainMenu(os, MWMENU_ACTIONS);
	GTUtilsDialog::PopupChooser chooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE, GTGlobals::UseMouse);
	GTUtilsDialog::RemovePartFromSequenceDialogFiller removeDialog(os,
		GTUtilsDialog::RemovePartFromSequenceDialogFiller::Remove,
		true,
		testDir+"_common_data/scenarios/sandbox/result.gb",
		GTUtilsDialog::RemovePartFromSequenceDialogFiller::Genbank
	);
	GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
	GTUtilsDialog::preWaitForDialog(os, &removeDialog, GUIDialogWaiter::Modal);
	GTGlobals::sleep(1000);
    GTSequenceViewUtils::openSequenceView(os, "result.gb");
/*	QString sequence = GTSequenceViewUtils::getSequenceAsString(os);
	if (sequence.length()!= 199900) {
		os.setError("incorrect sequence length");
    }*/
}

GUI_TEST_CLASS_DEFINITION(test_0006) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/dp_view/", "NC_014267.gb");
    QWidget *mdiWindow = GTUtilsMdi::activeWindow(os);

    GTUtilsDialog::PopupChooser chooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_RESERVE_COMPLEMENT_SEQUENCE, GTGlobals::UseKey);
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTMenu::showContextMenu(os, mdiWindow);
    GTGlobals::sleep(1000);

    QString expectedSequenceBegin = "ATCAGATT";
    QString sequenceBegin = GTSequenceViewUtils::getBeginOfSequenceAsString(os, 8);

    if (expectedSequenceBegin != sequenceBegin && !os.hasError()) {
        os.setError("Bad sequence");
    }
}

GUI_TEST_CLASS_DEFINITION(test_0009) {
	GTUtilsProject::openFiles(os, testDir + "_common_data/fasta/AMINO.fa");
	GTUtilsDialog::selectSequenceRegionDialogFiller dialog(os, 10, 13);
	GTUtilsDialog::preWaitForDialog(os, &dialog, GUIDialogWaiter::Modal);
	GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
	GTGlobals::sleep(1000);
	GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
	GTGlobals::sleep(1000);
	QString sequence = QApplication::clipboard()->text();
	CHECK_SET_ERR("ACCC" == sequence, "Incorrect sequence is copied");
}

} // namespace
} // namespace U2


