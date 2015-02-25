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

#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsOptionPanelMSA.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"

#include "api/GTFileDialog.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWidget.h"

#include "runnables/qt/PopupChooser.h"

#include "GTTestsRegressionScenarios.h"

namespace U2 {

namespace GUITest_regression_scenarios {

GUI_TEST_CLASS_DEFINITION(test_4008) {
//    1. Open "samples/CLUSTALW/COI.aln".
//    2. Use Alignment Viewer context menu -> View.
//    Expected: "Show offsets" option is enabled.
//    3. Open "_common_data/clustal/big.aln".
//    4. Use Alignment Viewer context menu -> View.
//    Expected: "Show offsets" option is enabled.
//    Current: "Show offsets" option is disabled.

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << "MSAE_MENU_VIEW" << "show_offsets",
                                                            PopupChecker::IsEnabled | PopupChecker::IsChecable));
    GTUtilsMSAEditorSequenceArea::callContextMenu(os);
    GTGlobals::sleep();

    GTFileDialog::openFile(os, testDir + "_common_data/clustal/", "big.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << "MSAE_MENU_VIEW" << "show_offsets",
                                                            PopupChecker::IsEnabled | PopupChecker::IsChecable));
    GTUtilsMSAEditorSequenceArea::callContextMenu(os);
}

GUI_TEST_CLASS_DEFINITION(test_4026) {
    //1. Open "samples/Genbank/sars.gb".
    //Expected: there are a lot of annotations in the panoramic and details views.
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/sars.gb");

    //2. Close the MDI window.
    GTKeyboardDriver::keyClick(os, 'w', GTKeyboardDriver::key["ctrl"]);

    //3. Double click the sequence in the project.
    GTUtilsProjectTreeView::doubleClickItem(os, "NC_004718");

    //Expected: there is the same amount of annotations in the panoramic and details views.
    //Actual: annotations are now shown in the views. Their locations and qualifier names are deleted.
    QString value = GTUtilsAnnotationsTreeView::getQualifierValue(os, "evidence", "5'UTR");
    CHECK_SET_ERR("not_experimental" == value, QString("Unexpected qualifier value"));
}

GUI_TEST_CLASS_DEFINITION(test_4030) {
    //1. Open "samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/COI.aln");

    //2. Open the "Statistics" tab.
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::Statistics);

    //3. Edit the alignment.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(1, 1), QPoint(1, 1));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);

    //Expected state: hint about reference sequence is hidden
    QWidget *label = GTWidget::findWidget(os, "refSeqWarning");
    CHECK_SET_ERR(!label->isVisible(), "Label is shown");
}

}

}

