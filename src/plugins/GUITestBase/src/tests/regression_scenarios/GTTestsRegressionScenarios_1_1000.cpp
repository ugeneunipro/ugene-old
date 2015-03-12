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

GUI_TEST_CLASS_DEFINITION(test_0910) {
//    1. Create a scheme with the "Read sequence" element and the "Write sequence element".
//    2. Take a file with more then one sequence as an input for the "Read sequence" element.
//    3. Set the "Accumulate objects" parameters to False
//    4. Set the "Existing file" parameter to "Rename"
//    5. Run the scheme
//    6. The number of output files must be equel to the number of input sequences

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

GUI_TEST_CLASS_DEFINITION(test_994) {
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
