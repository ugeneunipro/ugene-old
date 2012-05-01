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
 
#include "GTUtilsTaskTreeView.h" 
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"
#include "GTUtilsDialog.h"

#include <U2Gui/MainWindow.h>
#include <QtGui/qtreewidget.h>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsTaskTreeView"

const QString GTUtilsTaskTreeView::widgetName = DOCK_TASK_TREE_VIEW;

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

QList<QTreeWidgetItem*> GTUtilsTaskTreeView::getTaskTreeViewItems(QTreeWidgetItem* root) {

    QList<QTreeWidgetItem*> treeItems;

    for (int i=0; i<root->childCount(); i++) {
        treeItems.append(root->child(i));
        treeItems.append(getTaskTreeViewItems(root->child(i)));
    }

    return treeItems;
}

void GTUtilsTaskTreeView::cancelTask( U2OpStatus& os, const QString &itemName ){
    GTUtilsDialog::PopupChooser popupChooser(os, QStringList() << "cancel_task_action", GTGlobals::UseMouse);
    GTUtilsDialog::preWaitForDialog(os, &popupChooser, GUIDialogWaiter::Popup);
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
        p.setY(p.y() + treeWidget->visualItemRect(item).height()); //+ height because of header item
    }

    return treeWidget->mapToGlobal(p);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
