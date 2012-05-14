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
#include "U2View/MSAEditorSequenceArea.h"

namespace U2 {

namespace GUITest_common_scenarios_msa_editor {

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
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);

    MSAEditorSequenceArea *msaEditArea = qobject_cast<MSAEditorSequenceArea*>(GTWidget::findWidget(os, "msa_editor_sequence_area", mdiWindow));
    CHECK_SET_ERR(msaEditArea != NULL, "MsaEditorSequenceArea not found");

    QRect expectedSelection(5, 0, 1, 1);
    CHECK_SET_ERR(expectedSelection == msaEditArea->getSelection().getRect(), "Unexpected selection region");

    GTUtilsDialogRunnables::GoToDialogFiller filler1(os, 6);
    GTUtilsDialog::preWaitForDialog(os, &filler1);
    GTKeyboardDriver::keyClick(os, 'g', GTKeyboardDriver::key["ctrl"]);
    CHECK_SET_ERR(expectedSelection == msaEditArea->getSelection().getRect(), "Unexpected selection region");
}

} // namespace GUITest_common_scenarios_msa_editor
} // namespace U2
