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

namespace U2{

namespace GUITest_common_scenarios_project{

GUI_TEST_CLASS_DEFINITION(test_0004) {

    add(new ProjectUtils::OpenProjectGUIAction(
        testDir+"_common_data/scenarios/project/proj1.uprj",
        "proj1 UGENE",
        "1CF7.PDB")
        );

    add( new ProjectUtils::ExportProjectGUIAction(testDir+"_common_data/scenarios/sandbox") );
    add( new ProjectUtils::CloseProjectGUIAction() );

    add(new ProjectUtils::OpenProjectGUIAction(
        testDir+"_common_data/scenarios/sandbox/proj1.uprj",
        "proj1 UGENE",
        "1CF7.PDB")
        );

    add( new ProjectTreeViewUtils::CheckToolTipGUIAction("1CF7.PDB", "_common_data/scenarios/sandbox/1CF7.PDB") );

    add( new ProjectTreeViewUtils::ClickEnterGUIAction("1CF7.PDB"));
    add( new DocumentUtils::CheckDocumentExistsGUIAction("1CF7.PDB", AnnotatedDNAViewFactory::ID) );
}

void test_0005::execute( U2OpStatus &os ){
//     ProjectUtils::openFile(os, testDir+"_common_data/scenarios/project/proj1.uprj");
//     DocumentUtils::checkDocumentExists(os, "1CF7.PDB");
//     AppUtils::checkUGENETitle(os, "proj1 UGENE");
// 
//     ProjectUtils::saveProjectAs(os, "proj2", testDir+"_common_data/scenarios/sandbox", "proj2");
//     ProjectUtils::closeProject(os);
// 
//     ProjectUtils::openFile(os, testDir+"_common_data/scenarios/sandbox/proj2.uprj");
//     DocumentUtils::checkDocumentExists(os, "1CF7.PDB");
//     AppUtils::checkUGENETitle(os, "proj2 UGENE");
// 
//     ToolTipUtils::checkProjectTreeToolTip(os, "samples/PDB/1CF7.PDB", 0);
}

void test_0006::execute( U2OpStatus &os ){
    AppUtils::checkUGENETitle(os, "UGENE");
    QAction *result = QtUtils::getMenuAction(os, ACTION_PROJECTSUPPORT__EXPORT_PROJECT, MWMENU_FILE);
    CHECK_SET_ERR(result == NULL, "Export menu item present in menu without any project created");
}

void test_0009::execute( U2OpStatus &os ){

//     ProjectUtils::openFile(os, testDir + "_common_data/fasta/fa1.fa.gz");
//     DocumentUtils::checkDocumentExists(os, "fa1.fa.gz", "MSAEditor");
}

void test_0011::execute(U2OpStatus &os) {

//     ProjectUtils::openFile(os, testDir + "_common_data/scenarios/project/1.gb");
// 
//     GUIDialogUtils::openExportProjectDialog(os);
//     GUIDialogUtils::checkExportProjectDialog(os, "project.uprj");
}

void test_0017::execute(U2OpStatus &os) {

//     QList<QUrl> urls;
//     urls << dataDir+"samples/Genbank/murine.gb";
//     urls << dataDir+"samples/Genbank/sars.gb";
//     urls << dataDir+"samples/Genbank/CVU55762.gb";
//     ProjectUtils::openFiles(os, urls);
// 
//     DocumentUtils::checkDocumentExists(os, "murine.gb");
//     DocumentUtils::checkDocumentExists(os, "sars.gb");
//     DocumentUtils::checkDocumentExists(os, "CVU55762.gb");
}

void test_0023::execute(U2OpStatus &os) {

//    ProjectUtils::openFile(os, testDir + "_common_data/fasta/fa1.fa");
}

void test_0030::execute(U2OpStatus &os){

//     LogTracer log;
//     ProjectUtils::openFile(os, dataDir + "samples/FASTA/human_T1.fa");
// 
//     ProjectUtils::CloseProjectSettings button_to_press;
//     button_to_press.saveOnClose = ProjectUtils::CloseProjectSettings::CANCEL;
// 
//     ProjectUtils::closeProject(os, button_to_press);
// 
//     LogUtils::checkHasError(os, log);
}

}

}