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
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTAction.h"
#include "api/GTWidget.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsBookmarksTreeView.h"
#include "GTUtilsDialogRunnables.h"
#include "GTUtilsProject.h"
#include "GTUtilsMdi.h"

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
    };
    Runnable *escClicker = new EscClicker(os);

    GTUtilsDialog::waitForDialog(os, escClicker);
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep(3000);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0986_1) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(3000);

    GTUtilsDialogRunnables::SmithWatermanDialogFiller *filler = new GTUtilsDialogRunnables::SmithWatermanDialogFiller(os);
    filler->button = GTUtilsDialogRunnables::SmithWatermanDialogFiller::Cancel;
    GTUtilsDialog::waitForDialog(os, filler);

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "find_pattern_smith_waterman_action", GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTGlobals::sleep(5000);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0986_2) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(3000);

    GTRegionSelector::RegionSelectorSettings regionSelectorSettings(1, 2);
    Runnable *filler = new GTUtilsDialogRunnables::SmithWatermanDialogFiller(os, "ATCG", regionSelectorSettings);
    GTUtilsDialog::waitForDialog(os, filler);

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "find_pattern_smith_waterman_action", GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTGlobals::sleep(5000);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1001) {

    GTUtilsProject::openFiles(os, dataDir+"samples/FASTA/human_T1.fa");
    GTUtilsProject::openFiles(os, testDir+"_common_data/fasta/human_T1_cutted.fa");
    GTGlobals::sleep();

    Runnable *r = new GTUtilsDialogRunnables::DotPlotFiller(os, 4);
    GTUtilsDialog::waitForDialog(os, r);
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
    GTGlobals::sleep();

    GTGlobals::sleep(15000);
}

GUI_TEST_CLASS_DEFINITION(test_1001_1) {

    GTUtilsProject::openFiles(os, dataDir+"samples/FASTA/human_T1.fa");
    GTUtilsProject::openFiles(os, testDir+"_common_data/fasta/human_T1_cutted.fa");
    GTGlobals::sleep();

    Runnable *r = new GTUtilsDialogRunnables::DotPlotFiller(os, 100, 50);
    GTUtilsDialog::waitForDialog(os, r);
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
    GTGlobals::sleep();

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1001_2) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTUtilsProject::openFiles(os, testDir+"_common_data/fasta/human_T1_cutted.fa");
    GTGlobals::sleep();

    Runnable *r = new GTUtilsDialogRunnables::DotPlotFiller(os, 99, 99, true);
    GTUtilsDialog::waitForDialog(os, r);

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "build_dotplot_action", GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTGlobals::sleep();

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1015) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    Runnable *r = new GTUtilsDialogRunnables::DotPlotFiller(os, 3);
    GTUtilsDialog::waitForDialog(os, r);
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Close);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1015_1) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    Runnable *r = new GTUtilsDialogRunnables::DotPlotFiller(os, 30, 50);
    GTUtilsDialog::waitForDialog(os, r);
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTUtilsMdi::click(os, GTGlobals::Minimize);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1015_2) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    Runnable *r = new GTUtilsDialogRunnables::DotPlotFiller(os, 100, 50, true);
    GTUtilsDialog::waitForDialog(os, r);

    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    Runnable *chooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "build_dotplot_action", GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, chooser, GUIDialogWaiter::Popup);
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTUtilsMdi::click(os, GTGlobals::Close);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1021) {

    for (int i=0; i<2; i++) {
// 1) Open data\samples\FASTA\human_T1.fa
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

// 2) Click "build dotplot" tooltip
// 3) Click OK in opened dotplot dialog
    Runnable *r = new GTUtilsDialogRunnables::DotPlotFiller(os, 100);
    GTUtilsDialog::waitForDialog(os, r);
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
    GTGlobals::sleep();

// 4) Click on human_T1.fa project tree view item
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
    GTMouseDriver::click(os);
    GTGlobals::sleep();

// 5) Press delete key
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

// Expected state: there are no empty MDI window opened, no bookmarks
    GTGlobals::sleep();
    QWidget* activeWindow = GTUtilsMdi::activeWindow(os, GTGlobals::FindOptions(false));
    CHECK_SET_ERR(activeWindow == NULL, "there is active window");

    QTreeWidget* bookmarksTree = GTUtilsBookmarksTreeView::getTreeWidget(os);
    CHECK_SET_ERR(bookmarksTree != NULL, "bookmarksTreeWidget is NULL");

    int bookmarksCount = bookmarksTree->topLevelItemCount();
    CHECK_SET_ERR(bookmarksCount == 0, "there are bookmarks");
    }

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F4"], GTKeyboardDriver::key["alt"]);
    GTGlobals::sleep();
}


} // GUITest_regression_scenarios namespace

} // U2 namespace
