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

#include "GTTestsProjectDocumentModifying.h"
#include "api/GTMenu.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include "GTUtilsProject.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsDialogRunnables.h"
#include "api/GTFileDialog.h"
#include "api/GTKeyboardDriver.h"

#include <U2View/AnnotatedDNAViewFactory.h>

namespace U2{

namespace GUITest_common_scenarios_project_document_modifying{

GUI_TEST_CLASS_DEFINITION(test_0001) {
// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2-1.uprj
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/project/", "proj2-1.uprj");

// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");

//     2) UGENE window titled with text "proj2-1 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2-1 UGENE");

// 2. Open view for 1.gb document.
    Runnable *popupChooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << "action_load_selected_documents", GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, popupChooser, GUIDialogWaiter::Popup);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1.gb"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Press Ctrl+N and add annotation to it annotations table.
    Runnable *filler = new GTUtilsDialogRunnables::CreateAnnotationDialogFiller(os, "<auto>", "CCC", "1.. 10");
    GTUtilsDialog::waitForDialog(os, filler);
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    GTGlobals::sleep();
// Expected state: in project view 1.gb document has marked as modified (with blue color)
    QTreeWidgetItem *d = GTUtilsProjectTreeView::findItem(os, "1.gb");
    GTUtilsProjectTreeView::itemModificationCheck(os, d, true);
}

GUI_TEST_CLASS_DEFINITION(test_0002) {

// 1. Use menu {File->Open}. Open project _common_data/scenarios/project/proj2.uprj
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/project/", "proj2.uprj");

// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window has title "proj2 UGENE"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 2. Use menu {File->Export Project}
// Expected state: "Export project" dialog has appeared
// 
// 3. Fill the next field in dialog and click Save button:
//     {Project Folder:} _common_data/scenarios/sandbox
    Runnable *filler = new GTUtilsDialogRunnables::ExportProjectDialogFiller(os, testDir + "_common_data/scenarios/sandbox");
    GTUtilsDialog::waitForDialog(os, filler);
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__EXPORT_PROJECT);
    GTGlobals::sleep();

// 4. Close project. No questions must be asked
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTGlobals::sleep();
    GTUtilsProject::checkProject(os, GTUtilsProject::NotExists);

// 5. Open exported project
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/sandbox/", "proj2.uprj");
// Expected state: 
//     1) Project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");
//     2) UGENE window has title "proj2"
    GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");

// 6. Load 1.gb document using context menu.
    Runnable *popupChooser = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << "action_load_selected_documents", GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, popupChooser, GUIDialogWaiter::Popup);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1.gb"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Press Ctrl+N and add annotation "misc_feature" to the annotations table in 1.gb document.
    Runnable *filler2 = new GTUtilsDialogRunnables::CreateAnnotationDialogFiller(os, "<auto>", "misc_feature", "complement(1.. 20)");
    GTUtilsDialog::waitForDialog(os, filler2);
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

// Expected state: in project view 1.gb document has marked as modified (with blue color)
    GTGlobals::sleep();
    QTreeWidgetItem *d = GTUtilsProjectTreeView::findItem(os, "1.gb");
    GTUtilsProjectTreeView::itemModificationCheck(os, d, true);

// 7. Use menu {File->Close Project}
// Expected state: "Question?" dialog has appeared that proposes to save 1.gb file
// 8. Click Yes button
    Runnable *filler3 = new GTUtilsDialogRunnables::MessageBoxDialogFiller(os, QMessageBox::Yes);
    GTUtilsDialog::waitForDialog(os, filler3, GUIDialogWaiter::Modal);
    GTMenu::clickMenuItem(os, GTMenu::showMainMenu(os, MWMENU_FILE), ACTION_PROJECTSUPPORT__CLOSE_PROJECT);
    GTGlobals::sleep();

// 9. Load _common_data/scenarios/sandbox/proj2.uprj
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/sandbox/", "proj2.uprj");
// Expected state: project view with document "1.gb" has been opened
    GTUtilsDocument::checkDocument(os, "1.gb");

// 10) Open 1.gb view by context menu
    Runnable *popupChooser2 = new GTUtilsDialogRunnables::PopupChooser(os, QStringList() << "action_load_selected_documents", GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, popupChooser2, GUIDialogWaiter::Popup);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1.gb"));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep();

// Expected state: annotation added at step 6 already presents in document
    GTGlobals::sleep();
    QTreeWidgetItem* ann = GTUtilsAnnotationsTreeView::findItem(os, "misc_feature");
    CHECK_SET_ERR(ann != NULL, "There is no annotation");
}

}

}
