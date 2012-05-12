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

#include "GTTestsRegressionScenarios.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "GTUtilsDialogRunnables.h"

#include <U2View/ADVConstants.h>

namespace U2 {

namespace GUITest_regression_scenarios {

GUI_TEST_CLASS_DEFINITION(test_0986) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(3000);

    class EscClicker : public Runnable {
    public:
        EscClicker(U2OpStatus& _os) : os(_os){}
        virtual void run(){ GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["esc"]); }
        U2OpStatus& os;
    } escClicker(os);

    GTUtilsDialog::preWaitForDialog(os, &escClicker);
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep(3000);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0986_1) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(3000);

    GTUtilsDialogRunnables::SmithWatermanDialogFiller filler(os);
    filler.button = GTUtilsDialogRunnables::SmithWatermanDialogFiller::Cancel;
    GTUtilsDialog::preWaitForDialog(os, &filler);

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTUtilsDialogRunnables::PopupChooser chooser(os, QStringList() << ADV_MENU_ANALYSE << "find_pattern_smith_waterman_action", GTGlobals::UseMouse);
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTGlobals::sleep(5000);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0986_2) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(3000);

    GTRegionSelector::RegionSelectorSettings regionSelectorSettings(1, 2);
    GTUtilsDialogRunnables::SmithWatermanDialogFiller filler(os, "ATCG", regionSelectorSettings);
    GTUtilsDialog::preWaitForDialog(os, &filler);

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTUtilsDialogRunnables::PopupChooser chooser(os, QStringList() << ADV_MENU_ANALYSE << "find_pattern_smith_waterman_action", GTGlobals::UseMouse);
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTGlobals::sleep(5000);

    GTGlobals::sleep(5000);
}

} // GUITest_regression_scenarios namespace

} // U2 namespace
