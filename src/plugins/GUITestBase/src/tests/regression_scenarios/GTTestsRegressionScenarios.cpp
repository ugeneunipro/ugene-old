    /**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "../../GTUtilsMsaEditorSequenceArea.h"
#include "../../GTUtilsEscClicker.h"
#include "GTTestsRegressionScenarios.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTAction.h"
#include "api/GTWidget.h"
#include "api/GTTreeWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTClipboard.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsOptionsPanel.h"
#include "GTUtilsBookmarksTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsDialog.h"
#include "GTUtilsProject.h"
#include "GTUtilsMdi.h"
#include "runnables/ugene/corelibs/U2View/utils_smith_waterman/SmithWatermanDialogBaseFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ConsensusSelectorDialogFiller.h"
#include "runnables/ugene/plugins/dotplot/DotPlotDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/RangeSelectionDialogFiller.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/AlignShortReadsDialogFiller.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/corelibs/U2Gui/util/ProjectTreeItemSelectorDialogBaseFiller.h"
#include "runnables/ugene/corelibs/U2Gui/DownloadRemoteFileDialogFiller.h"
#include "runnables/ugene/ugeneui/SelectDocumentFormatDialogFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/umuscle/MuscleDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/kalign/KalignDialogFiller.h"
#include "GTUtilsLog.h"
#include <U2View/ADVSingleSequenceWidget.h>


#include <U2View/ADVConstants.h>
#include <U2View/MSAEditor.h>

namespace U2 {

namespace GUITest_regression_scenarios {

GUI_TEST_CLASS_DEFINITION(test_0734) {
    //1. Open "_common_data/fasta/test.TXT".
    GTUtilsProject::openFiles(os, testDir + "_common_data/fasta/test.TXT");

    //2. Open "_common_data/clustal/test_alignment.aln".
    GTUtilsProject::openFiles(os, testDir + "_common_data/clustal/test_alignment.aln");

    //Expected state: two documents are opened in the project view; MSA Editor are shown with test_alignment.
    QTreeWidgetItem *seqDoc = GTUtilsProjectTreeView::findItem(os, "test.TXT");
    QTreeWidgetItem *msaDoc = GTUtilsProjectTreeView::findItem(os, "test_alignment.aln");
    QWidget *msaView = GTUtilsMdi::activeWindow(os);
    CHECK(NULL != seqDoc && NULL != msaDoc, );
    CHECK(NULL != msaView, );

    //3. Drag'n'drop "Sequence4" object of "test.TXT" document from the project tree to the MSA Editor.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_LOAD << "Sequence from current project"));
    GTUtilsDialog::waitForDialog(os, new ProjectTreeItemSelectorDialogBaseChecker(os, "[s] Sequence4"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();

    //Expected state: UGENE does not crash; a new "Sequence4" row appears in the alignment.
    QStringList names = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(names.size() == 4, QString("Sequence count mismatch. Expected: %1. Actual: %2").arg(4).arg(names.size()));
    CHECK_SET_ERR(names.last() == "Sequence4",
        QString("Inserted sequence name mismatch. Expected: %1. Actual: %2").arg("Sequence4").arg(names.last()));
}

GUI_TEST_CLASS_DEFINITION(test_0928) {
    // 1. Open "samples/FASTA/human_T1.fa".
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");

    class OkClicker : public Filler {
    public:
        OkClicker(U2OpStatus& _os) : Filler(_os, "ORFDialogBase"){}
        virtual void run() {
            QWidget *w = QApplication::activeWindow();
            CHECK(NULL != w, );
            QDialogButtonBox *buttonBox = w->findChild<QDialogButtonBox*>(QString::fromUtf8("buttonBox"));
            CHECK(NULL != buttonBox, );
            QPushButton *button = buttonBox->button(QDialogButtonBox::Ok);
            CHECK(NULL != button, );
            GTWidget::click(os, button);
        }
    };

    // 2. Click "Find ORFs" button on the sequence view toolbar.
    // Expected state: ORF dialog appears. UGENE does not crash.
    // 3. Click "Ok" button and wait for the end of the task.
    GTUtilsDialog::waitForDialog(os, new OkClicker(os));
    GTWidget::click(os, GTAction::button(os, "Find ORFs"));
    GTGlobals::sleep();

    // Expected state: 837 orfs are found.
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "orf");
    CHECK(NULL != item, );
    CHECK_SET_ERR(item->childCount() == 837, QString("ORFs count mismatch. Expected: %1. Actual: %2").arg(837).arg(item->childCount()));
}

GUI_TEST_CLASS_DEFINITION(test_0986) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(3000);

    class EscClicker : public Filler {
    public:
        EscClicker(U2OpStatus& _os) : Filler(_os, "SmithWatermanDialogBase"){}
        virtual void run(){
            GTGlobals::sleep();
#ifdef Q_OS_MAC
            GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
            GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["cmd"]);
            GTWidget::click(os,GTWidget::findWidget(os,"bttnCancel"));
#else
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);
#endif
        }
    };

    GTUtilsDialog::waitForDialog(os, new EscClicker(os));
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep(3000);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0986_1) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(3000);

    SmithWatermanDialogFiller *filler = new SmithWatermanDialogFiller(os);
    filler->button = SmithWatermanDialogFiller::Cancel;
    GTUtilsDialog::waitForDialog(os, filler);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "find_pattern_smith_waterman_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(5000);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0986_2) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(3000);

    GTRegionSelector::RegionSelectorSettings regionSelectorSettings(1, 2);
    GTUtilsDialog::waitForDialog(os, new SmithWatermanDialogFiller(os, "ATCG", regionSelectorSettings));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "find_pattern_smith_waterman_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(5000);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0986_3) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(3000);

    GTRegionSelector::RegionSelectorSettings regionSelectorSettings(1, 2);
    GTUtilsDialog::waitForDialog(os, new SmithWatermanDialogFiller(os, "ATCGAT", regionSelectorSettings));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "find_pattern_smith_waterman_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(5000);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0986_4) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(3000);

    GTRegionSelector::RegionSelectorSettings regionSelectorSettings(1, 2);
    GTUtilsDialog::waitForDialog(os, new SmithWatermanDialogFiller(os, "ATCGAT", regionSelectorSettings));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "find_pattern_smith_waterman_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(5000);

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1001) {

    GTUtilsProject::openFiles(os, dataDir+"samples/FASTA/human_T1.fa");
    GTUtilsProject::openFiles(os, testDir+"_common_data/fasta/human_T1_cutted.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 4));
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
    GTGlobals::sleep();

    GTGlobals::sleep(15000);
}

GUI_TEST_CLASS_DEFINITION(test_1001_1) {

    GTUtilsProject::openFiles(os, dataDir+"samples/FASTA/human_T1.fa");
    GTUtilsProject::openFiles(os, testDir+"_common_data/fasta/human_T1_cutted.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 100, 50));
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
    GTGlobals::sleep();

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1001_2) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTUtilsProject::openFiles(os, testDir+"_common_data/fasta/human_T1_cutted.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 99, 99, true));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "build_dotplot_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1001_3) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTUtilsProject::openFiles(os, testDir+"_common_data/fasta/human_T1_cutted.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 99, 99, true));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "build_dotplot_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(5000);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1001_4) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTUtilsProject::openFiles(os, testDir+"_common_data/fasta/human_T1_cutted.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 99, 99, true));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "build_dotplot_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(5000);

    GTUtilsMdi::click(os, GTGlobals::Close);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1015) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 3));
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Close);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1015_1) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 30, 50));
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTUtilsMdi::click(os, GTGlobals::Minimize);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1015_2) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 100, 50, true));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "build_dotplot_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTUtilsMdi::click(os, GTGlobals::Close);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1015_3) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 100, 50, true));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "build_dotplot_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTUtilsMdi::click(os, GTGlobals::Close);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1015_4) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 100, 50, true));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "build_dotplot_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTUtilsMdi::click(os, GTGlobals::Minimize);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1021) {

    for (int i=0; i<2; i++) {
        // 1) Open data\samples\FASTA\human_T1.fa
        GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
        GTGlobals::sleep();

        // 2) Click "build dotplot" tooltip
        // 3) Click OK in opened dotplot dialog
        GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 100));
        GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
        GTGlobals::sleep();

        // 4) Click on human_T1.fa project tree view item
        GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
        GTMouseDriver::click(os);

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

//    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F4"], GTKeyboardDriver::key["alt"]);
//    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_1021_1) {

    for (int i=0; i<3; i++) {
        // 1) Open data\samples\FASTA\human_T1.fa
        GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
        GTGlobals::sleep();

        // 2) Click "build dotplot" tooltip
        // 3) Click OK in opened dotplot dialog
        GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 100));
        GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
        GTGlobals::sleep();

        // 4) Click on human_T1.fa project tree view item
        GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
        GTMouseDriver::click(os);

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

//    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F4"], GTKeyboardDriver::key["alt"]);
//    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_1021_2) {

    for (int i=0; i<2; i++) {
        // 1) Open data\samples\FASTA\human_T1.fa
        GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
        GTGlobals::sleep();

        // 2) Click "build dotplot" tooltip
        // 3) Click OK in opened dotplot dialog
        GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 100));
        GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
        GTGlobals::sleep();

        // 4) Click on human_T1.fa project tree view item
        GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
        GTMouseDriver::click(os);

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

//    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION__EXIT);
//    GTGlobals::sleep();

//    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_1021_3) {

    for (int i=0; i<2; i++) {
        // 1) Open data\samples\FASTA\human_T1.fa
        GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
        GTGlobals::sleep();

        // 2) Click "build dotplot" tooltip
        // 3) Click OK in opened dotplot dialog
        GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 120, 100, true));
        GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
        GTGlobals::sleep();

        if (i==0) {
            GTUtilsMdi::click(os, GTGlobals::Minimize);
            GTGlobals::sleep();
        }

        // 4) Click on human_T1.fa project tree view item
        GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
        GTMouseDriver::click(os);

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

//    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION__EXIT);
//    GTGlobals::sleep();

//    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_1021_4) {

    for (int i=0; i<2; i++) {
        // 1) Open data\samples\FASTA\human_T1.fa
        GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
        GTGlobals::sleep();

        // 2) Click "build dotplot" tooltip
        // 3) Click OK in opened dotplot dialog
        GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 110, 100, true));
        GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
        GTGlobals::sleep();

        if (i==0) {
            GTUtilsMdi::click(os, GTGlobals::Minimize);
            GTGlobals::sleep();
        }

        // 4) Click on human_T1.fa project tree view item
        GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
        GTMouseDriver::click(os);

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

//    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION__EXIT);
//    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_1022) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 3));
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
    GTMouseDriver::click(os);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_1199) {
//1. Open any samples/PDB/1CF7.pdb file.
    GTFileDialog::openFile(os, dataDir+"samples/PDB/", "1CF7.PDB");

//2. Select document in project tree view. Press 'Delete'
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1CF7.PDB"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
        
//Expected state: document removed, UGENE not crashes.
    GTUtilsProject::checkProject(os, GTUtilsProject::Empty);
}

GUI_TEST_CLASS_DEFINITION(test_1083) {
//1. open files data\samples\FASTA\human_T1.fa
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
//2. select sequence oject in projrct tree view. press delete
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
//Expected state: UGENE is not crashed
    GTUtilsProject::checkProject(os, GTUtilsProject::Empty);
}

GUI_TEST_CLASS_DEFINITION(test_1093) {
//    1. do menu "tools->Align to refrence->Align short reads"

//    2. fill next fields in appeared dialog:
//        Refrence sequence "_common_data\scenarios\_regression\1093\refrence.fa"
//        SAM output checkbox {set checked}
//        Short reads add next file: "_common_data\scenarios\_regression\1093\read.fa"

//    3. Press 'Align'
//    Expected state: message window appears "The short reads can't be mapped to the reference sequence!"
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, testDir + "_common_data/scenarios/_regression/1093/","refrence.fa", testDir + "_common_data/scenarios/_regression/1093/", "read.fa",true)) ;
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));

    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Align to reference" << "Align short reads");

    GTGlobals::sleep(5000);
//The short reads can't be mapped to the reference sequence!
}

GUI_TEST_CLASS_DEFINITION(test_1107){//commit GUIInitionalChecks
//1) Open an MSA file (e.g. _common_data\scenarios\msa\ma2_gapped.aln)

    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/msa/", "ma2_gapped.aln");
//2) Menu File->Close Project
//3) Press No in the Save current project dialog
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
//Expected state: UGENE not crashes
}
GUI_TEST_CLASS_DEFINITION(test_1113){//commit AboutDialogController.cpp
//1. Open UGENE
//2. Press F1
    class EscClicker : public Filler {
    public:
        EscClicker(U2OpStatus& _os) : Filler(_os, "AboutDialog"){}
        virtual void run(){

            GTGlobals::sleep();
#ifdef Q_OS_MAC
            GTKeyboardDriver::keyRelease(os,GTKeyboardDriver::key["f1"]);
#endif

            QWidget* dialog = QApplication::activeModalWidget();
//getting an info string
            QWidget *w = new QWidget();
            w=dialog->findChild<QWidget*>("about_widget");
            CHECK_SET_ERR(w!=NULL, "aboutWidget not found");

            QObject *parent = new QObject();
            parent= w->findChild<QObject*>("parent");
            CHECK_SET_ERR(parent!=NULL, "parentObject not found");

            QObject *child = new QObject();
            child=parent->findChild<QObject*>();
            CHECK_SET_ERR(child!=NULL, "childObject not found");

            QString text = child->objectName();
            CHECK_SET_ERR(text.contains("64-bit")||text.contains("32-bit"),text);
#ifdef Q_OS_MAC
            GTWidget::click(os,GTWidget::findWidget(os,"close_button"));
#else
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);
#endif
        }
    };
    GTGlobals::sleep(1000);
    GTUtilsDialog::waitForDialog(os, new EscClicker(os));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["f1"]);
    GTGlobals::sleep(1000);
//Expected state: About dialog appeared, shown info includes platform info (32/64)

}

GUI_TEST_CLASS_DEFINITION(test_1113_1){//commit AboutDialogController.cpp
//1. Open UGENE
//2. Help->About
    class EscClicker : public Filler {
    public:
        EscClicker(U2OpStatus& _os) : Filler(_os, "AboutDialog"){}
        virtual void run(){
            GTGlobals::sleep();
#ifdef Q_OS_MAC
            GTMouseDriver::release(os);
#endif
            QWidget* dialog = QApplication::activeModalWidget();
//getting an info string
            QWidget *w = new QWidget();
            w=dialog->findChild<QWidget*>("about_widget");
            CHECK_SET_ERR(w!=NULL, "aboutWidget not found");

            QObject *parent = new QObject();
            parent= w->findChild<QObject*>("parent");
            CHECK_SET_ERR(parent!=NULL, "parentObject not found");

            QObject *child = new QObject();
            child=parent->findChild<QObject*>();
            CHECK_SET_ERR(child!=NULL, "childObject not found");

            QString text = child->objectName();
            CHECK_SET_ERR(text.contains("64-bit")||text.contains("32-bit"),text);
#ifdef Q_OS_MAC
            GTWidget::click(os,GTWidget::findWidget(os,"close_button"));
#else
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);
#endif
        }
    };

    GTUtilsDialog::waitForDialog(os, new EscClicker(os));
    QMenu *menu = GTMenu::showMainMenu(os, MWMENU_HELP);
    GTMenu::clickMenuItem(os, menu, QStringList() << ACTION__ABOUT);
    GTGlobals::sleep(1000);
//Expected state: About dialog appeared, shown info includes platform info (32/64)

}

GUI_TEST_CLASS_DEFINITION(test_1165){
//1. Open file "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");
//2. open context menu in msa, "Add"->"Sequence from file...",
//3. browse the file "data/samples/FASTA/human_T1.fa".
    QWidget *nameList;
    nameList=GTWidget::findWidget(os,"msa_editor_name_list");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_LOAD_SEQ"<<"Sequence from file"));
    GTFileDialogUtils *ob = new GTFileDialogUtils(os, dataDir + "/samples/FASTA/", "human_T1.fa");
    GTUtilsDialog::waitForDialog(os, ob);

    GTMenu::showContextMenu(os,nameList);
//4. Then choose any sequence in sequence names area (except that which you've just added), press "Delete"
//Expected state: UGENE not crashes
    GTWidget::click(os,nameList);
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["delete"]);
}

GUI_TEST_CLASS_DEFINITION(test_1189){
//1) Open samples/FASTA/human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
//2) Press Ctrl+F, click "Show more options"
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    GTWidget::click(os, GTWidget::findWidget(os, "lblShowMoreLess"));
//3) Select any region of the sequence
    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os,100,200));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Select"<< "Sequence region"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));
//4) Select "Selected region" in "Region" combobox of "Search in" area.
    QComboBox *regBox =(QComboBox*)GTWidget::findWidget(os, "boxRegion");
    GTComboBox::setCurrentIndex(os,regBox,2);
//5) Ensure that two lineedits became visible and contain correct region
    QLineEdit *start = (QLineEdit*)GTWidget::findWidget(os,"editStart");
    CHECK_SET_ERR(start->isVisible(), "editStart line is not visiable");
    CHECK_SET_ERR(start->text()=="100","Wrong startValue. Current value is: "+start->text());

    QLineEdit *end = (QLineEdit*)GTWidget::findWidget(os,"editEnd");
    CHECK_SET_ERR(end->isVisible(), "editEnd line is not visiable");
    CHECK_SET_ERR(end->text()=="201","Wrong endValue. Current value is: "+end->text());
}

GUI_TEST_CLASS_DEFINITION(test_1189_1){
//1) Open samples/FASTA/human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
//2) Press Ctrl+F, click "Show more options"
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    GTWidget::click(os, GTWidget::findWidget(os, "lblShowMoreLess"));
//3) Select any region of the sequence
    GTWidget::click(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));
    QPoint p;
    p=GTMouseDriver::getMousePosition();
    p.setX(p.x()-100);
    GTMouseDriver::moveTo(os,p);
    GTMouseDriver::press(os);
    p.setX(p.x()+200);
    GTMouseDriver::moveTo(os,p);
    GTMouseDriver::release(os);
//4) Select "Selected region" in "Region" combobox of "Search in" area.
    QComboBox *regBox =(QComboBox*)GTWidget::findWidget(os, "boxRegion");
    GTComboBox::setCurrentIndex(os,regBox,2);
//5) Ensure that two lineedits became visible and contain correct region
    QLineEdit *start = (QLineEdit*)GTWidget::findWidget(os,"editStart");
    CHECK_SET_ERR(start->isVisible(), "editStart line is not visiable");

    QLineEdit *end = (QLineEdit*)GTWidget::findWidget(os,"editEnd");
    CHECK_SET_ERR(end->isVisible(), "editEnd line is not visiable");
}

GUI_TEST_CLASS_DEFINITION(test_1190){//add AlignShortReadsFiller

//1) Align shortreads with genome aligner
//Tools -> Align to reference -> Align short reads
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, testDir + "_common_data/fasta/","N.fa", testDir + "_common_data/fasta/", "RAW.fa",true)) ;
    QMenu *menu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTLogTracer l;

    GTMenu::clickMenuItem(os, menu, QStringList() << "Align to reference" << "Align short reads");

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os,QMessageBox::Ok));
    GTGlobals::sleep(10000);

    GTUtilsLog::check(os, l);

    //}
//Reference sequence: _common_data/fasta/N.fa  /home/vmalin/ugene/trunk/test/_common_data/fasta/N.ugenedb
//Short reads: _common_data/reads/shortreads15Mb.fasta

//Click "Start"

//2) wait for dialog to appear, click "OK"

//repeat these steps 3 times, UGENE shouldn't crash
}
GUI_TEST_CLASS_DEFINITION(test_1212){
//    1. Open any sequence. (human_t1.fa)
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
//    2. Do "Select->Sequence Region..."
//    3. In single selection mode enter any region
//    4. Press "Go" and UGENE hangs up/crashes
    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os,"100..200"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Select"<< "Sequence region"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));
//    Expected: region is selected
    ADVSingleSequenceWidget *w=(ADVSingleSequenceWidget*)GTWidget::findWidget(os,"ADV_single_sequence_widget_0");
    CHECK_SET_ERR(!w->getSequenceSelection()->isEmpty(), "No selected region");
}

GUI_TEST_CLASS_DEFINITION(test_1212_1){
//    1. Open any sequence. (human_t1.fa)
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
//    2. Do "Select->Sequence Region..."
//    3. In single selection mode enter any region
//    4. Press "Go" and UGENE hangs up/crashes
    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os,"0..199950"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Select"<< "Sequence region"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));
//    Expected: region is selected
    ADVSingleSequenceWidget *w=(ADVSingleSequenceWidget*)GTWidget::findWidget(os,"ADV_single_sequence_widget_0");
    CHECK_SET_ERR(!w->getSequenceSelection()->isEmpty(), "No selected region");

}

GUI_TEST_CLASS_DEFINITION(test_1252){
//    1. Open human_t1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
//    2. Find any pattern. A new annotation document is created
    GTUtilsOptionsPanel::runFindPatternWithHotKey("TTTTTAAAAA", os);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature");
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
//    3. Delete found annotations from human_t1 annotations tree
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
//    4. Delete created annotations document
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "MyDocument.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ACTION_PROJECT__REMOVE_SELECTED));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(500);
//check delition of annotation document
   CHECK_SET_ERR(GTUtilsProjectTreeView::findItem(os, "Annotations", GTGlobals::FindOptions(false))==NULL, "Annotations document not deleted");
//    5. Click search again
    GTWidget::click(os, GTWidget::findWidget(os,"btnSearch"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    item = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature");
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
// delete annotations manually to cache MessageBox
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "MyDocument.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ACTION_PROJECT__REMOVE_SELECTED));
    GTMouseDriver::click(os, Qt::RightButton);
//    Expected: pattern is found and annotation is stored in a new document

}

GUI_TEST_CLASS_DEFINITION(test_1252_1){
//DIFFERENCE: DEL KEY IS USED TO DELETE ANNOTATION DOCUMENT
//    1. Open human_t1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
//    2. Find any pattern. A new annotation document is created
    GTUtilsOptionsPanel::runFindPatternWithHotKey("TTTTTAAAAA", os);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature");
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
//    3. Delete found annotations from human_t1 annotations tree
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
//    4. Delete created annotations document
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "MyDocument.gb"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(500);
//check delition of annotation document
    CHECK_SET_ERR(GTUtilsProjectTreeView::findItem(os, "Annotations", GTGlobals::FindOptions(false))==NULL, "Annotations document not deleted");
//    5. Click search again
    GTWidget::click(os, GTWidget::findWidget(os,"btnSearch"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    item = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature");
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
// delete annotations manually to cache MessageBox
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "MyDocument.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ACTION_PROJECT__REMOVE_SELECTED));
    GTMouseDriver::click(os, Qt::RightButton);
//    Expected: pattern is found and annotation is stored in a new document
}

GUI_TEST_CLASS_DEFINITION(test_1255){
//1. Open human_T1.fa sequence
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
//2. Open Find Pattern on the Option Panel. Enter a vaild pattern
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTKeyboardDriver::keySequence(os, "TA");
//3. Input invalid annotation name (empty, too long, illegal)
    GTWidget::click(os, GTWidget::findWidget(os, "lblShowMoreLess"));
    GTWidget::click(os, GTWidget::findWidget(os, "titleWidget"));
    for (int i=0; i<15; i++){
        GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["down"]);
        GTGlobals::sleep(50);
    }

    QLabel *label;
    label = (QLabel*)GTWidget::findWidget(os,"lblErrorMessage");

    QLineEdit *annotationNameEdit;
    annotationNameEdit=(QLineEdit *)GTWidget::findWidget(os,"annotationNameEdit");

//3.1 Input invalid annotation name (empty)
    GTLineEdit::setText(os,annotationNameEdit,"");
    QString s=label->text();
    CHECK_SET_ERR(s.contains("empty"),"Error message is: "+s);
    GTGlobals::sleep(500);

    for (int i=0; i<5; i++){
        GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["down"]);
        GTGlobals::sleep(50);
    }

//3.2 Input invalid annotation name (illegal)

    GTLineEdit::setText(os,annotationNameEdit," test");
    s=label->text();
    CHECK_SET_ERR(s.contains("Illegal"),"Error message is: "+s);
    GTGlobals::sleep(500);

    for (int i=0; i<5; i++){
        GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["down"]);
        GTGlobals::sleep(50);
    }
//3.3 Input invalid annotation name (illegal)
    GTLineEdit::setText(os,annotationNameEdit,"Too long annotation name");
    s=label->text();
    CHECK_SET_ERR(s.contains("too long"),"Error message is: "+s);
    GTGlobals::sleep(500);
//Expected: error message appears if the annotations are invalid
}

GUI_TEST_CLASS_DEFINITION(test_1262) {
    //1. Open human_t1.fa
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    //2. Find any pattern. A new annotation document is created
    GTUtilsOptionsPanel::runFindPatternWithHotKey("AGGAAAAAATGCTAAGGGCAGCCAGAGAGAGGTCAGG", os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ACTION_PROJECT__REMOVE_SELECTED));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));

    //3. Delete created annotations document
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "MyDocument.gb"));
    GTMouseDriver::click(os, Qt::RightButton);
    //4. Click search again
    GTWidget::click(os, GTWidget::findWidget(os, "btnSearch"));

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature");
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));

    //delete new doc
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ACTION_PROJECT__REMOVE_SELECTED));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "MyDocument.gb"));
    GTMouseDriver::click(os, Qt::RightButton);


}

GUI_TEST_CLASS_DEFINITION(test_1475) {
    GTUtilsDialog::waitForDialog(os, new SelectDocumentFormatDialogFiller(os));
    GTFileDialog::openFile(os, testDir + "_common_data/raw_sequence/", "NC_000117.txt");
}

GUI_TEST_CLASS_DEFINITION(test_1508) {
    //1. Open COI2.fa as an alignment
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Join));
    GTFileDialog::openFile(os, testDir+"_common_data/fasta/", "COI2.fa");

    //2. {MSA Editor context menu} -> Align -> Align with MUSCLE
    //3. Choose the mode "Refine only"
    GTUtilsDialog::waitForDialog(os, new MuscleDialogFiller(os, MuscleDialogFiller::Refine));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with muscle", GTGlobals::UseMouse));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_1527) {
    //1. Open COI2.aln as an alignment
    GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");
    GTGlobals::sleep();

    //2. {MSA Editor context menu} -> Align -> Align profile to profile with MUSCLE
    //3. Select empty "test.aln" in the profile browsing dialog.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align profile to profile with MUSCLE", GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/regression/1527/", "test.aln"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));

}

GUI_TEST_CLASS_DEFINITION(test_1527_1) {
//1. Open COI2.aln as an alignment
GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");
GTGlobals::sleep();

//2. {MSA Editor context menu} -> Align -> Align sequences to profile with MUSCLE
//3. Select empty "test.aln" in the profile browsing dialog.
GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align sequences to profile with MUSCLE", GTGlobals::UseMouse));
GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/regression/1527/", "test.aln"));
GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
}

GUI_TEST_CLASS_DEFINITION( test_1622 )
{
    // 1. Open document "ma.aln"
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma.aln" );

    // 2. Save the initial content
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 11, 17 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString initialContent = GTClipboard::text( os );

    // 2.1. Remove selection
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );
    GTGlobals::sleep(200);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( ) );

    // 3. Select a region in the sequence area
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 5, 4 ), QPoint( 10, 12 ) );

    // 4. Shift the region
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 6, 7 ), QPoint( 11, 7 ) );

    // 5. Obtain undo button
    QAbstractButton *undo = GTAction::button( os, "msa_action_undo" );

    // 6. Undo shifting, e.g. alignment should restore to the init state
    GTWidget::click( os, undo );

    // 7. Check the undone state
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 11, 17 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString undoneContent = GTClipboard::text( os );
    CHECK_SET_ERR( undoneContent == initialContent,
        "Undo works wrong. Found text is: " + undoneContent );
}

GUI_TEST_CLASS_DEFINITION(test_1689){
//1. Open "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");
//2. Select "Consensus mode..." in the context menu on MSA area
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Consensus mode",GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new ConsensusSelectionDialogFiller(os,1,10));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
//3. Remember current threshold value (by default: 100% for the default algorithm)

//4. Change threshold value with spinbox: delete one zero. New value is 10%

//5. Click the "OK" button

//6. Repeat the 2nd step
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Consensus mode",GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new ConsensusSelectionDialogFiller(os,1,10,true));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
//7. Restore previous value (with the "Reset" button, or by entering the deleted zero into the spinbox)

//8. Click the "OK" button

//9. Close MSA Editor
     GTUtilsMdi::click(os,GTGlobals::Close);

//10. Open "COI.aln" from the project view
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os, "COI"));
    GTMouseDriver::doubleClick(os);
//11. Repeat the 2nd step
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Consensus mode",GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new CheckConsensusValues(os,-1,100));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));
//Expected state: the threshold is 100%
}

GUI_TEST_CLASS_DEFINITION( test_1703 )
{
    // 1. Open document "ma.aln"
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma.aln" );

    // 2. Select some row in the name list area
    GTUtilsMSAEditorSequenceArea::click( os, QPoint( -5, 6 ) );
    GTGlobals::sleep(200);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( 0, 6, 12, 1 ) );

    GTKeyboardDriver::keyPress(os,GTKeyboardDriver::key["shift"]);
    // 3. Select the upper row
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["up"] );
    GTGlobals::sleep(500);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( 0, 5, 12, 2 ) );

    // 4. Select the bottom row
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["down"] );
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["down"] );
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyRelease(os,GTKeyboardDriver::key["shift"]);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( 0, 6, 12, 2 ) );
}

GUI_TEST_CLASS_DEFINITION(test_1708){
    //1. Open COI.aln or HIV-1.aln from samples
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(9,1));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString initAln = GTClipboard::text(os);

    QString expectedAln("TAAGACTT-C\n"
                        "TAAG-CTTAC");

    //2. Align with KAlign
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_ALIGN<<"align_with_kalign", GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os,10));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));

    GTWidget::click(os, GTUtilsMdi::activeWindow(os));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(9,1));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString changedAln = GTClipboard::text(os);
    CHECK_SET_ERR(changedAln==expectedAln, "Unexpected alignment\n" + changedAln);

    QAbstractButton *undo= GTAction::button(os,"msa_action_undo");

    //3. Press Undo
    GTWidget::click(os,undo);
    GTWidget::click(os, GTUtilsMdi::activeWindow(os));
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(9,1));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    changedAln = GTClipboard::text(os);

    CHECK_SET_ERR(changedAln==initAln, "Undo works wrong\n" + changedAln);
}

GUI_TEST_CLASS_DEFINITION(test_1720){
//1. Use menu {File->Access remote database...}
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFiller(os, "D11266", 0));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE),ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);

    GTLogTracer l;
    GTGlobals::sleep(8000);//some time needed for request
//2. Fill field "Resource ID" with value D11266. Click "OK"

//3. Use menu {File->Access remote database...}
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFiller(os, "D11266", 0));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE),ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);

    GTGlobals::sleep(8000);
//4. Fill field "Resource ID" with value D11266. Click "OK"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "D11266.gb"));
    GTUtilsLog::check(os,l);
//Expected state: project view with document "D11266.gb", no error messages in log appear
}

GUI_TEST_CLASS_DEFINITION( test_1884 )
{
    // 1. Open document "ma.aln"
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma.aln" );

    // 2. Select some row in the name list area
    const int startRowNumber = 6;
    const int alignmentLength = 12;
    GTUtilsMSAEditorSequenceArea::click( os, QPoint( -5, startRowNumber ) );
    GTGlobals::sleep( 200 );
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( 0, startRowNumber,
        alignmentLength, 1 ) );

    // 3. Select all the upper rows and make some extra "selection"
    const int extraUpperSelectionCount = 3;
    GTKeyboardDriver::keyPress( os, GTKeyboardDriver::key["shift"] );
    GTGlobals::sleep(200);
    for ( int i = 0; i < extraUpperSelectionCount; ++i ) {
        GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["up"] );
        GTGlobals::sleep(200);
    }
    const int upperSequencesCount = 4;
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( 0, startRowNumber-extraUpperSelectionCount,
        alignmentLength, upperSequencesCount ) );

    // 4. Decrease the selection
    const int deselectionCount = extraUpperSelectionCount - 1;
    for ( int i = 0; i < deselectionCount; ++i ) {
        GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["down"] );
        GTGlobals::sleep(200);
    }
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( 0, startRowNumber-1,
        alignmentLength, upperSequencesCount - deselectionCount ) );
    GTKeyboardDriver::keyRelease( os, GTKeyboardDriver::key["shift"] );
}

GUI_TEST_CLASS_DEFINITION( test_1886_1 )
{
    // 1. Open document "ma.aln"
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma.aln" );

    // 2. Select a region in the sequence area
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 5, 4 ), QPoint( 10, 12 ) );

    // 3. Shift the region but don't release left mouse button
    const QPoint mouseDragPosition( 7, 7 );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseDragPosition );
    GTMouseDriver::press( os );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseDragPosition + QPoint( 3, 0 ) );

    // 4. Call context menu
    GTUtilsDialog::waitForDialog( os, new GTUtilsEscClicker( os, "msa sequence area context menu" ) );
    GTMouseDriver::click( os, Qt::RightButton );
    GTGlobals::sleep( 200 );
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( QPoint( 8, 4 ), QPoint( 13, 12 ) ) );

    // 5. Release left mouse button
    GTMouseDriver::release( os );
    GTGlobals::sleep( 200 );

    // 6. Insert gaps with the Space button
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["space"] );
    GTGlobals::sleep( 500 );
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( QPoint( 14, 4 ), QPoint( 19, 12 ) ) );
}

GUI_TEST_CLASS_DEFINITION( test_1886_2 )
{
    // 1. Open document "ma.aln"
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma.aln" );

    // 2. Select a region in the sequence area
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 5, 4 ), QPoint( 10, 12 ) );

    // 3. Shift the region but don't release left mouse button
    const QPoint mouseDragPosition( 7, 7 );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseDragPosition );
    GTMouseDriver::press( os );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseDragPosition + QPoint( 3, 0 ) );

    // 4. Replace selected rows with reverse
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT
        << "replace_selected_rows_with_reverse") );
    GTMouseDriver::click(os, Qt::RightButton);

    // 5. Obtain selection
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text( os );
    CHECK_SET_ERR( "--TGAC\n--TGAT\n--AGAC\n--AGAT\n--AGAT\n"
        "--TGAA\n--CGAT\n--CGAT\n--CGAT" == selectionContent, "MSA changing is failed" );
}

} // GUITest_regression_scenarios namespace

} // U2 namespace
