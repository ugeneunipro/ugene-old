/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <U2View/MSAEditorFactory.h>
#include <U2View/AnnotatedDNAViewFactory.h>

#include "GTTestsProjectRemoteRequest.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsMdi.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsToolTip.h"
#include "api/GTFileDialog.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTSequenceReadingModeDialogUtils.h"
#include "api/GTTreeWidget.h"
#include "runnables/ugene/corelibs/U2Gui/DownloadRemoteFileDialogFiller.h"

namespace U2{

namespace GUITest_common_scenarios_project_remote_request {

GUI_TEST_CLASS_DEFINITION(test_0001) {
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFillerDeprecated(os, "3EZB", 3));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE),QStringList()<<ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTGlobals::sleep();

    GTUtilsTaskTreeView::openView(os);
    GTUtilsTaskTreeView::cancelTask(os, "DownloadRemoteDocuments");
    GTGlobals::sleep(5000);
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
    GTUtilsDialog::waitForDialog(os, new RemoteDBDialogFillerDeprecated(os, "NC_001363", 0));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList()<<ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTGlobals::sleep();

    GTGlobals::sleep(20000);
    GTUtilsDocument::isDocumentLoaded(os, "NC_001363.gb");
    GTUtilsDocument::checkDocument(os, "NC_001363.gb", AnnotatedDNAViewFactory::ID);
}

GUI_TEST_CLASS_DEFINITION(test_0003) {
//    1. Select {File -> Access remote database} menu item in the main menu.

//    2. Fill the dialog:
//        Resource ID: NC_017775
//        Database: NCBI GenBank (DNA sequence)
//        Save to directory: any valid path
//        Force download the appropriate sequence: unchecked
//    and accept it.
//    Expected state: after the downloading task finish a new document appears in the project, it contains an annotation table only.

    QDir().mkpath(sandBoxDir + "remote_request/test_0003");
    QList<DownloadRemoteFileDialogFiller::Action> actions;

    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetResourceIds, QStringList() << "NC_017775");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "NCBI GenBank (DNA sequence)");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::EnterSaveToDirectoryPath, sandBoxDir + "remote_request/test_0003");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetForceSequenceDownload, false);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::ClickOk, "");

    GTUtilsDialog::waitForDialog(os, new DownloadRemoteFileDialogFiller(os, actions));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTGlobals::sleep();
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QSet<GObjectType> acceptableTypes;
    acceptableTypes.insert(GObjectTypes::ANNOTATION_TABLE);
    GTUtilsProjectTreeView::checkObjectTypes(os, acceptableTypes, GTUtilsProjectTreeView::findIndex(os, "NC_017775.gb"));
}

GUI_TEST_CLASS_DEFINITION(test_0004) {
//    1. Select {File -> Access remote database} menu item in the main menu.

//    2. Fill the dialog:
//        Resource ID: NC_017775
//        Database: NCBI GenBank (DNA sequence)
//        Save to directory: any valid path
//        Force download the appropriate sequence: checked
//    and accept it.
//    Expected state: after the downloading task finish a new document appears in the project, it contains both a sequence and an annotation table.

    QDir().mkpath(sandBoxDir + "remote_request/test_0004");
    QList<DownloadRemoteFileDialogFiller::Action> actions;

    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetResourceIds, QStringList() << "NC_017775");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "NCBI GenBank (DNA sequence)");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::EnterSaveToDirectoryPath, sandBoxDir + "remote_request/test_0004");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetForceSequenceDownload, true);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::ClickOk, "");

    GTUtilsDialog::waitForDialog(os, new DownloadRemoteFileDialogFiller(os, actions));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTGlobals::sleep();
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDocument::isDocumentLoaded(os, "NC_017775.gb");
    GTUtilsDocument::checkDocument(os, "NC_017775.gb", AnnotatedDNAViewFactory::ID);
}

GUI_TEST_CLASS_DEFINITION(test_0005) {
//    1. Select {File -> Access remote database} menu item in the main menu.

//    2. Fill the dialog:
//        Resource ID: NC_017775
//        Database: NCBI protein sequence database
//        Save to directory: any valid path
//        Force download the appropriate sequence: unchecked
//    and accept it.
//    Expected state: after the downloading task finish a new document appears in the project, it contains an annotation table only.

    QDir().mkpath(sandBoxDir + "remote_request/test_0005");
    QList<DownloadRemoteFileDialogFiller::Action> actions;

    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetResourceIds, QStringList() << "NC_017775");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "NCBI protein sequence database");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::EnterSaveToDirectoryPath, sandBoxDir + "remote_request/test_0005");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetForceSequenceDownload, false);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::ClickOk, "");

    GTUtilsDialog::waitForDialog(os, new DownloadRemoteFileDialogFiller(os, actions));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTGlobals::sleep();
    GTUtilsTaskTreeView::waitTaskFinished(os);

    QSet<GObjectType> acceptableTypes;
    acceptableTypes.insert(GObjectTypes::ANNOTATION_TABLE);
    GTUtilsProjectTreeView::checkObjectTypes(os, acceptableTypes, GTUtilsProjectTreeView::findIndex(os, "NC_017775.gb"));
}

GUI_TEST_CLASS_DEFINITION(test_0006) {
//    1. Select {File -> Access remote database} menu item in the main menu.

//    2. Fill the dialog:
//        Resource ID: NC_017775
//        Database: NCBI protein sequence database
//        Save to directory: any valid path
//        Force download the appropriate sequence: checked
//    and accept it.
//    Expected state: after the downloading task finish a new document appears in the project, it contains both a sequence and an annotation table.

    QDir().mkpath(sandBoxDir + "remote_request/test_0006");
    QList<DownloadRemoteFileDialogFiller::Action> actions;

    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetResourceIds, QStringList() << "NC_017775");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "NCBI protein sequence database");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::EnterSaveToDirectoryPath, sandBoxDir + "remote_request/test_0006");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetForceSequenceDownload, true);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::ClickOk, "");

    GTUtilsDialog::waitForDialog(os, new DownloadRemoteFileDialogFiller(os, actions));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTGlobals::sleep();
    GTUtilsTaskTreeView::waitTaskFinished(os);

    GTUtilsDocument::isDocumentLoaded(os, "NC_017775.gb");
    GTUtilsDocument::checkDocument(os, "NC_017775.gb", AnnotatedDNAViewFactory::ID);
}

GUI_TEST_CLASS_DEFINITION(test_0007) {
//    1. Select {File -> Access remote database} menu item in the main menu.

//    2. Check all databases.
//    Expected state: "Force download the appropriate sequence" checkbox is visible only for NCBI databases.
    QList<DownloadRemoteFileDialogFiller::Action> actions;

    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "NCBI GenBank (DNA sequence)");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::CheckForceSequenceDownloadVisibility, true);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "NCBI protein sequence database");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::CheckForceSequenceDownloadVisibility, true);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "ENSEMBL");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::CheckForceSequenceDownloadVisibility, false);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "PDB");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::CheckForceSequenceDownloadVisibility, false);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "SWISS-PROT");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::CheckForceSequenceDownloadVisibility, false);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "UniProtKB/Swiss-Prot");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::CheckForceSequenceDownloadVisibility, false);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "UniProtKB/TrEMBL");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::CheckForceSequenceDownloadVisibility, false);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "UniProt (DAS)");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::CheckForceSequenceDownloadVisibility, false);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::SetDatabase, "Ensembl Human Genes (DAS)");
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::CheckForceSequenceDownloadVisibility, false);
    actions << DownloadRemoteFileDialogFiller::Action(DownloadRemoteFileDialogFiller::ClickCancel, "");

    GTUtilsDialog::waitForDialog(os, new DownloadRemoteFileDialogFiller(os, actions));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_FILE), QStringList() << ACTION_PROJECTSUPPORT__ACCESS_REMOTE_DB, GTGlobals::UseKey);
    GTGlobals::sleep();
}

} // namespace
} // namespace U2
