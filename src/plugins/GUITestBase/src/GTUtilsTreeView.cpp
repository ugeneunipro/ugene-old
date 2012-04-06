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

#include "GTUtilsTreeView.h"
#include "GTUtilsProjectTreeView.h"
#include "api/GTMouseDriver.h"
#include <QtGui/QTreeWidget>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsTreeView"

#define GT_METHOD_NAME "expandTo"
void GTUtilsTreeView::expandTo(U2OpStatus &os, QTreeWidget *treeWidget, QTreeWidgetItem* item) {

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


#define GT_METHOD_NAME "doubleClickOnItem"
void GTUtilsTreeView::doubleClickOnItem(U2OpStatus &os, const QString &itemName)
{
    QPoint itemPos = GTUtilsProjectTreeView::getTreeViewItemPosition(os, itemName);
    GTMouseDriver::moveTo(os, itemPos);
    GTMouseDriver::doubleClick(os);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
