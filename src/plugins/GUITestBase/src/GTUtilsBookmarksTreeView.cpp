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

#include "GTUtilsBookmarksTreeView.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWidget.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsProjectTreeView.h"
#include <U2Core/ProjectModel.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/ObjectViewTreeController.h>
#include <QtGui/QMainWindow>
#include <QtGui/QTreeWidget>
#include <U2View/AnnotationsTreeView.h>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsBookmarksTreeView"

const QString GTUtilsBookmarksTreeView::widgetName = ACTION_BOOKMARK_TREE_VIEW;

#define GT_METHOD_NAME "getTreeWidget"
QTreeWidget* GTUtilsBookmarksTreeView::getTreeWidget(U2OpStatus &os) {

    QTreeWidget *treeWidget = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os, widgetName, NULL, false));

    if (!treeWidget) {
        GTUtilsProjectTreeView::toggleView(os);
        GTGlobals::sleep(3000);
    }

    treeWidget = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os, widgetName));
    return treeWidget;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findItem"
QTreeWidgetItem* GTUtilsBookmarksTreeView::findItem(U2OpStatus &os, const QString &itemName, const GTGlobals::FindOptions &options) {

    GT_CHECK_RESULT(itemName.isEmpty() == false, "Item name is empty", NULL);

    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget is NULL", NULL);

    for(int i =0; i< treeWidget->topLevelItemCount(); i++) {
        OVTViewItem* vi = static_cast<OVTViewItem*>(treeWidget->topLevelItem(i));
        if (vi->viewName == itemName) {
            return vi;
        }
        if (vi->isRootItem()){
            QList<QTreeWidgetItem*> treeItems = GTTreeWidget::getItems(vi);
            foreach (QTreeWidgetItem* item, treeItems) {
                if (item->text(0) == itemName) {
                    return item;
                }
            }
        }
    }
    GT_CHECK_RESULT(options.failIfNull == false, "Item " + itemName + " not found in tree widget", NULL);

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSelectedItem"
QString GTUtilsBookmarksTreeView::getSelectedItem(U2OpStatus &os)
{
    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget is NULL", NULL);

    for(int i =0; i< treeWidget->topLevelItemCount(); i++) {
        OVTViewItem* vi = static_cast<OVTViewItem*>(treeWidget->topLevelItem(i));
        if (vi->isSelected()) {
            return vi->viewName;
        }
    }

    return QString();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemCenter"
QPoint GTUtilsBookmarksTreeView::getItemCenter(U2OpStatus &os, const QString &itemName) {

    QTreeWidgetItem* item = findItem(os, itemName);
    GT_CHECK_RESULT(item != NULL, "Item " + itemName + " is NULL", QPoint());

    return GTTreeWidget::getItemCenter(os, item);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
