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
#include "GTUtilsProject.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsApp.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsMdi.h"
#include "GTUtilsTaskTreeView.h"
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>

namespace U2{

namespace GUITest_common_scenarios_project{

GUI_TEST_CLASS_DEFINITION(test_0004) {

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj1.uprj");
    GTUtilsApp::checkUGENETitle(os, "proj1 UGENE");
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");

    GTUtilsProject::exportProject(os, testDir+"_common_data/scenarios/sandbox");
    GTUtilsProject::closeProject(os);

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/sandbox/proj1.uprj");
    GTUtilsApp::checkUGENETitle(os, "proj1 UGENE");
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1CF7.PDB"));
    GTGlobals::sleep(2000);
    GTUtilsToolTip::checkExistingToolTip(os, "_common_data/scenarios/sandbox/1CF7.PDB");

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1CF7.PDB"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);

    GTUtilsDocument::checkDocument(os,
        "1CF7.PDB",
        AnnotatedDNAViewFactory::ID
    );
}

GUI_TEST_CLASS_DEFINITION(test_0005) {

    GTUtilsProject::openFiles(os, testDir+"_common_data/scenarios/project/proj1.uprj");
    GTUtilsApp::checkUGENETitle(os, "proj1 UGENE");
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");

    GTUtilsProject::saveProjectAs(os,
        "proj2",
        testDir+"_common_data/scenarios/sandbox",
        "proj2"
    );

    GTUtilsProject::closeProject(os);

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

    GTUtilsProject::openFiles(os, testDir + "_common_data/fasta/fa1.fa.gz");
    GTUtilsDocument::checkDocument(os, "fa1.fa.gz", MSAEditorFactory::ID);
}

GUI_TEST_CLASS_DEFINITION(test_0010) {

    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsProjectTreeView::rename(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)", "qqq");
    CHECK_SET_ERR(GTUtilsProjectTreeView::findItem(os, "qqq") != NULL, "Item qqq not found in tree widget");
}

GUI_TEST_CLASS_DEFINITION(test_0011) {

    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/1.gb");
    GTUtilsProject::exportProjectCheck(os, "project.uprj");
}

GUI_TEST_CLASS_DEFINITION(test_0014) {
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE),ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB);
    GTUtilsDialog::RemoteDBDialogFiller filler(os, "1HTQ", 2); 
    GTUtilsDialog::waitForDialog(os, &filler);
    GTUtilsTaskTreeView::openView(os);
    GTUtilsTaskTreeView::cancelTask(os, "DownloadRemoteDocuments");
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

GUI_TEST_CLASS_DEFINITION(test_0023) {
    GTUtilsProject::openFiles(os, testDir + "_common_data/fasta/fa1.fa");
    GTUtilsMdi::click(os, GTGlobals::Minimize);

    QWidget* w = GTUtilsMdi::findWindow(os, "1m.fa");
    CHECK_SET_ERR(w != NULL, "Sequence view window title is not 1m.fa");
}

GUI_TEST_CLASS_DEFINITION(test_0026) {

    GTUtilsProject::openFiles(os, dataDir + "samples/Genbank/sars.gb");
    GTUtilsDocument::checkDocument(os, "sars.gb", AnnotatedDNAViewFactory::ID);
    GTUtilsDocument::removeDocument(os, "sars.gb");
}

GUI_TEST_CLASS_DEFINITION(test_0028) {
	GTLogTracer logTracer;
	GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");
//	GTUtilsMdi::click(os, GTGlobals::Minimize);
	QMdiSubWindow* fasta = (QMdiSubWindow*)GTUtilsMdi::findWindow(os, "human_T1 [s] human_T1 (UCSC April 2002 chr7:115977709-117855134)");

	GTUtilsProject::openFiles(os, dataDir + "samples/CLUSTALW/COI.aln");
//	GTUtilsMdi::click(os, GTGlobals::Minimize);
	QWidget* coi = GTUtilsMdi::findWindow(os, "COI [m] COI");
	CHECK_SET_ERR(fasta->windowIcon().cacheKey() != coi->windowIcon().cacheKey() , "Icons must not be equals");
	GTUtilsLog::check(os, logTracer);
}

GUI_TEST_CLASS_DEFINITION(test_0030) {
    GTLogTracer logTracer;
    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");

    GTUtilsProject::CloseProjectSettings s;
    s.saveOnCloseButton = QMessageBox::Cancel;

    GTUtilsProject::closeProject(os, s);
    GTUtilsLog::check(os, logTracer);
}

}

}
