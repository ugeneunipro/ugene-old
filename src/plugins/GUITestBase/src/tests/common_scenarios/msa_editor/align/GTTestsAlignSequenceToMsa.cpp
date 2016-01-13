/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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
#include "GTTestsAlignSequenceToMsa.h"
#include <drivers/GTMouseDriver.h>
#include <drivers/GTKeyboardDriver.h>
#include "utils/GTKeyboardUtils.h"
#include <primitives/GTWidget.h>
#include <base_dialogs/GTFileDialog.h>
#include "primitives/GTMenu.h"
#include <primitives/GTTreeWidget.h>
#include <primitives/GTSpinBox.h>
#include "GTGlobals.h"
#include "system/GTClipboard.h"
#include "primitives/GTAction.h"
#include <primitives/GTTreeWidget.h>
#include <primitives/GTToolbar.h>
#include <primitives/GTComboBox.h>
#include <primitives/GTLineEdit.h>
#include <primitives/GTCheckBox.h>
#include "utils/GTUtilsDialog.h"
#include "GTUtilsExternalTools.h"
#include "GTUtilsLog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMsaEditor.h"
#include "GTUtilsMsaEditorSequenceArea.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "primitives/PopupChooser.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/DeleteGapsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/PositionSelectorFiller.h"
#include "runnables/ugene/corelibs/U2Gui/util/RenameSequenceFiller.h"
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorSequenceArea.h>

namespace U2 {
namespace GUITest_common_scenarios_align_sequences_to_msa{
using namespace HI;

void checkAlignedRegion(HI::GUITestOpStatus& os, QRect selectionRect, const QString& expectedContent) {
    GTUtilsDialog::waitForDialog(os, new GoToDialogFiller(os, selectionRect.center().x()));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_go_to_position"));
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));

    int leftOffset = GTUtilsMSAEditorSequenceArea::getLeftOffset(os) - 1;

    selectionRect.adjust(-leftOffset, 0, -leftOffset, 0);

    GTUtilsMSAEditorSequenceArea::selectArea(os, selectionRect.topLeft(), selectionRect.bottomRight());
    GTKeyboardDriver::keyClick(os, 'c',GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(500);
    QString clipboardTest = GTClipboard::text(os);

    CHECK_SET_ERR(clipboardTest== expectedContent, QString("Incorrect alignment of the region\n Expected: \n%1 \nResult: \n%2").arg(expectedContent).arg(clipboardTest));
}

GUI_TEST_CLASS_DEFINITION(test_0001){
    //Try to delete the MSA object during aligning
    //Expected state: the sequences are locked and and can not be deleted
    GTLogTracer logTracer;
    GTFileDialog::openFile(os, testDir + "_common_data/clustal/", "3000_sequences.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTFileDialog::openFile(os, testDir + "_common_data/alignment/align_sequence_to_an_alignment/", "tub1.txt");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsProjectTreeView::click(os, "tub");
    GTUtilsMdi::activateWindow(os, "3000_sequences [m] 3000_sequences");

    QAbstractButton *align = GTAction::button( os, "Align sequence to this alignment" );
    CHECK_SET_ERR(align != NULL, "MSA \"Align sequence to this alignment\" action not found");
    GTWidget::click( os, align);

    GTUtilsProjectTreeView::click(os, "tub1.txt");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

    const bool hasMessage = logTracer.checkMessage("Cannot remove document tub1.txt");
    CHECK_SET_ERR(hasMessage, "The expected message is not found in the log");

    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR(GTUtilsMsaEditor::getSequencesCount(os) == 3086, "Incorrect sequences count");
}

GUI_TEST_CLASS_DEFINITION(test_0002){
    //Try to delete the MSA object during aligning
    //Expected state: the MSA object is locked and and can not be deleted
    GTLogTracer logTracer;
    GTFileDialog::openFile(os, testDir + "_common_data/clustal/", "3000_sequences.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTFileDialog::openFile(os, testDir + "_common_data/alignment/align_sequence_to_an_alignment/", "tub1.txt");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsProjectTreeView::click(os, "tub");
    GTUtilsMdi::activateWindow(os, "3000_sequences [m] 3000_sequences");

    QAbstractButton *align = GTAction::button( os, "Align sequence to this alignment" );
    CHECK_SET_ERR(align != NULL, "MSA \"Align sequence to this alignment\" action not found");
    GTWidget::click( os, align);

    GTUtilsProjectTreeView::click(os, "3000_sequences.aln");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep();

    const bool hasMessage = logTracer.checkMessage("Cannot remove document 3000_sequences.aln");
    CHECK_SET_ERR(hasMessage, "The expected message is not found in the log");

    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR(GTUtilsMsaEditor::getSequencesCount(os) == 3086, "Incorrect sequences count");
}

GUI_TEST_CLASS_DEFINITION(test_0003){
    //Align short sequences with default settings(on platforms with MAFFT)
    //Expected state: MAFFT alignment started and finished succesfully with using option --addfragments
    GTLogTracer logTracer;
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, testDir + "_common_data/cmdline/primers/", "primers.fa");
    GTUtilsDialog::waitForDialog(os, ob);

    QAbstractButton *align = GTAction::button( os, "Align sequence to this alignment" );
    CHECK_SET_ERR(align != NULL, "MSA \"Align sequence to this alignment\" action not found");
    GTWidget::click( os, align);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR(GTUtilsMsaEditor::getSequencesCount(os) == 152, "Incorrect sequences count");

    const bool hasMessage = logTracer.checkMessage("--addfragments");
    CHECK_SET_ERR(hasMessage, "The expected message is not found in the log");

    checkAlignedRegion(os, QRect(QPoint(86,17), QPoint(114,23)),
        QString("CATGCCTTTGTAATAATCTTCTTTATAGT\n"
                "-----------------------------\n"
                "-----------------------------\n"
                "CTATCCTTCGCAAGACCCTTC--------\n"
                "-----------------------------\n"
                "-----------------------------\n"
                "---------ATAATACCGCGCCACATAGC"));
}


GUI_TEST_CLASS_DEFINITION(test_0004){
    //Remove MAFFT from external tools, then align short sequences
    //Expected state: UGENE alignment started and finished succesfully
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsExternalTools::removeTool(os, "MAFFT");

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, testDir + "_common_data/cmdline/primers/", "primers.fa");
    GTUtilsDialog::waitForDialog(os, ob);

    QAbstractButton *align = GTAction::button( os, "Align sequence to this alignment" );
    CHECK_SET_ERR(align != NULL, "MSA \"Align sequence to this alignment\" action not found");
    GTWidget::click( os, align);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR(GTUtilsMsaEditor::getSequencesCount(os) == 152, "Incorrect sequences count");

    checkAlignedRegion(os, QRect(QPoint(51, 17), QPoint(71, 19)),
        QString("GTGATAGTCAAATCTATAATG\n"
                "---------------------\n"
                "GACTGGTTCCAATTGACAAGC"));
}

GUI_TEST_CLASS_DEFINITION(test_0005){
    GTFileDialog::openFile(os, testDir + "_common_data/alignment/align_sequence_to_an_alignment/", "TUB.msf");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QStringList fileList;
    fileList << "tub1.txt" << "tub3.txt";
    GTFileDialogUtils_list *ob = new GTFileDialogUtils_list(os, testDir + "_common_data/alignment/align_sequence_to_an_alignment/", fileList);
    GTUtilsDialog::waitForDialog(os, ob);

    QAbstractButton *align = GTAction::button( os, "Align sequence to this alignment" );
    CHECK_SET_ERR(align != NULL, "MSA \"Align sequence to this alignment\" action not found");
    GTWidget::click( os, align);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR(GTUtilsMsaEditor::getSequencesCount(os) == 17, "Incorrect sequences count");

    checkAlignedRegion(os, QRect(QPoint(970, 7), QPoint(985, 15)),
        QString("TTCCCAGGTCAGCTCA\n"
                "----------------\n"
                "----------------\n"
                "----------------\n"
                "----------------\n"
                "----------------\n"
                "----------------\n"
                "----------------\n"
                "TTCCCAGGTCAGCTCA"));

    checkAlignedRegion(os, QRect(QPoint(875, 7), QPoint(889, 16)),
        QString("TCTGCTTCCGTACAC\n"
                "---------------\n"
                "---------------\n"
                "--------CGTACAC\n"
                "---------------\n"
                "---------------\n"
                "---------------\n"
                "---------------\n"
                "---------------\n"
                "TCTGCTTCCGTACAC"));
}

GUI_TEST_CLASS_DEFINITION(test_0006){
    GTFileDialog::openFile(os, testDir + "_common_data/alignment/align_sequence_to_an_alignment/", "TUB.msf");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsExternalTools::removeTool(os, "MAFFT");

    QStringList fileList;
    fileList << "tub1.txt" << "tub3.txt";
    GTFileDialogUtils_list *ob = new GTFileDialogUtils_list(os, testDir + "_common_data/alignment/align_sequence_to_an_alignment/", fileList);
    GTUtilsDialog::waitForDialog(os, ob);

    QAbstractButton *align = GTAction::button( os, "Align sequence to this alignment" );
    CHECK_SET_ERR(align != NULL, "MSA \"Align sequence to this alignment\" action not found");
    GTWidget::click( os, align);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR(GTUtilsMsaEditor::getSequencesCount(os) == 17, "Incorrect sequences count");

    checkAlignedRegion(os, QRect(QPoint(970, 7), QPoint(985, 15)),
        QString("TTCCCAGGTCAGCTCA\n"
        "----------------\n"
        "----------------\n"
        "----------------\n"
        "----------------\n"
        "----------------\n"
        "----------------\n"
        "----------------\n"
        "TTCCCAGGTCAGCTCA"));

    checkAlignedRegion(os, QRect(QPoint(875, 7), QPoint(889, 16)),
        QString("TCTGCTTCCGTACAC\n"
        "---------------\n"
        "---------------\n"
        "--------CGTACAC\n"
        "---------------\n"
        "---------------\n"
        "---------------\n"
        "---------------\n"
        "---------------\n"
        "TCTGCTTCCGTACAC"));
}

GUI_TEST_CLASS_DEFINITION(test_0007){
    //Do not select anything in the project. Click the button. Add a sequence in GenBank format.
    //Expected state: The sequence was added to the alignment and aligned.
    GTLogTracer logTracer;
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, dataDir + "samples/Genbank/", "CVU55762.gb");
    GTUtilsDialog::waitForDialog(os, ob);

    QAbstractButton *align = GTAction::button( os, "Align sequence to this alignment" );
    CHECK_SET_ERR(align != NULL, "MSA \"Align sequence to this alignment\" action not found");
    GTWidget::click( os, align);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR(GTUtilsMsaEditor::getSequencesCount(os) == 19, "Incorrect sequences count");
}

GUI_TEST_CLASS_DEFINITION(test_0008){
    //Do not select anything in the project. Click the button. Add several ABI files.
    //Expected state: The sequences were added to the alignment and aligned
    GTLogTracer logTracer;
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "COI.aln");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QStringList fileList;
    fileList << "39_034.ab1" << "19_022.ab1" << "25_032.ab1";
    GTFileDialogUtils_list *ob = new GTFileDialogUtils_list(os, testDir + "_common_data/abif/", fileList);
    GTUtilsDialog::waitForDialog(os, ob);

    QAbstractButton *align = GTAction::button( os, "Align sequence to this alignment" );
    CHECK_SET_ERR(align != NULL, "MSA \"Align sequence to this alignment\" action not found");
    GTWidget::click( os, align);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR(GTUtilsMsaEditor::getSequencesCount(os) == 21, "Incorrect sequences count");
}

GUI_TEST_CLASS_DEFINITION(test_0009){
    //Do not select anything in the project. Click the button. Add sequences in ClustalW format. Uncheck several sequences in the appeared dialog.
    //Expected state: Only checked sequences were added to the alignment.
    GTLogTracer logTracer;
    GTFileDialog::openFile(os, testDir + "_common_data/alignment/align_sequence_to_an_alignment/", "TUB.msf");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, testDir + "_common_data/clustal/", "COI na.aln");
    GTUtilsDialog::waitForDialog(os, ob);

    QAbstractButton *align = GTAction::button( os, "Align sequence to this alignment" );
    CHECK_SET_ERR(align != NULL, "MSA \"Align sequence to this alignment\" action not found");
    GTWidget::click( os, align);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    CHECK_SET_ERR(GTUtilsMsaEditor::getSequencesCount(os) == 33, "Incorrect sequences count");
}

GUI_TEST_CLASS_DEFINITION(test_0010) {
    //1. Open "_common_data/fasta/empty.fa" as msa.
    //2. Ensure that MAFFT tool is set.
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/empty.fa");
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //3. Click "Align sequence to this alignment" button on the toolbar.
    //4. Select "data/samples/FASTQ/eas.fastq".
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/FASTQ/eas.fastq"));
    GTWidget::click(os, GTAction::button(os, "Align sequence to this alignment"));
    GTUtilsTaskTreeView::waitTaskFinished(os);

    //Expected state: three sequences are added to the msa.
    CHECK_SET_ERR(GTUtilsMsaEditor::getSequencesCount(os) == 3, "Incorrect sequences count");
}

} // namespace
} // namespace U2

