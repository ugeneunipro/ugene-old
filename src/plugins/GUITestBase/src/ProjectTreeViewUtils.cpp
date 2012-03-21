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

#include "ProjectTreeViewUtils.h"
#include "QtUtils.h"
#include "ProjectUtils.h"

#include <U2Core/U2SafePoints.h>

#include <U2Gui/ProjectView.h>
#include <U2Core/ProjectModel.h>
#include <QtGui/QTreeWidget>
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"

namespace U2 {

const QString ProjectTreeViewUtils::widgetName = "documentTreeWidget";

void ProjectTreeViewUtils::openView(U2OpStatus& os) {

    QWidget *documentTreeWidget = QtUtils::findWidgetByName(os, widgetName, NULL, false);
    if (!documentTreeWidget) {
        toggleView(os);
    }
    QtUtils::sleep(500);
}

void ProjectTreeViewUtils::toggleView(U2OpStatus& os) {

    GTKeyboardDriver::keyClick(os, '1', GTKeyboardDriver::key["alt"]);
}

void ProjectTreeViewUtils::rename(U2OpStatus &os, const QString &itemName, const QString &newItemName) {

    click(os, itemName);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTKeyboardDriver::keySequence(os, newItemName);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);

    QtUtils::sleep(500);
}

void ProjectTreeViewUtils::click(U2OpStatus &os, const QString &itemName) {

    moveTo(os, itemName);
    GTMouseDriver::click(os);
}

void ProjectTreeViewUtils::moveTo(U2OpStatus &os,const QString &itemName) {

    openView(os);
    moveToOpenedView(os, itemName);
}

void ProjectTreeViewUtils::moveToOpenedView(U2OpStatus &os, const QString &itemName) {

    QPoint p = getTreeViewItemPosition(os, itemName);
    GTMouseDriver::moveTo(os, p);
}

void ProjectTreeViewUtils::checkToolTip(U2OpStatus &os, const QString& itemName, const QString& tooltip) {

    moveTo(os, itemName);
    QtUtils::sleep(2000);
    ToolTipUtils::checkExistingToolTip(os, tooltip);
}

void ProjectTreeViewUtils::checkItem(U2OpStatus &os, const QString &itemName, bool exists) {

    QTreeWidgetItem* item = getTreeWidgetItem(os, itemName);
    if (exists) {
        CHECK_SET_ERR(item != NULL, "Item " + itemName + " not found in tree widget");
    }
    else {
        CHECK_SET_ERR(item == NULL, "Item " + itemName + " found in tree widget");
    }
}

QPoint ProjectTreeViewUtils::getTreeViewItemPosition(U2OpStatus &os, const QString &itemName) {

    QTreeWidget *treeWidget = getTreeWidget(os);
    CHECK_SET_ERR_RESULT(treeWidget != NULL, "treeWidget is NULL", QPoint());
    QTreeWidgetItem *item = getTreeWidgetItem(os, itemName);

    QPoint p = treeWidget->rect().center();
    if (item) {
        p = treeWidget->visualItemRect(item).center();
    }

    return treeWidget->mapToGlobal(p);
}

QTreeWidget* ProjectTreeViewUtils::getTreeWidget(U2OpStatus &os) {

    QTreeWidget *treeWidget = static_cast<QTreeWidget*>(QtUtils::findWidgetByName(os, widgetName));
    return treeWidget;
}

QString ProjectTreeViewUtils::getProjectTreeItemName(ProjViewItem* projViewItem) {

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

QList<ProjViewItem*> ProjectTreeViewUtils::getProjectViewItems(QTreeWidgetItem* root) {

    QList<ProjViewItem*> treeItems;

    for (int i=0; i<root->childCount(); i++) {
        treeItems.append((ProjViewItem*)root->child(i));
        treeItems.append(getProjectViewItems(root->child(i)));
    }

    return treeItems;
}


QTreeWidgetItem* ProjectTreeViewUtils::getTreeWidgetItem(QTreeWidget* tree, const QString &itemName) {

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

QTreeWidgetItem* ProjectTreeViewUtils::getTreeWidgetItem(U2OpStatus &os, const QString &itemName) {

    QTreeWidget *treeWidget = getTreeWidget(os);
    CHECK_SET_ERR_RESULT(treeWidget != NULL, "Tree widget not found", NULL);

    QTreeWidgetItem *item = getTreeWidgetItem(treeWidget, itemName);
    CHECK_SET_ERR_RESULT(item != NULL, "Item " + itemName + " not found in tree widget", NULL);

    return item;
}

}
