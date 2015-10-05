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

#include <QApplication>
#include <QDialogButtonBox>
#include <QGraphicsView>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QSpinBox>
#include <QTableView>
#include <QTableWidget>
#include <QToolButton>
#include <QTreeWidget>

#include <U2Core/AppContext.h>

#include <U2View/MSAEditor.h>

#include "../../workflow_designer/src/WorkflowViewItems.h"
#include "GTUtilsMdi.h"
#include "GTUtilsWorkflowDesigner.h"
#include "api/GTAction.h"
#include "api/GTCheckBox.h"
#include "api/GTComboBox.h"
#include "api/GTDoubleSpinBox.h"
#include "api/GTFileDialog.h"
#include "api/GTGraphicsItem.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTLineEdit.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTSpinBox.h"
#include "api/GTTabWidget.h"
#include "api/GTTableView.h"
#include "api/GTToolbar.h"
#include "api/GTTreeWidget.h"
#include "api/GTWidget.h"
#include "runnables/qt/MessageBoxFiller.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/AppSettingsDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/DatasetNameEditDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/StartupDialogFiller.h"
#include "runnables/ugene/plugins/workflow_designer/WorkflowMetadialogFiller.h"

#include <U2Gui/ToolsMenu.h>

namespace U2 {

const int GTUtilsWorkflowDesigner::verticalShift = 35;
#define GT_CLASS_NAME "GTUtilsWorkflowDesigner"

#define GT_METHOD_NAME "openWorkflowDesigner"
void GTUtilsWorkflowDesigner::openWorkflowDesigner(U2OpStatus &os){
    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new StartupDialogFiller(os));

    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    GTMenu::clickMenuItemByName(os, menu, QStringList() << ToolsMenu::WORKFLOW_DESIGNER);
    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "currentTab"
GTUtilsWorkflowDesigner::tab GTUtilsWorkflowDesigner::currentTab(U2OpStatus &os) {
    QTabWidget *tabs = qobject_cast<QTabWidget*>(GTWidget::findWidget(os, "tabs"));
    GT_CHECK_RESULT(NULL != tabs, "tabs widget is not found", algoriths);
    return tab(tabs->currentIndex());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setCurrentTab"
void GTUtilsWorkflowDesigner::setCurrentTab(U2OpStatus &os, tab t) {
    QTabWidget *tabs = qobject_cast<QTabWidget*>(GTWidget::findWidget(os, "tabs"));
    GT_CHECK(NULL != tabs, "tabs widget is not found");
    GTTabWidget::setCurrentIndex(os, tabs, int(t));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "loadWorkflow"
void GTUtilsWorkflowDesigner::loadWorkflow(U2OpStatus &os, const QString &fileUrl) {
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, fileUrl));
    QToolBar *wdToolbar = GTToolbar::getToolbar(os, "mwtoolbar_activemdi");
    GT_CHECK(wdToolbar, "Toolbar is not found");
    QWidget *loadButton = GTToolbar::getWidgetForActionName(os, wdToolbar, "Load workflow");
    GT_CHECK(loadButton, "Load button is not found");
    GTWidget::click(os, loadButton);
    GTGlobals::sleep();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "saveWorkflow"
void GTUtilsWorkflowDesigner::saveWorkflow(U2OpStatus &os) {
    GTToolbar::clickButtonByTooltipOnToolbar(os, MWTOOLBAR_ACTIVEMDI, "Save workflow");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "saveWorkflowAs"
void GTUtilsWorkflowDesigner::saveWorkflowAs(U2OpStatus &os, const QString &fileUrl, const QString &workflowName) {
    GTUtilsDialog::waitForDialog(os, new WorkflowMetaDialogFiller(os, fileUrl, workflowName));
    GTToolbar::clickButtonByTooltipOnToolbar(os, MWTOOLBAR_ACTIVEMDI, "Save workflow as");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "runWorkflow"
void GTUtilsWorkflowDesigner::runWorkflow(U2OpStatus &os) {
    GTWidget::click(os, GTAction::button(os, "Run workflow"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "returnToWorkflow"
void GTUtilsWorkflowDesigner::returnToWorkflow(U2OpStatus &os) {
    GTWidget::click(os, GTAction::button(os, GTAction::findActionByText(os, "To Workflow Designer")));
}
#undef GT_METHOD_NAME

namespace {
bool compare(QString s1, QString s2, bool exactMatch){
    if(exactMatch){
        return s1==s2;
    }else{
        return s1.toLower().contains(s2.toLower());
    }
}
}

#define GT_METHOD_NAME "findTreeItem"
QTreeWidgetItem* GTUtilsWorkflowDesigner::findTreeItem(U2OpStatus &os,QString itemName, tab t, bool exactMatch, bool failIfNULL){

    QTreeWidgetItem* foundItem=NULL;
    QTreeWidget *w;
    if(t==algoriths){
        w=qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"WorkflowPaletteElements"));
    }
    else{
        w=qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"samples"));
    }
    GT_CHECK_RESULT(w!=NULL,"WorkflowPaletteElements is null", NULL);

    QList<QTreeWidgetItem*> outerList = w->findItems("",Qt::MatchContains);

    for (int i=0;i<outerList.count();i++){
        QList<QTreeWidgetItem*> innerList;

        for(int j=0;j<outerList.value(i)->childCount();j++ ){
           innerList.append(outerList.value(i)->child(j));
        }

        foreach(QTreeWidgetItem* item, innerList){
            if(t==algoriths){
                QString s = item->data(0,Qt::UserRole).value<QAction*>()->text();
                if(compare(s, itemName, exactMatch)){
                    GT_CHECK_RESULT(foundItem==NULL,"several items have this discription",item);
                    foundItem=item;
                }
            }
            else{
                QString s = item->text(0);
                if(compare(s, itemName, exactMatch)){
                    GT_CHECK_RESULT(foundItem==NULL,"several items have this discription",item);
                    foundItem=item;
                }
            }
        }
    }
    if(failIfNULL){
        GT_CHECK_RESULT(foundItem!=NULL,"Item \"" + itemName + "\" not found in treeWidget",NULL);
    }
    return foundItem;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getVisibleSamples"
QList<QTreeWidgetItem*> GTUtilsWorkflowDesigner::getVisibleSamples(U2OpStatus &os){
    QTreeWidget* w=qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"samples"));
    GT_CHECK_RESULT(w!=NULL,"WorkflowPaletteElements is null", QList<QTreeWidgetItem*>());

    QList<QTreeWidgetItem*> outerList = w->findItems("",Qt::MatchContains);
    QList<QTreeWidgetItem*> resultList;
    for (int i=0;i<outerList.count();i++){
        QList<QTreeWidgetItem*> innerList;

        for(int j=0;j<outerList.value(i)->childCount();j++ ){
           innerList.append(outerList.value(i)->child(j));
        }

        foreach(QTreeWidgetItem* item, innerList){
            if(!item->isHidden()){
                resultList.append(item);
            }
        }
    }
    return resultList;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addAlgorithm"
void GTUtilsWorkflowDesigner::addAlgorithm(U2OpStatus &os, QString algName, bool exactMatch){
    expandTabs(os);
    QTabWidget* tabs = qobject_cast<QTabWidget*>(GTWidget::findWidget(os,"tabs"));
    GT_CHECK(tabs!=NULL, "tabs widget not found");

    GTTabWidget::setCurrentIndex(os,tabs,0);

    QTreeWidgetItem *alg = findTreeItem(os, algName, algoriths, exactMatch);
    GTGlobals::sleep(100);
    GT_CHECK(alg!=NULL,"algorithm is NULL");

    selectAlgorithm(os,alg);
    QWidget* w = GTWidget::findWidget(os,"sceneView");

    int workerNum = getWorkers(os).size();
    QPoint p(w->rect().topLeft() + QPoint(100+300*(workerNum-(workerNum/2)*2),100 + 200*(workerNum/2)));//shifting workers position
    GTWidget::click(os, w,Qt::LeftButton, p);
    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addElement"
WorkflowProcessItem * GTUtilsWorkflowDesigner::addElement(U2OpStatus &os, const QString &algName, bool exactMatch) {
    addAlgorithm(os, algName, exactMatch);
    CHECK_OP(os, NULL);
    return getWorker(os, algName);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectAlgorithm"
void GTUtilsWorkflowDesigner::selectAlgorithm(U2OpStatus &os, QTreeWidgetItem* algorithm){
    GT_CHECK(algorithm!=NULL, "algorithm is NULL");
    GTGlobals::sleep(500);

    QTreeWidget *w=qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"WorkflowPaletteElements"));
    QList<QTreeWidgetItem*> childrenList = w->findItems("",Qt::MatchContains);
    foreach(QTreeWidgetItem* child,childrenList){
        child->setExpanded(false);
    }

    algorithm->parent()->setExpanded(true);
    algorithm->treeWidget()->scrollToItem(algorithm, QAbstractItemView::PositionAtCenter);
    GTMouseDriver::moveTo(os,GTTreeWidget::getItemCenter(os,algorithm));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addSample"
void GTUtilsWorkflowDesigner::addSample(U2OpStatus &os, QString sampName){
    expandTabs(os);
    QTabWidget* tabs = qobject_cast<QTabWidget*>(GTWidget::findWidget(os,"tabs"));
    GT_CHECK(tabs!=NULL, "tabs widget not found");

    GTTabWidget::setCurrentIndex(os,tabs,1);

    QTreeWidgetItem *samp = findTreeItem(os, sampName,samples);
    GTGlobals::sleep(100);
    GT_CHECK(samp!=NULL,"sample is NULL");

    selectSample(os,samp);
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "selectSample"
void GTUtilsWorkflowDesigner::selectSample(U2OpStatus &os, QTreeWidgetItem* sample){
    GT_CHECK(sample!=NULL, "sample is NULL");
    GTGlobals::sleep(500);

    QTreeWidget *w=qobject_cast<QTreeWidget*>(GTWidget::findWidget(os,"samples"));
    QList<QTreeWidgetItem*> childrenList = w->findItems("",Qt::MatchContains);
    foreach(QTreeWidgetItem* child,childrenList){
        child->setExpanded(false);
    }

    sample->parent()->setExpanded(true);
    GTMouseDriver::moveTo(os,GTTreeWidget::getItemCenter(os,sample));
    GTMouseDriver::doubleClick(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "expandTabs"
void GTUtilsWorkflowDesigner::expandTabs(U2OpStatus &os){
    QSplitter* splitter = qobject_cast<QSplitter*>(GTWidget::findWidget(os,"splitter"));
    GT_CHECK(splitter, "splitter not found");
    QList<int> s;
    s  = splitter->sizes();

    if(s.first()==0){//expands tabs if collapsed
        QPoint p;
        p.setX(splitter->geometry().left()+2);
        p.setY(splitter->geometry().center().y());
        GTMouseDriver::moveTo(os, p);
        GTGlobals::sleep(300);
        GTMouseDriver::press(os);
        p.setX(p.x()+200);
        GTMouseDriver::moveTo(os,p);
        GTMouseDriver::release(os);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickOnPalette"
void GTUtilsWorkflowDesigner::clickOnPalette(U2OpStatus &os, const QString &itemName, Qt::MouseButton mouseButton) {
    selectAlgorithm(os, findTreeItem(os, itemName, algoriths, true));
    GTMouseDriver::click(os, mouseButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getPaletteGroup"
QTreeWidgetItem * GTUtilsWorkflowDesigner::getPaletteGroup(U2OpStatus &os, const QString &groupName) {

    QTreeWidget *tree = getCurrentTabTreeWidget(os);
    GT_CHECK_RESULT(NULL != tree, "WorkflowPaletteElements is NULL", NULL);

    GTGlobals::FindOptions options;
    options.depth = 1;
    options.matchPolicy = Qt::MatchExactly;

    return GTTreeWidget::findItem(os, tree, groupName, NULL, 0, options);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getPaletteGroups"
QList<QTreeWidgetItem *> GTUtilsWorkflowDesigner::getPaletteGroups(U2OpStatus &os) {
    QList<QTreeWidgetItem *> groupItems;

    QTreeWidget *tree = getCurrentTabTreeWidget(os);
    GT_CHECK_RESULT(NULL != tree, "WorkflowPaletteElements is NULL", groupItems);

    GTGlobals::FindOptions options;
    options.depth = 1;
    options.matchPolicy = Qt::MatchContains;

    return GTTreeWidget::findItems(os, tree, "", NULL, 0, options);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getPaletteGroupNames"
QStringList GTUtilsWorkflowDesigner::getPaletteGroupNames(U2OpStatus &os) {
    QStringList groupNames;
    const QList<QTreeWidgetItem *> groupItems = getPaletteGroups(os);
    foreach (QTreeWidgetItem *groupItem, groupItems) {
        groupNames << groupItem->text(0);
    }
    return groupNames;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getPaletteGroupEntries"
QList<QTreeWidgetItem *> GTUtilsWorkflowDesigner::getPaletteGroupEntries(U2OpStatus &os, QTreeWidgetItem *groupItem) {
    QList<QTreeWidgetItem *> items;

    GT_CHECK_RESULT(NULL != groupItem, "Group item is NULL", items);

    QTreeWidget *tree = getCurrentTabTreeWidget(os);
    GT_CHECK_RESULT(NULL != tree, "WorkflowPaletteElements is NULL", items);

    GTGlobals::FindOptions options;
    options.depth = 0;
    options.matchPolicy = Qt::MatchContains;

    return GTTreeWidget::findItems(os, tree, "", groupItem, 0, options);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getPaletteGroupEntries"
QList<QTreeWidgetItem *> GTUtilsWorkflowDesigner::getPaletteGroupEntries(U2OpStatus &os, const QString &groupName) {
    return getPaletteGroupEntries(os, getPaletteGroup(os, groupName));
}
#undef GT_METHOD_NAME

QPoint GTUtilsWorkflowDesigner::getItemCenter(U2OpStatus &os,QString itemName){
    QRect r = getItemRect(os, itemName);
    QPoint p = r.center();
    return p;
}

#define GT_METHOD_NAME "removeItem"
void GTUtilsWorkflowDesigner::removeItem(U2OpStatus &os, QString itemName) {
    click(os, itemName);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
}
#undef GT_METHOD_NAME

int GTUtilsWorkflowDesigner::getItemLeft(U2OpStatus &os, QString itemName){
    QRect r = getItemRect(os, itemName);
    int i = r.left();
    return i;
}

int GTUtilsWorkflowDesigner::getItemRight(U2OpStatus &os, QString itemName){
    QRect r = getItemRect(os, itemName);
    int i = r.right();
    return i;
}

int GTUtilsWorkflowDesigner::getItemTop(U2OpStatus &os, QString itemName){
    QRect r = getItemRect(os, itemName);
    int i = r.top();
    return i;
}

int GTUtilsWorkflowDesigner::getItemBottom(U2OpStatus &os, QString itemName){
    QRect r = getItemRect(os, itemName);
    int i = r.bottom();
    return i;
}
#define GT_METHOD_NAME "click"
void GTUtilsWorkflowDesigner::click(U2OpStatus &os, QString itemName, QPoint p, Qt::MouseButton button){
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    GT_CHECK(sceneView!=NULL, "scene view is NULL");
    sceneView->ensureVisible(getWorker(os, itemName));

    GTMouseDriver::moveTo(os, getItemCenter(os, itemName) + p);
    GTMouseDriver::click(os);
    if (Qt::RightButton == button) {
        GTMouseDriver::click(os, Qt::RightButton);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "click"
void GTUtilsWorkflowDesigner::click(U2OpStatus &os, QGraphicsItem* item, QPoint p, Qt::MouseButton button){
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    GT_CHECK(sceneView!=NULL, "scene view is NULL");
    sceneView->ensureVisible(item);
    QRect rect = GTGraphicsItem::getGraphicsItemRect(os, item);

    GTMouseDriver::moveTo(os, rect.center() + p);
    GTMouseDriver::click(os);
    if (Qt::RightButton == button) {
        GTMouseDriver::click(os, Qt::RightButton);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getWorker"
WorkflowProcessItem* GTUtilsWorkflowDesigner::getWorker(U2OpStatus &os,QString itemName,const GTGlobals::FindOptions &options){
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    GT_CHECK_RESULT(sceneView, "sceneView not found", NULL);
    QList<QGraphicsItem *> items = sceneView->items();

    foreach(QGraphicsItem* it, items) {
        QGraphicsObject *itObj = it->toGraphicsObject();

        QGraphicsTextItem* textItemO = qobject_cast<QGraphicsTextItem*>(itObj);
        if (textItemO) {
            QString text = textItemO->toPlainText();

            int num = text.indexOf('\n');
            if(num == -1){
                continue;
            }
            text = text.left(num);

            if (text == itemName) {
                if(qgraphicsitem_cast<WorkflowProcessItem*>(it->parentItem()->parentItem()))
                    return (qgraphicsitem_cast<WorkflowProcessItem*>(it->parentItem()->parentItem()));
            }
        }
    }
    GT_CHECK_RESULT(options.failIfNull == false, "Item " + itemName + " not found at scene", NULL);

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getWorkerText"
QString GTUtilsWorkflowDesigner::getWorkerText(U2OpStatus &os, QString itemName, const GTGlobals::FindOptions &options) {
    WorkflowProcessItem *worker = getWorker(os, itemName, options);
    foreach (QGraphicsItem *child, worker->childItems()) {
        foreach (QGraphicsItem *subchild, child->childItems()) {
            QGraphicsObject *graphObject = subchild->toGraphicsObject();
            QGraphicsTextItem *textItem = qobject_cast<QGraphicsTextItem *>(graphObject);
            if (NULL != textItem) {
                return textItem->toPlainText();
            }
        }
    }
    return QString();
}
#undef GT_METHOD_NAME

void GTUtilsWorkflowDesigner::clickLink(U2OpStatus &os, QString itemName, Qt::MouseButton button, int step){
    WorkflowProcessItem* worker = getWorker(os, itemName);

    int left = GTUtilsWorkflowDesigner::getItemLeft(os, itemName);
    int right = GTUtilsWorkflowDesigner::getItemRight(os, itemName);
    int top = GTUtilsWorkflowDesigner::getItemTop(os, itemName);
    int bottom = GTUtilsWorkflowDesigner::getItemBottom(os, itemName);
    for(int i = left; i < right; i+=step){
        for(int j = top; j < bottom; j+=step){
            GTMouseDriver::moveTo(os, QPoint(i,j));
            if(worker->cursor().shape() == Qt::PointingHandCursor){
                GTMouseDriver::click(os, button);
                return;
            }
        }
    }
}

#define GT_METHOD_NAME "isWorkerExtended"
bool GTUtilsWorkflowDesigner::isWorkerExtended(U2OpStatus &os, const QString &itemName) {
    return "ext" == getWorker(os,itemName)->getStyle();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getPortById"
WorkflowPortItem* GTUtilsWorkflowDesigner::getPortById(U2OpStatus &os, WorkflowProcessItem *worker, QString id){
    QList<WorkflowPortItem*> list = getPorts(os, worker);
    foreach(WorkflowPortItem* p, list){
        if(p&&p->getPort()->getId()==id){
            return p;
        }
    }
    GT_CHECK_RESULT(false, "port with id " + id + "not found",NULL);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getPorts"
QList<WorkflowPortItem*> GTUtilsWorkflowDesigner::getPorts(U2OpStatus &os, WorkflowProcessItem *worker){
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    GT_CHECK_RESULT(sceneView,"sceneView not found", QList<WorkflowPortItem*>())
    return worker->getPortItems();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemRect"
QRect GTUtilsWorkflowDesigner::getItemRect(U2OpStatus &os,QString itemName){
//TODO: support finding items when there are several similar workers in scheme
    WorkflowProcessItem* w = getWorker(os, itemName);
    QRect result = GTGraphicsItem::getGraphicsItemRect(os, w);
    result.setTop(result.top() + verticalShift);
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getCurrentTabTreeWidget"
QTreeWidget *GTUtilsWorkflowDesigner::getCurrentTabTreeWidget(U2OpStatus &os) {
    switch (currentTab(os)) {
    case algoriths:
        return GTWidget::findExactWidget<QTreeWidget *>(os, "WorkflowPaletteElements");
    case samples:
        return GTWidget::findExactWidget<QTreeWidget *>(os, "samples");
    default:
        os.setError("An unexpected current tab");
        return NULL;
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "toggleDebugMode"
void GTUtilsWorkflowDesigner::toggleDebugMode(U2OpStatus &os, bool enable) {
    class DebugModeToggleScenario : public CustomScenario {
    public:
        DebugModeToggleScenario(bool enable) :
            enable(enable)
        {
        }

        void run(U2OpStatus &os) {
            QWidget *dialog = QApplication::activeModalWidget();
            GT_CHECK(dialog, "activeModalWidget is NULL");

            GTTreeWidget::click(os, GTTreeWidget::findItem(os, GTWidget::findExactWidget<QTreeWidget *>(os, "tree"), "  Workflow Designer"));
            GTCheckBox::setChecked(os, GTWidget::findExactWidget<QCheckBox *>(os, "debuggerBox"), enable);

            GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
        }

    private:
        bool enable;
    };

    GTUtilsDialog::waitForDialog(os, new AppSettingsDialogFiller(os, new DebugModeToggleScenario(enable)));
    GTMenu::clickMenuItemByName(os, GTMenu::showMainMenu(os, MWMENU_SETTINGS), QStringList() << "action__settings");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "toggleBreakpointManager"
void GTUtilsWorkflowDesigner::toggleBreakpointManager(U2OpStatus &os) {
    GTWidget::click(os, GTToolbar::getWidgetForActionTooltip(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "Show or hide breakpoint manager"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setBreakpoint"
void GTUtilsWorkflowDesigner::setBreakpoint(U2OpStatus &os, const QString &itemName) {
    click(os, itemName);
    GTWidget::click(os, GTToolbar::getWidgetForActionTooltip(os, GTToolbar::getToolbar(os, MWTOOLBAR_ACTIVEMDI), "Break at element"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getBreakpointList"
QStringList GTUtilsWorkflowDesigner::getBreakpointList(U2OpStatus &os) {
    return GTTreeWidget::getItemNames(os, GTWidget::findExactWidget<QTreeWidget *>(os, "breakpoints list"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getAllConnectionArrows"
QList<WorkflowBusItem*> GTUtilsWorkflowDesigner::getAllConnectionArrows(U2OpStatus &os){
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    GT_CHECK_RESULT(sceneView,"sceneView not found", QList<WorkflowBusItem*>());

    QList<WorkflowBusItem*> result;

    foreach (QGraphicsItem* item, sceneView->items()) {
        WorkflowBusItem* arrow = qgraphicsitem_cast<WorkflowBusItem*>(item);
        if(arrow != NULL){
            result.append(arrow);
        }
    };

    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "removeCmdlineWorkerFromPalette"
void GTUtilsWorkflowDesigner::removeCmdlineWorkerFromPalette(U2OpStatus &os, const QString &workerName) {
    QTabWidget* tabs = qobject_cast<QTabWidget*>(GTWidget::findWidget(os, "tabs"));
    GT_CHECK(tabs != NULL, "tabs widget not found");

    GTTabWidget::setCurrentIndex(os, tabs, 0);

    QTreeWidget* w = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os, "WorkflowPaletteElements"));
    GT_CHECK(w != NULL, "WorkflowPaletteElements is null");

    QTreeWidgetItem* foundItem = NULL;
    QList<QTreeWidgetItem*> outerList = w->findItems("", Qt::MatchContains);
    for (int i = 0; i < outerList.count(); i++){
        QList<QTreeWidgetItem*> innerList;

        for (int j = 0; j < outerList.value(i)->childCount(); j++){
            innerList.append(outerList.value(i)->child(j));
        }

        foreach (QTreeWidgetItem *item, innerList) {
            const QString s = item->data(0, Qt::UserRole).value<QAction*>()->text();
            if (s == workerName) {
                foundItem = item;
            }
        }
    }
    if (foundItem != NULL) {
        GTUtilsDialog::waitForDialog(os, new PopupChooserByText(os, QStringList() << "Remove"));
        GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok, "", "Remove element"));
        GTUtilsWorkflowDesigner::clickOnPalette(os, workerName, Qt::RightButton);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "connect"
void GTUtilsWorkflowDesigner::connect(U2OpStatus &os, WorkflowProcessItem * from , WorkflowProcessItem * to){
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(from->scene()->views().at(0));
    GT_CHECK(sceneView,"sceneView not found")
    QList<WorkflowPortItem*> fromList = from->getPortItems();
    QList<WorkflowPortItem*> toList = to->getPortItems();

    foreach(WorkflowPortItem* fromPort, fromList){
        foreach(WorkflowPortItem* toPort, toList){
            if(fromPort->getPort()->canBind(toPort->getPort())){
                GTMouseDriver::moveTo(os,GTGraphicsItem::getItemCenter(os,fromPort));
                GTMouseDriver::press(os);
                GTMouseDriver::moveTo(os,GTGraphicsItem::getItemCenter(os,toPort));
                GTMouseDriver::release(os);
                GTGlobals::sleep(1000);
                return;
            }
        }
    }

    GT_CHECK(false,"no suitable ports to connect");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "disconnect"
void GTUtilsWorkflowDesigner::disconect(U2OpStatus &os, WorkflowProcessItem * from , WorkflowProcessItem * to){
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(from->scene()->views().at(0));
    GT_CHECK(sceneView,"sceneView not found");

    WorkflowBusItem* arrow = getConnectionArrow(os, from, to);
    QGraphicsTextItem* hint = getArrowHint(os, arrow);
    click(os, hint);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getConnectionArrow"
WorkflowBusItem *GTUtilsWorkflowDesigner::getConnectionArrow(U2OpStatus &os, WorkflowProcessItem * from , WorkflowProcessItem * to){
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(from->scene()->views().at(0));
    GT_CHECK_RESULT(sceneView,"sceneView not found", NULL)
    QList<WorkflowPortItem*> fromList = from->getPortItems();
    QList<WorkflowPortItem*> toList = to->getPortItems();

    QList<WorkflowBusItem*> arrows = getAllConnectionArrows(os);

    foreach(WorkflowPortItem* fromPort, fromList){
        foreach(WorkflowPortItem* toPort, toList){
            foreach (WorkflowBusItem* arrow, arrows) {
                if(arrow->getInPort() == toPort && arrow->getOutPort() == fromPort){
                    return arrow;
                }
            }
        }
    }

    GT_CHECK_RESULT(false,"no suitable ports to connect", NULL);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getArrowHint"
QGraphicsTextItem* GTUtilsWorkflowDesigner::getArrowHint(U2OpStatus &os, WorkflowBusItem *arrow){
    GT_CHECK_RESULT(arrow != NULL, "arrow item is NULL", NULL);

    foreach (QGraphicsItem* item, arrow->childItems()) {
        QGraphicsTextItem* hint = qgraphicsitem_cast<QGraphicsTextItem*>(item);
        if(hint != NULL){
            return hint;
        }
    }

    GT_CHECK_RESULT(false, "hint not found", NULL);
}
#undef GT_METHOD_NAME

QList<WorkflowProcessItem*> GTUtilsWorkflowDesigner::getWorkers(U2OpStatus &os){
    QList<WorkflowProcessItem*> result;
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    QList<QGraphicsItem *> items = sceneView->items();
    foreach(QGraphicsItem* it, items){
        WorkflowProcessItem* worker = qgraphicsitem_cast<WorkflowProcessItem*>(it);
        if(worker)
            result.append(worker);
    }
    return result;
}

#define GT_METHOD_NAME "setDatasetInputFile"
void GTUtilsWorkflowDesigner::setDatasetInputFile(U2OpStatus &os, QString filePath, QString fileName){
    GTGlobals::sleep(200);
    QWidget* DatasetWidget = GTWidget::findWidget(os, "DatasetWidget");
    GT_CHECK(DatasetWidget, "DatasetWidget not found");

    QWidget* addFileButton = GTWidget::findWidget(os, "addFileButton", DatasetWidget);
    GT_CHECK(addFileButton, "addFileButton not found");

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, filePath, fileName, GTFileDialogUtils::Open, GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, ob);

    GTWidget::click(os, addFileButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "addInputFile"
void GTUtilsWorkflowDesigner::addInputFile(U2OpStatus &os, const QString &elementName, const QString &url) {
    click(os, elementName);
    CHECK_OP(os, );
    QFileInfo info(url);
    setDatasetInputFile(os, info.path(), info.fileName());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "createDataset"
void GTUtilsWorkflowDesigner::createDataset(U2OpStatus &os, QString datasetName){
    QWidget* DatasetWidget = GTWidget::findWidget(os, "DatasetWidget");
    GT_CHECK(DatasetWidget, "DatasetWidget not found");

    QWidget* plusButton = GTWidget::findButtonByText(os, "+");
    GT_CHECK(plusButton, "plusButton not found");

    GTUtilsDialog::waitForDialog(os, new DatasetNameEditDialogFiller(os, datasetName));

    GTWidget::click(os, plusButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setDatasetInputFolder"
void GTUtilsWorkflowDesigner::setDatasetInputFolder(U2OpStatus &os, QString filePath){
    QWidget* DatasetWidget = GTWidget::findWidget(os, "DatasetWidget");
    GT_CHECK(DatasetWidget, "DatasetWidget not found");

    QWidget* addDirButton = GTWidget::findWidget(os, "addDirButton", DatasetWidget);
    GT_CHECK(addDirButton, "addFileButton not found");

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, filePath, "", GTFileDialogUtils::Choose, GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, ob);

    GTWidget::click(os, addDirButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setParameter"
void GTUtilsWorkflowDesigner::setParameter(U2OpStatus &os, QString parameter, QVariant value, valueType type, GTGlobals::UseMethod method){
    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"table"));
    CHECK_SET_ERR(table,"tableView not found");

    //FIND CELL
    QAbstractItemModel* model = table->model();
    int iMax = model->rowCount();
    int row = -1;
    for(int i = 0; i<iMax; i++){
        QString s = model->data(model->index(i,0)).toString();
        if (s.compare(parameter,Qt::CaseInsensitive) == 0){
            row = i;
            break;
        }
    }
    GT_CHECK(row != -1, QString("parameter not found: %1").arg(parameter));
    table->scrollTo(model->index(row,1));

    GTMouseDriver::moveTo(os,GTTableView::getCellPosition(os,table,1,row));
    GTMouseDriver::click(os);
    GTGlobals::sleep(500);

    //SET VALUE
    setCellValue(os, table, value, type, method);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setTableValue"
void GTUtilsWorkflowDesigner::setTableValue(U2OpStatus &os,  QString parameter, QVariant value, valueType type, QTableWidget *table, GTGlobals::UseMethod method){
    int row = -1;
    for(int i = 0; i<table->rowCount(); i++){
        QString s = table->item(i,0)->text();
        if(s == parameter){
            row = i;
            break;
        }
    }
    GT_CHECK(row != -1, QString("parameter not found: %1").arg(parameter));

    QRect rect = table->visualItemRect(table->item(row, 1));
    QPoint globalP = table->viewport()->mapToGlobal(rect.center());
    GTMouseDriver::moveTo(os, globalP);
    GTMouseDriver::click(os);
    GTGlobals::sleep(500);


    //SET VALUE
    setCellValue(os, table, value, type, method);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setCellValue"
void GTUtilsWorkflowDesigner::setCellValue(U2OpStatus &os, QWidget* parent, QVariant value, valueType type, GTGlobals::UseMethod method){
    bool ok = true;
    switch(type){
    case(comboWithFileSelector) : {
        GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, value.toString()));
        GTWidget::click(os, GTWidget::findButtonByText(os, "...", parent));
#ifdef Q_OS_WIN
        //added to fix UGENE-3597
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
#endif
        break;
    }
    case(lineEditWithFileSelector) : {
        GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, value.toString()));
        GTWidget::click(os, GTWidget::findButtonByText(os, "...", parent));
#ifdef Q_OS_WIN
        //added to fix UGENE-3597
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
#endif
        break;
    }
    case(spinValue):{
        int spinVal = value.toInt(&ok);
        GT_CHECK(ok,"Wrong input. Int required for GTUtilsWorkflowDesigner::spinValue")
        QSpinBox* box = qobject_cast<QSpinBox*>(parent->findChild<QSpinBox*>());
        GT_CHECK(box, "spinBox not found. Widget in this cell might be not QSpinBox");
        GTSpinBox::setValue(os, box, spinVal, GTGlobals::UseKeyBoard);
        break;
    }
    case(doubleSpinValue):{
        double spinVal = value.toDouble(&ok);
        GT_CHECK(ok,"Wrong input. Double required for GTUtilsWorkflowDesigner::doubleSpinValue")
        QDoubleSpinBox* box = qobject_cast<QDoubleSpinBox*>(parent->findChild<QDoubleSpinBox*>());
        GT_CHECK(box, "QDoubleSpinBox not found. Widget in this cell might be not QDoubleSpinBox");
        GTDoubleSpinbox::setValue(os, box, spinVal, GTGlobals::UseKeyBoard);
        break;
    }
    case(comboValue):{
        int comboVal = value.toInt(&ok);
        QComboBox* box = qobject_cast<QComboBox*>(parent->findChild<QComboBox*>());
        GT_CHECK(box, "QComboBox not found. Widget in this cell might be not QComboBox");

        if(!ok){
            QString comboString = value.toString();
            GTComboBox::setIndexWithText(os, box, comboString, true, method);
        }else{
            GTComboBox::setCurrentIndex(os, box, comboVal, true, method);
        }
#ifdef Q_OS_WIN
        //added to fix UGENE-3597
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
#endif
        break;
    }
    case(textValue):{
        QString lineVal = value.toString();
        QLineEdit* line = qobject_cast<QLineEdit*>(parent->findChild<QLineEdit*>());
        GT_CHECK(line, "QLineEdit not found. Widget in this cell might be not QLineEdit");
        GTLineEdit::setText(os, line, lineVal);
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
        break;
    }
    case ComboChecks: {
        QStringList values = value.value<QStringList>();
        QComboBox *box = qobject_cast<QComboBox*>(parent->findChild<QComboBox*>());
        GT_CHECK(box, "QComboBox not found");
        GTComboBox::checkValues(os, box, values);
#ifndef Q_OS_WIN
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["esc"]);
#endif
        break;
    }
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getCellValue"
QString GTUtilsWorkflowDesigner::getCellValue(U2OpStatus &os, QString parameter, QTableWidget *table){
    Q_UNUSED(os);
    int row = -1;
    for(int i = 0; i<table->rowCount(); i++){
        QString s = table->item(i,0)->text();
        if(s == parameter){
            row = i;
            break;
        }
    }
    GT_CHECK_RESULT(row != -1, QString("parameter not found: %1").arg(parameter), QString());

    QString result = table->item(row, 1)->text();
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getInputPortsTable"
QTableWidget* GTUtilsWorkflowDesigner::getInputPortsTable(U2OpStatus &os, int index){
    QWidget* inputPortBox = GTWidget::findWidget(os, "inputPortBox");
    QList<QTableWidget*> tables= inputPortBox->findChildren<QTableWidget*>();
    foreach (QTableWidget* w, tables) {
        if(!w->isVisible()){
            tables.removeOne(w);
        }
    }
    int number = tables.count();
    GT_CHECK_RESULT(index<number, QString("there are %1 visiable tables for input ports").arg(number), NULL);
    return tables[index];
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getAllParameters"
QStringList GTUtilsWorkflowDesigner::getAllParameters(U2OpStatus &os){
    QStringList result;
    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"table"));
    GT_CHECK_RESULT(table,"tableView not found", result);

    QAbstractItemModel* model = table->model();
    int iMax = model->rowCount();
    for(int i = 0; i<iMax; i++){
        QString s = model->data(model->index(i,0)).toString();
            result<<s;
    }
    return result;
}
#undef GT_METHOD_NAME

namespace {
    bool equalStrings(const QString &where, const QString &what, bool exactMatch) {
        if (exactMatch) {
            return (where == what);
        } else {
            return where.contains(what, Qt::CaseInsensitive);
        }
    }
}


#define GT_METHOD_NAME "getParameter"
QString GTUtilsWorkflowDesigner::getParameter(U2OpStatus &os, QString parameter, bool exactMatch){
    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"table"));
    GT_CHECK_RESULT(table,"tableView not found", "");

    QAbstractItemModel* model = table->model();
    GT_CHECK_RESULT(model,"model not found", "");
    int iMax = model->rowCount();
    int row = -1;
    for(int i = 0; i<iMax; i++){
        QString s = model->data(model->index(i,0)).toString();
        if (equalStrings(s, parameter, exactMatch)) {
            row = i;
            break;
        }
    }
    GT_CHECK_RESULT(row != -1, "parameter " + parameter + " not found","");

    QVariant var = model->data(model->index(row,1));
    return var.toString();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isParameterEnabled"
bool GTUtilsWorkflowDesigner::isParameterEnabled(U2OpStatus &os, QString parameter){
    clickParameter(os, parameter);
    QWidget *w = QApplication::widgetAt(GTMouseDriver::getMousePosition());
    QString s =  w->metaObject()->className();

    bool result = !(s == "QWidget");//if parameter is disabled QWidget is under cursor
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isParameterRequired"
bool GTUtilsWorkflowDesigner::isParameterRequired(U2OpStatus &os, const QString &parameter) {
    QTableView *table = qobject_cast<QTableView *>(GTWidget::findWidget(os, "table"));
    GT_CHECK_RESULT(table, "tableView not found", false);

    // find a cell
    QAbstractItemModel *model = table->model();
    int iMax = model->rowCount();
    int row = -1;
    for (int i = 0; i < iMax; i++) {
        QString s = model->data(model->index(i, 0)).toString();
        if (s.contains(parameter, Qt::CaseInsensitive)) {
            row = i;
        }
    }
    GT_CHECK_RESULT(row != -1, "parameter not found", false);
    table->scrollTo(model->index(row, 0));

    const QFont font = model->data(model->index(row, 0), Qt::FontRole).value<QFont>();
    return font.bold();
}
#undef GT_METHOD_NAME

namespace {

int getParameterRow(QTableView *table, const QString &parameter) {
    QAbstractItemModel *model = table->model();
    int iMax = model->rowCount();
    for (int i = 0; i < iMax; i++) {
        QString s = model->data(model->index(i, 0)).toString();
        if (s == parameter) {
            return i;
        }
    }
    return -1;
}

}

#define GT_METHOD_NAME "clickParameter"
void GTUtilsWorkflowDesigner::clickParameter(U2OpStatus &os, const QString &parameter) {
    QTableView *table = qobject_cast<QTableView *>(GTWidget::findWidget(os, "table"));
    GT_CHECK_RESULT(table, "tableView not found", );

    //FIND CELL
    const int row = getParameterRow(table, parameter);
    GT_CHECK_RESULT(row != -1, "parameter not found", );

    QAbstractItemModel *model = table->model();
    table->scrollTo(model->index(row, 1));
    GTMouseDriver::moveTo(os,GTTableView::getCellPosition(os, table, 1, row));
    GTMouseDriver::click(os);
    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isParameterVisible"
bool GTUtilsWorkflowDesigner::isParameterVisible(U2OpStatus &os, const QString &parameter) {
    QTableView *table = qobject_cast<QTableView *>(GTWidget::findWidget(os, "table"));
    GT_CHECK_RESULT(table, "tableView not found", false);
    return -1 != getParameterRow(table, parameter);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getParametersTable"
QTableView *GTUtilsWorkflowDesigner::getParametersTable(U2OpStatus &os){
    return qobject_cast<QTableView *>(GTWidget::findWidget(os, "table"));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "setParameterScripting"
void GTUtilsWorkflowDesigner::setParameterScripting(U2OpStatus &os, QString parameter, QString scriptMode, bool exactMatch){
    QTableView* table = qobject_cast<QTableView*>(GTWidget::findWidget(os,"table"));
    CHECK_SET_ERR(table,"tableView not found");

    //FIND CELL
    QAbstractItemModel* model = table->model();
    int iMax = model->rowCount();
    int row = -1;
    for(int i = 0; i<iMax; i++){
        QString s = model->data(model->index(i,0)).toString();
        if (equalStrings(s, parameter, exactMatch))
            row = i;
    }
    GT_CHECK(row != -1, "parameter not found");
    table->scrollTo(model->index(row,1));
    GTMouseDriver::moveTo(os,GTTableView::getCellPosition(os,table,2,row));
    GTMouseDriver::click(os);
    GTGlobals::sleep(500);

    //SET VALUE
    QComboBox* box = qobject_cast<QComboBox*>(table->findChild<QComboBox*>());
    GT_CHECK(box, "QComboBox not found. Scripting might be unavaluable for this parameter");
    GTComboBox::setIndexWithText(os, box, scriptMode, false);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkErrorList"
int GTUtilsWorkflowDesigner::checkErrorList(U2OpStatus &os, QString error){
    QListWidget* w = qobject_cast<QListWidget*>(GTWidget::findWidget(os,"infoList"));
    GT_CHECK_RESULT(w, "ErrorList widget not found", 0);


    QList<QListWidgetItem *> list =  w->findItems(error,Qt::MatchContains);
    return list.size();
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
} // namespace
