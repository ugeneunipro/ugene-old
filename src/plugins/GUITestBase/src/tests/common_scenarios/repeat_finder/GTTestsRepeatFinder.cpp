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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QTreeWidgetItem>
#else
#include <QtWidgets/QTreeWidgetItem>
#endif

#include "GTTestsRepeatFinder.h"
#include <base_dialogs/GTFileDialog.h>
#include "primitives/GTMenu.h"
#include "GTUtilsDocument.h"
#include "GTUtilsAnnotationsTreeView.h"
#include <drivers/GTMouseDriver.h>
#include "GTUtilsProjectTreeView.h"
#include <primitives/GTTreeWidget.h>
#include "GTUtilsMdi.h"
#include "primitives/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/FindRepeatsDialogFiller.h"
#include <U2View/ADVConstants.h>

namespace U2 {

namespace GUITest_common_scenarios_repeat_finder {
using namespace HI;
GUI_TEST_CLASS_DEFINITION(test_0001) {
// The Test Runs Repeat Finder then checks if the qualifier "repeat homology" exists in resulting annotations
//
// Steps:
//
// 1. Use menu {File->Open}. Open file _common_data/fasta/seq4.fa.
    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "seq4.fa");
    GTUtilsDocument::checkDocument(os, "seq4.fa");

// 2. Run Find Repeats dialog
    Runnable * swDialog = new FindRepeatsDialogFiller(os, testDir + "_common_data/scenarios/sandbox/");
    GTUtilsDialog::waitForDialog(os, swDialog);

    GTMenu::clickMainMenuItem(os, QStringList() << "Actions" << "Analyze" << "Find repeats...", GTGlobals::UseMouse);
    GTGlobals::sleep();

// 3. Close sequence view, then reopen it
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTMouseDriver::click(os);

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, "seq4.fa"));
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep(1000);

// 4. Check that annotation have the qualifier "repeat homology"
    QTreeWidget *treeWidget = GTUtilsAnnotationsTreeView::getTreeWidget(os);
    CHECK_SET_ERR(treeWidget != NULL, "Tree widget is NULL");

    QTreeWidgetItem *annotationsRoot = GTUtilsAnnotationsTreeView::findItem(os, "repeat_unit  (0, 325)");
    GTMouseDriver::moveTo(os, GTTreeWidget::getItemCenter(os, annotationsRoot->child(0)));
    GTMouseDriver::doubleClick(os);
    GTUtilsAnnotationsTreeView::findItem(os, "repeat_homology(%)");

// 5. Close sequence view (it's needed to refresh screen since Ugene cannot close correctly on Win7 32bit)
    GTUtilsMdi::click(os, GTGlobals::Close);
    GTMouseDriver::click(os);
}

} // namespace GUITest_common_scenarios_repeat_finder

} // namespace U2
