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
 
#include "GTUtilsTaskTreeView.h" 
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"
#include "runnables/qt/PopupChooser.h"

#include <U2Core/Task.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/AppContext.h>
#include <QtCore/QTimer>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QTreeWidget>
#else
#include <QtWidgets/QTreeWidget>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTUtilsTaskTreeView"

const QString GTUtilsTaskTreeView::widgetName = DOCK_TASK_TREE_VIEW;


void GTUtilsTaskTreeView::waitTaskFinished(U2OpStatus &os, long timeout){


    TaskScheduler* scheduller = AppContext::getTaskScheduler();
    int i = 0;
    while(!scheduller->getTopLevelTasks().isEmpty()){
       GTGlobals::sleep(1000);
       i++;
       if(i > (timeout/1000)){
           os.setError(os.getError() + getTasksInfo(scheduller->getTopLevelTasks(),0));
           break;
       }
    }
}

QString GTUtilsTaskTreeView::getTasksInfo(QList<Task *> tasks, int level){
    QString result;
    QMap<Task::State, QString> stateMap;
    stateMap.insert(Task::State_New, "State_New");
    stateMap.insert(Task::State_Prepared, "State_Prepared");
    stateMap.insert(Task::State_Running, "State_Running");
    stateMap.insert(Task::State_Finished, "State_Finished");
    foreach(Task* t, tasks){
        for (int i=0; i<level; i++){
            result.append("  ");
        }
        result.append(QString("%1:  %2\n").arg(t->getTaskName()).arg(stateMap.value(t->getState())));
        result.append(getTasksInfo(t->getSubtasks(),level+1));
    }
    return result;
}
void GTUtilsTaskTreeView::openView(U2OpStatus& os) {
    GTGlobals::FindOptions options;
    options.failIfNull = false;

    QWidget *documentTreeWidget = GTWidget::findWidget(os, widgetName, NULL, options);
    if (!documentTreeWidget) {
        toggleView(os);
    }
    GTGlobals::sleep(500);
}

void GTUtilsTaskTreeView::toggleView(U2OpStatus& os) {
    GTKeyboardDriver::keyClick(os, '2', GTKeyboardDriver::key["alt"]);
}

#define GT_METHOD_NAME "getTreeWidgetItem"
QTreeWidgetItem* GTUtilsTaskTreeView::getTreeWidgetItem( U2OpStatus &os, const QString &itemName ){
    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget not found", NULL);

    QTreeWidgetItem *item = getTreeWidgetItem(treeWidget, itemName);
    GT_CHECK_RESULT(item != NULL, "Item " + itemName + " not found in tree widget", NULL);

    return item;
}
#undef GT_METHOD_NAME

QTreeWidgetItem* GTUtilsTaskTreeView::getTreeWidgetItem( QTreeWidget* tree, const QString &itemName ){
    if (itemName.isEmpty()) {
        return NULL;
    }

    QList<QTreeWidgetItem*> treeItems = getTaskTreeViewItems(tree->invisibleRootItem());
    foreach (QTreeWidgetItem* item, treeItems) {
        QString treeItemName = item->text(0);
        if (treeItemName == itemName) {
            return item;
        }
    }

    return NULL;
}

QTreeWidget* GTUtilsTaskTreeView::getTreeWidget(U2OpStatus &os) {
    GTGlobals::FindOptions options;
    options.failIfNull = false;

    QTreeWidget *treeWidget = static_cast<QTreeWidget*>(GTWidget::findWidget(os, widgetName, NULL, options));
    return treeWidget;
}

QList<QTreeWidgetItem*> GTUtilsTaskTreeView::getTaskTreeViewItems(QTreeWidgetItem* root, bool recursively) {

    QList<QTreeWidgetItem*> treeItems;

    for (int i=0; i<root->childCount(); i++) {
        treeItems.append(root->child(i));
        if (recursively) {
            treeItems.append(getTaskTreeViewItems(root->child(i)));
        }
    }

    return treeItems;
}

void GTUtilsTaskTreeView::cancelTask( U2OpStatus& os, const QString &itemName ){
    Runnable *popupChooser = new PopupChooser(os, QStringList() << "cancel_task_action", GTGlobals::UseMouse);
    GTUtilsDialog::waitForDialog(os, popupChooser);
    click(os, itemName, Qt::RightButton);
    GTGlobals::sleep(3000);
}

void GTUtilsTaskTreeView::click( U2OpStatus &os, const QString &itemName, Qt::MouseButton b ){
    moveTo(os, itemName);
    GTMouseDriver::click(os, b);
}

void GTUtilsTaskTreeView::moveTo(U2OpStatus &os,const QString &itemName) {
    openView(os);
    moveToOpenedView(os, itemName);
}

#define GT_METHOD_NAME "getTopLevelTasksCount"
int GTUtilsTaskTreeView::getTopLevelTasksCount(U2OpStatus &os) {
    openView(os);

    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget not found", -1);

    return getTaskTreeViewItems(treeWidget->invisibleRootItem(), false).size();
}
#undef GT_METHOD_NAME

void GTUtilsTaskTreeView::moveToOpenedView( U2OpStatus& os, const QString &itemName ){
    QPoint p = getTreeViewItemPosition(os, itemName);
    GTMouseDriver::moveTo(os, p);
}

#define GT_METHOD_NAME "getTreeViewItemPosition"
QPoint GTUtilsTaskTreeView::getTreeViewItemPosition( U2OpStatus &os, const QString &itemName ){
    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "treeWidget is NULL", QPoint());
    QTreeWidgetItem *item = getTreeWidgetItem(os, itemName);

    QPoint p = treeWidget->rect().center();
    if (item) {
        p = treeWidget->visualItemRect(item).center();
        p.setY(p.y() + treeWidget->visualItemRect(item).height() + 5); //+ height because of header item; +5 because height is not enough
    }

    return treeWidget->mapToGlobal(p);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
