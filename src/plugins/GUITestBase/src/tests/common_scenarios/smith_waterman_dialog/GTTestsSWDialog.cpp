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

#include <U2View/ADVConstants.h>

#include "GTUtilsProject.h"
#include "api/GTFileDialog.h"
#include "api/GTMouseDriver.h"
#include "api/GTMenu.h"
#include "api/GTTreeWidget.h"

#include "GTUtilsSequenceView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTTestsSWDialog.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsMdi.h"

#include "../../../runnables/ugene/corelibs/U2View/utils_smith_waterman/SmithWatermanDialogBaseFiller.h"
#include "../../../runnables/qt/PopupChooser.h"
#include "../../../GTUtilsMsaEditorSequenceArea.h"

namespace U2 {

namespace GUITest_common_scenarios_sw_dialog {

GUI_TEST_CLASS_DEFINITION(test_0001) {
//Performing Smith-Waterman algorithm with multiple alignment as result
//
// Steps:
//
// 1. Use menu {File->Open}. Open file _common_data/smith_waterman2/multi/06/query.txt. Get pattern sequence
	GTFileDialog::openFile(os, testDir + "_common_data/smith_waterman2/multi/06/", "query.txt");
	GTUtilsDocument::checkDocument(os, "query.txt");
	QString patternSequence;
	GTUtilsSequenceView::getSequenceAsString(os, patternSequence);
	CHECK_SET_ERR(!patternSequence.isEmpty(), "Pattern sequence is empty");

// 2. Use menu {File->Open}. Open file _common_data/smith_waterman2/multi/06/search.txt
	GTFileDialog::openFile(os, testDir + "_common_data/smith_waterman2/multi/06/", "search.txt");
	GTUtilsDocument::checkDocument(os, "search.txt");

// 3. Run Smith-Waterman Search by SW dialog
	Runnable * swDialog = new SmithWatermanDialogFiller(os, SmithWatermanSettings::MULTIPLE_ALIGNMENT, testDir + "_common_data/scenarios/sandbox/",
		patternSequence);
	GTUtilsDialog::waitForDialog(os, swDialog);

	GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "find_pattern_smith_waterman_action", GTGlobals::UseMouse));
	GTMenu::showMainMenu(os, MWMENU_ACTIONS);
	GTGlobals::sleep();


// 4. Check names of alignment files and names of found subsequences
    const QString seqNameMismatchErrorMessage = "sequences name list mismatch detected in file ";
    const QString seqNumberMismatchErrorMessage = "count of sequences mismatch detected in file ";
    const QString alignmentFileExtension = ".aln";

    for(int i = 2; i > 0; i--) {
        const QString expectedFileName = "P1_S_" + QString::number(i) + "_test]" + alignmentFileExtension;
        GTUtilsDocument::checkDocument(os, expectedFileName);
        
        GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, expectedFileName.left(expectedFileName.length()
            - alignmentFileExtension.length())));
        GTMouseDriver::doubleClick(os);
        GTGlobals::sleep();
        
        const QStringList sequencesNameList = GTUtilsMSAEditorSequenceArea::getNameList(os);
        CHECK_SET_ERR(2 == sequencesNameList.length(), seqNumberMismatchErrorMessage + expectedFileName);
        switch(i) {
        case 1:
            CHECK_SET_ERR(sequencesNameList[0] == "S_2_15_test]" && sequencesNameList[1] == "P1_4_16",
                seqNameMismatchErrorMessage + expectedFileName);
            break;

        case 2:
            CHECK_SET_ERR(sequencesNameList[0] == "S_34_42_test]" && sequencesNameList[1] == "P1_5_13",
                seqNameMismatchErrorMessage + expectedFileName);
            break;

        default:
            assert(0);
        }
        GTUtilsMdi::click(os, GTGlobals::Close);
        GTMouseDriver::click(os);
    }
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
//Performing Smith-Waterman algorithm with annotations as result
//
// Steps:
//
// 1. Use menu {File->Open}. Open file _common_data/smith_waterman2/multi/06/query.txt. Get pattern sequence
    GTFileDialog::openFile(os, testDir + "_common_data/smith_waterman2/multi/06/", "query.txt");
    GTUtilsDocument::checkDocument(os, "query.txt");
    QString patternSequence;
    GTUtilsSequenceView::getSequenceAsString(os, patternSequence);
    CHECK_SET_ERR(!patternSequence.isEmpty(), "Pattern sequence is empty");

// 2. Use menu {File->Open}. Open file _common_data/smith_waterman2/multi/06/search.txt
    GTFileDialog::openFile(os, testDir + "_common_data/smith_waterman2/multi/06/", "search.txt");
    GTUtilsDocument::checkDocument(os, "search.txt");

// 3. Run Smith-Waterman Search by SW dialog
    Runnable * swDialog = new SmithWatermanDialogFiller(os, SmithWatermanSettings::ANNOTATIONS, testDir + "_common_data/scenarios/sandbox/",
        patternSequence);
    GTUtilsDialog::waitForDialog(os, swDialog);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "find_pattern_smith_waterman_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

// 4. Сlose sequence view, then reopen it
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTMouseDriver::click(os);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "S"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep(1000);
    
// 5. Check names and count of annotations
    QTreeWidget *treeWidget = GTUtilsAnnotationsTreeView::getTreeWidget(os);
    CHECK_SET_ERR(treeWidget != NULL, "Tree widget is NULL");

    QList<QTreeWidgetItem*> treeItems = GTTreeWidget::getItems(treeWidget->invisibleRootItem());
    int annotationsCounter = 0;
    foreach (QTreeWidgetItem* item, treeItems) {
        QString treeItemName = GTUtilsAnnotationsTreeView::getAVItemName(os, (AVItem*)item);
        if ("test" == treeItemName) {
            annotationsCounter++;
        }
    }

    CHECK_SET_ERR(3 == annotationsCounter, "Result count mismatch detected");
}

} // namespace GUITest_common_scenarios_sw_dialog

} // namespace U2