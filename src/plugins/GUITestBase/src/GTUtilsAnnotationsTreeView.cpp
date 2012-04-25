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

#include "GTUtilsAnnotationsTreeView.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWidget.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsTaskTreeView.h"
#include <U2Core/ProjectModel.h>
#include <U2Gui/MainWindow.h>
#include <QtGui/QMainWindow>
#include <QtGui/QTreeWidget>
#include <U2View/AnnotationsTreeView.h>
#include <U2Core/AnnotationTableObject.h>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsAnnotationsTreeView"

const QString GTUtilsAnnotationsTreeView::widgetName = "annotations_tree_widget";

QTreeWidget* GTUtilsAnnotationsTreeView::getTreeWidget(U2OpStatus &os) {

    QTreeWidget *treeWidget = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os, widgetName));
    return treeWidget;
}

#define GT_METHOD_NAME "getAVItemName"
QString GTUtilsAnnotationsTreeView::getAVItemName(U2OpStatus &os, AVItem* avItem) {

    GT_CHECK_RESULT(avItem != NULL, "avItem is NULL", "");

    switch (avItem->type) {
        case AVItemType_Annotation:
            {
                AVAnnotationItem* avAnnotationItem= (AVAnnotationItem*)avItem;
                GT_CHECK_RESULT(avAnnotationItem != NULL, "avAnnotationItem is NULL", "");

                Annotation *annotation = avAnnotationItem->annotation;
                GT_CHECK_RESULT(annotation != NULL, "avAnnotationItem->annotation is NULL", "");
                return annotation->getAnnotationName();
            }
            break;

        case AVItemType_Group:
            break;

        case AVItemType_Qualifier:
            {
                AVQualifierItem* avQualifierItem = (AVQualifierItem*)avItem;
                GT_CHECK_RESULT(avQualifierItem != NULL, "avQualifierItem is NULL", "");
                return avQualifierItem->qName;
            }
            break;

        default:
            break;
    }

    return "";
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getTreeWidgetItem"
QTreeWidgetItem* GTUtilsAnnotationsTreeView::findItem(U2OpStatus &os, const QString &itemName, const GTGlobals::FindOptions &options) {

    GT_CHECK_RESULT(itemName.isEmpty() == false, "Item name is empty", NULL);

    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget is NULL", NULL);

    QList<QTreeWidgetItem*> treeItems = GTTreeWidget::getItems(treeWidget->invisibleRootItem());
    foreach (QTreeWidgetItem* item, treeItems) {
        QString treeItemName = getAVItemName(os, (AVItem*)item);
        if (treeItemName == itemName) {
            return item;
        }
    }
    GT_CHECK_RESULT(options.failIfNull == false, "Item " + itemName + " not found in tree widget", NULL);

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findRegion"
bool GTUtilsAnnotationsTreeView::findRegion(U2OpStatus &os, const QString &itemName, const U2Region& r) {

    AVAnnotationItem* item = (AVAnnotationItem*)GTUtilsAnnotationsTreeView::findItem(os, itemName);
    CHECK_SET_ERR_RESULT(item != NULL, "Item " + itemName + " not found", false);
    Annotation* ann = item->annotation;
    CHECK_SET_ERR_RESULT(ann != NULL, "Annotation is NULL", false);

    U2Region neededRegion(r.startPos-1, r.length-r.startPos+1);

    bool found = false;
    QVector<U2Region> regions = ann->getRegions();
    foreach (const U2Region& r, regions) {
        if (r.contains(neededRegion)) {
            found = true;
            break;
        }
    }

    return found;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSelectedItem"
QString GTUtilsAnnotationsTreeView::getSelectedItem(U2OpStatus &os)
{
    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget is NULL", NULL);

    QList<QTreeWidgetItem*> treeItems = GTTreeWidget::getItems(treeWidget->invisibleRootItem());
    foreach (QTreeWidgetItem* item, treeItems) {
        if (item->isSelected()) {
            return getAVItemName(os, (AVItem*)item);
        }
    }

    return QString();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemCenter"
QPoint GTUtilsAnnotationsTreeView::getItemCenter(U2OpStatus &os, const QString &itemName) {

	QTreeWidgetItem* item = findItem(os, itemName);
	GT_CHECK_RESULT(item != NULL, "Item " + itemName + " is NULL", QPoint());

	return GTTreeWidget::getItemCenter(os, item);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
