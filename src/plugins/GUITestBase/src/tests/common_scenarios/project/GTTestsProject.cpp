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
#include <api/GTKeyboardDriver.h>
#include "api/GTMenu.h"
#include "GTUtilsProject.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsApp.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsDialog.h"
#include "GTUtilsProjectTreeView.h"
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>

namespace U2{

namespace GUITest_common_scenarios_project{

GUI_TEST_CLASS_DEFINITION(test_0004) {

    GTUtilsProject::openProject(os,
        testDir+"_common_data/scenarios/project/proj1.uprj",
        "proj1 UGENE",
        "1CF7.PDB"
    );

    GTUtilsProject::exportProject(os, testDir+"_common_data/scenarios/sandbox");
    GTUtilsProject::closeProject(os);

    GTUtilsProject::openProject(os,
        testDir+"_common_data/scenarios/sandbox/proj1.uprj",
        "proj1 UGENE",
        "1CF7.PDB"
    );

    GTUtilsProjectTreeView::checkToolTip(os,
        "1CF7.PDB",
        "_common_data/scenarios/sandbox/1CF7.PDB"
    );

    GTUtilsProjectTreeView::click(os, "1CF7.PDB");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);

    GTUtilsDocument::checkDocument(os,
        "1CF7.PDB",
        AnnotatedDNAViewFactory::ID
    );
}

GUI_TEST_CLASS_DEFINITION(test_0005) {

    GTUtilsProject::openProject(os,
        testDir+"_common_data/scenarios/project/proj1.uprj",
        "proj1 UGENE",
        "1CF7.PDB"
    );

    GTUtilsProject::saveProjectAs(os,
        "proj2",
        testDir+"_common_data/scenarios/sandbox",
        "proj2"
    );

    GTUtilsProject::closeProject(os);

    GTUtilsProject::openProject(os,
        testDir+"_common_data/scenarios/sandbox/proj2.uprj",
        "proj2 UGENE",
        "1CF7.PDB"
    );

    GTUtilsProjectTreeView::checkToolTip(os,
        "1CF7.PDB",
        "samples/PDB/1CF7.PDB"
    );
}

GUI_TEST_CLASS_DEFINITION(test_0006) {
    GTUtilsApp::checkUGENETitle(os, "UGENE");

    QMenu *m = GTMenu::showMainMenu(os, MWMENU_FILE);
    QAction *result = GTMenu::getMenuItem(os, m, ACTION_PROJECTSUPPORT__EXPORT_PROJECT);

    CHECK_SET_ERR(result == NULL, "Export menu item present in menu without any project created");
}
GUI_TEST_CLASS_DEFINITION(test_0007) {

    GTUtilsProject::openProject(os, 
        testDir+"_common_data/scenarios/project/proj1.uprj", 
        "proj1 UGENE", 
        "1CF7.PDB"
        );
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
    GTUtilsProjectTreeView::checkItem(os, "qqq", true);
}

GUI_TEST_CLASS_DEFINITION(test_0011) {

    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/1.gb");
    GTUtilsProject::exportProjectCheck(os, "project.uprj");
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
    GTUtilsProjectTreeView::click(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)");
    GTUtilsProjectTreeView::rename(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)", "qqq");
    GTUtilsProjectTreeView::rename(os, "qqq", "eee");
    GTUtilsDocument::removeDocument(os, "human_T1.fa");
    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsProjectTreeView::checkItem(os, "human_T1.fa", true);
}
GUI_TEST_CLASS_DEFINITION(test_0023) {
    os.setError("Test not implemented");
    //ProjectUtils::openFile(os, testDir + "_common_data/fasta/fa1.fa");
    //TODO: minimized sequence view and check title
}
GUI_TEST_CLASS_DEFINITION(test_0026) {

    GTUtilsProject::openFiles(os, dataDir + "samples/Genbank/sars.gb");
    GTUtilsDocument::checkDocument(os, "sars.gb", AnnotatedDNAViewFactory::ID);
    GTUtilsDocument::removeDocument(os, "sars.gb");
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
