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

#include "GTTestsRegressionScenarios.h"

#include "api/GTAction.h"
#include "api/GTCheckBox.h"
#include "api/GTClipboard.h"
#include "api/GTComboBox.h"
#include "api/GTFileDialog.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTLineEdit.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTSpinBox.h"
#include "api/GTTableView.h"
#include "api/GTToolbar.h"
#include "api/GTTreeWidget.h"
#include "api/GTWidget.h"

#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsBookmarksTreeView.h"
#include "GTUtilsDialog.h"
#include "GTUtilsEscClicker.h"
#include "GTUtilsLog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsOptionsPanel.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsWorkflowDesigner.h"

#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/AlignShortReadsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/AppSettingsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ConvertAssemblyToSAMDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/ugene/corelibs/U2Gui/DownloadRemoteFileDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditAnnotationDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditQualifierDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditSequenceDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/FindTandemsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ImportBAMFileDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/RangeSelectionDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/util/ProjectTreeItemSelectorDialogBaseFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/BuildTreeDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ConsensusSelectorDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/LicenseAgreemntDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/utils_smith_waterman/SmithWatermanDialogBaseFiller.h"
#include "runnables/ugene/plugins/annotator/FindAnnotationCollocationsDialogFiller.h"
#include "runnables/ugene/plugins/dotplot/DotPlotDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/WorkflowMetadialogFiller.h"
#include "runnables/ugene/plugins/external_tools/TCoffeeDailogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/StartupDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/kalign/KalignDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/umuscle/MuscleDialogFiller.h"
#include "runnables/ugene/ugeneui/NCBISearchDialogFiller.h"
#include "runnables/ugene/ugeneui/SelectDocumentFormatDialogFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include "runnables/ugene/ugeneui/SaveProjectDialogFiller.h"

#include <U2Core/AppContext.h>
#include <U2Core/ExternalToolRegistry.h>

#include <U2View/ADVConstants.h>
#include <U2View/ADVSingleSequenceWidget.h>
#include <U2View/AnnotatedDNAViewFactory.h>
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
    AlignShortReadsFiller::Parameters parameters(testDir + "_common_data/scenarios/_regression/1093/",
                                                 "refrence.fa",
                                                 testDir + "_common_data/scenarios/_regression/1093/",
                                                 "read.fa");
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, &parameters)) ;
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
    GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
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
//2) Press Ctrl+F
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
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
//2) Press Ctrl+F
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
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
    AlignShortReadsFiller::Parameters parameters(testDir + "_common_data/fasta/",
                                                 "N.fa",
                                                 testDir + "_common_data/fasta/",
                                                 "RAW.fa");
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, &parameters)) ;
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
    GTGlobals::sleep(200);
    GTKeyboardDriver::keySequence(os, "TA");
//3. Input invalid annotation name (empty, too long, illegal)
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
#ifdef Q_OS_MAC
    GTGlobals::sleep(10000);
#endif
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
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFiller(os, "", 0));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE),ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);

    GTLogTracer l;
    GTGlobals::sleep(8000);//some time needed for request
//2. Fill field "Resource ID" with value D11266. Click "OK"

//3. Use menu {File->Access remote database...}
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFiller(os, "", 0));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE),ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);

    GTGlobals::sleep(8000);
//4. Fill field "Resource ID" with value D11266. Click "OK"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "D11266.gb"));
    GTUtilsLog::check(os,l);
//Expected state: project view with document "D11266.gb", no error messages in log appear
}

GUI_TEST_CLASS_DEFINITION( test_1813 )
{
    // 1) Select the menu {File->Access remote database}
    // 2) Fill the "Fetch Data from Remote Database" dialog with the next values:
    //      Resource ID: I7G8J3
    //      Database: UniProt (DAS)
    // 3) Press "OK"
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFiller(os, "", 7));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE),ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);

    // Expected: the sequence view with I7G8J3 sequence is opened. UGENE does not crash.
    GTGlobals::sleep(20000);
    GTUtilsDocument::isDocumentLoaded(os, "I7G8J3_das.gb");
    GTUtilsDocument::checkDocument(os, "I7G8J3_das.gb", AnnotatedDNAViewFactory::ID);
}

GUI_TEST_CLASS_DEFINITION( test_1821 ) {
    QString workflowOutputDirPath( testDir + "_common_data/scenarios/sandbox" );
    QDir workflowOutputDir( workflowOutputDirPath );

    GTUtilsDialog::waitForDialog( os, new StartupDialogFiller( os, true,
        workflowOutputDir.absolutePath( ) ) );

    //1. Open WD
    QMenu* menu=GTMenu::showMainMenu( os, MWMENU_TOOLS );
    GTMenu::clickMenuItem( os, menu, QStringList( ) << "Workflow Designer" );
    GTGlobals::sleep( 500 );

    //2. Select "Align sequences with MUSCLE"
    GTUtilsWorkflowDesigner::addSample( os, "Align sequences with MUSCLE" );
    GTGlobals::sleep( 500 );

    //3. Change the value of the scale spinbox. E.g. set it to 75%
    QComboBox *scaleCombo = dynamic_cast<QComboBox *>( GTWidget::findWidget( os, "wdScaleCombo" ) );
    CHECK_SET_ERR( NULL != scaleCombo, "Unable to find scale combobox!" );
    GTComboBox::setIndexWithText( os, scaleCombo, "75%" );

    //4. Store the scheme to some file using "Save scheme as" button
    GTUtilsDialog::waitForDialog( os, new WorkflowMetaDialogFiller( os,
        workflowOutputDir.absolutePath( ) + "/" + "test.uwl", "Scheme") );
    GTMenu::clickMenuItem( os, GTMenu::showMainMenu( os, MWMENU_ACTIONS ),
        QStringList( ) <<  "Save workflow action", GTGlobals::UseKey );

    //5. Close WD
    GTUtilsMdi::click( os, GTGlobals::Close );
    GTMouseDriver::click( os );

    //6. Open the file containing the saved scheme using "Open" button
    GTFileDialog::openFile( os, workflowOutputDirPath, "test.uwl" );

    //Expected state: scheme is opened in WD, its scale is 75%
    scaleCombo = dynamic_cast<QComboBox *>( GTWidget::findWidget( os, "wdScaleCombo" ) );
    CHECK_SET_ERR( NULL != scaleCombo, "Unable to find scale combobox!" );
    CHECK_SET_ERR( scaleCombo->currentText( ) == "75%", "Unexpected scale value!" );
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

GUI_TEST_CLASS_DEFINITION( test_1919 )
{
    //1) Create the WD scheme: File list -> File conversions.
    //2) Set input file: a BAM file (e.g _common_data/bam/scerevisiae.bam).
    //3) Set the result format of the converter: BAM
    //4) Run the scheme.
    //Expected: the scheme is finished well, no errors appeared.

    GTLogTracer l;

    QMenu *menu=GTMenu::showMainMenu( os, MWMENU_TOOLS );
    GTMenu::clickMenuItem( os, menu, QStringList() << "Workflow Designer" );

    GTUtilsWorkflowDesigner::addAlgorithm( os, "File List" );
    GTUtilsWorkflowDesigner::addAlgorithm( os, "File Format Conversion" );

    WorkflowProcessItem* fileList = GTUtilsWorkflowDesigner::getWorker(os, "File List");
    WorkflowProcessItem* fileConversion = GTUtilsWorkflowDesigner::getWorker(os, "File Format Conversion");

    GTUtilsWorkflowDesigner::connect(os, fileList, fileConversion);

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "File List"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/bam", "scerevisiae.bam");

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "File Format Conversion"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setParameter(os, "Document format", 1, GTUtilsWorkflowDesigner::comboValue);

    // add setting source url in input data

    //GTWidget::click(os,GTAction::button(os,"Run workflow"));

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION( test_1921 )
{
//    1. Click rapidly on the "Show/hide log" button several (20) times (or use the Alt+3 hotkey).
    GTGlobals::sleep(2000);
    for (int i = 0; i < 20; i++) {
        GTKeyboardDriver::keyClick(os, '3', GTKeyboardDriver::key["alt"]);
        GTGlobals::sleep(20);
    }
//    Expected state: UGENE not crashes.
}

GUI_TEST_CLASS_DEFINITION( test_2006 )
{
    const int MSA_WIDTH = 50;
    const int MSA_HEIGHT = 3;

    // 1. Open "data/samples/CLUSTAL/COI.aln" and save it's part to a string
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "COI.aln" );
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( MSA_WIDTH, MSA_HEIGHT ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString initialMsaContent = GTClipboard::text( os );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );

    // 2. Select the second symbol in the first line
    const QPoint initialSelectionPos( 1, 0 );
    GTUtilsMSAEditorSequenceArea::click( os, initialSelectionPos );
    GTGlobals::sleep(200);

    // 3. Drag it to the first symbol in the first line
    const QPoint mouseDragPosition( 1, 0 );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseDragPosition );
    GTMouseDriver::press( os );
    GTGlobals::sleep( 200 );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseDragPosition + QPoint( 0, 0 ) );
    GTMouseDriver::release( os );
    GTGlobals::sleep( 200 );

    // 4. Check that the content has not been changed
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( MSA_WIDTH, MSA_HEIGHT ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR( initialMsaContent == finalMsaContent, "MSA has unexpectedly changed" );

    // 5. Check that "Undo" and "Redo" buttons are disabled
    const QAbstractButton *undo = GTAction::button( os, "msa_action_undo" );
    CHECK_SET_ERR( !undo->isEnabled( ), "Undo button is unexpectedly enabled" );
    const QAbstractButton *redo = GTAction::button( os, "msa_action_redo" );
    CHECK_SET_ERR( !redo->isEnabled( ), "Redo button is unexpectedly enabled" );
}

GUI_TEST_CLASS_DEFINITION( test_2007 )
{
    // 1. Open "_common_data/scenarios/msa/ma_empty_line.aln (and save it to string)
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma_empty_line.aln" );
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 11, 4 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString initialMsaContent = GTClipboard::text( os );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );

    // 2. Select the fourth column of the second, third and fourth lines
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 3, 1 ), QPoint( 3, 3 ) );
    GTGlobals::sleep(200);

    // 3. Drag it to the left.
    const QPoint mouseDragPosition( 3, 2 );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseDragPosition );
    GTMouseDriver::press( os );
    GTGlobals::sleep( 200 );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseDragPosition - QPoint( 1, 0 ) );
    GTMouseDriver::release( os );
    GTGlobals::sleep( 200 );

    // Expected state: nothing happens
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 11, 4 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR( initialMsaContent == finalMsaContent, "MSA has changed unexpectedly!" );
}

GUI_TEST_CLASS_DEFINITION( test_2012 ){
    //1. Open {data/samples/CLUSTALW/COI.aln}.
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "COI.aln" );

    //2. Open the "General" tab on the options panel.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_GENERAL"));
    GTGlobals::sleep(500);
    QLineEdit *sequencelineEdit = (QLineEdit*)GTWidget::findWidget(os, "sequenceLineEdit");
    CHECK(NULL != sequencelineEdit, );

    GTLineEdit::setText(os, sequencelineEdit, "m");
    GTGlobals::sleep(500);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
    GTGlobals::sleep(500);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
    GTGlobals::sleep(500);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
    GTGlobals::sleep(500);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep(500);

    QString selectedSequence = sequencelineEdit->text();
    CHECK("Mecopoda_elongata__Sumatra_" == selectedSequence, );

    //3. Set focus to the reference sequence name line edit and enter 'm' into it.
}

GUI_TEST_CLASS_DEFINITION( test_2021_1 )
{
    // 1. Open document "ma.aln" and save it to string
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma.aln" );
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 11, 17 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString initialMsaContent = GTClipboard::text( os );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );

    // 2. Select a character in the sequence area
    const QPoint initialSelectionPos( 10, 10 );
    GTUtilsMSAEditorSequenceArea::click( os, initialSelectionPos );
    GTGlobals::sleep(200);

    // 3. Add gaps by pressing "Space" key
    const int totalShiftCount = 3;
    int shiftCounter = 0;
    QPoint currentSelection = initialSelectionPos;
    while ( shiftCounter++ < totalShiftCount ) {
        GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["space"] );
        GTGlobals::sleep(200);
    }
    currentSelection.rx( ) += totalShiftCount;

    // 4. Remove gaps with "Backspace" key
    shiftCounter = 0;
    while ( shiftCounter++ < totalShiftCount ) {
        GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"] );
        GTGlobals::sleep(200);
        currentSelection.rx( ) -= 1;
        GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( currentSelection,
            currentSelection ) );
    }

    // 5. Check that alignment content has returned to initial state
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 11, 17 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR( initialMsaContent == finalMsaContent, "MSA has unexpectedly changed" );
}

GUI_TEST_CLASS_DEFINITION( test_2021_2 )
{
    // 1. Open document "ma.aln" and save it to string
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma.aln" );
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 11, 17 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString initialMsaContent = GTClipboard::text( os );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );

    // 2. Select a region in the sequence area
    const QPoint initialSelectionLeftTop( 5, 5 );
    const QPoint initialSelectionRightBottom( 11, 10 );
    const int selectionWidth = initialSelectionRightBottom.x( ) - initialSelectionLeftTop.x( ) + 1;
    GTUtilsMSAEditorSequenceArea::selectArea( os, initialSelectionLeftTop,
        initialSelectionRightBottom );
    GTGlobals::sleep(200);

    // 3. Add gaps by pressing "Space" key
    const int totalShiftCount = 3;
    int shiftCounter = 0;
    QPoint currentSelectionLeftTop = initialSelectionLeftTop;
    QPoint currentSelectionRightBottom = initialSelectionRightBottom;
    while ( shiftCounter++ < totalShiftCount ) {
        GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["space"] );
        GTGlobals::sleep( 200 );
    }
    currentSelectionLeftTop.rx( ) += totalShiftCount * selectionWidth;
    currentSelectionRightBottom.rx( ) += totalShiftCount * selectionWidth;
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( currentSelectionLeftTop,
        currentSelectionRightBottom ) );

    // 4. Remove gaps with "Backspace" key
    shiftCounter = 0;
    while ( shiftCounter++ < totalShiftCount ) {
        GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"] );
        GTGlobals::sleep( 200 );
        currentSelectionLeftTop.rx( ) -= selectionWidth;
        currentSelectionRightBottom.rx( ) -= selectionWidth;
        GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( currentSelectionLeftTop,
            currentSelectionRightBottom ) );
    }

    // 5. Check that alignment content has returned to initial state
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 11, 17 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR( initialMsaContent == finalMsaContent, "MSA has unexpectedly changed" );
}

GUI_TEST_CLASS_DEFINITION( test_2021_3 )
{
    // 1. Open document "ma2_gap_8_col.aln"
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma2_gap_8_col.aln" );

    // 2. Select a column in the sequence area
    GTUtilsMSAEditorSequenceArea::moveTo( os, QPoint( 0, 0 ) );
    GTMouseDriver::click( os );
    GTUtilsMSAEditorSequenceArea::selectColumnInConsensus( os, 9 );
    GTGlobals::sleep( 200 );

    // 3. Remove gap columns by pressing "Backspace" key
    const int totalShiftCount = 6;
    int shiftCounter = 0;
    while ( shiftCounter++ < totalShiftCount ) {
        GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"] );
        GTGlobals::sleep( 200 );
    }

    // 4. Check that all the empty columns were removed
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 14, 9 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR(  "AAGCTTCTTTTAA--\nAAGTTACTAA-----\nTAG---TTATTAA--\nAAGC---TATTAA--\n"
                    "TAGTTATTAA-----\nTAGTTATTAA-----\nTAGTTATTAA-----\nAAGCTTT---TAA--\n"
                    "A--AGAATAATTA--\nAAGCTTTTAA-----" == finalMsaContent,
                    "Unexpected MSA content has occurred" );
}

GUI_TEST_CLASS_DEFINITION( test_2021_4 )
{
    // 1. Open document "ma2_gap_8_col.aln" and save it to string
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma2_gap_8_col.aln" );
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 20, 9 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString initialMsaContent = GTClipboard::text( os );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );

    // 2. Select a region of trailing gaps
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 17, 4 ), QPoint( 19, 6 ) );

    // 3. Press "Backspace"
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"] );
    GTGlobals::sleep( 200 );

    // 4. Check that MSA does not changed
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 20, 9 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR( initialMsaContent == finalMsaContent, "MSA has unexpectedly changed" );

    // 5. Check that "Undo" and "Redo" buttons are disabled
    const QAbstractButton *undo = GTAction::button( os, "msa_action_undo" );
    CHECK_SET_ERR( !undo->isEnabled( ), "Undo button is unexpectedly enabled" );
    const QAbstractButton *redo = GTAction::button( os, "msa_action_redo" );
    CHECK_SET_ERR( !redo->isEnabled( ), "Redo button is unexpectedly enabled" );
}

GUI_TEST_CLASS_DEFINITION( test_2021_5 )
{
    // 1. Open document "ma2_gap_8_col.aln"
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma2_gap_8_col.aln" );

    // 2. Select a column in the sequence area
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 9, 0 ), QPoint( 16, 9 ) );

    // 3. Remove gap columns by pressing "Backspace" key
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"] );
    GTGlobals::sleep( 200 );

    // 4. Check that all the empty columns were removed
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 14, 9 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR(  "AAGCTTCTTTTAA--\nAAGTTACTAA-----\nTAG---TTATTAA--\nAAGC---TATTAA--\n"
        "TAGTTATTAA-----\nTAGTTATTAA-----\nTAGTTATTAA-----\nAAGCTTT---TAA--\n"
        "A--AGAATAATTA--\nAAGCTTTTAA-----" == finalMsaContent,
        "Unexpected MSA content has occurred" );
}

GUI_TEST_CLASS_DEFINITION( test_2021_6 )
{
    // 1. Open "data/samples/CLUSTAL/COI.aln".
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "COI.aln" );

    //2. Set cursor to the position 45 of the first line (after gaps).
    const QPoint initialSelectionPos( 44, 0 );
    GTUtilsMSAEditorSequenceArea::click( os, initialSelectionPos );
    GTGlobals::sleep(200);

    //3. Press BACKSPACE.
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"] );
    GTGlobals::sleep( 200 );

    // 4. Expected state: the gap was deleted, selection moves to the previous symbol.
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 43, 0 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR(  "TAAGACTTCTAATTCGAGCCGAATTAGGTCAACCAGGATAC--C" == finalMsaContent,
                    "Unexpected MSA content has occurred" );
}

GUI_TEST_CLASS_DEFINITION( test_2021_7 )
{
    // 1. Open "data/samples/CLUSTAL/COI.aln".
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "COI.aln" );

    //2. Select the 45 and 46 of the second line (two symbols after gaps).
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 44, 1 ), QPoint( 45, 1 ) );
    GTGlobals::sleep(200);

    //3. Press BACKSPACE.
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"] );
    GTGlobals::sleep( 200 );

    // 4. Expected state: the gap was deleted, selection moves to the previous symbol.
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 1 ), QPoint( 44, 1 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR(  "TAAGCTTACTAATCCGGGCCGAATTAGGTCAACCTGGTTAT-CTA" == finalMsaContent,
                    "Unexpected MSA content has occurred" );

}

GUI_TEST_CLASS_DEFINITION( test_2021_8 )
{
    // 1. Open "data/samples/CLUSTAL/COI.aln".
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "COI.aln" );

    //2. Select the 45 and 46 of the second line (two symbols after gaps).
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 44, 1 ), QPoint( 45, 1 ) );
    GTGlobals::sleep(200);

    //3. Press BACKSPACE.
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"] );
    GTGlobals::sleep( 200 );

    // 4. Expected state: the gap was deleted, selection moves to the previous symbol.
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 1 ), QPoint( 44, 1 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR(  "TAAGCTTACTAATCCGGGCCGAATTAGGTCAACCTGGTTAT-CTA" == finalMsaContent,
                    "Unexpected MSA content has occurred" );
}

GUI_TEST_CLASS_DEFINITION( test_2021_9 )
{
    // 1. Open "data/samples/CLUSTAL/COI.aln".
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "COI.aln" );

    //2. Select the 45 and 46 of the second line (two symbols after gaps).
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 44, 2 ), QPoint( 46, 2 ) );
    GTGlobals::sleep(200);

    //3. Press BACKSPACE.
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"] );
    GTGlobals::sleep( 200 );

    // 4. Expected state: the gap was deleted, selection moves to the previous symbol.
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 2 ), QPoint( 46, 2 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR(  "TTAGTTTATTAATTCGAGCTGAACTAGGTCAACCAGGCTATTTAATT" == finalMsaContent,
                    "Unexpected MSA content has occurred" );
}

GUI_TEST_CLASS_DEFINITION(test_2024){
//    1. Open WD
    QMenu *menu=GTMenu::showMainMenu( os, MWMENU_TOOLS );
    GTMenu::clickMenuItem( os, menu, QStringList( ) << "Workflow Designer" );

//    2. Add element "Local BLAST Search"
    GTUtilsWorkflowDesigner::addAlgorithm( os, "Local BLAST Search" );
    GTMouseDriver::moveTo(os,GTUtilsWorkflowDesigner::getItemCenter(os, "blast"));
    GTMouseDriver::click(os);

//    Expected state: element has parameters "gap cost" and "match score"
    GTUtilsWorkflowDesigner::setParameter(os, "gap cost", 2, GTUtilsWorkflowDesigner::comboValue);
    GTUtilsWorkflowDesigner::setParameter(os, "Match scores", 1, GTUtilsWorkflowDesigner::comboValue);

}

GUI_TEST_CLASS_DEFINITION( test_2026 ) {
    // 1. Open data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "COI.aln" );

    // 2. Select Montana_montana in name list
    GTUtilsMSAEditorSequenceArea::selectSequence(os, QString("Montana_montana"));

    // 3. Press shift, click down_arrow 4 times. Release shift.
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);
    for (int i = 0; i < 4; ++i) {
        GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["down"] );
        GTGlobals::sleep( 500 );
    }
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep(3000);

    // Expected state: 5 sequences are selected
    CHECK_SET_ERR( 5 == GTUtilsMSAEditorSequenceArea::getSelectedSequencesNum(os),
        "Unexpected number of selected sequences");
    CHECK_SET_ERR( GTUtilsMSAEditorSequenceArea::isSequenceSelected(os, QString("Montana_montana")),
        "Expected sequence is not selected");
    CHECK_SET_ERR( GTUtilsMSAEditorSequenceArea::isSequenceSelected(os, QString("Zychia_baranovi")),
        "Expected sequence is not selected");

    // 4. Release shift. Press shift again and click down arrow
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["down"] );
    GTGlobals::sleep( 500 );
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep(3000);

    // Expected state: 6 sequences selected
    CHECK_SET_ERR( 6 == GTUtilsMSAEditorSequenceArea::getSelectedSequencesNum(os),
        "Unexpected number of selected sequences");
    CHECK_SET_ERR( GTUtilsMSAEditorSequenceArea::isSequenceSelected(os, QString("Montana_montana")),
        "Expected sequence is not selected");
    CHECK_SET_ERR( GTUtilsMSAEditorSequenceArea::isSequenceSelected(os, QString("Zychia_baranovi")),
        "Expected sequence is not selected");
    CHECK_SET_ERR( GTUtilsMSAEditorSequenceArea::isSequenceSelected(os, QString("Tettigonia_viridissima")),
        "Expected sequence is not selected");
}

GUI_TEST_CLASS_DEFINITION( test_2030 ) {
    // 1. Open {_common_data/scenarios/msa/ma_one_line.aln}
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma_one_line.aln");

    // 2. Set cursor to the end of the line
    const QPoint endLinePos( 11, 0 );
    GTUtilsMSAEditorSequenceArea::click( os, endLinePos );
    GTGlobals::sleep(200);

    // 3. Insert four gaps with SPACE.
    for (int i = 0; i < 4; ++i) {
        GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["space"] );
        GTGlobals::sleep( 500 );
    }

    // 4. Set cursor to the beginning of the line
    const QPoint begLinePos( 0, 0 );
    GTUtilsMSAEditorSequenceArea::click( os, begLinePos );
    GTGlobals::sleep(200);

    // 5. Delete one symbol with DELETE
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["delete"] );
    GTGlobals::sleep( 200 );

    // Expected state: line length is 15.
    CHECK_SET_ERR( 15 == GTUtilsMSAEditorSequenceArea::getLength( os ),
        "Unexpected MSA length!" );
}

GUI_TEST_CLASS_DEFINITION( test_2032 ) {
    // 1. Open {_common_data/fasta/abcd.fa} as separate sequences
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Separate));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "abcd.fa");
    GTGlobals::sleep(500);

    // Expected result: order of sequences in the project view is {d, a, c, b}
    QTreeWidgetItem *documentItem = GTUtilsProjectTreeView::findItemByText(os,
        GTUtilsProjectTreeView::getTreeWidget(os),
        "abcd.fa",
        GTGlobals::FindOptions(true));

    CHECK_SET_ERR(NULL != documentItem, "No document in the project view!");
    CHECK_SET_ERR(4 == documentItem->childCount(), "Unexpected number of objects in the document!");
    CHECK_SET_ERR("[s] d" == documentItem->child(0)->text(0), "Unexpected name of the object in the project view!");
    CHECK_SET_ERR("[s] a" == documentItem->child(1)->text(0), "Unexpected name of the object in the project view!");
    CHECK_SET_ERR("[s] c" == documentItem->child(2)->text(0), "Unexpected name of the object in the project view!");
    CHECK_SET_ERR("[s] b" == documentItem->child(3)->text(0), "Unexpected name of the object in the project view!");

    // Expected result: order of sequences in the sequences view is {d, a, c, b}
    ADVSingleSequenceWidget *seq0 = dynamic_cast<ADVSingleSequenceWidget*>(GTWidget::findWidget(os, "ADV_single_sequence_widget_0"));
    ADVSingleSequenceWidget *seq1 = dynamic_cast<ADVSingleSequenceWidget*>(GTWidget::findWidget(os, "ADV_single_sequence_widget_1"));
    ADVSingleSequenceWidget *seq2 = dynamic_cast<ADVSingleSequenceWidget*>(GTWidget::findWidget(os, "ADV_single_sequence_widget_2"));
    ADVSingleSequenceWidget *seq3 = dynamic_cast<ADVSingleSequenceWidget*>(GTWidget::findWidget(os, "ADV_single_sequence_widget_3"));

    CHECK_SET_ERR(NULL != seq0, "Failed to find a sequence widget!");
    CHECK_SET_ERR(NULL != seq1, "Failed to find a sequence widget!");
    CHECK_SET_ERR(NULL != seq2, "Failed to find a sequence widget!");
    CHECK_SET_ERR(NULL != seq3, "Failed to find a sequence widget!");

    CHECK_SET_ERR("d" == seq0->getSequenceObject()->getSequenceName(), "Unexpected sequence name!");
    CHECK_SET_ERR("a" == seq1->getSequenceObject()->getSequenceName(), "Unexpected sequence name!");
    CHECK_SET_ERR("c" == seq2->getSequenceObject()->getSequenceName(), "Unexpected sequence name!");
    CHECK_SET_ERR("b" == seq3->getSequenceObject()->getSequenceName(), "Unexpected sequence name!");
}

GUI_TEST_CLASS_DEFINITION( test_2070 ){
    GTLogTracer lt;
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFiller(os, "", 7));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE),ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTGlobals::sleep();

    GTGlobals::sleep(18000);
    GTUtilsDocument::isDocumentLoaded(os, "Q9IGQ6_das.gb");
    GTUtilsDocument::checkDocument(os, "Q9IGQ6_das.gb", AnnotatedDNAViewFactory::ID);
    
    GTUtilsLog::check(os, lt);
}
GUI_TEST_CLASS_DEFINITION( test_2077 ){
    // 1) Open WD

    // 2) Add elements "Read Sequence" and "Write sequence" to the scheme
    // 3) Connect "Read Sequence" to "Write sequence"

    QMenu *menu=GTMenu::showMainMenu( os, MWMENU_TOOLS );
    GTMenu::clickMenuItem( os, menu, QStringList( ) << "Workflow Designer" );

    GTUtilsWorkflowDesigner::addAlgorithm( os, "Read Sequence" );
    GTUtilsWorkflowDesigner::addAlgorithm( os, "Write Sequence" );

    WorkflowProcessItem *seqReader = GTUtilsWorkflowDesigner::getWorker( os, "Read Sequence" );
    WorkflowProcessItem *seqWriter = GTUtilsWorkflowDesigner::getWorker( os, "Write Sequence" );

    GTUtilsWorkflowDesigner::connect( os, seqReader, seqWriter );

    // 4) Add file "human_T1.fa" to "Read Sequence" twice 

    GTMouseDriver::moveTo( os, GTUtilsWorkflowDesigner::getItemCenter( os, "Read Sequence" ) );
    GTMouseDriver::click( os );
    QString dirPath = dataDir + "samples/FASTA/";
    GTUtilsWorkflowDesigner::setDatasetInputFile( os, dirPath, "human_T1.fa" );

    GTMouseDriver::moveTo( os, GTUtilsWorkflowDesigner::getItemCenter( os, "Read Sequence" ) );
    GTMouseDriver::click( os );
    QString dirPath1 = dataDir + "samples/FASTA/";
    GTUtilsWorkflowDesigner::setDatasetInputFile( os, dirPath1, "human_T1.fa" );

    // 5) Validate scheme
    GTUtilsDialog::waitForDialog( os, new MessageBoxDialogFiller( os, QMessageBox::Ok,
        "Please fix issues listed in the error list (located under workflow)." ) );
    GTWidget::click( os,GTAction::button( os,"Validate workflow" ) );
    GTGlobals::sleep( 200 );

    //Expected state: The "File not found" error has appeared in the "Error list"
    GTUtilsWorkflowDesigner::checkErrorList( os, "file '../human_T1.fa' was specified several times" );
    }

GUI_TEST_CLASS_DEFINITION( test_2089 )
{
    // 1. Start UGENE with a new *.ini file.
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os, true, "!@#$%^&*()_+\";:?/", false));

    // 2. Open WD
    // 3. Set any folder without write access as workflow output directory. Click OK.
    QMenu *menu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
}

GUI_TEST_CLASS_DEFINITION( test_2100_1 ){
    //1. Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    //2. Select Hetrodes_pupus_EF540832
    GTUtilsMSAEditorSequenceArea::selectSequence(os, QString("Hetrodes_pupus_EF540832"));

    //2. Click toolbutton "Enable collapsing"
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Enable collapsing"));

    //Expected state: Hetrodes_pupus_EF540832 is still selected
    CHECK_SET_ERR( GTUtilsMSAEditorSequenceArea::isSequenceSelected(os, QString("Hetrodes_pupus_EF540832")),
                   "Required sequence is not selected");
}

GUI_TEST_CLASS_DEFINITION( test_2100_2 ){
    //1. Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    //2. Click toolbutton "Enable collapsing"
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Enable collapsing"));

    //expected state: Mecopoda_elongata__Ishigaki__J and Mecopoda_elongata__Sumatra_ are collapsed
    CHECK_SET_ERR( !GTUtilsMSAEditorSequenceArea::isSequenceVisible(os, QString("Mecopoda_elongata__Sumatra_")),
                   "Required sequence is not collapsed");

    //3. Select Mecopoda_sp.__Malaysia_
    GTUtilsMSAEditorSequenceArea::selectSequence(os, QString("Mecopoda_sp.__Malaysia_"));

    //4. Expand Mecopoda_elongata__Ishigaki__J


    //Expected state: Mecopoda_sp.__Malaysia_ is still selected
    CHECK_SET_ERR( GTUtilsMSAEditorSequenceArea::isSequenceSelected(os, QString("Mecopoda_sp.__Malaysia_")),
                   "Expected sequence is not selected");
}

GUI_TEST_CLASS_DEFINITION( test_2100_3 ){
    //1. Open COI.aln
    GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");

    //2. Click toolbutton "Enable collapsing"
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Enable collapsing"));

    //3. Select Mecopoda_sp.__Malaysia_
    GTUtilsMSAEditorSequenceArea::selectSequence(os, QString("Mecopoda_sp.__Malaysia_"));

    //4. Expand Select Mecopoda_sp.__Malaysia_
    //Expected state: Mecopoda_elongata__Ishigaki__J and Mecopoda_elongata__Sumatra_ are selected

}

GUI_TEST_CLASS_DEFINITION( test_2122 ){
//    1. Open some sequence translated to amino
    GTFileDialog::openFile(os, testDir + "_common_data/fasta", "AMINO.fa");
//    2. Click Option panel button "das annotations"
    GTWidget::click(os,GTWidget::findWidget(os, "OP_DAS"));
    GTGlobals::sleep(500);
//    3. Click "show more options"
    GTWidget::click(os,GTWidget::findWidget(os,"lblShowMoreLess"),Qt::LeftButton,QPoint(10,10));
//    Expected state:Algorithm settings, DAS features sourses, Annotation settings submenus appeared
    QWidget* settingsContainerWidget = GTWidget::findWidget(os,"settingsContainerWidget");
    QWidget* sourcesContainerWidget = GTWidget::findWidget(os,"sourcesContainerWidget");
    QWidget* annotationsSettingsContainerWidget = GTWidget::findWidget(os,"annotationsSettingsContainerWidget");

    CHECK_SET_ERR(settingsContainerWidget->isVisible(), "settingsContainerWidget is not visible");
    CHECK_SET_ERR(sourcesContainerWidget->isVisible(), "sourcesContainerWidget is not visible");
    CHECK_SET_ERR(annotationsSettingsContainerWidget->isVisible(), "annotationsSettingsContainerWidget is not visible");
}

GUI_TEST_CLASS_DEFINITION( test_2124 ) {
    // 1. Open "data/samples/CLUSTALW/ty3.aln.gz".
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "ty3.aln.gz" );

    // 2. Call the context menu on the sequence area.
    const QString colorSchemeName = "Scheme";
    GTUtilsDialog::waitForDialog( os, new PopupChooser( os, QStringList( ) << "Colors"
        << "Custom schemes" << "Create new color scheme" ) );
    GTUtilsDialog::waitForDialog( os, new NewColorSchemeCreator( os, colorSchemeName,
        NewColorSchemeCreator::amino ) );
    GTMouseDriver::click( os, Qt::RightButton );

    // 3. Create a new color scheme for the amino alphabet.
    GTUtilsDialog::waitForDialog( os, new PopupChooser( os, QStringList( ) << "Colors"
        << "Custom schemes" << colorSchemeName ) );
    GTMouseDriver::click( os, Qt::RightButton );
}

GUI_TEST_CLASS_DEFINITION(test_2091) {
    //1. Open "data/samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");

    //Expected state: document are opened in the project view; MSA Editor are shown with test_alignment.
    QTreeWidgetItem *msaDoc = GTUtilsProjectTreeView::findItem(os, "COI.aln");
    QWidget *msaView = GTUtilsMdi::activeWindow(os);
    CHECK(NULL != msaDoc, );
    CHECK(NULL != msaView, );

    //2. Select any sequence.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint( -5, 4));
    QStringList originalNames = GTUtilsMSAEditorSequenceArea::getNameList(os);


    //3. Call context menu on the name list area, select the {Edit -> Remove current sequence} menu item.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "Remove current sequence"));
    GTMouseDriver::click(os, Qt::RightButton);
    //Expected state: the sequence is removed.
    QStringList modifiedNames = GTUtilsMSAEditorSequenceArea::getNameList(os);

    CHECK_SET_ERR(originalNames.length()-modifiedNames.length() == 1, "The number of sequences remained unchanged.");
    CHECK_SET_ERR(!modifiedNames.contains("Montana_montana"), "Removed sequence is present in multiple alignment.");
}

GUI_TEST_CLASS_DEFINITION(test_2093_1) {
//    1. Run a scheme, e.g. "Call variants with SAMtools" from the NGS samples (or any other like read->write).
    QMenu* menu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    CHECK_SET_ERR(menu, "Main menu not found");
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");

    // Simple scheme: read file list.
    GTUtilsWorkflowDesigner::addAlgorithm(os, "File list");
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "File list"));
    GTMouseDriver::click(os);
    QString dirPath = dataDir + "samples/FASTA/";
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dirPath, "human_T1.fa");

    QToolBar* wdToolbar = GTToolbar::getToolbar(os, "mwtoolbar_activemdi");
    CHECK_SET_ERR(wdToolbar, "Toolbar not found");
    GTToolbar::getWidgetForActionName(os, wdToolbar, "Run workflow");

    GTGlobals::sleep();

//    2. Select "Load schema" button on the dashboard menu line.
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Discard));

    // We must click to the "Load schema" button on dashboard's toolbar.
    // The follow code is incorrect, it should be fixed.
    QWidget* loadSchemaButton = GTWidget::findWidget(os, "Load schema");
    CHECK_SET_ERR(loadSchemaButton, "Load schema button not found");
    GTWidget::click(os, loadSchemaButton);

    GTGlobals::sleep();

//    Expected result: the scheme with parameters is loaded.
    QWidget* wdElement = GTWidget::findWidget(os, "File list");
    CHECK_SET_ERR(wdElement, "Schema wasn't loaded");
}

GUI_TEST_CLASS_DEFINITION(test_2093_2) {
    // 1. Open WD.
    QMenu* menu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    CHECK_SET_ERR(menu, "Main menu not found");
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");

    // 2. Open any shema with the "Load workflow" button on the toolbar (not the "Open" button!)
    QString schemaPath = testDir + "_common_data/scenarios/workflow designer/222.uwl";
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, schemaPath));

    QToolBar* wdToolbar = GTToolbar::getToolbar(os, "mwtoolbar_activemdi");
    CHECK_SET_ERR(wdToolbar, "Toolbar not found");
    QWidget* loadButton = GTToolbar::getWidgetForActionName(os, wdToolbar, "Load workflow");
    CHECK_SET_ERR(loadButton, "Load button not found");
    GTWidget::click(os, loadButton);

    GTGlobals::sleep();

    // Expected result: the scheme with parameters is loaded.
    QWidget* wdElement = GTWidget::findWidget(os, "Read-sequence");
    CHECK_SET_ERR(wdElement, "Schema wasn't loaded");
}


GUI_TEST_CLASS_DEFINITION( test_2128 )
{
    // 1. Open document "ma.aln" and save it to string
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma.aln" );
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 11, 17 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString initialMsaContent = GTClipboard::text( os );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );

    // 2. Select a region in the sequence area
    QRect currentSelection( QPoint( 2, 5 ), QPoint( 8, 11 ) );
    GTUtilsMSAEditorSequenceArea::selectArea( os, currentSelection.topLeft( ),
        currentSelection.bottomRight( ) );

    // 3. Add gaps by pressing "Ctrl + Space" key
    const int totalShiftCount = 3;
    for ( int shiftCounter = 0; shiftCounter < totalShiftCount; ++shiftCounter ) {
        GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["space"],
            GTKeyboardDriver::key["ctrl"] );
        GTGlobals::sleep( 200 );
        currentSelection.moveRight( currentSelection.right( ) + 1 );
        GTGlobals::sleep( 200 );
        GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, currentSelection );
    }

    // 4. Remove gaps with "Ctrl + Backspace" key
    for ( int shiftCounter = 0; shiftCounter < totalShiftCount; ++shiftCounter ) {
        GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"],
            GTKeyboardDriver::key["ctrl"] );
        GTGlobals::sleep( 200 );
        currentSelection.moveLeft( currentSelection.left( ) - 1 );
        GTGlobals::sleep( 200 );
        GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, currentSelection );
    }

    // 5. Check that alignment content has returned to initial state
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 11, 17 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR( initialMsaContent == finalMsaContent, "MSA has unexpectedly changed" );
}

GUI_TEST_CLASS_DEFINITION( test_2128_1 )
{
    // 1. Open "data/samples/CLUSTAL/COI.aln".
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "COI.aln" );

    //2. Select the six first symbols in the first sequence and press Space. 
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 5, 0 ) );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["space"] );
    GTGlobals::sleep( 2000 );

    // Expected state: The first symbol T is on the 7 position.
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );
    GTGlobals::sleep(200);
    const QPoint initialSelectionPos( 6, 0 );
    GTUtilsMSAEditorSequenceArea::click( os, initialSelectionPos );
    GTGlobals::sleep( 2000 );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );
    GTGlobals::sleep(200);
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR(  "T" == finalMsaContent,
                    "Unexpected MSA content has occurred" );

    //3. Press the Ctrl + Space. 
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 6, 0 ), QPoint( 12, 0 ) );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["space"], GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );

    // Expected state: The first symbol T is on the 8 position.
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );
    GTGlobals::sleep(200);
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 7, 0 ), QPoint( 7, 0 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );
    GTGlobals::sleep(200);
    const QString finalMsaContent1 = GTClipboard::text( os );
    CHECK_SET_ERR(  "T" == finalMsaContent1,
                    "Unexpected MSA content has occurred" );
    
    //4. Press the Ctrl + Backspace. 
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 7, 0 ), QPoint( 13, 0 ) );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"], GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );

    // Expected state: The first symbol T is on the 8 position.
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );
    GTGlobals::sleep(200);
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 6, 0 ), QPoint( 6, 0 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );
    GTGlobals::sleep(200);
    const QString finalMsaContent2 = GTClipboard::text( os );
    CHECK_SET_ERR(  "T" == finalMsaContent2,
                    "Unexpected MSA content has occurred" );

    //5. Press the Backspace. 
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 6, 0 ), QPoint( 12, 0 ) );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"] );
    GTGlobals::sleep( 200 );

    // Expected state: The first symbol T is on the 1 position.
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );
    GTGlobals::sleep(200);
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 0, 0 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["esc"] );
    GTGlobals::sleep(200);
    const QString finalMsaContent3 = GTClipboard::text( os );
    CHECK_SET_ERR(  "T" == finalMsaContent3,
                    "Unexpected MSA content has occurred" );
}

GUI_TEST_CLASS_DEFINITION( test_2138 )
{
    //1. Open _common_data/scenarios/_regression/2138/1.fa
    //2. Press "Join sequences into alignment..." radio button
    //3. Press "OK" button

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Join));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/_regression/2138/1.fa");

    GTGlobals::sleep(1000);
    //Expected state: alignment has been opened and whole msa alphabet is amino
    bool isAmino = GTUtilsMSAEditorSequenceArea::hasAminoAlphabet(os);
    CHECK_SET_ERR(true == isAmino, "Aligment has wrong alphabet type");
}

GUI_TEST_CLASS_DEFINITION( test_2140 )
{
    //1. Use main menu {Tools->Dna assembly->convert UGENE assembly data base to SAM format}
    //2. Select any .fasta file instead of .ugenedb file
    //3. Click "convert"
    //Expected state: UGENE not crashes. Error message is written to log

    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new ConvertAssemblyToSAMDialogFiller(os, dataDir + "samples/FASTA/", "human_T1.fa"));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_TOOLS),
                          QStringList() << "DNA assembly" << "Convert UGENE Assembly data base to SAM format");

    CHECK_SET_ERR(l.hasError() == true, "There is no error message in log");
}

GUI_TEST_CLASS_DEFINITION( test_2150 ){
    // 1. Open Workflow Designer.
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
    // 2. Open the "Align sequences with MUSCLE" sample scheme.
    GTUtilsWorkflowDesigner::addSample(os, "Align sequences with MUSCLE");

    // 3. Set "data/samples/CLUSTALW/ty3.aln.gz" as the input file.
    GTMouseDriver::moveTo(os,GTUtilsWorkflowDesigner::getItemCenter(os,"Read alignment"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/CLUSTALW/", "ty3.aln.gz");

    // 4. Set some name to the result file.
    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"table"));
    CHECK_SET_ERR(table,"tableView not found");
    GTMouseDriver::moveTo(os,GTUtilsWorkflowDesigner::getItemCenter(os,"Write alignment"));
    GTMouseDriver::click(os);
    GTMouseDriver::moveTo(os,GTTableView::getCellPosition(os,table,1,1));
    GTMouseDriver::click(os);
    QString s = QFileInfo(testDir + "_common_data/scenarios/sandbox/").absoluteFilePath();
    GTKeyboardDriver::keySequence(os, s+"/2150_0001.sto");
    GTWidget::click(os,GTUtilsMdi::activeWindow(os));

    // 5. Run the workflow.
    GTWidget::click(os,GTAction::button(os,"Run workflow"));
    GTGlobals::sleep(1000);

    // 6. During the workflow execution open the "Tasks" panel in the bottom, find in the task tree the "MUSCLE alignment" subtask and cancel it.
    GTUtilsTaskTreeView::cancelTask(os, "MUSCLE alignment");
}

GUI_TEST_CLASS_DEFINITION( test_2156 ){
//    1. Open "data/samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/","COI.aln");
//    2. Select six symbols (45-50) of the first line.
    GTUtilsMSAEditorSequenceArea::selectArea(os,QPoint(44,0),QPoint(49,0));
//    3. Press BACKSPACE.
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["back"]);
//    Expected state: three gaps before the selected area are removed.
    GTWidget::click(os, GTUtilsMdi::activeWindow(os));
    GTUtilsMSAEditorSequenceArea::selectArea(os,QPoint(41,0),QPoint(44,0));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR(clipboardText == "CTAA", QString("Expected: CTAA, found: %1").arg(clipboardText) );

}

GUI_TEST_CLASS_DEFINITION( test_2157 )
{
//    1. Open file "https://ugene.unipro.ru/tracker/secure/attachment/12864/pdb1a07.ent.gz".
    GTFileDialog::openFile(os, testDir + "_common_data/pdb/", "pdb1a07.ent.gz");

//    Expected state: UGENE displays sequence and 3d structure
    GTUtilsMdi::findWindow(os, "pdb1a07.ent.gz");
}

GUI_TEST_CLASS_DEFINITION( test_2160 )
{
    // 1. Open document "ma.aln"
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma.aln" );

    // 2. Select whole msa
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 11, 17 ) );

    // 3. Shift msa
    const QPoint mouseDragPosition( 7, 7 );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseDragPosition );
    GTMouseDriver::press( os );
    GTGlobals::sleep( 200 );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseDragPosition + QPoint( 3, 0 ) );
    GTMouseDriver::release( os );
    GTGlobals::sleep( 200 );

    // 4. Press "Delete" key
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["delete"] );
    GTGlobals::sleep( 200 );

    // 6. Check that alignment has not changed
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( QPoint( 3, 0 ), QPoint( 14, 17 ) ) );
    CHECK_SET_ERR( 15 == GTUtilsMSAEditorSequenceArea::getLength( os ),
        "MSA length unexpectedly changed" );
    CHECK_SET_ERR( 18 == GTUtilsMSAEditorSequenceArea::getNameList( os ).size( ),
        "MSA row count unexpectedly changed" );
}

GUI_TEST_CLASS_DEFINITION( test_1924 )
{   
    //1. Open any sequence
    GTFileDialog::openFile( os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    //2. Use context menu on the sequence     {Edit sequence -> Insert subsequence}
    //3. Fill in "atcgtac" or any valid sequence containing lower case
    Runnable *filler = new InsertSequenceFiller(os,
        "atcgtac"
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_ACTIONS), QStringList() <<  ADV_MENU_EDIT << ACTION_EDIT_INSERT_SUBSEQUENCE, GTGlobals::UseKey);
    GTGlobals::sleep();

    //4. Click OK
    //Expected state: subsequence inserted
    //Bug state: Warning message is shown first	

    int sequenceLength = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(sequenceLength == 199957, "Sequence length is " + QString::number(sequenceLength) + ", expected 199957");

    QString sequenceBegin = GTUtilsSequenceView::getBeginOfSequenceAsString(os, 7);
    CHECK_SET_ERR(sequenceBegin == "ATCGTAC", "Sequence starts with <" + sequenceBegin + ">, expected ATCGTAC");

}

GUI_TEST_CLASS_DEFINITION(test_1986){
//1. Run UGENE
//2. Use main toolbar { File -> Search NCBI Genbank }
    GTUtilsDialog::waitForDialog(os, new NCBISearchDialogFiller(os, "human", false,5));

    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__SEARCH_GENBANK);
    GTGlobals::sleep();
//Expected state: the "NCBI Sequence Search" dialog has appeared

//3. Type "human" to the request string

//4. In the dialog's right bottom corner set "Result limit" to 5

//5. Press the "Search" button

//Expected state: only 5 results has appeared in the "Results" list

//6. Choose some result sequence

//7. Press the "Download" button

//Expected state: the "Fetch Data from Remote Database" dialog has appeared, it has the "Output format" combobox

//8. Select "fasta" output format

//9. Press "OK"

    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os, "XR_325540.fasta"));

//Expected state: the chosen sequence has been downloaded, saved in FASTA format and displayed in sequence view
}
GUI_TEST_CLASS_DEFINITION( test_2163 ) {
    // 1. Open "_common_data/fasta/AMINO.fa".
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "AMINO.fa");

    // 2. Open the DAS widget on the options panel
    GTWidget::click(os, GTWidget::findWidget(os, "OP_DAS"));
    GTGlobals::sleep(500);
    QWidget *dasPanel = GTWidget::findWidget(os, "DasOptionsPanelWidget");
    CHECK(NULL != dasPanel, );
   
    //3. Press "Fetch IDs".
    QWidget *searchIdsButton = GTWidget::findWidget(os, "searchIdsButton");
    GTWidget::click(os, searchIdsButton);
    GTGlobals::sleep(500);
   
   //4. Double click on the results table.
    TaskScheduler* scheduller = AppContext::getTaskScheduler();
    GTGlobals::sleep(5000);
    while(!scheduller->getTopLevelTasks().isEmpty()){
       GTGlobals::sleep();
    }
    QTableWidget *idList = qobject_cast<QTableWidget*>(GTWidget::findWidget(os, "idList"));
    GTWidget::click(os, idList);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    // 4. Select a result id in the table.
    QPoint p1 = idList->mapFromGlobal(GTTableView::getCellPosition(os, idList, 0, 3));
    GTWidget::click(os, idList, Qt::LeftButton, p1);

    int count1 = idList->model()->rowCount();
    QString value1 = idList->itemAt(p1)->text();

    //Expected: the table contains only the last results. There are no previous items and additional empty lines.
    QPoint p2 = idList->mapFromGlobal(GTTableView::getCellPosition(os, idList, 0, 3));
    GTWidget::click(os, idList, Qt::LeftButton, p2);

    int count2 = idList->model()->rowCount();
    QString value2 = idList->itemAt(p2)->text();

    CHECK_SET_ERR(count1 == count2, "There are empty rows!");
    CHECK_SET_ERR(value1 == value2, "Results differ!");

}

GUI_TEST_CLASS_DEFINITION( test_2164 ) {
    // 1. Open "_common_data/fasta/AMINO.fa".
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "AMINO.fa");

    // 2. Open the DAS widget on the options panel
    GTWidget::click(os, GTWidget::findWidget(os, "OP_DAS"));
    GTGlobals::sleep(500);
    QWidget *dasPanel = GTWidget::findWidget(os, "DasOptionsPanelWidget");
    CHECK(NULL != dasPanel, );
    
    //3. Press "Fetch IDs".
    QWidget *searchIdsButton = GTWidget::findWidget(os, "searchIdsButton");
    GTWidget::click(os, searchIdsButton);
    GTGlobals::sleep(500);
    
    //Expected: the "Fetch ID" button is disabled until the task is finished or canceled.
    CHECK_SET_ERR(!searchIdsButton->isEnabled(), "Fetch IDs is enabled!");
    TaskScheduler* scheduller = AppContext::getTaskScheduler();
    GTGlobals::sleep(5000);
    while(!scheduller->getTopLevelTasks().isEmpty()){
        GTGlobals::sleep();
    }
    CHECK_SET_ERR(searchIdsButton->isEnabled(), "Fetch IDs is disabled!");
    //4. Right click on the results table.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() <<"Fetch IDs"));
    GTWidget::click(os, GTWidget::findWidget(os, "idList"), Qt::RightButton);

    //Expected: the context menu action "Fetch IDs" is disabled until the task is finished or canceled.
    CHECK_SET_ERR(!searchIdsButton->isEnabled(), "Fetch IDs is enabled!");
    GTGlobals::sleep(500);
    TaskScheduler* scheduller1 = AppContext::getTaskScheduler();
    GTGlobals::sleep(5000);
    while(!scheduller1->getTopLevelTasks().isEmpty()){
       GTGlobals::sleep();
    }
    CHECK_SET_ERR(searchIdsButton->isEnabled(), "Fetch IDs is disabled!");
    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION( test_2165 ) {
    GTLogTracer l;
    //1. Open human_t1
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");

    //2. Copy the whole sequence to the clipboard
    GTWidget::click(os, GTWidget::findWidget(os, "annotated_DNA_scrollarea"));
    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os));
    GTWidget::click(os, GTWidget::findWidget(os, "select_range_action"));
    GTGlobals::sleep(500);

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    //3. Past the whole sequence to the find pattern field
    GTWidget::click(os, GTWidget::findWidget(os, "OP_FIND_PATTERN"));
    GTWidget::click(os, GTWidget::findWidget(os, "textPattern"));

    GTKeyboardDriver::keyClick(os, 'v', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);

    //4. Press the "Search" button
    GTWidget::click(os, GTWidget::findWidget(os, "btnSearch"));
    GTGlobals::sleep(500);

    //Expected: UGENE finds the sequence or shows a error message
    CHECK_SET_ERR(l.hasError() == true, "Error message expected in log");
}

GUI_TEST_CLASS_DEFINITION( test_2167 ) {
    // 1. Open "_common_data/fasta/AMINO.fa".
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "AMINO.fa");

    // 2. Open the DAS widget on the options panel
    GTWidget::click(os, GTWidget::findWidget(os, "OP_DAS"));
    GTGlobals::sleep(500);
    QWidget *dasPanel = GTWidget::findWidget(os, "DasOptionsPanelWidget");
    CHECK_SET_ERR(NULL != dasPanel, "DasOptionsPanelWidget is NULL!");

    // 3. Click "Fetch ids".
    QWidget *searchIdsButton = GTWidget::findWidget(os, "searchIdsButton");
    GTWidget::click(os, searchIdsButton);
    GTGlobals::sleep(500);

    TaskScheduler* scheduller = AppContext::getTaskScheduler();
    while (!scheduller->getTopLevelTasks().isEmpty()){
       GTGlobals::sleep();
    }

    // 4. Select a result id in the table.
    QTableWidget* resultsView = qobject_cast<QTableWidget*>(GTWidget::findWidget(os, "idList"));
    CHECK_SET_ERR(NULL != resultsView, "Results widget is NULL!");
    QPoint p1 = resultsView->mapFromGlobal(GTTableView::getCellPosition(os, resultsView, 0, 3));
    GTWidget::click(os, resultsView, Qt::LeftButton, p1);

    int count1 = resultsView->model()->rowCount();
    QString value1 = resultsView->itemAt(p1)->text();

    // 5. Click "Fetch ids" once again.
    GTWidget::click(os, searchIdsButton);
    GTGlobals::sleep(500);

    while (!scheduller->getTopLevelTasks().isEmpty()){
       GTGlobals::sleep();
    }

    // Expected: the table contains only the last results. There are no previous items and additional empty lines.
    QPoint p2 = resultsView->mapFromGlobal(GTTableView::getCellPosition(os, resultsView, 0, 3));
    GTWidget::click(os, resultsView, Qt::LeftButton, p2);

    int count2 = resultsView->model()->rowCount();
    QString value2 = resultsView->itemAt(p2)->text();

    CHECK_SET_ERR(count1 == count2, "There are empty rows!");
    CHECK_SET_ERR(value1 == value2, "Results differ!");
}

GUI_TEST_CLASS_DEFINITION( test_2169 ) {
    // 1. Open "data/PDB/1CF7.pdb".
    GTFileDialog::openFile(os, dataDir + "samples/PDB", "1CF7.PDB");

    // 2. Open the DAS widget on the options panel
    GTWidget::click(os, GTWidget::findWidget(os, "OP_DAS"));
    GTGlobals::sleep(500);

    QWidget *dasPanel = GTWidget::findWidget(os, "DasOptionsPanelWidget");
    CHECK(NULL != dasPanel, );
    QWidget *regionSelector = GTWidget::findWidget(os, "range_selector");
    CHECK(NULL != regionSelector, );

    GTWidget::click(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));
    GTGlobals::sleep(500);
    CHECK_SET_ERR(dasPanel->isEnabled(), "DAS panel is disabled for amino acid sequence");
    CHECK_SET_ERR(regionSelector->isEnabled(), "DAS panel child is disabled for amino acid sequence");

    GTWidget::click(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_2"));
    GTGlobals::sleep(500);
    CHECK_SET_ERR(!dasPanel->isEnabled(), "DAS panel is enabled for nucleic acid sequence");
    CHECK_SET_ERR(!regionSelector->isEnabled(), "DAS panel child is enabled for nucleic acid sequence");
}

GUI_TEST_CLASS_DEFINITION( test_2171 )
{
    //1. Open file _common_data/fasta/AMINO.fa
    GTFileDialog::openFile( os, testDir + "_common_data/fasta", "AMINO.fa");
    //2. Open DAS option panel
    GTWidget::click( os, GTWidget::findWidget( os, "OP_DAS" ) );
    //3. Click fetch IDs
    GTWidget::click( os, GTWidget::findWidget( os, "searchIdsButton" ) );
    //4. Cancel task  "Fetch IDs"
    GTUtilsTaskTreeView::cancelTask(os, "Get an ID for the sequence");
    //Expected state: task canceled
    QTreeWidget *taskTreeWidget = GTUtilsTaskTreeView::getTreeWidget(os);
    CHECK_SET_ERR(taskTreeWidget->invisibleRootItem()->childCount() == 0, "Task is not canceled");
}

GUI_TEST_CLASS_DEFINITION( test_2172 ) {
    GTLogTracer logTracer;

    // 1. Open file _common_data/fasta/amino_multy.fa
    GTUtilsDialog::waitForDialog( os, new SequenceReadingModeSelectorDialogFiller( os ) );
    const QString filePath = testDir + "_common_data/fasta/amino_multy.fa";
    GTUtilsProject::openFiles(os, QList<QUrl>( ) << filePath );
    GTGlobals::sleep( );

    // 2. Open DAS option panel
    GTWidget::click( os, GTWidget::findWidget( os, "OP_DAS" ) );
    GTGlobals::sleep( 200 );

    // 3. Check that warning message exists
    const QLabel *warnignLabel = qobject_cast<QLabel*>( GTWidget::findWidget( os, "hintLabel" ) );
    CHECK_SET_ERR( NULL != warnignLabel, "Cannot find the hint label" );

    QString warningText = warnignLabel->text( );
    CHECK_SET_ERR( warningText.startsWith( "Warning" ), "No warning message found" );

    // 4. Press the "Fetch IDs" button
    QPushButton *fetchButton = qobject_cast<QPushButton *>( GTWidget::findWidget( os,
        "searchIdsButton" ) );
    CHECK_SET_ERR( NULL != fetchButton, "Cannot find the \"Fetch IDs\" button" );
    fetchButton->click( );
    GTGlobals::sleep( 200 );

    // 5. Check error message in log
    CHECK_SET_ERR( logTracer.hasError( ), "Error message expected in log" );
}

GUI_TEST_CLASS_DEFINITION( test_2174 ) {
    //1. Open file _common_data/fasta/AMINO.fa
    GTFileDialog::openFile( os, testDir + "_common_data/fasta/", "AMINO.fa");
    GTGlobals::sleep();

    //2. Open DAS option panel
    GTWidget::click(os,GTWidget::findWidget(os, "OP_DAS"));
    GTGlobals::sleep(500);

    //3. Select region 1-2
    GTRegionSelector::RegionSelectorSettings regionSelectorSettings(1, 2);
    RegionSelector *regionSelector = qobject_cast<RegionSelector*>(GTWidget::findWidget(os, "range_selector"));
    GTRegionSelector::setRegion(os, regionSelector, regionSelectorSettings);

    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["enter"] );
    GTGlobals::sleep( 200 );

    //Expected state: warning message appeares
    
    QLabel *warnignLabel = qobject_cast<QLabel*>(GTWidget::findWidget(os, "hintLabel"));
    CHECK_SET_ERR(warnignLabel != NULL, "Cannot find the hint label");

    QString warningText = warnignLabel->text();
    CHECK_SET_ERR( warningText.startsWith("Warning"), "No warning message found");
}

GUI_TEST_CLASS_DEFINITION( test_2186 ) {
//    1. Open file _common_data/fasta/amino_multy.fa
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Separate));
    GTFileDialog::openFile( os, testDir + "_common_data/fasta/", "amino_multy.fa");
    GTGlobals::sleep(500);
//    2. Open the DAS widget on the options panel
    GTWidget::click(os,GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));
    GTWidget::click(os,GTWidget::findWidget(os, "OP_DAS"));
    GTGlobals::sleep(500);
//    3. Select first sequence, select region 1-100
    GTRegionSelector::RegionSelectorSettings regionSelectorSettings(1, 100);
    RegionSelector *regionSelector = qobject_cast<RegionSelector*>(GTWidget::findWidget(os, "range_selector"));
    GTRegionSelector::setRegion(os, regionSelector, regionSelectorSettings);

    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["enter"] );
    GTGlobals::sleep( 200 );
//    4. Click fetch IDs
    GTWidget::click( os, GTWidget::findWidget( os, "searchIdsButton" ) );
    TaskScheduler* scheduller = AppContext::getTaskScheduler();
    while(!scheduller->getTopLevelTasks().isEmpty()){
        GTGlobals::sleep();
    }
//    Expected state:2 IDs are found
    GTGlobals::sleep();
    QTableWidget* idList = qobject_cast<QTableWidget*>(GTWidget::findWidget(os, "idList"));
    CHECK_SET_ERR(idList,"idList widget not found");
    CHECK_SET_ERR(idList->rowCount()==2,QString("idList contains %1 rows, expected 2").arg(idList->rowCount()));

//    5. select second sequence
    GTWidget::click(os,GTWidget::findWidget(os,"ADV_single_sequence_widget_1"));
    GTGlobals::sleep(500);
//    Expected state: IDs table on option panel cleared
     CHECK_SET_ERR(idList->rowCount()==0,QString("idList contains %1 rows, expected 0").arg(idList->rowCount()));


}

GUI_TEST_CLASS_DEFINITION( test_2188 ) {
    class Helper {
    public:
        Helper(const QString &dataDir, const QString &testDir, U2OpStatus &os) {
            dir = testDir + "_common_data/scenarios/sandbox/";
            fileName = "regression_test_2188.fa";
            url = dir + fileName;
            bool copied = QFile::copy(dataDir + "samples/FASTA/human_T1.fa", url);
            if (!copied) {
                os.setError("Can not copy the file");
                url = "";
            }
        }

        QString url;
        QString dir;
        QString fileName;
    };

    // 1. Open the file "data/samples/FASTA/human_T1.fa"
    Helper helper(dataDir, testDir, os);
    CHECK_OP(os, );
    GTFileDialog::openFile( os, helper.dir, helper.fileName );
    GTGlobals::sleep(500);

    // 2. At the same time open the file with a text editor
    // 3. Change something and save
    // Expected state: Dialog suggesting file to reload has appeared in UGENE
    // 4. Press "Yes to All" button
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
    QFile file(helper.url);
    bool opened = file.open(QIODevice::Append);
    CHECK_SET_ERR(opened, "Can not open the file for writing");
    qint64 writed = file.write("AAAAAAAAAA");
    CHECK_SET_ERR(10 == writed, "Can not write to the file");
    file.close();
    GTGlobals::sleep(6000);

    // Expected state: All the sequences were reloaded and displayed correctly in sequence view
    int length = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_OP(os, );
    CHECK_SET_ERR(199960 == length, "The file lenght is wrong");
}

GUI_TEST_CLASS_DEFINITION( test_2187 ) {
    //1. Open "data/samples/FASTA/human_T1.fa"
    //d:\src\ugene\trunk\test\_common_data\scenarios\_regression\2187\seq.fa

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/_regression/2187/", "seq.fa");
    GTUtilsDocument::checkDocument(os, "seq.fa");

    //2. Open {Actions -> Analyze -> Find tandems...}
    //3. Click ok

    Runnable * tDialog = new FindTandemsDialogFiller(os, testDir + "_common_data/scenarios/sandbox/result_2187.gb");
    GTUtilsDialog::waitForDialog(os, tDialog);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE
        << "find_tandems_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();

    QTreeWidget *treeWidget = GTUtilsAnnotationsTreeView::getTreeWidget(os);
    CHECK_SET_ERR(treeWidget != NULL, "Tree widget is NULL");

    QTreeWidgetItem *annotationsRoot = GTUtilsAnnotationsTreeView::findItem(os, "repeat_unit");
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, annotationsRoot->child(0)));
    GTMouseDriver::doubleClick(os);

    Runnable *filler = new EditAnnotationChecker(os, "repeat_unit", "join(251..251,252..252,253..253,254..254,255..255,256..256,257..257,258..258,259..259)");
    GTUtilsDialog::waitForDialog(os, filler);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTMouseDriver::click(os);
}

GUI_TEST_CLASS_DEFINITION( test_2202 )
{
//    1. Open Workflow Designer first time (e.g. remove UGENE.ini before it).
//    Expected: the dialog appears.

//    2. Write the path to the directory which does not exist.

//    3. Click OK.
//    Expected: the directory is created, there are no error messages about write access.

    QDir workflowOutputDir(testDir + "_common_data/scenarios/sandbox/regression_test_2202/1/2/3/4/");
    CHECK_SET_ERR(!workflowOutputDir.exists(), "Dir already exists");

    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os, true, workflowOutputDir.absolutePath()));
    QMenu* menu = GTMenu::showMainMenu(os, MWMENU_TOOLS);

    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");

    GTGlobals::sleep();
    CHECK_SET_ERR(workflowOutputDir.exists(), "Dir wasn't created");
}

GUI_TEST_CLASS_DEFINITION( test_2224 )
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
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 4 ), QPoint( 7, 12 ) );

    // 4. Shift the region in one base
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 6, 7 ), QPoint( 7, 7 ) );

    // 5. Click in the leftmost column of the selected region
    GTUtilsMSAEditorSequenceArea::click( os, QPoint( 1, 7 ) );

    // 7. Check the result state
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 11, 17 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString undoneContent = GTClipboard::text( os );
    CHECK_SET_ERR( undoneContent == initialContent,
        "Scribbling works wrong. Found text is:\n" + undoneContent );
}

GUI_TEST_CLASS_DEFINITION( test_2224_flow )
{
    // 1. Open file {_common_data/scenarios/msa/ma.aln"
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma.aln" );

    // copy the initial MSA
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(11, 17));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString initialMSA = GTClipboard::text(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);

    // 2. Select some region starting from the 1st column and ending in the 8th one
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 2), QPoint(7, 6));

    // 3. Drag it to the right in one column
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(4, 4), QPoint(5, 4));

    // 4. Click left mouse button in the leftmost side of the selected region
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(1, 4));
    GTMouseDriver::click(os);

    // copy the result MSA
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(11, 17));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    QString resultMSA = GTClipboard::text(os);

    CHECK_SET_ERR(initialMSA == resultMSA, "The result MSA is wrong");
}

GUI_TEST_CLASS_DEFINITION( test_2225_1 ){
    Runnable *filler = new NCBISearchDialogFiller(os, "rat", true);

    GTUtilsDialog::waitForDialog(os, filler);

    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__SEARCH_GENBANK, GTGlobals::UseKey);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION( test_2225_2 ){
    Runnable *filler = new NCBISearchDialogFiller(os, "rat", true);

    GTUtilsDialog::waitForDialog(os, filler);

    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__SEARCH_GENBANK, GTGlobals::UseKey);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION( test_2267_1 ){
//     1. Open human_T1.fa
// 
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");

//     2. Select random subsequence, press Ctrl+N, press Enter
// 
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "DDD", "D", "10..16"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();
//     3. Press Insert, press '1' key until there is no new symbols in lineedit
// 
    Runnable *filler = new EditQualifierFiller(os, "111111111111111111111111111111111111111111111111111111111111111111111111111111111", "val", GTGlobals::UseMouse, true);
    GTUtilsDialog::waitForDialog(os, filler);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "D"));
    GTMouseDriver::click(os, Qt::RightButton);
//     4. Press Enter
//     Expected state: Edit qualifier window closes
}

GUI_TEST_CLASS_DEFINITION( test_2267_2 ){
//     1. Open human_T1.fa
// 
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
//     2. Select random subsequence, press Ctrl+N, press Enter
// 
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "DDD", "D", "10..16"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();
//     3. Press Insert, set the name of qualifier: '))()((_'
// 
//     4. Press Enter
//     Expected state: Error message appears once
    Runnable *filler = new EditQualifierFiller(os, "))()((_", "val", GTGlobals::UseMouse, true, true);
    GTUtilsDialog::waitForDialog(os, filler);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "D"));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION( test_2268 ) {
    class PermissionsSetter {
    public:
        PermissionsSetter() {}

        ~PermissionsSetter() {
            foreach (const QString& path, previousState.keys()) {
                QFile file(path);
                QFile::Permissions p = file.permissions();

                p = previousState.value(path, p);
                file.setPermissions(p);
            }
        }

        bool setPermissions(const QString& path, QFile::Permissions perm, bool recursive = true) {
            if (recursive) {
                return setRecursive(path, perm);
            } else {
                return setOnce(path, perm);
            }
        }

    private:
        bool setRecursive(const QString& path, QFile::Permissions perm) {
            QFileInfo fileInfo(path);
            CHECK(fileInfo.exists(), false);
            CHECK(!fileInfo.isSymLink(), false);

            if (fileInfo.isDir()) {
                QDir dir(path);
                foreach (const QString& entryPath, dir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
                    bool res = setRecursive(path + "/" + entryPath, perm);
                    CHECK(res, res);
                }
            }

            bool res = setOnce(path, perm);

            return res;
        }

        bool setOnce(const QString& path, QFile::Permissions perm) {
            QFileInfo fileInfo(path);
            CHECK(fileInfo.exists(), false);
            CHECK(!fileInfo.isSymLink(), false);

            QFile file(path);
            QFile::Permissions p = file.permissions();
            previousState.insert(path, p);

            p &= perm;
            return file.setPermissions(p);
        }

        QMap<QString, QFile::Permissions> previousState;
    };

//    1. Forbid write access to the t-coffee directory (chmod 555 %t-coffee-dir%).
    // Permissions will be returned to the original state, if UGENE won't crash.
    ExternalToolRegistry* etRegistry = AppContext::getExternalToolRegistry();
    CHECK_SET_ERR(etRegistry, "External tool registry is NULL");
    ExternalTool* tCoffee = etRegistry->getByName("T-Coffee");
    CHECK_SET_ERR(tCoffee, "T-coffee tool is NULL");
    QFileInfo toolPath(tCoffee->getPath());
    CHECK_SET_ERR(toolPath.exists(), "T-coffee tool is not set");

    QDir toolDir = toolPath.dir();
    toolDir.cdUp();

    PermissionsSetter permSetter;
    QFile::Permissions p = QFile::WriteOwner |
                           QFile::WriteUser |
                           QFile::WriteGroup |
                           QFile::WriteOther;
    bool res = permSetter.setPermissions(toolDir.path(), ~p);
    CHECK_SET_ERR(res, "Can't set permissions");

//    2. Open "sample/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "/samples/CLUSTALW/", "COI.aln");

//    3. Right click on the MSA -> Align -> Align with T-Coffee.
//    4. Click the "Align" button.
    GTLogTracer lt;
    GTUtilsDialog::waitForDialog(os, new TCoffeeDailogFiller(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with T-Coffee", GTGlobals::UseMouse));
    QMenu* contextMenu = GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    CHECK_SET_ERR(contextMenu, "Context menu not found");

//    Expected: the t-coffee task started and finished well.
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    CHECK_SET_ERR(scheduler, "Task scheduler is NULL");
    GTGlobals::sleep(5000);
    while(!scheduler->getTopLevelTasks().isEmpty()){
       GTGlobals::sleep();
    }

    GTUtilsLog::check(os, lt);
}

GUI_TEST_CLASS_DEFINITION( test_2269 ){
    //1. Use main menu: {tools->Align short reeds}
    //2. Select Bowtie2 as alignment method
    //3. Try to set incorrect value in "Seed lingth" spinbox(Correct boundaries are: >3, <32)
    AlignShortReadsFiller::Bowtie2Parameters parameters(testDir + "_common_data/scenarios/_regression/1093/",
                                                        "refrence.fa",
                                                        testDir + "_common_data/scenarios/_regression/1093/",
                                                        "read.fa");
    parameters.seedLengthCheckBox = true;
    parameters.seedLength = 33;

    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, &parameters));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Align to reference" << "Align short reads");

    CHECK_SET_ERR( !os.hasError(), "Uncorrect value is available");
}

GUI_TEST_CLASS_DEFINITION( test_2281 ){
    //GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    // 1. Open WD sample "Align Sequences with MUSCLE
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
   // GTUtilsWorkflowDesigner::addSample(os, "Align sequences with MUSCLE");
    GTGlobals::sleep(500);
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
//    GT_CHECK_RESULT(sceneView, "sceneView not found", NULL);
    QList<QGraphicsItem *> items = sceneView->items();
    QString s;
    foreach(QGraphicsItem* it, items) {
        QGraphicsObject *itObj = it->toGraphicsObject();
        QGraphicsTextItem* textItemO = qobject_cast<QGraphicsTextItem*>(itObj);
        if (textItemO) {
            QString text = textItemO->toPlainText();
            s.append(text + "  ");
        }
    }
    /*QList<QWidget*> list = AppContext::getMainWindow()->getQMainWindow()->findChildren<QWidget*>();

    foreach(QWidget* w, list){
        s.append(w->metaObject()->className()).append("  " + w->objectName()).append('\n');


        }*/
    CHECK_SET_ERR(false, s)
    GTGlobals::sleep(1000);
}

GUI_TEST_CLASS_DEFINITION( test_2309 ) {
    // 1. Open file "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");

    // 2. Build tree for the alignment
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/2309.nwk", 0, 0, true));
    GTUtilsDialog::waitForDialog(os,new LicenseAgreemntDialogFiller(os));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os, tree);
    GTGlobals::sleep(500);

    QStringList initialNames = GTUtilsMSAEditorSequenceArea::getNameList(os);
    QAbstractButton *refresh= GTAction::button(os,"Refresh tree");
    CHECK(NULL != refresh, );
    GTWidget::click(os, refresh);
    GTGlobals::sleep(500);

    QStringList newNames = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(newNames == initialNames, "Wrong sequences order");
}

GUI_TEST_CLASS_DEFINITION( test_2318 ) {
    class FirstItemPopupChooser : public PopupChooser {
    public:
        FirstItemPopupChooser(U2OpStatus& os) :
            PopupChooser(os, QStringList()) {}

        virtual void run() {
            GTGlobals::sleep(1000);
            GTMouseDriver::release(os);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
        }
    };

    class PlusClicker : public FindAnnotationCollocationsDialogFiller {
    public:
        PlusClicker(U2OpStatus& os, const QString& _annName) :
            FindAnnotationCollocationsDialogFiller(os),
            annName(_annName) {}

        virtual void run() {
            QToolButton* plusButton = getPlusButton();
            CHECK_SET_ERR(plusButton, "First plus toolbutton is NULL");
            firstSize = plusButton->size();

            GTUtilsDialog::waitForDialog(os, new FirstItemPopupChooser(os));
            GTWidget::click(os, plusButton);

            GTGlobals::sleep();
            plusButton = getPlusButton();
            CHECK_SET_ERR(plusButton, "Second plus toolbutton is NULL");
            secondSize = plusButton->size();

            bool result = test();
            CHECK_SET_ERR(result, "Incorrect result");

            FindAnnotationCollocationsDialogFiller::run();
        }

    private:
        bool test() const {
            return firstSize == secondSize;
        }

        QString annName;
        QSize firstSize;
        QSize secondSize;
    };

//    1) Open {data/samples/murine.gb}
    GTFileDialog::openFile(os, dataDir + "/samples/Genbank/", "murine.gb");

//    2) Use context menu: {Analyze -> Find annotated regions}
//    3) Click plus button, select any annotation
//    Expected state: plus button changes place, its size wasn't change
    QToolBar* toolbar = GTToolbar::getToolbar(os, "mwtoolbar_activemdi");
    CHECK_SET_ERR(toolbar, "Toolbar is NULL");
    QWidget* farButton = GTToolbar::getWidgetForActionTooltip(os, toolbar, "Find annotated regions...");
    CHECK_SET_ERR(farButton, "Find annotated region button is NULL");

    // Expected state is checked in PlusClicker
    GTUtilsDialog::waitForDialog(os, new PlusClicker(os, "CDS"));
    GTWidget::click(os, farButton);
}

GUI_TEST_CLASS_DEFINITION( test_2375 ) {
//    1. Open {_common_data/sam/broken_invalid_cigar.sam}
//    Expected state: import dialog appears.

//    2. Fill the import dialog with valid data. Begin the importing.
//    Expected state: importing fails, UGENE doesn't crash.
    QString destUrl = testDir + "_common_data/scenarios/sandbox/test_2375.ugenedb";
    GTLogTracer logtracer;
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, destUrl));
    GTFileDialog::openFile(os, testDir + "_common_data/sam/", "broken_invalid_cigar.sam");

    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    CHECK_SET_ERR(scheduler, "Task scheduler is NULL");
    GTGlobals::sleep(5000);
    while (!scheduler->getTopLevelTasks().isEmpty()) {
        GTGlobals::sleep();
    }

    CHECK_SET_ERR(logtracer.hasError(), "There wasn't errors in the log");
}

GUI_TEST_CLASS_DEFINITION( test_2377 ) {
    GTLogTracer l;

    GTUtilsDialog::waitForDialog( os, new StartupDialogFiller( os, true ) );

    QMenu *menu=GTMenu::showMainMenu( os, MWMENU_TOOLS );
    GTMenu::clickMenuItem( os, menu, QStringList( ) << "Workflow Designer" );

    const QString assemblyReaderName = "Read Assembly";
    const QString assemblyWriterName = "Write Assembly";

    GTUtilsWorkflowDesigner::addAlgorithm( os, assemblyReaderName );
    GTUtilsWorkflowDesigner::addAlgorithm( os, assemblyWriterName );

    WorkflowProcessItem *assemblyReader = GTUtilsWorkflowDesigner::getWorker( os,
        assemblyReaderName );
    WorkflowProcessItem *assemblyWriter = GTUtilsWorkflowDesigner::getWorker( os,
        assemblyWriterName );

    GTUtilsWorkflowDesigner::connect( os, assemblyReader, assemblyWriter );

    GTMouseDriver::moveTo( os, GTUtilsWorkflowDesigner::getItemCenter( os, assemblyReaderName ) );
    GTMouseDriver::click( os );
    GTUtilsWorkflowDesigner::setDatasetInputFile( os, testDir + "_common_data/sam",
        "broken_without_reads.sam" );

    GTMouseDriver::moveTo( os, GTUtilsWorkflowDesigner::getItemCenter( os, assemblyWriterName ) );
    GTMouseDriver::click( os );
    GTUtilsWorkflowDesigner::setParameter( os, "Output file", "test_out",
        GTUtilsWorkflowDesigner::textValue );

    GTWidget::click( os, GTAction::button( os, "Run workflow" ) );

    CHECK_SET_ERR( l.hasError( ), "Error message expected!" );
}

} // GUITest_regression_scenarios namespace

} // U2 namespace

