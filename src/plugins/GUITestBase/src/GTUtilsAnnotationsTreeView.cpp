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

#include <QMainWindow>
#include <QTreeWidget>

#include <U2Core/ProjectModel.h>

#include <U2Gui/MainWindow.h>

#include <U2View/AnnotationsTreeView.h>

#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsTaskTreeView.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTMouseDriver.h"
#include "api/GTTreeWidget.h"
#include "api/GTWidget.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/CreateAnnotationWidgetFiller.h"
#include "runnables/ugene/corelibs/U2Gui/EditQualifierDialogFiller.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsAnnotationsTreeView"

const QString GTUtilsAnnotationsTreeView::widgetName = "annotations_tree_widget";

QTreeWidget* GTUtilsAnnotationsTreeView::getTreeWidget(U2OpStatus &os) {

    QTreeWidget *treeWidget = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os, widgetName, GTUtilsMdi::activeWindow(os)));
    return treeWidget;
}

#define GT_METHOD_NAME "getAVItemName"
QString GTUtilsAnnotationsTreeView::getAVItemName(U2OpStatus &os, AVItem* avItem) {

    GT_CHECK_RESULT(avItem != NULL, "avItem is NULL", "");

    switch (avItem->type) {
        case AVItemType_Annotation:
            {
                AVAnnotationItem* avAnnotationItem = (AVAnnotationItem*)avItem;
                GT_CHECK_RESULT(avAnnotationItem != NULL, "avAnnotationItem is NULL", "");

                const Annotation annotation = avAnnotationItem->annotation;
                return annotation.getName();
            }
            break;

        case AVItemType_Group:
            {
                AVGroupItem* avGroupItem = (AVGroupItem*)avItem;
                GT_CHECK_RESULT(avGroupItem!= NULL, "avAnnotationItem is NULL", "");

                const AnnotationGroup group= avGroupItem->group;
                return group.getName();
            }
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

#define GT_METHOD_NAME "getQualifierValue"
QString GTUtilsAnnotationsTreeView::getQualifierValue(U2OpStatus &os, const QString &qualName, const QString &parentName) {
    getItemCenter(os, parentName);
    QTreeWidgetItem *qualItem = findItem(os, qualName);
    GT_CHECK_RESULT(NULL != qualItem, "Qualifier item not found", "");
    return qualItem->text(1);
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "findFirstAnnotation"
QTreeWidgetItem * GTUtilsAnnotationsTreeView::findFirstAnnotation(U2OpStatus &os, const GTGlobals::FindOptions &options) {
    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget is NULL", NULL);

    QList<QTreeWidgetItem*> treeItems = GTTreeWidget::getItems(treeWidget->invisibleRootItem());
    foreach (QTreeWidgetItem *item, treeItems) {
        AVItem *avItem = dynamic_cast<AVItem*>(item);
        GT_CHECK_RESULT(NULL != avItem, "Cannot convert QTreeWidgetItem to AVItem", NULL);
        if (AVItemType_Annotation == avItem->type) {
            return item;
        }
    }
    GT_CHECK_RESULT(options.failIfNull == false, "No items in tree widget", NULL);
    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findItem"
QTreeWidgetItem * GTUtilsAnnotationsTreeView::findItem(U2OpStatus &os, const QString &itemName, const GTGlobals::FindOptions &options) {

    GT_CHECK_RESULT(itemName.isEmpty() == false, "Item name is empty", NULL);

    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget is NULL", NULL);

    QList<QTreeWidgetItem*> treeItems = GTTreeWidget::getItems(treeWidget->invisibleRootItem());
    foreach (QTreeWidgetItem* item, treeItems) {
        QString treeItemName = item->text(0);
        if (treeItemName == itemName) {
            return item;
        }
    }
    GT_CHECK_RESULT(options.failIfNull == false, "Item " + itemName + " not found in tree widget", NULL);

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findItems"
QList<QTreeWidgetItem*> GTUtilsAnnotationsTreeView::findItems(U2OpStatus &os, const QString &itemName, const GTGlobals::FindOptions &options) {
    QList<QTreeWidgetItem*> result;
    GT_CHECK_RESULT(itemName.isEmpty() == false, "Item name is empty", result);

    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget is NULL", result);

    QList<QTreeWidgetItem*> treeItems = GTTreeWidget::getItems(treeWidget->invisibleRootItem());
    foreach (QTreeWidgetItem* item, treeItems) {
        QString treeItemName = item->text(0);
        if (treeItemName == itemName) {
            result.append(item);
        }
    }
    GT_CHECK_RESULT(options.failIfNull == false, "Item " + itemName + " not found in tree widget", result);

    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getAnnotationNamesOfGroup"
QStringList GTUtilsAnnotationsTreeView::getAnnotationNamesOfGroup(U2OpStatus &os, const QString &groupName) {
    Q_UNUSED(os);
    QStringList names;
    QTreeWidgetItem *groupItem = findItem(os, groupName);
    for (int i = 0; i < groupItem->childCount(); i++) {
        AVItem *avItem = dynamic_cast<AVItem *>(groupItem->child(i));
        GT_CHECK_RESULT(NULL != avItem, "Cannot convert QTreeWidgetItem to AVItem", QStringList());
        names << getAVItemName(os, avItem);
    }
    return names;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getAnnotatedRegionsOfGroup"
QList<U2Region> GTUtilsAnnotationsTreeView::getAnnotatedRegionsOfGroup(U2OpStatus &os, const QString &groupName) {
    Q_UNUSED(os);
    QList<U2Region> regions;
    QTreeWidgetItem *groupItem = findItem(os, groupName);
    GT_CHECK_RESULT(groupItem != NULL, QString("Cannot find group item '%1'").arg(groupName), regions);
    for (int i = 0; i < groupItem->childCount(); i++) {
        AVItem *avItem = dynamic_cast<AVItem *>(groupItem->child(i));
        GT_CHECK_RESULT(NULL != avItem, "Cannot convert QTreeWidgetItem to AVItem", QList<U2Region>());
        AVAnnotationItem* item = (AVAnnotationItem*)avItem;
        GT_CHECK_RESULT(item != NULL, "sdf", regions);
        regions << item->annotation.getRegions().toList();
    }
    return regions;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getAnnotatedRegionsOfGroup"
QList<U2Region> GTUtilsAnnotationsTreeView::getAnnotatedRegionsOfGroup(U2OpStatus &os, const QString &groupName, const QString &parentName) {
    Q_UNUSED(os);
    QList<U2Region> regions;
    QTreeWidgetItem *parentItem = findItem(os, parentName);
    GT_CHECK_RESULT( parentItem != NULL, "Parent item not found!", regions);

    QList<QTreeWidgetItem*> treeItems = GTTreeWidget::getItems(parentItem);
    foreach (QTreeWidgetItem* childItem, treeItems) {
        QString treeItemName = childItem->text(0);
        if (treeItemName == groupName) {
            for (int i = 0; i < childItem->childCount(); i++) {
                AVItem *avItem = dynamic_cast<AVItem *>(childItem->child(i));
                GT_CHECK_RESULT(NULL != avItem, "Cannot convert QTreeWidgetItem to AVItem", QList<U2Region>());
                AVAnnotationItem* item = (AVAnnotationItem*)avItem;
                GT_CHECK_RESULT(item != NULL, "sdf", regions);
                regions << item->annotation.getRegions().toList();
            }
        }
    }
    return regions;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findRegion"
bool GTUtilsAnnotationsTreeView::findRegion(U2OpStatus &os, const QString &itemName, const U2Region& r) {

    AVAnnotationItem* item = (AVAnnotationItem*)GTUtilsAnnotationsTreeView::findItem(os, itemName);
    CHECK_SET_ERR_RESULT(item != NULL, "Item " + itemName + " not found", false);
    const Annotation ann = item->annotation;

    U2Region neededRegion(r.startPos-1, r.length-r.startPos+1);

    bool found = false;
    QVector<U2Region> regions = ann.getRegions();
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

#define GT_METHOD_NAME "getAnnotatedRegions"
QList<U2Region> GTUtilsAnnotationsTreeView::getAnnotatedRegions(U2OpStatus &os) {
    QList<U2Region> res;

    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget is NULL", res);

    QList<QTreeWidgetItem*> treeItems = GTTreeWidget::getItems(treeWidget->invisibleRootItem());
    foreach (QTreeWidgetItem* item, treeItems) {
        AVAnnotationItem* annotationItem = static_cast<AVAnnotationItem*>(item);
        CHECK_OPERATION(annotationItem != NULL, continue);

        const Annotation ann = annotationItem->annotation;
        res.append( ann.getRegions().toList() );
    }
    return res;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "createQualifier"
void GTUtilsAnnotationsTreeView::createQualifier(U2OpStatus &os, const QString &qualName, const QString &qualValue, const QString &parentName) {
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_ADD" << "add_qualifier_action"));
    GTUtilsDialog::waitForDialog(os, new EditQualifierFiller(os, qualName, qualValue));
    GTMouseDriver::moveTo(os, getItemCenter(os, parentName));
    GTMouseDriver::click(os, Qt::RightButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectItems"
void GTUtilsAnnotationsTreeView::selectItems(U2OpStatus &os, const QStringList &items) {
    GT_CHECK_RESULT(items.size() != 0, "List of items to select is empty", );
    // remove previous selection
    QPoint p = getItemCenter(os, items.first());
    GTMouseDriver::moveTo(os, p);
    GTMouseDriver::click(os);
#ifdef Q_OS_MAC
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["cmd"]);
#else
    GTKeyboardDriver::keyPress(os, GTKeyboardDriver::key["ctrl"]);
#endif
    foreach (const QString& item, items) {
        QPoint p = getItemCenter(os, item);
        GTMouseDriver::moveTo(os, p);

        QTreeWidgetItem* treeItem = findItem(os, item);
        GT_CHECK_RESULT(treeItem != NULL, "Tree item is NULL", );
        if (!treeItem->isSelected()) {
            GTMouseDriver::click(os);
        }
    }
#ifdef Q_OS_MAC
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["cmd"]);
#else
    GTKeyboardDriver::keyRelease(os, GTKeyboardDriver::key["ctrl"]);
#endif
    GTGlobals::sleep();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "createAnnotation"
void GTUtilsAnnotationsTreeView::createAnnotation(U2OpStatus &os, const QString &groupName, const QString &annotationName, const QString &location, bool createNewTable, const QString &saveTo) {
    QTreeWidget *annotationsTreeView = getTreeWidget(os);
    GT_CHECK(NULL != annotationsTreeView, "No annotation tree view");
    GTWidget::click(os, annotationsTreeView);

    GTUtilsDialog::waitForDialog(os, new CreateAnnotationWidgetFiller(os, createNewTable, groupName, annotationName, location, saveTo));
    GTKeyboardDriver::keyClick(os, 'n', GTKeyboardDriver::key["ctrl"]);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "deleteItem"
void GTUtilsAnnotationsTreeView::deleteItem(U2OpStatus &os, const QString &itemName) {
    deleteItem(os, findItem(os, itemName));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "deleteItem"
void GTUtilsAnnotationsTreeView::deleteItem(U2OpStatus &os, QTreeWidgetItem *item) {
    GT_CHECK(item != NULL, "Item is NULL");
    GTTreeWidget::getItemCenter(os, item);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(100);
    GTUtilsTaskTreeView::waitTaskFinished(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "callContextMenuOnItem"
void GTUtilsAnnotationsTreeView::callContextMenuOnItem(U2OpStatus &os, const QString &itemName) {
    GTMouseDriver::moveTo(os, getItemCenter(os, itemName));
    GTMouseDriver::click(os, Qt::RightButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "callContextMenuOnQualifier"
void GTUtilsAnnotationsTreeView::callContextMenuOnQualifier(U2OpStatus &os, const QString &parentName, const QString &qualifierName) {
    getItemCenter(os, parentName);
    callContextMenuOnItem(os, qualifierName);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
