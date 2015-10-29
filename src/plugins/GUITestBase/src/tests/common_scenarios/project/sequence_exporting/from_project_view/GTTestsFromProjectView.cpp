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

#include "GTTestsFromProjectView.h"
#include "GTGlobals.h"
#include "drivers/GTKeyboardDriver.h"
#include "utils/GTKeyboardUtils.h"
#include "drivers/GTMouseDriver.h"
#include "primitives/GTMenu.h"
#include "api/GTSequenceReadingModeDialogUtils.h"
#include "api/GTTreeWidget.h"
#include "api/GTFileDialog.h"
#include "GTUtilsProject.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "utils/GTUtilsApp.h"
#include "utils/GTUtilsToolTip.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsMdi.h"
#include "primitives/PopupChooser.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSequencesDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSequences2MSADialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportMSA2SequencesDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ExportChromatogramFiller.h"
#include "base_dialogs/MessageBoxFiller.h"

#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>

namespace U2 {

namespace GUITest_common_scenarios_project_sequence_exporting_from_project_view {
using namespace HI;

GUI_TEST_CLASS_DEFINITION(test_0001) {

// 1. Use menu {File->Open}. Open project _common_data/scenario/project/proj4.uprj
    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj4.uprj");

// Expected state:
//     1) Project view with document "1.gb" and "2.gb" is opened, both documents are unloaded
    GTUtilsDocument::checkDocument(os, "1.gb", GTUtilsDocument::DocumentUnloaded);
    GTUtilsDocument::checkDocument(os, "2.gb", GTUtilsDocument::DocumentUnloaded);

// 2. Double click on [a] Annotations sequence object, in project view tree
    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, "Annotations");
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// Expected result: NC_001363 sequence has been opened in sequence view
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);

// 3. Right click on [s] NC_001363 sequence object, in project view tree. Use context menu item {Export->Export sequences}
// Expected state: Export sequences dialog open
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE, GTGlobals::UseMouse));

// 4. Fill the next field in dialog:
//     {Export to file:} _common_data/scenarios/sandbox/exp.fasta
//     {File format to use:} FASTA
//     {Add created document to project} set checked
//
// 5. Click Export button.
    Runnable *filler = new ExportSequenceOfSelectedAnnotationsFiller(os,
        testDir+"_common_data/scenarios/sandbox/exp.fasta",
        ExportSequenceOfSelectedAnnotationsFiller::Fasta,
        ExportSequenceOfSelectedAnnotationsFiller::SaveAsSeparate
        );
    GTUtilsDialog::waitForDialog(os, filler);

    GTGlobals::sleep(5000);

    QModelIndex parent = GTUtilsProjectTreeView::findIndex(os, "1.gb");
    QModelIndex child = GTUtilsProjectTreeView::findIndex(os, "NC_001363 sequence", parent);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os,child));
    GTMouseDriver::click(os, Qt::RightButton);


    GTGlobals::sleep(3000);
// Expected state: sequence view NC_001363 sequence has been opened, with sequence same as in 1.gb document
    GTUtilsDocument::checkDocument(os, "exp.fasta", AnnotatedDNAViewFactory::ID);
}

GUI_TEST_CLASS_DEFINITION(test_0002) {

// 1. Use menu {File->Open}. Open project _common_data/scenario/project/proj4.uprj
    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj4.uprj");
// Expected state:
//     1) Project view with document "1.gb" and "2.gb" is opened, both documents are unloaded
    GTUtilsDocument::checkDocument(os, "1.gb", GTUtilsDocument::DocumentUnloaded);
    GTUtilsDocument::checkDocument(os, "2.gb", GTUtilsDocument::DocumentUnloaded);

// 2. Double click on [a] Annotations sequence object, in project view tree
    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, "Annotations");
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
// Expected result: NC_001363 sequence has been opened in sequence view
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);

// 3. Right click on [s] NC_001363 sequence object, in project view tree. Use context menu item {Export->Export sequence as alignment}
// Expected state: Export sequences dialog open
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT, GTGlobals::UseMouse));

// 4. Select file to save: _common_data/scenarios/sandbox/exp2.aln and set 'file format to use' to CLUSTALW,
// Than to uncheck the 'add document to the project' checkbox and click Save button.
    Runnable *filler = new ExportSequenceAsAlignmentFiller(os,
        testDir+"_common_data/scenarios/sandbox/",
        "exp2.aln",
        ExportSequenceAsAlignmentFiller::Clustalw,
        false
    );
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// 5. Open file _common_data/scenarios/sandbox/exp2.aln
// Expected state: multiple aligniment view with NC_001363 sequence has been opened
    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/sandbox/exp2.aln");
    GTUtilsDocument::checkDocument(os, "exp2.aln");
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep(1000);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "multiple.fa"));
    GTMouseDriver::click(os);

    GTKeyboardUtils::selectAll(os);
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT, GTGlobals::UseMouse));

    Runnable *filler = new ExportSequenceAsAlignmentFiller(os,
        testDir+"_common_data/scenarios/sandbox/",
        "exp2.aln",
        ExportSequenceAsAlignmentFiller::Clustalw,
        GTGlobals::UseMouse
        );
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/sandbox/exp2.aln");
    GTUtilsDocument::checkDocument(os, "exp2.aln");
}

GUI_TEST_CLASS_DEFINITION(test_0004) {
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/", "HIV-1.aln");
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_PROJECT__EXPORT_AS_SEQUENCES_ACTION));

    GTUtilsDialog::waitForDialog(os, new ExportToSequenceFormatFiller(os, dataDir + " _common_data/scenarios/sandbox/", "export1.fa", ExportToSequenceFormatFiller::FASTA, true, true));

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "HIV-1.aln"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, "export1.fa");
    GTUtilsProjectTreeView::scrollTo(os, "ru131");

    GTKeyboardDriver::keyClick(os, 'w', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);

    itemPos = GTUtilsProjectTreeView::getItemCenter(os, "ru131");
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep(1000);

    QWidget *activeWindow =  GTUtilsMdi::activeWindow(os);
    if (! activeWindow->windowTitle().contains("ru131") && !os.hasError()) {
        os.setError("fasta file with sequences has been not opened");
    }

    QString sequenceEnd = GTUtilsSequenceView::getEndOfSequenceAsString(os, 1);
    if (sequenceEnd.at(0) != '-' && !os.hasError()) {
        os.setError("sequence [s] ru131 has not NOT'-' symbols at the end of sequence");
    }
}

GUI_TEST_CLASS_DEFINITION(test_0005) {

    // 1. Use menu {File->Open}. Open project data/samples/CLUSTALW/COI.aln

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTGlobals::sleep(1000);

    // 3. Right click [m] COI object, in project view tree. Use context menu item {Export->Export to FASTA}
    //    Expected state: Export aligniment dialog open

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_PROJECT__EXPORT_AS_SEQUENCES_ACTION));
    GTUtilsDialog::waitForDialog(os, new ExportToSequenceFormatFiller(os, dataDir + " _common_data/scenarios/sandbox/", "export1.fa",
                                ExportToSequenceFormatFiller::FASTA, true, false));

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI.aln"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(1000);

    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Open View" << "Open New View", GTGlobals::UseMouse));

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Zychia_baranovi"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(1000);

    GTGlobals::sleep(1000);

    QWidget *activeWindow =  GTUtilsMdi::activeWindow(os);
    if (! activeWindow->windowTitle().contains("Zychia_baranovi") && !os.hasError()) {
        os.setError("fasta file with sequences has been not opened");
        }

    int sequenceLength = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(sequenceLength == 586, "Sequence length is " + QString::number(sequenceLength) + ", expected 586");
    }

GUI_TEST_CLASS_DEFINITION(test_0005_1) {

    // 1. Use menu {File->Open}. Open project data/samples/CLUSTALW/COI.aln

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTGlobals::sleep(1000);

    // 3. Right click [m] COI object, in project view tree. Use context menu item {Export->Export to FASTA}
    //    Expected state: Export aligniment dialog open

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_PROJECT__EXPORT_AS_SEQUENCES_ACTION));
    GTUtilsDialog::waitForDialog(os, new ExportToSequenceFormatFiller(os, dataDir + " _common_data/scenarios/sandbox/", "export1.fa",
        ExportToSequenceFormatFiller::FASTA, true, true));

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI.aln"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(1000);

    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Open View" << "Open New View", GTGlobals::UseMouse));

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Zychia_baranovi"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(1000);

    GTGlobals::sleep(1000);

    QWidget *activeWindow =  GTUtilsMdi::activeWindow(os);
    if (! activeWindow->windowTitle().contains("Zychia_baranovi") && !os.hasError()) {
        os.setError("fasta file with sequences has been not opened");
        }

    int sequenceLength = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(sequenceLength == 604, "Sequence length is " + QString::number(sequenceLength) + ", expected 586");
    }

GUI_TEST_CLASS_DEFINITION(test_0005_2) {


    // 1. Use menu {File->Open}. Open project data/samples/CLUSTALW/COI.aln

    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW", "COI.aln");
    GTGlobals::sleep(1000);

    // 3. Right click [m] COI object, in project view tree. Use context menu item {Export->Export to FASTA}
    //    Expected state: Export aligniment dialog open

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_PROJECT__EXPORT_AS_SEQUENCES_ACTION));
    GTUtilsDialog::waitForDialog(os, new ExportToSequenceFormatFiller(os, dataDir + " _common_data/scenarios/sandbox/", "export1.fa",
        ExportToSequenceFormatFiller::FASTA, true, false));

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "COI"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(1000);

    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Open View" << "Open New View", GTGlobals::UseMouse));

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "Zychia_baranovi"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(1000);

    GTGlobals::sleep(1000);

    QWidget *activeWindow =  GTUtilsMdi::activeWindow(os);
    if (! activeWindow->windowTitle().contains("Zychia_baranovi") && !os.hasError()) {
        os.setError("fasta file with sequences has been not opened");
        }

    int sequenceLength = GTUtilsSequenceView::getLengthOfSequence(os);
    CHECK_SET_ERR(sequenceLength == 586, "Sequence length is " + QString::number(sequenceLength) + ", expected 586");

    }

GUI_TEST_CLASS_DEFINITION(test_0006) {
    const QString doc1("1.gb"), doc2("2.gb");

// 1. Use menu {File->Open}. Open project _common_data/scenario/project/proj4.uprj

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj4.uprj");
    GTGlobals::sleep(1000);

// Expected state:
//     1) Project view with document "1.gb" and "2.gb" is opened, both documents are unloaded
    GTUtilsProjectTreeView::findIndex(os, doc1);//checks inside
    GTUtilsProjectTreeView::findIndex(os, doc2);
    if (GTUtilsDocument::isDocumentLoaded(os, doc1) || GTUtilsDocument::isDocumentLoaded(os, doc2)) {
        os.setError("Documents is not unload");
        return;
    }

// 2. Double click on [a] Annotations sequence object, in project view tree
    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, "Annotations");
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// Expected result: NC_001363 sequence has been opened in sequence view
    GObjectViewWindow *activeWindow = qobject_cast<GObjectViewWindow*> (GTUtilsMdi::activeWindow(os));
    if (! activeWindow->getViewName().contains("NC_001363")) {
        os.setError("NC_001363 sequence has been not opened in sequence view");
        return;
    }

// 3. Right click on [s] NC_001363 sequence object, in project view tree. Use context menu item {Export->Export sequence as alignment}
// Expected state: Export sequences dialog open
// 4. Select file to save: _common_data/scenarios/sandbox/exp2.msf and set 'file format to use' to MSF,
// Than to uncheck the 'add document to the project' checkbox and click Save button.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT));

    Runnable *filler = new ExportSequenceAsAlignmentFiller(os, dataDir + "_common_data/scenarios/sandbox/",
        "exp2.msf", ExportSequenceAsAlignmentFiller::Msf);
    GTUtilsDialog::waitForDialog(os, filler);

    QModelIndex parent = GTUtilsProjectTreeView::findIndex(os, "1.gb");
    QModelIndex child = GTUtilsProjectTreeView::findIndex(os, "NC_001363 sequence", parent);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, child));
    GTMouseDriver::click(os, Qt::RightButton);

// 5. Open file _common_data/scenarios/sandbox/exp2.msf
    GTGlobals::sleep();
    GTFileDialog::openFile(os, dataDir + "_common_data/scenarios/sandbox/", "exp2.msf");
    GTGlobals::sleep(1000);

// Expected state: multiple aligniment view with NC_001363 sequence has been opened
    if (GTUtilsProjectTreeView::getSelectedItem(os) != "[s] NC_001363 sequence") {
        os.setError("multiple alignment view with NC_001363 sequence has been not opened");
    }
}

GUI_TEST_CLASS_DEFINITION(test_0007) {

    const QString doc1("1.gb"), doc2("2.gb");

    // 1. Use menu {File->Open}. Open project _common_data/scenario/project/proj4.uprj

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj4.uprj");
    GTGlobals::sleep(1000);

    // Expected state:
    //     1) Project view with document "1.gb" and "2.gb" is opened, both documents are unloaded
    GTUtilsProjectTreeView::findIndex(os, doc1);//checks inside
    GTUtilsProjectTreeView::findIndex(os, doc2);
    if (GTUtilsDocument::isDocumentLoaded(os, doc1) || GTUtilsDocument::isDocumentLoaded(os, doc2)) {
        os.setError("Documents is not unload");
        return;
        }

    // 2. Double click on [a] Annotations sequence object, in project view tree
    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, "Annotations");
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    // Expected result: NC_001363 sequence has been opened in sequence view
    GObjectViewWindow *activeWindow = qobject_cast<GObjectViewWindow*> (GTUtilsMdi::activeWindow(os));
    if (! activeWindow->getViewName().contains("NC_001363")) {
        os.setError("NC_001363 sequence has been not opened in sequence view");
        return;
        }

    // 3. Right click on [s] NC_001363 sequence object, in project view tree. Use context menu item {Export->Export sequence as alignment}
    // Expected state: Export sequences dialog open
    // 4. Select file to save: _common_data/scenarios/sandbox/exp2.msf and set 'file format to use' to MSF,
    // Than to uncheck the 'add document to the project' checkbox and click Save button.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT));

    Runnable *filler = new ExportSequenceAsAlignmentFiller(os, dataDir + "_common_data/scenarios/sandbox/",
        "exp2.sto", ExportSequenceAsAlignmentFiller::Stockholm);
    GTUtilsDialog::waitForDialog(os, filler);

    QModelIndex parent = GTUtilsProjectTreeView::findIndex(os, "1.gb");
    QModelIndex child = GTUtilsProjectTreeView::findIndex(os, "NC_001363 sequence", parent);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, child));
    GTMouseDriver::click(os, Qt::RightButton);

    // 5. Open file _common_data/scenarios/sandbox/exp2.msf
    GTGlobals::sleep();
    GTFileDialog::openFile(os, dataDir + "_common_data/scenarios/sandbox/", "exp2.sto");
    GTGlobals::sleep(1000);

    // Expected state: multiple alignment view with NC_001363 sequence has been opened
    if (GTUtilsProjectTreeView::getSelectedItem(os) != "[s] NC_001363 sequence") {
        os.setError("multiple alignment view with NC_001363 sequence has been not opened");
        }
    }

GUI_TEST_CLASS_DEFINITION(test_0007_1) {

    const QString doc1("1.gb"), doc2("2.gb");

    // 1. Use menu {File->Open}. Open project _common_data/scenario/project/proj4.uprj

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj4.uprj");
    GTGlobals::sleep(1000);

    // Expected state:
    //     1) Project view with document "1.gb" and "2.gb" is opened, both documents are unloaded
    GTUtilsProjectTreeView::findIndex(os, doc1);//checks inside
    GTUtilsProjectTreeView::findIndex(os, doc2);
    if (GTUtilsDocument::isDocumentLoaded(os, doc1) || GTUtilsDocument::isDocumentLoaded(os, doc2)) {
        os.setError("Documents is not unload");
        return;
        }

    // 2. Double click on [a] Annotations sequence object, in project view tree
    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, "Annotations");
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    // Expected result: NC_001363 sequence has been opened in sequence view
    GObjectViewWindow *activeWindow = qobject_cast<GObjectViewWindow*> (GTUtilsMdi::activeWindow(os));
    if (! activeWindow->getViewName().contains("NC_001363")) {
        os.setError("NC_001363 sequence has been not opened in sequence view");
        return;
        }

    // 3. Right click on [s] NC_001363 sequence object, in project view tree. Use context menu item {Export->Export sequence as alignment}
    // Expected state: Export sequences dialog open
    // 4. Select file to save: _common_data/scenarios/sandbox/exp2.msf and set 'file format to use' to MSF,
    // Than to uncheck the 'add document to the project' check box and click Save button.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT));

    Runnable *filler = new ExportSequenceAsAlignmentFiller(os, dataDir + "_common_data/scenarios/sandbox/",
        "exp2.fa", ExportSequenceAsAlignmentFiller::Fasta);
    GTUtilsDialog::waitForDialog(os, filler);

    QModelIndex parent = GTUtilsProjectTreeView::findIndex(os, "1.gb");
    QModelIndex child = GTUtilsProjectTreeView::findIndex(os, "NC_001363 sequence", parent);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, child));
    GTMouseDriver::click(os, Qt::RightButton);

    // 5. Open file _common_data/scenarios/sandbox/exp2.msf
    GTGlobals::sleep();
    GTFileDialog::openFile(os, dataDir + "_common_data/scenarios/sandbox/", "exp2.fa");
    GTGlobals::sleep(1000);

    // Expected state: multiple alignment view with NC_001363 sequence has been opened
    if (GTUtilsProjectTreeView::getSelectedItem(os) != "[s] NC_001363 sequence") {
        os.setError("multiple alignment view with NC_001363 sequence has been not opened");
        }
    }

GUI_TEST_CLASS_DEFINITION(test_0007_2) {

    const QString doc1("1.gb"), doc2("2.gb");

    // 1. Use menu {File->Open}. Open project _common_data/scenario/project/proj4.uprj

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj4.uprj");
    GTGlobals::sleep(1000);

    // Expected state:
    //     1) Project view with document "1.gb" and "2.gb" is opened, both documents are unloaded
    GTUtilsProjectTreeView::findIndex(os, doc1);//checks inside
    GTUtilsProjectTreeView::findIndex(os, doc2);
    if (GTUtilsDocument::isDocumentLoaded(os, doc1) || GTUtilsDocument::isDocumentLoaded(os, doc2)) {
        os.setError("Documents is not unload");
        return;
        }

    // 2. Double click on [a] Annotations sequence object, in project view tree
    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, "Annotations");
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

    // Expected result: NC_001363 sequence has been opened in sequence view
    GObjectViewWindow *activeWindow = qobject_cast<GObjectViewWindow*> (GTUtilsMdi::activeWindow(os));
    if (! activeWindow->getViewName().contains("NC_001363")) {
        os.setError("NC_001363 sequence has been not opened in sequence view");
        return;
        }

    // 3. Right click on [s] NC_001363 sequence object, in project view tree. Use context menu item {Export->Export sequence as alignment}
    // Expected state: Export sequences dialog open
    // 4. Select file to save: _common_data/scenarios/sandbox/exp2.msf and set 'file format to use' to MSF,
    // Than to uncheck the 'add document to the project' check box and click Save button.
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT));

    Runnable *filler = new ExportSequenceAsAlignmentFiller(os, dataDir + "_common_data/scenarios/sandbox/",
        "exp2.meg", ExportSequenceAsAlignmentFiller::Mega);
    GTUtilsDialog::waitForDialog(os, filler);

    QModelIndex parent = GTUtilsProjectTreeView::findIndex(os, "1.gb");
    QModelIndex child = GTUtilsProjectTreeView::findIndex(os, "NC_001363 sequence", parent);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, child));
    GTMouseDriver::click(os, Qt::RightButton);

    // 5. Open file _common_data/scenarios/sandbox/exp2.msf
    GTGlobals::sleep();
    GTFileDialog::openFile(os, dataDir + "_common_data/scenarios/sandbox/", "exp2.meg");
    GTGlobals::sleep(1000);

    // Expected state: multiple alignment view with NC_001363 sequence has been opened
    if (GTUtilsProjectTreeView::getSelectedItem(os) != "[s] NC_001363 sequence") {
        os.setError("multiple alignment view with NC_001363 sequence has been not opened");
        }
    }
GUI_TEST_CLASS_DEFINITION(test_0008) {
    GTFileDialog::openFile(os, dataDir + "samples/ABIF/", "A01.abi");
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_CHROMATOGRAM));
    GTGlobals::sleep(1000);

    Runnable *filler = new ExportChromatogramFiller(os, sandBoxDir,
        "pagefile.sys", ExportChromatogramFiller::SCF, false, true, true);
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "A01.abi"));
    GTMouseDriver::click(os, Qt::RightButton);

    }
GUI_TEST_CLASS_DEFINITION(test_0008_1) {
    GTFileDialog::openFile(os, dataDir + "samples/ABIF/", "A01.abi");
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_CHROMATOGRAM));
    GTGlobals::sleep(1000);

    Runnable *filler = new ExportChromatogramFiller(os, sandBoxDir,
        "pagefile.sys", ExportChromatogramFiller::SCF, true, true, true);
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "A01.abi"));
    GTMouseDriver::click(os, Qt::RightButton);
    }
GUI_TEST_CLASS_DEFINITION(test_0008_2) {
    GTFileDialog::openFile(os, dataDir + "samples/ABIF/", "A01.abi");
    GTGlobals::sleep(1000);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_IMPORT_MENU_ACTION << ACTION_EXPORT_CHROMATOGRAM));
    GTGlobals::sleep(1000);

    Runnable *filler = new ExportChromatogramFiller(os, sandBoxDir,
        "pagefile.sys", ExportChromatogramFiller::SCF, false, true, false);
    GTUtilsDialog::waitForDialog(os, filler);
    GTGlobals::sleep(1000);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "A01.abi"));
    GTMouseDriver::click(os, Qt::RightButton);
    }

} // namespace
} // namespace U2
