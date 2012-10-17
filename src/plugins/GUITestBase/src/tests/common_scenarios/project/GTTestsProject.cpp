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

#include "GTTestsProject.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTMenu.h"
#include "api/GTFile.h"
#include "api/GTFileDialog.h"
#include "api/GTLineEdit.h"
#include "GTUtilsProject.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsApp.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsMdi.h"
#include "GTUtilsTaskTreeView.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include "runnables/ugene/ugeneui/ExportProjectDialogFiller.h"
#include "runnables/ugene/ugeneui/CreateNewProjectWidgetFiller.h"
#include "runnables/ugene/ugeneui/ExportProjectDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ExportDocumentDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/DownloadRemoteFileDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"

#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>

namespace U2{

namespace GUITest_common_scenarios_project{

GUI_TEST_CLASS_DEFINITION(test_0004) {
// 1. Use menu {File->Open}. Open project _common_data/scenario/project/proj1.uprj
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/project/", "proj1.uprj");
// Expected state: 
//     1) Project view with document "1CF7.PDB" is opened
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");
//     2) UGENE window titled with text "proj1 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj1 UGENE");

// 2. Use menu {File->Export Project}
// Expected state: "Export Project" dialog has appeared
// 
// 3. Fill the next field in dialog:
//     {Destination Directory} _common_data/scenarios/sandbox
// 4. Click OK button
    GTUtilsDialog::waitForDialog(os, new ExportProjectDialogFiller(os, testDir+"_common_data/scenarios/sandbox"));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__EXPORT_PROJECT);
    GTGlobals::sleep();

// 5. Use menu {File->Close project}
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTGlobals::sleep();

// Expected state: project is unloaded and project view is closed
    GTUtilsProject::checkProject(os, GTUtilsProject::NotExists);

// 6. Use menu {File->Open}. Open project _common_data/sandbox/proj1.uprj
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/sandbox/", "proj1.uprj");
    GTGlobals::sleep();

// Expected state: 
//     1) project view with document "1CF7.PDB" has been opened, 
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");
//     2) UGENE window titled with text "proj1 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj1 UGENE");

//     3) File path at tooltip for "1CF7.PDB" must be "_common_data/scenarios/sandbox/1CF7.PDB"
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1CF7.PDB"));
    GTGlobals::sleep(2000);
    GTUtilsToolTip::checkExistingToolTip(os, "_common_data/scenarios/sandbox/1CF7.PDB");

// 7. Select "1CF7.PDB" in project tree and press Enter
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1CF7.PDB"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);

// Expected state: 
//     1) Document is loaded, 
    GTUtilsDocument::checkDocument(os, "1CF7.PDB", AnnotatedDNAViewFactory::ID);
//     2) 4 sequences and 3D Viewer with molecule is appeared
}

GUI_TEST_CLASS_DEFINITION(test_0005) {

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj1.uprj");
    GTUtilsApp::checkUGENETitle(os, "proj1 UGENE");
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");

    GTUtilsDialog::waitForDialog(os, new SaveProjectAsDialogFiller(os, "proj2", testDir+"_common_data/scenarios/sandbox", "proj2"));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__SAVE_AS_PROJECT);
    GTGlobals::sleep();

    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTGlobals::sleep();

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/sandbox/proj2.uprj");
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1CF7.PDB"));
    GTGlobals::sleep(2000);
    GTUtilsToolTip::checkExistingToolTip(os, "samples/PDB/1CF7.PDB");
}

GUI_TEST_CLASS_DEFINITION(test_0006) {
    GTUtilsApp::checkUGENETitle(os, "UGENE");

    QMenu *m = GTMenu::showMainMenu(os, MWMENU_FILE);
    QAction *result = GTMenu::getMenuItem(os, m, ACTION_PROJECTSUPPORT__EXPORT_PROJECT);
    GTGlobals::sleep();
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);
    GTGlobals::sleep(100);

    CHECK_SET_ERR(result == NULL, "Export menu item present in menu without any project created");
}
GUI_TEST_CLASS_DEFINITION(test_0007) {

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj1.uprj");
    GTUtilsApp::checkUGENETitle(os, "proj1 UGENE");
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");

    GTUtilsDocument::removeDocument(os, "1CF7.PDB", GTGlobals::UseMouse);
    GTUtilsProject::checkProject(os, GTUtilsProject::Empty);
}

GUI_TEST_CLASS_DEFINITION(test_0009) {

    GTUtilsProject::openFiles(os, dataDir + "samples/CLUSTALW/ty3.aln.gz");
    GTUtilsDocument::checkDocument(os, "ty3.aln.gz", MSAEditorFactory::ID);
}

GUI_TEST_CLASS_DEFINITION(test_0010) {

    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsProjectTreeView::rename(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)", "qqq");
    CHECK_SET_ERR(GTUtilsProjectTreeView::findItem(os, "qqq") != NULL, "Item qqq not found in tree widget");
}

GUI_TEST_CLASS_DEFINITION(test_0011) {

    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/1.gb");

    GTUtilsDialog::waitForDialog(os, new ExportProjectDialogChecker(os, "project.uprj"));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__EXPORT_PROJECT);
    GTGlobals::sleep();
}

GUI_TEST_CLASS_DEFINITION(test_0012) {

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "1.gb");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Export document", GTGlobals::UseMouse));

    Runnable *filler = new ExportDocumentDialogFiller(os, testDir + "_common_data/scenarios/sandbox/", "1.gb", 
                                                   ExportDocumentDialogFiller::Genbank, true, true, GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, filler);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1.gb"));
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsDocument::checkDocument(os, "1.gb.gz");
    GTGlobals::sleep(100);
    QString fileNames[] = {"_common_data/scenarios/project/test_0012.gb", "_common_data/scenarios/project/1.gb"};
    QString fileContent[2];

    for (int i = 0; i < 2; i++) {
        QFile file(testDir + fileNames[i]);
        if (! file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (! os.hasError()) {
                os.setError("Can't open file \"" + testDir + fileNames[i]);
            }
        }
        QTextStream in(&file);
        QString temp;
        temp = in.readLine();
        while (! in.atEnd()) {
            temp = in.readLine();
            fileContent[i] += temp;
        }
        file.close();
    }

    qDebug() << "file 1 = " << fileContent[0] << "file 2 = " << fileContent[1];
    if (fileContent[0] != fileContent[1] && !os.hasError() ) {
        os.setError("File is not expected file");
    }

}

GUI_TEST_CLASS_DEFINITION(test_0013) {
    //Add "Open project in new window" (0001629)

    //1. Open project _common_data\scenario\project\proj1.uprj
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/project/", "proj1.uprj");
    GTUtilsApp::checkUGENETitle(os, "proj1 UGENE");
    GTUtilsProject::checkProject(os);

    GTUtilsDialog::waitForDialog(os, new MessageBoxOpenAnotherProject(os));

    //2. Do menu {File->Open}. Open project _common_data\scenario\project\proj2.uprj
    //Expected state: dialog with text "Open project in new window" has appear

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj2.uprj"); // TODO: ask Shutov what to do

    /*
    this test just checking appearing of dialog not its behavior    
    */
}

GUI_TEST_CLASS_DEFINITION(test_0014) {
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE),ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFiller(os, "1HTQ", 2));
    GTGlobals::sleep();

    GTUtilsTaskTreeView::openView(os);
    GTUtilsTaskTreeView::cancelTask(os, "DownloadRemoteDocuments");
}

GUI_TEST_CLASS_DEFINITION(test_0016) {
    GTUtilsProject::openFiles(os, testDir + "_common_data/genbank/.dir/murine.gb");
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Export document", GTGlobals::UseMouse));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "murine.gb"));
    GTUtilsDialog::waitForDialog(os, new ExportDocumentDialogFiller(os));
    GTMouseDriver::click(os, Qt::RightButton);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "murine_copy1.gb"));
    GTGlobals::sleep();

    GTGlobals::sleep(2000);
    GTUtilsToolTip::checkExistingToolTip(os, ".dir");
}

GUI_TEST_CLASS_DEFINITION(test_0017) {

    GTUtilsProject::openFiles(os, QList<QUrl>()
        << dataDir+"samples/Genbank/murine.gb"
        << dataDir+"samples/Genbank/sars.gb"
        << dataDir+"samples/Genbank/CVU55762.gb"
    );
    GTUtilsDocument::checkDocument(os, "murine.gb");
    GTUtilsDocument::checkDocument(os, "sars.gb");
    GTUtilsDocument::checkDocument(os, "CVU55762.gb");
}
GUI_TEST_CLASS_DEFINITION(test_0018) {

    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsProjectTreeView::rename(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)", "qqq");
    GTUtilsProjectTreeView::rename(os, "qqq", "eee");
    GTUtilsDocument::removeDocument(os, "human_T1.fa");

    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");
    CHECK_SET_ERR(GTUtilsProjectTreeView::findItem(os, "human_T1.fa") != NULL, "Item human_T1.fa not found in tree widget");
}

GUI_TEST_CLASS_DEFINITION(test_0019) {

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep(1000);

    QTreeWidgetItem* se1 = GTUtilsProjectTreeView::findItem(os, "se1");
    QTreeWidgetItem* se2 = GTUtilsProjectTreeView::findItem(os, "se2");

    CHECK_SET_ERR(se1->font(0).bold(), "se1 are not marked with bold text");
    CHECK_SET_ERR(se2->font(0).bold(), "se2 are not marked with bold text");

    QWidget *w = GTWidget::findWidget(os, "render_area_se1");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_REMOVE" << ACTION_EDIT_SELECT_SEQUENCE_FROM_VIEW));
    GTMenu::showContextMenu(os, w);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);

    CHECK_SET_ERR(!se1->font(0).bold(), "se1 are not marked with regular text");
}

GUI_TEST_CLASS_DEFINITION(test_0020) {
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep(1000);

    QTreeWidgetItem* se1 = GTUtilsProjectTreeView::findItem(os, "se1");
    GTUtilsProjectTreeView::itemActiveCheck(os, se1);

    QTreeWidgetItem* se2 = GTUtilsProjectTreeView::findItem(os, "se2");
    GTUtilsProjectTreeView::itemActiveCheck(os, se2);

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTUtilsProjectTreeView::itemActiveCheck(os, se1, false);
    GTUtilsProjectTreeView::itemActiveCheck(os, se2, false);

    GTUtilsSequenceView::openSequenceView(os, "se1");
    GTUtilsProjectTreeView::itemActiveCheck(os, se1);

    GTUtilsSequenceView::addSequenceView(os, "se2");
    GTUtilsProjectTreeView::itemActiveCheck(os, se2);
}

GUI_TEST_CLASS_DEFINITION(test_0021) {
    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep(1000);

    QTreeWidgetItem* item = GTUtilsProjectTreeView::findItem(os, "se1");
    QFont font = item->font(0);
    CHECK_SET_ERR(font.bold(), "se1 item font is not a bold");
    item = GTUtilsProjectTreeView::findItem(os, "se2");
    font = item->font(0);
    CHECK_SET_ERR(font.bold(), "se2 item font is not a bold");

    GTUtilsMdi::click(os, GTGlobals::Close);
    GTGlobals::sleep(1000);
    item = GTUtilsProjectTreeView::findItem(os, "se1");
    font = item->font(0);
    CHECK_SET_ERR(!font.bold(), "se1 item font is not a bold");

    GTUtilsSequenceView::openSequenceView(os, "se1");	
    item = GTUtilsProjectTreeView::findItem(os, "se1");
    font = item->font(0);
    CHECK_SET_ERR(font.bold(), "se1 item font is not a bold");

    GTUtilsSequenceView::openSequenceView(os, "se2");	
    item = GTUtilsProjectTreeView::findItem(os, "se2");
    font = item->font(0);
    CHECK_SET_ERR(font.bold(), "se2 item font is not a bold");	 
}

GUI_TEST_CLASS_DEFINITION(test_0023) {
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/1m.fa");
    GTUtilsMdi::click(os, GTGlobals::Minimize);

    QWidget* w = GTUtilsMdi::findWindow(os, "1m [m] Multiple alignment");
    CHECK_SET_ERR(w != NULL, "Sequence view window title is not 1m [m] Multiple alignment");
}

GUI_TEST_CLASS_DEFINITION(test_0025) {

//     GTFile::backup(os, testDir + "_common_data/scenarios/project/proj4.uprj");
    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "proj4.uprj");

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_load_selected_documents", GTGlobals::UseMouse));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1.gb"));
    GTGlobals::sleep(1000);
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "action_load_selected_documents", GTGlobals::UseMouse));
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "2.gb"));
    GTGlobals::sleep(1000);
    GTMouseDriver::click(os, Qt::RightButton);

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, true, "<auto>", "misc_feature", "complement(1.. 20)"));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTGlobals::sleep(2000);
/*
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION__EXIT);
    GTGlobals::sleep();
*/
//     GTFile::restore(os, testDir + "_common_data/scenarios/project/proj4.uprj");
}

GUI_TEST_CLASS_DEFINITION(test_0026) {

    GTUtilsProject::openFiles(os, dataDir + "samples/Genbank/sars.gb");
    GTUtilsDocument::checkDocument(os, "sars.gb", AnnotatedDNAViewFactory::ID);
    GTUtilsDocument::removeDocument(os, "sars.gb");
}

GUI_TEST_CLASS_DEFINITION(test_0028) {
    GTLogTracer logTracer;
    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");
    QMdiSubWindow* fasta = (QMdiSubWindow*)GTUtilsMdi::findWindow(os, "human_T1 [s] human_T1 (UCSC April 2002 chr7:115977709-117855134)");

    GTUtilsProject::openFiles(os, dataDir + "samples/CLUSTALW/COI.aln");
    QWidget* coi = GTUtilsMdi::findWindow(os, "COI [m] COI");
    CHECK_SET_ERR(fasta->windowIcon().cacheKey() != coi->windowIcon().cacheKey() , "Icons must not be equals");
    GTUtilsLog::check(os, logTracer);
}

GUI_TEST_CLASS_DEFINITION(test_0030) {
    GTLogTracer logTracer;
    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Cancel));
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTGlobals::sleep();

    GTUtilsLog::check(os, logTracer);
}

GUI_TEST_CLASS_DEFINITION(test_0031) {

    GTFileDialog::openFile(os, dataDir+"samples/FASTA/", "human_T1.fa");
    GTUtilsDocument::checkDocument(os, "human_T1.fa");

    GTUtilsProjectTreeView::openView(os);
    GTGlobals::sleep();

    QLineEdit* nameFilterEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "nameFilterEdit"));
    GTLineEdit::setText(os, nameFilterEdit, "BBBB");
    GTGlobals::sleep();

    GTUtilsMdi::click(os, GTGlobals::Close);

    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
    GTUtilsProject::openFiles(os, dataDir+"samples/FASTA/human_T1.fa");

    GTGlobals::sleep();
}

}

}
