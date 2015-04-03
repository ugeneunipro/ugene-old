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

#include "GTTestsRegressionScenarios_1_1000.h"

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
#include "api/GTListWidget.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTSequenceReadingModeDialog.h"
#include "api/GTSequenceReadingModeDialogUtils.h"
#include "api/GTSlider.h"
#include "api/GTSpinBox.h"
#include "api/GTTabWidget.h"
#include "api/GTTableView.h"
#include "api/GTToolbar.h"
#include "api/GTTreeWidget.h"
#include "api/GTWidget.h"

#include "GTDatabaseConfig.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsAnnotationsHighlightingTreeView.h"
#include "GTUtilsAssemblyBrowser.h"
#include "GTUtilsBookmarksTreeView.h"
#include "GTUtilsCircularView.h"
#include "GTUtilsDashboard.h"
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
#include "GTUtilsTask.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsWizard.h"
#include "GTUtilsWorkflowDesigner.h"

#include "runnables/qt/EscapeClicker.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/AlignShortReadsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/AppSettingsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/BuildIndexDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ConvertAssemblyToSAMDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/ugene/corelibs/U2Gui/CreateDocumentFromTextDialogFiller.h"
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
#include "runnables/ugene/corelibs/U2Gui/util/RenameSequenceFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_assembly/ExportConsensusDialogFiller.h"
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
#include "runnables/ugene/plugins/cap3/CAP3SupportDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportAnnotationsDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportMSA2MSADialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportMSA2SequencesDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSelectedSequenceFromAlignmentDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSequences2MSADialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSequencesDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ImportAnnotationsToCsvFiller.h"
#include "runnables/ugene/plugins/dotplot/BuildDotPlotDialogFiller.h"
#include "runnables/ugene/plugins/dotplot/DotPlotDialogFiller.h"
#include "runnables/ugene/plugins/enzymes/ConstructMoleculeDialogFiller.h"
#include "runnables/ugene/plugins/enzymes/DigestSequenceDialogFiller.h"
#include "runnables/ugene/plugins/enzymes/FindEnzymesDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/BlastAllSupportDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/ClustalOSupportRunDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/FormatDBDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/RemoteBLASTDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/SpadesGenomeAssemblyDialogFiller.h"
#include "runnables/ugene/plugins/external_tools/TCoffeeDailogFiller.h"
#include "runnables/ugene/plugins/weight_matrix/PwmBuildDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/AliasesDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/ConfigurationWizardFiller.h"
#include "runnables/ugene/plugins/workflow_designer/CreateElementWithCommandLineToolFiller.h"
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
#include "runnables/ugene/plugins_3rdparty/primer3/Primer3DialogFiller.h"
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
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/ProjectViewModel.h>
#include <U2Gui/ToolsMenu.h>

#include "../../workflow_designer/src/WorkflowViewItems.h"

#include <U2View/ADVConstants.h>
#include <U2View/ADVSingleSequenceWidget.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/AnnotationsTreeView.h>
#include <U2View/AssemblyNavigationWidget.h>
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorNameList.h>

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QTableWidget>
#include <QWebElement>
#include <QWebFrame>
#include <QWebView>
#include <QWizard>

namespace U2 {

namespace GUITest_regression_scenarios {

GUI_TEST_CLASS_DEFINITION(test_0407) {
    // 1. Open _common_data/scenarios/_regression/407/trail.fas
    // Expected state: a message box appears
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/_regression/407/trail.fas");
}
GUI_TEST_CLASS_DEFINITION(test_0490) {
    // 1. Select "Tools > Multiple alignment > [any item]"
    // 2. In the dialog appeared select _common_data/scenarios/_regression/490/fasta-example.fa
    // 3. Click "Align"
    // Expected stat:  UGENE not crashes

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/_regression/490/fasta-example.fa");
    GTUtilsDialog::waitForDialog(os, new KalignDialogFiller(os));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "align_with_kalign"));
    GTWidget::click(os, GTUtilsMdi::activeWindow(os), Qt::RightButton);
    GTGlobals::sleep();
    GTGlobals::sleep();
}
GUI_TEST_CLASS_DEFINITION(test_0677) {
    // 1. Open the file bamExample.bam.
    // 2. Check the box 'Import unmapped reads' and import the file.
    // Expected state: UGENE not crashed

    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, sandBoxDir + "test_0677/test_0677.ugenedb", "", "", true));
    GTFileDialog::openFile(os, testDir + "_common_data/bam/", "1.bam");
    GTUtilsLog::check(os, l);
}
GUI_TEST_CLASS_DEFINITION(test_0678) {
    // 1. Open samples/PDB/1CF7.pdb
    // 2. Navigate in annotation tree, unfolding following items: {1CF7 chain 1 annotation <97> chain_info (0, 1) <97> chain_info}
    // Expected state: UGENE not crashes

    GTFileDialog::openFile(os, dataDir + "samples/PDB/1CF7.PDB");
    GTUtilsAnnotationsTreeView::selectItems(os, QStringList() << "chain_info");

}
GUI_TEST_CLASS_DEFINITION(test_0685) {
    // 1. Do menu tools->Blast+ Search (ext. tools must be configured)
    // 2. Set next parameters:
    // {Select input file} _common_data\scenarios\external_tools\blast\SequenceLength_00003000.txt
    // {Select search} blastp
    // Expected state: UGENE not crashes
    BlastAllSupportDialogFiller::Parameters blastParams;
    blastParams.runBlast = true;
    blastParams.programNameText = "blastp";
    blastParams.withInputFile = true;
    blastParams.inputPath = testDir + "_common_data/scenarios/external_tools/blast/SequenceLength_00003000.txt";
    blastParams.dbPath = testDir + "_common_data/cmdline/external-tool-support/blastplus/human_T1/human_T1.nhr";
    GTUtilsDialog::waitForDialog(os, new BlastAllSupportDialogFiller(blastParams, os));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << ToolsMenu::BLAST_MENU << ToolsMenu::BLAST_SEARCHP);
    GTUtilsTaskTreeView::waitTaskFinished(os);

}

GUI_TEST_CLASS_DEFINITION(test_0700) {
/* Selecting "Cancel" in the "Import BAM file" dialog causes an error (UGENE-700)
 * 1. Open a _common_data/scenarios/assembly/example-alignment.bam
 * 2. Click "Cancel".
 *   Expected state: UGENE not crashed
*/
    class CancelScenario : public CustomScenario {
    public:
        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");
            GTGlobals::sleep(500);
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, new CancelScenario()));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/assembly/example-alignment.bam");
}

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
GUI_TEST_CLASS_DEFINITION(test_0746) {
    // 1. Open file _common_data\scenarios\_regression\764\short.fa
    // 2. Make 'Detailed view' visible (if not)
    // Expected state: 'Show amino translation' and 'Complement strand' buttons are enabled (not grey)
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/_regression/764/short.fa");
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    Runnable *chooser = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    QAbstractButton* translation = GTAction::button(os, "translation_action");
    CHECK_SET_ERR(translation -> isEnabled() == true, "button is not enabled");
    GTGlobals::sleep();
    QAbstractButton* complement = GTAction::button(os, "complement_action");
    CHECK_SET_ERR(complement -> isEnabled() == true, "button is not enabled");
}

GUI_TEST_CLASS_DEFINITION(test_0762) {
// 1. Open human_T1.fa from examples
// 
// 2. Try search tandems with default settings and with new Annotations Table.
// Expected state: UGENE not crashes
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new FindTandemsDialogFiller(os, sandBoxDir + "test_0762.gb"));
    GTToolbar::clickButtonByTooltipOnToolbar(os, MWTOOLBAR_ACTIVEMDI, "Find tandems");
    GTGlobals::sleep(200);
    GTUtilsTaskTreeView::waitTaskFinished(os);
}

GUI_TEST_CLASS_DEFINITION(test_0774) {
//    1. Create new scheme in Workflow Designer: "Read sequence" > "Write sequence".
//    2. Input two files in the "Input files" parameter of the "Read sequence" element.
//    3. Select "Merge" in the "Mode" parameter of the "Read sequence" element.
//    4. Set location of an output data file in the "Output file" parameter of the "Write sequence".
//    5. Run the schema.
//    Expected result: The scheme finished successfully.
//    6. Open the result file in Sequence View.
//    Expected result: The result file contains two sequences. Each sequence is merged sequences from
//    each input file respectively.

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    WorkflowProcessItem* read = GTUtilsWorkflowDesigner::addElement(os, "Read Sequence");
    CHECK_SET_ERR(read != NULL, "Read Sequence element not found");
//    GTUtilsWorkflowDesigner::setDatasetInputFolder(os, dataDir + "samples/Genbank");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Genbank/", "murine.gb");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Genbank/", "sars.gb");
    GTUtilsWorkflowDesigner::setParameter(os, "Mode", "Merge", GTUtilsWorkflowDesigner::comboValue);

    WorkflowProcessItem* write = GTUtilsWorkflowDesigner::addElement(os, "Write Sequence");
    CHECK_SET_ERR(write != NULL, "Write Sequence element not found");
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", QDir(sandBoxDir).absolutePath() +"/test_0774", GTUtilsWorkflowDesigner::textValue);

    GTUtilsWorkflowDesigner::connect(os, read, write);

    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTFileDialog::openFile(os, sandBoxDir + "/test_0774");

    CHECK_SET_ERR( GTUtilsSequenceView::getSeqWidgetsNumber(os) == 2, "Incorrect count of sequences");
}

GUI_TEST_CLASS_DEFINITION(test_0776) {
/* 1. Open WD.
 * 2. Create a scheme with the "Search for TFBS with weight matrix" element.
 * 3. Run the scheme with following parameters: input sequence - human_t1, input weight matrix - position_weight_matrix/UniPROBE/Cell08/Alx3_3418.2.pwm.
 *   Expected state: error report "Bad sequence supplied to Weight Matrix Search" doesn't appear
*/
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    WorkflowProcessItem* read = GTUtilsWorkflowDesigner::addElement(os, "Read Sequence");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    WorkflowProcessItem* readWM = GTUtilsWorkflowDesigner::addElement(os, "Read Weight Matrix");
    GTUtilsWorkflowDesigner::setParameter(os, "Input file(s)", QDir(dataDir).absolutePath() + "/position_weight_matrix/UniPROBE/Cell08/Alx3_3418.2.pwm", GTUtilsWorkflowDesigner::textValue);

    WorkflowProcessItem* search = GTUtilsWorkflowDesigner::addElement(os, "Search for TFBS with Weight Matrix");
    //Search for TFBS with Weight Matrix

    WorkflowProcessItem* write = GTUtilsWorkflowDesigner::addElement(os, "Write Sequence");

    GTUtilsWorkflowDesigner::connect(os, read, search);
    GTUtilsWorkflowDesigner::connect(os, readWM, search);
    GTUtilsWorkflowDesigner::connect(os, search, write);

    GTUtilsWorkflowDesigner::click(os, write);
    GTUtilsWorkflowDesigner::setParameter(os, "Document format", "genbank", GTUtilsWorkflowDesigner::comboValue);
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", QDir(sandBoxDir).absolutePath() + "/test_0776.gb", GTUtilsWorkflowDesigner::textValue);

    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
}

GUI_TEST_CLASS_DEFINITION(test_0778) {
//    1. Open file _common_data\scenarios\_regression\778\mixed.fa
//    2. Open menu {Settings->Prfrences}
//    Expected state: Application settings dialog opened
//    3. Select File format section, set {Create annotations for case switchings:} "Upper case annotation" and press OK button
//    Expected state: sequence view reopened, sequence marked with two "upper_case" annotations with coodinates 1..4, 8..10
//    4. Open menu {Settings->Prfrences}
//    Expected state: Application settings dialog opened
//    5. Select File format section, set {Create annotations for case switchings:} "Lower case annotation" and press OK button
//    Expected state: sequence view reopened, sequence marked with two "upper_case" annotations with coodinates 5..7, 11..13

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/_regression/778/mixed.fa");

    class CaseAnnotations : public CustomScenario {
    public:
        CaseAnnotations(QString name = QString())
            : name(name) {}
        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::FileFormat);

            QComboBox* caseCombo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "caseCombo"));
            CHECK_SET_ERR(caseCombo != NULL, "No caseCombo");
            GTComboBox::setIndexWithText(os, caseCombo, name);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    private:
        QString name;
    };

    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new CaseAnnotations("Use upper case annotations")));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action__settings"));
    GTMenu::showMainMenu(os, MWMENU_SETTINGS);
    GTGlobals::sleep();

    QList<U2Region> regions = GTUtilsAnnotationsTreeView::getAnnotatedRegions(os);
    CHECK_SET_ERR( regions.size() == 2, "Annotated regions number is incorrect");
    CHECK_SET_ERR( regions.contains( U2Region(0, 4) ), "No annotation 1..4");
    CHECK_SET_ERR( regions.contains( U2Region(7, 3) ), "No annotation 8..10");

    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new CaseAnnotations("Use lower case annotations")));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action__settings"));
    GTMenu::showMainMenu(os, MWMENU_SETTINGS);
    GTGlobals::sleep();

    regions = GTUtilsAnnotationsTreeView::getAnnotatedRegions(os);
    CHECK_SET_ERR( regions.size() == 2, "Annotated regions number is incorrect");
    CHECK_SET_ERR( regions.contains( U2Region(4, 3) ), "No annotation 1..4");
    CHECK_SET_ERR( regions.contains( U2Region(10, 3) ), "No annotation 8..10");

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0779) {
    // 1.Create a simple scheme with two elements:
    // The Read sequence element
    // The Write annotations element
    // 2. Connect the elements.
    // 3. Switch the "File format" property of the Write annotations element from "genbank" (defualt) to "csv".
    // 4. Click on the scheme area and you will get the crash.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Read Sequence");
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Write Annotations");
    GTUtilsWorkflowDesigner::setParameter(os, "Document format", "csv", GTUtilsWorkflowDesigner::comboValue);
    GTGlobals::sleep(500);
    GTUtilsWorkflowDesigner::connect(os, GTUtilsWorkflowDesigner::getWorker(os, "Read Sequence"), GTUtilsWorkflowDesigner::getWorker(os, "Write Annotations"));

}
GUI_TEST_CLASS_DEFINITION(test_0801) {
    // 1. Open human_T1.fa sequence
    // 2. Open find pattern tab on options panel {Ctrl+f}. Fill fields with next data:
    // {Search for} AAAGCTTTA
    // {Region} Custom region 2 5
    // Expected state: UGENE does not crash}
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsOptionsPanel::runFindPatternWithHotKey("AAAGCTTTA", os);
    GTUtilsOptionPanelSequenceView::setRegionType(os, "Custom region");
    GTLineEdit::setText(os, qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "editStart")), "2");
    GTLineEdit::setText(os, qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "editEnd")), "5");
}


GUI_TEST_CLASS_DEFINITION(test_0812) {
    // 1. Create a "seq.txt" file in <some_path> location.
    // 2. Click "Create element with command line tool".
    // 3. Input a name.
    // 4. Specify a slot.
    // 5. There is no need to add a parameter.
    // 6. Input the following execution string: "copy <some_path>\seq.txt <some_path>\seq2.txt" (change <some_path> to your location).
    // or the following execution string: "copy "<some_path>\seq.txt" "<some_path>\seq3.txt"" (with quotes for paths).
    // 7. Execute the schema (that contains this element only).
    // => The schema executes successfully.
    // 4. Verify whether the file has been copied.

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    CreateElementWithCommandLineToolFiller::ElementWithCommandLineSettings settings;
    settings.elementName = "Element_0812";

    QList<CreateElementWithCommandLineToolFiller::InOutData> input;
    CreateElementWithCommandLineToolFiller::InOutDataType inOutDataType;
    inOutDataType.first = CreateElementWithCommandLineToolFiller::Sequence;
    inOutDataType.second = "FASTA";
    input << CreateElementWithCommandLineToolFiller::InOutData("in1",
        inOutDataType);
    settings.input = input;
    settings.executionString = "copy _common_data/scenarios/_regression/812/seq.txt _common_data/scenarios/_regression/812/seq2.txt";

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "Continue"));

    GTUtilsDialog::waitForDialog(os, new CreateElementWithCommandLineToolFiller(os, settings));
    QAbstractButton *createElement = GTAction::button(os, "createElementWithCommandLineTool");
    GTWidget::click(os, createElement);
    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "Ok"));

    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
}
GUI_TEST_CLASS_DEFINITION(test_0814) {
//    1. Open UGENE preferences in main menu.
//    Expected state: "Application settings" dialog appeared.
//    2. Go to the {Logging} part.
//    Expected state: logging settings were shown.
//    3. Check the {Save output to file} checkbox, click OK button.
//    Expected state: warning message appeared.
//    4. Close warning message.
//    Expected state: "Application settings" dialog closed.
//    5. Repeat steps 1-3.
//    Expected state: logging settings were shown, {Save output to file} checkbox is unchecked.
//    6. Check the {Save output to file} checkbox, enter some file (manually or by select file dialog) and click OK button.
//    Expected state: "Application settings dialog" closed without any messages.
//    7. Find your file on on the disk.
//    Expected state: log file exists.

    class LogFile_1 : public CustomScenario {
    public:
        LogFile_1(QString name = QString())
            : name(name) {}
        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");

            AppSettingsDialogFiller::openTab(os, AppSettingsDialogFiller::Logging);

            QCheckBox* fileOut = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "fileOutCB"));
            CHECK_SET_ERR(fileOut != NULL, "No fileOutCB");
            CHECK_SET_ERR(!fileOut->isChecked(), "CheckBox is checked!");
            GTCheckBox::setChecked(os, fileOut);

            QLineEdit* fileName = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "outFileEdit"));
            CHECK_SET_ERR(fileName != NULL, "No outFileEdit");
            GTLineEdit::setText(os, fileName, name);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    private:
        QString name;
    };

    GTLogTracer l;

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));

    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new LogFile_1()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action__settings"));
    GTMenu::showMainMenu(os, MWMENU_SETTINGS);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new LogFile_1(QDir(sandBoxDir).absolutePath() + "test_0814_log")));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action__settings"));
    GTMenu::showMainMenu(os, MWMENU_SETTINGS);

    CHECK_SET_ERR(GTFile::check(os, QDir(sandBoxDir).absolutePath() + "test_0814_log") == true, "Log file not found");
}

GUI_TEST_CLASS_DEFINITION(test_0818) {
//    1) Open WD, Click "create element with command line tool"
//    2) Fill in the "Name" lineedit with a name containing space symbol
//    Expected state:
//        Status - text is red
//        Next button is disabled

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    class SpaceNameFiller: public Filler {
    public:
        SpaceNameFiller(U2OpStatus& _os) : Filler(_os, "CreateExternalProcessWorkerDialog"){}
        virtual void run() {
            QWidget *w = QApplication::activeWindow();
            CHECK(NULL != w, );

            QLineEdit* nameEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "nameLineEdit", w));
            CHECK_SET_ERR(nameEdit != NULL, "nameLineEdit not found");
            GTLineEdit::setText(os, nameEdit, "External tool element");
            GTGlobals::sleep();

            QWidget* nextButton = GTWidget::findWidget(os, "__qt__passive_wizardbutton1", w);
            CHECK_SET_ERR(nextButton != NULL, "Next button not found");
            CHECK_SET_ERR(!nextButton->isEnabled(), "Next button is enabled");
            GTGlobals::sleep();

            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);
        }
    };

    GTUtilsDialog::waitForDialog(os, new SpaceNameFiller(os));
    QAbstractButton *createElement = GTAction::button(os, "createElementWithCommandLineTool");
    GTWidget::click(os, createElement);
}
GUI_TEST_CLASS_DEFINITION(test_0821) {
    // 1. Open files samples/genbank/sars.gb and samples/genbank/murine.gb in merge mode
    // Expected state: annotations in both files has right coordinates

    GTSequenceReadingModeDialog::mode = GTSequenceReadingModeDialog::Merge;
    GTUtilsDialog::waitForDialog(os, new GTSequenceReadingModeDialogUtils(os));
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils_list(os, dataDir + "samples/Genbank/", QStringList() << "sars.gb" << "murine.gb"));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__OPEN_PROJECT);

    GTGlobals::sleep(5000);

    // TODO: QList<U2Region> cds = GTUtilsAnnotationsTreeView::getAnnotatedRegionsOfGroup(os, "contig (0, 2)");
    // TODO: CHECK_SET_ERR( cds.contains(U2Region(0, 5833)), "No 1..5833 region");

}

GUI_TEST_CLASS_DEFINITION(test_0828) {
//     1. Open WD.
//     2. Add to scheme "Sequence marker" element.
//     3. In property editor groupbox "Parameters" press "Add" button.
//     Expected state: "Create marker group" dialog appears, its combobox "Marker group type" contains field "Sequence name markers".
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::addElement(os, "Sequence Marker");
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

            QComboBox* combo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "typeBox"));
            GTComboBox::setIndexWithText(os, combo, "Sequence name markers", true);

            QPushButton *button = buttonBox->button(QDialogButtonBox::Cancel);
            CHECK(NULL != button, );
            GTWidget::click(os, button);
        }
    };
    GTUtilsDialog::waitForDialog(os, new OkClicker(os));
    GTWidget::click(os, addButton);
    GTGlobals::sleep(2000);
}

GUI_TEST_CLASS_DEFINITION(test_0829) {
//1. Add input files into CAP3, for example, 1.fa, 2.fa, 3.fa and press "Run". The result contig name will be "3.cap.ace".
//2. Add input files into CAP3, for example, 4.fa, 5.fa, 3.fa and press "Run". The result contig name will be "3.cap.ace". But error occurs:
//Bug state:
//[20:46:07] 'CAP3SupportTask' task failed: Subtask {Opening view for document: 3.cap.ace} is failed: Subtask {Adding document to project: D:/Documents/tests/3.cap.ace} is failed: Document is already added to the project D:/Documents/tests/3.cap.ace.
//Expected state:
//Dialog with the following options appears: rewrite the existent file, rename the new file or cancel.
    GTLogTracer lt;
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    WorkflowProcessItem *readSeq = GTUtilsWorkflowDesigner::addElement(os, "Read Sequence");
    WorkflowProcessItem *cap3 = GTUtilsWorkflowDesigner::addElement(os, "Assembly Sequences with CAP3");
    
    GTUtilsWorkflowDesigner::connect(os, readSeq, cap3);
    
    GTUtilsWorkflowDesigner::click(os, "Read Sequence");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/scenarios/_regression/829", "1.fa");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/scenarios/_regression/829", "2.fa");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/scenarios/_regression/829", "3.fa");
    
    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    QApplication::activeWindow();
    GTWidget::click(os,
        GTToolbar::getWidgetForActionName(os,
        GTToolbar::getToolbar(os, "mwtoolbar_activemdi")
        , "toggleDashboard"));

    GTUtilsWorkflowDesigner::removeItem(os, "Read Sequence");
    GTUtilsWorkflowDesigner::removeItem(os, "Assembly Sequences with CAP3");

    readSeq = GTUtilsWorkflowDesigner::addElement(os, "Read Sequence");
    cap3 = GTUtilsWorkflowDesigner::addElement(os, "Assembly Sequences with CAP3");

    GTUtilsWorkflowDesigner::connect(os, readSeq, cap3);

    GTUtilsWorkflowDesigner::click(os, "Read Sequence");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/scenarios/_regression/829", "1.fa");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/scenarios/_regression/829", "2.fa");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/scenarios/_regression/829", "4.fa");

    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR(!lt.hasError(), "Log contains error");
}

GUI_TEST_CLASS_DEFINITION(test_0830) {
    //1) Tools->DNA assembly->Config assembly with CAPS3
    //2) Base ->
    //    Input Files:
    //        _common_data/scenarios/CAP3/region2.fa
    //        _common_data/scenarios/CAP3/region4.fa
    //    Run
    QString outUrl = sandBoxDir + "830.ace";
    GTUtilsDialog::waitForDialog(os, new CAP3SupportDialogFiller(os, QStringList()
        << testDir + "_common_data/scenarios/CAP3/region2.fa"
        << testDir + "_common_data/scenarios/CAP3/region4.fa",
        outUrl));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ToolsMenu::SANGER_MENU << ToolsMenu::SANGER_DENOVO));
    GTMenu::showMainMenu(os, "mwmenu_tools");

    //3) wait for task error, ensure that no output files are created
    GTUtilsTaskTreeView::waitTaskFinished(os);
    CHECK_SET_ERR(!QFile::exists(outUrl), "The output file is created");
}

GUI_TEST_CLASS_DEFINITION(test_0834) {
    GTFileDialog::openFile(os,  testDir + "_common_data/genbank/NC_014267.1_cut.gb");

    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getQualifierValue(os, "gene", "gene  (0, 1)") == "join(1..74213,77094..140426)", 
        "Annotation \"gene\" has incorrect location");
}

GUI_TEST_CLASS_DEFINITION(test_0835) {
    //1. Open, for example, "murine.gb" and "sars.gb".
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/murine.gb");
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/sars.gb");

    //2. Open the sequence objects in one Sequence View.
    //= > Both sequences are shown in one Sequence View.
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363"));
    GTMouseDriver::press(os);
    GTGlobals::sleep(100);
    QWidget *seqView = GTUtilsSequenceView::getSeqWidgetByNumber(os);
    GTMouseDriver::moveTo(os, seqView->mapToGlobal(seqView->rect().center()));
    GTMouseDriver::release(os);

    //3. Enable Circular View for one of the sequences(for example, "murine.gb").
    //= > The Circular View with the Restriction Site Map is shown.
    GTWidget::click(os, GTWidget::findWidget(os, "CircularViewAction"));

    QWidget *restrictionMapTreeWidget = GTWidget::findWidget(os, "restrictionMapTreeWidget");
    CHECK_SET_ERR(NULL != restrictionMapTreeWidget && restrictionMapTreeWidget->isVisible(),
        "Restriction map widget isn't visible unexpectedly");

    //4. Delete the sequence with the Circular View from the Project View.
    //= > The sequence has been deleted, the Circular View is not shown.
    //!= > The Restriction Site Map is NOT still shown.
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "sars.gb"));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(100);

    restrictionMapTreeWidget = GTWidget::findWidget(os, "restrictionMapTreeWidget");
    CHECK_SET_ERR(NULL == restrictionMapTreeWidget, "Restriction map widget is visible unexpectedly");
}

GUI_TEST_CLASS_DEFINITION(test_0839) {
// 1. Use menu {Tools->Weight matrix->Build Weight Matrix}.
// Expected state: "Build weight or frequency matrix" dialog appeared.
// 
// 2. Click {...} button for "Input item".
// Expected state: "Select file with alignment" dialog appeared.
// 
// 3. Open any non msa file (e.g. a tree file  - *.nwk format).
// Expected state: 
// 1). UGENE doesn`t crach.
// 2). Messagebox  about unsupported format appeared.
    QList<PwmBuildDialogFiller::Action> actions;
    actions << PwmBuildDialogFiller::Action(PwmBuildDialogFiller::ExpectInvalidFile, "");
    actions << PwmBuildDialogFiller::Action(PwmBuildDialogFiller::SelectInput, dataDir + "samples/GFF/5prime_utr_intron_A20.gff");
    actions << PwmBuildDialogFiller::Action(PwmBuildDialogFiller::ClickCancel, "");
    GTUtilsDialog::waitForDialog(os, new PwmBuildDialogFiller(os, actions));

    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_TOOLS), QStringList() << ToolsMenu::TFBS_MENU << ToolsMenu::TFBS_WEIGHT);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0840) {
    //"Digest Into Fragments" displays number of cuts incorrectly
    //When enzyme is presented both in auto-annotation and in permanent annotation, the number of cuts shows +1.

    class DigestCircularSequenceScenario : public CustomScenario {
    public:
        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();

            QListWidget *availableEnzymeWidget = GTWidget::findExactWidget<QListWidget *>(os, "availableEnzymeWidget", dialog);
            CHECK_SET_ERR(NULL != availableEnzymeWidget, "Cannot find available enzyme list widget");

            QList<QListWidgetItem *> items = availableEnzymeWidget->findItems("EcoRV : 1 cut(s)", Qt::MatchExactly);
            CHECK_SET_ERR(items.size() == 1, "Unexpected number of enzymes");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    class CreateAnnotationScenario : public CustomScenario {
    public:
        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "Active modal widget is NULL");

            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit *>(os, "leAnnotationName", dialog), "EcoRV");
            GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit *>(os, "leGroupName", dialog), "enzyme");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    //1. Open "_common_data/enzymes/bsai_reverse_test.fa".
    GTFileDialog::openFile(os, testDir + "_common_data/enzymes/bsai_reverse_test.fa");

    //2. Click "Find restriction sites" on toolbar.
    //3. Click OK button in dialog appeared.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ANALYSE" << "Find restriction sites"));
    GTUtilsDialog::waitForDialog(os, new FindEnzymesDialogFiller(os, QStringList("EcoRV")));
    GTMenu::showContextMenu(os, GTUtilsSequenceView::getSeqWidgetByNumber(os));
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: One new auto-annotation appears ("enzyme/EcoRV")
    //4. Select this annotation.
    CHECK_SET_ERR(1 == GTUtilsAnnotationsTreeView::findItems(os, "EcoRV").size(), "Unexpected annotation count");
    GTUtilsAnnotationsTreeView::selectItems(os, QStringList() << "EcoRV");

    //5. Select "New annotation" in context menu.
    //6. Fill fields with: "Group name" - "enzyme", "Annotation name" - "EcoRV".
    //7. Click Create button.
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, new CreateAnnotationScenario));
    GTToolbar::clickButtonByTooltipOnToolbar(os, MWTOOLBAR_ACTIVEMDI, "New annotation");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: Non-auto annotation appears ("enzyme/EcoRV") for the same region.
    QList<QTreeWidgetItem *> items = GTUtilsAnnotationsTreeView::findItems(os, "EcoRV");
    CHECK_SET_ERR(2 == items.size(), "Unexpected annotation count");
    CHECK_SET_ERR(items[0]->parent() != items[1]->parent(), "Annotations are siblings unexpectedly");

    //8. Select "Actions"->"Cloning"->"Digest into Fragments..." in main menu.
    //Expected state: there is only one enzyme in the "Available enzymes" field with 1 cut.
    //Bug state: there is one enzyme but with 2 cuts.
    GTUtilsDialog::waitForDialog(os, new DigestSequenceDialogFiller(os, new DigestCircularSequenceScenario));
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Cloning" << "Digest into fragments..."));
    GTMenu::showContextMenu(os, GTUtilsSequenceView::getSeqWidgetByNumber(os));
    GTUtilsTaskTreeView::waitTaskFinished(os);
}

GUI_TEST_CLASS_DEFINITION(test_0844) {
/* 1. Open "samples/human_t1".
 * 2. In advanced settings of Tandem Finder choose "Suffix array" (unoptimized algorithm)
 * 3. Start finding tandems
 *   Expected state: UGENE no crashes
*/
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/human_T1.fa");

    class Scenario : public CustomScenario {
        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "Active modal widget is NULL");

            QLineEdit* pathEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "leNewTablePath"));
            pathEdit->setText(sandBoxDir + "test_0844.gb");
            GTGlobals::sleep();

            GTTabWidget::setCurrentIndex(os, GTWidget::findExactWidget<QTabWidget *>(os, "tabWidget"), 1);

            QComboBox* combo = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "algoComboBox"));
            CHECK_SET_ERR(combo != NULL, "algoComboBox not found!");
            GTComboBox::setIndexWithText(os, combo , "Suffix array");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTUtilsDialog::waitForDialog(os, new FindTandemsDialogFiller(os, new Scenario));
    GTToolbar::clickButtonByTooltipOnToolbar(os, MWTOOLBAR_ACTIVEMDI, "Find tandems");
    GTGlobals::sleep(200);
    GTUtilsTaskTreeView::waitTaskFinished(os);
}

GUI_TEST_CLASS_DEFINITION(test_0854) {
/* 1. Open samples/genbank/PBR322.gb.
 * 2. Export the sequence into alignment using context menu in project tab.
 * 3. Align it with MUSCLE.
 *   Expected result: UGENE doesn't crash.
*/
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/PBR322.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "PBR322.gb"));
    GTMouseDriver::click(os);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT));
    GTUtilsDialog::waitForDialog(os, new ExportSequenceAsAlignmentFiller(os, sandBoxDir, "test_0854.aln", ExportSequenceAsAlignmentFiller::Clustalw, true));
    GTMouseDriver::click(os, Qt::RightButton);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new MuscleDialogFiller(os, MuscleDialogFiller::Default));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_ALIGN << "Align with muscle", GTGlobals::UseMouse));
    GTUtilsMSAEditorSequenceArea::moveTo(os, QPoint(0, 0));
    GTMouseDriver::click(os, Qt::RightButton);
    GTUtilsTaskTreeView::waitTaskFinished(os);
}

GUI_TEST_CLASS_DEFINITION(test_0858) {
//    1. Open human_t1.fa
//    2. Open the Statistics bar on the Options Panel
//    3. Select and copy Characters Occurrence table
//    4. Paste the table in a Text Editor
//    Expected state: the table in the editor and in the Options Panel have the same content

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Statistics);

    QLabel* label = GTWidget::findExactWidget<QLabel*>(os, "characters_occurrence_label");

    QString expected = "<table cellspacing=5><tr><td><b>A:&nbsp;&nbsp;</td><td>62 842 &nbsp;&nbsp;</td><td>"
            "31.4%&nbsp;&nbsp;</td></tr><tr><td><b>C:&nbsp;&nbsp;</td><td>40 041 &nbsp;&nbsp;</td><td>20.0%&nbsp;"
            "&nbsp;</td></tr><tr><td><b>G:&nbsp;&nbsp;</td><td>37 622 &nbsp;&nbsp;</td><td>18.8%&nbsp;&nbsp;</td>"
            "</tr><tr><td><b>T:&nbsp;&nbsp;</td><td>59 445 &nbsp;&nbsp;</td><td>29.7%&nbsp;&nbsp;</td></tr></table>";
    QString got = label->text();
    CHECK_SET_ERR(got == expected, QString("The clipboard text is incorrect: [%1], expected [%2]").arg(got).arg(expected));
}

GUI_TEST_CLASS_DEFINITION(test_0861_1) {
    //1. Open sars.gb
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/sars.gb");

    //2. Open the "Annotations Highlighting" bar of the Options Panel
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::AnnotationsHighlighting);

    //Expected state : all annotation groups of sars.gb are in the "Annotation type" window:
    // 3'UTR, 5'UTR, CDS, comment, gene, mat_peptide, misc_feature, source
    QTreeWidget *highlightTree = GTWidget::findExactWidget<QTreeWidget *>(os, "OP_ANNOT_HIGHLIGHT_TREE");
    CHECK_SET_ERR(highlightTree->topLevelItemCount() == 8, "Unexpected number of annotations");
    CHECK_SET_ERR(highlightTree->topLevelItem(0)->text(0) == "3'UTR", QString("Unexpected annotation name at row %1").arg(0));
    CHECK_SET_ERR(highlightTree->topLevelItem(1)->text(0) == "5'UTR", QString("Unexpected annotation name at row %1").arg(1));
    CHECK_SET_ERR(highlightTree->topLevelItem(2)->text(0) == "CDS", QString("Unexpected annotation name at row %1").arg(2));
    CHECK_SET_ERR(highlightTree->topLevelItem(3)->text(0) == "comment", QString("Unexpected annotation name at row %1").arg(3));
    CHECK_SET_ERR(highlightTree->topLevelItem(4)->text(0) == "gene", QString("Unexpected annotation name at row %1").arg(4));
    CHECK_SET_ERR(highlightTree->topLevelItem(5)->text(0) == "mat_peptide", QString("Unexpected annotation name at row %1").arg(5));
    CHECK_SET_ERR(highlightTree->topLevelItem(6)->text(0) == "misc_feature", QString("Unexpected annotation name at row %1").arg(6));
    CHECK_SET_ERR(highlightTree->topLevelItem(7)->text(0) == "source", QString("Unexpected annotation name at row %1").arg(7));
}

GUI_TEST_CLASS_DEFINITION(test_0861_2) {
    //1. Open sars.gb
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/sars.gb");

    //2. Open the "Annotations Highlighting" bar of the Options Panel
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::AnnotationsHighlighting);
    QTreeWidget *highlightTree = GTWidget::findExactWidget<QTreeWidget *>(os, "OP_ANNOT_HIGHLIGHT_TREE");
    CHECK_SET_ERR(highlightTree->topLevelItemCount() == 8, "Unexpected number of annotations");

    //3. Switch on ORFs auto annotation
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Show ORFs"));
    GTWidget::click(os, GTWidget::findWidget(os, "toggleAutoAnnotationsButton"));
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected : "ORF" annotation appeared in the "Annotation type" window
    CHECK_SET_ERR(highlightTree->topLevelItemCount() == 9, "Unexpected number of annotations");
    CHECK_SET_ERR(highlightTree->topLevelItem(7)->text(0) == "orf", QString("Unexpected annotation name at row %1").arg(7));
}

GUI_TEST_CLASS_DEFINITION(test_0861_3) {
    //1. Open sars.gb
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/sars.gb");

    //2. Open the "Annotations Highlighting" bar of the Options Panel
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::AnnotationsHighlighting);
    QTreeWidget *highlightTree = GTWidget::findExactWidget<QTreeWidget *>(os, "OP_ANNOT_HIGHLIGHT_TREE");
    CHECK_SET_ERR(highlightTree->topLevelItemCount() == 8, "Unexpected number of annotations");

    //3. Remove 3'UTR annotation group.
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "3'UTR  (0, 1)"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(100);

    //Expected state : 3'UTR group disapeared from "Annotation type" window
    CHECK_SET_ERR(highlightTree->topLevelItemCount() == 7, "Unexpected number of annotations");
    CHECK_SET_ERR(highlightTree->topLevelItem(0)->text(0) == "5'UTR", QString("Unexpected annotation name at row %1").arg(0));
}

GUI_TEST_CLASS_DEFINITION(test_0861_4) {
    // 1. Open human_t1.fa
    // 2. Open the "Annotations Highlighting" bar of the Options Panel
    // TODO: Expected state: the message "The sequence doesn't have any annotations." is shown on the bar

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::AnnotationsHighlighting);
    GTGlobals::sleep(500);
    QLabel* noAnnotTypesLabel = GTWidget::findExactWidget<QLabel*>(os, "noAnnotTypesLabel");
    CHECK_SET_ERR(noAnnotTypesLabel->isVisible(), "label is not visiable");
    CHECK_SET_ERR(noAnnotTypesLabel->text() == "The sequence doesn't have any annotations.",
                  "unexpected text: " + noAnnotTypesLabel->text());
}

GUI_TEST_CLASS_DEFINITION(test_0866) {
//1. Open WD
//2. Ad the following elements to the scheme: File List, Write Plain Text 1, Write Plain Text 2
//3. Connect File List to both writers
//Expected state: File List connected successfully

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    WorkflowProcessItem *fileList = GTUtilsWorkflowDesigner::addElement(os, "File List");
    WorkflowProcessItem *writer1 = GTUtilsWorkflowDesigner::addElement(os, "Write Plain Text");
    GTUtilsWorkflowDesigner::addElement(os, "Write Plain Text");

    GTUtilsWorkflowDesigner::connect(os, fileList, writer1);
    GTUtilsWorkflowDesigner::connect(os, fileList, GTUtilsWorkflowDesigner::getWorker( os, "Write Plain Text 1"));
}

GUI_TEST_CLASS_DEFINITION(test_0868){
//    1. Open chrM in Assembly Browser
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, sandBoxDir + "chrM.sorted.bam.ugenedb"));
    GTFileDialog::openFile(os, dataDir + "samples/Assembly", "chrM.sorted.bam");
    GTUtilsTaskTreeView::waitTaskFinished(os);
//    2. Zoom to any covered
    GTWidget::click(os, GTUtilsMdi::activeWindow(os));
    for (int i = 0;i < 24;i++){
        GTKeyboardDriver::keyClick(os, '=', GTKeyboardDriver::key["shift"]);
        GTGlobals::sleep(100);
    }
//    3. Add bookmark
    GTUtilsBookmarksTreeView::addBookmark(os, GTUtilsMdi::activeWindow(os)->objectName(), "bookmark");
    GTGlobals::sleep();
    GTMouseDriver::moveTo(os, GTUtilsBookmarksTreeView::getItemCenter(os, "bookmark"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    QWidget* assembly_reads_area = GTWidget::findWidget(os, "assembly_reads_area");
    QPixmap pixmap = QPixmap::grabWidget(assembly_reads_area, assembly_reads_area->rect());
    QImage initImg = pixmap.toImage();
//    4. Go to any other region
    GTWidget::click(os, GTUtilsMdi::activeWindow(os));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["home"]);
//    5. Double click on the bookmark
    GTMouseDriver::moveTo(os, GTUtilsBookmarksTreeView::getItemCenter(os, "bookmark"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
//    Expected state: it shows the location that you saved before
    assembly_reads_area = GTWidget::findWidget(os, "assembly_reads_area");
    pixmap = QPixmap::grabWidget(assembly_reads_area, assembly_reads_area->rect());
    QImage finalImg = pixmap.toImage();
    CHECK_SET_ERR(initImg == finalImg, "bookmark does not work");
}

GUI_TEST_CLASS_DEFINITION(test_0871) {
    // 1. Open WD
    // 2. Create a scheme with the following elements: Read Sequence, Amino Translation, Write Sequence
    // 3. Put humant_t1.fa as an input sequence
    // 4. Run the scheme
    // Expected state: the sequences that correspond to 3 translations frames of human_t1 are created

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    WorkflowProcessItem* read = GTUtilsWorkflowDesigner::addElement(os, "Read Sequence");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    WorkflowProcessItem* amino = GTUtilsWorkflowDesigner::addElement(os, "Amino Translation");
    WorkflowProcessItem* write = GTUtilsWorkflowDesigner::addElement(os, "Write Sequence");
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", QDir(sandBoxDir).absolutePath() + "/test_0871", GTUtilsWorkflowDesigner::textValue);
    GTUtilsWorkflowDesigner::connect(os, read, amino);
    GTUtilsWorkflowDesigner::connect(os, amino, write);
    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTFileDialog::openFile(os, sandBoxDir, "/test_0871");

    GTUtilsProjectTreeView::rename(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134) Amino translation 0 direct", "0");
    GTUtilsProjectTreeView::rename(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134) Amino translation 1 direct", "1");
    GTUtilsProjectTreeView::rename(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134) Amino translation 2 direct", "2");
}

GUI_TEST_CLASS_DEFINITION( test_0873 ){
    //    1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    GTUtilsWorkflowDesigner::addSample(os, "Merge sequences and shift corresponding annotations");

    GTUtilsWorkflowDesigner::click(os, "Read Sequence");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Genbank", "murine.gb");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, dataDir + "samples/Genbank", "sars.gb");

    //    3. fill all needed parameters and run schema
    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    //    Expected state: there are more then 10 result files and they are grouped into sublists

    QWebElement button = GTUtilsDashboard::findElement(os, "merged.fa", "BUTTON");
    GTUtilsDashboard::click(os, button);

    int sequenceLength = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(sequenceLength == 35594, "Sequence length is " + QString::number(sequenceLength) + ", expected 35594");
}

GUI_TEST_CLASS_DEFINITION(test_0878) {
//    1. Open several documents of any kind - sequence view, workflow designer, whatever.
//    2. Activate any document except the #1 (as numbered in Window menu).
//    3. Click on the Log button at bottom toolbar.
//    Expected state: Log view is opened, selected document is shown.
//    4. Click on the Log button again.
//    Expected state: Log view is closed, selected document is active (not first).

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/sars.gb");

    QWidget* win = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(win != NULL, "Active window not found");

    GTWidget::click(os, GTWidget::findWidget(os, "doc_lable_dock_log_view"));
    CHECK_SET_ERR(win == GTUtilsMdi::activeWindow(os), "Incorrect active window");
    GTWidget::click(os, GTWidget::findWidget(os, "doc_lable_dock_log_view"));
    CHECK_SET_ERR(win == GTUtilsMdi::activeWindow(os), "Incorrect active window");
}

GUI_TEST_CLASS_DEFINITION(test_0882) {
    //1. Open "data/samples/FASTQ/eas.fastq".
    //2. Choose the "Align reads to reference sequence" option in appearing "Sequence reading option" dialog.
    //3. Click "Ok" and specify a reference sequence in the "Align short reads" dialog (Use UGENE genome aligner).
    //4. Click "Start" button.
    //Expected result: The dialog disappears, a notification about alignment results appears.
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Align));
    AlignShortReadsFiller::UgeneGenomeAlignerParams parameters(dataDir + "samples/FASTA/human_T1.fa", QStringList());
    GTUtilsDialog::waitForDialog(os, new AlignShortReadsFiller(os, &parameters));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "can't be mapped"));
    GTFileDialog::openFile(os, dataDir + "samples/FASTQ/eas.fastq");
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0886) {
    // 1. Open file "_common_data/fasta/Gene.fa" in UGENE.
    // Expected result: UGENE doesn't crash
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "Gene.fa");

}

GUI_TEST_CLASS_DEFINITION(test_0908) {
    //1) Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    //2) Click "Create element with command line tool"
    //3) input name "test"
    //4) input data : "in1" and "in2" of FASTA
    //5) output data : "out1" of FASTA
    //6) Execution string : "cmd /c copy $in1 $out1 | copy $in2 $out1"
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, testDir + "_common_data/scenarios/_regression/908/test.etc"));
    GTWidget::click(os, GTAction::button(os, "AddElementWithCommandLineTool"));

    //7) Add input and output readers of FASTA
    GTUtilsWorkflowDesigner::addElement(os, "Read Sequence");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/fasta", "AMINO.fa");

    GTUtilsWorkflowDesigner::addElement(os, "Read Sequence");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/fasta", "alphabet.fa");

    WorkflowProcessItem* writer = GTUtilsWorkflowDesigner::addElement(os, "Write Sequence");

    WorkflowProcessItem *cmdlineWorker = GTUtilsWorkflowDesigner::getWorker(os, "test");

    GTUtilsWorkflowDesigner::connect(os, GTUtilsWorkflowDesigner::getWorker(os, "Read Sequence"), cmdlineWorker);
    GTUtilsWorkflowDesigner::connect(os, GTUtilsWorkflowDesigner::getWorker(os, "Read Sequence 1"), cmdlineWorker);
    GTUtilsWorkflowDesigner::connect(os, cmdlineWorker, writer);

    //8) Run schema
    //Expected state : UGENE not crashed
    GTUtilsWorkflowDesigner::runWorkflow(os);
}

GUI_TEST_CLASS_DEFINITION(test_0889) {
//  1) Open RNA.fa
//  2) Use context menu on sequence {Align->Align sequence to mRNA}
//  3) Select any item
//  4) Click "Create"
//
//  Expected state: UGENE not crashed
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os, SequenceReadingModeSelectorDialogFiller::Merge));
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "RNA.fa");

    class OkClicker : public Filler {
    public:
        OkClicker(U2OpStatus& _os) : Filler(_os, "CreateAnnotationDialog"){}
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
    GTUtilsDialog::waitForDialog(os, new ProjectTreeItemSelectorDialogFiller(os, "RNA.fa", "Sequence"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ALIGN" << "Align sequence to mRNA"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
}

GUI_TEST_CLASS_DEFINITION(test_0896) {
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, testDir + "_common_data/scenarios/_regression/896/_input", "SAMtools.etc");
    GTUtilsDialog::waitForDialog(os, ob);
    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new MessageBoxDialogFiller(os, QMessageBox::Discard));

    QAbstractButton* button = GTAction::button(os, "AddElementWithCommandLineTool");
    GTWidget::click(os, button);
    GTUtilsMdi::click(os, GTGlobals::Close);

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/_regression/896/_input/url_out_in_exttool.uwl");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QString outputFile = QDir(sandBoxDir).absolutePath() + "/test_0896out.bam";

    class OkClicker : public Filler {
    public:
        OkClicker(U2OpStatus& _os) : Filler(_os, "CreateExternalProcessWorkerDialog"){}
        virtual void run() {
            QWidget *w = QApplication::activeWindow();
            CHECK(NULL != w, );

            ExternalTool *samtools = AppContext::getExternalToolRegistry()->getByName("SAMtools");
            QLineEdit *ed = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "templateLineEdit", w));
            GTLineEdit::setText(os, ed, samtools->getPath() + " view -b -S -o " + QDir(sandBoxDir).absolutePath() + "/test_0896out.bam $sam");

            QAbstractButton *button = GTWidget::findButtonByText(os, "Finish");
            CHECK(NULL != button, );
            GTWidget::click(os, button);
        }
    };

    GTUtilsDialog::waitForDialog(os, new OkClicker(os));
    GTMouseDriver::moveTo(os, GTUtilsWorkflowDesigner::getItemCenter(os, "SAMtools"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "editConfiguration"));
    GTMouseDriver::click(os);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTUtilsWorkflowDesigner::click(os, "File List");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/bowtie/pattern", "e_coli_1000.sam");

    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, sandBoxDir + "/test_0896"));
    GTFileDialog::openFile(os, sandBoxDir, "/test_0896out.bam");


}

GUI_TEST_CLASS_DEFINITION(test_0898){
//    1. Open _common_data/ugenedb/example_alignment.ugenedb
    GTFileDialog::openFile(os, testDir + "_common_data/ugenedb/example-alignment.ugenedb");
//    2. Zoom in until reads appear.

//    3. Try to find consensus above reads area.
//    Expected state: you found consensus.
    QWidget* consArea = GTWidget::findWidget(os, "Consensus area");
    CHECK_SET_ERR(consArea->isVisible(), "consensus area is not visiable");
//    4. Open {Assembly Browser Settings} on options panel.
    GTWidget::click(os, GTWidget::findWidget(os, "OP_ASS_SETTINGS"));
    GTGlobals::sleep(500);
//    5. Look for any algorithms in {Consensus algorithm} combobox (SAMtools, for example).
    QComboBox* consensusAlgorithmCombo = GTWidget::findExactWidget<QComboBox*>(os, "consensusAlgorithmCombo");
//    Expected state: there are some algorithms.
    GTComboBox::checkValuesPresence(os, consensusAlgorithmCombo, QStringList()<<"SAMtools"<<"Default");
}

GUI_TEST_CLASS_DEFINITION(test_0899){
//    1) Import data\samples\Assembly\chrM.sorted.bam
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, sandBoxDir + "chrM.sorted.bam.ugenedb"));
    GTFileDialog::openFile(os, dataDir + "samples/Assembly", "chrM.sorted.bam");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    class Scenario : public CustomScenario {
        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "Active modal widget is NULL");

            //    Expected state:
            //        1) opened dialog have File formats: {FASTA, FASTQ, GFF, Genbank, Vector NTI sequence}
            QComboBox* documentFormatComboBox = GTWidget::findExactWidget<QComboBox*>(os, "documentFormatComboBox", dialog);
            QStringList comboList;
            comboList<<"FASTA"<<"FASTQ"<<"GFF"<<"Genbank"<<"Vector NTI sequence";
            GTComboBox::checkValuesPresence(os, documentFormatComboBox, comboList);

            //        2) region: {whole sequence, visible, custom}
            QComboBox* region_type_combo = GTWidget::findExactWidget<QComboBox*>(os, "region_type_combo", dialog);
            QStringList regionComboList;
            regionComboList<<"Whole sequence"<<"Visible"<<"Custom region";
            GTComboBox::checkValuesPresence(os, region_type_combo, regionComboList);

            GTComboBox::setIndexWithText(os, documentFormatComboBox, "Genbank");
            GTComboBox::setIndexWithText(os, region_type_combo, "Whole sequence");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };
//    Export consensus
    GTUtilsDialog::waitForDialog(os, new ExportConsensusDialogFiller(os, new Scenario()));
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Export consensus..."));
    GTWidget::click(os, GTWidget::findWidget(os, "Consensus area"), Qt::RightButton);
    GTUtilsProjectTreeView::checkItem(os, "chrM_consensus.gb");
}

GUI_TEST_CLASS_DEFINITION(test_0910) {
//    1. Create a scheme with the "Read sequence" element and the "Write sequence element".
//    2. Take a file with more then one sequence as an input for the "Read sequence" element.
//    3. Set the "Accumulate objects" parameters to False
//    4. Set the "Existing file" parameter to "Rename"
//    5. Run the scheme
//    6. The number of output files must be equal to the number of input sequences

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);

    WorkflowProcessItem* read = GTUtilsWorkflowDesigner::addElement(os, "Read Sequence");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/fasta", "multy_fa.fa");

    WorkflowProcessItem* write = GTUtilsWorkflowDesigner::addElement(os, "Write Sequence");
    GTUtilsWorkflowDesigner::setParameter(os, "Accumulate objects", "False", GTUtilsWorkflowDesigner::comboValue);
    GTUtilsWorkflowDesigner::setParameter(os, "Existing file", "Rename", GTUtilsWorkflowDesigner::comboValue);
    GTUtilsWorkflowDesigner::setParameter(os, "Output file", QDir(sandBoxDir).absolutePath() + "/test_0910", GTUtilsWorkflowDesigner::textValue);

    GTUtilsWorkflowDesigner::connect(os, read, write);
    GTUtilsWorkflowDesigner::runWorkflow(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QDir dir(sandBoxDir);
    CHECK_SET_ERR( dir.entryList(QDir::Files).count() == 2, QString("Incorrect count of sequence files: got %1, expected 2").arg(dir.entryList(QDir::Files).count()));
    foreach (const QString &fileName, dir.entryList(QDir::Files)) {
        CHECK_SET_ERR(fileName.startsWith("test_0910"), "Incorrect result file");
    }
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

GUI_TEST_CLASS_DEFINITION(test_0930){
//    1. Open any *.bam file (e.g. "_common_data/bam/scerevisiae.bam").

    class Scenario : public CustomScenario {
        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "Active modal widget is NULL");

            //    2. Look at the first column in the table above "Source URL" label.
            //    Expected state: you saw "Assembly name" in the colunms head.

            QTableWidget* tableWidget = GTWidget::findExactWidget<QTableWidget*>(os, "tableWidget", dialog);
            QString header = tableWidget->horizontalHeaderItem(0)->text();
            CHECK_SET_ERR(header == "Assembly name", "unexpected header: " + header);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, new Scenario()));
    GTFileDialog::openFile(os, testDir + "_common_data/bam/scerevisiae.bam");
    GTGlobals::sleep();
//    Expected state: the "Import BAM File" dialog appeared.
}


GUI_TEST_CLASS_DEFINITION(test_0935){
//    1. Start the Workflow Designer.
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
//    Expected state: the Eorkflow Designeg opened.

//    2. Add to the scene three elements: any Data Reader (e.g. Read Alignment), any Data Writer(e.g. Write Alignment), any element with input and output (e.g. Find Repeats).
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Read Alignment");
    GTUtilsWorkflowDesigner::addAlgorithm(os, "Write Alignment");
    GTUtilsWorkflowDesigner::addAlgorithm(os, "CD-Search");
//    Expected state: three elements are presented on the scene.

//    3. For each element: select element, see to its properties.
    GTUtilsWorkflowDesigner::click(os, "Read Alignment");
    QGroupBox* out = GTWidget::findExactWidget<QGroupBox*>(os, "outputPortBox");
    CHECK_SET_ERR(out->title() == "Output data", "unexpected out box title: " + out->title());

    GTUtilsWorkflowDesigner::click(os, "Write Alignment", QPoint(-20,-20));
    QGroupBox* in = GTWidget::findExactWidget<QGroupBox*>(os, "inputPortBox");
    CHECK_SET_ERR(in->title() == "Input data", "unexpected in box title: " + in->title());

    GTUtilsWorkflowDesigner::click(os, "CD-Search");
    in = GTWidget::findExactWidget<QGroupBox*>(os, "inputPortBox");
    CHECK_SET_ERR(in->title() == "Input data", "unexpected in box title: " + in->title());
    out = GTWidget::findExactWidget<QGroupBox*>(os, "outputPortBox");
    CHECK_SET_ERR(out->title() == "Output data", "unexpected out box title: " + out->title());
//    Expected state: if element hasn't the input port, there is no "Input data" section in properties.
//            if element hasn't the output port, there is no "Outpu data" section in propeties.
}

GUI_TEST_CLASS_DEFINITION(test_0938) {
//    1. Open any file in assembly view.
//    2. Browse options panel.
//    3. Open "Navigation" tab.
//    Expected state: The tab contains "Enter position in assembly" edit field, "Go!" button and "Most Covered Regions".

    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, sandBoxDir + "chrM.sorted.bam.ugenedb"));
    GTFileDialog::openFile(os, dataDir + "samples/Assembly", "chrM.sorted.bam");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    GTWidget::click(os, GTWidget::findWidget(os, "OP_ASS_NAVIGATION"));
    GTGlobals::sleep();
    GTWidget::click(os, GTWidget::findWidget(os, "OP_ASS_INFO"));
    GTGlobals::sleep();
    GTWidget::click(os, GTWidget::findWidget(os, "OP_ASS_SETTINGS"));
    GTGlobals::sleep();
    GTWidget::click(os, GTWidget::findWidget(os, "OP_ASS_NAVIGATION"));
    GTGlobals::sleep();

    CHECK_SET_ERR(GTWidget::findWidget(os, "go_to_pos_line_edit") != NULL, "go_to_pos_line_edit not found");
    CHECK_SET_ERR(GTWidget::findWidget(os, "Go!") != NULL, "Go! button not found");
    CHECK_SET_ERR(GTWidget::findWidget(os, "COVERED") != NULL, "Covered regions widget not found");
}

GUI_TEST_CLASS_DEFINITION(test_0940) {
    //1. Open samples/CLUSTALW/COI.aln.
    //2. Select any part of the alignment.
    //3. Context menu of the selected part -> Export -> Save subalignment.
    //4. Choose the same output file (samples/CLUSTALW/COI.aln) and press "Extract" button.
    //Expected state: UGENE does not crash.

    GTFile::copy(os, dataDir+"samples/CLUSTALW/COI.aln", sandBoxDir + "test_0940.aln");

    GTFileDialog::openFile(os, sandBoxDir, "test_0940.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Save subalignment"));
    GTUtilsDialog::waitForDialog(os, new ExtractSelectedAsMSADialogFiller(os,
        sandBoxDir + "test_0940.aln",
        GTUtilsMSAEditorSequenceArea::getNameList(os)));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"msa_editor_sequence_area"));
}

namespace {

    QString getFileContent(const QString &path) {
        QFile file(path);
        CHECK(file.open(QFile::ReadOnly), QString("Can not open file"));
        QTextStream fileReader(&file);
        return fileReader.readAll();
    }

}

GUI_TEST_CLASS_DEFINITION(test_0941) {
    //1. Open COI.aln
    //2. Select the first sequence and choose {Edit->Replace slected row with reverse}. Expected state: The sequences is reversed
    //3. Select the second sequence and choose {Edit->Replace slected row with complement}. Expected state: The sequences is complemented
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsMSAEditorSequenceArea::selectSequence(os, "Phaneroptera_falcata");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"msa_editor_sequence_area"));

    GTUtilsMSAEditorSequenceArea::selectSequence(os, "Isophya_altaica_EF540820");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "replace_selected_rows_with_reverse-complement"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"msa_editor_sequence_area"));

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EXPORT << "Save subalignment"));
    GTUtilsDialog::waitForDialog(os, new ExtractSelectedAsMSADialogFiller(os,
        sandBoxDir + "test_0941.aln",
        GTUtilsMSAEditorSequenceArea::getNameList(os)));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"msa_editor_sequence_area"));
    GTGlobals::sleep();

    QString resFileContent = getFileContent(sandBoxDir + "test_0941.aln");
    QString testFileContent = getFileContent(testDir + "_common_data/scenarios/_regression/941/test_0941.aln");
    CHECK_SET_ERR(resFileContent == testFileContent, "Incorrect result file");
}

GUI_TEST_CLASS_DEFINITION(test_0947) {
/*  1. Open "data/samples/ABIF/A01.abi".
 *  3. Open GC Content (%) graph
 *  2. Close chomatogram view
 *    Expected state: GC Content (%) graph view resized
*/
    GTFileDialog::openFile(os, dataDir + "/samples/ABIF/", "A01.abi");
    GTGlobals::sleep();

    QWidget *sequenceWidget = GTWidget::findWidget( os, "ADV_single_sequence_widget_0" );
    CHECK_SET_ERR( NULL != sequenceWidget, "sequenceWidget is not present" );

    GTWidget::click( os, sequenceWidget );

    QWidget *graphAction = GTWidget::findWidget( os, "GraphMenuAction", sequenceWidget, false );
    Runnable *chooser = new PopupChooser( os, QStringList( ) << "GC Content (%)" );
    GTUtilsDialog::waitForDialog( os, chooser );

    GTWidget::click( os, graphAction );
    GTGlobals::sleep();

    QToolButton *zoomAction = GTWidget::findExactWidget<QToolButton *>(os, "action_zoom_in_A1#berezikov");
    CHECK_SET_ERR( NULL != zoomAction, "zoomAction is not present" );

    for(int i = 0; i < 10; i++) {
        GTWidget::click( os, zoomAction);
        GTGlobals::sleep(100);
    }
}

GUI_TEST_CLASS_DEFINITION(test_0948) {

    // 1. Open "open file" dialog
    // 2. Select both files _common_data\scenarios\_regression\948\s1.fa and _common_data\scenarios\_regression\948\s2.fa
    // 3. Press "Open" and select "merge mode"
    // Expected state: warning messagebox about different alphabets has appeared, sequences not merged, but opened in different views
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));

    GTSequenceReadingModeDialog::mode = GTSequenceReadingModeDialog::Merge;
    GTUtilsDialog::waitForDialog(os, new GTSequenceReadingModeDialogUtils(os));
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils_list(os, testDir + "_common_data/scenarios/_regression/948/", QStringList() << "s1.fa" << "s2.fa"));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__OPEN_PROJECT);

    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(500);

}

GUI_TEST_CLASS_DEFINITION(test_0952) {
    //1. Start UGENE.
    //2. Press "Ctrl+O" or "Open" button on the main toolbar.
    //Expected state: "Select files to open..." dialog appears.
    //3. Explore to the directory "data/samples/Genbank", then choose "CVU55762.gb" and "murine.gb" using Ctrl key
    //and press "Open" button.
    //Expected state: "Multiple sequence reading mode" dialog appears.
    //4. Set radio button "Merge sequence mode", set "New document name" if you need, then press "OK".

    GTSequenceReadingModeDialog::mode = GTSequenceReadingModeDialog::Merge;
    GTUtilsDialog::waitForDialog(os, new GTSequenceReadingModeDialogUtils(os));
    GTFileDialog::openFileList(os, dataDir + "samples/Genbank/", QStringList() << "murine.gb" << "sars.gb");

    //Expected state: New merged document appears in project view.
    //5. Open the sequences from the third step in sequence view.
    //Expected state: all the annotations from both sequences were converted correctly to the merged document i.e. without
    //any shifts relatively to a start of sequence.
    GTUtilsAnnotationsTreeView::getItemCenter(os, "3'UTR");
    bool found = GTUtilsAnnotationsTreeView::findRegion(os, "3'UTR", U2Region(35232, 363));
    CHECK_SET_ERR(found, "Wrong annotations shifting");
}

GUI_TEST_CLASS_DEFINITION(test_0958) {
//    1. Create *.csv file with the following content
//    "Name","Start","End","Length","Complementary","Gene","desc","property","prop1","prop2"
//    "test01","1","400","400","no","tEs01","unknown funtion","blablabla","",""
//    "test02","60108","71020","10913","yes","","","","kobietghiginua","addsomethinghere"
//    2. Open data/samples/FASTA/human_T1.fa.
//    3. In the project tree's context menu choose option "Import" > "Import annotations from CSV file".
//    4. In appearing "Import annotations from CSV file" dialog specify the *.csv file you have created,
//    "Result file" in Genbank format. Then in "Results preview" field specify the desirable interpretations
//    for each column accordingly to the first row in the preview table.
//    5. Press "Run".
//    Expected result: annotations have been imported to the sequence with correct locations and qualifiers.

    QFile file(sandBoxDir + "test_0958.csv");
    file.open(QFile::WriteOnly);
    file.write("\"Name\",\"Start\",\"End\",\"Length\",\"Complementary\",\"Gene\",\"desc\",\"property\",\"prop1\",\"prop2\"\n"
                "\"test01\",\"1\",\"400\",\"400\",\"no\",\"tEs01\",\"unknown funtion\",\"blablabla\",\"\",\"\"\n"
                "\"test02\",\"60108\",\"71020\",\"110913\",\"yes\",\"\",\"\",\"\",\"kobietghiginua\",\"addsomethinghere\"\n");
    file.close();

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/human_T1.fa");

    ImportAnnotationsToCsvFiller::RoleParameters r;
    r << ImportAnnotationsToCsvFiller::RoleColumnParameter(0,  new ImportAnnotationsToCsvFiller::NameParameter())
      << ImportAnnotationsToCsvFiller::RoleColumnParameter(1,  new ImportAnnotationsToCsvFiller::StartParameter(false))
      << ImportAnnotationsToCsvFiller::RoleColumnParameter(2,  new ImportAnnotationsToCsvFiller::EndParameter(true))
      << ImportAnnotationsToCsvFiller::RoleColumnParameter(3,  new ImportAnnotationsToCsvFiller::LengthParameter())
      << ImportAnnotationsToCsvFiller::RoleColumnParameter(4,  new ImportAnnotationsToCsvFiller::StrandMarkParameter(true, "yes"))
      << ImportAnnotationsToCsvFiller::RoleColumnParameter(5,  new ImportAnnotationsToCsvFiller::QualifierParameter("Gene"))
      << ImportAnnotationsToCsvFiller::RoleColumnParameter(6,  new ImportAnnotationsToCsvFiller::QualifierParameter("desc"))
      << ImportAnnotationsToCsvFiller::RoleColumnParameter(7,  new ImportAnnotationsToCsvFiller::QualifierParameter("property"))
      << ImportAnnotationsToCsvFiller::RoleColumnParameter(8,  new ImportAnnotationsToCsvFiller::QualifierParameter("prop1"))
      << ImportAnnotationsToCsvFiller::RoleColumnParameter(9,  new ImportAnnotationsToCsvFiller::QualifierParameter("prop2"));

    ImportAnnotationsToCsvFiller* filler = new ImportAnnotationsToCsvFiller(os, sandBoxDir + "test_0958.csv", sandBoxDir + "test_0958.gb",
                                                                            ImportAnnotationsToCsvFiller::Genbank, true, true, ",", false, 1, "#",
                                                                            false, true, "misc_feature", r);
    GTUtilsDialog::waitForDialog(os, filler);
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, QStringList() << "Export/Import" << "Import Annotations from CSV file"));
    GTUtilsProjectTreeView::click(os, "human_T1.fa", Qt::RightButton);
    GTGlobals::sleep();

    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "test01") != NULL, "Annotation item not found");
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getAnnotationRegionString(os, "test01") == "1..400", "Annotation region was improted incorrectly")
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getQualifierValue(os, "Gene", "test01") == "tEs01", "Qualifier Gene was improted incorrectly");
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getQualifierValue(os, "desc", "test01") == "unknown funtion", "Qualifier desc was improted incorrectly");
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getQualifierValue(os, "property", "test01") == "blablabla", "Qualifier property was improted incorrectly");
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getQualifierValue(os, "prop1", "test01") == "", "Qualifier prop1 was improted incorrectly");
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getQualifierValue(os, "prop2", "test01") == "", "Qualifier prop2 was improted incorrectly");

    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findItem(os, "test02") != NULL, "Annotation item not found");
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getAnnotationRegionString(os, "test02") == "complement(60108..71020)", "Annotation region was improted incorrectly")
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getQualifierValue(os, "Gene", "test02") == "",
                  QString("Qualifier Gene was improted incorrectly: got '%1', expected ''").arg(GTUtilsAnnotationsTreeView::getQualifierValue(os, "Gene", "test02")));
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getQualifierValue(os, "desc", "test02") == "", "Qualifier desc was improted incorrectly");
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getQualifierValue(os, "property", "test02") == "", "Qualifier property was improted incorrectly");
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getQualifierValue(os, "prop1", "test02") == "kobietghiginua", "Qualifier prop1 was improted incorrectly");
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::getQualifierValue(os, "prop2", "test02") == "addsomethinghere", "Qualifier prop2 was improted incorrectly");
}

GUI_TEST_CLASS_DEFINITION(test_0965) {
    //1. Open a few files supporting bookmarks.
    //Expected state: in "Bookmarks" area corresponding number of root bookmarks are created.
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/sars.gb");
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/murine.gb");

    //2. Create a few sub bookmarks for each view.
    GTUtilsBookmarksTreeView::addBookmark(os, GTUtilsMdi::activeWindow(os)->objectName(), "murine");
    GTMouseDriver::moveTo(os, GTUtilsBookmarksTreeView::getItemCenter(os, "sars [s] NC_004718"));
    GTMouseDriver::doubleClick(os);
    GTUtilsBookmarksTreeView::addBookmark(os, GTUtilsMdi::activeWindow(os)->objectName(), "sars");

    //3. Press right mouse button on any bookmark connected with currently invisible view.
    //Expected state: "Add bookmark" action is disabled in appeared context menu.
    GTMouseDriver::moveTo(os, GTUtilsBookmarksTreeView::getItemCenter(os, "murine"));
    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList(ACTION_ADD_BOOKMARK), PopupChecker::IsDisabled));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_0967_1) {
/* 1. Open any document
 *   Expected state: Project View showed.
 * 2. Minimize and then restore the main window.
 *   Expected state: Project View should be not hidden.
*/
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTUtilsProject::checkProject(os);
    CHECK_SET_ERR(true == GTUtilsProjectTreeView::isVisible(os), "ProjectTreeView is not visible (check #1)");

    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    mw->showMinimized();
    GTGlobals::sleep();
    mw->showMaximized();
    GTGlobals::sleep();
    GTUtilsProject::checkProject(os);
    CHECK_SET_ERR(true == GTUtilsProjectTreeView::isVisible(os), "ProjectTreeView is not visible (check #2)");
}
GUI_TEST_CLASS_DEFINITION(test_0967_2) {
/* 1. Open Log view
 *   Expected state: Log view showed.
 * 2. Minimize and then restore the main window.
 *   Expected state: Log view should be not hidden.
*/
    GTKeyboardDriver::keyClick(os, '3', GTKeyboardDriver::key["alt"]);
    GTGlobals::sleep();
    QWidget *logView = GTWidget::findWidget(os, "dock_log_view");
    CHECK_SET_ERR(NULL != logView, "Log View is NULL (check #1)");
    CHECK_SET_ERR(true == logView->isVisible(), "Log View is not visible (check #1)");

    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    mw->showMinimized();
    GTGlobals::sleep();
    mw->showMaximized();
    GTGlobals::sleep();
    logView = GTWidget::findWidget(os, "dock_log_view");
    CHECK_SET_ERR(NULL != logView, "Log View is NULL (check #2)");
    CHECK_SET_ERR(true == logView->isVisible(), "Log View is not visible (check #2)");
}
GUI_TEST_CLASS_DEFINITION(test_0967_3) {
/* 1. Open Tasks view
 *   Expected state: Tasks view showed.
 * 2. Minimize and then restore the main window.
 *   Expected state: Tasks view should be not hidden
*/
    GTKeyboardDriver::keyClick(os, '2', GTKeyboardDriver::key["alt"]);
    GTGlobals::sleep();
    QWidget *logView = GTWidget::findWidget(os, "taskViewTree");
    CHECK_SET_ERR(NULL != logView, "taskViewTree is NULL (check #1)");
    CHECK_SET_ERR(true == logView->isVisible(), "taskViewTree is not visible (check #1)");

    QMainWindow* mw = AppContext::getMainWindow()->getQMainWindow();
    mw->showMinimized();
    GTGlobals::sleep();
    mw->showMaximized();
    GTGlobals::sleep();
    logView = GTWidget::findWidget(os, "taskViewTree");
    CHECK_SET_ERR(NULL != logView, "taskViewTree is NULL (check #2)");
    CHECK_SET_ERR(true == logView->isVisible(), "taskViewTree is not visible (check #2)");
}

GUI_TEST_CLASS_DEFINITION(test_0969) {
    // 1. Open any scheme file and do nothing with the opened scene (do not change).
    // 2. Click on 'Load scheme' or 'New scheme' button.
    // Expected state: WD don't asks to save the current scene

    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
    GTUtilsWorkflowDesigner::addSample(os, "call variants");
    GTWidget::click(os, GTAction::button(os, "New workflow action"));
    GTGlobals::sleep();

}

GUI_TEST_CLASS_DEFINITION(test_0970) {

    // 1. Enable Auto Annotations. Open human_T1.fa
    // 2. Open context menu for the Auto annotation in annotation tree view
    // Expected state: "Disable 'annotation' highlighting" item not presents in menu
}

GUI_TEST_CLASS_DEFINITION(test_0981_1) {
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTGlobals::sleep();
    Runnable *filler1 = new InsertSequenceFiller(os,
        "qweqwea", InsertSequenceFiller::Resize, 1, "", InsertSequenceFiller::FASTA, false, false, GTGlobals::UseMouse, true
        );
    GTUtilsDialog::waitForDialog(os, filler1);
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_ACTIONS), QStringList() <<  ADV_MENU_EDIT << ACTION_EDIT_INSERT_SUBSEQUENCE, GTGlobals::UseKey);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0981_2) {
    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 1, 2));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Select" << "Sequence region"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(1000);

    GTGlobals::sleep();
    Runnable *filler1 = new ReplaceSubsequenceDialogFiller(os,
        "qweqwea"
        );
    GTUtilsDialog::waitForDialog(os, filler1);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<< ADV_MENU_EDIT << ACTION_EDIT_REPLACE_SUBSEQUENCE,GTGlobals::UseMouse));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));
    GTGlobals::sleep();
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

GUI_TEST_CLASS_DEFINITION(test_0994) {
/* 1. _common_data/scenarios/regression/994/musMusc.gb
 * 2. Expand contigs_snp group.
 * 3. Call a tooltip of he first annotation in the group.
 *   Expected state: UGENE not crashes
*/
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/_regression/994/", "musMusc.gb");
    GTGlobals::sleep(1000);
    GTMouseDriver::moveTo(os,GTUtilsAnnotationsTreeView::getItemCenter(os,"106-c1_38ftp"));
    GTGlobals::sleep(1000);
}

}

}
