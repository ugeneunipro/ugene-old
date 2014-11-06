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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QHeaderView>
#include <QtGui/QTreeWidget>
#else
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTreeWidget>
#endif

#include "GTTreeWidget.h"
#include "GTUtilsProjectTreeView.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsTreeView"

#define GT_METHOD_NAME "expand"
void GTTreeWidget::expand(U2OpStatus &os, QTreeWidgetItem* item) {
    if (item == NULL) {
        return;
    }
    expand(os, item->parent());
    GT_CHECK(item->isHidden() == false, "parent item is hidden");

    QTreeWidget *treeWidget = item->treeWidget();
    GT_CHECK(item->isHidden() == false, "parent item is hidden");

    treeWidget->scrollToItem(item);

    QRect itemRect = treeWidget->visualItemRect(item);
    if (!item->isExpanded()) {
        QPoint p = QPoint(itemRect.left() - 8, itemRect.center().y());

        GTMouseDriver::moveTo(os, treeWidget->viewport()->mapToGlobal(p));
        GTMouseDriver::click(os);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkItem"
void GTTreeWidget::checkItem(U2OpStatus &os, QTreeWidgetItem *item, int column, GTGlobals::UseMethod method) {
    Q_UNUSED(os);
    GT_CHECK(NULL != item, "treeWidgetItem is NULL");
    GT_CHECK(0 <= column, "The column number is invalid");

    QTreeWidget *tree = item->treeWidget();
    GT_CHECK(NULL != tree, "The tree widget is NULL");

    const QRect itemRect = getItemRect(os, item);
    const QPoint indentationOffset(tree->indentation(), 0);
    const QPoint itemStartPos = itemRect.topLeft() - indentationOffset;
    const QPoint columnOffset(tree->columnViewportPosition(column), 0);
    const QPoint itemLevelOffset(getItemLevel(os, item) * tree->indentation(), 0);

    switch (method) {
    case GTGlobals::UseKeyBoard: {
        const QPoint cellCenterOffset(tree->columnWidth(column) / 2, itemRect.height() / 2);
        GTMouseDriver::moveTo(os, itemStartPos + itemLevelOffset + columnOffset + cellCenterOffset);
        GTMouseDriver::click(os);
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
        break;
    }
    case GTGlobals::UseMouse: {
        const QPoint magicCheckBoxOffset = QPoint(15, 35);
        GTMouseDriver::moveTo(os, tree->mapToGlobal(itemStartPos + itemLevelOffset + columnOffset + magicCheckBoxOffset));
        GTMouseDriver::click(os);
        coreLog.error(QString("column: %1").arg(columnOffset.x()));
        break;
    }
    default:
        GT_CHECK(false, "Method is not implemented");
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemRect"
QRect GTTreeWidget::getItemRect(U2OpStatus &os, QTreeWidgetItem* item) {
    GT_CHECK_RESULT(item != NULL, "treeWidgetItem is NULL", QRect());

    QTreeWidget *treeWidget = item->treeWidget();
    GT_CHECK_RESULT(treeWidget != NULL, "treeWidget is NULL", QRect());

    expand(os, item);
    GT_CHECK_RESULT(item->isHidden() == false, "item is hidden", QRect());

    QRect rect = treeWidget->visualItemRect(item);

    return rect;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemCenter"
QPoint GTTreeWidget::getItemCenter(U2OpStatus &os, QTreeWidgetItem* item) {

    GT_CHECK_RESULT(item != NULL, "item is NULL", QPoint());

    QTreeWidget *treeWidget = item->treeWidget();
    GT_CHECK_RESULT(treeWidget != NULL, "treeWidget is NULL", QPoint());

    QPoint p = getItemRect(os, item).center();

    return treeWidget->viewport()->mapToGlobal(p);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItems"
QList<QTreeWidgetItem*> GTTreeWidget::getItems(QTreeWidgetItem* root) {

    QList<QTreeWidgetItem*> treeItems;

    for (int i=0; i<root->childCount(); i++) {
        treeItems.append(root->child(i));
        treeItems.append(getItems(root->child(i)));
    }

    return treeItems;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItems"
QList<QTreeWidgetItem *> GTTreeWidget::getItems(U2OpStatus &os, QTreeWidget *treeWidget) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(NULL != treeWidget, "Tree widget is NULL", QList<QTreeWidgetItem *>());
    return getItems(treeWidget->invisibleRootItem());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemNames"
QStringList GTTreeWidget::getItemNames(U2OpStatus &os, QTreeWidget *treeWidget) {
    QStringList itemNames;
    QList<QTreeWidgetItem *> items = getItems(os, treeWidget);
    foreach (QTreeWidgetItem *item, items) {
        itemNames << item->text(0);
    }
    return itemNames;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findItem"
QTreeWidgetItem* GTTreeWidget::findItem(U2OpStatus &os, QTreeWidget *tree, const QString& text, QTreeWidgetItem *parent, int column, Qt::MatchFlags flags) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(tree != NULL, "tree widget is null", NULL);

    if (parent == NULL) {
        parent = tree->invisibleRootItem();
    }

    QList<QTreeWidgetItem *> list = getItems(parent);
    foreach (QTreeWidgetItem *item, list){
        const QString itemText = item->text(column);
        if (flags.testFlag(Qt::MatchExactly) && itemText == text) {
            return item;
        } else if (flags.testFlag(Qt::MatchContains) && itemText.contains(text)) {
            return item;
        }
    }

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "click"
void GTTreeWidget::click(U2OpStatus &os, QTreeWidgetItem *item){
    GT_CHECK(item != NULL, "item is NULL");
    QTreeWidget* tree = item->treeWidget();
    tree->scrollToItem(item);

    GTMouseDriver::moveTo(os, getItemCenter(os, item));
    GTMouseDriver::click(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemLevel"
int GTTreeWidget::getItemLevel(U2OpStatus &os, QTreeWidgetItem *item) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(item != NULL, "item is NULL", -1);

    int level = 0;
    while (NULL != item->parent()) {
        level++;
        item = item->parent();
    }

    return level;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
