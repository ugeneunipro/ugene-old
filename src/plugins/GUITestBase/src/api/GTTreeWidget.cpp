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

#include "GTTreeWidget.h"
#include "GTUtilsProjectTreeView.h"
#include "api/GTMouseDriver.h"
#include <QtGui/QTreeWidget>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsTreeView"

#define GT_METHOD_NAME "expandTo"
void GTTreeWidget::expandTo(U2OpStatus &os, QTreeWidget *treeWidget, QTreeWidgetItem* item) {

    GT_CHECK(item != NULL, "item is NULL");
    GT_CHECK(treeWidget != NULL, "treeWidget is NULL");

    QTreeWidgetItem* parentItem = item->parent();
    if (!parentItem) {
        return;
    }

    expandTo(os, treeWidget, parentItem);
    GT_CHECK(parentItem->isHidden() == false, "parent item is hidden");

    QRect parentItemRect = treeWidget->visualItemRect(parentItem);

    if (!parentItem->isExpanded()) {
        QPoint p = treeWidget->mapToGlobal(QPoint(parentItemRect.left(), parentItemRect.height()/2));
        GTMouseDriver::moveTo(os, p);
        GTMouseDriver::click(os);
        GTGlobals::sleep(500);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemRect"
QRect GTTreeWidget::getItemRect(U2OpStatus &os, QTreeWidget* treeWidget, QTreeWidgetItem* item) {

    GT_CHECK_RESULT(treeWidget != NULL, "treeWidget is NULL", QRect());
    GT_CHECK_RESULT(item != NULL, "treeWidgetItem is NULL", QRect());

    expandTo(os, treeWidget, item);
    GT_CHECK_RESULT(item->isHidden() == false, "item is hidden", QRect());

    return treeWidget->visualItemRect(item);
}
#undef GT_METHOD_NAME


QList<QTreeWidgetItem*> GTTreeWidget::getItems(QTreeWidgetItem* root) {

    QList<QTreeWidgetItem*> treeItems;

    for (int i=0; i<root->childCount(); i++) {
        treeItems.append(root->child(i));
        treeItems.append(getItems(root->child(i)));
    }

    return treeItems;
}

#define GT_METHOD_NAME "doubleClickOnItem"
void GTTreeWidget::doubleClickOnItem(U2OpStatus &os, const QString &itemName)
{
    QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, itemName);
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "scrollTo"
void GTTreeWidget::scrollTo(U2OpStatus &os, const QString &itemName)
{
    QTreeWidget *treeWidget = GTUtilsProjectTreeView::getTreeWidget(os);
    GT_CHECK(treeWidget != NULL, "QTreeWidget not found");

    while (! treeWidget->rect().contains(GTUtilsProjectTreeView::getTreeViewItemLocalPosition(os, itemName))) {
        GTMouseDriver::scroll(os, -1);
        GTGlobals::sleep(200);
    }
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
