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

#include <U2View/MSAGraphOverview.h>

#include "GTTestsRegressionScenarios_4001_5000.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsMsaEditor.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsOptionPanelMSA.h"
#include "GTUtilsOptionPanelSequenceView.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsWorkflowDesigner.h"
#include "GTUtilsDialog.h"

#include "api/GTAction.h"
#include "api/GTClipboard.h"
#include "api/GTFile.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTTextEdit.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"

#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/ImportBAMFileDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/ugene/corelibs/U2Gui/CreateObjectRelationDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ExportDocumentDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/RangeSelectionDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/DeleteGapsDialogFiller.h"
#include "runnables/ugene/ugeneui/DocumentFormatSelectorDialogFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include "runnables/ugene/plugins/pcr/PrimersDetailsDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSequences2MSADialogFiller.h"

#include <U2View/ADVConstants.h>

namespace U2 {

namespace GUITest_regression_scenarios {


GUI_TEST_CLASS_DEFINITION(test_4007) {
    GTLogTracer l;
    //    1. Open file {data/samples/Genbank/murine.gb}
    QDir().mkpath(sandBoxDir + "test_4007");
    GTFile::copy(os, dataDir + "samples/Genbank/murine.gb", sandBoxDir + "test_4007/murine.gb");
    GTFileDialog::openFile(os, sandBoxDir + "test_4007", "murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //    2. Open file {data/samples/FASTA/human_T1.fa}
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //    3. Drag and drop annotations object to the human_T1 sequence.
    GTUtilsDialog::waitForDialog(os, new CreateObjectRelationDialogFiller(os));
    GTUtilsProjectTreeView::dragAndDrop(os, GTUtilsProjectTreeView::findIndex(os, "NC_001363 features"), GTUtilsAnnotationsTreeView::getTreeWidget(os));


    //    4. Edit "murine.gb" file with an external editor.
    //    Expected state: UGENE offers to reload the changed file.
    //    5. Agree to reload the file.
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Yes));

    QFile murineFile(sandBoxDir + "test_4007/murine.gb");
    const bool opened = murineFile.open(QFile::ReadWrite);
    CHECK_SET_ERR(opened, "Can't open the file: " + sandBoxDir + "test_4007/murine.gb");
    murineFile.write("L");
    murineFile.close();

    GTGlobals::sleep(5000);


    GTGlobals::FindOptions murineOptions(false);
    CHECK_SET_ERR(GTUtilsAnnotationsTreeView::findFirstAnnotation(os, murineOptions) == NULL, "Annotations are connected to murine.gb");

    //    Expected state: the file is reloaded, annotations object still have an association only with human_T1 sequence (if annotations object exists and has the same name as before reloading),
    //there is no errors in the log.
    GTUtilsProjectTreeView::doubleClickItem(os, "human_T1.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(5000);
    GTUtilsAnnotationsTreeView::findFirstAnnotation(os);

    CHECK_SET_ERR(!l.hasError(), "There is error in the log");
}

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

GUI_TEST_CLASS_DEFINITION(test_4009) {
    //1. Open file "_common_data/clustal/big.aln"
    GTFileDialog::openFile(os, testDir + "_common_data/clustal/big.aln");
    GTGlobals::sleep();

    //2. Open "Export Consensus" OP tab
    GTUtilsOptionPanelMsa::openTab(os, GTUtilsOptionPanelMsa::ExportConsensus);

    //3. Press "Export" button
    GTWidget::click(os, GTWidget::findWidget(os, "exportBtn"));

    //4. Remove "big.aln" document
    GTUtilsDocument::removeDocument(os, "big.aln");

    //Current state: the task hangs, debug error occured with message "Infinite wait has timed out"
    class Scenario : public CustomScenario {
    public:
        void run(U2OpStatus &os) {
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);
        }
    };
    GTUtilsDialog::waitForDialog(os, new DocumentFormatSelectorDialogFiller(os, new Scenario()));
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_4011){
    GTLogTracer l;
//    1. Open WD
    GTUtilsWorkflowDesigner::openWorkflowDesigner(os);
//    2. Open sample "Align sequences with MUSCLE"
    GTUtilsWorkflowDesigner::addSample(os, "Align sequences with MUSCLE");
//    3. Align attached file
    GTUtilsWorkflowDesigner::click(os, "Read alignment");
    GTUtilsWorkflowDesigner::setDatasetInputFile(os, testDir + "_common_data/regression/4011", "human_T1.aln");
    GTUtilsWorkflowDesigner::runWorkflow(os);
//    Current state:
//    Runtime error occured(x86 version of UGENE)
//    Windows hangs(x64 version)
    l.checkMessage("Nothing to write");
}

GUI_TEST_CLASS_DEFINITION(test_4010) {
//    1. Open "samples/FASTA/human_T1.fa".
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

//    2. Open the PCR OP tab.
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::InSilicoPcr);

//    3. Enter the forward primer: AAGGAAAAAATGCT.
    GTUtilsOptionPanelSequenceView::setForwardPrimer(os, "AAGGAAAAAATGCT");

//    4. Enter the reverse primer: AGCATTTTTTCCTT.
    GTUtilsOptionPanelSequenceView::setReversePrimer(os, "AGCATTTTTTCCTT");

//    5. Click the Primers Details dialog.
//    Expected: the primers are whole dimers, 14 red lines.
    class Scenario : public CustomScenario {
        void run(U2::U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "Active modal widget is NULL");

            GTTextEdit::containsString(os, GTWidget::findExactWidget<QTextEdit *>(os, "textEdit"), "||||||||||||||");
            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }
    };

    GTUtilsDialog::waitForDialog(os, new PrimersDetailsDialogFiller(os, new Scenario));
    GTUtilsOptionPanelSequenceView::showPrimersDetails(os);
}

GUI_TEST_CLASS_DEFINITION(test_4013) {
    GTFileDialog::openFile(os, dataDir+"samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    int length = GTUtilsMSAEditorSequenceArea::getLength(os);

    GTUtilsMSAEditorSequenceArea::scrollToPosition(os, QPoint(length - 1, 1));
    int columnsNumber = GTUtilsMSAEditorSequenceArea::getNumVisibleBases(os);
    GTUtilsMSAEditorSequenceArea::selectArea(os, QPoint(columnsNumber - 10, 0), QPoint(columnsNumber, 10));

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);

    GTGlobals::sleep();
    QRect oldRect = GTUtilsMSAEditorSequenceArea::getSelectedRect(os);

    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTKeyboardDriver::keySequence(os, "ACCCTATTTTATACCAACAAACTare");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTUtilsMSAEditorSequenceArea::checkSelectedRect(os, oldRect);
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

GUI_TEST_CLASS_DEFINITION(test_4033) {
    class Scenario : public CustomScenario {
    public:
        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(NULL != dialog, "Active modal widget is NULL");

            QWidget *cbExistingTable = GTWidget::findWidget(os, "cbExistingTable", dialog);
            QWidget *tbBrowseExistingTable = GTWidget::findWidget(os, "tbBrowseExistingTable", dialog);
            QWidget *leNewTablePath = GTWidget::findWidget(os, "leNewTablePath", dialog);
            QWidget *tbBrowseNewTable = GTWidget::findWidget(os, "tbBrowseNewTable", dialog);

            GTWidget::click(os, GTWidget::findWidget(os, "rbExistingTable", dialog));
            CHECK_SET_ERR(cbExistingTable->isEnabled() && tbBrowseExistingTable->isEnabled(), "Create annotation dialog controls are disabled unexpectedly");
            CHECK_SET_ERR(!leNewTablePath->isEnabled() && !tbBrowseNewTable->isEnabled(), "Create annotation dialog controls are enabled unexpectedly");

            GTWidget::click(os, GTWidget::findWidget(os, "rbCreateNewTable", dialog));
            CHECK_SET_ERR(!cbExistingTable->isEnabled() && !tbBrowseExistingTable->isEnabled(), "Create annotation dialog controls are enabled unexpectedly");
            CHECK_SET_ERR(leNewTablePath->isEnabled() && tbBrowseNewTable->isEnabled(), "Create annotation dialog controls are disabled unexpectedly");

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        }
    };

    //1. Open "samples/Genbank/murine.gb".
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/murine.gb");

    //2. Ctrl + N.
    //Expected: if the "Existing table" radio button is not checked, then the URL line edit is disabled.
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, new Scenario));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_4034) {
    //1. Open "samples/Genbank/murine.gb".
    //2. Ctrl + N.
    //There are two check boxes for switching the location type. Usually, when you choose between several actions (switch them), GUI shows you radio buttons, not check boxes.
    //Check boxes are used to switch on/off an option but not to choose between options. In this dialog, you even can't switch off the check box when you click it.
    //It is a wrong behavior for this graphic primitive
    //Solution: replace the check boxes with radio buttons.
    GTFileDialog::openFile(os, dataDir+"samples/Genbank/", "murine.gb");

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "exon", "annotation", "200..300",
        sandBoxDir + "ann_test_4034.gb"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTWidget::click(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"), Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_4036) {
//     1. Open "_common_data/clustal/gap_column.aln".
//     2. MSA sequence area context menu -> Edit -> Remove columns of gaps.
//     3. Choose "Remove all gap-only columns".
//     4. Click "Remove".
//     UGENE 1.16-dev: it take ~15 minutes to remove gaps.
//     UGENE 1.15.1: it takes ~5 seconds to remove gaps.

    GTFileDialog::openFile(os, testDir + "_common_data/clustal/", "gap_column.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << MSAE_MENU_EDIT << "remove_columns_of_gaps"));
    GTUtilsDialog::waitForDialog(os, new DeleteGapsDialogFiller(os, 1));

    GTUtilsMSAEditorSequenceArea::callContextMenu(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);
}

GUI_TEST_CLASS_DEFINITION(test_4045) {
    //1. Open "murine.gb"
    //2. Find ORFs
    //3. Unload "murine.gb"
    //4. Open "murine.gb"
    //Current state: SAFE_POINT is triggered
    GTLogTracer logTracer;
    GTFileDialog::openFile(os, dataDir+"samples/Genbank/", "murine.gb");

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

    GTUtilsDialog::waitForDialog(os, new OkClicker(os));
    GTWidget::click(os, GTAction::button(os, "Find ORFs"));
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDocument::unloadDocument(os, "murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsDocument::loadDocument(os, "murine.gb");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsLog::check(os, logTracer);
}

GUI_TEST_CLASS_DEFINITION(test_4046){
//    Open 'human_T1.fa'
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/human_T1.fa");
//    Create a new annotation
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, false, "<auto>", "ann1", "complement(1.. 20)"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep();
//    Current state: new annotation has an empty qualifier 'note'
    QTreeWidgetItem* ann = GTUtilsAnnotationsTreeView::findItem(os, "ann1");
//    Expected state: new annotation does not have any qualifiers
    CHECK_SET_ERR(ann->childCount() == 0, QString("Unexpected qualifier number: %1").arg(ann->childCount()));
}

GUI_TEST_CLASS_DEFINITION(test_4059) {
    GTLogTracer l;
    //1. Open "_common_data/text/text.txt".
    GTUtilsDialog::waitForDialog(os, new DocumentFormatSelectorDialogFiller(os, "Plain text"));
    GTFileDialog::openFile(os, testDir + "_common_data/text/text.txt");

    //2. Delete the "Text" object.
    GTUtilsProjectTreeView::click(os, "text");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    //3. Unload the document.
    MessageBoxDialogFiller *filler = new MessageBoxDialogFiller(os, "No");
    GTUtilsDialog::waitForDialog(os, filler);
    GTUtilsDocument::unloadDocument(os, "text.txt", false);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //4. Load the document.
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "text.txt"));
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);

    //Expected: no safe points triggered.
    CHECK_SET_ERR(!l.hasError(), "Errors in log");
}

GUI_TEST_CLASS_DEFINITION(test_4064) {
//    1. Copy "_common_data/bam/scerevisiae.bam" and "_common_data/bam/scerevisiae.bam.bai" to separate folder
//    2. Rename "scerevisiae.bam.bai" to "scerevisiae.bai"
//    3. Open "scerevisiae.bam" in UGENE
//    Expected state: "Import BAM file" dialog appeared - there is no "Index is not available" warning message.

    GTFile::copy(os, testDir + "_common_data/bam/scerevisiae.bam", sandBoxDir + "test_4064.bam");

    class CustomImportBAMDialogFiller : public Filler {
    public:
        CustomImportBAMDialogFiller(U2OpStatus &os, bool warningExistence)
            : Filler(os, "Import BAM File"),
              warningExistence(warningExistence) {}
        virtual void run() {
            QWidget* dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog, "activeModalWidget is NULL");

            QLabel *label = qobject_cast<QLabel*>(GTWidget::findWidget(os,"indexNotAvailableLabel",dialog));
            CHECK_SET_ERR(label != NULL, "indexNotAvailableLabel not found");
            CHECK_SET_ERR(label->isVisible() == warningExistence, "Warning message is shown");

            QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
            QPushButton* button = box->button(QDialogButtonBox::Cancel);
            CHECK_SET_ERR(button !=NULL, "cancel button is NULL");
            GTWidget::click(os, button);
        }
    private:
        bool warningExistence;
    };

    GTUtilsDialog::waitForDialog(os, new CustomImportBAMDialogFiller(os, true));
    GTFileDialog::openFile(os, sandBoxDir, "test_4064.bam");

    GTFile::copy(os, testDir + "_common_data/bam/scerevisiae.bam.bai", sandBoxDir + "test_4064.bai");
    GTUtilsDialog::waitForDialog(os, new CustomImportBAMDialogFiller(os, false));
    GTFileDialog::openFile(os, sandBoxDir, "test_4064.bam");
}

GUI_TEST_CLASS_DEFINITION(test_4065) {
/* 1. Open _common_data/scenarios/_regression/4065/example_bam.bam
 * 2. Check log for error: "No bam index given, preparing sequential import"
*/
    GTLogTracer l;
    GTUtilsDialog::waitForDialog(os, new ImportBAMFileFiller(os, sandBoxDir + "/test_4065.ugenedb"));
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/_regression/4065/example_bam.bam");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    bool hasMessage = l.checkMessage("No bam index given");
    CHECK_SET_ERR(false == hasMessage , "Error message is found. Bam index file not found.");
}

GUI_TEST_CLASS_DEFINITION(test_4070) {
//    1. Open file "_common_data/scenarios/msa/ma.aln".
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/msa", "ma.aln");

//    2. Switch on collapsing mode.
    GTUtilsMsaEditor::toggleCollapsingMode(os);

//    3. Expand "Conocephalus_discolor" group.
    GTUtilsMsaEditor::toggleCollapsingGroup(os, "Conocephalus_discolor");

//    Expected state: the overview is calculated and shown.
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QWidget *graphOverview = GTUtilsMsaEditor::getGraphOverview(os);
    CHECK_SET_ERR(NULL != graphOverview, "Graph overview widget is NULL");

    bool colorFound = false;
    for (int i = 0; i < graphOverview->width() && !colorFound; i++) {
        for (int j = 0; j < graphOverview->height() && !colorFound; j++) {
            if (QColor(0, 0, 0) == GTUtilsMsaEditor::getGraphOverviewPixelColor(os, QPoint(i, j))) {
                colorFound = true;
            }
        }
    }

    CHECK_SET_ERR(colorFound, "The overview doesn't contain white color");
}

GUI_TEST_CLASS_DEFINITION(test_4072) {
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/COI.aln");

    QWidget *hSeqScroll = GTWidget::findWidget(os, "horizontal_sequence_scroll");
    CHECK_SET_ERR(hSeqScroll != NULL, "No scroll bar at the bottom of sequence area");
    CHECK_SET_ERR(hSeqScroll->isVisible(), "Scroll bat at the bottom of sequence area is invisible");

    QWidget *vSeqScroll = GTWidget::findWidget(os, "vertical_sequence_scroll");
    CHECK_SET_ERR(vSeqScroll != NULL, "No scroll bar at the bottom of sequence area");
    CHECK_SET_ERR(!vSeqScroll->isVisible(), "Scroll bat at the rigth side of sequence area is visible");

    QWidget* hNameScroll = GTWidget::findWidget(os, "horizontal_names_scroll");
    CHECK_SET_ERR(hNameScroll != NULL, "No scroll bar at the bottom of name list area");
    CHECK_SET_ERR(!hNameScroll->isVisible(), "Scroll bar at the botton of name list area is visible");

    QSplitter* splitter = qobject_cast<QSplitter*>(GTWidget::findWidget(os, "msa_editor_horizontal_splitter"));
    CHECK_SET_ERR(splitter != NULL, "MSA Splitter not found");
    QSplitterHandle* handle = splitter->handle(1);
    CHECK_SET_ERR(handle != NULL, "MSA Splitter handle is NULL");

    QWidget* nameList = GTWidget::findWidget(os, "msa_editor_name_list");
    CHECK_SET_ERR(nameList != NULL, "MSA Editor name list not found");

    GTWidget::click(os, handle);
    QPoint p = GTMouseDriver::getMousePosition();
    p.setX( p.x() - 2*nameList->width()/3);
    GTMouseDriver::press(os);
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::release(os);

    CHECK_SET_ERR(hNameScroll->isVisible(), "Scroll bar at the botton of name list area is invisible");


    GTFileDialog::openFile(os, testDir + "_common_data/clustal/fungal - all.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    hNameScroll = GTWidget::findWidget(os, "horizontal_names_scroll");
    CHECK_SET_ERR(hNameScroll != NULL, "No scroll bar at the bottom of name list area");
    CHECK_SET_ERR(hNameScroll->isVisible(), "Scroll bar at the botton of name list area is visible");
}

GUI_TEST_CLASS_DEFINITION(test_4084) {
    //1. Open "_common_data/fasta/human_T1_cutted.fa".
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/human_T1_cutted.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //2. Create any annotation.
    //Expected state: there is an annotation group with an annotation within.
    //Current state: there is an annotation group with two similar annotations within.
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "group", "feature", "50..60"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "create_annotation_action"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep();

    QTreeWidgetItem *annotationGroup = GTUtilsAnnotationsTreeView::findItem(os, "group  (0, 1)");
    CHECK_SET_ERR(NULL != annotationGroup, "Wrong annotations number");
}

GUI_TEST_CLASS_DEFINITION(test_4095) {
/* 1. Open file "test/_common_data/fasta/fa1.fa"
 * 2. Call context menu on the sequence view { Edit sequence -> Reverse sequence }
 *   Expected state: nucleotides order has reversed
 *   Current state: nothing happens
*/
    GTFileDialog::openFile(os, testDir + "_common_data/fasta", "fa1.fa");
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<ADV_MENU_EDIT<<ACTION_EDIT_RESERVE_SEQUENCE));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os, "ADV_single_sequence_widget_0"));
    GTUtilsTaskTreeView::waitTaskFinished(os);
    //GTCA
    GTUtilsDialog::waitForDialog(os, new selectSequenceRegionDialogFiller(os, 1, 4));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Select" << "Sequence region"));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"ADV_single_sequence_widget_0"));
    GTKeyboardDriver::keyClick( os, 'c', GTKeyboardDriver::key["ctrl"] );
    GTGlobals::sleep(200);
    const QString selectionContent = GTClipboard::text( os );
    CHECK_SET_ERR( "GTCA" == selectionContent, "Sequence reversing is failed" );
}

namespace {
    QString getFileContent(const QString &path) {
        QFile file(path);
        CHECK(file.open(QFile::ReadOnly), QString());
        QTextStream fileReader(&file);
        return fileReader.readAll();
    }

}
GUI_TEST_CLASS_DEFINITION(test_4096) {
    // 1. Open "human_T1.fa"
    // 2. Use context menu on sequence object
    // {Export/Import->Export sequences as alignment}
    // Expected state: "Export Sequences as Alignment" dialog appeared
    // 3. Press "Export"
    // Current state: only part of human_T1(128000 nb) exported to alignment

    class ExportSeqsAsMsaScenario : public CustomScenario {
        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            CHECK_SET_ERR(dialog != NULL, "dialog not found");

            QCheckBox *addToProjectBox = qobject_cast<QCheckBox *>(GTWidget::findWidget(os, "addToProjectBox", dialog));
            CHECK_SET_ERR(addToProjectBox->isChecked(), "'Add document to project' checkbox is not set");

            QLineEdit *lineEdit = qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "fileNameEdit", dialog));
            GTLineEdit::setText(os, lineEdit, sandBoxDir + "test_4096.aln");

            GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
        }
    };

    //GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT));
    GTUtilsDialog::waitForDialog(os, new ExportSequenceAsAlignmentFiller(os, new ExportSeqsAsMsaScenario));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "human_T1.fa"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsProjectTreeView::checkItem(os, "test_4096.aln");

    const QString referenceMsaContent = getFileContent(testDir + "_common_data/clustal/test_4096.aln");
    const QString resultMsaContent = getFileContent(sandBoxDir + "test_4096.aln");
    CHECK_SET_ERR(!referenceMsaContent.isEmpty() && referenceMsaContent == resultMsaContent, "Unexpected MSA content");

}
GUI_TEST_CLASS_DEFINITION(test_4097) {
/* 1. Open "_common_data/vector_nti_sequence/unrefined.gb".
 * 2. Export the document somewhere to the vector NTI sequence format.
 *   Expected state: the saved file contains only 8 entries in the COMMENT section and doesn't contain "Vector_NTI_Display_Data_(Do_Not_Edit!)" comment.
*/
    GTFileDialog::openFile(os, testDir + "_common_data/vector_nti_sequence", "unrefined.gb");

    GTUtilsDialog::waitForDialog(os, new ExportDocumentDialogFiller(os, sandBoxDir, "test_4097.gb", ExportDocumentDialogFiller::VectorNTI, false, false));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Export document"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "unrefined.gb"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    const QString resultFileContent = getFileContent(sandBoxDir + "test_4097.gb");
    CHECK_SET_ERR(false == resultFileContent.contains("Vector_NTI_Display_Data_(Do_Not_Edit!)",Qt::CaseInsensitive), "Unexpected file content");
    QRegExp rx("COMMENT");
    int pos = 0;
    int count = 0;
    while (pos >= 0) {
        pos = rx.indexIn(resultFileContent, pos);
        if (pos >= 0) {
            ++pos;
            ++count;
        }
    }
    CHECK_SET_ERR(8 == count, "The saved file contains more/less then 8 entries in the COMMENT section");
}
GUI_TEST_CLASS_DEFINITION(test_4099) {
/* 1. Open file _common_data/scenarios/_regression/4099/p4228.gb
 * 2. Select CDS annotation 1656..2450 and select 'label' item
 *   Expected state: Value has "Tn5 neomycin resistance"
 *   Note: The value of the "label" qualifier is displayed both on the circular view and the zoom view as annotation names.
*/
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/_regression/4099", "p4228.gb");
    QList<QTreeWidgetItem*> items = GTUtilsAnnotationsTreeView::findItems(os,"CDS");
    CHECK_SET_ERR( 2 == items.length(), "CDS annotations count is not 2");
    foreach (QTreeWidgetItem* item, items) {
        if (item->text(2)=="1656..2450")
        {
            CHECK_SET_ERR("Tn5 neomycin resistance" == GTUtilsAnnotationsTreeView::getQualifierValue(os,"label",item), "Wrong label value");
            break;
        }
    }
}
GUI_TEST_CLASS_DEFINITION(test_4122) {
/* 1. Open "data/samples/Genbank/murine.gb".
 * 2. Search any existing pattern.
 * 3. Try to create annotations from the search results to the existing annotations table.
 *   Expected state: annotations are created.
 *   Current state: annotations are not created, a safe point is triggered
*/
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "murine.gb");
    GTUtilsOptionPanelSequenceView::openTab(os, GTUtilsOptionPanelSequenceView::Search);
    GTUtilsOptionPanelSequenceView::enterPattern(os, "GAGTTCTGAACACCCGGC");
    GTUtilsOptionPanelSequenceView::clickGetAnnotation(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QTreeWidgetItem* annotationTable = GTUtilsAnnotationsTreeView::findItem(os, "NC_001363 features [murine.gb] *");
    CHECK_SET_ERR(NULL != annotationTable, "AnnotationTable (NC_001363 features [murine.gb]) is NULL or not changed");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "NC_001363 features [murine.gb] *"));
    GTMouseDriver::click(os);

    QTreeWidgetItem* newItem = GTUtilsAnnotationsTreeView::findItem(os, "Misc. Feature",annotationTable);
    CHECK_SET_ERR(NULL != newItem, "New annotation is NULL or not created");
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "Misc. Feature"));
    GTMouseDriver::click(os);

   }

} // namespace GUITest_regression_scenarios

} // namespace U2
