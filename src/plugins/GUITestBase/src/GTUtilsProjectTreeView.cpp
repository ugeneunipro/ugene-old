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

#include "GTUtilsProjectTreeView.h"
#include "GTUtilsProject.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include <U2Gui/ProjectView.h>
#include <U2Core/ProjectModel.h>
#include <QtGui/QTreeWidget>

namespace U2 {

const QString GTUtilsProjectTreeView::widgetName = "documentTreeWidget";

void GTUtilsProjectTreeView::openView(U2OpStatus& os) {

    QWidget *documentTreeWidget = GTGlobals::findWidgetByName(os, widgetName, NULL, false);
    if (!documentTreeWidget) {
        toggleView(os);
    }
    GTGlobals::sleep(500);
}

void GTUtilsProjectTreeView::toggleView(U2OpStatus& os) {

    GTKeyboardDriver::keyClick(os, '1', GTKeyboardDriver::key["alt"]);
}

void GTUtilsProjectTreeView::rename(U2OpStatus &os, const QString &itemName, const QString &newItemName) {

    click(os, itemName);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTKeyboardDriver::keySequence(os, newItemName);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);

    GTGlobals::sleep(500);
}

void GTUtilsProjectTreeView::click(U2OpStatus &os, const QString &itemName, Qt::MouseButton b) {

    moveTo(os, itemName);
    GTMouseDriver::click(os, b);
}

void GTUtilsProjectTreeView::moveTo(U2OpStatus &os,const QString &itemName) {

    openView(os);
    moveToOpenedView(os, itemName);
}

void GTUtilsProjectTreeView::moveToOpenedView(U2OpStatus &os, const QString &itemName) {

    QPoint p = getTreeViewItemPosition(os, itemName);
    GTMouseDriver::moveTo(os, p);
}

void GTUtilsProjectTreeView::checkToolTip(U2OpStatus &os, const QString& itemName, const QString& tooltip) {

    moveTo(os, itemName);
    GTGlobals::sleep(2000);
    GTUtilsToolTip::checkExistingToolTip(os, tooltip);
}

void GTUtilsProjectTreeView::checkItem(U2OpStatus &os, const QString &itemName, bool exists) {

    QTreeWidgetItem* item = getTreeWidgetItem(os, itemName);
    if (exists) {
        CHECK_SET_ERR(item != NULL, "Item " + itemName + " not found in tree widget");
    }
    else {
        CHECK_SET_ERR(item == NULL, "Item " + itemName + " found in tree widget");
    }
}

QPoint GTUtilsProjectTreeView::getTreeViewItemPosition(U2OpStatus &os, const QString &itemName) {

    QTreeWidget *treeWidget = getTreeWidget(os);
    CHECK_SET_ERR_RESULT(treeWidget != NULL, "treeWidget is NULL", QPoint());
    QTreeWidgetItem *item = getTreeWidgetItem(os, itemName);

    QPoint p = treeWidget->rect().center();
    if (item) {
        p = treeWidget->visualItemRect(item).center();
    }

    return treeWidget->mapToGlobal(p);
}

QTreeWidget* GTUtilsProjectTreeView::getTreeWidget(U2OpStatus &os) {

    QTreeWidget *treeWidget = static_cast<QTreeWidget*>(GTGlobals::findWidgetByName(os, widgetName));
    return treeWidget;
}

QString GTUtilsProjectTreeView::getProjectTreeItemName(ProjViewItem* projViewItem) {

    if (projViewItem->isDocumentItem()) {
        ProjViewDocumentItem *documentItem = (ProjViewDocumentItem*)projViewItem;
        CHECK_EXT((documentItem != NULL) && (documentItem->doc != NULL),,"");
        return documentItem->doc->getName();
    }

    if (projViewItem->isObjectItem()) {
        ProjViewObjectItem *objectItem = (ProjViewObjectItem*)projViewItem;
        CHECK_EXT((objectItem!=NULL) && (objectItem->obj != NULL),,"");
        return objectItem->obj->getGObjectName();
    }

    if (projViewItem->isTypeItem()) {
        ProjViewTypeItem *typeItem = (ProjViewTypeItem*)projViewItem;
        CHECK_EXT(typeItem != NULL,,"");
        return typeItem->typePName;
    }

    return "";
}

QList<ProjViewItem*> GTUtilsProjectTreeView::getProjectViewItems(QTreeWidgetItem* root) {

    QList<ProjViewItem*> treeItems;

    for (int i=0; i<root->childCount(); i++) {
        treeItems.append((ProjViewItem*)root->child(i));
        treeItems.append(getProjectViewItems(root->child(i)));
    }

    return treeItems;
}


QTreeWidgetItem* GTUtilsProjectTreeView::getTreeWidgetItem(QTreeWidget* tree, const QString &itemName) {

    if (itemName.isEmpty()) {
        return NULL;
    }

    QList<ProjViewItem*> treeItems = getProjectViewItems(tree->invisibleRootItem());
    foreach (ProjViewItem* item, treeItems) {
        QString treeItemName = getProjectTreeItemName(item);
        if (treeItemName == itemName) {
            return item;
        }
    }

    return NULL;
}

QTreeWidgetItem* GTUtilsProjectTreeView::getTreeWidgetItem(U2OpStatus &os, const QString &itemName) {

    QTreeWidget *treeWidget = getTreeWidget(os);
    CHECK_SET_ERR_RESULT(treeWidget != NULL, "Tree widget not found", NULL);

    QTreeWidgetItem *item = getTreeWidgetItem(treeWidget, itemName);
    CHECK_SET_ERR_RESULT(item != NULL, "Item " + itemName + " not found in tree widget", NULL);

    return item;
}

}
