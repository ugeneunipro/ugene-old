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

#include "GTTestsFromProjectView.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTMouseDriver.h"
#include "api/GTMenu.h"
#include "api/GTSequenceReadingModeDialogUtils.h"
#include "api/GTTreeWidget.h"
#include "api/GTFileDialog.h"
#include "GTUtilsProject.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsApp.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsMdi.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSequencesDialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportSequences2MSADialogFiller.h"
#include "runnables/ugene/plugins/dna_export/ExportMSA2SequencesDialogFiller.h"

#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>

namespace U2 {

namespace GUITest_common_scenarios_project_sequence_exporting_from_project_view {

GUI_TEST_CLASS_DEFINITION(test_0001) {

// 1. Use menu {File->Open}. Open project _common_data/scenario/project/proj4.uprj
    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj4.uprj");

// Expected state: 
//     1) Project view with document "1.gb" and "2.gb" is opened, both documents are unloaded
    GTUtilsDocument::checkDocument(os, "1.gb", GTUtilsDocument::DocumentUnloaded);
    GTUtilsDocument::checkDocument(os, "2.gb", GTUtilsDocument::DocumentUnloaded);

//     2) UGENE window titled with text "proj4 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj4 UGENE");

// 2. Double click on [a] Annotations sequence object, in project view tree
    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, "Annotations");
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();

// Expected result: NC_001363 sequence has been opened in sequence view
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);

// 3. Right click on [s] NC_001363 sequence object, in project view tree. Use context menu item {Export->Export sequences}
// Expected state: Export sequences dialog open
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE, GTGlobals::UseMouse));

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

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 sequence"));
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
//     2) UGENE window titled with text "proj4 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj4 UGENE");

// 2. Double click on [a] Annotations sequence object, in project view tree
    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, "Annotations");
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep();
// Expected result: NC_001363 sequence has been opened in sequence view
    GTUtilsDocument::checkDocument(os, "1.gb", AnnotatedDNAViewFactory::ID);

// 3. Right click on [s] NC_001363 sequence object, in project view tree. Use context menu item {Export->Export sequence as alignment}
// Expected state: Export sequences dialog open
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT, GTGlobals::UseMouse));

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

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT, GTGlobals::UseMouse));

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

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_MENU_ACTION << ACTION_PROJECT__EXPORT_AS_SEQUENCES_ACTION));

    GTUtilsDialog::waitForDialog(os, new ExportToSequenceFormatFiller(os, dataDir + " _common_data/scenarios/sandbox/", "export1.fa"));

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

GUI_TEST_CLASS_DEFINITION(test_0006) {
    const QString doc1("1.gb"), doc2("2.gb");

// 1. Use menu {File->Open}. Open project _common_data/scenario/project/proj4.uprj

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj4.uprj");
    GTGlobals::sleep(1000);

// Expected state: 
//     1) Project view with document "1.gb" and "2.gb" is opened, both documents are unloaded
    QTreeWidgetItem *item1 = GTUtilsProjectTreeView::findItem(os, doc1);
    QTreeWidgetItem *item2 = GTUtilsProjectTreeView::findItem(os, doc2);
    if (item1 == NULL || item2 == NULL) {
        os.setError("Project view with document \"1.gb\" and \"2.gb\" is not opened");
        return;
    }
    if (GTUtilsDocument::isDocumentLoaded(os, doc1) || GTUtilsDocument::isDocumentLoaded(os, doc2)) {
        os.setError("Documents is not unload");
        return;
    }

//     2) UGENE window titled with text "proj4 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj4 UGENE");


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
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__EXPORT_MENU_ACTION << ACTION_EXPORT_SEQUENCE_AS_ALIGNMENT));

    Runnable *filler = new ExportSequenceAsAlignmentFiller(os, dataDir + "_common_data/scenarios/sandbox/",
        "exp2.msf", ExportSequenceAsAlignmentFiller::Msf);
    GTUtilsDialog::waitForDialog(os, filler);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "NC_001363 sequence"));
    GTMouseDriver::click(os, Qt::RightButton);

// 5. Open file _common_data/scenarios/sandbox/exp2.msf
    GTGlobals::sleep();
    GTFileDialog::openFile(os, dataDir + "_common_data/scenarios/sandbox/", "exp2.msf");
    GTGlobals::sleep(1000);

// Expected state: multiple aligniment view with NC_001363 sequence has been opened
    if (GTUtilsProjectTreeView::getSelectedItem(os) != "NC_001363 sequence") {
        os.setError("multiple alignment view with NC_001363 sequence has been not opened");
    }
}


} // namespace
} // namespace U2
