/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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
#include "GTUtilsLog.h"
#include "GTUtilsBookmarksTreeView.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/BuildTreeDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/BranchSettingsDialogFiller.h"
#include "runnables/ugene/corelibs/U2Gui/ExportImageDialogFiller.h"
#include "runnables/ugene/plugins_3rdparty/umuscle/MuscleDialogFiller.h"
#include "runnables/ugene/corelibs/U2View/ov_msa/LicenseAgreemntDialogFiller.h"
#include <QGraphicsItem>
#include <U2Core/AppContext.h>
#include <QGraphicsView>
#include <QColor>
#include <QRgb>

#include <U2View/MSAEditor.h>
#include <U2View/GraphicsRectangularBranchItem.h>
#include <U2View/GraphicsButtonItem.h>

namespace U2 {

namespace GUITest_common_scenarios_tree_viewer {
GUI_TEST_CLASS_DEFINITION(test_0001){
//Screenshoting MSA editor (regression test)

//1. Open file samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os,dataDir + "samples/CLUSTALW/", "COI.aln");
    GTGlobals::sleep(500);
//2. Click on "Build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk"));
    GTUtilsDialog::waitForDialog(os,new LicenseAgreemntDialogFiller(os));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep(500);
//Expected state: "Create Philogenetic Tree" dialog appears

//3. Set save path to _common_data/scenarios/sandbox/COI.nwk . Click  OK button
//Expected state: philogenetic tree appears

//4. Use "Capture tree" button on toolbar to make screenshots
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Screen Capture"));
    GTUtilsDialog::waitForDialog(os, new ExportImage(os,testDir + "_common_data/scenarios/sandbox/image.svg", 4,50));
    GTWidget::click(os,GTWidget::findWidget(os,"cameraMenu"));

    GTFileDialog::getSize(os,testDir + "_common_data/scenarios/sandbox/","image.jpeg");
//Expected state: images on screenshots same as on your screen
}

GUI_TEST_CLASS_DEFINITION(test_0001_1){
//Screenshoting MSA editor (regression test)

//1. Open file samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os,dataDir + "samples/CLUSTALW/", "COI.aln");
    GTGlobals::sleep(500);
//2. Click on "Build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk"));
    GTUtilsDialog::waitForDialog(os,new LicenseAgreemntDialogFiller(os));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep(500);
//Expected state: "Create Philogenetic Tree" dialog appears

//3. Set save path to _common_data/scenarios/sandbox/COI.nwk . Click  OK button
//Expected state: philogenetic tree appears

//4. Use "Capture tree" button on toolbar to make screenshots
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Export Tree Image"<<"Screen Capture"));
    GTUtilsDialog::waitForDialog(os, new ExportImage(os,testDir + "_common_data/scenarios/sandbox/image.svg", 4,50));
    GTMenu::showContextMenu(os, GTWidget::findWidget(os,"treeView"));

    GTFileDialog::getSize(os,testDir + "_common_data/scenarios/sandbox/","image.jpeg");
//Expected state: images on screenshots same as on your screen
}

GUI_TEST_CLASS_DEFINITION(test_0001_2){
//Screenshoting MSA editor (regression test)

//1. Open file samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os,dataDir + "samples/CLUSTALW/", "COI.aln");
    GTGlobals::sleep(500);
//2. Click on "Build tree" button on toolbar
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk"));
    GTUtilsDialog::waitForDialog(os,new LicenseAgreemntDialogFiller(os));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep(500);
//Expected state: "Create Philogenetic Tree" dialog appears

//3. Set save path to _common_data/scenarios/sandbox/COI.nwk . Click  OK button
//Expected state: philogenetic tree appears

//4. Use "Capture tree" button on toolbar to make screenshots
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Export Tree Image"<<"Screen Capture"));
    GTUtilsDialog::waitForDialog(os, new ExportImage(os,testDir + "_common_data/scenarios/sandbox/image.svg", 4,50));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);

    GTFileDialog::getSize(os,testDir + "_common_data/scenarios/sandbox/","image.jpeg");
//Expected state: images on screenshots same as on your screen
}

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

GUI_TEST_CLASS_DEFINITION(test_0003){
//    Building tree with specific parameters
//    1. Open file samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os,dataDir + "samples/CLUSTALW/", "COI.aln");
    GTLogTracer l;
    GTGlobals::sleep(500);
//    2. Click on "Build tree" button on toolbar
//    Expected state: "Create Philogenetic Tree" dialog appears

    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk",2,52));
    GTUtilsDialog::waitForDialog(os,new LicenseAgreemntDialogFiller(os));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep(500);

//    3. Fill next fields in dialog:
//    {Distance matrix model:} jukes-cantor
//    {Gamma distributed rates across sites} [checked]
//    {Coefficient of variation of substitution rat among sites:} 99
//    {Path to file:} _common_data/scenarios/sandbox/COI.nwk

//    4. Click  OK button
    CHECK_SET_ERR(l.hasError(), "there is no error it the log");
//    Expected state: no crash, philogenetic tree not appears
//    Error message in the log: "Calculated weight matrix is invalid"
}

GUI_TEST_CLASS_DEFINITION(test_0004){
//    Disabling views
//    1. Open file samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os,dataDir + "samples/CLUSTALW/", "COI.aln");
    GTGlobals::sleep(500);
//    2. Click on "Build tree" button on toolbar
//    Expected state: "Create Philogenetic Tree" dialog appears
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk"));
    GTUtilsDialog::waitForDialog(os,new LicenseAgreemntDialogFiller(os));
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep();
//    3. Set save path to _common_data/scenarios/sandbox/COI.nwk Click  OK button
//    Expected state: philogenetic tree appears
//    4. Disable "Show sequence name"
//    Expected state: sequence name labels are not shown
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Show Names"));
    GTWidget::click(os,GTWidget::findWidget(os,"Show Labels"));

    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    QList<QGraphicsItem*> list = treeView->scene()->items();

    foreach(QGraphicsItem* item, list){
        QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if(node && node->isVisible()){
            CHECK_SET_ERR(!node->text().contains("o")||!node->text().contains("a"), "names are visiable");
        }
    }
//    5. Disable "Show distance labels"
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Show Distances"));
    GTWidget::click(os,GTWidget::findWidget(os,"Show Labels"));

    foreach(QGraphicsItem* item, list){
            QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
            if(node && node->isVisible()){
                if(node->text()!="0.011"){
                    CHECK_SET_ERR(!node->text().contains("0."), "Distances are visiable");
                }
            }
        }
//    Expected state: distance labels are not shown
}

GUI_TEST_CLASS_DEFINITION(test_0005){
//Align with muscle, then build tree
//1. Open file samples/CLUSTALW/COI.aln
    GTFileDialog::openFile(os,dataDir + "samples/CLUSTALW/", "COI.aln");
    GTGlobals::sleep(500);
//2. Do menu {Actions->Align->Align With Muscle}
//Expected state: "Align with muscle" dialog appears
    GTUtilsDialog::waitForDialog(os, new BuildTreeDialogFiller(os, testDir + "_common_data/scenarios/sandbox/COI.nwk"));
    GTUtilsDialog::waitForDialog(os, new LicenseAgreemntDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<MSAE_MENU_ALIGN<<"Align with muscle", GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new MuscleDialogFiller(os));

    GTMenu::showMainMenu(os,MWMENU_ACTIONS);
//3. Click "Align" button
//4. Click on "Build tree" button on toolbar
    QAbstractButton *tree= GTAction::button(os,"Build Tree");
    GTWidget::click(os,tree);
    GTGlobals::sleep(1000);
//Expected state: "Create Philogenetic Tree" dialog appears
//5. Set save path to _common_data/scenarios/sandbox/COI.nwk Click  OK button
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    CHECK_SET_ERR(treeView!=NULL,"TreeView not found");
//Expected state: philogenetic tree appears
}
int getCoord(U2OpStatus &os, QGraphicsSimpleTextItem *node){
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    QPointF sceneCoord = node->mapToScene(node->boundingRect().bottomLeft());
    QPoint globalCoord = treeView->mapToGlobal(sceneCoord.toPoint());
    return globalCoord.y();
}
/*GUI_TEST_CLASS_DEFINITION(test_0006){
//    Tree layouts test
//    1. Open file _common_data/scenario/tree_view/COI.nwk
//    Expected state: philogenetic tree appears
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/tree_view/", "COI.nwk");
    GTGlobals::sleep(500);
//    2. Click on {Layout->Circilar layout} button on toolbar
    //GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Circular"));
    //GTWidget::click(os,GTWidget::findWidget(os,"Layout"));
//    Expected state: tree view type changed to circular
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    QList<QGraphicsItem*> list = treeView->scene()->items();

    QString s;
    QGraphicsSimpleTextItem * highestNode = new QGraphicsSimpleTextItem();
    QGraphicsSimpleTextItem * lowestNode = new QGraphicsSimpleTextItem();

    foreach(QGraphicsItem* item, list){
        QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if(node && node->text().length()>5){
            if(getCoord(os, highestNode)<getCoord(os, node)){highestNode = node;}
            if(getCoord(os, lowestNode)>getCoord(os, node)){lowestNode = node;}

            //s.append("\n" + node->text() + "  " + QString().setNum(globalCoord.y()));
            //item->moveBy(200,0);
            //GTGlobals::sleep(500);
        }
    }
//    3. Click on {Layout->Unrooted layout} button on toolbar
//    Expected state: tree view type changed to unrooted
    s.append("\n" + highestNode->text() + "  " + QString().setNum(getCoord(os, highestNode)));
    s.append("\n" + lowestNode->text() + "  " + QString().setNum(getCoord(os, lowestNode)));
    os.setError(s);
//    4. Click on {Layout->Rectangular layout} button on toolbar
//    Expected state: tree view type changed to rectangular
}*/

GUI_TEST_CLASS_DEFINITION(test_0007){
//Labels aligniment test

//1. Open file _common_data/scenario/tree_view/COI.nwk
//Expected state: philogenetic tree appears
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/tree_view/", "COI.nwk");
    GTGlobals::sleep(500);

    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    QList<QGraphicsItem*> list = treeView->scene()->items();

    QList<int> initPos;
    foreach(QGraphicsItem* item, list){
        QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if(node && node->boundingRect().width()>100){
            QPointF sceneCoord = node->mapToScene(node->boundingRect().bottomRight());
            QPoint globalCoord = treeView->mapToGlobal(sceneCoord.toPoint());
            initPos.append(globalCoord.x());
        }
    }
//2. Click on "Align name labels" button on toolbar
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Align Labels"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(500);
    //GTWidget::click(os, GTAction::button(os,"Align Labels"));

    int i = 0;
    foreach(QGraphicsItem* item, list){
        QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if(node && node->boundingRect().width()>100 && i==0){
            QPointF sceneCoord = node->mapToScene(node->boundingRect().bottomRight());
            QPoint globalCoord = treeView->mapToGlobal(sceneCoord.toPoint());
            i=globalCoord.x();
        }
        if(node && node->boundingRect().width()>100){
            QPointF sceneCoord = node->mapToScene(node->boundingRect().bottomRight());
            QPoint globalCoord = treeView->mapToGlobal(sceneCoord.toPoint());
            CHECK_SET_ERR(i == globalCoord.x(), "elements are not aligned");
        }
    }
//Expected state: sequence labels aligned at right side of the screen

//3. Click on "Align name labels" button on toolbar
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Align Labels"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS);
    GTGlobals::sleep(500);

    QList<int> finalPos;
    foreach(QGraphicsItem* item, list){
        QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if(node && node->boundingRect().width()>100){
            QPointF sceneCoord = node->mapToScene(node->boundingRect().bottomRight());
            QPoint globalCoord = treeView->mapToGlobal(sceneCoord.toPoint());
            finalPos.append(globalCoord.x());
        }
    }
    CHECK_SET_ERR(initPos==finalPos, "items aligned wrong");
//Expected state: sequence label aligned near end of its branches
}

GUI_TEST_CLASS_DEFINITION(test_0008){
//Sequence labels test

//1. Open file _common_data/scenario/tree_view/COI.nwk
//Expected state: philogenetic tree appears
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/tree_view/", "COI.nwk");
    GTGlobals::sleep(500);
//2. Click on "Show sequence names" button on toolbar
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Show Names"));
    GTWidget::click(os,GTWidget::findWidget(os,"Show Labels"));

    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    QList<QGraphicsItem*> list = treeView->scene()->items();

    foreach(QGraphicsItem* item, list){
        QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if(node && node->isVisible()){
            CHECK_SET_ERR(!node->text().contains("o")||!node->text().contains("a"), "names are visiable");
        }
    }
//Expected state: sequence name labels disappers

//3. Click on "Show distance labels" button on toolbar
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Show Distances"));
    GTWidget::click(os,GTWidget::findWidget(os,"Show Labels"));

    foreach(QGraphicsItem* item, list){
            QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
            if(node && node->isVisible()){
                if(node->text()!="0.011"){
                    CHECK_SET_ERR(!node->text().contains("0."), "Distances are visiable");
                }
            }
        }
//Expected state: distance labels disappers

//4. Click on "Show sequence names" button on toolbar
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Show Names"));
    GTWidget::click(os,GTWidget::findWidget(os,"Show Labels"));
    GTGlobals::sleep(200);
    int i=0;

    QString s;
    foreach(QGraphicsItem* item, list){
        QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if(node){
            s.append("\n" + node->text());
        }
        if(node && node->isVisible()&&node->text().contains("o")){
            i++;
        }
    }
    //CHECK_SET_ERR(i==18, "Names are not shown" + QString().setNum(i) + s);
//Expected state: sequence name labels appers

//5. Click on "Show distance labels" button on toolbar
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Show Distances"));
    GTWidget::click(os,GTWidget::findWidget(os,"Show Labels"));
    GTGlobals::sleep(200);
    i=0;

    QString s1;
    foreach(QGraphicsItem* item, list){
            QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
            if(node && node->isVisible() && node->text().contains("0.")){
                s1.append("\n" + node->text());
                i++;
            }
        }
    CHECK_SET_ERR(i==31, "distances are not shown" + QString().setNum(i) + s1);
//Expected state: distance labels appers
}

GUI_TEST_CLASS_DEFINITION(test_0008_1){//difference: main menu is used
//Sequence labels test

//1. Open file _common_data/scenario/tree_view/COI.nwk
//Expected state: philogenetic tree appears
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/tree_view/", "COI.nwk");
    GTGlobals::sleep(500);
//2. Click on "Show sequence names" button on toolbar
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"show_labels_action"<<"Show Names"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS,GTGlobals::UseMouse);

    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    QList<QGraphicsItem*> list = treeView->scene()->items();

    foreach(QGraphicsItem* item, list){
        QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if(node && node->isVisible()){
            CHECK_SET_ERR(!node->text().contains("o")||!node->text().contains("a"), "names are visiable");
        }
    }
//Expected state: sequence name labels disappers

//3. Click on "Show distance labels" button on toolbar
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"show_labels_action"<<"Show Distances"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS,GTGlobals::UseMouse);

    foreach(QGraphicsItem* item, list){
            QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
            if(node && node->isVisible()){
                if(node->text()!="0.011"){
                    CHECK_SET_ERR(!node->text().contains("0."), "Distances are visiable");
                }
            }
        }
//Expected state: distance labels disappers

//4. Click on "Show sequence names" button on toolbar
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"show_labels_action"<<"Show Names"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS,GTGlobals::UseMouse);
    GTGlobals::sleep(200);
    int i=0;

    foreach(QGraphicsItem* item, list){
        QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if(node && node->isVisible()&&node->text().contains("o")){
            i++;
        }
    }
    //CHECK_SET_ERR(i==18, "Names are not shown");
//Expected state: sequence name labels appers

//5. Click on "Show distance labels" button on toolbar
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"show_labels_action"<<"Show Distances"));
    GTMenu::showMainMenu(os, MWMENU_ACTIONS,GTGlobals::UseMouse);
    GTGlobals::sleep(200);
    i=0;

    foreach(QGraphicsItem* item, list){
            QGraphicsSimpleTextItem * node = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
            if(node && node->isVisible() && node->text().contains("0.")){
                i++;
            }
        }
    CHECK_SET_ERR(i==31, "distances are not shown");
//Expected state: distance labels appers
}

GUI_TEST_CLASS_DEFINITION(test_0009){
//UGENE crashes when tree view bookmark is activated (0001431)

//1. Open Newick file (.NWK)
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/tree_view/", "COI.nwk");
    GTGlobals::sleep(500);
//2. Create new bookmark for the file
    QPoint p = GTUtilsBookmarksTreeView::getItemCenter(os, "COI [tr] Tree");
    GTMouseDriver::moveTo(os, p);
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_ADD_BOOKMARK, GTGlobals::UseMouse));
    GTMouseDriver::click(os, Qt::RightButton);
    GTGlobals::sleep(500);

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(500);
    GTKeyboardDriver::keySequence(os, "start bookmark");
    GTGlobals::sleep(500);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep(500);

    GTUtilsMdi::click(os, GTGlobals::Close);

    p = GTUtilsBookmarksTreeView::getItemCenter(os, "start bookmark");
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::doubleClick(os);
    GTGlobals::sleep(500);

    QWidget* treeView = GTWidget::findWidget(os,"treeView");
    CHECK_SET_ERR(treeView!=NULL, "treeView not found");
//3. Close the opened view

//4. Activate bookmark
//Expected state: UGENE not crash
}

GUI_TEST_CLASS_DEFINITION(test_0010){
//PhyTree branch settings

//1. Open file _common_data/scenario/tree_view/COI.nwk
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/tree_view/", "COI.nwk");
    GTGlobals::sleep(500);
//Expected state: philogenetic tree appears

//2. Open context menu on branch and  select {change settings} menu item
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    QList<QGraphicsItem*> list = treeView->scene()->items();
    QList<QGraphicsItem*> nodeList;

    foreach(QGraphicsItem* item, list){
        if(item->boundingRect().width()==10){
            nodeList.append(item);
        }
    }

    QGraphicsItem* node = nodeList.last();
    QPointF sceneCoord = node->mapToScene(node->boundingRect().center());
    QPoint viewCord = treeView->mapFromScene(sceneCoord);
    QPoint globalCoord = treeView->mapToGlobal(viewCord);

    GTUtilsDialog::waitForDialog(os, new BranchSettingsDialogFiller(os));
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Branch Settings"));
    GTMouseDriver::moveTo(os, globalCoord);
    GTMouseDriver::click(os);
    GTMouseDriver::click(os,Qt::RightButton);


    globalCoord.setX(globalCoord.x()-10);
    QPixmap content;
    content = QPixmap::grabWidget(treeView,treeView->rect());

    QRgb rgb = content.toImage().pixel(treeView->mapFromGlobal(globalCoord));
    QColor color(rgb);

    CHECK_SET_ERR(color.name()=="#0000ff","Expected: #0000ff, found: " + color.name());

//Expected state: Branch settings dialog appears

//3. Change thickness and collor to differ than standard. Click OK
//Expected state: selected branch changed

//4. Select bunch of branches with holding shift button. Perform  steps 2,3 for it.
}

GUI_TEST_CLASS_DEFINITION(test_0011){
//    Collapse/expand action in phylogenetic tree (0002168)

//    1. Open file _common_data/scenario/tree_view/COI.nwk
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/tree_view/", "COI.nwk");
    GTGlobals::sleep(500);
//    Expected state: philogenetic tree appears
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    QList<QGraphicsItem*> list = treeView->scene()->items();
    QList<QGraphicsItem*> nodeList;

    foreach(QGraphicsItem* item, list){
        if(item->boundingRect().width()==10){
            nodeList.append(item);
        }
    }

    QGraphicsItem* node = nodeList.last();
    QPointF sceneCoord = node->mapToScene(node->boundingRect().center());
    QPoint viewCord = treeView->mapFromScene(sceneCoord);
    QPoint globalCoord = treeView->mapToGlobal(viewCord);
//    2. Do context menu {Collapse} for any node
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Collapse"));
    GTMouseDriver::moveTo(os, globalCoord);
    GTMouseDriver::click(os);
    GTMouseDriver::click(os,Qt::RightButton);

    QList<QGraphicsSimpleTextItem *> branchList;
    foreach(QGraphicsItem* item, list){
            QGraphicsSimpleTextItem * textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
            if(textItem && (textItem->text().contains("0.052") || textItem->text().contains("0.045")
                   || textItem->text().contains("bicolor") || textItem->text().contains("roeseli"))){
                branchList.append(textItem);
            }
        }

    foreach(QGraphicsSimpleTextItem* item, branchList){
        CHECK_SET_ERR(!item->isVisible(), item->text() + " is visiable");
    }
//    Expected state: this node's branches has dissapered

//    3. Do context menu {Expand} for same
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os,QStringList()<<"Collapse"));
    GTMouseDriver::moveTo(os, globalCoord);
    GTMouseDriver::click(os,Qt::RightButton);

    foreach(QGraphicsSimpleTextItem* item, branchList){
        CHECK_SET_ERR(item->isVisible(), item->text() + " is not visiable");
    }
//    Expected state: this node's branches has dissapered
}

GUI_TEST_CLASS_DEFINITION(test_0011_1){
//    Collapse/expand action in phylogenetic tree (0002168)

//    1. Open file _common_data/scenario/tree_view/COI.nwk
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/tree_view/", "COI.nwk");
    GTGlobals::sleep(500);
//    Expected state: philogenetic tree appears
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    QList<QGraphicsItem*> list = treeView->scene()->items();
    QList<QGraphicsItem*> nodeList;

    foreach(QGraphicsItem* item, list){
        if(item->boundingRect().width()==10){
            nodeList.append(item);
        }
    }

    QGraphicsItem* node = nodeList.last();
    QPointF sceneCoord = node->mapToScene(node->boundingRect().center());
    QPoint viewCord = treeView->mapFromScene(sceneCoord);
    QPoint globalCoord = treeView->mapToGlobal(viewCord);
//    2. Do context menu {Collapse} for any node
    GTMouseDriver::moveTo(os, globalCoord);
    GTMouseDriver::doubleClick(os);

    QList<QGraphicsSimpleTextItem *> branchList;
    foreach(QGraphicsItem* item, list){
            QGraphicsSimpleTextItem * textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
            if(textItem && (textItem->text().contains("0.052") || textItem->text().contains("0.045")
                   || textItem->text().contains("bicolor") || textItem->text().contains("roeseli"))){
                branchList.append(textItem);
            }
        }

    foreach(QGraphicsSimpleTextItem* item, branchList){
        CHECK_SET_ERR(!item->isVisible(), item->text() + " is visiable");
    }
//    Expected state: this node's branches has dissapered

//    3. Do context menu {Expand} for same
    GTMouseDriver::moveTo(os, globalCoord);
    GTMouseDriver::doubleClick(os);

    foreach(QGraphicsSimpleTextItem* item, branchList){
        CHECK_SET_ERR(item->isVisible(), item->text() + " is not visiable");
    }
//    Expected state: this node's branches has dissapered
}

GUI_TEST_CLASS_DEFINITION(test_0011_2){
//    Collapse/expand action in phylogenetic tree (0002168)

//    1. Open file _common_data/scenario/tree_view/COI.nwk
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/tree_view/", "COI.nwk");
    GTGlobals::sleep(500);
//    Expected state: philogenetic tree appears
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    QList<QGraphicsItem*> list = treeView->scene()->items();
    QList<QGraphicsItem*> nodeList;

    foreach(QGraphicsItem* item, list){
        if(item->boundingRect().width()==10){
            nodeList.append(item);
        }
    }

    QGraphicsItem* node = nodeList.first();
    QPointF sceneCoord = node->mapToScene(node->boundingRect().center());
    QPoint viewCord = treeView->mapFromScene(sceneCoord);
    QPoint globalCoord = treeView->mapToGlobal(viewCord);
//    2. Do context menu {Collapse} for any node
    GTMouseDriver::moveTo(os, globalCoord);
    GTMouseDriver::doubleClick(os);

    QList<QGraphicsSimpleTextItem *> branchList;
    foreach(QGraphicsItem* item, list){
            QGraphicsSimpleTextItem * textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
            if(textItem && !textItem->text().contains("0.011")){
                branchList.append(textItem);
            }
        }

    foreach(QGraphicsSimpleTextItem* item, branchList){
        CHECK_SET_ERR(!item->isVisible(), item->text() + " is visiable");
    }
//    Expected state: this node's branches has dissapered

//    3. Do context menu {Expand} for same
    GTMouseDriver::moveTo(os, globalCoord);
    GTMouseDriver::doubleClick(os);

    foreach(QGraphicsSimpleTextItem* item, branchList){
        CHECK_SET_ERR(item->isVisible(), item->text() + " is not visiable");
    }
//    Expected state: this node's branches has dissapered
}

GUI_TEST_CLASS_DEFINITION(test_0012){
//1. Run Ugene.
//   Open file _common_data/scenarios/tree_view/D120911.tre
    GTFileDialog::openFile(os,testDir + "_common_data/scenarios/tree_view/", "D120911.tre");
    GTGlobals::sleep(500);
//   Expected state: philogenetic tree appears
    QGraphicsView* treeView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os, "treeView"));
    QList<QGraphicsItem*> list = treeView->scene()->items();

    QList<QGraphicsSimpleTextItem *> branchList;
    foreach(QGraphicsItem* item, list){
            QGraphicsSimpleTextItem * textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
            if(textItem && !textItem->text().contains("0.011")){
                branchList.append(textItem);
            }
        }
//2. Make sure the tree doesn't look like a vertical line. It should have some width
    QList<QGraphicsItem *> lineList;
    foreach(QGraphicsItem* item, list){
            QGraphicsSimpleTextItem * textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
            if(!textItem){
                lineList.append(item);
            }
        }

    qreal w=0;
    foreach(QGraphicsItem * item, lineList){
        if(w<item->boundingRect().width()){
            w = item->boundingRect().width();
        }
    }

    CHECK_SET_ERR(w>100, "tree seems to be too narrow");
//3. Choose any node and do the context menu command "Swap siblings"
    QList<QGraphicsItem*> nodeList;
    foreach(QGraphicsItem* item, list){
        if(item->boundingRect().width()==10){
            nodeList.append(item);
        }
    }

    QGraphicsItem* node = nodeList.at(1);
    QPointF sceneCoord = node->mapToScene(node->boundingRect().center());
    QPoint viewCord = treeView->mapFromScene(sceneCoord);
    QPoint globalCoord = treeView->mapToGlobal(viewCord);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"Swap Siblings"));
    GTMouseDriver::moveTo(os,globalCoord);
    GTMouseDriver::click(os);
    GTMouseDriver::click(os,Qt::RightButton);

    qreal finalW=0;
    foreach(QGraphicsItem * item, lineList){
        if(finalW<item->boundingRect().width()){
            finalW = item->boundingRect().width();
        }
    }
    CHECK_SET_ERR(w == finalW, "tree weights seems to be changed");
//   Expected state: again, tree should have some width

}
} // namespace GUITest_common_scenarios_tree_viewer
} // namespace U2
