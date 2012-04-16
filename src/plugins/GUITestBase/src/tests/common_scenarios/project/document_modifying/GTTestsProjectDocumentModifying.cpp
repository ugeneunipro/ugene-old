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

#include "api/GTMenu.h"
#include "GTTestsProjectDocumentModifying.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include "GTUtilsProject.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsDialog.h"
#include "api/GTFileDialog.h"
#include "api/GTKeyboardDriver.h"

#include <U2View/AnnotatedDNAViewFactory.h>


namespace U2{

namespace GUITest_common_scenarios_project_document_modifying{

GUI_TEST_CLASS_DEFINITION(test_0001) {
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/project/", "proj2-1.uprj");
    GTUtilsDocument::checkDocument(os, "1.gb");
    GTUtilsApp::checkUGENETitle(os, "proj2-1 UGENE");
    GTUtilsDialog::PopupChooser popupChooser(os, QStringList() << "action_load_selected_documents", GTGlobals::UseMouse);
    GTUtilsDialog::preWaitForDialog(os, &popupChooser, GUIDialogWaiter::Popup);
    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1.gb"));
    GTGlobals::sleep(2000);
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(100);
    GTUtilsProject::createAnnotation(os, "<auto>", "CCC", "1.. 10");
    QTreeWidgetItem *d = GTUtilsProjectTreeView::findItem(os, "1.gb");
    GTUtilsProjectTreeView::itemModificationCheck(os, d, true);
}

GUI_TEST_CLASS_DEFINITION(test_0002) {
   GTFileDialog::openFile(os, testDir+"_common_data/scenarios/project/", "proj2.uprj");
   GTUtilsDocument::checkDocument(os, "1.gb");
   GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");
   GTUtilsProject::exportProject(os, testDir + "_common_data/scenarios/sandbox");
   GTUtilsProject::closeProject(os);
   GTFileDialog::openFile(os, testDir+"_common_data/scenarios/sandbox/", "proj2.uprj");
   GTUtilsDocument::checkDocument(os, "1.gb");
   GTUtilsApp::checkUGENETitle(os, "proj2 UGENE");
   GTUtilsDialog::PopupChooser popupChooser(os, QStringList() << "action_load_selected_documents", GTGlobals::UseMouse);
   GTUtilsDialog::preWaitForDialog(os, &popupChooser, GUIDialogWaiter::Popup);
   GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "1.gb"));
   GTGlobals::sleep(1000);
   GTMouseDriver::click(os, Qt::RightButton);
   GTGlobals::sleep(100);
   GTUtilsProject::createAnnotation(os, "<auto>", "misc_feature", "complement(1.. 20)");
   GTGlobals::sleep(100);
   QTreeWidgetItem *d = GTUtilsProjectTreeView::findItem(os, "MyDocument.gb");
   GTUtilsProjectTreeView::itemModificationCheck(os, d, true);
   GTUtilsProject::CloseProjectSettings s;
   s.saveOnCloseButton = QMessageBox::Yes;
   GTUtilsProject::closeProject(os, s);
   GTFileDialog::openFile(os, testDir+"_common_data/scenarios/sandbox/", "proj2.uprj");
   GTUtilsDocument::checkDocument(os, "MyDocument.gb");
}

}

}


