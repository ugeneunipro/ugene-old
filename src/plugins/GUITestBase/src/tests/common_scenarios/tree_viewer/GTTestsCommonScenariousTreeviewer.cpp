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

#include "GTTestsCommonScenariousTreeviewer.h"

#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTGlobals.h"
#include "api/GTAction.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsMdi.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/BuildTreeDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/LicenseAgreemntDialogFiller.h"
#include <QGraphicsItem>
#include <U2Core/AppContext.h>
#include <QGraphicsView>

#include <U2View/MSAEditor.h>

namespace U2 {

namespace GUITest_common_scenarios_tree_viewer {

GUI_TEST_CLASS_DEFINITION(test_0002){
//Rebuilding tree after removing tree file

//1. Open file samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os,dataDir + "samples/CLUSTALW/", "COI.aln");
    GTGlobals::sleep(500);
//2. Click on "Build tree" button on toolbar "Build Tree"
//Expected state: "Create Philogenetic Tree" dialog appears
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk"));
    GTUtilsDialog::waitForDialog(os,new LicenseAgreemntDialogFiller(os));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep(500);
//3. Set save path to _common_data/scenarios/sandbox/COI.nwk Click  OK button
//Expected state: philogenetic tree appears
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
    GTGlobals::sleep();
//4. Remove document "COI.nwk" from project view.
    //GTUtilsDialog::waitForDialog(os,new MessageBoxDialogFiller(os,QMessageBox::No));
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"COI.nwk"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    GTGlobals::sleep(500);
    QWidget* w = GTWidget::findWidget(os, "treeView",NULL,GTGlobals::FindOptions(false));

    QTreeWidgetItem* item = GTUtilsProjectTreeView::findItem(os,GTUtilsProjectTreeView::getTreeWidget(os),"COI.nwk",GTGlobals::FindOptions(false));
    CHECK_SET_ERR(item==NULL,"COI.nkw is not deleted");
//Expected state: document "COI.nwk" not presents at project tree, tree editor view window closes

//5. Double click on COI object.
//Expected state: MSA editor view window opens
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"COI.aln"));
    GTGlobals::sleep(500);
    GTMouseDriver::doubleClick(os);
//6. Click on "Build tree" button on toolbar
//Expected state: "Create Philogenetic Tree" dialog appears
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os,testDir + "_common_data/scenarios/sandbox/COI.nwk"));
    GTGlobals::sleep(500);
    tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
//7. Click  OK button
//Expected state: philogenetic tree appears
    GTGlobals::sleep(500);
    QWidget* w1 = GTWidget::findWidget(os, "treeView");
    CHECK_SET_ERR(w1!=NULL,"treeView not found");
}

GUI_TEST_CLASS_DEFINITION(test_0002_1){
//Rebuilding tree after removing tree file
//1. Open file samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os,dataDir + "samples/CLUSTALW/", "COI.aln");
    GTGlobals::sleep(500);
//2. Click on "Build tree" button on toolbar "Build Tree"
//Expected state: "Create Philogenetic Tree" dialog appears
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk"));
    GTUtilsDialog::waitForDialog(os,new LicenseAgreemntDialogFiller(os));
    //DIFFERENCE: Context menu is used for building tree
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_TREES<<"Build Tree"));

    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
    GTGlobals::sleep(500);
//3. Set save path to _common_data/scenarios/sandbox/COI.nwk Click  OK button
//Expected state: philogenetic tree appears
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
    GTGlobals::sleep();
//4. Remove document "COI.nwk" from project view.
    //GTUtilsDialog::waitForDialog(os,new MessageBoxDialogFiller(os,QMessageBox::No));
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"COI.nwk"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    GTGlobals::sleep(500);

    QWidget* w = GTWidget::findWidget(os, "treeView",NULL,GTGlobals::FindOptions(false));
    CHECK_SET_ERR(w==0, "treeView not deleted")

    QTreeWidgetItem* item = GTUtilsProjectTreeView::findItem(os,GTUtilsProjectTreeView::getTreeWidget(os),"COI.nwk",GTGlobals::FindOptions(false));
    CHECK_SET_ERR(item==NULL,"COI.nkw is not deleted");
//Expected state: document "COI.nwk" not presents at project tree, tree editor view window closes

//5. Double click on COI object.
//Expected state: MSA editor view window opens
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"COI.aln"));
    GTGlobals::sleep(500);
    GTMouseDriver::doubleClick(os);
//6. Click on "Build tree" button on toolbar
//Expected state: "Create Philogenetic Tree" dialog appears
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os,testDir + "_common_data/scenarios/sandbox/COI.nwk"));
    //DIFFERENCE: Context menu is used for building tree
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_TREES<<"Build Tree"));

    GTGlobals::sleep(500);
    GTMenu::showContextMenu(os, GTUtilsMdi::activeWindow(os));
//7. Click  OK button
//Expected state: philogenetic tree appears
    GTGlobals::sleep(500);
    QWidget* w1 = GTWidget::findWidget(os, "treeView");
    CHECK_SET_ERR(w1!=NULL,"treeView not found");
}

GUI_TEST_CLASS_DEFINITION(test_0002_2){
//Rebuilding tree after removing tree file
//1. Open file samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os,dataDir + "samples/CLUSTALW/", "COI.aln");
    GTGlobals::sleep(500);
//2. Click on "Build tree" button on toolbar "Build Tree"
//Expected state: "Create Philogenetic Tree" dialog appears
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk"));
    GTUtilsDialog::waitForDialog(os,new LicenseAgreemntDialogFiller(os));
    //DIFFERENCE: Main menu is used for building tree
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_TREES<<"Build Tree"));

    GTMenu::showMainMenu(os,MWMENU_ACTIONS);
    GTGlobals::sleep(500);
//3. Set save path to _common_data/scenarios/sandbox/COI.nwk Click  OK button
//Expected state: philogenetic tree appears
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found")
    GTGlobals::sleep();
//4. Remove document "COI.nwk" from project view.
    //GTUtilsDialog::waitForDialog(os,new MessageBoxDialogFiller(os,QMessageBox::No));
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"COI.nwk"));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);

    GTGlobals::sleep(500);

    QWidget* w = GTWidget::findWidget(os, "treeView",NULL,GTGlobals::FindOptions(false));
    CHECK_SET_ERR(w==0, "treeView not deleted")

    QTreeWidgetItem* item = GTUtilsProjectTreeView::findItem(os,GTUtilsProjectTreeView::getTreeWidget(os),"COI.nwk",GTGlobals::FindOptions(false));
    CHECK_SET_ERR(item==NULL,"COI.nkw is not deleted");
//Expected state: document "COI.nwk" not presents at project tree, tree editor view window closes

//5. Double click on COI object.
//Expected state: MSA editor view window opens
    GTMouseDriver::moveTo(os,GTUtilsProjectTreeView::getItemCenter(os,"COI.aln"));
    GTGlobals::sleep(500);
    GTMouseDriver::doubleClick(os);
//6. Click on "Build tree" button on toolbar
//Expected state: "Create Philogenetic Tree" dialog appears
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os,testDir + "_common_data/scenarios/sandbox/COI.nwk"));
    //DIFFERENCE: Main menu is used for building tree
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_TREES<<"Build Tree"));

    GTGlobals::sleep(500);
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
//7. Click  OK button
//Expected state: philogenetic tree appears
    GTGlobals::sleep(500);
    QWidget* w1 = GTWidget::findWidget(os, "treeView");
    CHECK_SET_ERR(w1!=NULL,"treeView not found");
}
} // namespace GUITest_common_scenarios_tree_viewer
} // namespace U2
