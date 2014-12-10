/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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
#include <U2Core/U2ObjectDbi.h>

#include "GTTestsRegressionScenarios.h"

#include "api/GTRadioButton.h"
#include "api/GTAction.h"
#include "api/GTCheckBox.h"
#include "api/GTClipboard.h"
#include "api/GTComboBox.h"
#include "api/GTFile.h"
#include "api/GTFileDialog.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTLineEdit.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTSequenceReadingModeDialog.h"
#include "api/GTSequenceReadingModeDialogUtils.h"
#include "api/GTSpinBox.h"
#include "api/GTTabWidget.h"
#include "api/GTTableView.h"
#include "api/GTToolbar.h"
#include "api/GTTreeWidget.h"
#include "api/GTWidget.h"

#include "GTDatabaseConfig.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsAssemblyBrowser.h"
#include "GTUtilsBookmarksTreeView.h"
#include "GTUtilsCircularView.h"
#include "GTUtilsDialog.h"
#include "GTUtilsEscClicker.h"
#include "GTUtilsExternalTools.h"
#include "GTUtilsLog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditor.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsNotifications.h"
#include "GTUtilsOptionPanelMSA.h"
#include "GTUtilsOptionsPanel.h"
#include "GTUtilsOptionPanelSequenceView.h"
#include "GTUtilsPhyTree.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsSharedDatabaseDocument.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsWorkflowDesigner.h"

#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/AlignShortReadsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/AppSettingsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/BuildIndexDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ConvertAssemblyToSAMDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/ugene/corelibs/U2Gui/CreateObjectRelationDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/DownloadRemoteFileDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditAnnotationDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditConnectionDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditQualifierDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditSequenceDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ExportDocumentDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ExportImageDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/FindQualifierDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/FindRepeatsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/FindTandemsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ImportBAMFileDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/PositionSelectorFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ProjectTreeItemSelectorDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/RangeSelectionDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/RemovePartFromSequenceDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ReplaceSubsequenceDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/SharedConnectionsDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_assembly/ExportReadsDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/BuildTreeDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/DeleteGapsDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/DistanceMatrixDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ExportHighlightedDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/ExtractSelectedAsMSADialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/LicenseAgreemntDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/utils_smith_waterman/SmithWatermanDialogBaseFiller.h"
#include "runnables/ugene/plugins/annotator/FindAnnotationCollocationsDialogFiller.h"
#include "runnables/ugene/plugins/biostruct3d_view/StructuralAlignmentDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportAnnotationsDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportMSA2MSADialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportMSA2SequencesDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSequences2MSADialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSequencesDialogFiller.h"
#include "runnables/ugene/plugins/dotplot/BuildDotPlotDialogFiller.h"
#include "runnables/ugene/plugins/dotplot/DotPlotDialogFiller.h"
#include "runnables/ugene/plugins/enzymes/ConstructMoleculeDialogFiller.h"
#include "runnables/ugene/plugins/enzymes/DigestSequenceDialogFiller.h"
#include "runnables/ugene/plugins/enzymes/FindEnzymesDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/BlastAllSupportDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/ClustalOSupportRunDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/FormatDBDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/RemoteBLASTDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/TCoffeeDailogFiller.h"
#include "runnables/ugene/plugins/weight_matrix/PwmBuildDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/AliasesDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/CreateElementWithScriptDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/StartupDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/WizardFiller.h"
#include "runnables/ugene/plugins/workflow_designer/WorkflowMetadialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/hmm3/UHMM3PhmmerDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/hmm3/UHMM3SearchDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/kalign/KalignDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/umuscle/MuscleDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/clustalw/ClustalWDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/MAFFT/MAFFTSupportRunDialogFiller.h"
#include "runnables/ugene/ugeneui/ConvertAceToSqliteDialogFiller.h"
#include "runnables/ugene/ugeneui/CreateNewProjectWidgetFiller.h"
#include "runnables/ugene/ugeneui/DocumentFormatSelectorDialogFiller.h"
#include "runnables/ugene/ugeneui/DocumentProviderSelectorDialogFiller.h"
#include "runnables/ugene/ugeneui/NCBISearchDialogFiller.h"
#include "runnables/ugene/ugeneui/SaveProjectDialogFiller.h"
#include "runnables/ugene/ugeneui/SelectDocumentFormatDialogFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"

#include <U2Core/AppContext.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2View/ADVConstants.h>
#include <U2View/ADVSingleSequenceWidget.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/AnnotationsTreeView.h>
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorNameList.h>

#include <QProgressBar>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>
#include <QtGui/QTableWidget>
#else
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#endif

namespace U2 {

namespace GUITest_regression_scenarios {

GUI_TEST_CLASS_DEFINITION(test_0734) {
    //1. Open "_common_data/fasta/test.TXT".
    GTUtilsProject::openFiles(os, testDir + "_common_data/fasta/test.TXT");

    //2. Open "_common_data/clustal/test_alignment.aln".
    GTUtilsProject::openFiles(os, testDir + "_common_data/clustal/test_alignment.aln");

    //Expected state: two documents are opened in the project view; MSA Editor are shown with test_alignment.
    GTUtilsProjectTreeView::findIndex(os, "test.TXT");//checks are inside
    GTUtilsProjectTreeView::findIndex(os, "test_alignment.aln");//checks are inside
    QWidget *msaView = GTUtilsMdi::activeWindow(os);
    CHECK(NULL != msaView, );

    //3. Drag'n'drop "Sequence4" object of "test.TXT" document from the project tree to the MSA Editor.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_LOAD << "Sequence from current project"));
    GTUtilsDialog::waitForDialog(os, new ProjectTreeItemSelectorDialogFiller(os, "test.TXT", "Sequence4"));
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
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    //GTGlobals::sleep(20000);

    // Expected state: 837 orfs are found.
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "orf  (0, 837)");
    CHECK(NULL != item, );
    CHECK_SET_ERR(item->childCount() == 837, QString("ORFs count mismatch. Expected: %1. Actual: %2").arg(837).arg(item->childCount()));
}

GUI_TEST_CLASS_DEFINITION(test_0986) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(3000);

    SmithWatermanDialogFiller *filler = new SmithWatermanDialogFiller(os);
    filler->button = SmithWatermanDialogFiller::Cancel;
    GTUtilsDialog::waitForDialog(os, filler);

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
    GTUtilsMdi::click(os, GTGlobals::Close);
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
    GTUtilsMdi::click(os, GTGlobals::Close);
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
    GTUtilsMdi::click(os, GTGlobals::Close);
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
    GTUtilsMdi::click(os, GTGlobals::Close);
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
    GTUtilsMdi::click(os, GTGlobals::Close);
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

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Align to reference" << "Align short reads");

    GTUtilsTaskTreeView::waitTaskFinished(os);
//The short reads can't be mapped to the reference sequence!
}

GUI_TEST_CLASS_DEFINITION(test_1107){//commit GUIInitionalChecks
//1) Open an MSA file (e.g. _common_data\scenarios\msa\ma2_gapped.aln)

    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/msa/", "ma2_gapped.aln");
//2) Menu File->Close Project
//3) Press No in the Save current project dialog
    GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
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
#ifdef Q_OS_MAC
    //hack for mac
    MainWindow *mw = AppContext::getMainWindow();
    CHECK_SET_ERR(mw != NULL, "MainWindow is NULL");
    QMainWindow *mainWindow = mw->getQMainWindow();
    CHECK_SET_ERR(mainWindow != NULL, "QMainWindow is NULL");
    QWidget* w = qobject_cast<QWidget*>(mainWindow);
    GTWidget::click(os, w, Qt::LeftButton, QPoint(5,5));
    GTGlobals::sleep(500);
#endif
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
    GTMenu::clickMenuItemByName(os, menu, QStringList() << ACTION__ABOUT);
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
    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 100, 200));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Select" << "Sequence region"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"));

//4) Select "Selected region" in "Region" combobox of "Search in" area.
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search in"));
    QComboBox *regBox = (QComboBox *)GTWidget::findWidget(os, "boxRegion");
    GTComboBox::setCurrentIndex(os, regBox, 2);

//5) Ensure that two lineedits became visible and contain correct region
    QLineEdit *start = (QLineEdit *)GTWidget::findWidget(os, "editStart");
    CHECK_SET_ERR(start->isVisible(), "editStart line is not visiable");
    CHECK_SET_ERR(start->text() == "100", "Wrong startValue. Current value is: " + start->text());

    QLineEdit *end = (QLineEdit *)GTWidget::findWidget(os, "editEnd");
    CHECK_SET_ERR(end->isVisible(), "editEnd line is not visiable");
    CHECK_SET_ERR(end->text() == "200", "Wrong endValue. Current value is: " + end->text());
}

GUI_TEST_CLASS_DEFINITION(test_1189_1){
//1) Open samples/FASTA/human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

//2) Press Ctrl+F
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);

//3) Select any region of the sequence
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"));
    QPoint p;
    p=GTMouseDriver::getMousePosition();
    p.setX(p.x() - 100);
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::press(os);
    p.setX(p.x() + 200);
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::release(os);

//4) Select "Selected region" in "Region" combobox of "Search in" area.
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search in"));
    QComboBox *regBox =(QComboBox *)GTWidget::findWidget(os, "boxRegion");
    GTComboBox::setCurrentIndex(os, regBox, 2);

//5) Ensure that two lineedits became visible and contain correct region
    QLineEdit *start = (QLineEdit *)GTWidget::findWidget(os, "editStart");
    CHECK_SET_ERR(start->isVisible(), "editStart line is not visiable");

    QLineEdit *end = (QLineEdit *)GTWidget::findWidget(os,"editEnd");
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

    GTMenu::clickMenuItemByName(os, menu, QStringList() << "Align to reference" << "Align short reads");

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
    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os,"1..199950"));
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

    GTWidget::click(os, GTWidget::findWidget(os, "getAnnotationsPushButton"));
    GTGlobals::sleep(500);
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
    GTUtilsProjectTreeView::findIndex(os, "Annotations", GTGlobals::FindOptions(false));
//    5. Click search again

    GTWidget::click(os, GTWidget::findWidget(os, "btnSearch"));

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
    GTUtilsProjectTreeView::findIndex(os, "Annotations", GTGlobals::FindOptions(false));//checks inside
//    5. Click search again
    GTWidget::click(os, GTWidget::findWidget(os, "btnSearch"));

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
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

//2. Open Find Pattern on the Option Panel. Enter a vaild pattern
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    GTKeyboardDriver::keySequence(os, "TA");

//3. Input invalid annotation name (empty, too long, illegal)
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Annotation parameters"));
    GTWidget::click(os, GTWidget::findWidget(os, "titleWidget"));
    for (int i = 0; i < 15; i++) {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
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
    GTGlobals::sleep(1000);

    GTWidget::click(os, GTWidget::findWidget(os, "getAnnotationsPushButton"));
    GTGlobals::sleep(500);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ACTION_PROJECT__REMOVE_SELECTED));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));

    //3. Delete created annotations document
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "MyDocument.gb"));
    GTMouseDriver::click(os, Qt::RightButton);
    //4. Click search again

    GTWidget::click(os, GTWidget::findWidget(os, "btnSearch"));

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    GTUtilsTaskTreeView::waitTaskFinished(os);
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

GUI_TEST_CLASS_DEFINITION( test_1597 ) {

    // 1. Open a sequence in UGENE.
    // 2. Input a pattern and search for it.
    // 3. Select "Create new table" in the "Save annotations(s)" group. You may also modify the pattern. Search again.
    // Expected result: the new table is created and found annotations are written to it.

    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    GTKeyboardDriver::keySequence(os, "ACAATGTATGCCTCTTGGTTTCTTCTATC");

    QLabel *obj = qobject_cast<QLabel*>(GTWidget::findWidget(os, "ArrowHeader_Save annotation(s) to"));
    GTWidget::click(os, obj);

    QRadioButton *newTable = qobject_cast<QRadioButton *>(GTWidget::findWidget(os, "newFileRB"));
    GTWidget::click(os, newTable);
        
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    GTKeyboardDriver::keySequence(os, "ACGTCGTCGTCGTCAATGTATGCCTCTTGGTTTCTTCTATC");


}
GUI_TEST_CLASS_DEFINITION( test_1603 ) {
    // 1. Open "_data/samples/CLUSTALW/COI.aln"
    // 2. Add a tree to the alignment by creating a new one
    // 3. Close MSA View
    // 4. Unload tree from project
    // 5. Open "COI.aln" from the Project View
    // Expected state: MSA Editor opens with the Tree View inside

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/1603.nwk", 0, 0, true));
    QAbstractButton *tree = GTAction::button(os,"Build Tree");
    GTWidget::click(os, tree);
    GTGlobals::sleep();

    GTUtilsMdi::closeWindow(os, "COI [m] COI");
    QString docName = "1603.nwk";                      

    GTUtilsDocument::unloadDocument(os, docName, false);
    GTGlobals::sleep();
    GTMouseDriver::moveTo( os, GTUtilsProjectTreeView::getItemCenter(os, "COI.aln"));
    GTMouseDriver::doubleClick(os);
    //TODO: check the expected state
}

GUI_TEST_CLASS_DEFINITION( test_1606 ) {
//    1.  File->Access remote database...
//    2.  Fill    "Resource ID": 1ezg
//                "Database": PDB
//        Open.
//    3.  Build SAS molecular surface.
//    4.  Close UGENE.
//    Expected state: UGENE doesn't crash on closing.
    GTLogTracer l;

    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFillerDeprecated(os, "1ezg", 3, true, false,
                                                                        sandBoxDir));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE),
                                QStringList() << ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Molecular Surface" << "SES"));
    QWidget *widget3d = GTWidget::findWidget(os, "1-1EZG");
    GTWidget::click(os, widget3d, Qt::RightButton);
    GTGlobals::sleep();

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_1607) {
    GTLogTracer l;
    //1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Create schema read variations->write variations
    WorkflowProcessItem *reader = GTUtilsWorkflowDesigner::addElement(os, "Read Variations");
    WorkflowProcessItem *writer = GTUtilsWorkflowDesigner::addElement(os, "Write Variations");
    GTUtilsWorkflowDesigner::connect(os, reader, writer);

    //3. Use input file "_common_data/NIAID_pipelines/Call_variants/data_to_compare_with/test_0001/out.vcf"
    GTUtilsWorkflowDesigner::addInputFile(os, "Read Variations", testDir + "_common_data/NIAID_pipelines/Call_variants/data_to_compare_with/test_0001/out.vcf");

    GTUtilsWorkflowDesigner::click(os, "Write Variations");
    QFile outputFile(sandBoxDir + "out.vcf");
    const QString outputFilePath = QFileInfo(outputFile).absoluteFilePath();
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", outputFilePath, GTUtilsWorkflowDesigner::textValue);

    //4. Run schema
    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state : output file not empty
    CHECK_SET_ERR(outputFile.exists() && outputFile.size() > 0, "Workflow output file is invalid");
    GTUtilsLog::check(os, l);
    outputFile.remove();
}

GUI_TEST_CLASS_DEFINITION(test_1609) {
    // 1) Open any file in UGENE
    QFile::copy(dataDir + "samples/FASTA/human_T1.fa", sandBoxDir + "human_T1.fa");
    GTFileDialog::openFile(os, sandBoxDir, "human_T1.fa");

    class CustomFileDialogUtils : public CustomScenario {
    public:
        CustomFileDialogUtils() {}
        virtual void run(U2::U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
            GTWidget::click(os, GTWidget::findButtonByText(os, "Cancel"));
        }
    };

    // 2) Delete that file from the file system
    // 3) Press "Yes" in appeared UGENE "Do you wish to save" dialog
    // 4) Press "Cancel" in appeared "Save as" dialog
    // Expected state : "Do you wish to save" dialog appeared
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, "sandBoxDir", "human_T1.fa", GTFileDialogUtils::Cancel, GTGlobals::UseMouse));
    QFile::remove(sandBoxDir + "human_T1.fa");
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

GUI_TEST_CLASS_DEFINITION( test_1628 ) {
//    1. Open COI.aln
//    2. Renames any row in an alignment, use a non-english characters in the alignment name.
//    3. Undo and redo.
//    Expected state: renamed named with name given at step 2

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsMSAEditorSequenceArea::renameSequence(os, "Montana_montana", "йцукен123");

    QAbstractButton *undo = GTAction::button( os, "msa_action_undo" );
    CHECK_SET_ERR(undo != NULL, "Undo button is NULL");
    GTWidget::click(os, undo);

    QAbstractButton *redo = GTAction::button( os, "msa_action_redo" );
    CHECK_SET_ERR(redo != NULL, "Redo button is NULL");
    GTWidget::click(os, redo);

    GTGlobals::sleep();
    QStringList names = GTUtilsMSAEditorSequenceArea::getNameList(os);

    CHECK_SET_ERR(names.contains("йцукен123") && !names.contains("Montana_montana"), "Undo-redo worked incorrectly");
}

GUI_TEST_CLASS_DEFINITION(test_1629) {
    //1. Open UGENE.
    //2. Close currect project if it is opened.
    //3. Click the menu item "File".
    //Expected: "Open As" sub-item exists and is available to click.
    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << ACTION_PROJECTSUPPORT__OPEN_AS, 
        PopupChecker::IsEnabled, GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_FILE);
}

GUI_TEST_CLASS_DEFINITION( test_1644 ) {
//    1) Run UGENE
//    2) Choose File/Access remote database
//    3) Choose UniProt (DAS) or Ensembl Human Genes (DAS) from combobox
//    4) Enter Resourse Id
//    State: if Ensembl Human Genes is choosen and example ID (ENSG00000139618) is entered then UGENE crashes
//    trying to recognize .gb format ( failed to detect ...)
    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFillerDeprecated(os, "ENSG00000139618", 7, true, false,
                                                                        sandBoxDir));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE),
                                QStringList() << ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTGlobals::sleep(5000);

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_1645) {
    //1. Open "_common_data/fasta/base_ext_nucl_all_symb.fa".

    //2. Select the "Join sequences into alignment and open in multiple alignment viewer" option and click "OK" in the appeared "Sequence reading options" dialog.
    //Expected state: The file opens in the MSA Editor.

    //3. Right click to the loaded document in the project view and select "Unload selected objects" from the context menu.
    //Expected state: The message box appeares.

    //4. Agree to close an opened view in the appeared message box.
    //Expected state: the document unloads.
    //GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "base_ext_nucl_all_symb.fa");
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Join));
    GTUtilsProject::openFiles(os, testDir + "_common_data/fasta/base_ext_nucl_all_symb.fa"); 
    GTGlobals::sleep();
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "base_ext_nucl_all_symb.fa"));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ACTION_PROJECT__UNLOAD_SELECTED));
    GTMouseDriver::click(os, Qt::RightButton);
    CHECK_SET_ERR(!GTUtilsDocument::isDocumentLoaded(os, "base_ext_nucl_all_symb.fa"), "Document should't be loaded");
}

GUI_TEST_CLASS_DEFINITION( test_1653 ) {
/*  1. Open some sequence
    2. Open find pattern option panel
    3. click "Show more options"
    Expected state: Algorithm - exact
    "should match" spinbox is not shown
    Actual: spinbox is shown
*/
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
    GTWidget::click(os, GTWidget::findWidget(os, "OP_FIND_PATTERN"));
    GTGlobals::sleep(500);
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search algorithm"));
    QSpinBox* spin = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "spinBoxMatch"));
    CHECK_SET_ERR(spin != NULL, "spinBoxMatch not found!");
    CHECK_SET_ERR(spin->isHidden(), "Warning spinbox is unexpectedly visible");
}

GUI_TEST_CLASS_DEFINITION( test_1654 ) {
    // 1. Open some sequence
    // 2. Copy some subsequence(for example first 10 symbols)
    // 3. Open find pattern option panel
    // 4. Use ctrl+v to paste subsequence to find pattern text area
    // 5. Collapse option panel
    // 6. Expand it and paste subsequence again
    // Expected state: {Search} button is enabled
    // Actual: {Search} button is disabled

    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 1, 15));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Select" << "Sequence region"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"));

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);

    GTKeyboardDriver::keyClick(os, 'v', GTKeyboardDriver::key["ctrl"]);

    GTWidget::click(os, GTWidget::findWidget(os, "OP_FIND_PATTERN"));
    GTGlobals::sleep(500);

    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);

    GTKeyboardDriver::keyClick(os, 'v', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(5000);

    QWidget *createAnnotationsButton = GTWidget::findWidget(os, "getAnnotationsPushButton");
    GTGlobals::sleep(500);
    CHECK_SET_ERR(createAnnotationsButton->isEnabled(), "Create annotations button is disabled!");

}

GUI_TEST_CLASS_DEFINITION( test_1660 ) {
    // 1. Open any sequence
    // 2. Open find pattern option panel
    // 3. Enter any valid pattern
    // 4. Click "Show more options" and select "custom region" in "Region" combobox
    // 5. Delete default value from right edge line edit
    // Expected state: lineedit became red Actual: UGENE crashes

    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 1, 15));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Select" << "Sequence region"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"));

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);

    GTKeyboardDriver::keyClick(os, 'v', GTKeyboardDriver::key["ctrl"]);

    //TODO setSearchInregion method checking
}
GUI_TEST_CLASS_DEFINITION(test_1661) {
    // 1. Open human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    // 2. Open find pattern option panel
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);

    // 3. Enter pattern ACAATGTATGCCTCTTGGTTTCTTCTATC
    GTKeyboardDriver::keySequence(os, "ACAATGTATGCCTCTTGGTTTCTTCTATC");

    // 4. Use settings : Region - custom region; 1 - 10000.
    QComboBox *boxRegion = qobject_cast<QComboBox *>(GTWidget::findWidget(os, "boxRegion"));
    GTWidget::click(os, boxRegion);
    GTComboBox::setCurrentIndex(os, boxRegion, 1);
    GTLineEdit::setText(os, qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "editStart")), "1");
    GTLineEdit::setText(os, qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "editEnd")), "10000");

    // Expected state : nothing found
    QLabel *resultLabel = qobject_cast<QLabel *>(GTWidget::findWidget(os, "resultLabel"));
    CHECK_SET_ERR(resultLabel->text() == "Results: 0/0", "Unexpected find algorithm result count");

    // 5. Use settings : Region - Whole sequence.
    GTComboBox::setCurrentIndex(os, boxRegion, 0);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    // Expected state : One match found
    CHECK_SET_ERR(resultLabel->text() == "Results: 1/1", "Unexpected find algorithm result count");
}

GUI_TEST_CLASS_DEFINITION(test_1664){
//    1. Open COI.aln.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//    2. Select something in the sequence area.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0,0), QPoint(5,5));
//    3. Try to modify the selection with SHIFT+arrows.
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["right"], GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep();
//    Expected state: selection changes its size.
    QRect expected = QRect(0, 0, 7, 6);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, expected);
//    Bug state: UGENE crashes.
}

GUI_TEST_CLASS_DEFINITION(test_1672) {
    //1. Open "COI.aln".
    //2. Set the "Zychia_baranovi" sequence as reference.
    //3. Open the "Statistics" options panel tab.
    //4. Set checked "Show distance algorithm" box, select the "Identity" algorithm.
    //Expected state: the reference sequence has 100% match with itself.
    //Bug state: the reference sequence has 103% match with itself.

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTWidget::click(os, GTWidget::findWidget(os, "OP_SEQ_STATISTICS_WIDGET"));
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-8, 8));
    GTWidget::click(os, GTWidget::findWidget(os, "addSeq"));
    QCheckBox* showDistancesColumnCheck = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "showDistancesColumnCheck"));
    GTCheckBox::setChecked(os, showDistancesColumnCheck, true);
    QComboBox *algoCombo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "algoComboBox"));
    GTComboBox::setIndexWithText(os, algoCombo, "Identity");
    QString num1 = GTUtilsMSAEditorSequenceArea::getSimilarityValue(os, 8);
    CHECK_SET_ERR(num1 == "100%", "unexpected sumilarity value an line 1: " + num1);
}

GUI_TEST_CLASS_DEFINITION(test_1680) {
//    For Mac only

//    1. Open "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

//    2. Choose some sequence by mouse in the name list area
    GTUtilsMsaEditor::clickSequenceName(os, "Tettigonia_viridissima");

//    3. Press "Shift"+"Arrow Up" a few times, then "Shift"+"Arrow Down" a few times
//    Expected state: The selection changes on every keystroke, for "Arrow Up" it grows to
//    the top of sequences list, for "Arrow Down" it does to the bottom
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);

    int expectedSelectionSize = 1;
    for (int i = 0; i < 9; i++) {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["up"]);
        expectedSelectionSize++;
        const int currentSelectionSize = GTUtilsMSAEditorSequenceArea::getSelectedSequencesNum(os);
        CHECK_SET_ERR(expectedSelectionSize == currentSelectionSize, QString("An unexpected selection size: expect %1, got %2").arg(expectedSelectionSize).arg(currentSelectionSize));
        GTGlobals::sleep(100);
    }

    {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["up"]);
        const int currentSelectionSize = GTUtilsMSAEditorSequenceArea::getSelectedSequencesNum(os);
        CHECK_SET_ERR(10 == currentSelectionSize, QString("An unexpected selection size: expect %1, got %2").arg(10).arg(currentSelectionSize));
        GTGlobals::sleep(100);
    }

    for (int i = 0; i < 9; i++) {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
        expectedSelectionSize--;
        const int currentSelectionSize = GTUtilsMSAEditorSequenceArea::getSelectedSequencesNum(os);
        CHECK_SET_ERR(expectedSelectionSize == currentSelectionSize, QString("An unexpected selection size: expect %1, got %2").arg(expectedSelectionSize).arg(currentSelectionSize));
        GTGlobals::sleep(100);
    }

    for (int i = 0; i < 8; i++) {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
        expectedSelectionSize++;
        const int currentSelectionSize = GTUtilsMSAEditorSequenceArea::getSelectedSequencesNum(os);
        CHECK_SET_ERR(expectedSelectionSize == currentSelectionSize, QString("An unexpected selection size: expect %1, got %2").arg(expectedSelectionSize).arg(currentSelectionSize));
        GTGlobals::sleep(100);
    }

    {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
        const int currentSelectionSize = GTUtilsMSAEditorSequenceArea::getSelectedSequencesNum(os);
        CHECK_SET_ERR(9 == currentSelectionSize, QString("An unexpected selection size: expect %1, got %2").arg(9).arg(currentSelectionSize));
        GTGlobals::sleep(100);
    }

    {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["up"]);
        const int currentSelectionSize = GTUtilsMSAEditorSequenceArea::getSelectedSequencesNum(os);
        CHECK_SET_ERR(8 == currentSelectionSize, QString("An unexpected selection size: expect %1, got %2").arg(8).arg(currentSelectionSize));
        GTGlobals::sleep(100);
    }

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["shift"]);
}

GUI_TEST_CLASS_DEFINITION(test_1687) {
    // 1. open samples / clustalW / COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    // 2. Open 'statistic tab'
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::Statistics);

    // 3. Be sure here is no reference sequence selected.
    QLineEdit *refSeqEdit = qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "sequenceLineEdit"));
    CHECK_SET_ERR(refSeqEdit->text().isEmpty(), "Unexpected reference sequence in MSA");

    QLabel *refSeqWarning = qobject_cast<QLabel *>(GTWidget::findWidget(os, "refSeqWarning"));
    CHECK_SET_ERR(refSeqWarning->isHidden(), "Warning label is unexpectedly visible");

    // 4. Set 'Set distance column' checked
    QCheckBox *check = qobject_cast<QCheckBox *>(GTWidget::findWidget(os, "showDistancesColumnCheck"));
    GTCheckBox::setChecked(os, check);

    // Expected state : hint with green text appears at the bottom of the tab.
    CHECK_SET_ERR(refSeqWarning->isVisible(), "Warning label is unexpectedly invisible");
    CHECK_SET_ERR(!refSeqWarning->text().isEmpty(), "Warning label contains no text");
}

GUI_TEST_CLASS_DEFINITION( test_1688 ) {
    // 1) Open file "_common_data/scenarios/_regression/1688/sr100.000.fa"
    // Expected state: UGENE show error, not crashed
    GTLogTracer l;

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os) );
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/_regression/1688/", "sr100.000.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsLog::check( os, l );
}

GUI_TEST_CLASS_DEFINITION(test_1693) {
//    1. Open WD.
//    2. Launch tuxedo pipeline.
//    3. Try to open any other sample while pipeline is running.
//    Expected state: UGENE doesn't crash.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    WizardFiller::pairValList parameters;
    WizardFiller::lineEditValue bowtieIndexFolder(testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data/test_0004/bowtie2_index/", true);
    WizardFiller::lineEditValue bowtieIndexBasename("NC_010473", false);
    parameters << WizardFiller::pairValLabel("Bowtie index directory", &bowtieIndexFolder);
    parameters << WizardFiller::pairValLabel("Bowtie index basename", &bowtieIndexBasename);
    GTUtilsDialog::waitForDialog(os, new ConfigureTuxedoWizardFiller(os, ConfigureTuxedoWizardFiller::full, ConfigureTuxedoWizardFiller::singleReads));
    GTUtilsDialog::waitForDialog(os, new TuxedoWizardFiller(os, testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data/test_0004/fastq1/", "exp_1_1.fastq", testDir + "_common_data/NIAID_pipelines/tuxedo_pipeline/data/test_0004/fastq2/", "exp_2_1.fastq", parameters));

    GTUtilsWorkflowDesigner::addSample(os, "RNA-seq analysis with Tuxedo tools");
    GTUtilsWorkflowDesigner::setCurrentTab(os, GTUtilsWorkflowDesigner::samples);
    GTUtilsWorkflowDesigner::runWorkflow(os);

    GTGlobals::sleep();
    const int tasksCount = GTUtilsTaskTreeView::getTopLevelTasksCount(os);
    CHECK_SET_ERR(1 == tasksCount, QString("An unexpected top level tasks count: expect %1, got %2. Workflow didn't launch?").arg(1).arg(tasksCount));

    GTUtilsWorkflowDesigner::returnToWorkflow(os);
    QWidget *samplesWidget = GTWidget::findWidget(os, "samples");
    CHECK_SET_ERR(NULL != samplesWidget, "Samples widget is NULL");
    CHECK_SET_ERR(!samplesWidget->isEnabled(), "Samples widget is unexpectedly enabled");
}

GUI_TEST_CLASS_DEFINITION(test_1700) {
    //    1. Open file "https://ugene.unipro.ru/tracker/secure/attachment/12864/pdb1a07.ent.gz".
    GTFileDialog::openFile(os, testDir + "_common_data/pdb/", "pdb1a07.ent.gz");

    // Expected state: UGENE displays sequence and 3d structure
    GTUtilsMdi::findWindow(os, "pdb1a07.ent.gz");


    // 2. In context menu of 3d view: {Render style -> Ball-and-Stick}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< "Render Style"<< "Ball-and-Stick"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "1-1A07"));

    //3) In context menu of 3d view: {Coloring scheme -> <Any different from current scheme>}
    //Repeat step 3) many times

    bool curSchemeFlag = true;
    for(int i = 0; i < 10; i++) {
        if(curSchemeFlag) {
            GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Coloring Scheme" << "Chemical Elements"));
            curSchemeFlag = false;
        }
        else {
            GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Coloring Scheme" << "Secondary Structure"));
            curSchemeFlag = true;
        }
        GTMenu::showContextMenu(os, GTWidget::findWidget(os, "1-1A07"));
    }
}

GUI_TEST_CLASS_DEFINITION( test_1701 ) {
    //1. Open 2 PDB files("_common_data/pdb/1A5H.pdb" and "_common_data/pdb/1CF7.pdb")
    //2. In each of them set {Render style -> Ball and Stick}
    //3. Close one of the views
    //Expected state: UGENE works fine. The opened view is displayed correctly(no black screen instead of molecule).

    GTFileDialog::openFile( os, testDir + "_common_data/pdb", "1A5H.pdb");
    GTFileDialog::openFile( os, testDir + "_common_data/pdb", "1CF7.pdb");

    QWidget* pdb1 = GTWidget::findWidget(os, "1-1A5H");
    QWidget* pdb2 = GTWidget::findWidget(os, "2-1CF7");
    CHECK_SET_ERR( NULL != pdb1, "No 1A5H view");
    CHECK_SET_ERR( NULL != pdb2, "No 1CF7 view");

    GTUtilsProjectTreeView::click(os, "1CF7.pdb");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< "Render Style"<< "Ball-and-Stick"));
    GTMenu::showContextMenu(os, pdb2);

    GTGlobals::sleep();
    QPixmap before = QPixmap::grabWidget(pdb2, pdb2->rect());

    GTMouseDriver::moveTo( os, GTUtilsProjectTreeView::getItemCenter(os, "1A5H.pdb"));
    GTMouseDriver::doubleClick(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< "Render Style"<< "Ball-and-Stick"));
    GTMenu::showContextMenu(os, pdb1);

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_ACTIONS), QStringList()<<"Close active view", GTGlobals::UseKey);

    GTGlobals::sleep();
    QPixmap after = QPixmap::grabWidget(pdb2, pdb2->rect());

    GTGlobals::sleep(500);
    CHECK_SET_ERR( before.toImage() == after.toImage(), "The view has changed");
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

GUI_TEST_CLASS_DEFINITION( test_1704 ){
//    1. Open _common_data\_regession\1704\lrr_test_new.gb
    GTFileDialog::openFile( os, testDir + "_common_data/regression/1704", "lrr_test_new.gb" );
//    2. Search HMM3 signals with _common_data\_regession\1704\LRR_4.hmm model
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()
                                                      << "ADV_MENU_ANALYSE"
                                                      << "Search HMM signals with HMMER3"));
    GTUtilsDialog::waitForDialog(os, new UHMM3SearchDialogFiller(os, testDir + "_common_data/regression/1704/LRR_4.hmm",
                                                                 sandBoxDir + "1704.gb"));

    GTMenu::showMainMenu(os, "mwmenu_actions");
    GTGlobals::sleep(1000);
//    3. Export document with annotations into genbank format. Do not add to the project
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Export document"));
    GTUtilsDialog::waitForDialog(os, new ExportDocumentDialogFiller(os, sandBoxDir, "1704_exported.gb"));
    GTUtilsProjectTreeView::click(os, "1704.gb", Qt::RightButton);
    GTGlobals::sleep();
//    4. Open the exported file with UGENE
    GTFileDialog::openFile(os, sandBoxDir, "1704_exported.gb");
    GTUtilsProjectTreeView::doubleClickItem(os, "1704_exported.gb");
    GTGlobals::sleep(1000);
//    Expected state: all the annotations of the new file are identical to annotations of the old file
    GTUtilsAnnotationsTreeView::findItem(os, "hmm_signal  (0, 27)");
//    Bug state: only the first annotations without qualifiers is opened
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
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_ALIGN<<"align_with_kalign", GTGlobals::UseKey));
    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os,10));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTUtilsTaskTreeView::waitTaskFinished(os);

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

GUI_TEST_CLASS_DEFINITION(test_1710_1){
    //1. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Add the "Find Patterns" sample.
    GTUtilsWorkflowDesigner::addSample(os, "Find patterns");

    //3. Setup inputs and outputs.
    GTUtilsWorkflowDesigner::addInputFile(os, "Read Sequence", dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsWorkflowDesigner::click(os, "Find Pattern");
    GTUtilsWorkflowDesigner::setParameter(os, "Pattern(s)", "AAA", GTUtilsWorkflowDesigner::textValue);

    //4) Press the validate button.
    //Expected state: The message appeared saying "Well done!".
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "Well done!"));
    GTWidget::click( os,GTAction::button(os, "Validate workflow"));
}

GUI_TEST_CLASS_DEFINITION(test_1710_2){
    //1. Open Settings -> Preferences -> External Tools.
    //2. Select any invalid file for Blast -> BlastAll.
    GTUtilsExternalTools::setToolUrl(os, "BlastAll", dataDir + "samples/FASTA/human_T1.fa");

    //3. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //4. Add the "Gene-by-gene approach" sample.
    GTUtilsWorkflowDesigner::addSample(os, "Gene-by-gene approach");

    //5. Setup inputs and outputs.
    GTUtilsWorkflowDesigner::addInputFile(os, "Read Sequence", dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsWorkflowDesigner::click(os, "Local BLAST Search");
    GTUtilsWorkflowDesigner::setParameter(os, "Database Path", sandBoxDir, GTUtilsWorkflowDesigner::textValue);
    GTUtilsWorkflowDesigner::setParameter(os, "Database Name", "test", GTUtilsWorkflowDesigner::textValue);

    //6. Press the validate button.
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "non-critical warnings"));
    GTWidget::click( os,GTAction::button(os, "Validate workflow"));

    //Expected state: there must be a warning "External tool is invalid. UGENE may not support this version of the tool or a wrong path to the tools is selected".
    GTUtilsWorkflowDesigner::checkErrorList(os, "External tool \"BlastAll\" is invalid. UGENE may not support this version of the tool or a wrong path to the tools is selected");
}

GUI_TEST_CLASS_DEFINITION(test_1714){
//    1. Open the "external tools" configuration window using Settings/Preferences menu
//    2. Select path for external tools package (if not set). External tools package can be downloaded from http://ugene.unipro.ru/external.html
//    3. Deselect all Cistrome tools
//    4. Deselect python external tool
//    Expected state: the python tool is deselected. UGENE doesn't hangs up (or crashes)

    class DeselectCistromeAndPython : public CustomScenario {
    public:
        DeselectCistromeAndPython() {}
        virtual void run(U2::U2OpStatus &os) {
            QStringList cistromeTools;
            cistromeTools << "go_analysis" << "seqpos" << "conservation_plot" << "peak2gene"
                          << "MACS" << "CEAS Tools";

            foreach (QString toolName, cistromeTools) {
                AppSettingsDialogFiller::clearToolPath(os, toolName);
            }
            GTGlobals::sleep(2000);

            AppSettingsDialogFiller::clearToolPath(os, "python");
            GTGlobals::sleep(2000);

            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new DeselectCistromeAndPython()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action__settings"));
    GTMenu::showMainMenu(os, MWMENU_SETTINGS);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_1720){
//1. Use menu {File->Access remote database...}
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFillerDeprecated(os, "D11266", 0));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);

    GTLogTracer l;
    GTGlobals::sleep(8000);//some time needed for request
//2. Fill field "Resource ID" with value D11266. Click "OK"

//3. Use menu {File->Access remote database...}
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFillerDeprecated(os, "D11266", 0));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);

    GTGlobals::sleep(8000);
//4. Fill field "Resource ID" with value D11266. Click "OK"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "D11266.gb"));
    GTUtilsLog::check(os,l);
//Expected state: project view with document "D11266.gb", no error messages in log appear
}

GUI_TEST_CLASS_DEFINITION(test_1747){
    //1. Open \data\samples\CLUSTALW\ty3.aln.gz
    //2. Enable the distances column in options panel or create distances matrix by using menu {statistics->Generate distance matrix}
    //Expected state: progress for "Generete distance matrix" correctly displays current state of calculation
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "ty3.aln.gz");
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Statistics" << "Generate distance matrix"));
    GTUtilsDialog::waitForDialog(os, new DistanceMatrixDialogFiller(os));
    GTUtilsMSAEditorSequenceArea::callContextMenu(os);


    QProgressBar* taskProgressBar = GTWidget::findExactWidget<QProgressBar*>(os, "taskProgressBar");
    QString text = taskProgressBar->text();
    CHECK_SET_ERR(text.contains("%"), "unexpected text: " + text);
    text = text.left(text.length() - 1);
    bool isNumber = false;
    int progress = text.toInt(&isNumber);
    CHECK_SET_ERR(isNumber, QString("The progress must be a number: %1").arg(text));
    CHECK_SET_ERR(progress >= 0 && progress <= 100, QString("Incorrect progress: %1").arg(progress));
    int oldProgress = progress;

    GTGlobals::sleep(5000);
    text = taskProgressBar->text();
    CHECK_SET_ERR(text.contains("%"), "unexpected text: " + text);
    text = text.left(text.length() - 1);
    isNumber = false;
    progress = text.toInt(&isNumber);
    CHECK_SET_ERR(isNumber, QString("The progress must be a number: %1").arg(text));
    CHECK_SET_ERR(progress >= 0 && progress <= 100, QString("Incorrect progress: %1").arg(progress));

    CHECK_SET_ERR(progress > oldProgress, "Progress didn't groving up");
}
GUI_TEST_CLASS_DEFINITION(test_1756){
/*  1. Open WD.
    2. Add the element "Collect Motifs with SeqPos" to the scene.
    3. Click the element.
    Expected: the property "Motif database" is not required (no bold text).
*/
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Collect Motifs with SeqPos");
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Collect Motifs with SeqPos"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setParameter(os, "Motif database", QStringList(), GTUtilsWorkflowDesigner::ComboChecks);
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Collect Motifs with SeqPos"));
    GTMouseDriver::click(os);

    GTUtilsDialog::waitForDialog( os, new MessageBoxDialogFiller( os, QMessageBox::Ok,
        "Please fix issues listed in the error list (located under workflow)."));
    GTWidget::click(os,GTAction::button( os,"Validate workflow" ));
    GTGlobals::sleep(500);


    CHECK_SET_ERR( 0 == GTUtilsWorkflowDesigner::checkErrorList( os, "Required parameter is not set: Motif database" ),
                   "The property Motif database is required. This is wrong.");

}
GUI_TEST_CLASS_DEFINITION(test_1731){
    //1. Open \data\samples\CLUSTALW\ty3.aln.gz

    //2. Select CopCinTy34 as a reference

    //3. Enable the distances column

    //4. Select the Identity algorithm
    //Expected state: identity of CopCinTy34 with each sequence is shown in the column

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "ty3.aln.gz");
    GTWidget::click(os, GTWidget::findWidget(os, "OP_SEQ_STATISTICS_WIDGET"));
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-5, 5));
    GTWidget::click(os, GTWidget::findWidget(os, "addSeq"));
    QCheckBox* showDistancesColumnCheck = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "showDistancesColumnCheck"));
    GTCheckBox::setChecked(os, showDistancesColumnCheck, true);
    GTGlobals::sleep(200000);
    QString num1 = GTUtilsMSAEditorSequenceArea::getSimilarityValue(os, 1);
    QString num3 = GTUtilsMSAEditorSequenceArea::getSimilarityValue(os, 3);
    CHECK_SET_ERR(num1 != "100%", "unexpected sumilarity value an line 1: " + num1);
    CHECK_SET_ERR(num3 != "100%", "unexpected sumilarity value an line 3: " + num3);
}

GUI_TEST_CLASS_DEFINITION(test_1786){
    // 1. Use menu {File->Access remote database...}
    // 2. Select database UniProt(DAS)
    // 3. Fill resource id: 1CRN. Press ok
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFillerDeprecated(os, "1CRN", 7));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    // Expected state: document P01542_das.gb appeared in project
    GTUtilsProjectTreeView::findIndex(os, "P01542_das.gb");
}

GUI_TEST_CLASS_DEFINITION(test_1797){
    // 1) Open _common_data\scenarios\formats\test_1797 file
    // 2) Choose "BED" format at appeared format dialog
    // Expected state: UGENE doesn't crash - it shows error: "BED parsing error: incorrect number of fields at line 1!"

    GTUtilsDialog::waitForDialog(os, new SelectDocumentFormatDialogFiller(os));
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/formats", "test_1797");
}

GUI_TEST_CLASS_DEFINITION(test_1798){
    //1. Open file _common_data\scenarios\_regression\1798\1.4k.aln.
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/_regression/1798", "1.4k.aln");
    //2. Start MAFFT with default values.
    GTUtilsDialog::waitForDialog(os, new MAFFTSupportRunDialogFiller(os, new MAFFTSupportRunDialogFiller::Parameters()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with MAFFT", GTGlobals::UseMouse));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep(3000);

    //3. Open Tasks tab
    //   Expected state: check tasks progress percentage is correct
    QProgressBar* taskProgressBar = GTWidget::findExactWidget<QProgressBar*>(os, "taskProgressBar");
    QString text = taskProgressBar->text();
    CHECK_SET_ERR(text.contains("%"), "unexpected text: " + text);
    text = text.left(text.length() - 1);
    bool isNumber = false;
    int progress = text.toInt(&isNumber);
    CHECK_SET_ERR(isNumber, QString("The progress must be a number: %1").arg(text));
    CHECK_SET_ERR(progress >= 0 && progress <= 100, QString("Incorrect progress: %1").arg(progress));
}

GUI_TEST_CLASS_DEFINITION(test_1808) {
    QDir outputDir(testDir + "_common_data/scenarios/sandbox");
    const QString outputFilePath = outputDir.absolutePath( ) + "/test_1808.gb";

    // 1.Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    // 2.Create schema "Read Annotations"->"Write annotations"
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Read Annotations");
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Write annotations");
    WorkflowProcessItem *read = GTUtilsWorkflowDesigner::getWorker(os, "Read Annotations");
    WorkflowProcessItem *write = GTUtilsWorkflowDesigner::getWorker(os, "Write annotations");
    GTUtilsWorkflowDesigner::connect(os, read, write);

    // 3.Select any input and output file
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read Annotations"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Genbank", "sars.gb");
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Write Annotations"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", outputFilePath, GTUtilsWorkflowDesigner::textValue);

    // 4.Validate schema
    // Expected state: No warning about empty input sequence slot
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "Well done!"));
    GTWidget::click( os,GTAction::button( os,"Validate workflow" ) );
}

GUI_TEST_CLASS_DEFINITION(test_1811_1) {
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFillerDeprecated(os, "A0N8V2", 5));
    GTUtilsDialog::waitForDialog(os, new SelectDocumentFormatDialogFiller(os));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);

    GTGlobals::sleep(10000);//some time needed for request
    GTUtilsDocument::isDocumentLoaded(os, "A0N8V2.txt");
}

GUI_TEST_CLASS_DEFINITION( test_1813 )
{
    // 1) Select the menu {File->Access remote database}
    // 2) Fill the "Fetch Data from Remote Database" dialog with the next values:
    //      Resource ID: I7G8J3
    //      Database: UniProt (DAS)
    // 3) Press "OK"
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFillerDeprecated(os, "I7G8J3", 7));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseMouse);

    // Expected: the sequence view with I7G8J3 sequence is opened. UGENE does not crash.
    GTGlobals::sleep(5000);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsDocument::isDocumentLoaded(os, "I7G8J3_das.gb");
    GTUtilsDocument::checkDocument(os, "I7G8J3_das.gb", AnnotatedDNAViewFactory::ID);
}

GUI_TEST_CLASS_DEFINITION( test_1821 ) {
    QString workflowOutputDirPath( testDir + "_common_data/scenarios/sandbox" );
    QDir workflowOutputDir( workflowOutputDirPath );

    GTUtilsDialog::waitForDialog( os, new StartupDialogFiller( os,
        workflowOutputDir.absolutePath( ) ) );

    //1. Open WD
    QMenu* menu=GTMenu::showMainMenu( os, MWMENU_TOOLS );
    GTMenu::clickMenuItemByName( os, menu, QStringList( ) << "Workflow Designer" );
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
    GTMenu::clickMenuItemByName( os, GTMenu::showMainMenu( os, MWMENU_ACTIONS ),
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

GUI_TEST_CLASS_DEFINITION(test_1831) {
    // 1) Create a schema with shrunk elements state.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::addSample(os, "Align sequences with MUSCLE");

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Align with MUSCLE"));
    GTMouseDriver::doubleClick(os);

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Write alignment"));
    GTMouseDriver::doubleClick(os);

    CHECK_SET_ERR(!GTUtilsWorkflowDesigner::isWorkerExtended(os, "Align with MUSCLE"), "\"Align with MUSCLE\" unexpectedly has extended style");
    CHECK_SET_ERR(!GTUtilsWorkflowDesigner::isWorkerExtended(os, "Write alignment"), "\"Write alignment\" unexpectedly has extended style");
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::isWorkerExtended(os, "Read alignment"), "\"Read alignment\" unexpectedly has simple style");

    // 2) Save the schema.
    const QString workflowOutputDirPath(testDir + "_common_data/scenarios/sandbox");
    QDir workflowOutputDir(workflowOutputDirPath);

    GTUtilsDialog::waitForDialog(os, new WorkflowMetaDialogFiller(os, workflowOutputDir.absolutePath() + "/" + "test.uwl", "Workflow"));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_ACTIONS), QStringList() << "Save workflow action", GTGlobals::UseKey);

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTMouseDriver::click(os);

    // 3) Reopen UGENE WD.
    GTFileDialog::openFile(os, workflowOutputDirPath, "test.uwl");

    // Expected: the state is saved.
    CHECK_SET_ERR(!GTUtilsWorkflowDesigner::isWorkerExtended(os, "Align with MUSCLE"), "\"Align with MUSCLE\" unexpectedly has extended style");
    CHECK_SET_ERR(!GTUtilsWorkflowDesigner::isWorkerExtended(os, "Write alignment"), "\"Write alignment\" unexpectedly has extended style");
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::isWorkerExtended(os, "Read alignment"), "\"Read alignment\" unexpectedly has simple style");
}

GUI_TEST_CLASS_DEFINITION(test_1834) {
    //1. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Drag the "Read Alignment" and "File Format Conversion" elements to the scene and link them.
    WorkflowProcessItem *reader = GTUtilsWorkflowDesigner::addElement(os, "Read Alignment");
    WorkflowProcessItem *converter = GTUtilsWorkflowDesigner::addElement(os, "File Format Conversion");
    GTUtilsWorkflowDesigner::connect(os, reader, converter);

    //3. Set the input file for the "Read Alignment" element to "data/samples/COI.aln".
    GTUtilsWorkflowDesigner::addInputFile(os, "Read Alignment", dataDir + "samples/CLUSTALW/COI.aln");

    //4. Set the "Document format" parameter of the "File Format Conversion" element to "mega".
    GTUtilsWorkflowDesigner::click(os, "File Format Conversion");
    GTUtilsWorkflowDesigner::setParameter(os, "Document format", 13, GTUtilsWorkflowDesigner::comboValue);
    GTUtilsWorkflowDesigner::setParameter(os, "Output directory", 0, GTUtilsWorkflowDesigner::comboValue);
    GTUtilsWorkflowDesigner::setParameter(os, "Custom directory", QDir().absoluteFilePath(sandBoxDir + "regression_1834"), GTUtilsWorkflowDesigner::textValue);

    //5. Run the scheme.
    GTWidget::click(os, GTAction::button(os, "Run workflow"));
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: Scheme ran successfully, the "COI.aln.mega" output file has appeared on the "Output Files" panel of the dashboard.
    //The output file is valid, might be opened with MSA editor and has the same content as the source file.
    CHECK_SET_ERR(QFile::exists(sandBoxDir + "regression_1834/COI.aln.meg"), "File does not exist");
}

GUI_TEST_CLASS_DEFINITION( test_1859 ) {
    QString workflowOutputDirPath( testDir + "_common_data/scenarios/sandbox" );
    QDir workflowOutputDir( workflowOutputDirPath );
    const QString outputFilePath = workflowOutputDir.absolutePath( ) + "/test.gb";

    // 1) Open Workflow Designer
    GTUtilsWorkflowDesigner::openWorkflowDesigner( os );

    const QString annReaderName = "Read Annotations";
    const QString annWriterName = "Write Annotations";

    // 2) Build workflow of elements: "Write Annotations" and "Read Annotations"
    GTUtilsWorkflowDesigner::addAlgorithm( os, annReaderName );
    GTUtilsWorkflowDesigner::addAlgorithm( os, annWriterName );

    WorkflowProcessItem *annReader = GTUtilsWorkflowDesigner::getWorker( os, annReaderName );
    WorkflowProcessItem *annWriter = GTUtilsWorkflowDesigner::getWorker( os, annWriterName );

    GTUtilsWorkflowDesigner::connect(os, annReader, annWriter );

    // 3) Set output file

    GTMouseDriver::moveTo( os, GTUtilsWorkflowDesigner::getItemCenter( os, annWriterName ) );
    GTMouseDriver::click( os );
    GTUtilsWorkflowDesigner::setParameter( os, "Output file", outputFilePath,
        GTUtilsWorkflowDesigner::textValue );

    // 4) Set input file
    GTMouseDriver::moveTo( os, GTUtilsWorkflowDesigner::getItemCenter( os, annReaderName ) );
    GTMouseDriver::click( os );
    GTUtilsWorkflowDesigner::setDatasetInputFile( os, dataDir + "samples/Genbank", "sars.gb" );

    GTLogTracer lt;

    // 5) Run workflow
    GTWidget::click( os,GTAction::button( os,"Run workflow" ) );
    GTGlobals::sleep( );

    // 6) Block file for writing
    PermissionsSetter permSetter;
    QFile::Permissions p = QFile::WriteOwner | QFile::WriteUser | QFile::WriteGroup
        | QFile::WriteOther;
    bool res = permSetter.setPermissions( outputFilePath, ~p );
    CHECK_SET_ERR( res, "Can't set permissions" );

    // 7) Run workflow again
    GTWidget::click( os,GTAction::button( os,"Run workflow" ) );
    GTGlobals::sleep( );

    CHECK_SET_ERR(lt.hasError(), "no error in log");
}

GUI_TEST_CLASS_DEFINITION(test_1860) {
    // 1) Open Workflow Designer
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    QMenu *menu=GTMenu::showMainMenu( os, MWMENU_TOOLS );
    GTMenu::clickMenuItemByName( os, menu, QStringList( ) << "Workflow Designer" );

    const QString textReaderName = "Read Plain Text";
    const QString textWriterName = "Write Plain Text";

    //2) Add elements: "Write plain text" and "Read plain text"
    GTUtilsWorkflowDesigner::addAlgorithm( os, textReaderName);
    GTUtilsWorkflowDesigner::addAlgorithm( os, textWriterName);

    WorkflowProcessItem *textReader = GTUtilsWorkflowDesigner::getWorker( os, textReaderName);
    WorkflowProcessItem *textWriter = GTUtilsWorkflowDesigner::getWorker( os, textWriterName);

    //3) Connect them to each other
    GTUtilsWorkflowDesigner::connect(os, textReader, textWriter);

    //4) Try to set output file

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, textWriterName));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", "test", GTUtilsWorkflowDesigner::textValue );
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, textReaderName));
    GTMouseDriver::click(os);
}

GUI_TEST_CLASS_DEFINITION(test_1865) {
    // 1) Open Workflow Designer
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    QMenu *menu=GTMenu::showMainMenu( os, MWMENU_TOOLS );
    GTMenu::clickMenuItemByName( os, menu, QStringList( ) << "Workflow Designer" );

    const QString sequenceWriterName = "Write Sequence";

    //2) Add elements: "Write sequence"
    GTUtilsWorkflowDesigner::addAlgorithm( os, sequenceWriterName);

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, sequenceWriterName));
    GTMouseDriver::click(os);

    }

GUI_TEST_CLASS_DEFINITION( test_1883 ){
// 1. Open file "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma.aln" );
// 2. Select any area in sequence view
    const int startRowNumber = 6;
    const int alignmentLength = 12;
    GTUtilsMSAEditorSequenceArea::click( os, QPoint( -5, startRowNumber ) );
    GTGlobals::sleep( 200 );
    GTUtilsMSAEditorSequenceArea::checkSelectedRect( os, QRect( 0, startRowNumber,
        alignmentLength, 1 ) );
// 3. Use context menu: "Edit -> Replace selected rows with complement" or "reverse" or "reverse-complement"
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT
        << "replace_selected_rows_with_reverse") );
    GTMouseDriver::click(os, Qt::RightButton);
// Expected state: the bases in the selected area were replaced accordingly to the chosen variant
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    QString selectionContent = GTClipboard::text( os );
    CHECK_SET_ERR( "AATTATTAGACT" == selectionContent, "MSA changing is failed" );
// 4. Press "Ctrl + Z"
    GTKeyboardDriver::keyClick( os, 'z', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
// Expected result: all rows in the selection were restored
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    selectionContent = GTClipboard::text( os );
    CHECK_SET_ERR( "TCAGATTATTAA" == selectionContent, "MSA changing is failed" );
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
    GTGlobals::sleep(2000);
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

GUI_TEST_CLASS_DEFINITION( test_1897 ) {
    //1) Run UGENE
    //2) Open data/samples/CLUSTALW/COI.aln
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW", "COI.aln" );

    //3) Selected any region on alignment view using mouse or keyboard
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 40, 4 ), QPoint( 50, 12 ) );

    //4) Choose Highlighting/Gaps action from context-menu
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList( ) << "Highlighting"
        << "Gaps" ) );
    GTMouseDriver::click( os, Qt::RightButton );

    //5) Look at Hightlighting/Gaps action again
    QAction *action = GTAction::findActionByText( os, "Gaps" );
    //Expected state: It must be checked
    CHECK_SET_ERR( action->isChecked( ), "Action has to be checked!" );
}

GUI_TEST_CLASS_DEFINITION(test_1908){

    // 1. Open WD
    // 2. Add "Call variants" sample
    // Expected state: datasets are avaluable

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::addSample(os, "Call variants with SAMtools");
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read Assembly (BAM/SAM)"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Assembly/", "chrM.sam");

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read Sequence"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Assembly/", "chrM.fa");

}

GUI_TEST_CLASS_DEFINITION(test_1918) {
    //1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Place to the empty scheme "File List" and "File Conversion" elements
    WorkflowProcessItem *fileList = GTUtilsWorkflowDesigner::addElement(os, "File List");
    WorkflowProcessItem *converter = GTUtilsWorkflowDesigner::addElement(os, "File Format Conversion");

    //3. Bind the elements
    GTUtilsWorkflowDesigner::connect(os, fileList, converter);

    //4. Add the following input files to the "File List" dataset: "test/_common_data/mega/MegaTest_1.meg",
    //                                                             "test/_common_data/mega/MegaTest_2.meg",
    //                                                             "test/_common_data/clustal/align.aln"
    GTUtilsWorkflowDesigner::addInputFile(os, "File List", testDir + "_common_data/mega/MegaTest_1.meg");
    GTUtilsWorkflowDesigner::addInputFile(os, "File List", testDir + "_common_data/mega/MegaTest_2.meg");
    GTUtilsWorkflowDesigner::addInputFile(os, "File List", testDir + "_common_data/clustal/align.aln");

    //5. Set the following parameters of the "File Conversion" element: { Document format : nexus },
    //                                                                  { Excluded formats : clustal }
    GTUtilsWorkflowDesigner::click(os, "File Format Conversion");
    GTUtilsWorkflowDesigner::setParameter(os, "Document format", "nexus", GTUtilsWorkflowDesigner::comboValue, GTGlobals::UseMouse);
    GTUtilsWorkflowDesigner::setParameter(os, "Excluded formats", QStringList("clustal"), GTUtilsWorkflowDesigner::ComboChecks);
    GTUtilsWorkflowDesigner::setParameter(os, "Output directory", 0, GTUtilsWorkflowDesigner::comboValue);
    GTUtilsWorkflowDesigner::setParameter(os, "Custom directory", QDir().absoluteFilePath(sandBoxDir + "regression_1918")
                                          , GTUtilsWorkflowDesigner::textValue);

    //6. Run the scheme.
    GTWidget::click(os, GTAction::button(os, "Run workflow"));

    //Expected state: After workflow finish there are two output files in the dashboard: "MegaTest_1.meg.nexus",
    //                                                                                   "MegaTest_2.meg.nexus"
    GTUtilsTaskTreeView::waitTaskFinished(os);
    CHECK_SET_ERR(QFile::exists(sandBoxDir + "regression_1918/MegaTest_1.meg.nex"), "File 1 does not exist");
    CHECK_SET_ERR(QFile::exists(sandBoxDir + "regression_1918/MegaTest_2.meg.nex"), "File 2 does not exist");
    CHECK_SET_ERR(!QFile::exists(sandBoxDir + "regression_1918/align.aln.nex"), "File 3 exists");
}

GUI_TEST_CLASS_DEFINITION( test_1919 )
{
    //1) Create the WD scheme: File list -> File conversions.
    //2) Set input file: a BAM file (e.g _common_data/bam/scerevisiae.bam).
    //3) Set the result format of the converter: BAM
    //4) Run the scheme.
    //Expected: the scheme is finished well, no errors appeared.

    GTLogTracer l;

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

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
    GTUtilsWorkflowDesigner::setParameter(os, "Document format", 2, GTUtilsWorkflowDesigner::comboValue);

    // add setting source url in input data

    GTWidget::click(os,GTAction::button(os,"Run workflow"));

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
    const int MSA_WIDTH = 30;
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

GUI_TEST_CLASS_DEFINITION( test_2009 ){
//    1) Open Settings/Preferences/External tools
//    2) Set correct BWA or any other tool path (preferably just executable)
    class BWAInactivation : public CustomScenario {
    public:
        BWAInactivation() {}
        virtual void run(U2::U2OpStatus &os) {
            //    3) Clear set path
            AppSettingsDialogFiller::clearToolPath(os, "BWA");

            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTLogTracer l;

    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new BWAInactivation()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action__settings"));
    GTMenu::showMainMenu(os, MWMENU_SETTINGS);
//    4) Look at UGENE log
    GTUtilsLog::check(os, l);
//    Expected state: UGENE doesn't write any error to log
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
    CHECK_SET_ERR(  "AAGCTTCTTTTAA\nAAGTTACTAA---\nTAG---TTATTAA\nAAGC---TATTAA\n"
                    "TAGTTATTAA---\nTAGTTATTAA---\nTAGTTATTAA---\nAAGCTTT---TAA\n"
                    "A--AGAATAATTA\nAAGCTTTTAA---" == finalMsaContent,
                    "Unexpected MSA content has occurred\n expected: \nAAGCTTCTTTTAA\nAAGTTACTAA---\nTAG---TTATTAA\nAAGC---TATTAA\n"
                    "TAGTTATTAA---\nTAGTTATTAA---\nTAGTTATTAA---\nAAGCTTT---TAA\n"
                    "A--AGAATAATTA\nAAGCTTTTAA---\n actual: \n" + finalMsaContent);
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
    CHECK_SET_ERR(  "AAGCTTCTTTTAA\nAAGTTACTAA---\nTAG---TTATTAA\nAAGC---TATTAA\n"
        "TAGTTATTAA---\nTAGTTATTAA---\nTAGTTATTAA---\nAAGCTTT---TAA\n"
        "A--AGAATAATTA\nAAGCTTTTAA---" == finalMsaContent,
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
    // 1. Open test/_common_data/scenarios/msa/ma2_gap_col.aln.
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/msa", "ma2_gap_col.aln" );

    //2. Select the 8 and 9 of the third line (two symbols after gaps).
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 7, 2 ), QPoint( 8, 2 ) );
    GTGlobals::sleep(200);

    //3. Press BACKSPACE.
    GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["back"] );
    GTGlobals::sleep( 200 );

    // 4. Expected state: the gap was deleted, selection moves to the previous symbol.
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 2 ), QPoint( 13, 2 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR(  "TAG--TTATTAA--" == finalMsaContent,
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
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

//    2. Add element "Local BLAST Search"
    GTUtilsWorkflowDesigner::addAlgorithm( os, "Local BLAST Search" );
    GTMouseDriver::moveTo(os,GTUtilsWorkflowDesigner::getItemCenter(os, "blast"));
    GTMouseDriver::click(os);

//    Expected state: element has parameters "gap cost" and "match score"
    GTUtilsWorkflowDesigner::setParameter(os, "Gap costs", 2, GTUtilsWorkflowDesigner::comboValue);
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
    QModelIndex documentItem = GTUtilsProjectTreeView::findIndex(os,"abcd.fa");

    CHECK_SET_ERR("[s] a" == documentItem.child(0, 0).data(), "Unexpected name of the object in the project view!");
    CHECK_SET_ERR("[s] b" == documentItem.child(1, 0).data(), "Unexpected name of the object in the project view!");
    CHECK_SET_ERR("[s] c" == documentItem.child(2,0).data(), "Unexpected name of the object in the project view!");
    CHECK_SET_ERR("[s] d" == documentItem.child(3, 0).data(), "Unexpected name of the object in the project view!");

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

GUI_TEST_CLASS_DEFINITION( test_2049 ){

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Codon table"));
    GTWidget::click(os, GTWidget::findWidget(os, "AminoToolbarButton"));
    GTGlobals::sleep(500);
    QWidget* w = GTWidget::findWidget(os, "Codon table widget");
    int ititHeight = GTWidget::findWidget(os, "Leucine (Leu, L)",w)->geometry().height();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"3. The Yeast Mitochondrial Code"));
    GTWidget::click(os, GTWidget::findWidget(os, "AminoToolbarButton"));
    GTGlobals::sleep(500);

    w = GTWidget::findWidget(os, "Codon table widget");
    int finalHeight = GTWidget::findWidget(os, "Leucine (Leu, L)",w)->geometry().height();
    CHECK_SET_ERR(ititHeight != finalHeight, "codone table not changed");
    }

GUI_TEST_CLASS_DEFINITION( test_2070 ){
    GTLogTracer lt;
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFillerDeprecated(os, "Q9IGQ6", 7));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTGlobals::sleep();
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsDocument::isDocumentLoaded(os, "Q9IGQ6_das.gb");
    GTUtilsDocument::checkDocument(os, "Q9IGQ6_das.gb", AnnotatedDNAViewFactory::ID);

    GTUtilsLog::check(os, lt);
}
GUI_TEST_CLASS_DEFINITION( test_2077 ){
    // 1) Open WD

    // 2) Add elements "Read Sequence" and "Write sequence" to the scheme
    // 3) Connect "Read Sequence" to "Write sequence"

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

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
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os, "!@#$%^&*()_+\";:?/", false));

    // 2. Open WD
    // 3. Set any folder without write access as workflow output directory. Click OK.
    QMenu *menu = GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, menu, QStringList() << "Workflow Designer");
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
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(1, 1));
    const QString colorSchemeName = getName() + "_Scheme";
    GTUtilsDialog::waitForDialog( os, new PopupChooser( os, QStringList( ) << "Colors"
        << "Custom schemes" << "Create new color scheme" ) );
    GTUtilsDialog::waitForDialog( os, new NewColorSchemeCreator( os, colorSchemeName,
        NewColorSchemeCreator::amino ) );
    GTMouseDriver::click( os, Qt::RightButton );

    // 3. Create a new color scheme for the amino alphabet.
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(1, 1));
    GTUtilsDialog::waitForDialog( os, new PopupChooser( os, QStringList( ) << "Colors"
        << "Custom schemes" << colorSchemeName ) );
    GTMouseDriver::click( os, Qt::RightButton );
}

GUI_TEST_CLASS_DEFINITION(test_2091) {
    //1. Open "data/samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");

    //Expected state: document are opened in the project view; MSA Editor are shown with test_alignment.
    GTUtilsProjectTreeView::findIndex(os, "COI.aln");
    QWidget *msaView = GTUtilsMdi::activeWindow(os);
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
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

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
    QWidget* loadSchemaButton = GTWidget::findWidget(os, "Load work");
    CHECK_SET_ERR(loadSchemaButton, "Load schema button not found");
    GTWidget::click(os, loadSchemaButton);

    GTGlobals::sleep();

//    Expected result: the scheme with parameters is loaded.
    WorkflowProcessItem* wdElement = GTUtilsWorkflowDesigner::getWorker(os, "File list");
    CHECK_SET_ERR(wdElement, "Schema wasn't loaded");
}

GUI_TEST_CLASS_DEFINITION(test_2093_2) {
    // 1. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

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

    WorkflowProcessItem* wdElement = GTUtilsWorkflowDesigner::getWorker(os, "Read sequence");
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
#ifdef Q_OS_MAC
    //hack for mac
    MainWindow *mw = AppContext::getMainWindow();
    CHECK_SET_ERR(mw != NULL, "MainWindow is NULL");
    QMainWindow *mainWindow = mw->getQMainWindow();
    CHECK_SET_ERR(mainWindow != NULL, "QMainWindow is NULL");
    QWidget* w = qobject_cast<QWidget*>(mainWindow);
    GTWidget::click(os, w, Qt::LeftButton, QPoint(5,5));
    GTGlobals::sleep(500);
#endif

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Join));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/_regression/2138/1.fa");

    GTGlobals::sleep(1000);
    //Expected state: alignment has been opened and whole msa alphabet is amino
    bool isAmino = GTUtilsMSAEditorSequenceArea::hasAminoAlphabet(os);
    CHECK_SET_ERR(true == isAmino, "Aligment has wrong alphabet type");
}

GUI_TEST_CLASS_DEFINITION( test_2140 )
{
    //1. Use main menu {Tools->Dna assembly->convert UGENE assembly database to SAM format}
    //2. Select any .fasta file instead of .ugenedb file
    //3. Click "convert"
    //Expected state: UGENE not crashes. Error message is written to log

    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new ConvertAssemblyToSAMDialogFiller(os, dataDir + "samples/FASTA/", "human_T1.fa"));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_TOOLS),
                          QStringList() << "DNA assembly" << "Convert UGENE Assembly database to SAM format");

    CHECK_SET_ERR(l.hasError() == true, "There is no error message in log");
}

GUI_TEST_CLASS_DEFINITION( test_2144 )
{
//    1. Open Workflow Designer.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

//    2. Open the NGS sample scheme "Call variants with SAM tools".
    GTUtilsWorkflowDesigner::addSample(os, "Call variants with SAMtools");

//    3. Fill input data, e.g.:
//        "data/samples/Assembly/chrM.sam" as input to "Read Assembly SAM/BAM" element;
//        "data/samples/Assembly/chrM.fa" as input to "Read Sequence" element;
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read Assembly (BAM/SAM)"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Assembly/", "chrM.sam");

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read Sequence"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Assembly/", "chrM.fa");

//    4. Chose "Estimate" option in tool bar.
//       "Estimate" option is available only for NGS samples (except "Extract transcript sequence").
//    Expected state: Estimation dialog appears and provides information about approximate time of workflow run.
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os,
                                                                QMessageBox::Close,
                                                                "Approximate estimation time of the workflow run is" ));
    GTWidget::click(os, GTAction::button(os, "Estimate workflow"));
}

GUI_TEST_CLASS_DEFINITION( test_2150 ){
    // 1. Open Workflow Designer.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
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
    GTGlobals::sleep(5000);
    //GTUtilsTaskTreeView::waitTaskFinished(os,1000);

    // 6. During the workflow execution open the "Tasks" panel in the bottom, find in the task tree the "MUSCLE alignment" subtask and cancel it.
    GTUtilsTaskTreeView::cancelTask(os, "MUSCLE alignment");
}

GUI_TEST_CLASS_DEFINITION( test_2152 ){
    //1. Open WD
    //2. Create a scheme with the following elements: File list, Assembly Sequences with CAP3
    //3. Put _common_data/cap3/region1.fa,
    //       _common_data/cap3/region2.fa,
    //       _common_data/cap3/region3.fa,
    //       _common_data/cap3/region4.fa as an input sequences
    //4. Run the scheme
    //Expected state: the dashboard appears, the WD task has been finished without errors.

    GTLogTracer l;
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    GTUtilsWorkflowDesigner::addAlgorithm( os, "File List" );
    GTUtilsWorkflowDesigner::addAlgorithm( os, "Assembly Sequences with CAP3" );

    WorkflowProcessItem* fileList = GTUtilsWorkflowDesigner::getWorker(os, "File List");
    WorkflowProcessItem* fileCAP3 = GTUtilsWorkflowDesigner::getWorker(os, "Assembly Sequences with CAP3");

    GTUtilsWorkflowDesigner::connect(os, fileList, fileCAP3);

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "CAP3"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setParameter(os,"Output file", QDir().absoluteFilePath(sandBoxDir + "out.ace"),GTUtilsWorkflowDesigner::textValue);

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "File List"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/cap3", "region1.fa");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/cap3", "region2.fa");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/cap3", "region3.fa");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/cap3", "region4.fa");

    GTWidget::click(os,GTAction::button(os,"Run workflow"));
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsLog::check(os, l);
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
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_ACTIONS), QStringList() <<  ADV_MENU_EDIT << ACTION_EDIT_INSERT_SUBSEQUENCE, GTGlobals::UseKey);
    GTGlobals::sleep();

    //4. Click OK
    //Expected state: subsequence inserted
    //Bug state: Warning message is shown first

    int sequenceLength = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(sequenceLength == 199957, "Sequence length is " + QString::number(sequenceLength) + ", expected 199957");

    QString sequenceBegin = GTUtilsSequenceView::getBeginOfSequenceAsString(os, 7);
    CHECK_SET_ERR(sequenceBegin == "ATCGTAC", "Sequence starts with <" + sequenceBegin + ">, expected ATCGTAC");

}

GUI_TEST_CLASS_DEFINITION(test_1984){
//    1) Run UGENE
//    2) Open Settings/Preferences/External tools
//    3) Set incorrect path for any external tool (cistrome toolkit, cufflinks toolkit, samtools toolkit, Rscript with its modules, python, perl, tophat)
//    4) Press OK
//    Expected state: UGENE doesn't show any warning to user. Error should be at UGENE log ("Details" columns should be enabled)

    class CuffDiffIncorrectPath : public CustomScenario {
    public:
        CuffDiffIncorrectPath() {}
        virtual void run(U2::U2OpStatus &os) {
            AppSettingsDialogFiller::setExternalToolPath(os, "Cuffdiff", "./");
            GTGlobals::sleep(2000);

            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new CuffDiffIncorrectPath()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action__settings"));
    GTMenu::showMainMenu(os, MWMENU_SETTINGS);

    CHECK_SET_ERR(l.checkMessage("Cuffdiff validate task failed: Tool does not start."), "No error in the log!");
}

GUI_TEST_CLASS_DEFINITION(test_1986){
//1. Run UGENE
//2. Use main toolbar { File -> Search NCBI Genbank }
    GTUtilsDialog::waitForDialog(os, new NCBISearchDialogFillerDeprecated(os, "human", false,5));

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__SEARCH_GENBANK);
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
    GTUtilsTaskTreeView::waitTaskFinished(os);
    QTreeView* treeView = GTUtilsProjectTreeView::getTreeView(os);
    QAbstractItemModel* model = treeView->model();
    QString text = model->data(model->index(0,0, QModelIndex()), Qt::DisplayRole).toString();

    CHECK_SET_ERR(text.contains(".fasta"),text);

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
    GTUtilsTaskTreeView::waitTaskFinished(os);
    QTableWidget *idList = qobject_cast<QTableWidget*>(GTWidget::findWidget(os, "idList"));

    GTGlobals::sleep();

    QPoint p1 = idList->mapFromGlobal(GTTableView::getCellPosition(os, idList, 0, 0));
    GTMouseDriver::moveTo(os, GTTableView::getCellPosition(os, idList, 0, 0 ));
    QString value1 = idList->itemAt(p1)->text();
    GTMouseDriver::doubleClick(os);


    // 4. Try to delete value from table
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    //Expected state: value not deleted
    QString value2 = idList->itemAt(p1)->text();
    CHECK_SET_ERR(value1 == value2, "Results differ!");
    GTUtilsTaskTreeView::waitTaskFinished(os);
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
    GTGlobals::sleep(5000);

    //4. Press the "Search" button
    //GTWidget::click(os, GTWidget::findWidget(os, "btnSearch"));

    GTGlobals::sleep(500);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected: UGENE finds the sequence or shows a error message
    CHECK_SET_ERR(l.hasError() == true, "Error message expected in log");
}

GUI_TEST_CLASS_DEFINITION( test_2167 ) {
    GTLogTracer logTracer;

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

    GTUtilsTaskTreeView::waitTaskFinished(os);

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

    GTUtilsTaskTreeView::waitTaskFinished(os);

    // Expected: the table contains only the last results. There are no previous items and additional empty lines.
    QPoint p2 = resultsView->mapFromGlobal(GTTableView::getCellPosition(os, resultsView, 0, 3));
    GTWidget::click(os, resultsView, Qt::LeftButton, p2);

    int count2 = resultsView->model()->rowCount();
    QString value2 = resultsView->itemAt(p2)->text();

    CHECK_SET_ERR(count1 == count2, "There are empty rows!");
    CHECK_SET_ERR(value1 == value2, "Results differ!");

    GTUtilsLog::check(os, logTracer);
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
    GTGlobals::sleep( 2000 );

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
    GTFileDialog::openFile( os, testDir + "_common_data/fasta", "amino_multy.fa");
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
    GTGlobals::sleep(200);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QTreeWidget *treeWidget = GTUtilsAnnotationsTreeView::getTreeWidget(os);
    CHECK_SET_ERR(treeWidget != NULL, "Tree widget is NULL");

    QTreeWidgetItem *annotationsRoot = GTUtilsAnnotationsTreeView::findItem(os, "repeat_unit  (0, 5)");
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

    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os, workflowOutputDir.absolutePath()));
    QMenu* menu = GTMenu::showMainMenu(os, MWMENU_TOOLS);

    GTMenu::clickMenuItemByName(os, menu, QStringList() << "Workflow Designer");

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
    GTGlobals::sleep(500);

    // 4. Shift the region in one base
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 6, 7 ), QPoint( 7, 7 ) );

    // 5. Click in the leftmost column of the selected region
    GTUtilsMSAEditorSequenceArea::click( os, QPoint( 1, 7 ) );
#ifdef Q_OS_MAC
    GTGlobals::sleep(1000);
    GTMouseDriver::click(os);
#endif
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
    Runnable *filler = new NCBISearchDialogFillerDeprecated(os, "rat", true);

    GTUtilsDialog::waitForDialog(os, filler);

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__SEARCH_GENBANK, GTGlobals::UseKey);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION( test_2225_2 ){
    Runnable *filler = new NCBISearchDialogFillerDeprecated(os, "rat", true);

    GTUtilsDialog::waitForDialog(os, filler);

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__SEARCH_GENBANK, GTGlobals::UseKey);
    GTGlobals::sleep();
}
GUI_TEST_CLASS_DEFINITION( test_2259 ){
    MainWindow *mw = AppContext::getMainWindow();
    CHECK_SET_ERR(mw != NULL, "MainWindow is NULL");
    QMainWindow *mainWindow = mw->getQMainWindow();
    CHECK_SET_ERR(mainWindow != NULL, "QMainWindow is NULL");

    QAction *menu = mainWindow->findChild<QAction*>(MWMENU_SETTINGS);
    CHECK_SET_ERR(menu->menu()->actions().size()==2, "wrong numder of actions");

    }

GUI_TEST_CLASS_DEFINITION( test_2266_1 ){
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os, testDir + "_common_data/scenarios/sandbox"));

    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, menu, QStringList() << "Workflow Designer");
    GTUtilsWorkflowDesigner::addSample(os, "call variants");

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read assembly"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/ugenedb/", "Klebsislla.sort.bam.ugenedb");

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read sequence"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/ugenedb/", "Klebsislla_ref.fa");

    //this is the name of output dir. Created here: WorkflowContextCMDLine::createSubDirectoryForRun
    QString baseDirName = QDateTime::currentDateTime().toString("yyyy.MM.dd_hh-mm");
    GTWidget::click(os,GTAction::button(os,"Run workflow"));

    GTUtilsTaskTreeView::waitTaskFinished(os, 6000000);

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/sandbox/" + baseDirName + "/", "variations.vcf");

    GTUtilsProjectTreeView::findIndex(os, "pkF70");
    GTUtilsProjectTreeView::findIndex(os, "pkf140");
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
    Runnable *filler = new EditQualifierFiller(os, "111111111111111111111111111111111111111111111111111111111111111111111111111111111", "val", GTGlobals::UseMouse, true,true);
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
//    0. Copy t-coffee tool to the place where UGENE has enough permissions to change file permissions;
//    Set the copied t-coffee tool in preferences.

    GTGlobals::sleep();
    ExternalToolRegistry* etRegistry = AppContext::getExternalToolRegistry();
    CHECK_SET_ERR(etRegistry, "External tool registry is NULL");
    ExternalTool* tCoffee = etRegistry->getByName("T-Coffee");
    CHECK_SET_ERR(tCoffee, "T-coffee tool is NULL");
    QFileInfo toolPath(tCoffee->getPath());
    CHECK_SET_ERR(toolPath.exists(), "T-coffee tool is not set");

    QDir toolDir = toolPath.dir();
    toolDir.cdUp();
    GTFile::copyDir(os, toolDir.absolutePath(), sandBoxDir + "GUITest_regression_scenarios_test_2268/");

    // Hack, it is better to set the tool path via the preferences dialog
    toolPath.setFile(sandBoxDir + "GUITest_regression_scenarios_test_2268/bin/t_coffee");
    CHECK_SET_ERR(toolPath.exists(), "The copied T-coffee tool does not exist");
    tCoffee->setPath(toolPath.absoluteFilePath());

    toolDir = toolPath.dir();
    toolDir.cdUp();

//    1. Forbid write access to the t-coffee directory (chmod 555 %t-coffee-dir%).
    // Permissions will be returned to the original state, if UGENE won't crash.

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
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));

//    Expected: the t-coffee task started and finished well.
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    CHECK_SET_ERR(scheduler, "Task scheduler is NULL");
    GTGlobals::sleep(5000);
    while(!scheduler->getTopLevelTasks().isEmpty()){
       GTGlobals::sleep();
    }

    GTUtilsLog::check(os, lt);
}

GUI_TEST_CLASS_DEFINITION( test_2316 ) {
    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new StartupDialogFiller(os));
    GTFileDialog::openFile(os, dataDir+"samples/../workflow_samples/Alignment", "basic_align.uwl");

    GTGlobals::sleep(5000);

    CHECK_SET_ERR(!GTUtilsProjectTreeView::isVisible(os), "Project tree view is visible");

//     1. Start UGENE
//
//     2. Drag an .uwl file inside UGENE to open a workflow
//     Expected state: now the project view is collapsed if a file.uwl is drag'n'dropped to UGENE when there is no project yet.
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
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Align to reference" << "Align short reads");

    CHECK_SET_ERR( !os.hasError(), "Uncorrect value is available");
}

GUI_TEST_CLASS_DEFINITION( test_2270 ){
    //1. Open file "data/cmdline/snp.uwl"
    //Ecpected state: scheme opened in WD without problems
    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new StartupDialogFiller(os));
    GTLogTracer lt;
    GTFileDialog::openFile(os, dataDir+"cmdline/", "snp.uwl");
    GTGlobals::sleep(500);
    GTUtilsLog::check(os, lt);
}

GUI_TEST_CLASS_DEFINITION( test_2281 ){
    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new StartupDialogFiller(os));
    //1. Open WD
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, menu, QStringList() << "Workflow Designer");
    QTabWidget* tabs = qobject_cast<QTabWidget*>(GTWidget::findWidget(os,"tabs"));
    CHECK_SET_ERR(tabs!=NULL, "tabs widget not found");

    //2. Click the "samples" bar. The samples hint is shown
    GTTabWidget::setCurrentIndex(os,tabs,1);
    GTGlobals::sleep(500);
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));

    QPixmap pixmap = QPixmap::grabWidget(sceneView, sceneView->rect());
    QImage img = pixmap.toImage();

    bool found = false;
    for(int i=sceneView->rect().left(); i< sceneView->rect().right(); i+=10){
        for(int j=sceneView->rect().top(); j< sceneView->rect().bottom(); j+=10){
            QRgb rgb = img.pixel(QPoint(i,j));
            QColor c(rgb);
            QColor yc = QColor(255,255,160);
            if (c == yc){
                found = true;
                break;
            }
        }
    }
    CHECK_SET_ERR(found, "hint not found");

    //3. Click the "elements" bar.
    GTTabWidget::setCurrentIndex(os,tabs,0);
    GTGlobals::sleep(500);

    //Expected: the samples hint is hidden
    pixmap = QPixmap::grabWidget(sceneView, sceneView->rect());
    img = pixmap.toImage();
    bool notFound = true;
    for(int i=sceneView->rect().left(); i< sceneView->rect().right(); i+=10){
        for(int j=sceneView->rect().top(); j< sceneView->rect().bottom(); j+=10){
            QRgb rgb = img.pixel(QPoint(i,j));
            QColor c(rgb);
            QColor yc = QColor(255,255,160);
            if (c == yc){
                notFound = false;
                break;
            }
        }
    }

    CHECK_SET_ERR(notFound, "hint is found");
    GTGlobals::sleep(1000);
}

GUI_TEST_CLASS_DEFINITION( test_2292 ){
    GTLogTracer l;

    QString destName = testDir + "_common_data/ugenedb/example-alignment.ugenedb";
    GTUtilsDialog::waitForDialog(os, new ConvertAceToSqliteDialogFiller(os, destName));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "There is opened view with destination file"));
    GTFileDialog::openFile(os, testDir+"_common_data/ugenedb/", "example-alignment.ugenedb");
    GTUtilsDialog::waitForDialog(os, new DocumentProviderSelectorDialogFiller
                                 (os, DocumentProviderSelectorDialogFiller::AssemblyBrowser));
    GTFileDialog::openFile(os, dataDir+"samples/ACE", "K26.ace");


    GTGlobals::sleep();
    //CHECK_SET_ERR( l.hasError( ), "Error message expected!" );
}

GUI_TEST_CLASS_DEFINITION( test_2298 ){
//    1. Open the file "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//    2. Build the tree and make it view together with msa
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/2298.nwk", 0, 0, true));
    QAbstractButton *tree = GTAction::button(os,"Build Tree");
    GTWidget::click(os, tree);
    GTGlobals::sleep();
//    3. Collapse any node on the tree
    QGraphicsItem* node = GTUtilsPhyTree::getNodes(os).at(1);
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    treeView->ensureVisible(node);
    GTMouseDriver::moveTo(os, GTUtilsPhyTree::getGlobalCoord(os, node));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
//    Expected state: the appropriate sequences on the msa has collapsed into a group as well
    QStringList l = GTUtilsMSAEditorSequenceArea::getVisibaleNames(os);
    int num = l.count();
    CHECK_SET_ERR(num == 3, QString("Unexpected visiable sequences number. Expected: 3, actual: %1").arg(num));
    GTGlobals::sleep();
}


GUI_TEST_CLASS_DEFINITION( test_2293 ){
//    0. Ensure that Bowtie2 Build index tool is not set. Remove it, if it is.
//    1. Do {main menu -> Tools -> ALign to reference -> Build index}.
//    Expected state: a "Build index" dialog appeared.
//    2. Fill the dialog:
//        {Align short reads method}: Bowtie2
//        {Reference sequence}:       data/samples/FASTA/human_T1.fa
//        {Index file name}:          set any valid data or use default
//    Click a "Start" button.

//    Expected state: a message box appeared: an offer to set "Bowtie2 Build index" tool in UGENE preferences.
    GTUtilsExternalTools::removeTool(os, "Bowtie 2");

    class CheckBowtie2Filler : public Filler {
    public:
        CheckBowtie2Filler(U2OpStatus &os)
            : Filler (os, "BuildIndexFromRefDialog") {}
        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");

            QComboBox* methodNamesBox = dialog->findChild<QComboBox*>("methodNamesBox");
            for(int i=0; i < methodNamesBox->count();i++){
                if(methodNamesBox->itemText(i) == "Bowtie2"){
                    GTComboBox::setCurrentIndex(os, methodNamesBox, i);
                }
            }

            GTFileDialogUtils *ob = new GTFileDialogUtils(os, dataDir + "samples/FASTA/", "human_T1.fa");
            GTUtilsDialog::waitForDialog(os, ob);
            GTWidget::click(os, GTWidget::findWidget(os, "addRefButton",dialog));

            QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
            CHECK_SET_ERR(box != NULL, "buttonBox is NULL");

            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "No"));
            QPushButton* okButton = box->button(QDialogButtonBox::Ok);
            CHECK_SET_ERR(okButton !=NULL, "ok button is NULL");
            GTWidget::click(os, okButton);

            QPushButton* cancelButton = box->button(QDialogButtonBox::Cancel);
            CHECK_SET_ERR(cancelButton !=NULL, "Cancel button is NULL");
            GTWidget::click(os, cancelButton);
        }
    };


    GTUtilsDialog::waitForDialog(os, new CheckBowtie2Filler(os));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Align to reference" << "Build index");
}

GUI_TEST_CLASS_DEFINITION( test_2282 ) {
    // 1. Open "chrM.sorted.bam" file using {File->Open} menu in UGENE.
    //    Expected state: "Import BAM file" dialog has appeared.
    // 2. Press "Enter".
    //    Expected state:
    //      1) The dialog has been closed.
    //      2) A new project has been created.
    QString assFileName = testDir + "_common_data/scenarios/sandbox/test_2282.chrM.sorted.ugenedb";
    QString assDocName = "test_2282.chrM.sorted.ugenedb";
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, assFileName));
    GTFileDialog::openFile(os, testDir + "_common_data/bam", "chrM.sorted.bam");
    GTGlobals::sleep(5000);

    //      3) The Project View with document "chrM.sorted.bam.ugenedb" has been opened.
    GTUtilsProjectTreeView::findIndex(os, assDocName);

    // 3. Delete "chrM.sorted.bam.ugenedb" from the file system (i.e. not from UGENE).
    bool deleteResult = QFile::remove(assFileName);
    CHECK(true == deleteResult, );
    GTGlobals::sleep();

    // Expected state: the project has been removed.
    GTUtilsProject::checkProject(os, GTUtilsProject::NotExists);
}

GUI_TEST_CLASS_DEFINITION( test_2284 ){
    //1. Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    //2. Press the "Switch on/off collapsing" button
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Enable collapsing"));

    //expected state: Mecopoda_elongata__Ishigaki__J and Mecopoda_elongata__Sumatra_ are collapsed
    CHECK_SET_ERR( !GTUtilsMSAEditorSequenceArea::isSequenceVisible(os, QString("Mecopoda_elongata__Sumatra_")),
        "Required sequence is not collapsed");

    //3. Select the first base in last row
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint(0, 16 ), QPoint(0, 16 ) );

    //4. In status bar search field type "AATT"
    GTKeyboardDriver::keyClick( os, 'f', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    GTKeyboardDriver::keySequence(os, "AATT");

    //5. Press the "Find forward" button a few times until selection reaches the end of the alignment
    //6. Press the button again
    for(int i = 0; i < 11; i++) {
        GTGlobals::sleep(200);
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    }
}

GUI_TEST_CLASS_DEFINITION( test_2285 ){
//    1. Open {data/samples/CLUSTALW/COI.aln}
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

//    2. Click the "Switch on/off collapsing" button on the toolbar.
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Enable collapsing"));
//    Expected state: Collapsed mode is switched on, there are one collapsed group.
    int visableNamesNum = GTUtilsMSAEditorSequenceArea::getVisibaleNames(os).count();
    CHECK_SET_ERR(visableNamesNum == 17,
                  QString("unexpected visable sequences number. Expected 17, actual: %1").arg(visableNamesNum));
//    3. Expand the collapsed group ("Mecopoda_elongata__Ishigaki__J" is the head sequence).
    GTUtilsMSAEditorSequenceArea::clickCollapceTriangle(os, "Mecopoda_elongata__Ishigaki__J");
    GTGlobals::sleep(1000);

//    4. Set the cursor to the 14 line (the "Mecopoda_elongata__Ishigaki__J" sequence), 45 base.

    QWidget *documentTreeWidget = GTWidget::findWidget(os, GTUtilsProjectTreeView::widgetName, NULL, GTGlobals::FindOptions(false));
    if (documentTreeWidget != NULL) {
        GTUtilsProjectTreeView::toggleView(os);
    }
    GTGlobals::sleep();
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(44, 13));
//    5. Press Backspace.
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["back"]);

//    Expected state: each sequence in the group loose one gap.
    GTUtilsMSAEditorSequenceArea::click(os);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(43, 13), QPoint(43, 14));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    QString clipboardText = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardText == "T\nT", "Unexpected selection. Expected: T\nT, actual: " + clipboardText);

}


GUI_TEST_CLASS_DEFINITION( test_2306 ) {
    // 1. Open file "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");

    // 2. Turn on collapsing mode in MSA
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Enable collapsing"));

    //expected state: Mecopoda_elongata__Ishigaki__J and Mecopoda_elongata__Sumatra_ are collapsed
    CHECK_SET_ERR( !GTUtilsMSAEditorSequenceArea::isSequenceVisible(os, QString("Mecopoda_elongata__Sumatra_")),
        "Required sequence is not collapsed");

    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 5, 10 ), QPoint( 15, 15 ) );

    // 3. Shift the region but don't release left mouse button
    const QPoint mouseDragPosition( 12, 12 );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseDragPosition );
    GTMouseDriver::press( os );
    GTUtilsMSAEditorSequenceArea::moveTo( os, mouseDragPosition + QPoint( 3, 0 ) );
    GTGlobals::sleep( 200 );
    GTMouseDriver::release( os );
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);
    GTGlobals::sleep( 200 );

    // 4. Call context menu
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 5, 10 ), QPoint( 7, 15 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep( 200 );
    const QString finalMsaContent = GTClipboard::text( os );
    CHECK_SET_ERR("---\n---\n---\n---\n---\n---\n---" == finalMsaContent, "Unexpected MSA content has occurred" );



    }
GUI_TEST_CLASS_DEFINITION( test_2309 ) {
    // 1. Open file "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");

    // 2. Build tree for the alignment
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/2309.nwk", 0, 0, true));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os, tree);
    GTGlobals::sleep(500);

    QStringList initialNames = GTUtilsMSAEditorSequenceArea::getNameList(os);
    QAbstractButton *refresh= GTAction::button(os,"Refresh tree");

    CHECK(NULL != refresh, );
    if(refresh->isVisible()){
        GTWidget::click(os, refresh);
    }else{
        GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Refresh tree"));
        QToolBar* toolBar = qobject_cast<QToolBar*>(refresh->parent());
        GTMouseDriver::moveTo(os, toolBar->mapToGlobal(toolBar->geometry().bottomRight())-QPoint(5,15));
        GTMouseDriver::click(os);
    }

    GTGlobals::sleep(500);

    QStringList newNames = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(newNames == initialNames, "Wrong sequences order");

    int key;
#ifdef Q_OS_MAC
    key = GTKeyboardDriver::key["cmd"];
#else
    key = GTKeyboardDriver::key["ctrl"];
#endif
        GTGlobals::sleep();
        GTWidget::click(os, GTUtilsProjectTreeView::getTreeView(os));
        GTKeyboardDriver::keyClick(os, 'a', key);
        GTGlobals::sleep(100);

        GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
        GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::NoToAll));
        GTGlobals::sleep(200);
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
        GTGlobals::sleep(100);
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
    QWidget* farButton = GTToolbar::getWidgetForActionTooltip(os, toolbar, "Find annotated regions");
    CHECK_SET_ERR(farButton, "Find annotated region button is NULL");

    // Expected state is checked in PlusClicker
    GTUtilsDialog::waitForDialog(os, new PlusClicker(os, "CDS"));
    GTWidget::click(os, farButton);
}

GUI_TEST_CLASS_DEFINITION( test_2351 ) {
//    1. Run UGENE
//    2. Press the "New Project" button on the main toolbar
//    Expected state: the "Create new project" dialog has appeared
//    3. Press the "Create" button or press the "Enter" key
//    4. Repeat steps 2 and 3 as fast as you can more than 10 times
//    Expected state: UGENE does not crash
    class RapidProjectCreator : public Filler {
    public:
        RapidProjectCreator(U2OpStatus& os, const QString& _projectName, const QString& _projectFolder, const QString& _projectFile) :
            Filler(os, "CreateNewProjectDialog"),
            projectName(_projectName),
            projectFolder(_projectFolder),
            projectFile(_projectFile) {}

        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            if (NULL == dialog) {
                os.setError("Modal dialog not found");
                return;
            }

            QLineEdit *projectNameEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectNameEdit", dialog));
            if (NULL == projectNameEdit) {
                os.setError("projectNameEdit not found");
                return;
            }
            projectNameEdit->setText(projectName);

            QLineEdit *projectFolderEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectFolderEdit", dialog));
            if (NULL == projectFolderEdit) {
                os.setError("projectFolderEdit not found");
                return;
            }
            projectFolderEdit->setText(projectFolder);

            QLineEdit *projectFileEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectFileEdit", dialog));
            if (NULL == projectFileEdit) {
                os.setError("projectFileEdit not found");
                return;
            }
            projectFileEdit->setText(projectFile);

            QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
            CHECK_SET_ERR(box != NULL, "buttonBox is NULL");
            QPushButton* button = box->button(QDialogButtonBox::Ok);
            CHECK_SET_ERR(button !=NULL, "ok button is NULL");
            GTWidget::click(os, button);
        }

    private:
        const QString projectName;
        const QString projectFolder;
        const QString projectFile;
    };

    const QString projectName = "test_2351";
    const QString projectFolder = testDir + "_common_data/scenarios/sandbox";
    const QString projectFile = "test_2351";

    for (int i = 0; i < 15; ++i) {
        GTUtilsDialog::waitForDialog(os, new RapidProjectCreator(os, projectName, projectFolder, projectFile));
        GTWidget::click(os,
                        GTToolbar::getWidgetForActionName(os,
                                                          GTToolbar::getToolbar(os, MWTOOLBAR_MAIN),
                                                          ACTION_PROJECTSUPPORT__NEW_PROJECT));
    }
}

GUI_TEST_CLASS_DEFINITION( test_2352 ) {
    //1. Close current project or open empty UGENE
    //2. Open menu {Tools->Build dotplot...}
    //3. Select any binary file as first file in dialog
    //Expected state: file is not selected, no crash

    QString bin = QCoreApplication::applicationFilePath();
    GTUtilsDialog::waitForDialog(os, new BuildDotPlotFiller(os, bin, bin, false, false, false, 5, 5,
                                                            true));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));

    QMenu *menu=GTMenu::showMainMenu( os, MWMENU_TOOLS );
    GTMenu::clickMenuItemByName( os, menu, QStringList( ) << "Build dotplot" );

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION( test_2360 ) {
    // 1. Open "data/samples/COI.aln".
    GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");

    // 2. Right click on document in project.
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI.aln"));

    // 3. Choose the context menu {Export/Import->Export nucleic alignment to amino translation}.
    // Expected state: Export dialog appears.
    // 4. Set "File format to use" to PHYLIP Sequantial.
    // 5. Click "Export".
    GTUtilsDialog::waitForDialog(os, new ExportMSA2MSADialogFiller(os, 6));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_PROJECT__EXPORT_TO_AMINO_ACTION));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION( test_2364 ) {
    //1. Open WD.'
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Create a workflow: Read sequence -> Write sequence.
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Read sequence");
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Write sequence");

    GTUtilsWorkflowDesigner::connect(os, GTUtilsWorkflowDesigner::getWorker(os, "Read sequence"),
                                         GTUtilsWorkflowDesigner::getWorker(os, "Write sequence"));

    //3. Set the input sequence file: "data/samples/FASTA/human_T1.fa".
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read sequence"));
    GTMouseDriver::click(os);
    GTGlobals::sleep(300);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    //4. Set the output file: "out.fa".
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Write sequence"));
    GTMouseDriver::click(os);
    GTGlobals::sleep(300);
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", "out.fa", GTUtilsWorkflowDesigner::textValue);

    //5. Validate the workflow.
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTWidget::click(os, GTAction::button(os,"Validate workflow"));
    GTGlobals::sleep(1000);

    //Expected: the workflow has the warning about FASTA format and annotations.
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::checkErrorList(os, "") != 0, "There is no any messages in infoLog");

    //6. Run the workflow.
    GTWidget::click(os,GTAction::button(os,"Run workflow"));
    GTGlobals::sleep(500);

    //Expected: the button "Load schema" is shown.
    // expected button is in dashboard - it can't be checked for now
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

    GTUtilsDialog::waitForDialog( os, new StartupDialogFiller( os ) );

    QMenu *menu=GTMenu::showMainMenu( os, MWMENU_TOOLS );
    GTMenu::clickMenuItemByName( os, menu, QStringList( ) << "Workflow Designer" );

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
    GTUtilsWorkflowDesigner::setParameter( os, "Output file", "test",
        GTUtilsWorkflowDesigner::textValue );

    GTWidget::click( os, GTAction::button( os, "Run workflow" ) );

    GTGlobals::sleep(5000);
    CHECK_SET_ERR( l.hasError( ), "Error message expected!" );
}

GUI_TEST_CLASS_DEFINITION( test_2378 ) {
    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, testDir + "_common_data/scenarios/sandbox/test_2378.ugenedb"));
    GTFileDialog::openFile(os, testDir + "_common_data/sam/", "scerevisiae.sam");
    CHECK_SET_ERR(!l.hasError( ), "Error message");
}

GUI_TEST_CLASS_DEFINITION( test_2378_1 ) {
    GTLogTracer l;
    // 1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    // 2. Create scheme: read assembly->write assembly
    // 3. set _common_data\sam\scerevisiae.sam as input file
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Read Assembly");
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read Assembly"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/sam", "scerevisiae.sam");

    GTUtilsWorkflowDesigner::addAlgorithm(os, "Write Assembly");
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Write Assembly"));
    GTMouseDriver::click(os);
    //QString //absPath =
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", QDir(testDir).absolutePath() + "_common_data/scenarios/sandbox/test_2378_1.bam", GTUtilsWorkflowDesigner::textValue);

    GTUtilsWorkflowDesigner::connect(os, GTUtilsWorkflowDesigner::getWorker(os, "Read Assembly"), GTUtilsWorkflowDesigner::getWorker(os, "Write Assembly"));

    // 4. Run scheme
    GTWidget::click( os, GTAction::button( os, "Run workflow" ) );
    GTGlobals::sleep(5000);
    CHECK_SET_ERR(!l.hasError( ), "Error message");
}

GUI_TEST_CLASS_DEFINITION( test_2379 ) {
    class CreateProjectFiller : public Filler {
        // It is a local support class, it is the same as SaveProjectAsDialogFiller,
        // but it clicks the final button with keyboard.
        // I know that it is bad practice to create so useless classes, but I don't need to extend the original class.
        // Do not move it to another place: if you need the same filler than extand the original class.
    public:
        CreateProjectFiller(U2OpStatus &_os,
                            const QString &_projectName,
                            const QString &_projectFolder,
                            const QString &_projectFile) :
            Filler(_os, "CreateNewProjectDialog"),
            projectName(_projectName),
            projectFolder(_projectFolder),
            projectFile(_projectFile) {}

        virtual void run() {
            GTGlobals::sleep();
            QWidget* dialog = QApplication::activeModalWidget();
            if (NULL == dialog) {
                os.setError("activeModalWidget is NULL");
                return;
            }

            QLineEdit *projectNameEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectNameEdit", dialog));
            GTLineEdit::setText(os, projectNameEdit, projectName);

            QLineEdit *projectFolderEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectFolderEdit", dialog));
            GTLineEdit::setText(os, projectFolderEdit, projectFolder);

            QLineEdit *projectFileEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectFileEdit", dialog));
            GTLineEdit::setText(os, projectFileEdit, projectFile);

            GTGlobals::sleep();
#ifdef Q_OS_MACX
            GTWidget::click(os, GTWidget::findButtonByText(os, "Create", dialog));
#else
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
#endif
            GTGlobals::sleep();
        }

    private:
        const QString projectName;
        const QString projectFolder;
        const QString projectFile;
    };

//    0. Create a project that will be "existing" in the second step
    const QString projectName = "test_2379";
    const QString projectFolder = testDir + "_common_data/scenarios/sandbox";
    const QString projectFile = "test_2379";

    GTUtilsDialog::waitForDialog(os, new CreateProjectFiller(os, projectName, projectFolder, projectFile));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__NEW_PROJECT);
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__SAVE_PROJECT);
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__CLOSE_PROJECT);

//    1. Press "Create new project" button
//    2. Specify the path to an existing project
//    3. Press "Create" button by using keyboard
//    Expected state: only one dialog with warning message appeared
    GTUtilsDialog::waitForDialog(os, new CreateProjectFiller(os, projectName, projectFolder, projectFile));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes, "Project file already exists"));

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__NEW_PROJECT);
}

GUI_TEST_CLASS_DEFINITION( test_2382 ) {
    // 1. Open "_common_data/ace/capres4.ace".
    // 2. "Import ACE file" dialog appeared. Press ok.
    // Expected state: export succeeded, assembly is displayed.
    GTLogTracer l;

    QString sandboxDir = testDir + "_common_data/scenarios/sandbox/";
    QString assDocName = "test_2382.ugenedb";
    GTUtilsDialog::waitForDialog(os, new DocumentProviderSelectorDialogFiller
                                 (os, DocumentProviderSelectorDialogFiller::AssemblyBrowser));
    GTUtilsDialog::waitForDialog(os, new ConvertAceToSqliteDialogFiller(os, sandboxDir + assDocName));
    GTFileDialog::openFile(os, testDir + "_common_data/ace/", "capres4.ace");

    GTGlobals::sleep();
    CHECK_SET_ERR( !l.hasError( ), "There must be no errors!" );

    GTUtilsProjectTreeView::findIndex(os, assDocName);
}

GUI_TEST_CLASS_DEFINITION( test_2382_1 ) {
    // 1. Open "_common_data/ace/test_new_.cap.ace".
    // 2. "Import ACE file" dialog appeared. Press ok.
    // Expected state: export succeeded, assembly is displayed.
    GTLogTracer l;

    QString sandboxDir = testDir + "_common_data/scenarios/sandbox/";
    QString assDocName = "test_2382_1.ugenedb";
    GTUtilsDialog::waitForDialog(os, new DocumentProviderSelectorDialogFiller
                                 (os, DocumentProviderSelectorDialogFiller::AssemblyBrowser));
    GTUtilsDialog::waitForDialog(os, new ConvertAceToSqliteDialogFiller(os, sandboxDir + assDocName));
    GTFileDialog::openFile(os, testDir + "_common_data/ace/", "test_new.cap.ace");

    GTGlobals::sleep();
    CHECK_SET_ERR( !l.hasError( ), "There must be no errors!" );

    GTUtilsProjectTreeView::findIndex(os, assDocName);
}

GUI_TEST_CLASS_DEFINITION( test_2387 ) {
    //1) Click Open button in UGENE
    //2) Select any two valid files, for example data/samples/Genbank/ PBR322.gb and sars.gb
    //3) Click Open
    //Expected state: MultipleDocumentsReadingModeDialog appears
    //4) Select merge option
    //5) Replace one of the files with any binary file
    //6) Click OK in MultipleDocumentsReadingModeDialog.
    //Expected state: UGENE not crashed

    QString sandbox = testDir + "_common_data/scenarios/sandbox/";

    GTFile::copy(os, dataDir + "samples/Genbank/PBR322.gb", sandbox + "PBR322.gb");
    GTFile::copy(os, dataDir + "samples/Genbank/sars.gb", sandbox + "sars.gb");

    class SequenceReadingModeDialogUtils : public GTSequenceReadingModeDialogUtils {
    public:
        SequenceReadingModeDialogUtils(U2OpStatus& _os) : GTSequenceReadingModeDialogUtils(_os){}
        virtual void run() {
            GTSequenceReadingModeDialog::mode = GTSequenceReadingModeDialog::Merge;
            GTFile::copy(os, testDir + "_common_data/scenarios/_regression/2387/binary.dll", testDir + "_common_data/scenarios/sandbox/sars.gb");
            GTSequenceReadingModeDialogUtils::run();
        }
    };

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeDialogUtils(os));
    GTFileDialog::openFileList(os, sandbox, QStringList() << "PBR322.gb" << "sars.gb");
}

GUI_TEST_CLASS_DEFINITION( test_2392 ) {
    // 1. Open file _common_data/genbank/multi.gb
    // Expected state: Open dialog "Sequence reading options"
    // 2. Select "Join sequences into alignment" option and press OK
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Join));
    GTFileDialog::openFile(os, testDir+"_common_data/genbank/", "multi.gb");

    // Expected state: Document opened in MSA view
    QStringList names = GTUtilsMSAEditorSequenceArea::getNameList(os);
    CHECK_SET_ERR(2 == names.size(), "Wrong sequence count");
}

GUI_TEST_CLASS_DEFINITION( test_2401 ) {
    // 1. Open the file "_common_data/ace/ace_test_1.ace".
    // 2. Set the ugenedb path for import: "_common_data/scenarios/sandbox/2401.ugenedb".
    // 3. Click OK
    QString sandbox = testDir + "_common_data/scenarios/sandbox/";
    QString fileName = "2401.ugenedb";
    QString ugenedb = sandbox + fileName;
    GTUtilsDialog::waitForDialog(os, new DocumentProviderSelectorDialogFiller
                                 (os, DocumentProviderSelectorDialogFiller::AssemblyBrowser));
    GTUtilsDialog::waitForDialog(os, new ConvertAceToSqliteDialogFiller(os, ugenedb));
    GTFileDialog::openFile(os, testDir + "_common_data/ace/", "ace_test_1.ace");

    // Expected: the file is imported without errors, the assembly is opened.
    // 4. Close the project.
    GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
#ifdef Q_OS_MAC
        QMenu *menu = GTMenu::showMainMenu(os, MWMENU_FILE);
        GTMenu::clickMenuItem(os, menu, ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
#else
    GTKeyboardDriver::keyClick(os, 'q', GTKeyboardDriver::key["ctrl"]);
#endif
    GTGlobals::sleep();

    // 5. Open the file "_common_data/ace/ace_test_11_(error).ace".
    // 6. Set the same ugenedb path for import: "_common_data/scenarios/sandbox/2401.ugenedb".
    // 7. Click OK.
    // 8. Click Append.
    GTUtilsDialog::waitForDialog(os, new DocumentProviderSelectorDialogFiller
                                 (os, DocumentProviderSelectorDialogFiller::AssemblyBrowser));
    GTUtilsDialog::waitForDialog(os, new ConvertAceToSqliteDialogFiller(os, ugenedb, ConvertAceToSqliteDialogFiller::APPEND));
    GTFileDialog::openFile(os, testDir + "_common_data/ace/", "ace_test_11_(error).ace");

    // Expected: the file is not imported but "2401.ugenedb" still exists.
    QString s = GUrl(ugenedb).getURLString();
    CHECK_SET_ERR(QFile::exists(s), "ugenedb file does not exist");

    // 9. Open the file "2401.ugenedb".
    GTFileDialog::openFile(os, sandbox, fileName);
    // Expected: assembly is opened without errors.
    GTUtilsProjectTreeView::findIndex(os, fileName);
}

GUI_TEST_CLASS_DEFINITION(test_2402) {
    QDir outputDir(testDir + "_common_data/scenarios/sandbox");
    const QString outputFilePath = outputDir.absolutePath( ) + "/test_2402.gb";

    // 1.Open Workflow Designer
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    // 2.Add 'Read Sequence' element
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Read Sequence");
    // 3. Add 'Assembly Sequences witn CAP3' element
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Assembly Sequences with CAP3");
    WorkflowProcessItem *readWorker = GTUtilsWorkflowDesigner::getWorker(os, "Read Sequence");
    WorkflowProcessItem *assemblyWorker = GTUtilsWorkflowDesigner::getWorker(os, "Assembly Sequences with CAP3");
    // 4. Connect elements
    GTUtilsWorkflowDesigner::connect(os, readWorker, assemblyWorker);

    //Expected state: all slots are connected
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read Sequence"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Genbank", "sars.gb");
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Assembly Sequences with CAP3"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", outputFilePath, GTUtilsWorkflowDesigner::textValue);

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "Well done!"));
    GTWidget::click( os,GTAction::button( os,"Validate workflow" ) );
}
GUI_TEST_CLASS_DEFINITION( test_2404 ) {
/*  1. Open human_T1.fa
    2. Open Search in sequence OP tab
    3. Input "AAAAA" pattern to the Search for: field
    4. Expand all available parameters
    5. Resize the main UGENE window
    Expected: scrollbar appears
    Current: layout breaks
*/
    GTFileDialog::openFile( os, dataDir + "samples/FASTA/", "human_T1.fa" );
    GTGlobals::sleep(500);
    GTUtilsOptionsPanel::runFindPatternWithHotKey("AAAAA", os);
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search algorithm"));
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search in"));
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Other settings"));
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Save annotation(s) to"));
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Annotation parameters"));
    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    mw->showNormal();
    mw->resize(800,600);
    GTGlobals::sleep();
    QScrollArea* sa = qobject_cast<QScrollArea*>(GTWidget::findWidget( os, "OP_SCROLL_AREA" ));
    QScrollBar* scroll = sa->verticalScrollBar();
    CHECK_SET_ERR( scroll != NULL, "Scroll bar is NULL");
    CHECK_SET_ERR( scroll->isVisible(), "Scroll bar is visible!");
}
GUI_TEST_CLASS_DEFINITION(test_2403) {
    //1. Open "human_T1.fa".
    //    2. Resize the main UGENE window that not all buttons will be visible on the sequence toolbar.
    //    3. Click on the "Show full toolbar" button.
    //Expected: all toolbar actions appears.

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa" );
    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    mw->resize(300, mw->size().height());

    QWidget *toolbarWidget = GTWidget::findWidget(os, "mwtoolbar_activemdi");
    CHECK_SET_ERR( NULL != toolbarWidget, "Toolbar  is not present");
    QWidget *expandWidget = GTWidget::findWidget(os, "qt_toolbar_ext_button", toolbarWidget);
    CHECK_SET_ERR( NULL != expandWidget, "\"Show full toolbar\" button  is not present");

    GTWidget::click( os, expandWidget);
    GTGlobals::sleep();
    QWidget *toggleWidget = GTWidget::findWidget(os, "toggleViewButton", toolbarWidget);
    CHECK_SET_ERR( NULL != toggleWidget, "\"Toggle view\" button  is not present");
}

GUI_TEST_CLASS_DEFINITION( test_2406 ) {
//    1. Create the {Read Sequence -> Write Sequence} workflow.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    const QString sequenceReaderName = "Read Sequence";
    const QString sequenceWriterName = "Write Sequence";

    GTUtilsWorkflowDesigner::addAlgorithm(os, sequenceReaderName);
    GTUtilsWorkflowDesigner::addAlgorithm(os, sequenceWriterName);

    WorkflowProcessItem *sequenceReader = GTUtilsWorkflowDesigner::getWorker(os, sequenceReaderName);
    WorkflowProcessItem *sequenceWriter = GTUtilsWorkflowDesigner::getWorker(os, sequenceWriterName);

    CHECK_SET_ERR(NULL != sequenceReader, "Sequence reader element is NULL");
    CHECK_SET_ERR(NULL != sequenceWriter, "Sequence writer element is NULL");

    GTUtilsWorkflowDesigner::connect(os, sequenceReader, sequenceWriter);

//    2. Click on the output file field and input "TEST" file name and press Enter.
//    Expected: TEST file name appears in the output file name field
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, sequenceWriterName));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", "TEST", GTUtilsWorkflowDesigner::textValue);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);

    GTGlobals::sleep(500);
    const QString expectedPreValue = "TEST";
    const QString resultPreValue = GTUtilsWorkflowDesigner::getParameter(os, "Output file");
    CHECK_SET_ERR(expectedPreValue == resultPreValue,
                  QString("Unexpected value: expected '%1' get '%2'").
                  arg(expectedPreValue).arg(resultPreValue));

//    3. Change the file format to the genbank
//    Expected: TEST.gb file name appears in the output file name field
    GTUtilsWorkflowDesigner::setParameter(os, "Document format", 2 /*"genbank"*/, GTUtilsWorkflowDesigner::comboValue);

    const QString expectedPostValue = "TEST.gb";
    const QString resultPostValue = GTUtilsWorkflowDesigner::getParameter(os, "Output file");
    CHECK_SET_ERR(expectedPostValue == resultPostValue,
                  QString("Unexpected value: expected '%1' get '%2'").
                  arg(expectedPostValue).arg(resultPostValue));
}

GUI_TEST_CLASS_DEFINITION(test_2407) {
    GTLogTracer l;
    GTFileDialog::openFile(os, testDir+"_common_data/clustal/", "10000_sequences.aln");

    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "10000_sequences.aln"));

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ACTION_PROJECT__UNLOAD_SELECTED));
    GTMouseDriver::click(os, Qt::RightButton);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "10000_sequences.aln"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep(500);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    CHECK_SET_ERR( !l.hasError( ), "File not removed from project!" );

    }

GUI_TEST_CLASS_DEFINITION( test_2410 ) {
    GTFileDialog::openFile( os, dataDir + "samples/FASTA/", "human_T1.fa" );

    GTGlobals::sleep( );

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller( os, 166740, 166755 ) );

    QWidget *sequenceWidget = GTWidget::findWidget( os, "ADV_single_sequence_widget_0" );
    CHECK_SET_ERR( NULL != sequenceWidget, "sequenceWidget is not present" );

    GTWidget::click( os, sequenceWidget );
    GTKeyboardUtils::selectAll( os );

    QWidget *graphAction = GTWidget::findWidget( os, "GraphMenuAction", sequenceWidget, false );
    Runnable *chooser = new PopupChooser( os, QStringList( ) << "GC Content (%)" );
    GTUtilsDialog::waitForDialog( os, chooser );

    GTWidget::click( os, graphAction );

    GTWidget::click( os, GTAction::button( os,
        "action_zoom_in_human_T1 (UCSC April 2002 chr7:115977709-117855134)" ) );

    QWidget *renderArea = GTWidget::findWidget( os, "GSequenceGraphViewRenderArea", sequenceWidget );
    const QPoint mouseInitialPos( 4 * renderArea->width( ) / 7, renderArea->height( ) / 2 );
    GTWidget::click(os, renderArea, Qt::LeftButton, mouseInitialPos );
    GTGlobals::sleep( 200 );

    const QPoint mouseInitialAbsPos = GTMouseDriver::getMousePosition( );
    const int rightMouseLimit = mouseInitialAbsPos.x( ) * 1.3;

    for ( int x = mouseInitialAbsPos.x( ); x < rightMouseLimit; x += 5 ) {
        const QPoint currentPos( x, mouseInitialAbsPos.y( ) );
        GTMouseDriver::moveTo( os, currentPos );
    }
}

GUI_TEST_CLASS_DEFINITION( test_2415 ) {
    // 1. Open "samples/FASTA/human_T1.fa".
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");

    // 2. Right click on the object sequence name in the project view.
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)"));

    // 3. Click the menu {Edit -> Rename}.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EDIT_MENU << "Rename"));
    GTMouseDriver::click(os, Qt::RightButton);

    // 4. Enter the new name: "name".
    GTKeyboardDriver::keySequence(os, "name");

    // 5. Press Enter.
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);

    // Expected state: the sequence is renamed.
    GTUtilsProjectTreeView::findIndex(os, "name");
}

GUI_TEST_CLASS_DEFINITION( test_2424 ) {
//     1. Open WD. Add Read Sequence, Write sequence and Filter quality (custm element with script section) workers. Make sure Fitler Quality worker's script text is empty.
    QString workflowOutputDirPath( testDir + "_common_data/scenarios/sandbox" );
    QDir workflowOutputDir( workflowOutputDirPath );
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    GTUtilsWorkflowDesigner::addAlgorithm( os, "Read Sequence" );
    GTUtilsWorkflowDesigner::addAlgorithm( os, "Write Sequence" );
    GTUtilsWorkflowDesigner::addAlgorithm( os, "Quality filter example" );

    WorkflowProcessItem *seqReader = GTUtilsWorkflowDesigner::getWorker( os, "Read Sequence" );
    WorkflowProcessItem *seqWriter = GTUtilsWorkflowDesigner::getWorker( os, "Write Sequence" );
    WorkflowProcessItem *qualFilter = GTUtilsWorkflowDesigner::getWorker( os, "Quality filter example" );

    GTUtilsWorkflowDesigner::connect(os, seqReader, qualFilter);
    GTUtilsWorkflowDesigner::connect(os, qualFilter, seqWriter);
    //     2. Connect workers into schema, set input data and output data (for example eas.fastq from samples)
    GTMouseDriver::moveTo( os, GTUtilsWorkflowDesigner::getItemCenter( os, "Read Sequence" ) );
    GTMouseDriver::click( os );
    QString dirPath = dataDir + "samples/FASTA/";
    GTUtilsWorkflowDesigner::setDatasetInputFile( os, dirPath, "human_T1.fa" );

    const QString outputFilePath = workflowOutputDir.absolutePath( ) + "/test.gb";

    GTMouseDriver::moveTo( os, GTUtilsWorkflowDesigner::getItemCenter( os, "Write Sequence" ) );
    GTMouseDriver::click( os );
    GTUtilsWorkflowDesigner::setParameter( os, "Output file", outputFilePath, GTUtilsWorkflowDesigner::textValue );
    //     3. Launch the schema.
    //     Expected state: schema didnt start, error "Quality filter example"
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTWidget::click( os,GTAction::button( os,"Run workflow" ) );
    GTGlobals::sleep( );
    GTUtilsWorkflowDesigner::checkErrorList(os, "Quality Filter Example: Empty script text");
}

GUI_TEST_CLASS_DEFINITION( test_2430 ) {
//    1. Check the debug mode checkbox in the applications settings
//    2. Add an element to the scene
//    3. Open breakpoints manager
//    4. Try to add the breakpoint to the element by shortcut (Ctrl+B)
//    Current: nothing happens
//    Expected: the breakpoint appears

    GTUtilsWorkflowDesigner::toggleDebugMode(os);
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Read sequence");
    GTUtilsWorkflowDesigner::toggleBreakpointManager(os);

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read sequence"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, 'b', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep();
    QStringList breakpoints = GTUtilsWorkflowDesigner::getBreakpointList(os);
    CHECK_SET_ERR(breakpoints.size() == 1, "Wrong amount of breakpoints!");
}

GUI_TEST_CLASS_DEFINITION(test_2431) {
    //1. Check the debug mode checkbox in the applications settings
    //2. Add an element to the scene
    //3. Try to delete the element by delete button
    //Current: nothing happens
    //Expected: the element will be deleted

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::toggleDebugMode(os);
    GTGlobals::sleep(1000);

    GTUtilsWorkflowDesigner::addAlgorithm(os, "Read sequence");
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read sequence"));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::getItemCenter(os, "Read sequence") == QRect().center(), "Item didn't deleted");
}

GUI_TEST_CLASS_DEFINITION(test_2432) {
//    1. Check the debug mode checkbox in the applications settings
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::toggleDebugMode(os);
    GTUtilsWorkflowDesigner::toggleBreakpointManager(os);

//    2. Add an element to the scene
    GTUtilsWorkflowDesigner::addAlgorithm(os, "File List");

//    3. Add the breakpoint to the element
    GTUtilsWorkflowDesigner::setBreakpoint(os, "File List");

//    4. Delete the element
//    Expected: there are no breakpoints in the breakpoints manager
    GTUtilsWorkflowDesigner::removeItem(os, "File List");
    GTGlobals::sleep();
    const QStringList breakpoints = GTUtilsWorkflowDesigner::getBreakpointList(os);
    CHECK_SET_ERR(breakpoints.isEmpty(), "There are breakpoints in the workflow");
}

GUI_TEST_CLASS_DEFINITION(test_2437) {
    //1. Select {Tools -> BLAST -> FormatDB...} in the main menu.
    //2. Fill the dialog:
    //    {Select input file(s) for formatting database} : "%datadir%/samples/FASTA/human_T1.fa"
    //    {Type of file(s)} : nucleotide
    //    {Select the path to save database into} : "%testdir%/_common_data/scenarios/sandbox/test_2437"
    //Click the "Format" button.
    FormatDBSupportRunDialogFiller::Parameters p;
    p.inputFilePath = dataDir + "samples/FASTA/human_T1.fa";
    p.alphabetType = FormatDBSupportRunDialogFiller::Parameters::Nucleotide;
    p.outputDirPath = sandBoxDir + "test_2437";
    QDir().mkpath(p.outputDirPath);
    GTUtilsDialog::waitForDialog(os, new FormatDBSupportRunDialogFiller(os, p));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "BLAST" << "FormatDB"));
    GTMenu::showMainMenu(os, MWMENU_TOOLS);

    //3. Wait for the task end.
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //4. Select {Tools -> BLAST -> BLAST Search...} in the main menu.
    //5. Click the "Select a database file" button and select ""%testdir%/_common_data/scenarios/sandbox/test_2437/human_T1formatDB.log".
    //Expected state: {Database path} and {Base name for BLAST DB files} fields are correctly filled.
    class Scenario : public CustomScenario {
    public:
        void run(U2OpStatus &os) {
            GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, sandBoxDir + "test_2437/human_T1formatDB.log"));
            GTWidget::click(os, GTWidget::findWidget(os, "selectDatabasePushButton"));

            QLineEdit *path = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "databasePathLineEdit"));
            CHECK_SET_ERR(!path->text().isEmpty(), "Empty database path");
            QLineEdit *name = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "baseNameLineEdit"));
            CHECK_SET_ERR(name->text() == "human_T1", "Wrong database name");

            GTUtilsDialog::clickButtonBox(os, QApplication::activeModalWidget(), QDialogButtonBox::Cancel);
        }
    };
    GTUtilsDialog::waitForDialog(os, new BlastAllSupportDialogFiller(os, new Scenario()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "BLAST" << "BLAST Search"));
    GTMenu::showMainMenu(os, MWMENU_TOOLS);
}

GUI_TEST_CLASS_DEFINITION( test_2449 ) {
//    1. Open "COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

//    2. Create a phylogenetic tree for the alignment.
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, sandBoxDir + "test_2449.nwk", 0, 0, true));
    GTMenu::clickMenuItemByText(os, GTMenu::showMainMenu(os, MWMENU_ACTIONS), QStringList() << "Tree" << "Build Tree");
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    3. Open tree options panel widget (it can be opened automatically after tree building).
//    4. Open font settings on the OP widget.
    GTWidget::click(os, GTWidget::findWidget(os, "lblFontSettings"));

//    There is a font size spinbox. You can set zero value to it: in this case font has its standard size (on mac), but this value is incorrect.
    QSpinBox* sizeSpinBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "fontSizeSpinBox"));
    CHECK_SET_ERR(NULL != sizeSpinBox, "Size spin box not found");

    GTWidget::setFocus(os, sizeSpinBox);
    int prev = 0;
    while (0 < sizeSpinBox->value()) {
        prev = sizeSpinBox->value();
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
        GTGlobals::sleep(100);
        if (prev <= sizeSpinBox->value()) {
            break;
        }
    }

    CHECK_SET_ERR(0 < sizeSpinBox->value(), "Invalid size spin box bound");
}

GUI_TEST_CLASS_DEFINITION(test_2459) {
//    1. Open "COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

//    2. Set any reference sequence.
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Set this sequence as reference"));
    GTWidget::click(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os), Qt::RightButton, QPoint(10, 10));

//    3. Open context menu, open the "Highlighting" submenu, set the "Agreements" type.
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Highlighting" << "Agreements"));
    GTWidget::click(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os), Qt::RightButton);

//    4. Open context menu again, open the "Export" submenu, choose the "Export highlighted" menu item.
//    Expected state: the "Export highlighted to file" dialog appears.

//    5. Set any valid filename, other settings should be default. Click "Export".
//    Expected state: exporting successfully completes, UGENE doesn't crash.
//    Current state: UGENE crashes.
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Export" << "Export highlighted"));
    GTUtilsDialog::waitForDialog(os, new ExportHighlightedDialogFiller(os, sandBoxDir + "test_2459.txt"));
    GTWidget::click(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os), Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION( test_2460 ) {
    //1. Open "COI.aln".
    //2. Remove all sequences except the first one.
    //3. Align the result one-line-msa by kalign with default values.
    //Expected state: Kalign task finishes with error. Redo button is disabled.

    GTLogTracer l;
    GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");

    QStringList list = GTUtilsMSAEditorSequenceArea::getNameList(os);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(-5, 1), QPoint(-5, list.size() - 1));
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["delete"]);

    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-5, 0));

    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR( l.hasError() == true, "There is no error in the log");

    QAbstractButton *redo= GTAction::button(os,"msa_action_redo");
    CHECK_SET_ERR( NULL != redo, "There is no REDO button");
    CHECK_SET_ERR( redo->isEnabled() == false, "REDO button is enabled");
}

GUI_TEST_CLASS_DEFINITION( test_2475 ) {
    //1. Open WD.
    //2. Open Single-sample Tuxedo Pipeline (NGS samples).
    //3. Set proper input data.
    //4. Validate scheme.
    //Expected state: validation passed.


    class EscClicker : public Filler {
    public:
        EscClicker(U2OpStatus& _os) : Filler(_os, "Tuxedo Wizard"){}
        virtual void run(){
            GTGlobals::sleep();
#ifdef Q_OS_MAC
            GTWidget::click(os, WizardFiller::getCancelButton(os));
#else
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);
#endif
        }
    };

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    GTUtilsDialog::waitForDialog(os, new EscClicker(os));
    GTUtilsDialog::waitForDialog(os, new ConfigureTuxedoWizardFiller(os,
                                                                     ConfigureTuxedoWizardFiller::single_sample,
                                                                     ConfigureTuxedoWizardFiller::singleReads));
    GTUtilsWorkflowDesigner::addSample(os, "RNA-seq analysis with Tuxedo tools");
    GTGlobals::sleep();

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read RNA-seq Short Reads"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/bowtie2", "reads_1.fq");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/bowtie2", "reads_2.fq");

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Find Splice Junctions with TopHat"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setParameter(os, "Bowtie index directory", testDir + "_common_data/bowtie2/index", GTUtilsWorkflowDesigner::textValue);
    GTUtilsWorkflowDesigner::setParameter(os, "Bowtie index basename", "human_T1_cutted", GTUtilsWorkflowDesigner::textValue);

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTWidget::click(os, GTAction::button(os,"Validate workflow"));
}

GUI_TEST_CLASS_DEFINITION( test_2482 ) {
    // 1. Open "COI.nwk".
    GTFileDialog::openFile(os, dataDir + "samples/Newick/", "COI.nwk");

    // 2. Change the tree layout to unrooted.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< "Unrooted"));
    GTWidget::click(os, GTWidget::findWidget(os, "Layout"));

    // 3. Select any node in the tree that is not a leaf.
    QList<QGraphicsItem*> items = GTUtilsPhyTree::getNodes(os);
    CHECK_SET_ERR(items.size() >= 4, "Incorrect tree size");

    QPoint nodeCoords = GTUtilsPhyTree::getGlobalCoord(os, items.at(3));
    GTMouseDriver::moveTo(os, nodeCoords);

    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    // 4. Call context menu on the Tree Viewer.
    // Expected state: menu items "Swap Sublings" and "Reroot tree" are disabled.
    QMenu* activePopupMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    CHECK_SET_ERR(activePopupMenu != NULL, "There is no popup menu appeared");

    const QList<QAction*> menuActions = activePopupMenu->actions();

    foreach (QAction* a, menuActions) {
        if (a->text() == "Reroot tree" || a->text() == "Swap Sublings") {
            CHECK_SET_ERR( !a->isEnabled(), a->text() + " action is enabled");
        }
    }
}


GUI_TEST_CLASS_DEFINITION( test_2487 ) {
    // 1. Open "COI.nwk".
    // 2. Select root-node.
    // 3. Call context menu.
    // 'Reroot tree' and 'Collapse ' options must be disabled

    GTFileDialog::openFile(os, dataDir + "samples/Newick/", "COI.nwk");

    QList<QGraphicsItem*> items = GTUtilsPhyTree::getNodes(os);
    CHECK_SET_ERR(items.size() != 0, "Tree is empty");

    QPoint rootCoords = GTUtilsPhyTree::getGlobalCoord(os, items.first());
    GTMouseDriver::moveTo(os, rootCoords);

    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    QMenu* activePopupMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    CHECK_SET_ERR(activePopupMenu != NULL, "There is no popup menu appeared");

    const QList<QAction*> menuActions = activePopupMenu->actions();

    foreach (QAction* a, menuActions) {
        if (a->text() == "Reroot tree" || a->text() == "Collapse") {
            CHECK_SET_ERR( !a->isEnabled(), a->text() + " action is enabled");
        }
    }
}

GUI_TEST_CLASS_DEFINITION( test_2496 ) {
    // 1. Open file testDir/_common_data/scenarios/assembly/example-alignment.bam
    // Expected state: import bam dialog appeared

    GTUtilsDialog::waitForDialog(os,
                                 new ImportBAMFileFiller(os, testDir + "_common_data/scenarios/sandbox/example-alignment.bam.ugenedb"));
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/assembly/", "example-alignment.bam");
}

GUI_TEST_CLASS_DEFINITION( test_2498 ) {
    // 1. Open the /test/_common_data/fasta/empty.fa empty msa file.
    // 2. Open context menu on the sequence area. Go to the {Export -> amino translation}
    //

    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "empty.fa");
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));

    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << MSAE_MENU_EXPORT << "amino_translation_of_alignment_rows",
                                                      PopupChecker::IsDisabled, GTGlobals::UseMouse));
    GTMouseDriver::click(os, Qt::RightButton);

    //TODO: Expected state: this menu item is disabled for the empty msa.
}

GUI_TEST_CLASS_DEFINITION( test_2506 ) {
    // 1. Open file _common_data/vcf_consensus/vcf_cons_out_damaged_1.vcf
    // Expected state: error message appears.
    GTLogTracer l;

    GTFileDialog::openFile(os, testDir + "_common_data/vcf_consensus/", "vcf_cons_out_damaged_1.vcf");

    CHECK_SET_ERR(l.hasError() == true, "There is no expected error message in log");
}

GUI_TEST_CLASS_DEFINITION( test_2506_1 ) {
    // 1. Open file _common_data/vcf_consensus/vcf_cons_out_damaged_2.vcf
    // Expected state: error message appears.
    GTLogTracer l;

    GTFileDialog::openFile(os, testDir + "_common_data/vcf_consensus/", "vcf_cons_out_damaged_2.vcf");

    CHECK_SET_ERR(l.hasError() == true, "There is no expected error message in log");
}
GUI_TEST_CLASS_DEFINITION( test_2519 ) {
    // 1. Open {data/samples/FASTA/human_T1.fa}.
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");

    // 2. Go to position 20000.
    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 20000));
    GTKeyboardDriver::keyClick(os, 'g', GTKeyboardDriver::key["ctrl"]);

    // 3. Call context menu, select menu item {Edit sequence -> Remove subsequence...}.
    // Expected state: a "Remove subsequence" dialog appears.
    // 4. Set the region (1..190950), other settings should be default, click the "Remove" button.
    // Expected state: UGENE doesn't crash.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE));
    GTUtilsDialog::waitForDialog(os, new RemovePartFromSequenceDialogFiller(os, "1..190950"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(2000);

    // 5. Call this dialog again, remove region (1..8999).
    // Expected state: UGENE doesn't crash.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE));
    GTUtilsDialog::waitForDialog(os, new RemovePartFromSequenceDialogFiller(os, "1..8999"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION( test_2538 ){
//    1. Open file "_common_data/scenarios/tree_view/COI.nwk"
    GTFileDialog::openFile(os, dataDir + "/samples/Newick/", "COI.nwk");
//    2. Call context menu on node just near root. Click "Collapse"
    GTGlobals::sleep(1000);
    GTMouseDriver::moveTo(os, GTUtilsPhyTree::getGlobalCoord(os, GTUtilsPhyTree::getNodes(os).at(1)));
    GTMouseDriver::click(os);
    GTGlobals::sleep(1000);

    QWidget* treeView = GTWidget::findWidget(os, "treeView");

    QPixmap pixmap = QPixmap::grabWidget(treeView, treeView->rect());
    QImage initImg = pixmap.toImage();

    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep(1000);
//    3. Call context menu on node just near root. Click "Expand"
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep(1000);
//    Expected state: tree has the same view as at the beginning
    pixmap = QPixmap::grabWidget(treeView, treeView->rect());
    QImage finalImg = pixmap.toImage();

    //images have several pixels differ. so sizes are compared
    CHECK_SET_ERR(initImg.size() == finalImg.size(), "different images");
}

GUI_TEST_CLASS_DEFINITION( test_2542 ) {
    // 1. Open "data/samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

    // 2. Lock the document
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_project__edit_menu"
        << "action_document_lock"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI.aln"));
    GTMouseDriver::click(os, Qt::RightButton);

    // 3. Do pairwise alignment with any sequences.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(1, 1));
    GTWidget::click(os, GTWidget::findWidget(os, "OP_PAIRALIGN"));
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Output settings"));

    QCheckBox *check = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "inNewWindowCheckBox"));
    CHECK_SET_ERR(check != NULL, "inNewWindowCheckBox not found!");
    GTCheckBox::setChecked(os, check, false);

    // State:
    // Align button not active if file locked for writing (user locking or format didn't support writing) and selected aligning in current file option
    QWidget* button = GTWidget::findWidget(os, "alignButton");

    CHECK_SET_ERR(!button->isEnabled(), "Align button is enabled");
}

GUI_TEST_CLASS_DEFINITION( test_2543 ) {
    class BuildTreeDialogFiller_test_2543 : public Filler {
    public:
        BuildTreeDialogFiller_test_2543(U2OpStatus &os, QString _saveTree="default") : Filler(os, "CreatePhyTree"),
            saveTree(_saveTree){}
        virtual void run(){
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");

            if(saveTree!="default"){
                QLineEdit* saveLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os,"fileNameEdit"));
                GTLineEdit::setText(os,saveLineEdit, saveTree);
            }

            GTUtilsDialog::waitForDialogWhichMayRunOrNot( os, new LicenseAgreemntDialogFiller( os ) );
            //Expected state: UGENE does not allow to create tree, the message dialog appears
            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller( os, QMessageBox::Ok ) );

            QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
            CHECK_SET_ERR(box != NULL, "buttonBox is NULL");
            QPushButton* button = box->button(QDialogButtonBox::Ok);
            CHECK_SET_ERR(button !=NULL, "cancel button is NULL");
            GTWidget::click(os, button);

            GTGlobals::sleep(1000);
            button = box->button(QDialogButtonBox::Cancel);
            CHECK_SET_ERR(button !=NULL, "cancel button is NULL");
            GTWidget::click(os, button);
        }
    private:
        QString saveTree;
    };

    //1. Open "samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "COI.aln" );

    //2. Click the "Build tree" button on the toolbar.
    //Expected state: a "Build Phylogenetic Tree" dialog appeared.
    //3. Set the output file location to any read-only folder.
    const QString outputFilePath = testDir + "_common_data/scenarios/sandbox/gui_regr_2543";
    QDir sandboxDir( testDir + "_common_data/scenarios/sandbox" );
    sandboxDir.mkdir( "gui_regr_2543" );
    PermissionsSetter permSetter;
    const QFile::Permissions p = QFile::WriteOwner | QFile::WriteUser | QFile::WriteGroup
        | QFile::WriteOther;
    bool res = permSetter.setPermissions( outputFilePath, ~p );
    CHECK_SET_ERR( res, "Can't set permissions" );
    GTGlobals::sleep( 2000 );

    GTUtilsDialog::waitForDialog( os, new BuildTreeDialogFiller_test_2543( os, outputFilePath + "/test.nwk") );


    QAbstractButton *tree= GTAction::button( os, "Build Tree" );
    GTWidget::click( os, tree );
    GTGlobals::sleep( 2000 );
}

GUI_TEST_CLASS_DEFINITION(test_2545) {
    //1. Remove the "spidey" external tool, if it exists.
    GTUtilsExternalTools::removeTool(os, "Spidey");

    //2. Open "human_t1.fa".
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    //3. Call context menu, select the {Align -> Align sequence to mRna} menu item.
    //Expected state: UGENE offers to select the "spidey" external tool.
    //4. Refuse the offering.
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ALIGN" << "Align sequence to mRNA"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));

    //Expected state: align task is cancelled.
    CHECK_SET_ERR(0 == GTUtilsTaskTreeView::getTopLevelTasksCount(os), "The task is not canceled");
}

GUI_TEST_CLASS_DEFINITION(test_2549) {
    //this needed to ensure that log view has text
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/gtf/invalid", "AB375112_annotations.gtf"));
    GTUtilsDialog::waitForDialog(os, new DocumentFormatSelectorDialogFiller(os, "GTF"));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__OPEN_AS);
    GTGlobals::sleep();

    GTKeyboardDriver::keyClick(os, '3', GTKeyboardDriver::key["alt"]);
    GTGlobals::sleep();
    GTWidget::click(os, GTWidget::findWidget(os, "dock_log_view"));
    GTKeyboardDriver::keySequence(os, "this sequence needed to ensure that log view has text");

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    const QString clipboardContent = GTClipboard::text(os);
    CHECK_SET_ERR(!clipboardContent.isEmpty(), "Clipboard is empty");
}

GUI_TEST_CLASS_DEFINITION(test_2562) {
    GTLogTracer l;

    GTUtilsDialog::waitForDialog(os, new DocumentFormatSelectorDialogFiller(os, "Swiss-Prot"));
    // 1. Open any amino sequence, e.g. "data/samples/Swiss-Prot/D0VTW9.txt" in the Sequence View.
    GTFileDialog::openFile(os, dataDir + "samples/Swiss-Prot/", "D0VTW9.txt");

    // 2. Open "DAS Annotations" options panel widget.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_DAS"));

    // 3. Press "Fetch IDs" button
    // Expected state : task "Get an ID for the sequence" started
    GTWidget::click(os, GTWidget::findWidget(os, "searchIdsButton"));
    GTGlobals::sleep(500);

    GTUtilsTaskTreeView::checkTask(os, "Get an ID for the sequence");

    // 4. Cancel the task
    // Expected state : the task has been canceled
    GTUtilsTaskTreeView::cancelTask(os, "Get an ID for the sequence");
    CHECK_SET_ERR(0 == GTUtilsTaskTreeView::getTopLevelTasksCount(os), "Uniprot BLAST task has not been cancelled");
    GTUtilsLog::check(os, l);

    // 5. Press "Fetch IDs" again
    // Expected state : task "Get an ID for the sequence" started
    // 6. Wait until the task is completed
    GTWidget::click(os, GTWidget::findWidget(os, "searchIdsButton"));
    GTGlobals::sleep(500);

    GTUtilsTaskTreeView::checkTask(os, "Get an ID for the sequence");
    GTUtilsTaskTreeView::waitTaskFinished(os, 240000);

    // 7. Choose any id and press "Fetch Annotations" button
    // Expected state : task "Load DAS annotations for current sequence" started
    QTableWidget *idTable = qobject_cast<QTableWidget *>(GTWidget::findWidget(os, "idList"));
    const QString selectedFeatureId = idTable->selectionModel()->selectedRows().first().data(Qt::DisplayRole).toString();

    GTWidget::click(os, GTWidget::findWidget(os, "annotateButton"));
    const QString annotateTaskName = "Convert ID and load DAS features for: " + selectedFeatureId;
    GTUtilsTaskTreeView::checkTask(os, annotateTaskName);

    // 9. Cancel the task
    // Expected state : the task cancelled
    GTUtilsTaskTreeView::cancelTask(os, annotateTaskName);
    GTGlobals::sleep(500);
    CHECK_SET_ERR(0 == GTUtilsTaskTreeView::getTopLevelTasksCount(os), "Load DAS annotations task has not been cancelled");
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_2562_1) {
    GTLogTracer l;
    // 1. Open "File -> Access Remote Database..." from the main menu.
    // 2. Get something adequate from the Uniprot(DAS) database(e.g.use example ID : P05067).
    // Expected state : "DownloadRemoteDocuments" task has been started
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFillerDeprecated(os, "P05067", 7));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB);
    GTUtilsTaskTreeView::checkTask(os, "Load DAS Documents");

    // 3. Cancel task
    // Expected state : task has been canceled.
    GTUtilsTaskTreeView::cancelTask(os, "Load DAS Documents");
    GTGlobals::sleep(500);
    CHECK_SET_ERR(0 == GTUtilsTaskTreeView::getTopLevelTasksCount(os), "Load DAS documents task has not been cancelled");
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION( test_2565 ) {
    //    1. Open "samples/Genbank/murine.gb".
    //    2. Press Ctrl+F.
    //    3. Insert the pattern "GCTAGCTTAAGTAACGCCACTTTT".
    //    4. Click "Search".
    //    Expected: the pattern is not found. Notification with this information appears.
    GTLogTracer l;
    GTFileDialog::openFile(os, dataDir+"samples/Genbank/", "murine.gb");
    GTUtilsOptionsPanel::runFindPatternWithHotKey("GCTAGCTTAAGTAACGCCACTTTT", os);
    CHECK_SET_ERR(l.checkMessage(QString("Searching patterns in sequence task: No results found.")),
                  "No expected message in the log");

}

GUI_TEST_CLASS_DEFINITION(test_2566) {
//1. Open "samples/Genbank/murine.gb".
    GTLogTracer l;
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "murine.gb");

//2. Press Ctrl+F.
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

//3. Enter the pattern: GCTAGCTTAAGTAACGCCAC
    QWidget *patternInputLine = QApplication::focusWidget();
    CHECK_SET_ERR(NULL != patternInputLine && patternInputLine->objectName() == "textPattern", "Focus is not on FindPattern widget");

    GTKeyboardDriver::keySequence(os, "GCTAGCTTAAGTAACGCCAC");
    GTGlobals::sleep(1000);

//4. Choose the algorithm: Substitute.
    GTUtilsOptionPanelSequenceView::setAlgorithm(os, "Substitute");

//5. Enter the "Should match" value: 97%
    GTUtilsOptionPanelSequenceView::setMatchPercentage(os, 97);

//6. Click "Search".
    // There is no "Search buttons since UGENE-3483 was done, the task is already launched
    GTGlobals::sleep(500);
    GTUtilsTaskTreeView::waitTaskFinished(os);

//Expected: 97% is entered.
    const int matchPercentage = GTUtilsOptionPanelSequenceView::getMatchPercentage(os);
     CHECK_SET_ERR(97 == matchPercentage, "Entered and actual values don't match");

//Expected state: the task finished successfully.
     CHECK_SET_ERR(!l.hasError(), "Unexpected error in log!: " + l.getError());
}

GUI_TEST_CLASS_DEFINITION( test_2567 ) {
    //1. Open "samples/Genbank/murine.gb".
    GTLogTracer l;
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "murine.gb");

    //2. Press Ctrl+F.
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    //3. Enter the pattern: GCTAGCTTAAGTAACGCCAC
    QWidget *patternInputLine = QApplication::focusWidget();
    CHECK_SET_ERR(NULL != patternInputLine && patternInputLine->objectName() == "textPattern", "Focus is not on FindPattern widget");

    GTKeyboardDriver::keySequence(os, "GCTAGCTTAAGTAACGCCAC");
    GTGlobals::sleep(1000);

    //4. Choose the algorithm: Substitute.
    GTUtilsOptionPanelSequenceView::setAlgorithm(os, "Substitute");

    //5. Enter the "Should match" value: 30%
    GTUtilsOptionPanelSequenceView::setMatchPercentage(os, 30);

    //6. Click "Search".
    // There is no "Search buttons since UGENE-3483 was done, the task is already launched
    GTGlobals::sleep(500);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: the task finished successfully.
    CHECK_SET_ERR(!l.hasError(), "Unexpected error in log!: " + l.getError());
}

GUI_TEST_CLASS_DEFINITION( test_2570 ) {
    GTLogTracer l;
    GTFileDialog::openFile( os, dataDir + "samples/FASTA/", "human_T1.fa" );

    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_BLAST_SUBMENU << ACTION_BLAST_FORMAT_DB));
    FormatDBSupportRunDialogFiller::Parameters p;
    p.justCancel = true;
    p.checkAlphabetType = true;
    p.alphabetType = FormatDBSupportRunDialogFiller::Parameters::Nucleotide;
    GTUtilsDialog::waitForDialog(os, new FormatDBSupportRunDialogFiller(os, p));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION( test_2577 ) {

    //1. Open {data/samples/CLUSTALW/COI.aln}.
    GTLogTracer l;
    GTFileDialog::openFile( os, dataDir + "samples/CLUSTALW/", "COI.aln" );

    //2. Call context menu somewhere on the alignment, select {Consenssus mode...} menu item.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Consensus mode",GTGlobals::UseMouse));
    GTMenu::showContextMenu(os,GTUtilsMdi::activeWindow(os));

    //Expected state: options panel "General" tab opens.
    QWidget *panel = GTWidget::findWidget(os, "OP_MSA_GENERAL");
    CHECK_SET_ERR(NULL != panel, "General OP tab does not appear");
    GTGlobals::sleep(500);

}

GUI_TEST_CLASS_DEFINITION( test_2578 ) {
//    1. Open {data/samples/CLUSTALW/COI.aln}.
    GTFileDialog::openFile(os, dataDir + "/samples/CLUSTALW/", "COI.aln");

//    2. Open options panel 'Highlighting' tab.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_HIGHLIGHTING"));

//    3. Select 'agreements' highlighting scheme.
    QComboBox* combo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "highlightingScheme"));
    CHECK_SET_ERR(combo != NULL, "highlightingScheme not found!");
    GTComboBox::setIndexWithText(os, combo , "Agreements");

    QWidget* exportButton = GTWidget::findWidget(os, "exportHighlightning");
    CHECK_SET_ERR(NULL != exportButton, "exportButton not found");
    CHECK_SET_ERR(!exportButton->isEnabled(), "exportButton is enabled unexpectedly");

//    4. Select any reference sequence.
//    Expected state: the "Export" button is active.
    GTWidget::click(os, GTWidget::findWidget(os, "sequenceLineEdit"));
    GTKeyboardDriver::keySequence(os, "Montana_montana");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep(300);
    CHECK_SET_ERR(exportButton->isEnabled(), "exportButton is disabled unexpectedly");

//    5. Select 'General' tab, then select 'Highlighting' tab again.
//    Expected state: the "Export" button is active.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_GENERAL"));
    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_HIGHLIGHTING"));

    exportButton = GTWidget::findWidget(os, "exportHighlightning");
    CHECK_SET_ERR(NULL != exportButton, "exportButton not found");
    CHECK_SET_ERR(exportButton->isEnabled(), "exportButton is disabled unexpectedly");
}

GUI_TEST_CLASS_DEFINITION( test_2579 ) {
//  Linux:
//  1. Click the menu Settings -> Preferences -> External Tools.
//  2. Find the MAFFT item and if it has the active red cross, click it.
//  3. Click the MAFFT's browse button and add the MAFFT executable from UGENE external tools package.
//  Expected: there are no errors in the log.

    class MafftInactivation : public CustomScenario {
    public:
        MafftInactivation() {}
        virtual void run(U2::U2OpStatus &os) {
            QString path = AppSettingsDialogFiller::getExternalToolPath(os, "MAFFT");
            AppSettingsDialogFiller::clearToolPath(os, "MAFFT");
            AppSettingsDialogFiller::setExternalToolPath(os, "MAFFT", path);

            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTLogTracer l;

    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new MafftInactivation()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action__settings"));
    GTMenu::showMainMenu(os, MWMENU_SETTINGS);

    GTUtilsLog::check(os, l);
}
GUI_TEST_CLASS_DEFINITION(test_2581) {

    // 1. Open file "_common_data/scenarios/msa/ma2_gapped_same_names.aln"
    // 2. Use context menu { Align -> Align with MUSCLE }
    // Expected state: the "Align with MUSCLE" dialog has appeared
    // 3. Press the "Align" button
    // Expected state: after a few seconds alignment has finished, UGENE does not crash
    GTLogTracer l;

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped_same_names.aln");

    GTUtilsDialog::waitForDialog(os, new MuscleDialogFiller(os, MuscleDialogFiller::Default));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with muscle", GTGlobals::UseMouse));

    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    GTUtilsLog::check(os, l);

}

GUI_TEST_CLASS_DEFINITION(test_2581_1) {
    // 1. Open file "_common_data/scenarios/msa/ma2_gapped_same_names.aln"
    // 2. Use context menu { Align -> Align with ClustalW }
    // Expected state: the "Align with ClustalW" dialog has appeared
    // 3. Press the "Align" button
    // Expected state: after a few seconds alignment has finished, UGENE does not crash

    GTLogTracer l;

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped_same_names.aln");

    GTUtilsDialog::waitForDialog(os, new ClustalWDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with ClustalW", GTGlobals::UseMouse));

    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_2581_2) {
//    1. Open file "_common_data/scenarios/msa/ma2_gapped_same_names.aln"
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa", "ma2_gapped_same_names.aln");

//    2. Use context menu { Align -> Align with ClustalO }
//    Expected state: the "Align with Clustal Omega" dialog has appeared

//    3. Press the "Align" button
//    Expected state: after a few seconds alignment has finished, UGENE does not crash
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Align" << "Align with ClustalO..."));
    GTUtilsDialog::waitForDialog(os, new ClustalOSupportRunDialogFiller(os));
    GTWidget::click(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os), Qt::RightButton);

    GTGlobals::sleep();
    GTUtilsTaskTreeView::waitTaskFinished(os);
}

GUI_TEST_CLASS_DEFINITION(test_2581_3) {
//    1. Open file "_common_data/scenarios/msa/ma2_gapped_same_names.aln"
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa", "ma2_gapped_same_names.aln");

//    2. Use context menu { Align -> Align with MAFFT }
//    Expected state: the "Align with MAFFT" dialog has appeared

//    3. Press the "Align" button
//    Expected state: after a few seconds alignment has finished, UGENE does not crash
    MAFFTSupportRunDialogFiller::Parameters parameters;
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Align" << "Align with MAFFT..."));
    GTUtilsDialog::waitForDialog(os, new MAFFTSupportRunDialogFiller(os, &parameters));
    GTWidget::click(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os), Qt::RightButton);

    GTGlobals::sleep();
    GTUtilsTaskTreeView::waitTaskFinished(os);
}

GUI_TEST_CLASS_DEFINITION(test_2581_4) {
    // 1. Open file "_common_data/scenarios/msa/ma2_gapped_same_names.aln"
    // 2. Use context menu { Align -> Align with T-Coffee }
    // Expected state: the "Align with T-Coffee" dialog has appeared
    // 3. Press the "Align" button
    // Expected state: after a few seconds alignment has finished, UGENE does not crash

    GTLogTracer l;

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped_same_names.aln");

    GTUtilsDialog::waitForDialog(os, new TCoffeeDailogFiller(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with T-Coffee", GTGlobals::UseMouse));

    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_2581_5) {
    // 1. Open file "_common_data/scenarios/msa/ma2_gapped_same_names.aln"
    // 2. Use context menu { Align -> Align with Kalign }
    // Expected state: the "Align with Kalign" dialog has appeared
    // 3. Press the "Align" button
    // Expected state: after a few seconds alignment has finished, UGENE does not crash

    GTLogTracer l;

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma2_gapped_same_names.aln");

    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign", GTGlobals::UseMouse));

    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsLog::check(os, l);
}


GUI_TEST_CLASS_DEFINITION( test_2605 ) {
    GTLogTracer logTracer;
    // 1. Open file _common_data/fasta/multy_fa.fa as multiple alignment
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Join));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "multy_fa.fa");
    // 2. Export subalignment from this msa to any MSA format
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_EXPORT<<"Save subalignment"));
    GTUtilsDialog::waitForDialog(os,new ExtractSelectedAsMSADialogFiller(os,
        testDir + "_common_data/scenarios/sandbox/2605.aln",
        QStringList() << "SEQUENCE_1", 6, 237));
    GTMenu::showContextMenu(os,GTWidget::findWidget(os,"msa_editor_sequence_area"));

    // Expected state: export successfull, no any messages in log like "There is no sequence objects in given file, unable to convert it in multiple alignment"
    CHECK_SET_ERR(!logTracer.hasError(), "Unexpected error");
}

GUI_TEST_CLASS_DEFINITION( test_2612 ) {
    // 1. Open sequence "samples/fasta/human_T1.fa".
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
    // 2. Search for 20 first nucleotides (TTGTCAGATTCACCAAAGTT) using Find Pattern.
    GTUtilsOptionsPanel::runFindPatternWithHotKey("TTGTCAGATTCACCAAAGTT", os);
    GTGlobals::sleep(1000);
    GTWidget::click(os, GTWidget::findWidget(os, "getAnnotationsPushButton"));
    GTGlobals::sleep(500);
    // Expected state: the annotation with pattern created and shown in sequence view.
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    GTGlobals::sleep(1000);
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature");
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    // 3. Delete annotation from annotation editor.
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    // Expected state: there is no annotation in sequence view.
    GTGlobals::sleep(100);
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "misc_feature", GTGlobals::FindOptions(false))==NULL, "Annotations document not deleted");
}

GUI_TEST_CLASS_DEFINITION(test_2619) {
//    1. Open file samples/genbank/sars.gb
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "sars.gb");

//    2. Open context menu for any qualifier on annotation table view.
//    Expected state: submenu "Copy" didn't contains items "Edit qualifier" and "Add 'evidence' column"
    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << ADV_MENU_COPY << "edit_qualifier_action", PopupChecker::NotExists));
    GTUtilsAnnotationsTreeView::callContextMenuOnQualifier(os, "5'UTR", "evidence");
    GTGlobals::sleep(500);

    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << ADV_MENU_COPY << "toggle_column", PopupChecker::NotExists));
    GTUtilsAnnotationsTreeView::callContextMenuOnQualifier(os, "5'UTR", "evidence");
    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_2622) {
    // 1. Open "_common_data/fasta/multy_fa.fa".
    // 2. Choose "Merge sequences ..." with 100 bases.
    GTLogTracer l;

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Merge, 100));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "multy_fa.fa");

    // 3. Press Ctrl + F.
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    // 4. Choose "Regular expression" algorithm.
    QComboBox *boxAlgorithm = qobject_cast<QComboBox *>(GTWidget::findWidget(os, "boxAlgorithm"));
    GTWidget::click(os, boxAlgorithm);
    GTComboBox::setCurrentIndex(os, boxAlgorithm, 1);

    // 5. Write "X+" in the pattern string.
    QWidget *textPattern = GTWidget::findWidget(os, "textPattern");
    GTWidget::click(os, textPattern);
    GTKeyboardDriver::keyClick(os, 'X');
    GTKeyboardDriver::keyClick(os, '=', GTKeyboardDriver::key["shift"]);

    // UGENE does not hang.
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_2622_1) {
    // 1. Open "_common_data/fasta/multy_fa.fa".
    // 2. Choose "Merge sequences ..." with 100 bases.
    GTLogTracer l;

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Merge, 100));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "multy_fa.fa");

    // 3. Press Ctrl + F.
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    // 4. Choose "Regular expression" algorithm.
    QComboBox *boxAlgorithm = qobject_cast<QComboBox *>(GTWidget::findWidget(os, "boxAlgorithm"));
    GTWidget::click(os, boxAlgorithm);
    GTComboBox::setCurrentIndex(os, boxAlgorithm, 1);

    // 5. Write "X+" in the pattern string.
    QWidget *textPattern = GTWidget::findWidget(os, "textPattern");
    GTWidget::click(os, textPattern);
    GTKeyboardDriver::keyClick(os, 'X');
    GTKeyboardDriver::keyClick(os, '=', GTKeyboardDriver::key["shift"]);

    // 6. Check "Results no longer than" and set the value 1.
    QCheckBox *boxUseMaxResultLen = qobject_cast<QCheckBox *>(GTWidget::findWidget(os, "boxUseMaxResultLen"));
    GTCheckBox::setChecked(os, boxUseMaxResultLen, true);

    QSpinBox *boxMaxResultLen = qobject_cast<QSpinBox *>(GTWidget::findWidget(os, "boxMaxResultLen"));
    GTSpinBox::setValue(os, boxMaxResultLen, 1, GTGlobals::UseKeyBoard);

    // UGENE does not hang and all results are 1 bp length (100 results).
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsLog::check(os, l);

    QLabel *resultLabel = qobject_cast<QLabel *>(GTWidget::findWidget(os, "resultLabel"));
    CHECK_SET_ERR(resultLabel->text() == "Results: 1/100", "Unexpected find algorithm results");
}

GUI_TEST_CLASS_DEFINITION(test_2651) {
    // 1. File->Search NCBI Genbank...
    // 2. In the search field paste
    // AB797204.1 AB797210.1 AB797201.1
    // 3. Click Search
    // 4. Select three results and download them
    // 5. Close the dialog
    GTLogTracer l;

    QList<int> resultNumbersToSelect;
    resultNumbersToSelect << 0 << 1 << 2;
    const QVariant variantNumbers = QVariant::fromValue<QList<int> >(resultNumbersToSelect);
    const QVariant searchField = QVariant::fromValue<QPair<int, QString> >(QPair<int, QString>(0, "AB797204.1 AB797210.1 AB797201.1"));

    QList<DownloadRemoteFileDialogFiller::Action> remoteDialogActions;
    remoteDialogActions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::ClickOk, QVariant());
    const QVariant remoteDialogActionsVariant = QVariant::fromValue<QList<DownloadRemoteFileDialogFiller::Action> >(remoteDialogActions);

    QList<NcbiSearchDialogFiller::Action> actions;
    actions << NcbiSearchDialogFiller::Action(NcbiSearchDialogFiller::SetTerm, searchField)
        << NcbiSearchDialogFiller::Action(NcbiSearchDialogFiller::ClickSearch, QVariant())
        << NcbiSearchDialogFiller::Action(NcbiSearchDialogFiller::WaitTasksFinish, QVariant())
        << NcbiSearchDialogFiller::Action(NcbiSearchDialogFiller::SelectResultsByNumbers, variantNumbers)
        << NcbiSearchDialogFiller::Action(NcbiSearchDialogFiller::ClickDownload, remoteDialogActionsVariant)
        << NcbiSearchDialogFiller::Action(NcbiSearchDialogFiller::ClickClose, QVariant());
    GTUtilsDialog::waitForDialog(os, new NcbiSearchDialogFiller(os, actions));

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__SEARCH_GENBANK);

    GTGlobals::sleep(5000);

    // 6. With Ctrl pressed, select all three annotation objects in the project view
    GTUtilsProjectTreeView::openView(os);
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["ctrl"]);

    GTUtilsProjectTreeView::click(os, "AB797210 features");
    GTUtilsProjectTreeView::click(os, "AB797204 features");
    GTUtilsProjectTreeView::click(os, "AB797201 features");

    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["ctrl"]);

    // 7. delete this objects through context menu
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_project__remove_selected_action"));
    GTMouseDriver::click(os, Qt::RightButton);

    // Expected state : the objects are deleted, the popup is shown
    GTGlobals::FindOptions safeOptions(false);
    const QModelIndex firstIndex = GTUtilsProjectTreeView::findIndex(os, "AB797210 features", safeOptions);
    CHECK_SET_ERR(!firstIndex.isValid(), "The \"AB797210 features\" item has not been deleted");
    const QModelIndex secondIndex = GTUtilsProjectTreeView::findIndex(os, "AB797204 features", safeOptions);
    CHECK_SET_ERR(!firstIndex.isValid(), "The \"AB797204 features\" item has not been deleted");
    const QModelIndex thirdIndex = GTUtilsProjectTreeView::findIndex(os, "AB797201 features", safeOptions);
    CHECK_SET_ERR(!firstIndex.isValid(), "The \"AB797201 features\" item has not been deleted");

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION( test_2656 ) {
//    0. Create a file with an empty sequence. A FASTA file with the first line ">seq1" and the empty second line
//    1. Open any sequence
//    2. On the toolbar, press "Build dotplot"
//    3. Press "Load sequence" in the dialog. Load the empty sequence
//    4. Select the empty sequence as the second sequence in the combobox

//    Bug: UGENE crashes
//    Expected: an error message is shown
    GTLogTracer l;

    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    class DotplotLoadSequenceFiller : public Filler {
    public:
        DotplotLoadSequenceFiller(U2OpStatus &os, const QString seqPath, const QString seqName)
            : Filler(os, "DotPlotDialog"), seqPath(seqPath), seqName(seqName) {}
        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");

            QPushButton* loadSeq = qobject_cast<QPushButton*>(GTWidget::findWidget(os, "loadSequenceButton", dialog));
            CHECK_SET_ERR(loadSeq != NULL, "Load sequence button no found");
            GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, seqPath, seqName));
            GTWidget::click(os, loadSeq);

            QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
            QPushButton* button = box->button(QDialogButtonBox::Cancel);
            CHECK_SET_ERR( button != NULL, "Cancel button is NULL");
            GTWidget::click(os, button);
        }

    private:
        QString seqPath;
        QString seqName;
    };

    GTUtilsDialog::waitForDialog(os, new DotplotLoadSequenceFiller(os, testDir + "_common_data/fasta", "empty_2.fa"));
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));

    CHECK_SET_ERR( l.hasError(), "An error should be in the log");
}

GUI_TEST_CLASS_DEFINITION( test_2667 ) {
//    1. Open {/data/samples/genbank/murine.gb}.
//    Expected state: a document was added, it contains two object: an annotation and a sequence
    GTFileDialog::openFile(os, dataDir + "/samples/Genbank/", "murine.gb");

//    2. Select the annotation object in the project view.
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::click(os);

//    3. Press "delete" key.
//    Expected state: the annotation object is removed from the document.
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::FindOptions options;
    options.failIfNull = false;
    GTUtilsProjectTreeView::findIndex(os, "NC_001363 features", options);
}

GUI_TEST_CLASS_DEFINITION(test_2690){
//    1. Open "human_t1.fa".
    GTFileDialog::openFile( os, dataDir + "samples/FASTA", "human_T1.fa" );
//    2. Create an annotation: Group name - "1", location - "1..1".
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "1", "ann1", "1..1"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
//    3. Create an annotation: Group name - "2", location - "5..5, 6..7".
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "2", "ann2", "5..5, 6..7"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
//    4. Open the "Annotation highlighting" OP widget.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_ANNOT_HIGHLIGHT"));
//    5. Select the first annotation.
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "ann1"));
    GTMouseDriver::click(os);
//    6. Click the "next annotation" button.
    QWidget* nextAnnotationButton = GTWidget::findWidget(os, "nextAnnotationButton");
    GTWidget::click(os, nextAnnotationButton);
    GTGlobals::sleep(500);
//    Expected state: the first location of the second annotation is selected.
    QString str = GTUtilsAnnotationsTreeView::getSelectedItem(os);
    CHECK_SET_ERR(str == "ann2", "unexpected selected annotation: " + str);
    CHECK_SET_ERR(nextAnnotationButton->isEnabled(), "nextAnnotationButton is unexpectidly disabled")
//    7. Click the "next annotation" button.
    GTWidget::click(os, nextAnnotationButton);
    GTGlobals::sleep(500);
    str = GTUtilsAnnotationsTreeView::getSelectedItem(os);
    CHECK_SET_ERR(str == "ann2", "unexpected selected annotation after click: " + str);
    CHECK_SET_ERR(!nextAnnotationButton->isEnabled(), "nextAnnotationButton is unexpectidly enabled");
//    Expected state: the second location of the second annotation is selected, "next" button is disabled.

}

GUI_TEST_CLASS_DEFINITION(test_2701) {
//    1. Open {/data/samples/genbank/CVU55762.gb}.
//    Expected state: a document was added, circular view is opened

//    2. Click on Save circular view as image.
//    Expected state: Opened Export Image dialog

//    3. Select vector format (svg, pdf or ps)
//    Expected state: Quality tuning slider is not showed.

//    4. Select jpeg format
//    Expected state: Quality tuning slider is showed.
    GTFileDialog::openFile(os, dataDir + "/samples/Genbank/", "CVU55762.gb");
    ADVSingleSequenceWidget* wgt = GTUtilsSequenceView::getSeqWidgetByNumber(os);
    CHECK_SET_ERR( wgt != NULL, "No sequence widget");
    CHECK_SET_ERR( GTUtilsCv::isCvPresent(os, wgt), "No CV opened");

    class ImageQualityChecker : public Filler {
    public:
        ImageQualityChecker(U2OpStatus &os)
            : Filler(os, "ImageExportForm") {}
        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");

            QComboBox* formatsBox = dialog->findChild<QComboBox*>("formatsBox");
            QWidget* spin = dialog->findChild<QSpinBox*>("qualitySpinBox");

            GTComboBox::setIndexWithText(os, formatsBox, "svg");
            CHECK_SET_ERR( !spin->isVisible(), "Quality spin box is visible!");
            GTComboBox::setIndexWithText(os, formatsBox, "pdf");
            CHECK_SET_ERR( !spin->isVisible(), "Quality spin box is visible!");
            GTComboBox::setIndexWithText(os, formatsBox, "ps");
            CHECK_SET_ERR( !spin->isVisible(), "Quality spin box is visible!");

            GTComboBox::setIndexWithText(os, formatsBox, "jpg");
            CHECK_SET_ERR( spin->isVisible(), "Quality spin box not visible!");
            GTComboBox::setIndexWithText(os, formatsBox, "jpeg");
            CHECK_SET_ERR( spin->isVisible(), "Quality spin box not visible!");

            QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
            CHECK_SET_ERR(box != NULL, "buttonBox is NULL");
            QPushButton* button = box->button(QDialogButtonBox::Cancel);
            CHECK_SET_ERR(button !=NULL, "Cancel button is NULL");
            GTWidget::click(os, button);
        }
    };

    GTUtilsDialog::waitForDialog(os, new ImageQualityChecker(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< ADV_MENU_EXPORT << "Save circular view as image", GTGlobals::UseMouse));

    QWidget* circularView = GTWidget::findWidget(os, "CV_ADV_single_sequence_widget_0");
    CHECK_OP_SET_ERR(os, "Failed to open circular view!");
    GTWidget::click(os, circularView, Qt::RightButton);
}

namespace {
class test_2709_canceler: public TuxedoWizardFiller{
public:
    test_2709_canceler(U2OpStatus &os):TuxedoWizardFiller(os, "","","","", pairValList()){}
    void run(){
        GTWidget::click(os, getCancelButton(os));
    }
};
}

GUI_TEST_CLASS_DEFINITION(test_2709) {
//    1. Open tuxedo sample "no-new-transcripts"

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsDialog::waitForDialog(os, new ConfigureTuxedoWizardFiller(os, ConfigureTuxedoWizardFiller::no_new_transkripts,
                                                                     ConfigureTuxedoWizardFiller::singleReads));
    GTUtilsDialog::waitForDialog(os, new test_2709_canceler(os));
    GTUtilsWorkflowDesigner::addSample(os, "tuxedo");
    GTGlobals::sleep();
//    Expected state: "No novel junctions" tophat parameter set to true by default
    GTUtilsWorkflowDesigner::click(os, "Find Splice Junctions with TopHat");
    QString result = GTUtilsWorkflowDesigner::getParameter(os, "No novel junctions");
    CHECK_SET_ERR(result == "True", "No novel junctions parameter is " + result);

}

GUI_TEST_CLASS_DEFINITION(test_2711){
//    1. Open "Settings"->"preferences"->"External tools"

//    2. Set path to R tool as path to Rscript


    class test_2711DialogFiller : public CustomScenario {
    public:
        test_2711DialogFiller(){}
        virtual void run(U2::U2OpStatus &os){
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");

            QString rScriptPath = AppSettingsDialogFiller::getExternalToolPath(os, "Rscript");
            QString rPath = rScriptPath.left(rScriptPath.length() - QString("script").length());
            AppSettingsDialogFiller::setExternalToolPath(os, "Rscript", rPath);
            GTGlobals::sleep(500);

            bool valid = AppSettingsDialogFiller::isExternalToolValid(os, "Rscript");
            CHECK_SET_ERR(!valid, "Rscript is unexpectidly valid");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };
//    Expected state: Rscript doesn't pass validation
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new test_2711DialogFiller()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"action__settings"));
    GTMenu::showMainMenu(os, MWMENU_SETTINGS);
    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_2713) {
//    1. Open file {data/samples/Genbank/murine.gb}
    QDir().mkpath(sandBoxDir + "test_2713");
    GTFile::copy(os, dataDir + "samples/Genbank/murine.gb", sandBoxDir + "test_2713/murine.gb");
    GTFileDialog::openFile(os, sandBoxDir + "test_2713", "murine.gb");

//    2. Open file {data/samples/FASTA/human_T1.fa}
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

//    3. Drag and drop annotation object "NC_001363 features" from project view to sequence view
//    Expected state: the "Edit Object Relations" dialog has appeared
//    4. Press "OK"
//    Expected state: annotations has appeared on the sequence view
    GTUtilsDialog::waitForDialog(os, new CreateObjectRelationDialogFiller(os));
    GTUtilsProjectTreeView::dragAndDrop(os, GTUtilsProjectTreeView::findIndex(os, "NC_001363 features"), GTUtilsAnnotationsTreeView::getTreeWidget(os));

//    5. Open file {data/samples/Genbank/murine.gb} with text editor, then make some identical modification (i.e. delete and type the same character) and save file
//    Expected state: dialog about detected file modification has appeared in UGENE window
//    6. Press "Yes"
//    Expected state: "human_T1" view has disappeared from the "Bookmarks" list, "murine.gb" has been reloaded.
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));

    QFile murineFile(sandBoxDir + "test_2713/murine.gb");
    const bool opened = murineFile.open(QFile::Append);
    CHECK_SET_ERR(opened, "Can't open the file: " + sandBoxDir + "test_2713/murine.gb");
    murineFile.write(" ");
    murineFile.close();

    GTGlobals::sleep();

//    7. Open "human_T1" sequence view
//    Expected state: annotations from "murine.gb" present on the sequence view
    GTUtilsProjectTreeView::doubleClickItem(os, "human_T1.fa");
    GTUtilsAnnotationsTreeView::findFirstAnnotation(os);
}

GUI_TEST_CLASS_DEFINITION(test_2726) {
    //1. Open "COI.aln".
    //2. Select the second symbol at the first line.
    //3. Press backspace twice.
    //Expected state: undo and redo buttons are disabled.
    GTFileDialog::openFile(os, dataDir + "/samples/CLUSTALW/", "COI.aln");

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(1,0), QPoint(1,0));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["backspace"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["backspace"]);

    //Expected state : MSA view has opened, the "Undo" button is disabled.
    QAbstractButton *undoButton = GTAction::button(os, "msa_action_undo");
    CHECK_SET_ERR(!undoButton->isEnabled(), "'Undo' button is unexpectedly enabled");
    //Expected state : MSA view has opened, the "Redo" button is disabled.
    QAbstractButton *redoButton = GTAction::button(os, "msa_action_undo");
    CHECK_SET_ERR(!redoButton->isEnabled(), "'Redo' button is unexpectedly enabled");
}

GUI_TEST_CLASS_DEFINITION(test_2729) {
//    1. Open {_common_data/fasta/AMINO.fa}
//    Expected state: there is a "Graphs" button on the sequence toolbar, it is enabled.
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "AMINO.fa");
    QAbstractButton *graphsButton = GTAction::button(os, "GraphMenuAction", GTUtilsSequenceView::getSeqWidgetByNumber(os));
    CHECK_SET_ERR(NULL != graphsButton, "Graphs button is NULL");
    CHECK_SET_ERR(graphsButton->isEnabled(), "Graphs button is unexpectedly disabled");

//    2. Click the "Graphs" button.
//    Expected state: menu is shown.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()));
    GTWidget::click(os, graphsButton);

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_2730) {
/*  1. Open "_common_data/fasta/abcd.fa" as separate sequences in the sequence viewer.
    2. Click the "Automatic Annotation Highlighting" button on the first sequence's toolbar.
    3. Click the "Plasmid features" button.
    Expected: UGENE does not crash.
*/
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTFileDialog::openFile(os, testDir +"_common_data/fasta/", "abcd.fa");
    GTGlobals::sleep(1000);
    QWidget* parent=GTWidget::findWidget(os, "ADV_single_sequence_widget_0");
    CHECK_SET_ERR( parent != NULL, "ADV_single_sequence_widget_0 not found!");

    QWidget* menuAction=GTWidget::findWidget(os, "AutoAnnotationUpdateAction",parent);
    CHECK_SET_ERR( menuAction != NULL, "AutoAnnotationUpdateAction not found!");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Plasmid features"));
    GTWidget::click(os, menuAction);
    GTGlobals::sleep(500);
//Close file - UGENE does not crash.
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "abcd.fa"));
    GTMouseDriver::click( os );
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_2737) {
    //1. Open any sequence without annotations (e.g. "_common_data/fasta/AMINO.fa")
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "AMINO.fa");

    //2. Add few annotations with different names.
    GTUtilsAnnotationsTreeView::createAnnotation(os, "group", "name1", "1..10");
    GTUtilsAnnotationsTreeView::createAnnotation(os, "group", "name2", "11..20", false);
    GTUtilsAnnotationsTreeView::createAnnotation(os, "group", "name3", "21..30", false);

    //3. Open Annotation Highlighting tab.
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::AnnotationsHighlighting);

    //4. Delete all annotations one by one.
    GTUtilsAnnotationsTreeView::deleteItem(os, "name1");
    GTUtilsAnnotationsTreeView::deleteItem(os, "name2");
    GTUtilsAnnotationsTreeView::deleteItem(os, "name3");

    //Expected state: there is no annotations is annotation tree.
    QWidget *annotationsTree = GTWidget::findWidget(os, "OP_ANNOT_HIGHLIGHT_TREE");
    CHECK_SET_ERR(!annotationsTree->isVisible(), "Annotations tree is shown");
}

GUI_TEST_CLASS_DEFINITION(test_2737_1) {
    GTLogTracer l;
    //1. Open "murine.gb";
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "murine.gb");

    //2. Delete all annotations in random order;
    QTreeWidgetItem *annotation = NULL;
    while (NULL != (annotation = GTUtilsAnnotationsTreeView::findFirstAnnotation(os, GTGlobals::FindOptions(false)))) {
        GTUtilsAnnotationsTreeView::deleteItem(os, annotation);
    }

    //Expected state: no errors in the log
    CHECK_SET_ERR(!l.hasError(), "Errors in log");
}

GUI_TEST_CLASS_DEFINITION(test_2754) {
    //1. Open "murine.gb";
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "murine.gb");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new FindQualifierFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "find_qualifier_action"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "CDS"));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_2773) {

    // 1. Open file test/_common_data/cmdline/custom-script-worker-functions/translateTest/translateTest.uwl
    // 2. Set parameters:
    // input: _common_data/cmdline/DNA.fa
    // offset: sss
    // out: some/valid/path
    // 3. run sheme.
    // Expected state: UGENE doesn't crash, error message appears.
    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    GTFileDialog::openFile(os, testDir + "_common_data/cmdline/custom-script-worker-functions/translateTest/", "translateTest.uwl");

    GTUtilsWorkflowDesigner::click(os, "Read Sequence");
    GTUtilsWorkflowDesigner::setDatasetInputFile( os, testDir + "_common_data/cmdline/", "DNA.fa" );

    GTUtilsWorkflowDesigner::click(os, "translateTest");
    GTUtilsWorkflowDesigner::setParameter( os, "offset", "sss", GTUtilsWorkflowDesigner::textValue);

    GTUtilsWorkflowDesigner::click(os, "Write sequence");
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", "_common_data/cmdline/result_test_offset", GTUtilsWorkflowDesigner::textValue);

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTWidget::click(os,GTAction::button(os,"Run workflow"));
    GTGlobals::sleep(5000);

    GTUtilsLog::check(os, l);
}
GUI_TEST_CLASS_DEFINITION(test_2778) {
    //1. Use main menu : tools->align to reference->align short reads
    //2. Set input parameters
    //input sequence : _common_data / genome_aligner / chrY.fa
    //short reads : _common_data / genome_aligner / shortreads15Mb.fasta
    //mismatches allowed : checked
    //3. Press start

    //Expected state : the task should be finished without errors.

    GTLogTracer l;

    AlignShortReadsFiller::UgeneGenomeAlignerParams parameters(testDir + "_common_data/genome_aligner/",
        "chrY.fa", testDir + "_common_data/genome_aligner/", "shortreads15Mb.fasta", true);
    parameters.samOutput = false;
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, &parameters));

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Align to reference" << "Align short reads");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_2784) {
    GTLogTracer lt;

    //1. Open the file "data/samples/CLUSTALW/COI.aln"
    GTFileDialog::openFile(os, dataDir + "/samples/CLUSTALW/", "COI.aln");

    //Expected state : MSA view has opened, the "Undo" button is disabled.
    QAbstractButton *undoButton = GTAction::button(os, "msa_action_undo");
    CHECK_SET_ERR(!undoButton->isEnabled(), "'Undo' button is unexpectedly enabled");

    //2. Choose in the context menu{ Align->Align with MUSCLE... }
    //Expected state : The "Align with MUSCLE" dialog has appeared
    //3. Check the "Translation to amino when aligning" checkbox and press "Align"
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(41, 0), QPoint(43, 17));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    const QString initialRegionContent = GTClipboard::text(os);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);

    GTUtilsDialog::waitForDialog(os, new MuscleDialogFiller(os, MuscleDialogFiller::Default, true, true));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with muscle", GTGlobals::UseMouse));

    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(10, 5));
    GTMouseDriver::click(os, Qt::RightButton);

    //Expected state : Alignment task has started.After some time it finishes without errors
    //and alignment gets changed somehow.The "Undo" button becomes active
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsLog::check(os, lt);
    CHECK_SET_ERR(undoButton->isEnabled(), "'Undo' button is unexpectedly disabled");

    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(41, 0), QPoint(43, 17));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    const QString alignedRegionContent = GTClipboard::text(os);
    CHECK_SET_ERR(alignedRegionContent != initialRegionContent, "Alignment content has not been changed");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);

    //4. Click on the "Undo" button
    GTWidget::click(os, undoButton);

    //Expected state : Alignment has been restored to its initial state.The "Undo" button gets disabled,
    //the "Redo" has been enabled
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(41, 0), QPoint(43, 17));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    const QString undoneRegionContent = GTClipboard::text(os);
    CHECK_SET_ERR(undoneRegionContent == initialRegionContent, "Undo hasn't reverted changes");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);

    CHECK_SET_ERR(!undoButton->isEnabled(), "'Undo' button is unexpectedly enabled");

    QAbstractButton *redoButton = GTAction::button(os, "msa_action_redo");
    CHECK_SET_ERR(redoButton->isEnabled(), "'Redo' button is unexpectedly disabled");

    //5. Click on the "Redo" button
    GTWidget::click(os, redoButton);

    //Expected state : Alignment has been changed.The "Redo" button gets disabled,
    //the "Undo" has been enabled
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(41, 0), QPoint(43, 17));
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    const QString redoneRegionContent = GTClipboard::text(os);
    CHECK_SET_ERR(redoneRegionContent == alignedRegionContent, "Redo hasn't changed the alignment");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);

    CHECK_SET_ERR(!redoButton->isEnabled(), "'Redo' button is unexpectedly enabled");
    CHECK_SET_ERR(undoButton->isEnabled(), "'Undo' button is unexpectedly disabled");
}

GUI_TEST_CLASS_DEFINITION(test_2796) {
    //1. Open file "_common_data/fasta/fa2.fa"
    GTFileDialog::openFile(os, testDir + "_common_data/fasta", "fa2.fa");

    //2. Use main menu { Actions -> Analyse -> Find pattern[Smith-Waterman]... }
    //Expected state: the "Smith-Waterman Search" dialog has appeared
    //3. Press "Cancel" in the dialog
    //Expected state: dialog has closed
    SmithWatermanDialogFiller *filler = new SmithWatermanDialogFiller(os);
    filler->button = SmithWatermanDialogFiller::Cancel;
    GTUtilsDialog::waitForDialog(os, filler);
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Analyze" << "Find pattern [Smith-Waterman]", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
}

GUI_TEST_CLASS_DEFINITION( test_2801 ){
    //1. Open {_common_data/clustal/100_sequences.aln}.
    GTFileDialog::openFile(os, testDir + "_common_data/clustal/", "3000_sequences.aln");
    GTGlobals::sleep(30000);
    //2. Start MAFFT with default values.
    GTUtilsDialog::waitForDialog(os, new MAFFTSupportRunDialogFiller(os, new MAFFTSupportRunDialogFiller::Parameters()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with MAFFT", GTGlobals::UseMouse));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep(5000);
    //3. Cancel the align task.
    GTUtilsTaskTreeView::openView(os);
    GTUtilsTaskTreeView::checkTask(os, "Run MAFFT alignment task");
    GTUtilsTaskTreeView::cancelTask(os, "Run MAFFT alignment task");
    //Expected state: the task is cancelled, there is no MAFFT processes with its subprocesses (check for the "disttbfast" process)
    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.setReadChannelMode(QProcess::MergedChannels);
    process.start("wmic.exe /OUTPUT:STDOUT PROCESS get Caption");

    process.waitForStarted(1000);
    process.waitForFinished(1000);

    QByteArray list = process.readAll();
    CHECK_SET_ERR(!list.contains("disttbfast"), "disttbfast is presents in process lise");
}

GUI_TEST_CLASS_DEFINITION( test_2801_1 ){
    //1. Open {_common_data/clustal/100_sequences.aln}.
    GTFileDialog::openFile(os, testDir + "_common_data/clustal/", "3000_sequences.aln");
    GTGlobals::sleep(30000);
    //2. Start MAFFT with default values.
    GTUtilsDialog::waitForDialog(os, new MAFFTSupportRunDialogFiller(os, new MAFFTSupportRunDialogFiller::Parameters()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with MAFFT", GTGlobals::UseMouse));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep(20000);
    //3. Cancel the align task.
    GTUtilsTaskTreeView::openView(os);
    GTUtilsTaskTreeView::checkTask(os, "Run MAFFT alignment task");
    GTUtilsTaskTreeView::cancelTask(os, "Run MAFFT alignment task");
    //Expected state: the task is cancelled, there is no MAFFT processes with its subprocesses (check for the "disttbfast" process)
    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.setReadChannelMode(QProcess::MergedChannels);
    process.start("wmic.exe /OUTPUT:STDOUT PROCESS get Caption");

    process.waitForStarted(1000);
    process.waitForFinished(1000);

    QByteArray list = process.readAll();
    CHECK_SET_ERR(!list.contains("disttbfast"), "disttbfast is presents in process lise");
}

GUI_TEST_CLASS_DEFINITION( test_2808 ){
//    1. Open WD.
//    2. Add "Sequence Marker" element to the scene, select it.
//    Expected state: there are buttons on the parameters widget: "add", "edit" and "remove". The "add" button is enabled, other buttons are disabled.
//    3. Add a new marker group (click the "add" button and fill the dialog).
//    Expected state: a new group was added, there is no selection in the marker group list, the "add" button is enabled, other buttons are disabled.
//    4. Select the added group.
//    Expected state: the group is selected, all buttons are enabled.
//    5. Click the "remove" button.
//    Expected state: the group is removed (the list is empty), the "add" button is enabled, other buttons are disabled.

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    GTUtilsWorkflowDesigner::addAlgorithm(os, "Sequence Marker");
    GTUtilsWorkflowDesigner::click(os, "Sequence Marker");

    QToolButton* addButton = qobject_cast<QToolButton*>(GTWidget::findWidget(os, "addButton"));
    CHECK_SET_ERR( addButton != NULL, "AddButton not found!");

    QToolButton* editButton = qobject_cast<QToolButton*>(GTWidget::findWidget(os, "editButton"));
    CHECK_SET_ERR( editButton != NULL, "EditButton not found!");

    QToolButton* removeButton = qobject_cast<QToolButton*>(GTWidget::findWidget(os, "removeButton"));
    CHECK_SET_ERR( removeButton != NULL, "RemoveButton not found!");

    CHECK_SET_ERR( addButton->isEnabled(), "AddButton is disabled!");
    CHECK_SET_ERR( !editButton->isEnabled(), "EditButton is enabled!");
    CHECK_SET_ERR( !removeButton->isEnabled(), "AddButton is enabled!");

    class OkClicker : public Filler {
    public:
        OkClicker(U2OpStatus& _os) : Filler(_os, "EditMarkerGroupDialog"){}
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
    GTUtilsDialog::waitForDialog(os, new OkClicker(os));
    GTWidget::click(os, addButton);
    GTGlobals::sleep(2000);

    CHECK_SET_ERR( addButton->isEnabled(), "AddButton is disabled!");
    CHECK_SET_ERR( !editButton->isEnabled(), "EditButton is enabled!");
    CHECK_SET_ERR( !removeButton->isEnabled(), "AddButton is enabled!");

    QTableView* groupTable = qobject_cast<QTableView *>(GTWidget::findWidget(os, "markerTable"));
    CHECK_SET_ERR( groupTable != NULL, "MerkerTable not found");
    GTWidget::click(os, groupTable);

    QPoint p = GTTableView::getCellPosition(os, groupTable, 0, 0);
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::click(os);

    CHECK_SET_ERR( addButton->isEnabled(), "AddButton is disabled!");
    CHECK_SET_ERR( editButton->isEnabled(), "EditButton is disabled!");
    CHECK_SET_ERR( removeButton->isEnabled(), "AddButton is disabled!");

    GTWidget::click(os, removeButton);

    CHECK_SET_ERR( groupTable->model() != NULL, "Abstract table model is NULL");
    CHECK_SET_ERR( groupTable->model()->rowCount() == 0, "Marker table is not empty!");
}

GUI_TEST_CLASS_DEFINITION( test_2809 ){
//    1. Open WD.
//    2. Add a "Sequence Marker" element to the scene, select it.
//    3. Add several items to marker group list on the parameters widget.
//    Expected state: if all items are visible, there is no vertical scroll bar.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    GTUtilsWorkflowDesigner::addAlgorithm(os, "Sequence Marker");
    GTUtilsWorkflowDesigner::click(os, "Sequence Marker");

    QToolButton* addButton = qobject_cast<QToolButton*>(GTWidget::findWidget(os, "addButton"));
    CHECK_SET_ERR( addButton != NULL, "AddButton not found!");

    class OkClicker : public Filler {
    public:
        OkClicker(U2OpStatus& _os) : Filler(_os, "EditMarkerGroupDialog"){}
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
    GTUtilsDialog::waitForDialog(os, new OkClicker(os));
    GTWidget::click(os, addButton);

    QTableView* groupTable = qobject_cast<QTableView *>(GTWidget::findWidget(os, "markerTable"));
    CHECK_SET_ERR( groupTable != NULL, "MerkerTable not found");
    GTWidget::click(os, groupTable);

    QScrollBar* scroll = groupTable->verticalScrollBar();
    CHECK_SET_ERR( scroll != NULL, "Scroll bar is NULL");
    CHECK_SET_ERR( !scroll->isVisible(), "Scroll bar is visible!");
}

GUI_TEST_CLASS_DEFINITION(test_2811) {
//    1. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::toggleDebugMode(os);
    GTGlobals::sleep(1000);
    GTUtilsWorkflowDesigner::toggleBreakpointManager(os);

//    2. Open any workflow, create a breakpoint for any element.
    GTUtilsWorkflowDesigner::addSample(os, "Align sequences with MUSCLE");
    GTUtilsWorkflowDesigner::setBreakpoint(os, "Align with MUSCLE");

//    3. Open another workflow.
//    Expected state: breakpoints list is cleared.
    GTUtilsWorkflowDesigner::addSample(os, "Align sequences with MUSCLE");
    QStringList breakpointList = GTUtilsWorkflowDesigner::getBreakpointList(os);
    CHECK_SET_ERR(breakpointList.isEmpty(), "There are unexpected breakpoints");
}

GUI_TEST_CLASS_DEFINITION(test_2829) {
    //1) Open files "data/samples/Genbank/murine.gb" and "data/samples/Genbank/sars.gb" in separated views
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "murine.gb");
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "sars.gb");

    //2) Click on toolbar 'Build dotplot' button
    //3) In opened dialog click 'OK' button
    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os));
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
    GTGlobals::sleep();

    //4) Choose some annotation by left mouse button on the upper sequence view
    //Expected state: horisontal or vertical selection is shown on DotPlot
    QList<QTreeWidgetItem*> geneItems = GTUtilsAnnotationsTreeView::findItems(os, "gene", GTGlobals::FindOptions(false));
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, geneItems.at(1)));
    GTMouseDriver::click(os);

    //5) In second sequence view choose { Toggle view -> Remove sequence } on the toolbar
    //Expected state: DotPlot closed and UGENE didn't crash
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "remove_sequence"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggle_view_button_NC_001363", GTUtilsSequenceView::getSeqWidgetByNumber(os, 1)));
}

GUI_TEST_CLASS_DEFINITION( test_2853 ){
    Runnable *filler = new NCBISearchDialogFillerDeprecated(os, "rat");

    GTUtilsDialog::waitForDialog(os, filler);

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__SEARCH_GENBANK, GTGlobals::UseKey);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_2866) {
//    1. Use main menu { Tools -> Align to reference -> Align short reads }
//    Expected state: the "Align Sequencing Reads" dialog has appeared
//    2. Fill dialog: alignment method: Bowtie
//                    reference sequence: _common_data/e_coli/NC_008253.gff
//                    short reads: "_common_data/e_coli/e_coli_1000.fastq"
//       Click start button
//    Expected state: message box with "These files have the incompatible format" has appeared
//    3. Click "Yes"
//    Expected state: UGENE assembles reads without errors and the "Import SAM File" dialog has appeared
    GTLogTracer l;

    AlignShortReadsFiller::Parameters parameters(testDir + "_common_data/e_coli/",
                                                 "NC_008253.gff",
                                                 testDir + "_common_data/e_coli/",
                                                 "e_coli_1000.fastq",
                                                 AlignShortReadsFiller::Parameters::Bowtie);

    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, &parameters)) ;
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "Yes"));
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, sandBoxDir + "test_2866.ugenedb"));

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Align to reference" << "Align short reads");
    GTGlobals::sleep(20000);

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_2884) {
    //1. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    //2. Place Cuffdiff element on the scene
    GTUtilsWorkflowDesigner::addAlgorithm( os, "Test for Diff. Expression with Cuffdiff");
    CHECK_OP(os, );

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Test for Diff. Expression with Cuffdiff"));
    GTMouseDriver::click(os);
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::getParameter(os, "Multi read correct") == "False", "'Mate inner distance', Parameter value doesn't amtch");
}

GUI_TEST_CLASS_DEFINITION(test_2887) {
    //1. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    //2. Place Tophat element on the scene
    GTUtilsWorkflowDesigner::addAlgorithm( os, "Find Splice Junctions with TopHat");
    CHECK_OP(os, );
    //3. check "Mate inner distance" parameter is 50
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Find Splice Junctions with TopHat"));
    GTMouseDriver::click(os);
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::getParameter(os, "Mate inner distance") == "50", "'Mate inner distance', Parameter value doesn't amtch");
}

GUI_TEST_CLASS_DEFINITION(test_2891) {
    // 1. Open file "data/samples/workflow_samples/NGS/cistrome/chip_seq.uwl"
    // 2. Set input file for the "Read Tags" element to "test/_common_data/NIAID_pipelines/Chip-seq/input_data/chr2.bed"
    // 3. Press the "Validate workflow" button on the main toolbar
    // Expected state: the message box about workflow errors has appeared. The "Error list" tab has appeared below the workflow
    // 4. Press "OK"
    // Expected state: there is no messages about the "Read tags" element on the "Error list" tab
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    GTFileDialog::openFile(os, dataDir + "/workflow_samples/NGS/cistrome/", "chip_seq.uwl");
    GTGlobals::sleep();

    GTUtilsWorkflowDesigner::click(os, "Gets paths of files");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/NIAID_pipelines/Chip-seq/input_data/", "chr2.bed");

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTWidget::click(os,GTAction::button(os, "Validate workflow"));
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::checkErrorList(os, "Read Tags") == 0, "Errors count dont match, should be 0 validation errors");


}GUI_TEST_CLASS_DEFINITION(test_2891_1) {

    // 1. Open file "data/samples/workflow_samples/NGS/cistrome/chip_seq.uwl"
    // 2. Set input file for the "Read Tags" element to "test/_common_data/NIAID_pipelines/Chip-seq/input_data/some_image.png"
    // 3. Press the "Validate workflow" button on the main toolbar
    // Expected state: the message box about workflow errors has appeared. The "Error list" tab has appeared below the workflow
    // 4. Press "OK"
    // Expected state: there is a warning about possible incompatibilities of the "Read tags" element on the "Error list" tab
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    GTFileDialog::openFile(os, dataDir + "/workflow_samples/NGS/cistrome/", "chip_seq.uwl");
    GTGlobals::sleep();

    GTUtilsWorkflowDesigner::click(os, "Gets paths of files");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/NIAID_pipelines/Chip-seq/input_data/", "some_image.png");

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTWidget::click(os,GTAction::button(os, "Validate workflow"));
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::checkErrorList(os, "Read Tags") == 1, "Errors count dont match, should be 1 validation error");

}

GUI_TEST_CLASS_DEFINITION(test_2894){
//    1. Open {_common_data/clustal/100_sequences.aln}.
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "100_sequences.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Use context menu {Tree->Build Tree}.
//    Expected state: "Build phylogenetic tree" dialog has been appeared.
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, sandBoxDir + "test_2894_COI.nwk", 0, 0, true));
    GTWidget::click(os, GTAction::button(os, "Build Tree"));
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();
//    3. Run "Phylip Neighbor Joining" with default parameters.
//    Expected state: tree view has been appeared.
    GTWidget::findWidget(os, "treeView");
    QWidget* qt_toolbar_ext_button = GTWidget::findWidget(os, "qt_toolbar_ext_button",
                                                          GTWidget::findWidget(os, "100_sequences [m] 100_sequences"), GTGlobals::FindOptions(false));
//    4. Press refresh tree button on the tree's toolbar.
//    Expected state: "Calculating Phylogenetic Tree" task has been started.
    if(qt_toolbar_ext_button != NULL && qt_toolbar_ext_button->isVisible()){
        GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Refresh tree"));
        GTWidget::click(os, qt_toolbar_ext_button);
    }else{
        GTWidget::click(os, GTAction::button(os, "Refresh tree"));
    }

    GTUtilsTaskTreeView::checkTask(os, "Calculating Phylogenetic Tree");
//    5. Press refresh button again.
//    Expected state: a new refresh task is not started, the old one is in process.
    if(qt_toolbar_ext_button != NULL && qt_toolbar_ext_button->isVisible()){
        GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Refresh tree"));
        GTWidget::click(os, qt_toolbar_ext_button);
    }else{
        GTWidget::click(os, GTAction::button(os, "Refresh tree"));
    }
    GTGlobals::sleep(100);

    int num = GTUtilsTaskTreeView::countTasks(os, "Calculating Phylogenetic Tree");
    CHECK_SET_ERR(num == 1, QString("Wrong tasks number. Expected 1, actual: ").arg(num));
//    6. Close the tree view while the task is performed.
//    Expected state: UGENE doesn't crash, view is closed, task cancels.
    GTUtilsProjectTreeView::click(os, "test_2894_COI.nwk");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

}

GUI_TEST_CLASS_DEFINITION(test_2895){
    //1. Open "_common_data/fasta/amino_multy.fa" as multiple alignment.
    //2. Open "_common_data/fasta/amino_multy_ext.fa" as separate sequences.
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Separate));
    GTFileDialog::openFile(os, testDir+"_common_data/fasta/", "amino_multy_ext.fa");
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Join));
    GTFileDialog::openFile(os, testDir+"_common_data/fasta/", "amino_multy.fa");
    //3. Try to add to the amino_multy.fa document, any sequence from the amino_multy_ext.fa document.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_LOAD << "Sequence from current project"));
    GTUtilsDialog::waitForDialog(os, new ProjectTreeItemSelectorDialogFiller(os, "amino_multy_ext.fa", "chr1_gl000191_random Amino translation 0 direct"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();
    //Expected: an object will be added.
}

GUI_TEST_CLASS_DEFINITION(test_2897) {
    //    1. Open {data/samples/CLUSTALW/COI.aln}.
    GTFileDialog::openFile(os, dataDir + "/samples/CLUSTALW/", "COI.aln");

    //    2. Open options panel 'Highlighting' tab.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_HIGHLIGHTING"));

    QComboBox* combo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "highlightingScheme"));
    CHECK_SET_ERR(combo != NULL, "highlightingScheme not found!");
    int oldItemsNumber = combo->count();

    //    3. Create a new custom nucleotide color scheme.
    const QString colorSchemeName = getName() + "_NewScheme";
    GTUtilsDialog::waitForDialog( os, new PopupChooser( os, QStringList( ) << "Colors"
        << "Custom schemes" << "Create new color scheme" ) );
    GTUtilsDialog::waitForDialog( os, new NewColorSchemeCreator( os, colorSchemeName,
        NewColorSchemeCreator::nucl ) );
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));

    GTGlobals::sleep(500);
    GTUtilsDialog::waitForDialog( os, new PopupChooser( os, QStringList( ) << "Colors"
        << "Custom schemes" << colorSchemeName ) );
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));

    combo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "highlightingScheme"));
    CHECK_SET_ERR(combo != NULL, "highlightingScheme not found!");
    int newItemsNumber = combo->count();

    CHECK_SET_ERR(newItemsNumber == oldItemsNumber, "exportButton is disabled unexpectedly");
}

GUI_TEST_CLASS_DEFINITION(test_2900) {
//    1. Open "samples/Genbank/murine.gb".
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "murine.gb");

//    2. Sequence view context menu -> Analyze -> Restriction sites.
//    3. Press "OK".
//    Expected: the 8 regions of auto-annotations are created.
    const QStringList defaultEnzymes = QStringList() << "BamHI" << "BglII" << "ClaI" << "DraI" << "EcoRI" << "EcoRV" << "HindIII" << "PstI" << "SalI" << "SmaI" << "XmaI";
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ANALYSE" << "Find restriction sites"));
    GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, defaultEnzymes));
    GTMenu::showContextMenu(os, GTUtilsSequenceView::getSeqWidgetByNumber(os));

    GTGlobals::sleep(500);
    const int firstAnnotationsCount = GTUtilsAnnotationsTreeView::getAnnotationNamesOfGroup(os, "enzyme  (8, 0)").size();

//    4. Repeast 2-3.
//    Expected: there are still 8 regions of auto-annotations. Old regions are removed, new ones are added.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ANALYSE" << "Find restriction sites"));
    GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, defaultEnzymes));
    GTMenu::showContextMenu(os, GTUtilsSequenceView::getSeqWidgetByNumber(os));

    GTGlobals::sleep(500);
    const int secondAnnotationsCount = GTUtilsAnnotationsTreeView::getAnnotationNamesOfGroup(os, "enzyme  (8, 0)").size();

    CHECK_SET_ERR(firstAnnotationsCount == secondAnnotationsCount, QString("Annotations count differs: %1 annotations in the first time, %2 annotations in the second time").arg(firstAnnotationsCount).arg(secondAnnotationsCount));
}

GUI_TEST_CLASS_DEFINITION(test_2903) {
//    1. Open the attached file
    GTLogTracer l;
    GTFileDialog::openFile( os, testDir + "_common_data/regression/2903", "unknown_virus.fa" );

    GTUtilsDialog::waitForDialog(os, new RemoteBLASTDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ANALYSE"
                                                      << "Query NCBI BLAST database"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "render_area_virus_X"));
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Click on the Analyze->Query NCBI BLAST database context menu
//    3. Click on the Search button
//    Expected state: the task has been finished without errors and blast result appears
//    Current state: the following error appears: 'RemoteBLASTTask' task failed: Database couldn't prepare the response
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_2907) {
    //1. Open file _common_data/genbank/pBR322.gb
    GTFileDialog::openFile(os, testDir + "_common_data/genbank/", "pBR322.gb");

    //2. In annotations tree view go to element Auto - annotations->enzyme->EcoRI(0, 1)->EcoRI
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Restriction Sites"));
    QWidget* qt_toolbar_ext_button = GTWidget::findWidget(os, "qt_toolbar_ext_button",
                                                          GTWidget::findWidget(os, "mwtoolbar_activemdi"), GTGlobals::FindOptions(false));
    if(qt_toolbar_ext_button != NULL && qt_toolbar_ext_button->isVisible()){
        GTWidget::click(os, qt_toolbar_ext_button);
        GTGlobals::sleep(1000);
    }
    GTWidget::click(os, GTWidget::findWidget(os, "toggleAutoAnnotationsButton"));

    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "EcoRI"));

    //3. Stop cursor on EcoRI qualifier.Wait for tooltip
    //Expected state : tooltip will appear
    GTUtilsToolTip::checkExistingToolTip(os, "db_xref");
}

GUI_TEST_CLASS_DEFINITION(test_2910) {
    // 1. Open {data/samples/FASTA/human_T1.fa}.
    GTFileDialog::openFile( os, dataDir + "samples/FASTA", "human_T1.fa" );

    // 2. Press Ctrl+A.
    // 3. Select a "Multiple Range Selection" mode, enter the region: 10000..15000
    // 4. Click the "Go" button.

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, "10000..15000"));
    GTKeyboardDriver::keyClick(os, 'A', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);

    // TODO: Expected state: the region is selected, there is a label "[5001 bp]" on the pan view.
}
GUI_TEST_CLASS_DEFINITION(test_2910_1) {
    // 1. Open {data/samples/FASTA/human_T1.fa}.
    GTFileDialog::openFile( os, dataDir + "samples/FASTA", "human_T1.fa" );

    // 2. Press Ctrl+A.
    // 3. Select a "Multiple Range Selection" mode, enter the region: 2000..5000,100000..110000
    // 4. Click the "Go" button.

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, "2000..5000,100000..110000"));
    GTKeyboardDriver::keyClick(os, 'A', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);

    // TODO: Expected state: the region is selected, there is a label "[3001 bp]" and "[10001 bp]" on the pan view.
}

GUI_TEST_CLASS_DEFINITION(test_2910_2) {
    // 1. Open {data/samples/FASTA/human_T1.fa}.
    GTFileDialog::openFile( os, dataDir + "samples/FASTA", "human_T1.fa" );
    GTGlobals::sleep(2000);

    // 2. Press Ctrl+A.
    // Expected state: a "Region Selection" dialog appeared.
    // 3. Select a "Multiple Range Selection" mode, enter the region: 0..5000
    // Expected state: the region is invalid, a "Go" button is disabled.
    class CancelClicker : public Filler {
    public:
        CancelClicker(U2OpStatus& _os) : Filler(_os, "RangeSelectionDialog"){}
        virtual void run() {
            QWidget *w = QApplication::activeWindow();
            CHECK(NULL != w, );
            QRadioButton *multipleButton = w->findChild<QRadioButton*>("miltipleButton");
            CHECK_SET_ERR(multipleButton != NULL, "RadioButton \"miltipleButton\" not found");
            GTRadioButton::click(os, multipleButton);

            QLineEdit *regionEdit= w->findChild<QLineEdit*>("multipleRegionEdit");
            CHECK_SET_ERR(regionEdit != NULL, "QLineEdit \"multipleRegionEdit\" not foud");
            GTLineEdit::setText(os, regionEdit, "0..5000");

            QDialogButtonBox *buttonBox = w->findChild<QDialogButtonBox*>(QString::fromUtf8("buttonBox"));
            CHECK(NULL != buttonBox, );
            QPushButton *button = buttonBox->button(QDialogButtonBox::Cancel);
            CHECK(NULL != button, );
            QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
            CHECK_SET_ERR(!okButton->isEnabled(), "OK button is unexpectedly enabled");
            GTWidget::click(os, button);
        }
    };
    GTUtilsDialog::waitForDialog(os, new CancelClicker(os));
    GTKeyboardDriver::keyClick(os, 'A', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);

}
GUI_TEST_CLASS_DEFINITION(test_2910_3) {
    // 1. Open {data/samples/FASTA/human_T1.fa}.
    GTFileDialog::openFile( os, dataDir + "samples/FASTA", "human_T1.fa" );
    GTGlobals::sleep(2000);

    // 2. Press Ctrl+A.
    // Expected state: a "Region Selection" dialog appeared.
    // 3. Select a "Multiple Range Selection" mode, enter the region: 1..199951
    // Expected state: the region is invalid, a "Go" button is disabled.
    class CancelClicker : public Filler {
    public:
        CancelClicker(U2OpStatus& _os) : Filler(_os, "RangeSelectionDialog"){}
        virtual void run() {
            QWidget *w = QApplication::activeWindow();
            CHECK(NULL != w, );
            QRadioButton *multipleButton = w->findChild<QRadioButton*>("miltipleButton");
            CHECK_SET_ERR(multipleButton != NULL, "RadioButton \"miltipleButton\" not found");
            GTRadioButton::click(os, multipleButton);

            QLineEdit *regionEdit= w->findChild<QLineEdit*>("multipleRegionEdit");
            CHECK_SET_ERR(regionEdit != NULL, "QLineEdit \"multipleRegionEdit\" not foud");
            GTLineEdit::setText(os, regionEdit, "1..199951");

            QDialogButtonBox *buttonBox = w->findChild<QDialogButtonBox*>(QString::fromUtf8("buttonBox"));
            CHECK(NULL != buttonBox, );
            QPushButton *button = buttonBox->button(QDialogButtonBox::Cancel);
            CHECK(NULL != button, );
            QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
            CHECK_SET_ERR(!okButton->isEnabled(), "OK button is unexpectedly enabled");
            GTWidget::click(os, button);
        }
    };
    GTUtilsDialog::waitForDialog(os, new CancelClicker(os));
    GTKeyboardDriver::keyClick(os, 'A', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_2923) {
    GTUtilsMdi::click(os, GTGlobals::Close);
    //1. Open "samples/FASTA/human_T1.fa".
    //Expected: the sequence view is opened.
    //2. Press Ctrl+W.
    //Expected: the sequence view is closed.
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTUtilsMdi::activeWindow(os);

    GTKeyboardDriver::keyClick(os, 'w', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    QWidget *mdi = GTUtilsMdi::activeWindow(os, GTGlobals::FindOptions(false));
    CHECK_SET_ERR(NULL == mdi, "Sequence view is not closed");
}

GUI_TEST_CLASS_DEFINITION(test_2924) {
    //1. Open "_common_data/scenarios/_regression/2924/human_T1_cutted.fa".
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/_regression/2924/", "human_T1_cutted.fa");

    //2. Open "_common_data/scenarios/_regression/2924/MyDocument_3.gb".
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/_regression/2924/", "MyDocument_3.gb");

    //3. Drag'n'drop the annotation object from the project to the sequence view.
    //Expected: the dialog appears.
    //4. Click "OK".
    //Expected : the annotations are shown on the sequence view.
    GTUtilsDialog::waitForDialog(os, new CreateObjectRelationDialogFiller(os));
    GTUtilsProjectTreeView::dragAndDrop(os, GTUtilsProjectTreeView::findIndex(os, "Annotations"), GTUtilsAnnotationsTreeView::getTreeWidget(os));

    //5. Click the "Shown circular view" button on the sequence toolbar.
    //Expected : the circular view is shown.
    ADVSingleSequenceWidget *seqWidget = GTUtilsSequenceView::getSeqWidgetByNumber(os);
    GTUtilsCv::cvBtn::click(os, seqWidget);

    //6. Click the "Zoom in" button several times until it becomes disabled.
    //Expected : UGENE does not crash.
    QWidget *zoomInButton = GTWidget::findWidget(os, "tbZoomIn_human_T1_cutted [s] human_T1 (UCSC April 2002 chr7:115977709-117855134)");

    while (zoomInButton->isEnabled()) {
        GTWidget::click(os, zoomInButton);
        GTGlobals::sleep(500);
    }
}

GUI_TEST_CLASS_DEFINITION(test_2930){
    GTUtilsDialog::waitForDialog(os, new DocumentProviderSelectorDialogFiller
                                 (os, DocumentProviderSelectorDialogFiller::AssemblyBrowser));
    GTUtilsDialog::waitForDialog(os, new ConvertAceToSqliteDialogFiller(os, sandBoxDir + "test_2930"));
    GTFileDialog::openFile(os, dataDir+"samples/ACE", "K26.ace");

    GTFileDialog::openFile(os, dataDir+"samples/FASTA", "human_T1.fa");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_2951) {
    //1. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Click the "Scripting mode" tool button -> Show scripting options.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show scripting options"));
    GTWidget::click(os, GTAction::button(os, GTAction::findActionByText(os, "Scripting mode")));

    //3. Create the workflow: "Read sequence" -> "Write sequence".
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Read Sequence");
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Write Sequence");
    GTUtilsWorkflowDesigner::connect(os, GTUtilsWorkflowDesigner::getWorker(os, "Read Sequence"), GTUtilsWorkflowDesigner::getWorker(os, "Write Sequence"));

    //4. Set the input sequence: _common_data/fasta/abcd.fa.
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read sequence"));
    GTMouseDriver::click(os);
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/fasta/", "abcd.fa");

    //5. Set the correct output sequence.
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Write sequence"));
    GTMouseDriver::click(os);
    QString outFile = GUrl(sandBoxDir + "test_2951.gb").getURLString();
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", outFile, GTUtilsWorkflowDesigner::textValue);

    //6. Edit a script for "Sequence count limit":
    //    printToLog("test");
    //    1
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Read Sequence"));
    GTMouseDriver::click(os);
    GTUtilsDialog::waitForDialog(os, new ScriptEditorDialogFiller(os, "", "printToLog(\"test message\");\n1"));
    GTUtilsWorkflowDesigner::setParameterScripting(os, "Sequence count limit", "user script");

    //Expected: the result file contains only the first sequence from the input file; there is a message "test" in the script details log.
    GTWidget::click(os, GTAction::button(os, "Run workflow"));
    GTUtilsTaskTreeView::waitTaskFinished(os);
    bool printed = GTLogTracer::checkMessage("test message");
    CHECK_SET_ERR(printed, "No message in the log");

    GTUtilsProject::openFiles(os, outFile);
    QModelIndex objIdx = GTUtilsProjectTreeView::findIndex(os, "d");
    QTreeView *tree = GTUtilsProjectTreeView::getTreeView(os);
    int objectsCount = tree->model()->rowCount(objIdx.parent());
    CHECK_SET_ERR(1 == objectsCount, "More than one objects in the result");
}

GUI_TEST_CLASS_DEFINITION(test_2962_1) {
//    1. Open "_common_data/scenarios/_regression/2924/human_T1_cutted.fa".
//    2. Click the "Shown circular view" button on the sequence toolbar.
//    Expected: the circular view is shown.
//    3. Unload project.
//    4. Repeat 1,2.
//    Expected: UGENE does not crash.

    GTLogTracer l;
    ADVSingleSequenceWidget *seqWidget =
            GTUtilsProject::openFileExpectSequence(os,
                                                   testDir + "_common_data/scenarios/_regression/2924",
                                                   "human_T1_cutted.fa",
                                                   "human_T1 (UCSC April 2002 chr7:115977709-117855134)");
    GTUtilsCv::cvBtn::click(os, seqWidget);

    CHECK_SET_ERR(GTUtilsCv::cvBtn::isChecked(os, seqWidget), "Unexpected state of CV button!");
    CHECK_SET_ERR(GTUtilsCv::isCvPresent(os, seqWidget), "Unexpected state of CV widget!");

    GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECTSUPPORT__CLOSE_PROJECT, GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_FILE);

    seqWidget = GTUtilsProject::openFileExpectSequence(os,
                                                       testDir + "_common_data/scenarios/_regression/2924",
                                                       "human_T1_cutted.fa",
                                                       "human_T1 (UCSC April 2002 chr7:115977709-117855134)");
    GTUtilsCv::cvBtn::click(os, seqWidget);
    CHECK_SET_ERR(GTUtilsCv::cvBtn::isChecked(os, seqWidget), "Unexpected state of CV button!");
    CHECK_SET_ERR(GTUtilsCv::isCvPresent(os, seqWidget), "Unexpected state of CV widget!");

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_2962_2) {
//    Open "_common_data/fasta/DNA.fa".
//    Remove the second sequence object from the document.
//    Click the "Toggle circular views" button on the main toolbar.
//    Expected state: the circular view is toggled it is possible.
//    Current state: UGENE crashes.

    GTLogTracer l;

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta", "DNA.fa");

    QPoint p = GTUtilsProjectTreeView::getItemCenter(os, "GXL_141618");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::click(os);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Delete"]);
    GTUtilsCv::commonCvBtn::click(os);

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_2971) {
/*  1. Open "COI.aln".
    2. Context menu -> Add -> Sequence from file...
    3. Choose a file with a large sequence (~50mb), e.g human chr21.
    Expected: UGENE does not crash.
*/
    GTFileDialog::openFile(os, dataDir +"samples/CLUSTALW/", "COI.aln");
    GTGlobals::sleep(500);

    GTWidget::click(os, GTAction::button(os, "Show overview"));//needed to swith off rendering overview
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/regression/2971", "hg18_21.fa" ));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_LOAD_SEQ" << "Sequence from file"));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep(500);
    GTUtilsTaskTreeView::waitTaskFinished(os);
//Close file - UGENE does not crash.
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI.aln"));
    GTMouseDriver::click( os );
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_2972){
//    1. Open "samples/FASTA/human_T1.fa".
    GTLogTracer l;
    GTFileDialog::openFile(os, dataDir +"samples/FASTA/", "human_T1.fa");
//    2. Click the menu Tools -> HMMER tools -> HMM3 -> Search with HMM3 phmmer.
    GTUtilsDialog::waitForDialog(os, new UHMM3PhmmerDialogFiller(os, dataDir + "samples/Newick/COI.nwk"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "HMMER tools"
                                                       << "HMMER3 tools"
                                                       << "Search with HMM3 phmmer"));
    GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTGlobals::sleep();

    CHECK_SET_ERR(l.hasError(), "no error in log");
    QString error = l.getError();
    QString expectedError = "No dna sequence objects found in document querySeq sequence";

    CHECK_SET_ERR(error.contains(expectedError), "actual error is " + error);
//    3. Choose the query sequence file: any non-sequence format file (e.g. *.mp3).
//    4. Click search.
//    Expected state: the search task fails, UGENE does not crash.

}

GUI_TEST_CLASS_DEFINITION(test_2975) {
//    1. Open "_common_data/fasta/all_alphabet.fa" as multiple alignment.
    GTUtilsProject::openMultiSequenceFileAsMalignment(os, testDir + "_common_data/fasta", "all_alphabet.fa");

//    2. Open the "Pairwise alignment" options panel tab.
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::PairwiseAlignment);

//    3. Set the first sequence as the first sequence and the second sequence as the second sequence, select the "Smith-Waterman" algorithm.
//    Expected state: align button is blocked
    GTUtilsOptionPanelMsa::addFirstSeqToPA(os, "SEQUENCE_1");
    GTUtilsOptionPanelMsa::addSecondSeqToPA(os, "SEQUENCE_2");
    GTUtilsOptionPanelMsa::setPairwiseAlignmentAlgorithm(os, "Smith-Waterman");

    QPushButton *alignButton = GTUtilsOptionPanelMsa::getAlignButton(os);
    CHECK_SET_ERR(NULL != alignButton, "Align button is NULL");
    CHECK_SET_ERR(!alignButton->isEnabled(), "Align button is unexpectedly enabled");
}

GUI_TEST_CLASS_DEFINITION(test_2981) {
//    1. Open "data/samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "/samples/CLUSTALW/", "COI.aln");
//    2. Click a "Build Tree" button on the main toolbar.
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, sandBoxDir + "test_3276_COI.wnk", 0, 0, true));
    GTWidget::click(os, GTAction::button(os, "Build Tree"));
    GTGlobals::sleep(1000);
//    Expected state: a "Build Phyligenetic Tree" dialog appears.

//    3. Set any acceptable path and build a tree with default parameters.
//    Expected state: the tree is shown in the MSA Editor.

//    4. Click a "Layout" button on the tree view toolbar, select a "Circular" menu item.
    QComboBox* layoutCombo = GTWidget::findExactWidget<QComboBox*>(os, "layoutCombo");
    GTComboBox::setIndexWithText(os, layoutCombo,"Circular");
    GTGlobals::sleep(500);
//    Expected state: the tree becomes circular.
    QGraphicsView* treeView = GTWidget::findExactWidget<QGraphicsView*>(os, "treeView");
    int initW = treeView->rect().width();
    GTGlobals::sleep(500);
//    5. Hide/show a project view.
    GTKeyboardDriver::keyClick(os, '1', GTKeyboardDriver::key["alt"]);
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os, '1', GTKeyboardDriver::key["alt"]);
    GTGlobals::sleep(500);
//    Expected state: the tree size is not changed.
    int finalW = treeView->rect().width();
    CHECK_SET_ERR(finalW == initW, QString("initial: %1, final: %2").arg(initW).arg(finalW));

}

GUI_TEST_CLASS_DEFINITION(test_2987) {
//      1. Open "_common_data/fasta/RAW.fa".
//      2. Create a complement annotation.
//      3. Select {Actions -> Export -> Export annotations...} in the main menu.
//      4. Fill the dialog:
//      "Export to file" - any valid file;
//      "File format" - csv;
//      "Save sequences under annotations" - checked
//      and click the "Ok" button.
//      Expected state: annotations are successfully exported.
    GTLogTracer logTracer;

    GTFileDialog::openFile(os, testDir + "_common_data/fasta", "RAW2.fa");

    QDir().mkpath(sandBoxDir + "test_3305");
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "misc_feature", "complement(1..5)", sandBoxDir + "test_2987/test_2987.gb"));
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "create_annotation_action"));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << "ep_exportAnnotations2CSV"));
    GTUtilsDialog::waitForDialog(os, new ExportAnnotationsFiller(sandBoxDir + "test_2987/test_2987", ExportAnnotationsFiller::csv, os));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "test_2987.gb"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    const QFile csvFile(sandBoxDir + "test_2987/test_2987");
    CHECK_SET_ERR(!(csvFile.exists() && csvFile.size() == 0), "An empty file exists");
}

GUI_TEST_CLASS_DEFINITION(test_2991) {
/*  1. Open file _common_data/alphabets/extended_amino_1000.fa
    Expected state: Alphabet of opened sequence must be [amino ext]
*/
    GTFileDialog::openFile(os, testDir +"_common_data/alphabets/", "extended_amino_1000.fa");
    GTGlobals::sleep(1000);
    QWidget* w=GTWidget::findWidget(os, "ADV_single_sequence_widget_0");
    QLabel* label = qobject_cast<QLabel*>(GTWidget::findWidget(os, "nameLabel",w));
    CHECK_SET_ERR(label->text().contains("[amino ext]"), QString("Unexpected label of sequence name: %1, must contain %2").arg(label->text()).arg("[amino ext]"));
}

GUI_TEST_CLASS_DEFINITION(test_2998) {
    // 1. Open human_T1.fa
    GTLogTracer l;
    GTFileDialog::openFile(os, dataDir +"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(1000);

    // 2. Find any pattern
    GTUtilsOptionsPanel::runFindPatternWithHotKey("TTTTTAAAAA", os);

    // Expected state: the task will finished without errors.
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3014) {
    //1. Open the _common_data/scenarios/_regression/3014/pdb2q41.ent.gz
    GTLogTracer l;
    GTFileDialog::openFile(os, testDir +"_common_data/scenarios/_regression/3014/", "pdb2q41.ent.gz");
    GTGlobals::sleep(10000);

    // 2. In context menu go to 'Molecular surface'->'SES'
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Molecular Surface" << "SES"));
    QWidget *widget3d = GTWidget::findWidget(os, "1-2Q41");
    GTWidget::click(os, widget3d, Qt::RightButton);
    GTGlobals::sleep(10000);

    //    Expected state: Moleculaar surface calculated and showed. Program not crached.
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3017) {
    //1. Open 'HIV-1.aln';
    //2. Select few columns;
    //3. Run ClastulW, ClustalO, Mafft or T-Coffee alignment task;
    //4. Try to change an alignment while the task is running: move region, delete region etc.;
    //Current state: there is no results of your actions because msa is blocked, overview is not recalculated.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "HIV-1.aln");

    GTUtilsDialog::waitForDialog(os, new MuscleDialogFiller(os, MuscleDialogFiller::Refine));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with muscle", GTGlobals::UseMouse));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();

    GTUtilsMSAEditorSequenceArea::click(os, QPoint(1,1));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);

    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();
    QString clipboardText = GTClipboard::text(os);
    CHECK_SET_ERR("T" == clipboardText, "Alignment is not locked");
}

GUI_TEST_CLASS_DEFINITION(test_3031){
    //    Open "data/samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    //    Open "Statistics" options panel tab.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_SEQ_STATISTICS_WIDGET"));
    //    Set "Montatna_montana" reference sequence.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-4, 4));
    GTWidget::click(os, GTWidget::findWidget(os, "addSeq"));
    //    Delete "Montana_montana" sequence
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-4, 4));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    //    5. Right click on any sequence name
    //    Expected state: action "Set this sequence as reference" is visible, "Unset reference sequence" is invisible
}

GUI_TEST_CLASS_DEFINITION(test_3034) {
//    1. Open "samples/FASTA/human_T1.fa".
//    2. Right click on the document -> Add -> Add object to document.
//    Expected: the dialog will appear. There are no human_T1 objects.
    GTLogTracer l;
    GTFileDialog::openFile( os, dataDir + "samples/FASTA", "human_T1.fa" );

    GTUtilsDialog::waitForDialog( os, new ProjectTreeItemSelectorDialogFiller(os, QMap<QString, QStringList>(),
                                                                              QSet<GObjectType>(),
                                                                              ProjectTreeItemSelectorDialogFiller::Separate, 0));
    GTUtilsDialog::waitForDialog( os, new PopupChooser( os, QStringList( ) << ACTION_PROJECT__ADD_MENU
                                                        << ACTION_PROJECT__ADD_OBJECT) );
    QPoint docCenter = GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa");
    GTMouseDriver::moveTo(os, docCenter);
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3052) {
//    1. Open "_common_data/bam/chrM.sorted.bam".
//    Expected state: an "Import BAM file" dialog appears.

//    2. Import the assembly somewhere to the "1.ugenedb" file.
//    Expected state: the assembly is imported, a view opens.

//    3. Close the view.
//    4. Open "_common_data/bam/scerevisiae.bam".
//    Expected state: an "Import BAM file" dialog appears.

//    5. Select the same file as for "chrM.sorted.bam". Try to start import.
//    Expected state: a message box appears, it allows to replace the file, to append appent to the file or to cancel operation.

//    6. Select the "Append" option.
//    Expected state: the assembly is imported, a view opens.

//    7. Remove the the first assembly object from the document in the project view.
//    Expected state: the object is removed, there is no errors in the log.

//    8. Remove the document from the project view (or just unload it - test_3052_1). Open it again.
//    Expected state: there is one assembly object in it.

    GTLogTracer l;

    QString ugenedbFileName = testDir + "_common_data/scenarios/sandbox/test_3052.ugenedb";
    QString docName = "test_3052.ugenedb";

    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, ugenedbFileName));
    GTFileDialog::openFile( os, testDir + "_common_data/bam", "chrM.sorted.bam" );

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsMdi::closeWindow(os, "test_3052 [as] chrM");

    GTUtilsDialog::waitForDialog( os, new MessageBoxDialogFiller( os, "Append" ) );
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, ugenedbFileName));
    GTFileDialog::openFile( os, testDir + "_common_data/bam", "scerevisiae.bam" );

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "chrM"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

    GTUtilsDocument::removeDocument(os, docName);
    GTFileDialog::openFile( os, testDir + "_common_data/scenarios/sandbox", "test_3052.ugenedb" );

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3052_1) {
    GTLogTracer l;

    QString ugenedbFileName = testDir + "_common_data/scenarios/sandbox/test_3052.ugenedb";
    QString docName = "test_3052.ugenedb";

    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, ugenedbFileName));
    GTFileDialog::openFile( os, testDir + "_common_data/bam", "chrM.sorted.bam" );

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsMdi::closeWindow(os, "test_3052 [as] chrM");

    GTUtilsDialog::waitForDialog( os, new MessageBoxDialogFiller( os, "Append" ) );
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, ugenedbFileName));
    GTFileDialog::openFile( os, testDir + "_common_data/bam", "scerevisiae.bam" );

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "chrM"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    GTUtilsDocument::unloadDocument(os, docName, false);
    GTGlobals::sleep();
    GTUtilsDocument::loadDocument(os, docName);

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3073) {
//    1. Open "human_T1.fa";
//    2. Create few annotations (new file MyDocument_n.gb appeared);
//    3. Save the project with these files and relaunch UGENE;
//    4. Open the project and open either sequence or annotation file;
//    Expected state: both files are loaded;
    GTLogTracer l;
    GTFileDialog::openFile( os, dataDir + "samples/FASTA", "human_T1.fa" );

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "group", "annotation_1",
                                                                      "10..20", sandBoxDir + "test_3073.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "create_annotation_action"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "render_area_human_T1 (UCSC April 2002 chr7:115977709-117855134)"));

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "group", "annotation_2","10000..100000"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "create_annotation_action"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "render_area_human_T1 (UCSC April 2002 chr7:115977709-117855134)"));

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "group", "annotation_3", "120000..180000"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "create_annotation_action"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "render_area_human_T1 (UCSC April 2002 chr7:115977709-117855134)"));

    GTUtilsDocument::saveDocument(os, "test_3073.gb");
    GTUtilsDialog::waitForDialog(os, new SaveProjectAsDialogFiller(os, "proj_test_3073", sandBoxDir, "proj_test_3073"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECTSUPPORT__SAVE_AS_PROJECT, GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_FILE);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECTSUPPORT__CLOSE_PROJECT, GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_FILE);
    GTGlobals::sleep();

    GTFileDialog::openFile( os, sandBoxDir, "proj_test_3073.uprj" );
    GTGlobals::sleep();

    GTUtilsDocument::loadDocument(os, "human_T1.fa");
    CHECK_SET_ERR( GTUtilsDocument::isDocumentLoaded(os, "test_3073.gb"), "Annotation file is not loaded!");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECTSUPPORT__CLOSE_PROJECT, GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_FILE);
    GTGlobals::sleep();

    GTFileDialog::openFile( os, sandBoxDir, "proj_test_3073.uprj" );
    GTGlobals::sleep();

    GTUtilsDocument::loadDocument(os, "test_3073.gb");
    CHECK_SET_ERR( GTUtilsDocument::isDocumentLoaded(os, "human_T1.fa"), "Sequence file is not loaded!");

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3074) {
//    Disconnect from the shared database with opened msa from this database

//    1. Connect to the "ugene_gui_test" database.

//    2. Double click the object "/view_test_0002/[m] COI".
//    Expected: the msa is opened in the MSA Editor.

//    3. Remove the database document from the project tree view.
//    Expected state: the MSA Editor is closed, UGENE doesn't crash.

    GTLogTracer logTracer;

    Document* databaseDoc = GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    GTUtilsSharedDatabaseDocument::openView(os, databaseDoc, "/view_test_0002/COI");

    QWidget* msaView = GTWidget::findWidget(os, " [m] COI");
    CHECK_SET_ERR(NULL != msaView, "View wasn't opened");

    GTUtilsSharedDatabaseDocument::disconnectDatabase(os, databaseDoc);

    GTUtilsLog::check(os, logTracer);
}

GUI_TEST_CLASS_DEFINITION(test_3092) {
//    1. Open "data/samples/FASTA/human_T1.fa".

//    2. Do {Actions -> Analyze -> Query with BLAST+...} in the main menu.
//    Expected state: a dialog "Request to Local BLAST Database" appears, UGENE doesn't crash.

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsDialog::waitForDialog(os, new BlastAllSupportDialogFiller(BlastAllSupportDialogFiller::Parameters(), os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "query_with_blast+", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
}

GUI_TEST_CLASS_DEFINITION(test_3102) {
    //1. Go to 'File->Connect to shared database...'
    //    Expected state: Showed dialog 'Shared Databases Connections'
    //    In list of connections showed 'UGENE public database'
    GTLogTracer logTracer;

    Document* databaseDoc = GTUtilsSharedDatabaseDocument::connectToUgenePublicDatabase(os);
    CHECK_SET_ERR(NULL != databaseDoc, "UGENE public databased connection error");
}

GUI_TEST_CLASS_DEFINITION(test_3103) {
    //1. Go to 'File->Connect to shared database...'
    //Expected state: Showed dialog 'Shared Databases Connections'
    //2. Click on 'Add' button
    //Expected state: Showed dailog 'Connection Settings' and 'Port' field filled with port 3306
    {
        QList<SharedConnectionsDialogFiller::Action> actions;
        actions << SharedConnectionsDialogFiller::Action(SharedConnectionsDialogFiller::Action::ADD);
        GTUtilsDialog::waitForDialog(os, new SharedConnectionsDialogFiller(os, actions));
    }
    {
        EditConnectionDialogFiller::Parameters params;
        params.checkDefaults = true;
        params.accept = false;
        GTUtilsDialog::waitForDialog(os, new EditConnectionDialogFiller(os, params, EditConnectionDialogFiller::FROM_SETTINGS));
    }
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__ACCESS_SHARED_DB);
}

GUI_TEST_CLASS_DEFINITION(test_3112) {
//     1. Open "_common_data\scenarios\msa\big.aln".
//     Expected state: the MSA editor is shown, there is an overview.
//     2. Click the "Overview" button on the main toolbar.
//     Expected state: the overview widget hides, the render task cancels.
//     Current state: the overview widget hides, the render task still works. If you change the msa (insert a gap somewhere), a safe point triggers.
    GTFileDialog::openFile(os, testDir + "_common_data/clustal/", "10000_sequences.aln");

    GTGlobals::sleep(20000); //in case of failing test try to increase this pause

    QToolButton* button = qobject_cast<QToolButton*>(GTAction::button(os, "Show overview"));
    CHECK_SET_ERR(button->isChecked(), "Overview button is not pressed");

    GTWidget::click(os, button);//uncheck
    GTWidget::click(os, button);//check
    GTWidget::click(os, button);//uncheck
    GTGlobals::sleep();
    CHECK_SET_ERR(0 == GTUtilsTaskTreeView::getTopLevelTasksCount(os), "There are unfinished tasks");
}

GUI_TEST_CLASS_DEFINITION(test_3124) {
    // 1. Connect to a shared database.
    // 2. Right click on the document->Add->Import to the database.
    // 3. Click "Add files".
    // 4. Choose "data/samples/Genbank/PBR322.gb".
    // 5. Click "Import".
    // Expected state : the file is imported, there are no errors in the log.

    GTLogTracer logTracer;
    Document* databaseDoc = GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);
    CHECK_OP(os, );

    GTUtilsSharedDatabaseDocument::importFiles(os, databaseDoc, "/test", QStringList() << dataDir + "samples/Genbank/PBR322.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsLog::check(os, logTracer);
}

GUI_TEST_CLASS_DEFINITION(test_3125) {
    //1. Connect to a shared database.
    //2. Find a msa in the database (or import a new one).
    //3. Find menu items: {Actions -> Export -> Save subalignment}
    //and {Actions -> Export -> Save sequence}

    //Expected state: actions are enabled, you can export a subalignment and a sequence.
    //Current state: actions are disabled.
    GTLogTracer logTracer;
    Document* databaseDoc = GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);
    CHECK_OP(os, );
    GTUtilsSharedDatabaseDocument::openView(os, databaseDoc, "/view_test_0002/COI");

    QWidget* msaView = GTWidget::findWidget(os, " [m] COI");
    CHECK_SET_ERR(NULL != msaView, "View wasn't opened");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<MSAE_MENU_EXPORT<<"Save subalignment"));
    GTUtilsDialog::waitForDialog(os,new ExtractSelectedAsMSADialogFiller(os,
        testDir + "_common_data/scenarios/sandbox/3125.aln",
        QStringList() << "Phaneroptera_falcata", 6, 600));
    GTMenu::showContextMenu(os,GTWidget::findWidget(os,"msa_editor_sequence_area"));

    GTUtilsSharedDatabaseDocument::disconnectDatabase(os, databaseDoc);

    GTUtilsLog::check(os, logTracer);
}

GUI_TEST_CLASS_DEFINITION(test_3126) {
//    1. Open "test/_common_data/ace/ace_test_1.ace".
//    2. Click "OK" in the import dialog.
//    Expected: the file is imported, UGENE does not crash.
    GTLogTracer l;

    GTUtilsDialog::waitForDialog(os, new DocumentProviderSelectorDialogFiller
                                 (os, DocumentProviderSelectorDialogFiller::AssemblyBrowser));
    GTUtilsDialog::waitForDialog(os, new ConvertAceToSqliteDialogFiller(os,
                                                                        sandBoxDir + "test_3126"));
    GTFileDialog::openFile(os, testDir + "_common_data/ace/", "ace_test_1.ace");

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3128) {
    // 1. Open file test/_common_data/cmdline/read-write/read_db_write_gen.uws"
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    GTLogTracer l;
    GTFileDialog::openFile(os, testDir + "_common_data/cmdline/read-write/", "read_db_write_gen.uws");
    GTUtilsTaskTreeView::waitTaskFinished(os);


    // 2. Set parameters:     db="NCBI protein sequence database", id="AAA59172.1"
    GTUtilsWorkflowDesigner::click(os, "Read from remote database", QPoint(-20, -20));
    GTUtilsWorkflowDesigner::setParameter( os, "Database", 2, GTUtilsWorkflowDesigner::comboValue);
    GTUtilsWorkflowDesigner::setParameter( os, "Resource ID(s)", "AAA59172.1", GTUtilsWorkflowDesigner::textValue);

    // 3. Launch scheme.
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTWidget::click( os,GTAction::button( os,"Run workflow" ) );
    GTGlobals::sleep( );

    // Expected state: no errors in the log.
    GTUtilsLog::check( os, l );

}

GUI_TEST_CLASS_DEFINITION(test_3128_1) {

    // 1. Open any genbank file with a COMMENT section
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "murine.gb");

    // 2. Find the "comment" annotation, click it.
    QTreeWidgetItem *item2 = GTUtilsAnnotationsTreeView::findItem(os, "comment");
    QPoint p2 = GTTreeWidget::getItemCenter(os, item2);
    GTMouseDriver::moveTo(os, p2);
    GTMouseDriver::click(os);
    //Expected state: UGENE does not crash.
}

GUI_TEST_CLASS_DEFINITION(test_3130) {
//    1. Create an invalid connection to a shared database (e.g. wring login/password).
//    2. Connect.
//    Expected: the error message is shown.
//    3. Connect again.
//    Expected:  the error message is shown, UGENE does not crash.
    GTDatabaseConfig::initTestConnectionInfo("test_3133");

    EditConnectionDialogFiller::Parameters parameters;
    parameters.connectionName = "test_3133";
    parameters.host = GTDatabaseConfig::host();
    parameters.port = QString::number(GTDatabaseConfig::port());
    parameters.database = GTDatabaseConfig::database();
    parameters.login = GTDatabaseConfig::login() + "_test_3130";
    parameters.password = GTDatabaseConfig::password();
    GTUtilsDialog::waitForDialog(os, new EditConnectionDialogFiller(os, parameters, EditConnectionDialogFiller::MANUAL));

    QList<SharedConnectionsDialogFiller::Action> actions;
    actions << SharedConnectionsDialogFiller::Action(SharedConnectionsDialogFiller::Action::CLICK, "test_3133");
    actions << SharedConnectionsDialogFiller::Action(SharedConnectionsDialogFiller::Action::EDIT, "test_3133");

    SharedConnectionsDialogFiller::Action connectAction(SharedConnectionsDialogFiller::Action::CONNECT, "test_3133");
    connectAction.expectedResult = SharedConnectionsDialogFiller::Action::WRONG_DATA;
    actions << connectAction;
    actions << connectAction;

    GTUtilsDialog::waitForDialog(os, new SharedConnectionsDialogFiller(os, actions));

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__ACCESS_SHARED_DB);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_3133) {
    //1. Connect to a shared database (e.g. ugene_gui_test).
    Document *dbDoc = GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);
    QString dbName = dbDoc->getName();
    GTUtilsProjectTreeView::findIndex(os, dbName);
    CHECK_OP(os, );
    //2. Save the project.
    GTUtilsDialog::waitForDialog(os, new SaveProjectAsDialogFiller(os, "test_3133", testDir + "_common_data/scenarios/sandbox/", "test_3133"));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__SAVE_AS_PROJECT);
    //3. Close the project.
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    //4. Open the saved project.
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/sandbox/", "test_3133.uprj");
    //Expected state: project view is present, there are no documents presented.
    QModelIndex idx = GTUtilsProjectTreeView::findIndex(os, dbName, GTGlobals::FindOptions(false));
    CHECK_SET_ERR(!idx.isValid(), "The database document is in the project");
}

GUI_TEST_CLASS_DEFINITION(test_3137) {
    GTLogTracer l;

    // 1. Connect to shared database(eg.ugene_gui_test_win);
    Document *dbDoc = GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 2. Add folder;
    GTUtilsSharedDatabaseDocument::createFolder(os, dbDoc, "/", "regression_test_3137");

    // 3. Import some file to this folder(eg.COI.aln);
    GTUtilsSharedDatabaseDocument::importFiles(os, dbDoc, "/regression_test_3137", QStringList() << dataDir + "samples/CLUSTALW/COI.aln");

    // 4. Delete folder;
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "regression_test_3137"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__REMOVE_SELECTED));
    GTMouseDriver::click(os, Qt::RightButton);

    // 5. Wait for several seconds;
    GTGlobals::sleep(10000);

    // Expected state : folder does not appear.
    GTGlobals::FindOptions findOptions(false);
    findOptions.depth = 1;
    const QModelIndex innerFolderNotFoundIndex = GTUtilsProjectTreeView::findIndex(os, "regression_test_3137", findOptions);
    CHECK_SET_ERR(!innerFolderNotFoundIndex.isValid(), "The 'regression_test_3137' folder was found in the database but expected to disappear");

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3138) {
//    1. Open "_common_data/fasta/abcd.fa"
//    2. Open Find Pattern on the Option Panel
//    3. Set algorithm to "Regular expression"
//    4. Find 'A*' pattern
//    Expected state: founded regions are valid.

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Merge));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "abcd.fa");

    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);

    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search algorithm"));

    QComboBox* algorithmBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "boxAlgorithm"));
    CHECK_SET_ERR( algorithmBox != NULL, "Cannot find boxAlgorithm widget!");

    GTComboBox::setIndexWithText(os, algorithmBox, "Regular expression");

    GTWidget::click(os, GTWidget::findWidget(os, "textPattern"));
    GTGlobals::sleep(200);
    GTKeyboardDriver::keySequence(os, "A*");

    GTGlobals::sleep(500);
    GTWidget::click(os, GTWidget::findWidget(os, "getAnnotationsPushButton"));
    GTGlobals::sleep(500);

    QList<U2Region> regions = GTUtilsAnnotationsTreeView::getAnnotatedRegions(os);
    foreach(const U2Region& r, regions) {
        CHECK_SET_ERR(r.length > 0, "Invalid annotated region!");
    }
}

GUI_TEST_CLASS_DEFINITION(test_3139) {
    //1. Do {File -> Open as...} in the main menu.
    //2. Open "data/samples/FASTA/human_T1.fa" as msa.
    //    Expected state: a MSA Editor is opened.
    //    Current state: a Sequence View is opened.
    GTFileDialogUtils *ob = new GTFileDialogUtils(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTUtilsDialog::waitForDialog(os, ob);

    GTUtilsDialog::waitForDialog(os, new DocumentFormatSelectorDialogFiller(os, "FASTA"));

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Join));

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__OPEN_AS);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QWidget* seqArea = GTWidget::findWidget(os, "msa_editor_sequence_area");
    CHECK_SET_ERR(NULL != seqArea, "MSA Editor isn't opened.!");
}

GUI_TEST_CLASS_DEFINITION(test_3140) {
//    1. Open "_common_data/clustal/big.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "big.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    2. Select the first symbol of the first line.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(0, 0));

//    3. Press the Space button and do not unpress it.
//    Expected: the alignment changes on every button press. UGENE does not crash.
//    4. Unpress the button.
//    Expected: the overview rendereing task is finished. The overview is shown.
    for (int i = 0; i < 100; i++) {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
    }

    GTGlobals::sleep(1000);
    int renderTasksCount = GTUtilsTaskTreeView::getTopLevelTasksCount(os);
    CHECK_SET_ERR(1 == renderTasksCount, QString("An unexpected overview render tasks count: expect %1, got %2").arg(1).arg(renderTasksCount));

    GTUtilsTaskTreeView::waitTaskFinished(os);
    const QColor currentColor = GTUtilsMsaEditor::getGraphOverviewPixelColor(os, QPoint(1, 1));
    const QColor expectedColor = QColor("white");
    const QString currentColorString = QString("(%1, %2, %3)").arg(currentColor.red()).arg(currentColor.green()).arg(currentColor.blue());
    const QString expectedColorString = QString("(%1, %2, %3)").arg(expectedColor.red()).arg(expectedColor.green()).arg(expectedColor.blue());
    CHECK_SET_ERR(expectedColor == currentColor, QString("An unexpected color, maybe overview was not rendered: expected %1, got %2").arg(expectedColorString).arg(currentColorString));
}

GUI_TEST_CLASS_DEFINITION(test_3142) {
//    1. Open "data/samples/CLUSTALW/COI.aln"
//    2. On the options panel press the "Open tree" button
//    Expected state: the "Select files to open..." dialog has opened
//    3. Choose the file "data/samples/Newick/COI.nwk"
//    Expected state: a tree view has appeared along with MSA view
    GTLogTracer l;

    GTFileDialog::openFile(os, dataDir + "/samples/CLUSTALW/", "COI.aln");
    GTGlobals::sleep();

    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_ADD_TREE_WIDGET"));
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/Newick/COI.nwk"));
    GTWidget::click(os, GTWidget::findWidget(os, "OpenTreeButton"));

    QWidget* msaWidget = GTWidget::findWidget(os, "msa_editor_sequence_area");
    CHECK_SET_ERR( msaWidget != NULL, "MSASequenceArea not found");

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3143){
//    1. Open file data/samples/Assembly/chrM.sorted.bam;
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, sandBoxDir + "chrM.sorted.bam.ugenedb"));
    GTFileDialog::openFile(os, dataDir + "samples/Assembly", "chrM.sorted.bam");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();
//    Expected state: Showed Import BAM File dialog.
//    2. Click Import;
//    Expected state: Imported file opened in Assembly Viewer.
    GTWidget::findWidget(os, "assembly_browser_chrM.sorted.bam [as] chrM");
//    3. Remove this file from project and try to open it again;
    GTUtilsProjectTreeView::click(os, "chrM.sorted.bam.ugenedb");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();
//    Expected state: Showed Import BAM File dialog.
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "Replace"));
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, sandBoxDir + "chrM.sorted.bam.ugenedb"));
    GTFileDialog::openFile(os, dataDir + "samples/Assembly", "chrM.sorted.bam");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();
//    4. Click Import;
//    Expected state: Showed message box with question about overwriting of existing file..
//    5. Click Replace;
    GTWidget::findWidget(os, "assembly_browser_chrM.sorted.bam [as] chrM");
//    Expected state: Imported file opened in Assembly Viewer without errors.

}

GUI_TEST_CLASS_DEFINITION(test_3144) {
    GTLogTracer l;

    // 1.Connect to a shared database.
    Document *dbDoc = GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    // 2. Create a folder "regression_test_3144_1" in the root folder.
    GTUtilsSharedDatabaseDocument::createFolder(os, dbDoc, "/", "regression_test_3144_1");

    // 3. Create a folder "regression_test_3144_2" in the folder "regression_test_3144_1".
    GTUtilsSharedDatabaseDocument::createFolder(os, dbDoc, "/regression_test_3144_1", "regression_test_3144_2");

    // 4. Remove the folder "regression_test_3144_2".
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "regression_test_3144_2"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__REMOVE_SELECTED));
    GTMouseDriver::click(os, Qt::RightButton);

    // Expected state : the folder "regression_test_3144_2" is moved to the "Recycle bin".
    QModelIndex rbIndex = GTUtilsProjectTreeView::findIndex(os, "Recycle bin");
    const QModelIndex innerFolderIndex = GTUtilsProjectTreeView::findIndex(os, "regression_test_3144_2", rbIndex);

    // 5. Remove the folder "regression_test_3144_1".
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "regression_test_3144_1"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__REMOVE_SELECTED));
    GTMouseDriver::click(os, Qt::RightButton);

    // Expected state : folders "regression_test_3144_1" is shown in the "Recycle bin", folder "regression_test_3144_2" disappears.
    rbIndex = GTUtilsProjectTreeView::findIndex(os, "Recycle bin");
    const QModelIndex outerFolderIndex = GTUtilsProjectTreeView::findIndex(os, "regression_test_3144_1", rbIndex);
    const QModelIndex innerFolderNotFoundIndex = GTUtilsProjectTreeView::findIndex(os, "regression_test_3144_2", rbIndex);

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3155) {
    // 1. Open "humam_T1"
    // Expected state: "Circular search" checkbox does not exist
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(200);
    // 2. Press "Find ORFs" tool button
    class CancelClicker : public Filler {
    public:
        CancelClicker(U2OpStatus& _os) : Filler(_os, "ORFDialogBase"){}
        virtual void run() {
            QWidget *w = QApplication::activeWindow();
            CHECK(NULL != w, );
            QDialogButtonBox *buttonBox = w->findChild<QDialogButtonBox*>(QString::fromUtf8("buttonBox"));
            CHECK(NULL != buttonBox, );
            QPushButton *button = buttonBox->button(QDialogButtonBox::Cancel);
            CHECK(NULL != button, );
            QCheckBox *check = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "ckCircularSearch", NULL, GTGlobals::FindOptions(false)));
            CHECK(NULL == check, );
            GTWidget::click(os, button);
        }
    };
    GTUtilsDialog::waitForDialog(os, new CancelClicker(os));
    GTWidget::click(os, GTAction::button(os, "Find ORFs"));
}

GUI_TEST_CLASS_DEFINITION(test_3156){
//    1. Connect to a shared database
    QString conName = "ugene_gui_test";
    Document* databaseDoc = GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);
    GTUtilsSharedDatabaseDocument::createFolder(os, databaseDoc, "/", "test_3156");
//    2. Open file "data/samples/Genbank/murine.gb"
    GTFile::copy(os, dataDir + "samples/Genbank/murine.gb", sandBoxDir + "test_3156_murine.gb");
    GTFileDialog::openFile(os, sandBoxDir, "test_3156_murine.gb");
//    3. Drag the document item onto the DB item in project view
    QModelIndex from = GTUtilsProjectTreeView::findIndex(os, "test_3156_murine.gb");
    QModelIndex to = GTUtilsProjectTreeView::findIndex(os, "test_3156");
    GTUtilsProjectTreeView::dragAndDrop(os, from, to);
    GTGlobals::sleep(10000);
//    Expected state: a new folder has appeared in the DB, objects from the document have been imported into it.
    GTUtilsProjectTreeView::checkItem(os, "murine.gb", to);
}

class test_3165_messageBoxDialogFiller: public MessageBoxDialogFiller{
public:
    test_3165_messageBoxDialogFiller(U2OpStatus &os, QMessageBox::StandardButton _b):
        MessageBoxDialogFiller(os, _b){}
    virtual void run(){
        QWidget* activeModal = QApplication::activeModalWidget();
        QMessageBox *messageBox = qobject_cast<QMessageBox*>(activeModal);
        CHECK_SET_ERR(messageBox != NULL, "messageBox is NULL");

        QAbstractButton* button = messageBox->button(b);
        CHECK_SET_ERR(button != NULL, "There is no such button in messagebox");

        GTWidget::click(os, button);
        GTGlobals::sleep(500);
        GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Save));
    }
};

GUI_TEST_CLASS_DEFINITION(test_3165){
//    1. Set file read-only: "test/_common_data/scenarios/msa/ma.aln".
    GTFile::copy(os, testDir + "_common_data/scenarios/msa/ma.aln", sandBoxDir + "ma.aln");
    PermissionsSetter permSetter;
    QFile::Permissions p = QFile::ReadUser | QFile::ReadOwner;
    bool res = permSetter.setPermissions( sandBoxDir + "ma.aln", p );
    CHECK_SET_ERR(res, "permission not set");
    //PermissionsSetter::setPermissions(sandBoxDir + "ma.aln"
//    2. Open it with UGENE.
    GTFileDialog::openFile(os, sandBoxDir, "ma.aln");
//    3. Change the alignment (e.g. insert a gap).
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(1,1));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
//    4. Close the project.
    GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
    GTUtilsDialog::waitForDialog(os, new test_3165_messageBoxDialogFiller(os, QMessageBox::Yes));
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, sandBoxDir, "test_3165_out.aln", GTFileDialogUtils::Save));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTGlobals::sleep();
    //GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Save));
//    Expected state: you are offered to save the project.
//    5. Do not save the project.
//    Expected state: you are offered to save the file.
//    6. Accept the offering.
//    Expected state: UGENE notices that it can't rewrite the file, it offers you to save the file to another location.
//    7. Save file anywhere.
//    Expected state: the project closes, the file is successfully saved, UGENE doesn't crash.
    //TODO: add this check after UGENE-3200 fix
    //GTUtilsProject::checkProject(os, GTUtilsProject::NotExists);
    CHECK_SET_ERR(GTFile::check(os, sandBoxDir + "test_3165_out.aln"), "file not saved");
//    Current state: file is successfully saved, then UGENE crashes.
}

GUI_TEST_CLASS_DEFINITION(test_3170) {
    // 1. Open human_T1.fa.
    // 2. Select the region [301..350].
    // 3. Context menu -> Analyze -> Query with BLAST+.
    // 5. Select the database.
    // 6. Run.
    // Expected: the found annotations don't start from the position 1.
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 51, 102));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Select" << "Sequence region"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));

    BlastAllSupportDialogFiller::Parameters blastParams;
    blastParams.runBlast = true;
    blastParams.dbPath = testDir + "_common_data/cmdline/external-tool-support/blastplus/human_T1/human_T1.nhr";
    GTUtilsDialog::waitForDialog(os, new BlastAllSupportDialogFiller(blastParams, os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "query_with_blast+", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    bool found1 = GTUtilsAnnotationsTreeView::findRegion(os, "blast result", U2Region(51, 51));
    CHECK_OP(os, );
    CHECK_SET_ERR(found1, "Can not find the blast result");
    bool found2 = GTUtilsAnnotationsTreeView::findRegion(os, "blast result", U2Region(1, 52));
    CHECK_OP(os, );
    CHECK_SET_ERR(!found2, "Wrong blast result");
}

GUI_TEST_CLASS_DEFINITION(test_3175) {
    // 1. Open "_common_data/scenarios/msa/ma.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa/", "ma.aln");
    GTGlobals::sleep(200);
    // Expected: the first sequence is "TAAGACTTCTAA".
    GTUtilsMSAEditorSequenceArea::selectArea( os, QPoint( 0, 0 ), QPoint( 12, 0 ) );
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text( os );
    CHECK_SET_ERR( "TAAGACTTCTAA" == selectionContent, "MSA changing is failed" );
}

GUI_TEST_CLASS_DEFINITION(test_3180) {
    //1. Open "samples/FASTA/human_T1.fa".
    //2. Click the "Find restriction sites" button on the main toolbar.
    //3. Accept the dialog.
    //Expected: the task becomes cancelled.
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Restriction Sites"));
    GTWidget::click(os, GTWidget::findWidget(os, "AutoAnnotationUpdateAction"));
    GTGlobals::systemSleep();
    foreach (Task *task, AppContext::getTaskScheduler()->getTopLevelTasks()) {
        if (task->getTaskName() != "Auto-annotations update task") {
            continue;
        }
        GTGlobals::systemSleep();
        task->cancel();
    }
    GTGlobals::sleep();
    CHECK_SET_ERR(AppContext::getTaskScheduler()->getTopLevelTasks().isEmpty(), "Task is not cancelled");
}

GUI_TEST_CLASS_DEFINITION(test_3187) {
    FormatDBSupportRunDialogFiller::Parameters p;
    p.inputFilePath = dataDir + "samples/FASTA/human_T1.fa";
    p.alphabetType = FormatDBSupportRunDialogFiller::Parameters::Nucleotide;
    p.outputDirPath = sandBoxDir + "test_3187";
    QDir().mkpath(p.outputDirPath);
    GTUtilsDialog::waitForDialog(os, new FormatDBSupportRunDialogFiller(os, p));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "BLAST" << "FormatDB"));
    GTMenu::showMainMenu(os, MWMENU_TOOLS);

    //GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();
    QMainWindow *mw = AppContext::getMainWindow()->getQMainWindow();
    QPoint botRight = mw->rect().bottomRight();
    botRight.setX(botRight.x() - 7);
    botRight.setY(botRight.y() + 10);

    GTMouseDriver::moveTo(os, botRight);
    GTMouseDriver::doubleClick(os);

    botRight.setX(botRight.x() - 35);
    botRight.setY(botRight.y() - 55);

    GTMouseDriver::moveTo(os, botRight);
    GTMouseDriver::click(os);

    QWidget *reportWidget = GTWidget::findWidget(os, "qt_scrollarea_viewport");
    GTMouseDriver::moveTo(os, reportWidget->rect().center());
    GTMouseDriver::click(os);

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    QString reportText = GTClipboard::text(os);

    CHECK_SET_ERR(reportText.contains("Blast database has been successfully created"), "report didn't contain expected text");
    CHECK_SET_ERR(reportText.contains("Type: nucleotide"), "report didn't contain expected text");
    CHECK_SET_ERR(reportText.contains("Source sequences:"), "report didn't contain expected text");
    CHECK_SET_ERR(reportText.contains("Formatdb log file path:"), "report didn't contain expected text");
}

GUI_TEST_CLASS_DEFINITION(test_3207){
//    Steps to reproduce:
//    1. Open "\samples\PDB\1CF7.PDB"
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "AMINO.fa");
//    2. Select region [30..31] in "1CF7 chain 1" sequence
    GTUtilsSequenceView::selectSequenceRegion(os, 30,31);
//    3. Open "DAS annotaions" OP
    GTWidget::click(os, GTWidget::findWidget(os, "OP_DAS"));
//    Expected state:
//    The options panel appeared, selected region in OP [30..31], there is warning, that region is too short
    QLineEdit* start_edit_line = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "start_edit_line"));
    CHECK_SET_ERR(start_edit_line != NULL, "start_edit_line not found");
    QString start = start_edit_line->text();
    CHECK_SET_ERR(start == "30", QString("unexpected start value: %1").arg(start));

    QLineEdit* end_edit_line = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "end_edit_line"));
    CHECK_SET_ERR(end_edit_line != NULL, "end_edit_line not found");
    QString end = end_edit_line->text();
    CHECK_SET_ERR(end == "31", QString("unexpected end value: %1").arg(end));

    QLabel* hintLabel = qobject_cast<QLabel*>(GTWidget::findWidget(os, "hintLabel"));
    CHECK_SET_ERR(hintLabel != NULL, "hintLabel not found");
    QString hint = hintLabel->text();
    CHECK_SET_ERR(hint == "Warning: Selected region is too short. It should be from 4 to 1900 amino acids.",
                  QString("unexpected hint: %1").arg(hint));
//    4. Select region [20..40] in the sequence
    GTUtilsSequenceView::selectSequenceRegion(os, 20, 40);
//    Expected state:
//    Selected region in OP is [20..40], warning is not shown
    start = start_edit_line->text();
    CHECK_SET_ERR(start == "20", QString("unexpected start value: %1").arg(start));

    end = end_edit_line->text();
    CHECK_SET_ERR(end == "40", QString("unexpected end value: %1").arg(end));

    CHECK_SET_ERR(hintLabel->isHidden(), "hintLabel unexpectidly presents");
//    Current state:
//    Selected region in OP is [30..31], warning is shown
}

GUI_TEST_CLASS_DEFINITION(test_3209_1) {
    // BLAST+ from file
    BlastAllSupportDialogFiller::Parameters blastParams;
    blastParams.runBlast = true;
    blastParams.programNameText = "blastn";
    blastParams.dbPath = testDir + "_common_data/cmdline/external-tool-support/blastplus/human_T1/human_T1.nhr";
    blastParams.withInputFile = true;
    blastParams.inputPath = dataDir + "samples/FASTA/human_T1.fa";
    GTUtilsDialog::waitForDialog(os, new BlastAllSupportDialogFiller(blastParams, os));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "BLAST" << "BLAST+ Search");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    bool found = GTUtilsAnnotationsTreeView::findRegion(os, "blast result", U2Region(5061, 291));
    CHECK_OP(os, );
    CHECK_SET_ERR(found, "Can not find the blast result");
}

GUI_TEST_CLASS_DEFINITION(test_3209_2) {
    // BLAST from file
    BlastAllSupportDialogFiller::Parameters blastParams;
    blastParams.runBlast = true;
    blastParams.programNameText = "blastn";
    blastParams.dbPath = testDir + "_common_data/cmdline/external-tool-support/blastplus/human_T1/human_T1.nhr";
    blastParams.withInputFile = true;
    blastParams.inputPath = dataDir + "samples/FASTA/human_T1.fa";
    GTUtilsDialog::waitForDialog(os, new BlastAllSupportDialogFiller(blastParams, os));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "BLAST" << "BLAST Search");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    bool found = GTUtilsAnnotationsTreeView::findRegion(os, "blast result", U2Region(5061, 291));
    CHECK_OP(os, );
    CHECK_SET_ERR(found, "Can not find the blast result");
}

GUI_TEST_CLASS_DEFINITION(test_3211) {
    //1. Select the "Tools" -> "BLAST" -> "BLAST Search..." item in the main menu.
    BlastAllSupportDialogFiller::Parameters parameters;
    parameters.test_3211 = true;
    parameters.inputPath = dataDir + "samples/FASTA/human_T1.fa";
    GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
    GTUtilsDialog::waitForDialog(os, new BlastAllSupportDialogFiller(parameters, os));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "BLAST" << "BLAST Search");
    //Expected state: there is a "Request to Local BLAST Database" dialog without an annotation widget.
    //2. Set any input sequence.
    //Expected state: an annotation widget was added.
    //3. Set any another input sequence.
    //Expected state: there is a single annotation widget.
}

GUI_TEST_CLASS_DEFINITION(test_3214) {
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Extract Consensus from Alignment as Sequence");

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Extract Consensus from Alignment as Sequence"));
    GTMouseDriver::click(os);

    GTUtilsWorkflowDesigner::setParameter(os, "Threshold", 49, GTUtilsWorkflowDesigner::spinValue);
    GTUtilsWorkflowDesigner::setParameter(os, "Algorithm", 0, GTUtilsWorkflowDesigner::comboValue);
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::getParameter(os, "Threshold") == "50", "Wrong parameter");

    GTUtilsWorkflowDesigner::addAlgorithm(os, "Extract Consensus from Alignment as Text");

    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "Extract Consensus from Alignment as Text"));
    GTMouseDriver::click(os);

    GTUtilsWorkflowDesigner::setParameter(os, "Algorithm", 0, GTUtilsWorkflowDesigner::comboValue);
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::getAllParameters(os).size() == 1, "Too many parameters");
}

GUI_TEST_CLASS_DEFINITION(test_3216_1) {
//    1. Open "test/_common_data/genbank/1anot_1seq.gen" file.
    QDir().mkpath(sandBoxDir + "test_3216");
    GTFile::copy(os, testDir + "_common_data/genbank/1anot_1seq.gen", sandBoxDir + "test_3216/test_3216_1.gen");
    GTFileDialog::openFile(os, sandBoxDir + "test_3216", "test_3216_1.gen");

//    2. Add a qualifier with the value "012345678901234567890123456789012345678901234567890123456789".
    const QString expectedValue = "012345678901234567890123456789012345678901234567890123456789";
    GTUtilsAnnotationsTreeView::createQualifier(os, "test_3216_1", expectedValue, "CDS");

//    3. Save the file, reopen the file.
//    Expected state: the qualifier value is the same.
    GTUtilsDocument::saveDocument(os, "test_3216_1.gen");
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTUtilsDocument::removeDocument(os, "test_3216_1.gen");

    GTFileDialog::openFile(os, sandBoxDir + "test_3216", "test_3216_1.gen");
    const QString actualValue = GTUtilsAnnotationsTreeView::getQualifierValue(os, "test_3216_1", "CDS");
    CHECK_SET_ERR(expectedValue == actualValue, QString("The qualifier value is incorrect: expect '%1', got '%2'").arg(expectedValue).arg(actualValue));
}

GUI_TEST_CLASS_DEFINITION(test_3216_2) {
//    1. Open "test/_common_data/genbank/1anot_1seq.gen" file.
    QDir().mkpath(sandBoxDir + "test_3216");
    GTFile::copy(os, testDir + "_common_data/genbank/1anot_1seq.gen", sandBoxDir + "test_3216/test_3216_2.gen");
    GTFileDialog::openFile(os, sandBoxDir + "test_3216", "test_3216_2.gen");

//    2. Add a qualifier with the value "012345678901234567890123456789 012345678901234567890123456789".
    const QString expectedValue = "012345678901234567890123456789 012345678901234567890123456789";
    GTUtilsAnnotationsTreeView::createQualifier(os, "test_3216_2", expectedValue, "CDS");

//    3. Save the file, reopen the file.
//    Expected state: the qualifier value is the same.
    GTUtilsDocument::saveDocument(os, "test_3216_2.gen");
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTUtilsDocument::removeDocument(os, "test_3216_2.gen");

    GTFileDialog::openFile(os, sandBoxDir + "test_3216", "test_3216_2.gen");
    const QString actualValue = GTUtilsAnnotationsTreeView::getQualifierValue(os, "test_3216_2", "CDS");
    CHECK_SET_ERR(expectedValue == actualValue, QString("The qualifier value is incorrect: expect '%1', got '%2'").arg(expectedValue).arg(actualValue));
}

GUI_TEST_CLASS_DEFINITION(test_3216_3) {
//    1. Open "test/_common_data/genbank/1anot_1seq.gen" file.
    QDir().mkpath(sandBoxDir + "test_3216");
    GTFile::copy(os, testDir + "_common_data/genbank/1anot_1seq.gen", sandBoxDir + "test_3216/test_3216_3.gen");
    GTFileDialog::openFile(os, sandBoxDir + "test_3216", "test_3216_3.gen");

//    2. Add a qualifier with the value "012345678901234567890123456789 0  1 2345678901234567890123456789".
    const QString expectedValue = "012345678901234567890123456789 0  1 2345678901234567890123456789";
    GTUtilsAnnotationsTreeView::createQualifier(os, "test_3216_3", expectedValue, "CDS");

//    3. Save the file, reopen the file.
//    Expected state: the qualifier value is the same.
    GTUtilsDocument::saveDocument(os, "test_3216_3.gen");
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTUtilsDocument::removeDocument(os, "test_3216_3.gen");

    GTFileDialog::openFile(os, sandBoxDir + "test_3216", "test_3216_3.gen");
    const QString actualValue = GTUtilsAnnotationsTreeView::getQualifierValue(os, "test_3216_3", "CDS");
    CHECK_SET_ERR(expectedValue == actualValue, QString("The qualifier value is incorrect: expect '%1', got '%2'").arg(expectedValue).arg(actualValue));
}

GUI_TEST_CLASS_DEFINITION(test_3218) {
    // 1. Open "test/_common_data/genbank/big_feature_region.gb".
    GTLogTracer l;
    GTFileDialog::openFile(os, testDir + "_common_data/genbank/", "big_feature_region.gb");

    // Expected state : file is opened, there are no errors in the log
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3220){
    //1. Open human_T1.fa
    //
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(1000);
    //2. Add an annotation
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "DDD", "D", "10..16"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();
    //2. Add qualifier with quotes
    Runnable *filler = new EditQualifierFiller(os, "newqualifier", "val\"", GTGlobals::UseMouse, true);
    GTUtilsDialog::waitForDialog(os, filler);
    CHECK_OP(os, );
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    CHECK_OP(os, );

    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "D"));
    GTMouseDriver::click(os, Qt::RightButton);

    //3. Add another qualifier to the same annotation
    filler = new EditQualifierFiller(os, "newqualifier2", "val\"2", GTGlobals::UseMouse, true);
    GTUtilsDialog::waitForDialog(os, filler);
    CHECK_OP(os, );
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "add_qualifier_action"));
    CHECK_OP(os, );

    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "D"));
    GTMouseDriver::click(os, Qt::RightButton);

    //4. Save the file and reload it
    GTUtilsDocument::unloadDocument(os, "human_T1.fa", true);
    GTUtilsDocument::loadDocument(os, "human_T1.fa");

    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "D"));
    GTMouseDriver::click(os, Qt::LeftButton);


    QTreeWidgetItem *generalItem = GTUtilsAnnotationsTreeView::findItem(os, "D");
    CHECK_SET_ERR(generalItem != NULL, "Invalid annotation tree item");

    AVAnnotationItem *annotation = dynamic_cast<AVAnnotationItem *>(generalItem);
    CHECK_SET_ERR(annotation != NULL, "Annotation is not found");
    CHECK_SET_ERR("val\"" == annotation->annotation.findFirstQualifierValue("newqualifier"), "Qualifier is not found");
    CHECK_SET_ERR("val\"2" == annotation->annotation.findFirstQualifierValue("newqualifier2"), "Qualifier 2 is not found");
}

GUI_TEST_CLASS_DEFINITION(test_3226) {
    //1. Create a workflow with a 'File List' element.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::addAlgorithm(os, "File list");

    ////2. Setup alias 'in' for input path.
    QMap<QPoint*, QString> map;
    QPoint p(1, 0);
    map[&p] = "in";
    GTUtilsDialog::waitForDialog(os, new AliasesDialogFiller(os, map));
    GTWidget::click(os, GTAction::button(os, "Configure parameter aliases"));

    ////3. Copy and paste the 'File list' element.
    GTUtilsWorkflowDesigner::click(os, "File List");
    //GTKeyboardUtils::copy(os);
    GTWidget::click(os, GTAction::button(os, "Copy action"));
    GTKeyboardUtils::paste(os);

    ////4. Save the workflow.
    QString path = sandBoxDir + "test_3226_workflow.uwl";
    GTUtilsDialog::waitForDialog(os, new WorkflowMetaDialogFiller(os, path, ""));
    GTWidget::click(os, GTAction::button(os, "Save workflow action"));

    //5. Close current workflow.
    GTWidget::click(os, GTAction::button(os, "New workflow action"));

    //7. Open the saved workflow.
    GTLogTracer l;
    GTUtilsWorkflowDesigner::loadWorkflow(os, path);

    //Expected state: the saved workflow is opened, there are no errors in the log, the alias it set only for the one element.
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3245) {
    // 1. Open "data/samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "/samples/CLUSTALW/", "COI.aln");

    // 2. Ensure that there is a single menu item (Create new color scheme) in the {Colors -> Custom schemes}
    // submenu of the context menu. Click it.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_HIGHLIGHTING"));

    QComboBox *combo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "colorScheme"));
    const int initialItemsNumber = combo->count();

    // 3. Create a new color scheme, accept the preferences dialog.
    const QString colorSchemeName = "test scheme";
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Colors" << "Custom schemes" << "Create new color scheme"));
    GTUtilsDialog::waitForDialog(os, new NewColorSchemeCreator(os, colorSchemeName, NewColorSchemeCreator::nucl));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(1, 1));
    GTMouseDriver::click(os, Qt::RightButton);

    // 4. Ensure that the new scheme is added to the context menu. Call the preferences dialog again.
    // 5. Remove the custom scheme and cancel the preferences dialog.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Colors" << "Custom schemes" << colorSchemeName));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(1, 1));
    GTMouseDriver::click(os, Qt::RightButton);

    combo = qobject_cast<QComboBox *>(GTWidget::findWidget(os, "colorScheme"));
    CHECK_SET_ERR(combo->count() - 1 == initialItemsNumber, "color scheme hasn't been added to the Options Panel");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Colors" << "Custom schemes" << "Create new color scheme"));
    GTUtilsDialog::waitForDialog(os, new NewColorSchemeCreator(os, colorSchemeName, NewColorSchemeCreator::nucl,
        NewColorSchemeCreator::Delete, true));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(1, 1));
    GTMouseDriver::click(os, Qt::RightButton);

    // Expected state: the scheme presents in the context menu, it is shown in the preferences dialog.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Colors" << "Custom schemes" << colorSchemeName));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(1, 1));
    GTMouseDriver::click(os, Qt::RightButton);

    combo = qobject_cast<QComboBox *>(GTWidget::findWidget(os, "colorScheme"));
    CHECK_SET_ERR(combo->count() - 1 == initialItemsNumber, "color scheme hasn't been added to the Options Panel");
}

GUI_TEST_CLASS_DEFINITION(test_3250) {
    //1. Connect to a shared database.
    //2. Right click on the document in the project view.
    //Expected: there are no the "Export/Import" menu for the database connection.
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);
    CHECK_OP(os, );
    QPoint p = GTUtilsProjectTreeView::getItemCenter(os, "ugene_gui_test");
    CHECK_OP(os, );
    GTMouseDriver::moveTo(os, p);
    U2OpStatus2Log opStatus;
    GTUtilsDialog::waitForDialog(os, new PopupChooser(opStatus, QStringList() << "Export/Import"));
    GTMouseDriver::click(os, Qt::RightButton);
    CHECK_SET_ERR(opStatus.hasError(), "Export item exists");
}

GUI_TEST_CLASS_DEFINITION(test_3255) {
//    1. Open "data/samples/Assembly/chrM.sam.bam".
//    Expected state: an import dialog appears.
//    2. Set any valid output path (or use default), check the "Import unmapped reads" option and click the "Import" button.
//    Expected state: the task finished without errors.
    GTLogTracer l;

    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, sandBoxDir + "test_3255/test_3255.ugenedb",
                                                             "", "", true));
    GTFileDialog::openFile(os, testDir + "_common_data/bam/", "1.bam");

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3263){
//    1. Open "_common_data/alphabets/standard_dna_rna_amino_1000.fa"
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTFileDialog::openFile(os, testDir + "_common_data/alphabets", "standard_dna_rna_amino_1000.fa");
//    2. Open CV for the first sequence
    QWidget* cvButton1 = GTWidget::findWidget(os, "CircularViewAction", GTWidget::findWidget(os, "ADV_single_sequence_widget_0"));
    QWidget* cvButton2 = GTWidget::findWidget(os, "CircularViewAction", GTWidget::findWidget(os, "ADV_single_sequence_widget_1"));
    GTWidget::click(os, cvButton2);
//    3. Open CV for the second sequence
    GTWidget::click(os, cvButton1);
//    4. Click CV button for the first sequence (turn it off and on again) few times
    QWidget* CV_ADV_single_sequence_widget_1 = GTWidget::findWidget(os, "CV_ADV_single_sequence_widget_1");
    QRect geometry = CV_ADV_single_sequence_widget_1->geometry();
    for(int i = 0; i<5; i++){
        GTWidget::click(os, cvButton1);
        GTGlobals::sleep(500);
        GTWidget::click(os, cvButton1);
        GTGlobals::sleep(500);
        CHECK_SET_ERR(geometry == CV_ADV_single_sequence_widget_1->geometry(), "geometry changed");
    }
//    See the result on the attached screenshot.

}

GUI_TEST_CLASS_DEFINITION(test_3266){
    //1. Connect to a shared database.
    Document *doc = GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    //2. Create a folder "1" somewhere.
    GTUtilsSharedDatabaseDocument::createFolder(os, doc, "/", "regression_3266_1");

    //3. Create a folder "2" in the folder "1".
    GTUtilsSharedDatabaseDocument::createFolder(os, doc, "/regression_3266_1", "regression_3266_2");

    //4. Remove the folder "2".
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "regression_3266_2"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__REMOVE_SELECTED));
    GTMouseDriver::click(os, Qt::RightButton);

    //Expected state: the folder "2" is in the Recycle bin.
    GTUtilsSharedDatabaseDocument::ensureItemExists(os, doc, "/Recycle bin/regression_3266_2");

    //5. Create another folder "2" in the folder "1".
    GTUtilsSharedDatabaseDocument::createFolder(os, doc, "/regression_3266_1", "regression_3266_2");

    //6. Remove the folder "1".
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "regression_3266_1"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__REMOVE_SELECTED));
    GTMouseDriver::click(os, Qt::RightButton);

    //Expected state: folders "1" and "2" both are in the Recycle bin.
    GTUtilsSharedDatabaseDocument::ensureItemExists(os, doc, "/Recycle bin/regression_3266_1");
    GTUtilsSharedDatabaseDocument::ensureItemExists(os, doc, "/Recycle bin/regression_3266_2");
}

GUI_TEST_CLASS_DEFINITION(test_3274) {
    QStringList expectedNames;
    QList<ADVSingleSequenceWidget*> seqWidgets;
    expectedNames << "seq1" << "seq3" << "seq5";

    seqWidgets = GTUtilsProject::openFileExpectSequences(os,
        testDir + "_common_data/alphabets",
        "standard_dna_rna_amino_1000.fa",
        expectedNames);
    CHECK_OP_SET_ERR(os, "Failed to open sequences!");

    ADVSingleSequenceWidget *seq1Widget = seqWidgets.at(0);
    ADVSingleSequenceWidget *seq3Widget = seqWidgets.at(1);

    GTUtilsCv::cvBtn::click(os, seq3Widget);
    GTUtilsCv::cvBtn::click(os, seq1Widget);

    QWidget* circularView = GTWidget::findWidget(os, "CV_ADV_single_sequence_widget_0");
    CHECK_OP_SET_ERR(os, "Failed to open circular view!");

    GTUtilsDialog::waitForDialog(os, new CircularViewExportImage(os, testDir + "_common_data/scenarios/sandbox/image.jpg", "", "seq3"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< ADV_MENU_EXPORT << "Save circular view as image", GTGlobals::UseMouse));

    GTWidget::click(os, circularView, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_3276) {
//    1. Open "data/samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");

//    2. Build a phylogenetic tree synchronized with the alignment.
    QDir().mkdir(QFileInfo(sandBoxDir + "test_3276/COI.nwk").dir().absolutePath());
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, sandBoxDir + "test_3276/COI.wnk", 0, 0, true));
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "Build Tree"));
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    3. Rename the first and the second sequences to "1".
    GTUtilsMSAEditorSequenceArea::renameSequence(os, "Isophya_altaica_EF540820", "1");
    GTUtilsMSAEditorSequenceArea::renameSequence(os, "Bicolorana_bicolor_EF540830", "1");

//    4. Remove the first sequence.
    GTUtilsMSAEditorSequenceArea::removeSequence(os, "1");

//    5. Ensure that the "Sort alignment by tree" button on the tree view toolbar is disabled.
    QAction *sortAction = GTAction::findAction(os, "Sort Alignment");
    CHECK_SET_ERR(NULL != sortAction, "'Sort alignment by tree' was not found");
    CHECK_SET_ERR(!sortAction->isEnabled(), "'Sort alignment by tree' is unexpectedly enabled");
}

GUI_TEST_CLASS_DEFINITION(test_3277){
//    Open "data/samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

    QWidget* seqArea = GTWidget::findWidget(os, "msa_editor_sequence_area");
    QColor before = GTWidget::getColor(os, seqArea, QPoint(1,1));
    QString bName = before.name();
//    Open the "Highlighting" options panel tab.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_HIGHLIGHTING"));
//    Set any reference sequence.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-5, 5));
    GTWidget::click(os, GTWidget::findWidget(os, "addSeq"));
//    Select different highlighting schemes.
    QComboBox* highlightingScheme = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "highlightingScheme"));
    GTComboBox::setIndexWithText(os, highlightingScheme, "Gaps");
//    Current state: the highlighting doesn't work for all sequences except the reference sequence.

    QColor after = GTWidget::getColor(os, seqArea, QPoint(1,1));
    QString aName = after.name();

    CHECK_SET_ERR(before != after, "colors not changed");

}

GUI_TEST_CLASS_DEFINITION(test_3279){
//    Open "data/samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
//    Open "Statistics" options panel tab.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_SEQ_STATISTICS_WIDGET"));
//    Set any reference sequence.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-5, 5));
    GTWidget::click(os, GTWidget::findWidget(os, "addSeq"));
//    Set "Show distances column" option.
    QCheckBox* showDistancesColumnCheck = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "showDistancesColumnCheck"));
    GTCheckBox::setChecked(os, showDistancesColumnCheck, true);
//    Expected state: the addition column is shown, it contains distances to the reference sequence.
    QString num1 = GTUtilsMSAEditorSequenceArea::getSimilarityValue(os, 1);
    QString num3 = GTUtilsMSAEditorSequenceArea::getSimilarityValue(os, 3);
    CHECK_SET_ERR(num1 == "19%", "unexpected sumilarity value an line 1: " + num1);
    CHECK_SET_ERR(num3 == "11%", "unexpected sumilarity value an line 3: " + num3);
//    Current state: the addition column is shown, it contains sequence names.


}

GUI_TEST_CLASS_DEFINITION(test_3283) {
    //    1. Open "data/Samples/MMDB/1CRN.prt".
    GTFileDialog::openFile(os, dataDir + "samples/MMDB", "1CRN.prt");
    //    2. Click to any annotation on the panoramic view.
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1CRN chain 1 annotation"));
    QTreeWidgetItem *item = GTUtilsAnnotationsTreeView::findItem(os, "sec_struct  (0, 5)");
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, item));
    GTMouseDriver::click(os);
}

GUI_TEST_CLASS_DEFINITION(test_3287) {
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    CHECK_OP(os, );

    ImageExportFormFiller::Parameters params;
    params.graphOverviewChecked = false;
    params.fileName = testDir + "_common_data/scenarios/sandbox/test_3287.bmp";
    params.format = "bmp";
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Export as image"));
    GTUtilsDialog::waitForDialog(os, new ImageExportFormFiller(os, params));

    QWidget *overview = GTWidget::findWidget(os, "msa_overview_area_graph");
    CHECK_OP(os, );
    GTWidget::click(os, overview, Qt::RightButton);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QImage image(params.fileName);
    CHECK_SET_ERR(70 == image.height(), "Wrong image height");
}

GUI_TEST_CLASS_DEFINITION(test_3288){
//1. Open "data/samples/CLUSTALW/HIV-1.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "HIV-1.aln");
//2. Click the "Build tree" button on the main toolbar.
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFillerPhyML(os, true));
    GTWidget::click(os, GTAction::button(os, "Build Tree"));
//3. Select the "PhyML" tool, set "Equilibrium frequencies" option to "opti,ized", build the tree
    GTGlobals::sleep(500);
    QProgressBar* taskProgressBar = GTWidget::findExactWidget<QProgressBar*>(os, "taskProgressBar");
    QString text = taskProgressBar->text();
    CHECK_SET_ERR(text.contains("%"), "unexpected text: " + text);
    GTUtilsTaskTreeView::waitTaskFinished(os);
//Expected state: the task progress is correct.
}

GUI_TEST_CLASS_DEFINITION(test_3305) {
    GTLogTracer logTracer;

//    1. Open "data/samples/FASTA/human_T1.fa".
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

//    2. Create an annotation.
    QDir().mkpath(sandBoxDir + "test_3305");
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "misc_feature", "1..5", sandBoxDir + "test_3305/test_3305.gb"));
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "create_annotation_action"));

//    3. Call context menu on the annotations document, select the {Export/Import -> Export annotations...} menu item.
//    4. Fill the dialog:
//        Export to file: any acceptable path;
//        File format: bed
//    and accept it.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << "ep_exportAnnotations2CSV"));
    GTUtilsDialog::waitForDialog(os, new ExportAnnotationsFiller(sandBoxDir + "test_3305/test_3305.bed", ExportAnnotationsFiller::bed, os));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "test_3305.gb"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

//    Expected state: the annotation is successfully exported, result file exists, there are no errors in the log.
    const QFile bedFile(sandBoxDir + "test_3305/test_3305.bed");
    CHECK_SET_ERR(bedFile.exists() && bedFile.size() != 0, "The result file is empty or does not exist!");

    GTUtilsLog::check(os, logTracer);
}

GUI_TEST_CLASS_DEFINITION(test_3306) {
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "sars.gb");

    GTUtilsAnnotationsTreeView::getItemCenter(os, "CDS  (0, 14)");
    GTUtilsAnnotationsTreeView::getItemCenter(os, "gene  (0, 13)");
    GTUtilsAnnotationsTreeView::getItemCenter(os, "mat_peptide  (0, 16)");
    GTUtilsAnnotationsTreeView::getItemCenter(os, "misc_feature  (0, 16)");

    QTreeWidget *annotTreeWidget = GTUtilsAnnotationsTreeView::getTreeWidget(os);
    QScrollBar *scrollBar = annotTreeWidget->verticalScrollBar();
    const int initialPos = scrollBar->value();

    for (int i = 0; i < 15; ++i) {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
        GTGlobals::sleep(200);
    }

    CHECK_SET_ERR(initialPos != scrollBar->value(), "ScrollBar hasn't moved");
}

GUI_TEST_CLASS_DEFINITION(test_3307){
    //1. Connect to shared database
    const QString folderName = "view_test_0001";
    const QString folderPath = U2ObjectDbi::PATH_SEP + folderName;
    const QString sequenceVisibleName = "NC_001363";
    const QString sequenceVisibleWidgetName = " [s] NC_001363";
    const QString databaseSequenceObjectPath = folderPath + U2ObjectDbi::PATH_SEP + sequenceVisibleName;

    Document* databaseDoc = GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    //2. Open any sequence from database
    GTUtilsSharedDatabaseDocument::openView(os, databaseDoc, databaseSequenceObjectPath);
    QWidget* seqView = GTWidget::findWidget(os, sequenceVisibleWidgetName);
    CHECK_SET_ERR(NULL != seqView, "View wasn't opened");

    //3. Use context menu on sequence area. Choose "new annotation"

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "ann1", "1.. 20"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "create_annotation_action"));
    GTMenu::showContextMenu(os, seqView);
    GTGlobals::sleep(500);
    GTUtilsTaskTreeView::waitTaskFinished(os, 60000);

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "MyDocument.gb"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_3308) {
//    1. Open "data/samples/PDB/1CF7.PDB".
    GTFileDialog::openFile(os, dataDir + "samples/PDB", "1CF7.PDB");

//    2. Call context menu on the 3dview, select {Structural Alignment -> Align With...} menu item.
//    3. Accept the dialog.
//    Expected state: UGENE doesn't crash.
    GTUtilsDialog::waitForDialog(os, new StructuralAlignmentDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Structural Alignment" << "align_with"));
    QWidget *widget3d = GTWidget::findWidget(os, "1-1CF7");
    CHECK_SET_ERR(NULL != widget3d, "3D widget was not found");
    GTWidget::click(os, widget3d, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_3312){
    GTLogTracer logTracer;

    //1. Connect to a shared database.
    Document* databaseDoc = GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    //2. Get any msa object in it.
    GTUtilsSharedDatabaseDocument::openView(os, databaseDoc, "/test_3312/COI_3312");

    QWidget* msaView = GTWidget::findWidget(os, " [m] COI_3312");
    CHECK_SET_ERR(NULL != msaView, "View wasn't opened");

    //3. Rename the object.
    //Expected state: object is successfully renamed, there are no errors in the log.
    GTUtilsProjectTreeView::rename(os, "COI_3312", "COI_3312_renamed");
    GTGlobals::sleep(3000);
    GTUtilsProjectTreeView::rename(os, "COI_3312_renamed", "COI_3312");
    GTGlobals::sleep(2000);

    GTUtilsSharedDatabaseDocument::disconnectDatabase(os, databaseDoc);

    GTUtilsLog::check(os, logTracer);
}

GUI_TEST_CLASS_DEFINITION(test_3313){
    //1. Open "data/samples/CLUSTALW/ty3.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "ty3.aln.gz");
    //2. Open "Statistics" options panel tab.
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTWidget::click(os, GTWidget::findWidget(os, "OP_SEQ_STATISTICS_WIDGET"));
    //3. Set any reference sequence.
    GTUtilsMSAEditorSequenceArea::click(os, QPoint(-5, 5));
    GTWidget::click(os, GTWidget::findWidget(os, "addSeq"));
    //4. Check the "Show distances column" option.
    QCheckBox* showDistancesColumnCheck = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "showDistancesColumnCheck"));
    GTCheckBox::setChecked(os, showDistancesColumnCheck, true);
    //5. Edit the msa fast enough, e.g. insert several gaps somewhere.

    GTUtilsMSAEditorSequenceArea::click( os, QPoint(10, 10));
    GTGlobals::sleep(200);

    for(int i = 0; i < 10; i++) {
        GTKeyboardDriver::keyClick( os, GTKeyboardDriver::key["space"] );
        GTGlobals::sleep(200);
    }
    GTGlobals::sleep();
    CHECK_SET_ERR(2 >= GTUtilsTaskTreeView::getTopLevelTasksCount(os), "There are several \"Generate distance matrix\" tasks");
}

GUI_TEST_CLASS_DEFINITION(test_3318) {
    // 1. Open human_T1.fa
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    CHECK_OP(os, );

    // 2. Open COI.aln
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    CHECK_OP(os, );

    // 3. Drag the sequence to the alignment
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_LOAD << "Sequence from current project"));
    GTUtilsDialog::waitForDialog(os, new ProjectTreeItemSelectorDialogFiller(os, "human_T1.fa", "human_T1 (UCSC April 2002 chr7:115977709-117855134)"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep();

    // 4. Make the sequence reference
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(-5, 18));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "set_seq_as_reference"));
    GTMouseDriver::click(os, Qt::RightButton);

    // 5. Change the highlighting mode to "Disagreements"
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::Highlighting);
    QComboBox *highlightingSchemeCombo = qobject_cast<QComboBox *>(GTWidget::findWidget(os, "highlightingScheme"));
    GTComboBox::setIndexWithText(os, highlightingSchemeCombo, "Disagreements");

    // 6. Use the dots
    QCheckBox *useDotsCheckBox = qobject_cast<QCheckBox *>(GTWidget::findWidget(os, "useDots"));
    GTCheckBox::setChecked(os, useDotsCheckBox);

    // 7. Drag the reference sequence in the list of sequences
    const QPoint mouseDragPosition(-5, 18);
    GTUtilsMSAEditorSequenceArea::moveTo(os, mouseDragPosition);
    GTMouseDriver::click(os);
    GTGlobals::sleep(1000);
    GTMouseDriver::press(os);
    GTGlobals::sleep(200);
    GTUtilsMSAEditorSequenceArea::moveTo(os, mouseDragPosition + QPoint(0, -10));
    GTMouseDriver::release(os);
    GTGlobals::sleep(200);

    // Expected result: the highlighting mode is the same, human_T1 is still the reference.
    CHECK_SET_ERR(highlightingSchemeCombo->currentText() == "Disagreements", "Invalid highlighting scheme");
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::isSequenceHightighted(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)"), "Unexpected reference sequence");
}

GUI_TEST_CLASS_DEFINITION(test_3328) {
    class TestBody_3328 : public QRunnable {
    public:
        TestBody_3328(U2OpStatus &os, QEventLoop *waiter) :
            QRunnable(),
            os(os),
            waiter(waiter) {}
        ~TestBody_3328() {
            waiter->exit();
        }

        void run() {
        //    1. Open "test/_common_data/fasta/human_T1_cutted.fa".
            GTFileDialog::openFile(os, GUITest::testDir + "/_common_data/fasta/", "human_T1_cutted.fa");

        //    2. Click the "Find restriction sites" button on the main toolbar.
        //    3. Select a single enzyme: "AbaBGI". Start the search.
            GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, QStringList() << "AbaBGI"));

            GTWidget::click(os, GTToolbar::getWidgetForActionTooltip(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "Find restriction sites..."));
            GTGlobals::sleep(2000);

        //    4. Close the sequence view until task has finished.
            GTUtilsMdi::click(os, GTGlobals::Close);

        //    Expected state: the task is canceled.
            CHECK_SET_ERR(0 == GTUtilsTaskTreeView::getTopLevelTasksCount(os), "There are unfinished tasks");
        }

    private:
        U2OpStatus &os;
        QEventLoop *waiter;
    };

    QThreadPool threadPool(this);
    QEventLoop waiter(this);

    TestBody_3328 *testBody = new TestBody_3328(os, &waiter);
    threadPool.start(testBody);
    waiter.exec();
}

GUI_TEST_CLASS_DEFINITION(test_3332) {
    //1. Open "data/sample/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    //2. Select {Edit -> Remove columns of gaps...} menu item from the context menu.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "remove_columns_of_gaps"));
    //3. Select the "all-gaps columns" option and accept the dialog.
    GTUtilsDialog::waitForDialog(os, new DeleteGapsDialogFiller(os, 1));
    GTMenu::showContextMenu(os, GTUtilsMSAEditorSequenceArea::getSequenceArea(os));
    //Expected state: nothing happens.
    CHECK_SET_ERR(GTUtilsMSAEditorSequenceArea::getLength(os) == 604, "Wrong msa length");
}

GUI_TEST_CLASS_DEFINITION(test_3333) {
//    1. Connect to the UGENE public database.

//    2. Drag and drop the object "/genomes/Arabidopsis thaliana (TAIR 10)/INFO" to the "/genomes/Arabidopsis thaliana (TAIR 10)" folder.
//    Expected state: nothing happens, there are no errors in the log.
    Document *connection = GTUtilsSharedDatabaseDocument::connectToUgenePublicDatabase(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTLogTracer logTracer;

    const QModelIndex object = GTUtilsSharedDatabaseDocument::getItemIndex(os, connection, "/genomes/Arabidopsis thaliana (TAIR 10)/INFO");
    const QModelIndex folder = GTUtilsSharedDatabaseDocument::getItemIndex(os, connection, "/genomes/Arabidopsis thaliana (TAIR 10)");
    GTUtilsProjectTreeView::dragAndDrop(os, object, folder);

    GTUtilsLog::check(os, logTracer);

    GTGlobals::FindOptions options;
    options.depth = 1;
    const int objectsCount = GTUtilsProjectTreeView::findIndecies(os, "", GTUtilsSharedDatabaseDocument::getItemIndex(os, connection, "/genomes/Arabidopsis thaliana (TAIR 10)"), 0, options).size();
    CHECK_SET_ERR(8 == objectsCount, QString("An unexpected objects count in the folder: expect %1, got %2").arg(8).arg(objectsCount));
}

GUI_TEST_CLASS_DEFINITION(test_3335) {
    GTLogTracer lt;

    //    1. Open "data/samples/FASTA/human_T1.fa".
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    //    2. Create an annotation.
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "test_3335", "misc_feature", "50..100", sandBoxDir + "test_3335/annotationTable.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ADD << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);

    //    Expected state: an annotation table object appears in a new document.
    GTUtilsDocument::checkDocument(os, "annotationTable.gb");

    //    3. Rename the sequence object.
    GTUtilsProjectTreeView::rename(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)", "renamed sequence");

    //    Expected state: the sequence object is renamed, object relations are correct, there are no errors in the log.
     const QModelIndex sequenceObjectIndex = GTUtilsProjectTreeView::findIndex(os, "renamed sequence");
    CHECK_SET_ERR(sequenceObjectIndex.isValid(), "Can't find the renamed sequence object");

     GTUtilsMdi::click(os, GTGlobals::Close);
     GTUtilsProjectTreeView::doubleClickItem(os, "Annotations");
     GTGlobals::sleep(60000);
    QWidget *relatedSequenceView = GTUtilsMdi::findWindow(os, "human_T1 [s] renamed sequence");
    CHECK_SET_ERR(NULL != relatedSequenceView, "A view for the related sequence was not opened");

    GTUtilsLog::check(os, lt);
}

GUI_TEST_CLASS_DEFINITION(test_3342) {
//    1. Open "human_T1.fa"
//    2. Press "Build dotlpot" toolbar button
//    Expected state: "DotPlot" dialog appeared
//    3. Press "Ok" button in the dialog
//    Expected state: Dotplot view has appeared
//    4. Close dotplot view
//    Expected state: "Save dot-plot" dialog has appeared
//    5. Press "Yes" button
//    Expected state: the view has been closed
//    Current state: the view can't be closed

    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
    const GTGlobals::FindOptions fo(false);
    QWidget* dotplotWgt = GTWidget::findWidget(os, "dotplot widget", NULL, fo);
    CHECK_SET_ERR(dotplotWgt == NULL, "There should be NO dotpot widget");

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os));
    GTWidget::click(os, GTWidget::findWidget(os, "build_dotplot_action_widget"));
    GTGlobals::sleep();

    dotplotWgt = GTWidget::findWidget(os, "dotplot widget");
    CHECK_SET_ERR(dotplotWgt != NULL, "No dotpot widget");

    GTUtilsDialog::waitForDialog(os, new MessageBoxNoToAllOrNo(os));
    GTWidget::click(os, GTWidget::findWidget(os, "exitButton"));
    GTGlobals::sleep();

    dotplotWgt = GTWidget::findWidget(os, "dotplot widget", NULL, fo);
    CHECK_SET_ERR(dotplotWgt == NULL, "There should be NO dotpot widget");
}

GUI_TEST_CLASS_DEFINITION(test_3344) {
//    Steps to reproduce:
//    1. Open "human_T1"
//    2. Press "Find repeats" tool button
//    3. Choose following settings in the dialog: region="whole sequence", min repeat length=10bp
//    4. Press "start"
//    5. Wait until repeats finding complete
//    Expected state: repeats finding completed and create annotations task started
//    6. Delete "repeat_unit" annotations group
//    Current state: UGENE hangs
    GTLogTracer l;

    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    Runnable * tDialog = new FindRepeatsDialogFiller(os, testDir + "_common_data/scenarios/sandbox/test_3344.gb",
                                                     false, 10);
    GTUtilsDialog::waitForDialog(os, tDialog);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE
        << "find_repeats_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "Annotations [test_3344.gb] *"));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "repeat_unit  (0, 3486)"));

    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3346) {
    GTLogTracer lt;

    QFile originalFile(dataDir + "samples/Genbank/murine.gb");
    CHECK_SET_ERR(originalFile.exists(), "Unable to find original file");

    const QString dstPath = sandBoxDir + "murine.gb";
    originalFile.copy(dstPath);
    QFile copiedFile(dstPath);
    CHECK_SET_ERR(copiedFile.exists(), "Unable to copy file");

    GTFileDialog::openFile(os, sandBoxDir, "murine.gb");
    CHECK_OP(os, );

    if (!copiedFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        os.setError("Unable to open file");
        return;
    }
    QString fileData = copiedFile.readAll();
    copiedFile.close();
    fileData.replace("\"gag polyprotein\"", "\"gag polyprotein");

    if (!copiedFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        os.setError("Unable to open file");
        return;
    }

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));

    QTextStream out(&copiedFile);
    out << fileData;
    copiedFile.close();

    GTGlobals::sleep(6000);

    CHECK_SET_ERR(lt.hasError(), "Error in log expected");
}

GUI_TEST_CLASS_DEFINITION(test_3348) {
    GTFileDialog::openFile(os, testDir + "_common_data/cmdline/", "DNA.fa");
    GTUtilsDocument::checkDocument(os, "DNA.fa");

    Runnable * findDialog = new FindRepeatsDialogFiller(os, testDir + "_common_data/scenarios/sandbox/", true, 10, 75, 100);
    GTUtilsDialog::waitForDialog(os, findDialog);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_ANALYSE << "find_repeats_action", GTGlobals::UseMouse));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(6000);

    QTreeWidget *treeWidget = GTUtilsAnnotationsTreeView::getTreeWidget(os);
    CHECK_SET_ERR(treeWidget != NULL, "Tree widget is NULL");

    QTreeWidgetItem *annotationGroup = GTUtilsAnnotationsTreeView::findItem(os, "repeat_unit  (0, 39)");
    QTreeWidgetItem *generalItem = annotationGroup->child(36);
    CHECK_SET_ERR(generalItem != NULL, "Invalid annotation tree item");

    AVAnnotationItem *annotation = dynamic_cast<AVAnnotationItem *>(generalItem);
    CHECK_SET_ERR(NULL != annotation, "Annotation tree item not found");
    CHECK_SET_ERR("76" == annotation->annotation.findFirstQualifierValue("repeat_homology(%)"), "Annotation qualifier not found");

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTMouseDriver::click(os);
}

GUI_TEST_CLASS_DEFINITION(test_3357) {
/*  1. Open file _common_data\alphabets\standard_dna_rna_amino_1000.fa
    2. Click the CV button on seq3 widget
    3. Select document in project view. Press delete key
    Expected state: UGENE doesn't crash.
*/
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTFileDialog::openFile(os, testDir +"_common_data/alphabets/", "standard_dna_rna_amino_1000.fa");
    GTGlobals::sleep(1000);
    QWidget* w=GTWidget::findWidget(os, "ADV_single_sequence_widget_1");
    GTWidget::click(os, GTWidget::findWidget(os, "CircularViewAction",w));
    GTGlobals::sleep(500);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "standard_dna_rna_amino_1000.fa"));
    GTMouseDriver::click( os );
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_3373) {
//    1. Launch WD
//    2. Create the following workflow: "Read Sequence" -> "Reverse Complement" -> "Write Sequence"
//    3. Set output format "genbank" (to prevent warnings about annotation support) and the "result.gb" output file name
//    4. Set input file "test/_common_data/fasta/seq1.fa"
//    5. Run the workflow
//    Expected state: workflow is successfully finished. "result.gb" contains reverse complement sequence for "seq1.fa"
    GTLogTracer l;

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    GTUtilsWorkflowDesigner::addAlgorithm( os, "Read Sequence");
    GTUtilsWorkflowDesigner::addAlgorithm( os, "Reverse Complement");
    GTUtilsWorkflowDesigner::addAlgorithm( os, "Write Sequence");

    WorkflowProcessItem *seqReader = GTUtilsWorkflowDesigner::getWorker( os, "Read Sequence");
    WorkflowProcessItem *revComplement = GTUtilsWorkflowDesigner::getWorker( os, "Reverse Complement");
    WorkflowProcessItem *seqWriter = GTUtilsWorkflowDesigner::getWorker( os, "Write Sequence");

    GTUtilsWorkflowDesigner::connect( os, seqReader, revComplement);
    GTUtilsWorkflowDesigner::connect( os, revComplement, seqWriter);

    GTMouseDriver::moveTo( os, GTUtilsWorkflowDesigner::getItemCenter( os, "Write Sequence" ) );
    GTMouseDriver::click( os );
    GTUtilsWorkflowDesigner::setParameter(os, "Document format", 2, GTUtilsWorkflowDesigner::comboValue);
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", "result.gb", GTUtilsWorkflowDesigner::textValue);
    GTWidget::click( os, GTUtilsMdi::activeWindow( os ) );

    GTMouseDriver::moveTo( os, GTUtilsWorkflowDesigner::getItemCenter( os, "Read Sequence"));
    GTMouseDriver::click( os);
    QString dirPath = testDir + "_common_data/fasta/";
    GTUtilsWorkflowDesigner::setDatasetInputFile( os, dirPath, "seq1.fa" );

    GTWidget::click( os, GTAction::button(os,"Run workflow"));

    GTUtilsLog::check( os, l );
}

GUI_TEST_CLASS_DEFINITION(test_3379) {
    //1. Open "_common_data/fasta/abds.fa" as separate sequences
    QStringList expectedNames;
    QList<ADVSingleSequenceWidget*> seqWidgets;
    expectedNames << "seq1" << "seq3" << "seq5";

    seqWidgets = GTUtilsProject::openFileExpectSequences(os,
        testDir + "_common_data/alphabets",
        "standard_dna_rna_amino_1000.fa",
        expectedNames);
    CHECK_OP_SET_ERR(os, "Failed to open sequences!");

    //2. Open a few CV
    ADVSingleSequenceWidget *seq1Widget = seqWidgets.at(0);
    ADVSingleSequenceWidget *seq3Widget = seqWidgets.at(1);

    GTUtilsCv::cvBtn::click(os, seq3Widget);
    GTUtilsCv::cvBtn::click(os, seq1Widget);

    QWidget* circularView = GTWidget::findWidget(os, "CV_ADV_single_sequence_widget_0");
    CHECK_OP_SET_ERR(os, "Failed to open circular view!");

    //3.Add more files to the project and open a few more views
    GTFileDialog::openFile(os, testDir + "_common_data/cmdline/", "DNA.fa");
    GTGlobals::sleep(500);

    //4. Return to 'abcd.fa' view
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "seq1"));
    GTMouseDriver::doubleClick(os);

    //5. Try to launch Export dialog using context menu
    GTUtilsDialog::waitForDialog(os, new CircularViewExportImage(os, testDir + "_common_data/scenarios/sandbox/image.jpg"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< ADV_MENU_EXPORT << "Save circular view as image", GTGlobals::UseMouse));

    GTWidget::click(os, circularView, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_3384){
    GTLogTracer l;
//    Open sequence data/samples/Genbank/murine.gb
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "murine.gb");
//    Open CV
    GTWidget::click(os, GTWidget::findWidget(os, "CircularViewAction"));
//    Insert at least one symbol to the sequence
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_EDIT"
                                                      << "action_edit_insert_sub_sequences"));
    GTUtilsDialog::waitForDialog(os, new InsertSequenceFiller(os, "A"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"));

//    Select an area on CV that contains zero position
    QWidget* cv = GTWidget::findWidget(os, "CV_ADV_single_sequence_widget_0");
    GTWidget::click(os, cv);
    GTMouseDriver::moveTo(os, GTMouseDriver::getMousePosition() + QPoint(20, -20));
    GTMouseDriver::press(os);
    GTMouseDriver::moveTo(os, GTMouseDriver::getMousePosition() + QPoint(0,40));
    GTMouseDriver::release(os);
//    Current state: SAFE_POINT triggers and selection is "beautiful" (see the attachment)
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3396){
//Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
//Add macs worker
    GTUtilsWorkflowDesigner::addAlgorithm(os, "macs");
//set paremeter wiggle output to false
    GTUtilsWorkflowDesigner::click(os, "macs");
    GTUtilsWorkflowDesigner::setParameter(os, "Wiggle output", 0, GTUtilsWorkflowDesigner::comboValue);
    GTUtilsWorkflowDesigner::click(os, "macs");
    GTGlobals::sleep(500);
//Expected state: parrameter wiggle space is hidden
    QStringList parameters = GTUtilsWorkflowDesigner::getAllParameters(os);
    CHECK_SET_ERR(!parameters.contains("Wiggle space"), "Wiggle space parameter is shown");

}

GUI_TEST_CLASS_DEFINITION(test_3398_1) {
//    1. Open "_common_data/fasta/broken/data_in_the_name_line.fa".
//    2. Select "As separate sequences" mode.
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/broken", "data_in_the_name_line.fa");

//    Expected state: an unloaded document appears, there are no objects within.
    GTUtilsTaskTreeView::waitTaskFinished(os);
    Document *doc = GTUtilsDocument::getDocument(os, "data_in_the_name_line.fa");
    CHECK_SET_ERR(NULL != doc, "Document is NULL");
    CHECK_SET_ERR(!doc->isLoaded(), "Document is unexpectedly loaded");

//    3. Call context menu on the document.
//    Expected state: UGENE doesn't crash, a context menu is shown.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_load_selected_documents"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "data_in_the_name_line.fa"));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_3398_2) {
    //    1. Open "_common_data/fasta/broken/data_in_the_name_line.fa".
    //    2. Select "As separate sequences" mode.
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Merge, 10));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/broken", "data_in_the_name_line.fa");

    //    Expected state: an unloaded document appears, there are no objects within.
    GTUtilsTaskTreeView::waitTaskFinished(os);
    Document *doc = GTUtilsDocument::getDocument(os, "data_in_the_name_line.fa");
    CHECK_SET_ERR(NULL != doc, "Document is NULL");
    CHECK_SET_ERR(!doc->isLoaded(), "Document is unexpectedly loaded");

    //    3. Call context menu on the document.
    //    Expected state: UGENE doesn't crash, a context menu is shown.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_load_selected_documents"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "data_in_the_name_line.fa"));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_3398_3) {
    //    1. Open "_common_data/fasta/broken/data_in_the_name_line.fa".
    //    2. Select "Merge into one sequence" mode, set 10 'unknown' symbols.
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Merge, 0));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/broken", "data_in_the_name_line.fa");

    //    Expected state: an unloaded document appears, there are no objects within.
    GTUtilsTaskTreeView::waitTaskFinished(os);
    Document *doc = GTUtilsDocument::getDocument(os, "data_in_the_name_line.fa");
    CHECK_SET_ERR(NULL != doc, "Document is NULL");
    CHECK_SET_ERR(!doc->isLoaded(), "Document is unexpectedly loaded");

    //    3. Call context menu on the document.
    //    Expected state: UGENE doesn't crash, a context menu is shown.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_load_selected_documents"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "data_in_the_name_line.fa"));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_3398_4) {
    //    1. Open "_common_data/fasta/broken/data_in_the_name_line.fa".
    //    2. Select "As separate sequences" mode.
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Join));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/broken", "data_in_the_name_line.fa");

    //    Expected state: an unloaded document appears, there are no objects within.
    GTUtilsTaskTreeView::waitTaskFinished(os);
    Document *doc = GTUtilsDocument::getDocument(os, "data_in_the_name_line.fa");
    CHECK_SET_ERR(NULL != doc, "Document is NULL");
    CHECK_SET_ERR(!doc->isLoaded(), "Document is unexpectedly loaded");

    //    3. Call context menu on the document.
    //    Expected state: UGENE doesn't crash, a context menu is shown.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_load_selected_documents"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "data_in_the_name_line.fa"));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_3437){
//    1. Open file test/_common_data/fasta/empty.fa
    GTFileDialog::openFile(os, testDir + "_common_data/fasta", "empty.fa");
//    Expected: file opened in msa editor
    QWidget* w = GTWidget::findWidget(os,"msa_editor_sequence_area");
    CHECK_SET_ERR(w != NULL, "msa editor not opened");
}

GUI_TEST_CLASS_DEFINITION(test_3402){
//    Open "test/_common_data/clustal/100_sequences.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "fungal - all.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    Call context menu on the "100_sequences" object.

    GTUtilsDialog::waitForDialog(os, new ExportToSequenceFormatFiller(os, sandBoxDir, "test_3402.fa",
                                                                      ExportToSequenceFormatFiller::FASTA, true, true));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_project__export_import_menu_action"
     << "action_project__export_as_sequence_action"));

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "fungal - all.aln"));
    GTMouseDriver::click(os, Qt::RightButton);
    //GTUtilsTaskTreeView::waitTaskFinished(os);
//    Select {Export/Import -> Export alignmnet to sequence format...}

//    menu item.
//    Export somewhere as fasta, sure that "Add document to the project" checkbox is checked.
//    Wait until open view task will start.
    TaskScheduler* scheduller = AppContext::getTaskScheduler();

    bool end = false;
    while(!end){
        QList<Task*> tList = scheduller->getTopLevelTasks();
        if(tList.isEmpty()){
            continue;
        }
        QList<Task*> innertList;
        foreach (Task* t, tList) {
            innertList.append(t->getSubtasks());
        }
        foreach (Task* t, innertList) {
            if(t->getTaskName().contains("Opening view")){
                end = true;
                break;
            }
        }
        GTGlobals::sleep(100);
    }

//    Expected state: the fasta document is present in the project, open view task is in progress.
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "test_3402.fa"));
//    Delete the fasta document from the project.
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(500);
//    Current state: UGENE not crashes.
}

GUI_TEST_CLASS_DEFINITION(test_3430) {
    //1. Open "_common_data/alphabets/standard_dna_rna_amino_1000.fa" as separate sequences
    QStringList expectedNames;
    QList<ADVSingleSequenceWidget*> seqWidgets;
    expectedNames << "seq1" << "seq3" << "seq5";

    seqWidgets = GTUtilsProject::openFileExpectSequences(os,
        testDir + "_common_data/alphabets",
        "standard_dna_rna_amino_1000.fa",
        expectedNames);
    CHECK_OP_SET_ERR(os, "Failed to open sequences!");

    //2. Open one circular view
    ADVSingleSequenceWidget *seq1Widget = seqWidgets.at(0);

    GTUtilsCv::cvBtn::click(os, seq1Widget);

    QWidget* circularView1 = GTWidget::findWidget(os, "CV_ADV_single_sequence_widget_0");
    CHECK_OP_SET_ERR(os, "Failed to open circular view!");

    QWidget* circularView2 = GTWidget::findWidget(os, "CV_ADV_single_sequence_widget_1", NULL, GTGlobals::FindOptions(false));
    CHECK_SET_ERR(NULL == circularView2, "Unexpected circular view is opened!");

    //3. Press "Toggle circular views" button

    GTWidget::click(os, GTWidget::findWidget(os, "globalToggleViewAction_widget"));

    circularView1 = GTWidget::findWidget(os, "CV_ADV_single_sequence_widget_0", NULL, GTGlobals::FindOptions(false));
    CHECK_SET_ERR(NULL == circularView1, "Unexpected circular view is opened!");

    circularView2 = GTWidget::findWidget(os, "CV_ADV_single_sequence_widget_1", NULL, GTGlobals::FindOptions(false));
    CHECK_SET_ERR(NULL == circularView2, "Unexpected circular view is opened!");
    //4. Press "Toggle circular views" again
    GTWidget::click(os, GTWidget::findWidget(os, "globalToggleViewAction_widget"));

    GTWidget::findWidget(os, "CV_ADV_single_sequence_widget_0");
    CHECK_OP_SET_ERR(os, "Failed to open circular view!");

    GTWidget::findWidget(os, "CV_ADV_single_sequence_widget_1");
    CHECK_OP_SET_ERR(os, "Failed to open circular view!");
}

GUI_TEST_CLASS_DEFINITION(test_3439){
    //Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    //Add macs worker
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Write Alignment");
    //Validate workflow
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTWidget::click(os,GTAction::button(os, "Validate workflow"));
    GTGlobals::sleep();
    //there is should be 2 errors
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::checkErrorList(os, "Write Alignment") == 2, "Errors count dont match, should be 2 validation errors");
    //set paramter "Data storage" to "Shared UGENE database"
    GTUtilsWorkflowDesigner::click(os, "Write Alignment");
    GTGlobals::sleep();
    GTUtilsWorkflowDesigner::setParameter(os, "Data storage", 1, GTUtilsWorkflowDesigner::comboValue);
    //Validate workflow
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTWidget::click(os,GTAction::button(os, "Validate workflow"));
    GTGlobals::sleep();
    //there is should be 3 errors
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::checkErrorList(os, "Write Alignment") == 3, "Errors count dont match, should be 2 validation errors");
}

GUI_TEST_CLASS_DEFINITION(test_3441) {
    //    Open file test_common_data\fasta\empty.fa
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "empty.fa");
    //    Open "Statistics" options panel tab.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_GENERAL"));
    //Sequence count is 0

    QLabel *seqCount = GTWidget::findExactWidget<QLabel*>(os, "alignmentHeight");
    CHECK_SET_ERR(seqCount->text() == "0", "Sequence count don't match");
}

GUI_TEST_CLASS_DEFINITION(test_3443) {
    GTKeyboardDriver::keyClick(os, '3', GTKeyboardDriver::key["alt"]);
    GTGlobals::sleep();

    QWidget *logViewWidget = GTWidget::findWidget(os, "dock_log_view");
    CHECK_SET_ERR(logViewWidget->isVisible(), "Log view is expected to be visible");

    GTKeyboardDriver::keyClick(os, '3', GTKeyboardDriver::key["alt"]);
    GTGlobals::sleep();
    CHECK_SET_ERR(!logViewWidget->isVisible(), "Log view is expected to be visible");

    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
    GTGlobals::sleep(500);

    QWidget *projectViewWidget = GTWidget::findWidget(os, "project_view");

    GTKeyboardDriver::keyClick(os, '1', GTKeyboardDriver::key["alt"]);
    GTGlobals::sleep();

    CHECK_SET_ERR(!projectViewWidget->isVisible(), "Project view is expected to be invisible");

    GTKeyboardDriver::keyClick(os, '1', GTKeyboardDriver::key["alt"]);
    GTGlobals::sleep();

    CHECK_SET_ERR(projectViewWidget->isVisible(), "Project view is expected to be visible");

    GTKeyboardDriver::keyClick(os, '2', GTKeyboardDriver::key["alt"]);
    GTGlobals::sleep();

    QWidget *taskViewWidget = GTWidget::findWidget(os, "dock_task_view");
    CHECK_SET_ERR(taskViewWidget->isVisible(), "Task view is expected to be visible");

    GTKeyboardDriver::keyClick(os, '2', GTKeyboardDriver::key["alt"]);
    GTGlobals::sleep();

    CHECK_SET_ERR(!taskViewWidget->isVisible(), "Task view is expected to be invisible");

    GTKeyboardDriver::keyClick(os, 'b', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    QWidget *codonTableWidget = GTWidget::findWidget(os, "Codon table widget");
    CHECK_SET_ERR(codonTableWidget->isVisible(), "Codon table is expected to be visible");

    GTKeyboardDriver::keyClick(os, 'b', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    CHECK_SET_ERR(!codonTableWidget->isVisible(), "Codon table is expected to be invisible");
}

GUI_TEST_CLASS_DEFINITION(test_3450) {
//    1. Open file "COI.aln"
//    2. Open "Highlighting" options panel tab
//    3. Set reference sequence
//    4. Set highlighting scheme
//    5. Press "Export" button in the tab
//    Expected state: "Export highlighted to file" dialog appeared, there is default file in "Export to file"
//    6. Delete the file path and press "Export" button
//    Expected state: message box appeared
//    6. Set the result file and press "Export" button
//    Current state: file is empty, but error is not appeared

    GTFileDialog::openFile(os, dataDir + "/samples/CLUSTALW/", "COI.aln");
    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_HIGHLIGHTING"));
    GTWidget::click(os, GTWidget::findWidget(os, "sequenceLineEdit"));
    GTKeyboardDriver::keySequence(os, "Montana_montana");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep(300);

    QComboBox* combo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "highlightingScheme"));
    CHECK_SET_ERR(combo != NULL, "highlightingScheme not found!");
    GTComboBox::setIndexWithText(os, combo , "Agreements");

    QWidget* exportButton = GTWidget::findWidget(os, "exportHighlightning");
    CHECK_SET_ERR(exportButton != NULL, "exportButton not found");

    class ExportHighlightedDialogFiller : public Filler {
    public:
        ExportHighlightedDialogFiller(U2OpStatus &os)
            : Filler(os, "ExportHighlightedDialog") {}
        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR( dialog, "activeModalWidget is NULL");

            QLineEdit *filePath = dialog->findChild<QLineEdit*>("fileNameEdit");
            CHECK_SET_ERR( filePath != NULL, "fileNameEdit is NULL");
            CHECK_SET_ERR( !GTLineEdit::copyText(os, filePath).isEmpty(), "Default file path is empty");
            GTLineEdit::setText(os, filePath, "");

            QPushButton *exportButton = dialog->findChild<QPushButton*>();
            CHECK_SET_ERR(exportButton != NULL, "ExportButton is NULL");

            GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "Ok"));
            GTWidget::click(os, exportButton);

            GTLineEdit::setText(os, filePath, sandBoxDir + "test_3450_export_hl.txt");
            GTWidget::click(os, exportButton);
        }
    };

    GTUtilsDialog::waitForDialog(os, new ExportHighlightedDialogFiller(os));
    GTWidget::click(os, exportButton);
    GTGlobals::sleep();

    CHECK_SET_ERR(GTFile::getSize(os, sandBoxDir + "test_3450_export_hl.txt") != 0, "Exported file is empty!");
}

GUI_TEST_CLASS_DEFINITION(test_3451) {
    //    1. Open file "COI.aln"
    //    2. Open "Highlighting" options panel tab
    //    3. Set reference sequence
    //    4. Set highlighting scheme
    //    5. Press "Export" button in the tab
    //    Expected state: "Export highlighted to file" dialog appeared, there is default file in "Export to file"
    //    6. Set the "from" as 5, "to" as 6
    //    7. Set the "to" as 5
    //    Expected state: "from" is 4


    GTFileDialog::openFile(os, dataDir + "/samples/CLUSTALW/", "COI.aln");
    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_HIGHLIGHTING"));
    GTWidget::click(os, GTWidget::findWidget(os, "sequenceLineEdit"));
    GTKeyboardDriver::keySequence(os, "Montana_montana");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep(300);

    QComboBox* combo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "highlightingScheme"));
    CHECK_SET_ERR(combo != NULL, "highlightingScheme not found!");
    GTComboBox::setIndexWithText(os, combo , "Agreements");

    QWidget* exportButton = GTWidget::findWidget(os, "exportHighlightning");
    CHECK_SET_ERR(exportButton != NULL, "exportButton not found");

    class CancelExportHighlightedDialogFiller : public Filler {
    public:
        CancelExportHighlightedDialogFiller(U2OpStatus &os)
            : Filler(os, "ExportHighlightedDialog") {}
        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR( dialog, "activeModalWidget is NULL");

            QSpinBox *startPos = dialog->findChild<QSpinBox*>("startPosBox");
            CHECK_SET_ERR( startPos != NULL, "startPosBox is NULL");

            QSpinBox *endPos = dialog->findChild<QSpinBox*>("endPosBox");
            CHECK_SET_ERR( endPos != NULL, "endPosBox is NULL");

            GTSpinBox::checkLimits(os, startPos, 1, 603);
            GTSpinBox::checkLimits(os, endPos, 2, 604);

            //GTGlobals::sleep();

            QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
            CHECK_SET_ERR(box != NULL, "buttonBox is NULL");
            QPushButton* button = box->button(QDialogButtonBox::Cancel);
            CHECK_SET_ERR(button !=NULL, "Cancel button is NULL");
            GTWidget::click(os, button);

        }
    };

    GTUtilsDialog::waitForDialog(os, new CancelExportHighlightedDialogFiller(os));
    GTWidget::click(os, exportButton);
    GTGlobals::sleep();

}

GUI_TEST_CLASS_DEFINITION(test_3452) {
    //1. Open "samples/Genbank/murine.gb".
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "murine.gb");

    //2. Find the annotation: NC_001363 -> CDS (0, 4) -> CDS.
    //Expected state: qualifiers of CDS are shown.
    QTreeWidgetItem *item1 = GTUtilsAnnotationsTreeView::findItem(os, "CDS");
    QTreeWidgetItem *item2 = GTUtilsAnnotationsTreeView::findItem(os, "comment");
    QPoint p1 = GTTreeWidget::getItemCenter(os, item1);
    QPoint pQual(p1.x(), p1.y() + 80);
    QPoint p2 = GTTreeWidget::getItemCenter(os, item2);

    //3. Select the annotaions and its several qualifiers.
    GTMouseDriver::moveTo(os, p1);
    GTMouseDriver::click(os);
    GTMouseDriver::moveTo(os, pQual);
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep(500);

    //4. Try to drag selected annotaions.
    GTMouseDriver::dragAndDrop(os, p1, p2);
    //Expected state: UGENE does not crash.
}

GUI_TEST_CLASS_DEFINITION(test_3455) {
    //1. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Activate samples.
    GTUtilsWorkflowDesigner::setCurrentTab(os, GTUtilsWorkflowDesigner::samples);

    //3. Choose a sample (but not open it).
    QTreeWidgetItem *sample = GTUtilsWorkflowDesigner::findTreeItem(os, "call variants", GTUtilsWorkflowDesigner::samples);
    sample->parent()->setExpanded(true);
    GTMouseDriver::moveTo(os,GTTreeWidget::getItemCenter(os, sample));
    GTMouseDriver::click(os);
    CHECK_OP(os, );

    //4. Load any workflow.
    QString schemaPath = testDir + "_common_data/scenarios/workflow designer/222.uwl";
    GTUtilsWorkflowDesigner::loadWorkflow(os, schemaPath);
    CHECK_OP(os, );

    //Expected: the elements tab is active.
    GTUtilsWorkflowDesigner::tab current = GTUtilsWorkflowDesigner::currentTab(os);
    CHECK_SET_ERR(GTUtilsWorkflowDesigner::algoriths == current, "Samples tab is active");
}

GUI_TEST_CLASS_DEFINITION(test_3471) {
//    1. Open "_common_data\bam\1.bam"
//    Expected state: "Import BAM file dialog" appeared
//    2. Press "Import" button in the dialog
//    Expected state: assembly is empty, there is text "Assembly has no mapped reads. Nothing to visualize."
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, sandBoxDir + "test_3471/test_3471.ugenedb"));
    GTFileDialog::openFile(os, testDir + "_common_data/bam/", "1.bam");

//    3. Add bookmark
//    Expected state: UGENE doesn't crash
    GTUtilsBookmarksTreeView::addBookmark(os, GTUtilsMdi::activeWindow(os)->windowTitle(), "test_3471");
}

GUI_TEST_CLASS_DEFINITION(test_3472) {
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::PairwiseAlignment);

    GTUtilsOptionPanelMsa::addFirstSeqToPA(os, "Conocephalus_discolor");
    GTUtilsOptionPanelMsa::addSecondSeqToPA(os, "Conocephalus_sp.");

    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Output settings"));

    QLineEdit *outputFilePathEdit = qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "outputFileLineEdit"));
    CHECK_SET_ERR(NULL != outputFilePathEdit, "Invalid output file path edit field");

    GTWidget::setFocus(os, outputFilePathEdit);
#ifndef Q_OS_MAC
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["home"]);
#else
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["left"], GTKeyboardDriver::key["cmd"]);
#endif
    GTGlobals::sleep();

    GTKeyboardDriver::keySequence(os, "///123/123/123");
    GTGlobals::sleep();

    int deleteCounter = 100;
    while (0 < --deleteCounter) {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
        GTGlobals::sleep(100);
    }

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTWidget::click(os, GTWidget::findWidget(os, "alignButton"));

    GTGlobals::sleep();

    GTWidget::setFocus(os, outputFilePathEdit);
#ifndef Q_OS_MAC
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["home"]);
#else
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["left"], GTKeyboardDriver::key["cmd"]);
#endif
    GTGlobals::sleep();

    GTKeyboardDriver::keySequence(os, sandBoxDir + "123/123/123/1.aln");
    GTGlobals::sleep();

    deleteCounter = 15;
    while (0 < --deleteCounter) {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
        GTGlobals::sleep(100);
    }

    GTWidget::click(os, GTWidget::findWidget(os, "alignButton"));

    GTGlobals::sleep();
    QString expected = "TTAGCTTATTAATT\n"
                       "TTAGCTTATTAATT";
    GTUtilsMSAEditorSequenceArea::checkSelection(os, QPoint(0, 0), QPoint(13, 1), expected);
}

GUI_TEST_CLASS_DEFINITION(test_3473) {
/*  1. Open "human_T1"
    2. Press "Show circular view" tool button
        Expected state: circular view appeared and button's hint change to "Remove circular view"
    3. Press the button again
        Expected state: the hint is "Show circular view"
*/
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(1000);
    QToolButton* a = qobject_cast<QToolButton*>(GTWidget::findWidget(os, "CircularViewAction"));
    CHECK_SET_ERR(a->toolTip() == "Show circular view", QString("Unexpected tooltip: %1, must be %2").arg(a->toolTip()).arg("Show circular view"));
    GTWidget::click(os, GTWidget::findWidget(os, "CircularViewAction"));
    GTGlobals::sleep(500);
    CHECK_SET_ERR(a->toolTip() == "Remove circular view", QString("Unexpected tooltip: %1, must be %2").arg(a->toolTip()).arg("Remove circular view"));
    GTGlobals::sleep(500);

}

GUI_TEST_CLASS_DEFINITION(test_3477) {
//    1. Open "data/samples/human_T1.fa".
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

//    2. Press "Find restriction sites" tool button.
//    Expected state: "Find restriction sites" dialog appeared.
//    3. Select enzyme "T(1, 105) -> TaaI" and accept the dialog.
    GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, QStringList() << "TaaI"));
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "Find restriction sites"));
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    4. Select {Tools -> Cloning -> Digest into Fragments...} menu item in the main menu.
//    Expected state: "Digest sequence into fragments" dialog appeared.
//    5. Add "TaaI" to selected enzymes and accept the dialog.
    GTUtilsDialog::waitForDialog(os, new DigestSequenceDialogFiller(os));
    GTMenu::clickMenuItemByText(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Cloning" << "Digest into Fragments...");
    GTUtilsTaskTreeView::waitTaskFinished(os);

//    6. Select {Tools -> Cloning -> Construct molecule} menu item in the main menu.
//    Expected state: "Construct molecule" dialog appeared.
//    7. Press "Add all" button.
//    9.Press several times to checkbox "Inverted" for any fragment.
//    Expected state: checkbox's state updates on every click, UGENE doesn't crash.
    QList<ConstructMoleculeDialogFiller::Action> actions;
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::AddAllFragments, "");
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::InvertAddedFragment, "Fragment 1");
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::InvertAddedFragment, "Fragment 1");
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::InvertAddedFragment, "Fragment 1");
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::InvertAddedFragment, "Fragment 1");
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::InvertAddedFragment, "Fragment 1");
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::InvertAddedFragment, "Fragment 1");
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::InvertAddedFragment, "Fragment 1");
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::InvertAddedFragment, "Fragment 1");
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::InvertAddedFragment, "Fragment 1");
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::InvertAddedFragment, "Fragment 1");
    actions << ConstructMoleculeDialogFiller::Action(ConstructMoleculeDialogFiller::ClickCancel, "");

    GTUtilsDialog::waitForDialog(os, new ConstructMoleculeDialogFiller(os, actions));
    GTMenu::clickMenuItemByText(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Cloning" << "Construct Molecule...");
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_3478) {
    //1. Open _common_data/fasta/multy_fa.fa
    //2. File context menu -> BLAST -> Format DB
    //Expected: there are not safe points errors in the log
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Separate));
    GTUtilsProject::openFiles(os, testDir + "_common_data/fasta/multy_fa.fa");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "multy_fa.fa"));
    GTLogTracer l;
    FormatDBSupportRunDialogFiller::Parameters p;
    p.justCancel = true;
    GTUtilsDialog::waitForDialog(os, new FormatDBSupportRunDialogFiller(os, p));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "BLAST" << "FormatDB"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3484) {
//    1. Open an alignment
//    2. Build the tree
//    3. Unload both documents (alignment and tree)
//    4. Delete the tree document from project
//    5. Load alignment
//    Expected state: only alignment is opened.
    GTFile::copy(os, dataDir + "samples/CLUSTALW/COI.aln", testDir + "_common_data/scenarios/sandbox/COI_3484.aln");

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/sandbox/", "COI_3484.aln");
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os,
                                                               testDir + "_common_data/scenarios/sandbox/COI_3484.nwk",
                                                               0, 0, true));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep(500);
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found");

    GTUtilsDocument::unloadDocument(os, "COI_3484.nwk", false);
    GTGlobals::sleep(500);
    GTUtilsDocument::saveDocument(os, "COI_3484.aln");
    GTUtilsDocument::unloadDocument(os, "COI_3484.aln", true);

    GTGlobals::sleep();
    GTUtilsDocument::removeDocument(os, "COI_3484.nwk");
    GTUtilsDocument::loadDocument(os, "COI_3484.aln");

    CHECK_SET_ERR( GTUtilsProjectTreeView::checkItem(os, "COI_3484  .nwk") == false, "Unauthorized tree opening!");
}

GUI_TEST_CLASS_DEFINITION(test_3484_1) {
//    1. Open alignment
//    2. Build the tree
//    Current state: tree and alignment are both in the project, loaded and visualized.
//    3. Save the file and the project, relaunch UGENE (or close and reload project)
//    4. Open project, open alignment
//    Current state: tree and alignment are both in the project, loaded and visualized.
//    5. Delete the tree document from the project
//    6. Save the alignment
//    7. Save the project
//    8. Relaunch UGENE and open the project
//    9. Load the alignment
//    Current state: tree document is added to the project, both documents are loaded.
//    Expected state: only alignment is loaded.

    GTFile::copy(os, dataDir + "samples/CLUSTALW/COI.aln", testDir + "_common_data/scenarios/sandbox/COI_3484_1.aln");

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/sandbox/", "COI_3484_1.aln");
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os,
                                                               testDir + "_common_data/scenarios/sandbox/COI_3484_1.nwk",
                                                               0, 0, true));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep(500);
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView != NULL,"TreeView not found");

    GTUtilsDialog::waitForDialog(os, new SaveProjectAsDialogFiller(os, "proj_3484_1",
                                                                   testDir + "_common_data/scenarios/sandbox/",
                                                                   "proj_3484_1"));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__SAVE_AS_PROJECT);
    GTGlobals::sleep();

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/scenarios/sandbox/", "proj_3484_1.uprj"));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__OPEN_PROJECT);
    GTGlobals::sleep();

    GTUtilsDocument::loadDocument(os, "COI_3484_1.aln");
    QGraphicsView* treeView1 = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView1 != NULL,"TreeView not found");

    GTUtilsDocument::removeDocument(os, "COI_3484_1.nwk");
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__SAVE_PROJECT);

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/scenarios/sandbox/", "proj_3484_1.uprj"));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__OPEN_PROJECT);
    GTGlobals::sleep();

    GTUtilsDocument::loadDocument(os, "COI_3484_1.aln");
    CHECK_SET_ERR( GTUtilsProjectTreeView::checkItem(os, "COI_3484_1.nwk") == false, "Unauthorized tree opening!");
}

GUI_TEST_CLASS_DEFINITION(test_3518) {
//    1. Select {Tools -> Weight matrix -> Build Weight Matrix} menu item in the main menu.
//    2. Set file "data/samples/GFF/5prime_utr_intron_A20.gff" as input.
//    Expected state: the dialog process the file and notify user if the file is inappropriate, UGENE doesn't crash.
    QList<PwmBuildDialogFiller::Action> actions;
    actions << PwmBuildDialogFiller::Action(PwmBuildDialogFiller::ExpectInvalidFile, "");
    actions << PwmBuildDialogFiller::Action(PwmBuildDialogFiller::SelectInput, dataDir + "samples/GFF/5prime_utr_intron_A20.gff");
    actions << PwmBuildDialogFiller::Action(PwmBuildDialogFiller::ClickCancel, "");
    GTUtilsDialog::waitForDialog(os, new PwmBuildDialogFiller(os, actions));

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << "Weight matrix" << "Build Weight Matrix");
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_3545){
//    1. Open "_common_data\scenarios\msa\big.aln"
    GTFile::copy(os, testDir + "_common_data/scenarios/msa/big.aln", sandBoxDir + "big.aln");
    GTFileDialog::openFile(os, sandBoxDir , "big.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Use context menu
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/fasta", "NC_008253.fna" ));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "MSAE_MENU_LOAD_SEQ"
                                                      << "Sequence from file"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "msa_editor_sequence_area"));
    GTGlobals::sleep();
//    {Add->Sequence from file}

//    Expected state: "Open file with sequence" dialog appeared
//    3. Select sequence "_common_data\fasta\NC_008253.fna" and press "Open"
//    4. Close MSAEditor
    GTUtilsMdi::click(os, GTGlobals::Close);
//    5. Save document "big.aln"
    GTUtilsDialog::waitForDialog(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
    GTKeyboardDriver::keyClick(os, 's', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(5000);

    //close project
    if (AppContext::getProject() != NULL) {
        int key;
#ifdef Q_OS_MAC
        key = GTKeyboardDriver::key["cmd"];
#else
        key = GTKeyboardDriver::key["ctrl"];
#endif

        GTGlobals::sleep();
        GTWidget::click(os, GTUtilsProjectTreeView::getTreeView(os));
        GTKeyboardDriver::keyClick(os, 'a', key);
        GTGlobals::sleep(100);

        //GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new SaveProjectDialogFiller(os, QDialogButtonBox::No));
        //GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new AppCloseMessageBoxDialogFiller(os));
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
        GTGlobals::sleep(500);
#ifdef Q_OS_MAC
        QMenu *menu = GTMenu::showMainMenu(os, MWMENU_FILE);
        GTMenu::clickMenuItem(os, menu, ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
#else
        GTKeyboardDriver::keyClick(os, 'q', key);
        GTGlobals::sleep(100);
#endif
        GTGlobals::sleep(500);
    }
    GTGlobals::sleep(20000);
//    Current state: UGENE crashes
}

GUI_TEST_CLASS_DEFINITION(test_3551){
    //1. Open "samples/FASTA/human_T1.fa".
    //2. Context menu of the document "human_T1.fa": {Blast-> FormatDB...}.
    //3. Click the "Select input file(s)" radio button.
    //Expected: the "Select input file(s)" tool button is enabled.
    //4. Click the "Select input file(s)" radio button.
    //Expected: the "Select input file(s)" tool button is enabled.

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
    FormatDBSupportRunDialogFiller::Parameters p;
    p.customFiller_3551 = true;
    GTUtilsDialog::waitForDialog(os, new FormatDBSupportRunDialogFiller(os, p));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "BLAST" << "FormatDB"));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_3552){
//1. Open "_common_data\clustal\fungal - all.aln"
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "10000_sequences.aln");

    QLabel* taskInfoLabel = GTWidget::findExactWidget<QLabel*>(os, "taskInfoLabel");
    while (!taskInfoLabel->text().contains("Render")) {
        uiLog.trace("actual text: " + taskInfoLabel->text());
        GTGlobals::sleep(100);
    }
    GTGlobals::sleep(500);
    QProgressBar* taskProgressBar = GTWidget::findExactWidget<QProgressBar*>(os, "taskProgressBar");
    QString text = taskProgressBar->text();
    CHECK_SET_ERR(text.contains("%"), "unexpected text: " + text);
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    GTWidget::click(os, GTWidget::findWidget(os, "doc_lable_dock_task_view"));
//    GTGlobals::sleep(300);
//    GTWidget::click(os, GTWidget::findWidget(os, "doc_lable_dock_task_view"));
//Expected state: render view task started, progress is correct
}

GUI_TEST_CLASS_DEFINITION(test_3553) {
    //1. Open "_common_data/clustal/big.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/clustal", "big.aln");
    //2. Select both sequences.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(1, 0), QPoint(1, 1));
    //3. Open the "Pairwise Alignment" OP tab.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_PAIRALIGN"));
    //4. Press the "Align" button several times(~5).
    for (int i=0; i<5; i++) {
        GTWidget::click(os, GTWidget::findWidget(os, "alignButton"));
        GTGlobals::sleep(500);
    }
    //Expected: UGENE does not crash.
}

GUI_TEST_CLASS_DEFINITION(test_3555) {
//    1. Open "_common_data\muscul4\prefab_1_ref.aln"
//    2. Press "Switch on/off collapsing" tool button
//    3. Scroll down sequences
//    Expected state: scrolling will continue until the last sequence becomes visible
//    Current state: see the attachment
//    4. Click on the empty space below "1a0cA" sequence
//    Current state: SAFE_POINT in debug mode and incorrect selection in release(see the attachment)
    GTLogTracer l;

    GTFileDialog::openFile(os, testDir + "_common_data/muscul4/", "prefab_1_ref.aln");
    GTWidget::click(os, GTToolbar::getWidgetForActionName(os, GTToolbar::getToolbar(os, "mwtoolbar_activemdi"), "Enable collapsing"));

    GTUtilsMSAEditorSequenceArea::scrollToBottom(os);

    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    MSAEditor* editor = mw->findChild<MSAEditor*>();
    CHECK_SET_ERR(editor != NULL, "MsaEditor not found");

    MSAEditorNameList* nameList = editor->getUI()->getEditorNameList();
    CHECK_SET_ERR(nameList != NULL, "MSANameList is empty");
    GTWidget::click(os, nameList, Qt::LeftButton, QPoint(10, nameList->height() - 1));

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3556) {
    //1. Open "_common_data/muscul4/prefab_1_ref.aln".
    //2. Press the "Switch on/off collapsing" tool button.
    //3. Select the sequence "1a0cA".
    //4. Context menu: {Set with sequence as reference}.
    //Expected state: the sequence became reference.

    GTFileDialog::openFile(os, testDir + "_common_data/muscul4/", "prefab_1_ref.aln");
    GTWidget::click(os, GTAction::button(os, "Enable collapsing"));

    GTUtilsMSAEditorSequenceArea::scrollToBottom(os);
    GTUtilsMSAEditorSequenceArea::selectSequence(os, "1a0cA");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "set_seq_as_reference"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTWidget::click(os, GTWidget::findWidget(os, "OP_MSA_GENERAL"));

    QLineEdit *refEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "sequenceLineEdit"));
    CHECK_SET_ERR(refEdit->text() == "1a0cA", "Wrong reference sequence");
}

GUI_TEST_CLASS_DEFINITION(test_3557) {
    //1. Open "_common_data/muscul4/prefab_1_ref.aln".
    //2. Press the "Switch on/off collapsing" tool button.
    //3. Select the "2|1a0cA|gi|32470780" and "1a0cA" sequences.
    //4. Open the "Pairwise Alignment" OP tab.
    //Expected: "2|1a0cA|gi|32470780" and "1a0cA" are in the OP.

    GTFileDialog::openFile(os, testDir + "_common_data/muscul4/", "prefab_1_ref.aln");
    GTWidget::click(os, GTAction::button(os, "Enable collapsing"));
    GTUtilsMSAEditorSequenceArea::scrollToBottom(os);

    GTUtilsMSAEditorSequenceArea::selectSequence(os, "2|1a0cA|gi|32470780");
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);
    GTUtilsMSAEditorSequenceArea::selectSequence(os, "1a0cA");
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);

    GTWidget::click(os, GTWidget::findWidget(os, "OP_PAIRALIGN"));

    QLineEdit *firstEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "sequenceLineEdit", GTWidget::findWidget(os, "firstSeqSelectorWC")));
    QLineEdit *secondEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "sequenceLineEdit", GTWidget::findWidget(os, "secondSeqSelectorWC")));
    CHECK_SET_ERR(firstEdit->text() == "2|1a0cA|gi|32470780", "Wrong first sequence");
    CHECK_SET_ERR(secondEdit->text() == "1a0cA", "Wrong second sequence");
}

GUI_TEST_CLASS_DEFINITION(test_3563_1) {
//    1. Open an alignment
//    2. Build the tree
//    3. Unload both documents (alignment and tree)
//    4. Load alignment
//    Expected state: no errors in the log
    GTLogTracer l;

    GTFile::copy(os, testDir + "_common_data/clustal/dna.fasta.aln",
                 testDir + "_common_data/scenarios/sandbox/test_3563_1.aln");
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/sandbox/", "test_3563_1.aln");

    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/test_3563_1.nwk", 0, 0, true));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os, tree);
    GTGlobals::sleep();
    GTUtilsDocument::saveDocument(os, "test_3563_1.aln");

    GTUtilsDocument::unloadDocument(os, "test_3563_1.nwk", false);
    GTGlobals::sleep();

    GTUtilsDocument::unloadDocument(os, "test_3563_1.aln", true);
    GTGlobals::sleep();

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3563_2) {
//    1. Open "human_T1.fa"
//    2. Open "GFF/5prime_utr_intron_A21.gff"
//    3. Drag and drop "Ca21 chr5 features" to "human_T1"
//    4. Unload both documents
//    5. Load "human_T1.fa" document
//    6. Load "GFF/5prime_utr_intron_A21.gff" document
//    Expected state: no errors in the log
    GTLogTracer l;

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTFileDialog::openFile(os, dataDir + "samples/GFF/", "5prime_utr_intron_A21.gff");

    QModelIndex idxGff = GTUtilsProjectTreeView::findIndex(os, "Ca21chr5 features");
    CHECK_SET_ERR( idxGff.isValid(), "Can not find 'Ca21 chr5 features' object");
    QWidget* seqArea = GTWidget::findWidget(os, "render_area_human_T1 (UCSC April 2002 chr7:115977709-117855134)");
    CHECK_SET_ERR( seqArea != NULL, "No sequence view opened");

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "Yes"));
    GTUtilsDialog::waitForDialog(os, new CreateObjectRelationDialogFiller(os));
    GTUtilsProjectTreeView::dragAndDrop(os, idxGff, seqArea);

    GTUtilsDocument::unloadDocument(os, "human_T1.fa");
    GTUtilsDocument::unloadDocument(os, "5prime_utr_intron_A21.gff");
    GTGlobals::sleep();

    GTUtilsDocument::loadDocument(os, "human_T1.fa");
    CHECK_SET_ERR( GTUtilsDocument::isDocumentLoaded(os, "5prime_utr_intron_A21.gff"),
                   "Connection between documents was lost");

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3571_1) {
    // 1. Open file "test/_common_data/fasta/numbers_in_the_middle.fa" in sequence view
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    //GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "numbers_in_the_middle.fa");

    // 2. Select first sequence
    ADVSingleSequenceWidget *firstSeqWidget = GTUtilsSequenceView::getSeqWidgetByNumber(os, 0);
    GTWidget::click(os, firstSeqWidget);

    // 3. Open statistics option panel tab.
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Statistics);

    // Expected state : only length info appears
    QWidget *commonStatisticsWidget = GTWidget::findWidget(os, "ArrowHeader_Common Statistics");

    GTGlobals::FindOptions widgetFindSafeOptions(false);
    QWidget *charOccurWidget = GTWidget::findWidget(os, "ArrowHeader_Characters Occurrence", NULL, widgetFindSafeOptions);
    CHECK_SET_ERR(!charOccurWidget->isVisible(), "Character Occurrence section is unexpectedly visible");

    // 4. Select second sequence
    ADVSingleSequenceWidget *secondSeqWidget = GTUtilsSequenceView::getSeqWidgetByNumber(os, 1);
    GTWidget::click(os, secondSeqWidget);

    // Expected state : length and characters occurrence info appears
    commonStatisticsWidget = GTWidget::findWidget(os, "ArrowHeader_Common Statistics");
    charOccurWidget = GTWidget::findWidget(os, "ArrowHeader_Characters Occurrence");
}

GUI_TEST_CLASS_DEFINITION(test_3571_2) {
    // 1. Open file test/_common_data/fasta/numbers_in_the_middle.fa in sequence view
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    //GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "numbers_in_the_middle.fa");

    // 2. Select second sequence
    ADVSingleSequenceWidget *secondSeqWidget = GTUtilsSequenceView::getSeqWidgetByNumber(os, 1);
    GTWidget::click(os, secondSeqWidget);

    // 3. Open statistics option panel tab.
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Statistics);

    // Expected state : length and characters occurrence info appears
    QWidget *commonStatisticsWidget = GTWidget::findWidget(os, "ArrowHeader_Common Statistics");
    QWidget *charOccurWidget = GTWidget::findWidget(os, "ArrowHeader_Characters Occurrence");

    // 4. Select first sequence
    ADVSingleSequenceWidget *firstSeqWidget = GTUtilsSequenceView::getSeqWidgetByNumber(os, 0);
    GTWidget::click(os, firstSeqWidget);

    // Expected state : only length info appears
    commonStatisticsWidget = GTWidget::findWidget(os, "ArrowHeader_Common Statistics");
    GTGlobals::FindOptions widgetFindSafeOptions(false);
    charOccurWidget = GTWidget::findWidget(os, "ArrowHeader_Characters Occurrence", NULL, widgetFindSafeOptions);
    CHECK_SET_ERR(!charOccurWidget->isVisible(), "Character Occurrence section is unexpectedly visible");
}

GUI_TEST_CLASS_DEFINITION(test_3585) {
    GTLogTracer l;
    // 1. Open "File -> Access Remote Database..." from the main menu.
    // 2. Get something adequate from the Uniprot(DAS) database(e.g.use example ID : P05067).
    // Expected state : "DownloadRemoteDocuments" task has been started
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFillerDeprecated(os, "P05067", 7));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB);
    GTUtilsTaskTreeView::checkTask(os, "Load DAS Documents");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3603) {
//    1. Open "data/samples/FASTA/human_T1.fa".
//    2. Open "Find Pattern" options panel tab.
//    3. Set "Selected" region type.
//    4. Call context menu on the sequence view, and select "Select sequence regions...".
//    5. Accept the dialog with default values (a single region, from min to max).
//    Expected state: the region selector widget contains "Selected" region type, region is (1..199950).
//    Current state: the region selector widget contains "Selected" region type, region is (1..199951).
    GTLogTracer l;

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTWidget::click(os, GTWidget::findWidget(os, "OP_FIND_PATTERN"));
    QComboBox* regionComboBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "boxRegion"));
    CHECK_SET_ERR(regionComboBox != NULL, "Region comboBox is NULL");
    if(!regionComboBox->isVisible()){
        GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Search in"));
    }
    GTComboBox::setIndexWithText(os, regionComboBox, "Selected region");

    QWidget* renderArea = GTUtilsSequenceView::getSeqWidgetByNumber(os);
    CHECK_SET_ERR(renderArea != NULL, "Render area is NULL");
    GTWidget::click(os, renderArea);

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os));
    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);

    QLineEdit* start = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "editStart"));
    CHECK_SET_ERR(start != NULL, "Region start lineEdit is NULL");
    QLineEdit* end = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "editEnd"));
    CHECK_SET_ERR(end != NULL, "Region end lineEdit is NULL");
    CHECK_SET_ERR( start->text() == "1" && end->text() == "199950", "Selection is wrong!");

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3609_1) {
//    1. Open "data/samples/FASTA/human_T1.fa".
//    2. Call context menu on the sequence view, select {Edit sequence -> Remove subsequence...} menu item.
//    3. Remove region (5000..199950).
//    4. Enter position 50000 to the "goto" widget on the tool bar, click the "Go" button.
//    Expected state: you can't enter this position.
//    Current state: you can enter this position, an error message appears in the log after button click (safe point triggers in the debug mode).
    GTLogTracer l;

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    QWidget* seqWidget = GTUtilsSequenceView::getSeqWidgetByNumber(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REMOVE_SUBSEQUENCE));
    GTUtilsDialog::waitForDialog(os, new RemovePartFromSequenceDialogFiller(os, "5000..199950"));
    GTWidget::click(os, seqWidget, Qt::RightButton);

    QLineEdit* goToPosLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "go_to_pos_line_edit"));
    CHECK_SET_ERR(goToPosLineEdit != NULL, "GoTo lineEdit is NULL");
    bool inputResult = GTLineEdit::tryToSetText(os, goToPosLineEdit, "50000");
    CHECK_SET_ERR(inputResult == false, "Invalid goToPosition is accepted");

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3609_2) {
//    1. Open "data/samples/FASTA/human_T1.fa".
//    2. Call context menu on the sequence view, select {Edit sequence -> Insert subsequence...} menu item.
//    3. Insert any subsequence long enough.
//    4. Enter position 199960 to the "goto" widget on the tool bar, click the "Go" button.
//    Expected state: you can enter this position, view shows the position.
//    Current state: you can't enter this position.
    GTLogTracer l;

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    QWidget* seqWidget = GTUtilsSequenceView::getSeqWidgetByNumber(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_INSERT_SUBSEQUENCE));
    GTUtilsDialog::waitForDialog(os, new InsertSequenceFiller(os, "AAACCCTTTGGGAAA"));
    GTWidget::click(os, seqWidget, Qt::RightButton);

    QLineEdit* goToPosLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "go_to_pos_line_edit"));
    CHECK_SET_ERR(goToPosLineEdit != NULL, "GoTo lineEdit is NULL");

    GTLineEdit::setText(os, goToPosLineEdit, "199960");

    QWidget* goBtn = GTWidget::findWidget(os, "Go!");
    CHECK_SET_ERR(goBtn != NULL, "Go! button is NULL");
    GTWidget::click(os, goBtn);

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3609_3) {
//    1. Open "data/samples/FASTA/human_T1.fa".
//    2. Call context menu on the sequence view, select {Edit sequence -> Insert subsequence...} menu item.
//    3. Insert any subsequence long enough.
//    4. Call context menu, select {Go to position...} menu item.
//    5. Enter position 199960 and accept the dialog.
//    Expected state: view shows the position, there are no errors in the log.
//    Current state: view shows the position, there is an error in the log (safe point triggers in the debug mode).
    GTLogTracer l;

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    QWidget* seqWidget = GTUtilsSequenceView::getSeqWidgetByNumber(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_INSERT_SUBSEQUENCE));
    GTUtilsDialog::waitForDialog(os, new InsertSequenceFiller(os, "AAACCCTTTGGGAAA"));
    GTWidget::click(os, seqWidget, Qt::RightButton);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_GOTO_ACTION));
    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, 199960));
    GTWidget::click(os, seqWidget, Qt::RightButton);

    GTUtilsLog::check(os, l);
}
GUI_TEST_CLASS_DEFINITION(test_3610) {
    // Open "data/samples/FASTA/human_T1.fa".
    // Select whole sequence.
    // Call context menu, select {Edit sequence -> Replace subsequence...}menu item.
    // Replace whole sequence with any inappropriate symbol, e.g. '='. Accept the dialog, agree with message box.
    // Expected state: UGENE doesn't crash.
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 1, 197350));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Select" << "Sequence region"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(1000);


    //GTMenu::showContextMenu(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"));

    Runnable *filler = new ReplaceSubsequenceDialogFiller(os,
        "atcgtac"
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EDIT << ACTION_EDIT_REPLACE_SUBSEQUENCE));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(1000);

//    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_ACTIONS), QStringList() <<  ADV_MENU_EDIT << ACTION_EDIT_INSERT_SUBSEQUENCE, GTGlobals::UseKey);
  //GTGlobals::sleep();
//    GTUtilsDialog::waitForDialog(os, new ReplaceSubsequenceDialogFiller(os));
//    GTWidget::click(os, seqWidget, Qt::RightButton);


}

GUI_TEST_CLASS_DEFINITION(test_3612) {
//    1. Open "_common_data/scenarios/msa/ma.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa", "ma.aln");

//    2. Turn on the collapsing mode.
    GTUtilsMsaEditor::toggleCollapsingMode(os);

//    3. Expand "Conocephalus_discolor" group.
    GTUtilsMsaEditor::toggleCollapsingGroup(os, "Conocephalus_discolor");

//    4. Open "Pairwise alignment" options panel tab.
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::PairwiseAlignment);

//    5. Set parameters:
//        First sequence: Tettigonia_viridissima
//        Second sequence: Conocephalus_discolor
//        Algorithm: Smith-Waterman
//        Gap open penalty: 1
//        In new window: unchecked
//    and start the align task.
//    Expected state: these two sequences are aligned, the same changes are applied to whole collapsing group.
    GTUtilsOptionPanelMsa::addFirstSeqToPA(os, "Tettigonia_viridissima");
    GTUtilsOptionPanelMsa::addSecondSeqToPA(os, "Conocephalus_discolor");

    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Algorithm settings"));
    GTWidget::click(os, GTWidget::findWidget(os, "ArrowHeader_Output settings"));
    GTComboBox::setIndexWithText(os, GTWidget::findExactWidget<QComboBox*>(os, "algorithmListComboBox"), "Smith-Waterman");
    GTSpinBox::setValue(os, GTWidget::findExactWidget<QSpinBox*>(os, "gapOpen"), 1);
    GTCheckBox::setChecked(os, GTWidget::findExactWidget<QCheckBox*>(os, "inNewWindowCheckBox"), false);
    GTWidget::click(os, GTWidget::findWidget(os, "alignButton"));

    GTUtilsTaskTreeView::waitTaskFinished(os);

    const QString firstSequence = GTUtilsMSAEditorSequenceArea::getSequenceData(os, "Conocephalus_discolor");
    const QString secondSequence = GTUtilsMSAEditorSequenceArea::getSequenceData(os, "Conocephalus_sp.");
    CHECK_SET_ERR(firstSequence == secondSequence, QString("Sequences are unexpectedly differs: '%1' and '%2'").arg(firstSequence).arg(secondSequence));
}

GUI_TEST_CLASS_DEFINITION(test_3613) {
//    1. Open any assembly.
//    2. Call a context menu on any read, select {Export -> Current Read} menu item.
//    3, Set any appropriate destination and apply the dialog.
//    Expected state: a new document is added to the project, a view opens (MSA Editor or Sequence view, it should be clarified, see the documentation).
//    Current state: a new unloaded document is added to the project. If you force it to open it will load but "open view" task will fail with an error: "Multiple alignment object not found".
    GTLogTracer l;

    GTUtilsDialog::waitForDialog(os,
                                 new ImportBAMFileFiller(os, sandBoxDir + "test_3613.bam.ugenedb"));
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/assembly/", "example-alignment.bam");
    GTUtilsAssemblyBrowser::zoomToMax(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Export" << "Current Read"));
    GTUtilsDialog::waitForDialog(os, new ExportReadsDialogFiller(os, sandBoxDir + "test_3613.fa"));
    QWidget* readsArea = GTWidget::findWidget(os, "assembly_reads_area");
    CHECK_SET_ERR(readsArea != NULL, "Assembly reads area not found");
    GTWidget::click(os, readsArea, Qt::RightButton);

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3622) {
//    1. Open "data/samples/FASTA/human_T1.fa".

//    2. Open "Find pattern" options panel tab.

//    3. Enter any valid pattern.
//    Expected state: a new search task is launched.

//    4. Set "InsDel" algorithm.
//    Expected state: a new search task is launched.

//    5. Set any another match value.
//    Expected state: a new search task is launched.

//    6. Set "Substitute" algorithm.
//    Expected state: a new search task is launched.

//    7. Set any another match value.
//    Expected state: a new search task is launched.

//    8. Turn on "Search with ambiguous bases" option.
//    Expected state: a new search task is launched.

//    9. Select any sequence region.

//    10. Set "Selected region" region type.
//    Expected state: a new search task is launched, the region type is set to "Custom region", the region is the same as the selected one.

//    11. Change the region.
//    Expected state: a new search task is launched.

    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Search);
    SchedulerListener listener;

    GTUtilsOptionPanelSequenceView::enterPattern(os, "ACGT");
    CHECK_SET_ERR(0 < listener.getRegisteredTaskCount(), "The search task wasn't registered");
    listener.reset();
    GTGlobals::sleep(200);

    GTUtilsOptionPanelSequenceView::setAlgorithm(os, "InsDel");
    CHECK_SET_ERR(0 < listener.getRegisteredTaskCount(), "The search task wasn't registered");
    listener.reset();
    GTGlobals::sleep(200);

    GTUtilsOptionPanelSequenceView::setMatchPercentage(os, 80);
    CHECK_SET_ERR(0 < listener.getRegisteredTaskCount(), "The search task wasn't registered");
    listener.reset();
    GTGlobals::sleep(200);

    GTUtilsOptionPanelSequenceView::setAlgorithm(os, "Substitute");
    CHECK_SET_ERR(0 < listener.getRegisteredTaskCount(), "The search task wasn't registered");
    listener.reset();
    GTGlobals::sleep(200);

    GTUtilsOptionPanelSequenceView::setMatchPercentage(os, 90);
    CHECK_SET_ERR(0 < listener.getRegisteredTaskCount(), "The search task wasn't registered");
    listener.reset();
    GTGlobals::sleep(200);

    GTUtilsOptionPanelSequenceView::setSearchWithAmbiguousBases(os);
    CHECK_SET_ERR(0 < listener.getRegisteredTaskCount(), "The search task wasn't registered");
    listener.reset();
    GTGlobals::sleep(200);

    GTUtilsSequenceView::selectSequenceRegion(os, 100, 200);
    GTUtilsOptionPanelSequenceView::setRegionType(os, "Selected region");
    CHECK_SET_ERR(0 < listener.getRegisteredTaskCount(), "The search task wasn't registered");
    listener.reset();
    GTGlobals::sleep(200);

    const QString currentRegionType = GTUtilsOptionPanelSequenceView::getRegionType(os);
    CHECK_SET_ERR("Custom region" == currentRegionType, QString("An unexpected region type: expect '%1', got '%2'").arg("Custom region").arg(currentRegionType));

    const QPair<int, int> currentRegion = GTUtilsOptionPanelSequenceView::getRegion(os);
    CHECK_SET_ERR(qMakePair(100, 200) == currentRegion, QString("An unexpected region: expect [%1, %2], got [%3, %4]").arg(100).arg(200).arg(currentRegion.first).arg(currentRegion.second));

    GTUtilsOptionPanelSequenceView::setRegion(os, 500, 1000);
    CHECK_SET_ERR(0 < listener.getRegisteredTaskCount(), "The search task wasn't registered");
}

GUI_TEST_CLASS_DEFINITION(test_3623) {
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Search);
    GTUtilsOptionPanelSequenceView::enterPattern(os, "AAAAAAAAAAAAAAAAAAAAAAAAAAA");
    GTUtilsOptionPanelSequenceView::toggleSaveAnnotationsTo(os);
    GTUtilsOptionPanelSequenceView::enterFilepathForSavingAnnotations(os, sandBoxDir + "op_seqview_test_0001.gb");
    GTUtilsOptionPanelSequenceView::clickGetAnnotation(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();
    CHECK_SET_ERR(GTUtilsOptionPanelSequenceView::isPrevNextEnabled(os), "Next and prev buttons are disabled");
}

GUI_TEST_CLASS_DEFINITION(test_3629) {
//    1. Open "data/samples/FASTA/human_T1.fa".
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

//    2. Select {Add->New annotation...} menu item in the context menu.
//    Expected state: "Create Annotation" dialog appeared, "create new table" option is selected.

//    3. Set any location and press "Create".
//    Expected state: new annotation object was created.
    QDir().mkpath(sandBoxDir + "test_3629");
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "misc_feature", "1..5", sandBoxDir + "test_3629/test_3629.gb"));
    GTWidget::click(os, GTToolbar::getWidgetForActionTooltip(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "New annotation"));
    GTUtilsProjectTreeView::checkItem(os, "test_3629.gb");

//    4. Open "data/samples/Genbank/sars.gb".
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "sars.gb");

//    5. Add the annotation object to "sars" sequence.
    GTUtilsDialog::waitForDialog(os, new CreateObjectRelationDialogFiller(os));
    GTUtilsProjectTreeView::dragAndDrop(os, GTUtilsProjectTreeView::findIndex(os, "Annotations"), GTUtilsAnnotationsTreeView::getTreeWidget(os));

//    6. Switch view to "human_T1".
//    Expected state: there are no attached annotations.
    GTUtilsProjectTreeView::doubleClickItem(os, "human_T1.fa");
    U2OpStatusImpl opStatus;
    GTUtilsAnnotationsTreeView::findItems(opStatus, "misc_feature");
    CHECK_SET_ERR(opStatus.isCoR(), "The annotaion table is unexpectedly attached");
}

GUI_TEST_CLASS_DEFINITION(test_3645) {
    //checking results with diffirent algorithms
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Search);

    GTUtilsOptionPanelSequenceView::toggleInputFromFilePattern(os);
    GTUtilsOptionPanelSequenceView::enterPatternFromFile(os, testDir + "_common_data/FindAlgorithm/", "find_pattern_op_2.fa");

    GTUtilsOptionPanelSequenceView::clickNext(os);
    GTUtilsOptionPanelSequenceView::clickNext(os);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ADV_MENU_COPY<< "Copy sequence",GTGlobals::UseMouse));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));
    QString clipStr = GTClipboard::text(os);
    CHECK_SET_ERR(clipStr == "GGGGG", "Found sequence don't match");
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_3634) {
    //1. File -> Open as -> "_common_data/gtf/invalid/AB375112_annotations.gtf".
    //Expected: the "Select correct document format" dialog appears.
    //2. Choose "GTF" in the combobox.
    //3. Click OK.
    // Expected: the file is not opened.
    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/gtf/invalid", "AB375112_annotations.gtf"));
    GTUtilsDialog::waitForDialog(os, new DocumentFormatSelectorDialogFiller(os, "GTF"));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__OPEN_AS);
    GTGlobals::sleep();
    CHECK_SET_ERR(l.hasError(), "No error about opening the file");
}

GUI_TEST_CLASS_DEFINITION(test_3639) {
    GTLogTracer logTracer;

    //2. Open "human_T1.fa".
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    //1. Connect to any shared database with write permissions.
    GTUtilsSharedDatabaseDocument::connectToTestDatabase(os);

    //3. Select "human_T1.fa" document and a 'Recycle bin" folder in the project view.
    GTGlobals::FindOptions options;
    options.depth = 1;
    QModelIndex humanT1Doc = GTUtilsProjectTreeView::findIndex(os, "human_T1.fa", options);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, humanT1Doc));
    GTMouseDriver::click(os);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Recycle bin"));
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["ctrl"]);
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["ctrl"]);

    //4. Remove items with "del" key.
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    //Expected state: the document is removed, the folder is not removed, no message boxes appear.
    CHECK_SET_ERR(!logTracer.hasError(), "Error message");
    GTUtilsProjectTreeView::getItemCenter(os, "Recycle bin");
    options.failIfNull = false;
    humanT1Doc = GTUtilsProjectTreeView::findIndex(os, "human_T1.fa", options);
    CHECK_SET_ERR(!humanT1Doc.isValid(), "The document is not removed");
}

GUI_TEST_CLASS_DEFINITION(test_3640) {
    GTLogTracer logTracer;

    //2. Open any document.
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    //1. Connect to a read-only shared database (e.g. to the UGENE public database).
    GTUtilsSharedDatabaseDocument::connectToUgenePublicDatabase(os);

    //3. Select the document and any folder in the database in the project view.
    GTGlobals::FindOptions options;
    options.depth = 1;
    QModelIndex humanT1Doc = GTUtilsProjectTreeView::findIndex(os, "human_T1.fa", options);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, humanT1Doc));
    GTMouseDriver::click(os);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "genomes"));
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["ctrl"]);
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["ctrl"]);

    //4. Remove selected items via "del" key.
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    //Expected state: the document is removed, the folder is not removed.
    CHECK_SET_ERR(!logTracer.hasError(), "Error message");
    GTUtilsProjectTreeView::findIndex(os, "genomes");
    options.failIfNull = false;
    humanT1Doc = GTUtilsProjectTreeView::findIndex(os, "human_T1.fa", options);
    CHECK_SET_ERR(!humanT1Doc.isValid(), "The document is not removed");
}

GUI_TEST_CLASS_DEFINITION(test_3649) {
    //1. Open "_common_data/smith-waterman2/simple/05/search.txt".
    GTFileDialog::openFile(os, testDir + "_common_data/smith_waterman2/simple/05", "search.txt");

    //2. Export the sequence object as alignment.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT));
    GTUtilsDialog::waitForDialog(os, new ExportSequenceAsAlignmentFiller(os, testDir + "_common_data/scenarios/sandbox", "test_3649.aln", ExportSequenceAsAlignmentFiller::Clustalw, true));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "S"));
    GTMouseDriver::click(os, Qt::RightButton);

    //3. Add a sequence from the file "_common_data/smith-waterman2/simple/05/query.txt" in the alignment.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_LOAD << "Sequence from file"));
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/smith_waterman2/simple/05", "query.txt"));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //4. Select both sequences.
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(0, 0), QPoint(1, 1));

    //5. Open pairwise alignment option panel tab.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_PAIRALIGN"));

    //6. Align using the Smith-Waterman algorithm.
    GTUtilsOptionPanelMsa::setPairwiseAlignmentAlgorithm(os, "Smith-Waterman");
    GTWidget::click(os, GTUtilsOptionPanelMsa::getAlignButton(os));

    //Expected: UGENE does not crash.
}

GUI_TEST_CLASS_DEFINITION(test_3656) {
//    1. Connect to the public database
//    2. Go to /genomes/Human (hg 19)
//    3. Right click on "chr2", then choose { Export/Import -> Export Sequences... }
//    Expected state: the "Export Sequences" dialog has appeared.
//    4. Press "OK"
//    Expected state: Export task has launched and successfully finished.
//    Current state: UGENE hangs for a half of minute, then Export task is launched.

    GTLogTracer lt;
    GTUtilsSharedDatabaseDocument::connectToUgenePublicDatabase(os);
    CHECK_OP(os, );

    QTreeView *treeView = GTUtilsProjectTreeView::getTreeView(os);
    CHECK_SET_ERR(NULL != treeView, "Invalid project tree view");

    QModelIndex prnt = GTUtilsProjectTreeView::findIndex(os, "Human (hg19)");
    QModelIndex idx = GTUtilsProjectTreeView::findIndex(os, "chr2", prnt);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, idx));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE));
    GTUtilsDialog::waitForDialog(os, new ExportSelectedRegionFiller(os, testDir + "_common_data/scenarios/sandbox/", "test_3656.fa", GTGlobals::UseMouse));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    CHECK_SET_ERR(GTUtilsTaskTreeView::checkTask(os, "Export sequence to document"), "Task is not running!");
    GTGlobals::sleep();

    GTUtilsTaskTreeView::cancelTask(os, "Export sequence to document");
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_3658){
//    1. Open the WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
//    2. Add "Call Varinats with Samtools" workflow element
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Write Annotations");
//    3. Press on the toolbar
//    {Scripting mode->Show scripting options}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Show scripting options"));
    GTWidget::click(os, GTAction::button(os, GTAction::findActionByText(os, "Scripting mode")));
//    4. Click on the workflow element
    GTUtilsWorkflowDesigner::click(os, "Write Annotations");
//    Expected state: property editor appeared
//    5. Press on the toolbar
//    {Scripting mode->Hide scripting options}
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Hide scripting options"));
    GTWidget::click(os, GTAction::button(os, GTAction::findActionByText(os, "Scripting mode")));
//    Expected state: scripting column is hidden
    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"table"));
    int count = table->model()->columnCount();
    CHECK_SET_ERR(count == 2, QString("wrong columns number. expected 2, actual: %1").arg(count));
}
GUI_TEST_CLASS_DEFINITION(test_3675){
/*  1. Open file COI.aln
    2. Press "build tree" button on main toolbar
    3. In build tree dialog set path like this: some_existing_folder/some_not_existing_folder/COI.nwk
    4. Press build.
    Expected state: new folder created, COI.nwk writter there
    Actual state: error messagebox appeares: "you don't have permission to write to this folder"
*/
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, sandBoxDir + "some_not_existing_folder/COI.nwk", 0, 0, true));
    GTWidget::click(os, GTAction::button(os, GTAction::findAction(os, "Build Tree")));
    GTGlobals::sleep();
    CHECK_SET_ERR(QFile::exists(sandBoxDir + "some_not_existing_folder/COI.nwk"), "File sandBoxDir/some_not_existing_folder/COI.nwk does not exist");
}
GUI_TEST_CLASS_DEFINITION(test_3690){
//    1. Open human_T1.fa
//    Expected state: there are two opened windows - start page and human_T1
//    2. Use short cut - Ctrl+Tab
//    Expected state: current active MDI window is changed to start page
//    3. Use short cut - Ctrl+Shift+Tab
//    Expected state: current active MDI window is changed back to human_T1

    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
    QWidget* wgt = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(wgt != NULL, "ActiveWindow is NULL");
    CHECK_SET_ERR(wgt->windowTitle() == "human_T1 [s] human_T1 (UCSC April 2002 chr7:115977709-117855134)", "human_T1.fa should be opened!");

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["tab"], GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    wgt = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(wgt != NULL, "ActiveWindow is NULL");
    CHECK_SET_ERR(wgt->windowTitle() == "Start Page", "Start Page should be opened!");

    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["shift"]);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["tab"], GTKeyboardDriver::key["ctrl"]);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["shift"]);
    GTGlobals::sleep();

    wgt = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(wgt != NULL, "ActiveWindow is NULL");
    CHECK_SET_ERR(wgt->windowTitle() == "human_T1 [s] human_T1 (UCSC April 2002 chr7:115977709-117855134)", "human_T1.fa should be opened!");
}

GUI_TEST_CLASS_DEFINITION(test_3697){
//    1. Create a connection to some shared database
//    2. Create the second connection with the same parameters except connection name
//    Current state: the second connection is not created, after acceptance of the 'Connection settings' dialog nothing had happened - no connection, no message.
//    Extected state: message is shown

//    Additional scenario:
//    1. Open WD
//    2. Add Read Sequence element and
//    3. Deselect Read Sequence element (click on empty space) and select it again
//    Current state: Connection Duplicate Detected dialog appeared.
//    Expected state: no message

    GTLogTracer l;

    QString conName1 = "test_3697: ugene_gui_test I";
    {
        QList<SharedConnectionsDialogFiller::Action> actions;
        actions << SharedConnectionsDialogFiller::Action(SharedConnectionsDialogFiller::Action::ADD);

        GTUtilsDialog::waitForDialog(os, new SharedConnectionsDialogFiller(os, actions));
    }

    EditConnectionDialogFiller::Parameters params1;
    params1.connectionName = conName1;
    params1.host = GTDatabaseConfig::host();
    params1.port = QString::number(GTDatabaseConfig::port());
    params1.database = GTDatabaseConfig::database();
    params1.login = "login";
    params1.password = "password";
    GTUtilsDialog::waitForDialog(os, new EditConnectionDialogFiller(os, params1, EditConnectionDialogFiller::MANUAL));

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__ACCESS_SHARED_DB);

    QString conName2 = "test_3697: ugene_gui_test II";
    {
        QList<SharedConnectionsDialogFiller::Action> actions;
        actions << SharedConnectionsDialogFiller::Action(SharedConnectionsDialogFiller::Action::ADD);

        GTUtilsDialog::waitForDialog(os, new SharedConnectionsDialogFiller(os, actions));
    }
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "Ok"));
    EditConnectionDialogFiller::Parameters params2 = params1;
    params2.connectionName = conName2;
    GTUtilsDialog::waitForDialog(os, new EditConnectionDialogFiller(os, params2, EditConnectionDialogFiller::MANUAL));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__ACCESS_SHARED_DB);

    GTUtilsDialog::waitForDialogWhichMustNotBeRunned(os, new MessageBoxDialogFiller(os, "Ok"));
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::addElement(os, "Read alignment");
    GTUtilsWorkflowDesigner::addElement(os, "Read sequence");
    GTUtilsWorkflowDesigner::click(os, "Read alignment");
    GTUtilsWorkflowDesigner::click(os, "Read sequence");

    GTUtilsLog::check(os, l);
}

GUI_TEST_CLASS_DEFINITION(test_3702){
    // 1. Open human_T1.fa
    // 2. Drag'n' drop it from the project to welcome screen
    // Expected state: sequence view is opened

    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
    GTUtilsMdi::closeWindow(os, "human_T1 [s] human_T1 (UCSC April 2002 chr7:115977709-117855134)");

    QWidget* welcomePage = GTUtilsMdi::findWindow(os, "Start Page");
    CHECK_SET_ERR(welcomePage != NULL, "WelcomePage widget not found");
    GTWidget::click(os, welcomePage);

    QPoint p  = AppContext::getMainWindow()->getQMainWindow()->geometry().center();

    QModelIndex idx = GTUtilsProjectTreeView::findIndex(os, "human_T1.fa");
    CHECK_SET_ERR(idx.isValid(), "Index is invalid!");
    GTMouseDriver::dragAndDrop(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"), p);

    QWidget* wgt = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(wgt != NULL, "ActiveWindow is NULL");
    CHECK_SET_ERR(wgt->windowTitle() == "human_T1 [s] human_T1 (UCSC April 2002 chr7:115977709-117855134)", "human_T1.fa should be opened!");
}

GUI_TEST_CLASS_DEFINITION(test_3715) {
    //1. Open WD.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2. Activate samples.
    GTUtilsWorkflowDesigner::setCurrentTab(os, GTUtilsWorkflowDesigner::samples);

    //3. Choose a sample
    GTUtilsWorkflowDesigner::addSample(os, "call variants");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTKeyboardDriver::keyClick(os, 'r', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    CHECK_SET_ERR(GTUtilsWorkflowDesigner::checkErrorList(os, "Read Assembly") != 0, "Workflow errors list cant be empty");
}

GUI_TEST_CLASS_DEFINITION(test_3723) {
//    1. Open simultaneously two files: "_common_data/fasta/fa1.fa.gz" and "_common_data/fasta/fa3.fa.gz".
//    Expected state: "Multiple Sequence Reading Mode" dialog appears.
//    2. Open them with "Merge" option.
//    Expected state: sequences are merged and opened, there are no errors in the log.
    GTLogTracer logTracer;

    GTSequenceReadingModeDialog::mode = GTSequenceReadingModeDialog::Merge;
    GTUtilsDialog::waitForDialog(os, new GTSequenceReadingModeDialogUtils(os));
    GTFileDialog::openFileList(os, testDir + "_common_data/fasta", QStringList() << "fa1.fa.gz" << "fa3.fa.gz");

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsDocument::checkDocument(os, "merged_document.gb");
    GTUtilsDocument::isDocumentLoaded(os, "merged_document.gb");
    GTUtilsLog::check(os, logTracer);
}

GUI_TEST_CLASS_DEFINITION(test_3724) {
//    1. Open "data/samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

//    2. Use sequence area context menu:
//    { Statistics -> Generate distance matrix }
//    Expected state: the "Generate Distance Matrix" dialog has appeared.

//    3. Click "Generate".
//    Expected state: the "Multiple Sequence Alignment Distance Matrix" view has appeared.
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Statistics" << "Generate distance matrix"));
    GTUtilsDialog::waitForDialog(os, new DistanceMatrixDialogFiller(os));
    GTUtilsMSAEditorSequenceArea::callContextMenu(os);

//    4. Click right mouse button on the view.
//    Expected state: nothing happens.
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsDialog::waitForDialogWhichMustNotBeRunned(os, new PopupChecker(os, QStringList()));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_3730) {
//  Note: the bug not always reproduced.

//  1. Open "data/samples/CLUSTALW/COI.aln".
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");

//  2. Create new custom nucleotide color scheme.
    GTUtilsMSAEditorSequenceArea::createColorScheme(os, "test_3730_scheme_1", NewColorSchemeCreator::nucl);

//  3. Go to Highlighting Options Panel tab and select this new color scheme.
    GTUtilsOptionPanelMsa::setColorScheme(os, "test_3730_scheme_1");

//  4. Go to Preferences again and create new amino color scheme.
//  5. Accept Application Settings dialog.
//  Expected state: UGENE doesn't crash, color scheme is not changed.
    GTUtilsMSAEditorSequenceArea::createColorScheme(os, "test_3730_scheme_2", NewColorSchemeCreator::amino);
    const QString colorScheme = GTUtilsOptionPanelMsa::getColorScheme(os);
    CHECK_SET_ERR(colorScheme == "test_3730_scheme_1", "The color scheme was unexpectedly changed");
}

GUI_TEST_CLASS_DEFINITION(test_3732) {
//    1. Open UGENE preferences, open "Resources" tab, set UGENE memory limit to 200Mb.
    class MemoryLimitSetScenario : public CustomScenario {
        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "Active modal widget is NULL");

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::Resourses);
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit *>(os, "memBox", dialog), "200");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };
    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new MemoryLimitSetScenario));

//    2. Open file "_common_data/scenarios/_regression/1688/sr100.000.fa" as separate sequences.
//    Expected state: there is an error in the log: "MemoryLocker - Not enough memory error, 41 megabytes are required".
    GTLogTracer logTracer;

    GTUtilsProject::openMultiSequenceFileAsMalignment(os, testDir + "_common_data/scenarios/_regression/1688", "sr100.000.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR(logTracer.checkMessage("MemoryLocker - Not enough memory error, 41 megabytes are required"), "An expected error message not found");
}

GUI_TEST_CLASS_DEFINITION(test_3816) {
    // Open some specific file with a tree and ensure that UGENE doesn't crash
    GTLogTracer l;
    GTFileDialog::openFile(os, testDir + "_common_data/newick/", "arb-silva.nwk");
    GTUtilsProjectTreeView::findIndex(os, "arb-silva.nwk", QModelIndex());
    GTUtilsLog::check(os, l);
}

} // GUITest_regression_scenarios namespace

} // U2 namespace

