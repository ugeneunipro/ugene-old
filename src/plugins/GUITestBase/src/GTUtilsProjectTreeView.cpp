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

#include "GTUtilsProjectTreeView.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWidget.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsTaskTreeView.h"
#include <U2Core/ProjectModel.h>
#include <U2Gui/MainWindow.h>
#include <QtGui/QMainWindow>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsProjectTreeView"

const QString GTUtilsProjectTreeView::widgetName = "documentTreeWidget";

#define GT_METHOD_NAME "openView"
void GTUtilsProjectTreeView::openView(U2OpStatus& os) {

    GTGlobals::FindOptions options;
    options.failIfNull = false;

    GTGlobals::sleep(500);
    QWidget *documentTreeWidget = GTWidget::findWidget(os, widgetName, NULL, options);
    if (!documentTreeWidget) {
        toggleView(os);
    }
    GTGlobals::sleep(500);

    documentTreeWidget = GTWidget::findWidget(os, widgetName, NULL, options);
    GT_CHECK(documentTreeWidget != NULL, "Can't open document tree widget view, findWidget returned NULL");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "toggleView"
void GTUtilsProjectTreeView::toggleView(U2OpStatus& os) {

    MainWindow* mw = AppContext::getMainWindow();
    GT_CHECK(mw != NULL, "MainWindow is NULL");
    QMainWindow *qmw = mw->getQMainWindow();
    GT_CHECK(qmw != NULL, "QMainWindow is NULL");

    GTMouseDriver::moveTo(os, qmw->mapToGlobal(qmw->rect().center()));
    GTMouseDriver::click(os);

    GTKeyboardDriver::keyClick(os, '1', GTKeyboardDriver::key["alt"]);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "rename"
void GTUtilsProjectTreeView::rename(U2OpStatus &os, const QString &itemName, const QString &newItemName) {

    GTMouseDriver::moveTo(os, getItemCenter(os, itemName));
    GTMouseDriver::click(os);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
    GTKeyboardDriver::keySequence(os, newItemName);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);

    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemCenter"
QPoint GTUtilsProjectTreeView::getItemCenter(U2OpStatus &os, const QString &itemName) {

    QTreeWidget *treeWidget = getTreeWidget(os);
    QRect r = GTTreeWidget::getItemRect(os, findItem(os, itemName));

    GT_CHECK_RESULT(treeWidget != NULL, "treeWidget " + itemName + " is NULL", QPoint());

    return treeWidget->mapToGlobal(r.center());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemLocalCenter"
QPoint GTUtilsProjectTreeView::getItemLocalCenter(U2OpStatus &os, const QString &itemName)
{
    QTreeWidgetItem *item = GTUtilsProjectTreeView::findItem(os, itemName);
    QTreeWidget *treeWidget = item->treeWidget();
    GT_CHECK_RESULT(treeWidget != NULL, "treeWidget is NULL", QPoint());
    GT_CHECK_RESULT(item != NULL, "item is NULL", QPoint());

    QRect r = treeWidget->visualItemRect(item);
    return r.center();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "scrollTo"
void GTUtilsProjectTreeView::scrollTo(U2OpStatus &os, const QString &itemName)
{
    QTreeWidget *treeWidget = GTUtilsProjectTreeView::getTreeWidget(os);
    GT_CHECK(treeWidget != NULL, "QTreeWidget not found");

    QRect treeWidgetRect = treeWidget->rect();
    QPoint localItemPosition = GTUtilsProjectTreeView::getItemLocalCenter(os, itemName);

    while (! treeWidgetRect.contains(localItemPosition)) {
        GTMouseDriver::scroll(os, -1);
        GTGlobals::sleep(200);
        localItemPosition = GTUtilsProjectTreeView::getItemLocalCenter(os, itemName);
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getTreeWidget"
QTreeWidget* GTUtilsProjectTreeView::getTreeWidget(U2OpStatus &os) {

    openView(os);

    QTreeWidget *treeWidget = qobject_cast<QTreeWidget*>(GTWidget::findWidget(os, widgetName));
    return treeWidget;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getProjectTreeItemName"
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
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findItem"
QTreeWidgetItem* GTUtilsProjectTreeView::findItem(U2OpStatus &os, const QString &itemName, const GTGlobals::FindOptions &options) {

    GT_CHECK_RESULT(itemName.isEmpty() == false, "Item name is empty", NULL);

    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget is NULL", NULL);

    return findItem(os, treeWidget, itemName, options);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findItem"
QTreeWidgetItem* GTUtilsProjectTreeView::findItem(U2OpStatus &os, const QTreeWidget *treeWidget, const QString &itemName,
                                                  const GTGlobals::FindOptions &options) {

    GT_CHECK_RESULT(itemName.isEmpty() == false, "Item name is empty", NULL);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget is NULL", NULL);

    QList<QTreeWidgetItem*> treeItems = GTTreeWidget::getItems(treeWidget->invisibleRootItem());
    foreach (QTreeWidgetItem* item, treeItems) {
        QString treeItemName = getProjectTreeItemName((ProjViewItem*)item);
        qDebug() << treeItemName << " ";
        if (treeItemName == itemName) {
            return item;
        }
    }
    GT_CHECK_RESULT(options.failIfNull == false, "Item " + itemName + " not found in tree widget", NULL);

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findItemByText("
QTreeWidgetItem* GTUtilsProjectTreeView::findItemByText(U2OpStatus &os, const QTreeWidget *treeWidget, const QString &itemName,
                                                  const GTGlobals::FindOptions &options) {

    GT_CHECK_RESULT(itemName.isEmpty() == false, "Item name is empty", NULL);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget is NULL", NULL);

    QList<QTreeWidgetItem*> treeItems = GTTreeWidget::getItems(treeWidget->invisibleRootItem());
    foreach (QTreeWidgetItem* item, treeItems) {
        qDebug() << "Found = " << itemName << " has = " << item->text(0) ;
        if (item->text(0) == itemName) {
            return item;
        }
    }
    GT_CHECK_RESULT(options.failIfNull == false, "Item " + itemName + " not found in tree widget", NULL);

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSelectedItem"
QString GTUtilsProjectTreeView::getSelectedItem(U2OpStatus &os)
{
    QTreeWidget *treeWidget = getTreeWidget(os);
    GT_CHECK_RESULT(treeWidget != NULL, "Tree widget is NULL", NULL);

    QList<QTreeWidgetItem*> treeItems = GTTreeWidget::getItems(treeWidget->invisibleRootItem());
    foreach (QTreeWidgetItem* item, treeItems) {
        if (item->isSelected()) {
            return getProjectTreeItemName((ProjViewItem*)item);
        }
    }

    return QString();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "itemModificationCheck"
void GTUtilsProjectTreeView::itemModificationCheck(U2OpStatus &os, QTreeWidgetItem* item, bool modified ){
    GT_CHECK(item != NULL, "item is NULL");
    QVariant data = item->data(0, Qt::TextColorRole);
    bool modState = !(QVariant() == data);
    GT_CHECK(modState == modified, "Document's " + item->text(0) + " modification state not equal with expected");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "itemActiveCheck"
void GTUtilsProjectTreeView::itemActiveCheck(U2OpStatus &os, QTreeWidgetItem* item, bool active) {
    GT_CHECK(item != NULL, "item is NULL");
    QVariant data = item->data(0, Qt::FontRole);

    bool modState = !(QVariant() == data);
    GT_CHECK(modState == active, "Document's " + item->text(0) + " active state not equal with expected");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "isVisible"
bool GTUtilsProjectTreeView::isVisible( U2OpStatus &os ){
    GTGlobals::FindOptions options;
    options.failIfNull = false;
    QWidget *documentTreeWidget = GTWidget::findWidget(os, widgetName, NULL, options);
    if (documentTreeWidget) {
        return true;
    }else{
        return false;
    }
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
