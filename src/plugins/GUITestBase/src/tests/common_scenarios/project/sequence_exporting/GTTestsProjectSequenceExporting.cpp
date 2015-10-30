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

#include <QApplication>
#include <QCheckBox>
#include <QTreeView>

#include "GTTestsProjectSequenceExporting.h"
#include "GTGlobals.h"
#include <base_dialogs/GTFileDialog.h>
#include <drivers/GTKeyboardDriver.h>
#include <primitives/GTComboBox.h>
#include "primitives/GTMenu.h"
#include <drivers/GTMouseDriver.h>
#include "system/GTFile.h"
#include <primitives/GTTreeWidget.h>
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsProject.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "utils/GTUtilsApp.h"
#include "utils/GTUtilsToolTip.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsMdi.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsTaskTreeView.h"
#include "primitives/PopupChooser.h"
#include <base_dialogs/MessageBoxFiller.h>
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/ugene/corelibs/U2Gui/CreateDocumentFromTextDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/CreateObjectRelationDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSequencesDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportAnnotationsDialogFiller.h"

#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>
#include <U2Core/DocumentModel.h>
#include <U2View/ADVConstants.h>
#include <U2Core/AppContext.h>
#include <QPlainTextEdit>

namespace U2 {

namespace GUITest_common_scenarios_project_sequence_exporting {
using namespace HI;

GUI_TEST_CLASS_DEFINITION(test_0001) {
// 1. Use menu {File->Open}. Open project _common_data/scenario/project/proj4.uprj
    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj4.uprj");
    GTGlobals::sleep();

    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsDocument::checkDocument(os, "2.gb");
// Expected state:
//     1) Project view with document "1.gb" and "2.gb" is opened, both documents are unloaded
    Document* doc1 = GTUtilsDocument::getDocument(os, "1.gb");
    Document* doc2 = GTUtilsDocument::getDocument(os, "2.gb");
    CHECK_SET_ERR(doc1 != NULL && doc2 != NULL, "there are no documents 1.gb and 2.gb");

    CHECK_SET_ERR(!doc1->isLoaded(), "1.gb is loaded");
    CHECK_SET_ERR(!doc2->isLoaded(), "2.gb is loaded");
//     2) UGENE window titled with text "proj4 UGENE"
    QString expectedTitle;
#ifdef Q_OS_MAC
    expectedTitle = "proj4 UGENE";
#else
    expectedTitle = "proj4 UGENE - [Start Page]";
#endif
    GTUtilsApp::checkUGENETitle(os, expectedTitle);

// 2. Double click on "[a] Annotations" sequence object, in project view tree
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// Expected result: NC_001363 sequence has been opened in sequence view
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);

// 3. Select region 1..4 at sequence view. Right click to selected region open context menu. Use menu {Export->Export Selected Sequence region}
// Expected state: Export DNA Sequences To FASTA Format dialog open
// 4. Fill the next field in dialog:
// {Export to file:} _common_data/scenarios/sandbox/exp.fasta
// {Add created document to project} set checked

    GTUtilsSequenceView::selectSequenceRegion(os, 1, 4);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_EXPORT" << "action_export_selected_sequence_region", GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new ExportSelectedRegionFiller(os, testDir + "_common_data/scenarios/sandbox/", "exp.fasta"));

    QWidget* activeWindow = GTUtilsMdi::activeWindow(os);
    CHECK_SET_ERR(activeWindow != NULL, "there is no active MDI window");

    QPoint p = activeWindow->mapToGlobal(activeWindow->rect().center());
    GTMouseDriver::moveTo(os, QPoint(p.x(), 200));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: sequence view [1..4] has been opened, with sequence "AAAT"
    GTGlobals::sleep();
    GTUtilsDocument::checkDocument(os, "exp.fasta");

    QString seq;
    GTUtilsSequenceView::getSequenceAsString(os, seq);
    GTGlobals::sleep();

    CHECK_SET_ERR(seq == "AAAT", "exported sequence differs from AAAT");
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
// 1. Use menu {File->Open}. Open project _common_data/scenario/project/proj4.uprj
    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj4.uprj");

// Expected state:
//     1) Project view with document "1.gb" and "2.gb" is opened
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsDocument::checkDocument(os, "2.gb");

// 2. Double click on "[a] Annotations" sequence object, in project view tree
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

//     Expected result: NC_001363 sequence has been opened in sequence view
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);

// 3. Select annotation C. Use context menu item {Export->Export Sequence of Selected Annotations}
// Expected state: Export Sequence of selected annotations will open
//
// 4. Fill the next field in dialog:
//     {Format } FASTA
//     {Export to file:} _common_data/scenarios/sandbox/exp.fasta
//     {Add created document to project} set checked
// 5. Click Export button.

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_EXPORT" << "action_export_sequence_of_selected_annotations", GTGlobals::UseKey));

    Runnable *filler = new ExportSequenceOfSelectedAnnotationsFiller(os,
        testDir + "_common_data/scenarios/sandbox/exp.fasta",
        ExportSequenceOfSelectedAnnotationsFiller::Fasta,
        ExportSequenceOfSelectedAnnotationsFiller::SaveAsSeparate
    );
    GTUtilsDialog::waitForDialog(os, filler);

    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "C"));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: sequence view C has been opened, with sequence "GAATAGAAAAG"
}

GUI_TEST_CLASS_DEFINITION(test_0003)
{
    const QString doc1("1.gb"), doc2("2.gb");

// 1. Use menu {File->Open}. Open project _common_data/scenario/project/proj4.uprj
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj4.uprj");
    GTGlobals::sleep();

// Expected state:
//     1) Project view with document "1.gb" and "2.gb" is opened
    GTUtilsProjectTreeView::findIndex(os, doc1);//checks inside
    GTUtilsProjectTreeView::findIndex(os, doc2);

// 2. Double click on "[a] Annotations" sequence object, in project view tree
    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, "Annotations");
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep(1000);

// Expected result: NC_001363 sequence has been opened in sequence view
    GTUtilsDocument::checkDocument(os, doc1, AnnotatedDNAViewFactory::ID);

// 3. Select joined annotation B. Use context menu item {Export->Export Sequence of Selected Annotations}
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B_joined"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// Expected state: Export Sequence of Selected Annotations
// 4. Fill the next field in dialog:
//     {Format } FASTA
//     {Export to file:} _common_data/scenarios/sandbox/exp.fasta
//     {Add created document to project} set checked
//     {Merge sequnces} set selected
//     {Gap length} 5
// 5. Click Export button.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_EXPORT" << "action_export_sequence_of_selected_annotations", GTGlobals::UseKey));
    Runnable *filler = new ExportSequenceOfSelectedAnnotationsFiller(os,
        testDir + "_common_data/scenarios/sandbox/exp.fasta",
        ExportSequenceOfSelectedAnnotationsFiller::Fasta,
        ExportSequenceOfSelectedAnnotationsFiller::Merge,
        5,
        true
    );
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B_joined"));
    GTMouseDriver::click(os, Qt::RightButton);

// Expected state: sequence view B part 1 of 3 has been opened, with sequence "ACCCCACCCGTAGGTGGCAAGCTAGCTTAAG"
    GTUtilsSequenceView::checkSequence(os, "ACCCCACCCGTAGGTGGCAAGCTAGCTTAAG");
}

GUI_TEST_CLASS_DEFINITION(test_0004) {

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj4.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsDocument::checkDocument(os, "2.gb");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Annotations"));
    GTMouseDriver::doubleClick(os);
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ADV_MENU_EXPORT << ACTION_EXPORT_ANNOTATIONS));
    GTUtilsDialog::waitForDialog(os, new ExportAnnotationsFiller(os, testDir+"_common_data/scenarios/sandbox/1.csv", ExportAnnotationsFiller::csv));
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "B_joined"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep(1000);
    bool equals = GTFile::equals(os, testDir+"_common_data/scenarios/sandbox/1.csv", testDir+"_common_data/scenarios/project/test_0004.csv");
    CHECK_SET_ERR(equals == true, "Exported file differs from the test file");
}
GUI_TEST_CLASS_DEFINITION(test_0005) {
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj4.uprj");

    QModelIndex nc_001363 = GTUtilsProjectTreeView::findIndex(os,"NC_001363 sequence", GTUtilsProjectTreeView::findIndex(os, "1.gb"));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, nc_001363));
    GTMouseDriver::doubleClick(os);
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);
    GTGlobals::sleep(100);

    QTreeWidgetItem* item = GTUtilsAnnotationsTreeView::findItem(os, "C");
    CHECK_SET_ERR(item != NULL, "AnnotationsTreeView is NULL");

    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "C"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_EXPORT" << "action_export_annotations", GTGlobals::UseKey));

    Runnable *filler = new ExportAnnotationsFiller(os,
        testDir+"_common_data/scenarios/sandbox/1.csv",
        ExportAnnotationsFiller::csv,
        true,
        false
    );
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep(1000);
    bool equals = GTFile::equals(os, testDir+"_common_data/scenarios/sandbox/1.csv", testDir+"_common_data/scenarios/project/test_0005.csv");
    CHECK_SET_ERR(equals == true, "Exported file differs from the test file");
}

GUI_TEST_CLASS_DEFINITION(test_0007) {

    Runnable *filler = new CreateDocumentFiller(os,
        "ACGTGTGTGTACGACAGACGACAGCAGACGACAGACAGACAGACAGCAAGAGAGAGAGAG", true,
        CreateDocumentFiller::StandardRNA, true, false, "",
        testDir + "_common_data/scenarios/sandbox/",
        CreateDocumentFiller::Genbank,
        "Sequence", false
    );
    GTUtilsDialog::waitForDialog(os, filler);
    GTMenu::clickMainMenuItem(os, QStringList() << "File" << "New document from text...", GTGlobals::UseKey);
    GTGlobals::sleep();

    GTGlobals::sleep();
    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "misc_feature_group", "misc_feature", "complement(1.. 20)"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTGlobals::sleep();


    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "misc_feature"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_EXPORT" << "action_export_sequence_of_selected_annotations", GTGlobals::UseKey));
    Runnable *filler3 = new ExportSequenceOfSelectedAnnotationsFiller(os,
        testDir + "_common_data/scenarios/sandbox/exp.gb",
        ExportSequenceOfSelectedAnnotationsFiller::Genbank,
        ExportSequenceOfSelectedAnnotationsFiller::SaveAsSeparate,
        0,
        true,
        true
    );
    GTUtilsDialog::waitForDialog(os, filler3);
    GTMouseDriver::moveTo(os, GTUtilsAnnotationsTreeView::getItemCenter(os, "misc_feature"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0008) {
    class CustomExportSelectedRegion : public CustomScenario {
    public:
        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();

            QComboBox *formatCombo = qobject_cast<QComboBox *>(GTWidget::findWidget(os, "formatCombo", dialog));
            QCheckBox *withAnnotationsBox = qobject_cast<QCheckBox *>(GTWidget::findWidget(os, "withAnnotationsBox", dialog));

            CHECK_SET_ERR(!withAnnotationsBox->isEnabled(), "Export with annotations flag is enabled unexpectedly");
            CHECK_SET_ERR(!withAnnotationsBox->isChecked(), "Export with annotations flag is checked unexpectedly");

            GTComboBox::setIndexWithText(os, formatCombo, "Genbank");
            CHECK_SET_ERR(withAnnotationsBox->isEnabled(), "Export with annotations flag is disabled unexpectedly");
            CHECK_SET_ERR(withAnnotationsBox->isChecked(), "Export with annotations flag is unchecked unexpectedly");

            GTComboBox::setIndexWithText(os, formatCombo, "FASTQ");
            CHECK_SET_ERR(!withAnnotationsBox->isEnabled(), "Export with annotations flag is enabled unexpectedly");
            CHECK_SET_ERR(!withAnnotationsBox->isChecked(), "Export with annotations flag is checked unexpectedly");

            GTComboBox::setIndexWithText(os, formatCombo, "GFF");
            CHECK_SET_ERR(withAnnotationsBox->isEnabled(), "Export with annotations flag is disabled unexpectedly");
            CHECK_SET_ERR(withAnnotationsBox->isChecked(), "Export with annotations flag is unchecked unexpectedly");

            QDialogButtonBox* box = qobject_cast<QDialogButtonBox *>(GTWidget::findWidget(os, "buttonBox", dialog));
            GTWidget::click(os, box->button(QDialogButtonBox::Cancel));
        }
    };

    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "murine.gb");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE));
    GTUtilsDialog::waitForDialog(os, new ExportSelectedRegionFiller(os, new CustomExportSelectedRegion()));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_0009) {
    // the test checks that a sequence associated with an annotation table can be exported
    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "murine.gb");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_CORRESPONDING_SEQ));
    GTUtilsDialog::waitForDialog(os, new ExportSelectedRegionFiller(os, sandBoxDir, "Project_export_test_0009.fa"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsTaskTreeView::waitTaskFinished(os);

    const QModelIndex docIndex = GTUtilsProjectTreeView::findIndex(os, "Project_export_test_0009.fa");
    GTUtilsProjectTreeView::findIndex(os, "NC_001363", docIndex);
}

GUI_TEST_CLASS_DEFINITION(test_0010) {
    // negative test for an annotation table not associated with any sequence
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTFileDialog::openFile(os, dataDir + "samples/GFF/", "5prime_utr_intron_A21.gff");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Ca21chr5 features"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_CORRESPONDING_SEQ));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTMouseDriver::click(os, Qt::RightButton);

    // the rest part of the test checks that a newly created association can be used for sequence export

    QModelIndex idxGff = GTUtilsProjectTreeView::findIndex(os, "Ca21chr5 features");
    QWidget *seqArea = GTWidget::findWidget(os, "render_area_human_T1 (UCSC April 2002 chr7:115977709-117855134)");

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, "Yes"));
    GTUtilsDialog::waitForDialog(os, new CreateObjectRelationDialogFiller(os));
    GTUtilsProjectTreeView::dragAndDrop(os, idxGff, seqArea);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Ca21chr5 features"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_CORRESPONDING_SEQ));
    GTUtilsDialog::waitForDialog(os, new ExportSelectedRegionFiller(os, sandBoxDir, "Project_export_test_0010.fa"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsTaskTreeView::waitTaskFinished(os);

    const QModelIndex docIndex = GTUtilsProjectTreeView::findIndex(os, "Project_export_test_0010.fa");
    GTUtilsProjectTreeView::findIndex(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)", docIndex);
}

GUI_TEST_CLASS_DEFINITION(test_0011) {
    // negative test for annotation table associated with a removed sequence

    GTFileDialog::openFile(os, dataDir + "samples/Genbank", "murine.gb");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(200);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_CORRESPONDING_SEQ));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_0012) {
    // test for an annotation table whose sequence association was changed
    GTFileDialog::openFile(os, dataDir + "samples/Genbank/", "murine.gb");
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");

    QModelIndex annIdx = GTUtilsProjectTreeView::findIndex(os, "NC_001363 features");
    QWidget *seqArea = GTWidget::findWidget(os, "render_area_human_T1 (UCSC April 2002 chr7:115977709-117855134)");

    GTUtilsDialog::waitForDialog(os, new CreateObjectRelationDialogFiller(os));
    GTUtilsProjectTreeView::dragAndDrop(os, annIdx, seqArea);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 features"));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_CORRESPONDING_SEQ));
    GTUtilsDialog::waitForDialog(os, new ExportSelectedRegionFiller(os, sandBoxDir, "Project_export_test_0012.fa"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsTaskTreeView::waitTaskFinished(os);

    const QModelIndex docIndex = GTUtilsProjectTreeView::findIndex(os, "Project_export_test_0012.fa");
    GTUtilsProjectTreeView::findIndex(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)", docIndex);
}

GUI_TEST_CLASS_DEFINITION(test_0013) {
    // test for multiple annotation object selection associated sequence import is not available
    GTFileDialog::openFile(os, dataDir + "samples/GFF/", "5prime_utr_intron_A21.gff");

    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Ca21chr5 features"));
    GTMouseDriver::click(os);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Ca21chr1 features"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(200);

    GTUtilsDialog::waitForDialog(os, new PopupChecker(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION
        << ACTION_EXPORT_CORRESPONDING_SEQ, PopupChecker::NotExists));
    GTMouseDriver::click(os, Qt::RightButton);
}

}

}
