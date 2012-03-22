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

#include "Project.h"
#include "ProjectUtils.h"
#include "DocumentUtils.h"
#include "LogUtils.h"
#include "AppUtils.h"
#include "ToolTipUtils.h"
#include "QtUtils.h"
#include "GUIDialogUtils.h"
#include "ProjectTreeViewUtils.h"
#include <U2View/AnnotatedDNAViewFactory.h>
#include <api/GTKeyboardDriver.h>
#include <api/GTMouseDriver.h>

namespace U2{

namespace GUITest_common_scenarios_project{

GUI_TEST_CLASS_DEFINITION(test_0004) {

    ProjectUtils::openProject(os,
        testDir+"_common_data/scenarios/project/proj1.uprj",
        "proj1 UGENE",
        "1CF7.PDB"
    );

    ProjectUtils::exportProject(os, testDir+"_common_data/scenarios/sandbox");
    ProjectUtils::closeProject(os);

    ProjectUtils::openProject(os,
        testDir+"_common_data/scenarios/sandbox/proj1.uprj",
        "proj1 UGENE",
        "1CF7.PDB"
    );

    ProjectTreeViewUtils::checkToolTip(os,
        "1CF7.PDB",
        "_common_data/scenarios/sandbox/1CF7.PDB"
    );

    ProjectTreeViewUtils::click(os, "1CF7.PDB");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);

    DocumentUtils::checkDocument(os,
        "1CF7.PDB",
        AnnotatedDNAViewFactory::ID
    );
}

GUI_TEST_CLASS_DEFINITION(test_0005) {

    ProjectUtils::openProject(os,
        testDir+"_common_data/scenarios/project/proj1.uprj",
        "proj1 UGENE",
        "1CF7.PDB"
    );

    ProjectUtils::saveProjectAs(os,
        "proj2",
        testDir+"_common_data/scenarios/sandbox",
        "proj2"
    );

    ProjectUtils::closeProject(os);

    ProjectUtils::openProject(os,
        testDir+"_common_data/scenarios/sandbox/proj2.uprj",
        "proj2 UGENE",
        "1CF7.PDB"
    );

    ProjectTreeViewUtils::checkToolTip(os,
        "1CF7.PDB",
        "samples/PDB/1CF7.PDB"
    );
}

GUI_TEST_CLASS_DEFINITION(test_0006) {
    AppUtils::checkUGENETitle(os, "UGENE");
    QAction *result = QtUtils::getMenuAction(os, ACTION_PROJECTSUPPORT__EXPORT_PROJECT, MWMENU_FILE);
    CHECK_SET_ERR(result == NULL, "Export menu item present in menu without any project created");
}

GUI_TEST_CLASS_DEFINITION(test_0009) {

    ProjectUtils::openFiles(os, testDir + "_common_data/fasta/fa1.fa.gz");
    DocumentUtils::checkDocument(os, "fa1.fa.gz", "MSAEditor");
}

GUI_TEST_CLASS_DEFINITION(test_0010) {

    ProjectUtils::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");
    ProjectTreeViewUtils::rename(os, "human_T1 (UCSC April 2002 chr7:115977709-117855134)", "qqq");
    ProjectTreeViewUtils::checkItem(os, "qqq", true);
}

GUI_TEST_CLASS_DEFINITION(test_0011) {

    ProjectUtils::openFiles(os, testDir + "_common_data/scenarios/project/1.gb");
    ProjectUtils::exportProjectCheck(os, "project.uprj");
}

GUI_TEST_CLASS_DEFINITION(test_0017) {

    ProjectUtils::openFiles(os, QList<QUrl>()
        << dataDir+"samples/Genbank/murine.gb"
        << dataDir+"samples/Genbank/sars.gb"
        << dataDir+"samples/Genbank/CVU55762.gb"
    );
    DocumentUtils::checkDocument(os, "murine.gb");
    DocumentUtils::checkDocument(os, "sars.gb");
    DocumentUtils::checkDocument(os, "CVU55762.gb");
}

GUI_TEST_CLASS_DEFINITION(test_0023) {
    os.setError("Test not implemented");
    //ProjectUtils::openFile(os, testDir + "_common_data/fasta/fa1.fa");
    //TODO: minimized sequence view and check title
}

GUI_TEST_CLASS_DEFINITION(test_0030) {
    LogTracer logTracer;
    ProjectUtils::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");

    ProjectUtils::CloseProjectSettings s;
    s.saveOnCloseButton = QMessageBox::Cancel;

    ProjectUtils::closeProject(os, s);
    LogUtils::check(os, logTracer);
}

}

}
