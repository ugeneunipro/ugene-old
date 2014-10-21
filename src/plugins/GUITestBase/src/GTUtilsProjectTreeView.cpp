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

#include <QtGui/QDropEvent>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMainWindow>
#include <QtGui/QTreeView>
#else
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTreeView>
#endif

#include "GTUtilsProjectTreeView.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWidget.h"
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Gui/MainWindow.h>

#include "runnables/qt/PopupChooser.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsProjectTreeView"

const QString GTUtilsProjectTreeView::widgetName = "documentTreeWidget";

#define GT_METHOD_NAME "openView"
void GTUtilsProjectTreeView::openView(U2OpStatus& os) {

    GTGlobals::FindOptions options;
    options.failIfNull = false;

    QWidget *documentTreeWidget = GTWidget::findWidget(os, widgetName, NULL, options);
    if (!documentTreeWidget) {
        toggleView(os);
    }
    GTGlobals::sleep(100);

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

#define GT_METHOD_NAME "getItemCenter"
QPoint GTUtilsProjectTreeView::getItemCenter(U2OpStatus &os, const QModelIndex &itemIndex) {
    GT_CHECK_RESULT(itemIndex.isValid(), "Item index is invalid", QPoint());

    QTreeView *treeView = getTreeView(os);
    return getItemCenter(os, treeView, itemIndex);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemCenter"
QPoint GTUtilsProjectTreeView::getItemCenter(U2OpStatus &os, QTreeView *treeView, const QModelIndex &itemIndex) {
    GT_CHECK_RESULT(itemIndex.isValid(), "Item index is invalid", QPoint());
    GT_CHECK_RESULT(NULL != treeView, "treeView is NULL", QPoint());

    QRect r = treeView->visualRect(itemIndex);

    return treeView->mapToGlobal(r.center());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "rename"
void GTUtilsProjectTreeView::rename(U2OpStatus &os, const QString &itemName, const QString &newItemName, GTGlobals::UseMethod invokeMethod) {

    GTMouseDriver::moveTo(os, getItemCenter(os, itemName));

    switch (invokeMethod) {
    case GTGlobals::UseKey :
        GTMouseDriver::click(os);
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["F2"]);
        break;
    case GTGlobals::UseMouse :
        GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Edit" << "Rename...", GTGlobals::UseMouse));
        GTMouseDriver::click(os, Qt::RightButton);
        break;
    default:
        os.setError("An unsupported way of a rename procedure invocation");
        return;
    }

    GTKeyboardDriver::keySequence(os, newItemName);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);

    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemCenter"
QPoint GTUtilsProjectTreeView::getItemCenter(U2OpStatus &os, const QString &itemName) {
    return getItemCenter(os, findIndex(os, itemName));
}
#undef GT_METHOD_NAME



#define GT_METHOD_NAME "scrollTo"
void GTUtilsProjectTreeView::scrollTo(U2OpStatus &os, const QString &itemName)
{
    QTreeView* treeView = getTreeView(os);
    GT_CHECK(treeView != NULL, "tree view not found");

    treeView->scrollTo(findIndex(os, itemName));

}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "doubleClickItem"
void GTUtilsProjectTreeView::doubleClickItem(U2OpStatus &os, const QModelIndex &itemIndex) {
    GT_CHECK(itemIndex.isValid(), "Item index is invalid");
    getTreeView(os)->scrollTo(itemIndex);

    GTMouseDriver::moveTo(os, getItemCenter(os, itemIndex));
    GTMouseDriver::doubleClick(os);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "doubleClickItem"
void GTUtilsProjectTreeView::doubleClickItem(U2OpStatus &os, const QString &itemName) {
    doubleClickItem(os, findIndex(os, itemName));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "click"
void GTUtilsProjectTreeView::click(U2OpStatus &os, const QString &itemName, Qt::MouseButton button) {
    QModelIndex itemIndex = findIndex(os, itemName);
    GT_CHECK(itemIndex.isValid(), "Item index is invalid");
    getTreeView(os)->scrollTo(itemIndex);

    GTMouseDriver::moveTo(os, getItemCenter(os, itemIndex));
    GTMouseDriver::click(os, button);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getTreeWidget"
QTreeView* GTUtilsProjectTreeView::getTreeView(U2OpStatus &os) {

    openView(os);

    QTreeView *treeView = qobject_cast<QTreeView*>(GTWidget::findWidget(os, widgetName));
    return treeView;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findItem"
QModelIndex GTUtilsProjectTreeView::findIndex(U2OpStatus &os, const QString &itemName, const GTGlobals::FindOptions &options) {
    QTreeView *treeView = getTreeView(os);
    GT_CHECK_RESULT(treeView != NULL, "Tree view is NULL", QModelIndex());
    return findIndex(os, treeView, itemName, QModelIndex(), options);
}

#define GT_METHOD_NAME "findItem"
QModelIndex GTUtilsProjectTreeView::findIndex(U2OpStatus &os, QTreeView *treeView, const QString &itemName, const GTGlobals::FindOptions &options) {
    GT_CHECK_RESULT(treeView != NULL, "Tree view is NULL", QModelIndex());
    return findIndex(os, treeView, itemName, QModelIndex(), options);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findItem"
QModelIndex GTUtilsProjectTreeView::findIndex(U2OpStatus &os, const QString &itemName, const QModelIndex& parent, const GTGlobals::FindOptions &options) {
    QTreeView *treeView = getTreeView(os);
    GT_CHECK_RESULT(treeView != NULL, "Tree view is NULL", QModelIndex());
    return findIndex(os, treeView, itemName, parent, options);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findItem"
QModelIndex GTUtilsProjectTreeView::findIndex(U2OpStatus &os, QTreeView *treeView, const QString &itemName, const QModelIndex &parent, const GTGlobals::FindOptions &options) {
    GT_CHECK_RESULT(treeView != NULL, "Tree view is NULL", QModelIndex());
    GT_CHECK_RESULT(itemName.isEmpty() == false, "Item name is empty", QModelIndex());

    QModelIndexList foundIndexes = findIndecies(os, treeView, itemName, parent, 0, options);
    if (foundIndexes.isEmpty()) {
        if(options.failIfNull){
            GT_CHECK_RESULT(foundIndexes.size() != 0, QString("Item with name %1 not found").arg(itemName), QModelIndex());
        } else {
            return QModelIndex();
        }
    }

    GT_CHECK_RESULT(foundIndexes.size() == 1, QString("there are %1 items with name %2").arg(foundIndexes.size()).arg(itemName), QModelIndex());

    treeView->scrollTo(foundIndexes.at(0));
    return foundIndexes.at(0);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findIndecies"
QModelIndexList GTUtilsProjectTreeView::findIndecies(U2OpStatus &os, const QString &itemName, const QModelIndex &parent, int parentDepth, const GTGlobals::FindOptions &options) {
    QTreeView *treeView = getTreeView(os);
    GT_CHECK_RESULT(treeView != NULL, "Tree widget is NULL", QModelIndexList());

    return findIndecies(os, treeView, itemName, parent, parentDepth, options);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findIndecies"
QModelIndexList GTUtilsProjectTreeView::findIndecies(U2OpStatus &os,
                                                     QTreeView* treeView,
                                                    const QString &itemName,
                                                    const QModelIndex& parent,
                                                    int parentDepth,
                                                    const GTGlobals::FindOptions &options) {
    QModelIndexList foundIndecies;
    CHECK(GTGlobals::FindOptions::INFINITE_DEPTH == options.depth || parentDepth < options.depth, foundIndecies);

    ProjectViewModel *model = qobject_cast<ProjectViewModel*>(treeView->model());
    CHECK_SET_ERR_RESULT(NULL != model, "Model is NULL", foundIndecies);

    int rowcount = model->rowCount(parent);
    for (int i = 0; i < rowcount; i++) {
        const QModelIndex index = model->index(i, 0, parent);
        QString s = index.data(Qt::DisplayRole).toString();

        GObject* object = model->toObject(index);
        if (NULL != object) {
            const QString prefix = "[" + GObjectTypes::getTypeInfo(object->getGObjectType()).treeSign + "]";
            if (s.startsWith(prefix) || prefix == "[u]") {
                s = s.mid(prefix.length() + 1);
            }

        }else{
            const QString unload = "[unloaded]";
            if(s.startsWith(unload)){
                s = s.mid(unload.length());
            }
        }

        if (!itemName.isEmpty()) {
            if (s == itemName) {
                foundIndecies << index;
            } else {
                foundIndecies <<  findIndecies(os, treeView, itemName, index, parentDepth + 1, options);
            }
        } else {
            foundIndecies << index;
            foundIndecies << findIndecies(os, treeView, itemName, index, parentDepth + 1, options);
        }
    }

    return foundIndecies;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkItem"
bool GTUtilsProjectTreeView::checkItem(U2OpStatus &os, const QString &itemName, const GTGlobals::FindOptions &options) {
    QTreeView *treeView = getTreeView(os);
    GT_CHECK_RESULT(treeView != NULL, "Tree view is NULL", false);
    return checkItem(os, treeView, itemName, QModelIndex(), options);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkItem"
bool GTUtilsProjectTreeView::checkItem(U2OpStatus &os, const QString &itemName, const QModelIndex &parent, const GTGlobals::FindOptions &options) {
    QTreeView *treeView = getTreeView(os);
    GT_CHECK_RESULT(treeView != NULL, "Tree view is NULL", false);
    return checkItem(os, treeView, itemName, parent, options);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkItem"
bool GTUtilsProjectTreeView::checkItem(U2OpStatus &os, QTreeView *treeView, const QString &itemName, const GTGlobals::FindOptions &options) {
    GT_CHECK_RESULT(treeView != NULL, "Tree view is NULL", false);
    return checkItem(os, treeView, itemName, QModelIndex(), options);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkItem"
bool GTUtilsProjectTreeView::checkItem(U2OpStatus &os, QTreeView *treeView, const QString &itemName, const QModelIndex &parent, const GTGlobals::FindOptions &options) {
    GT_CHECK_RESULT(treeView != NULL, "Tree view is NULL", false);
    GT_CHECK_RESULT(itemName.isEmpty() == false, "Item name is empty", false);

    QModelIndexList foundIndexes = findIndecies(os, treeView, itemName, parent, 0, options);
    return !foundIndexes.isEmpty();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkObjectTypes"
void GTUtilsProjectTreeView::checkObjectTypes(U2OpStatus &os, QTreeView *treeView, const QSet<GObjectType> &acceptableTypes, const QModelIndex &parent) {
    CHECK_SET_ERR(NULL != treeView, "Invalid tree view detected");
    CHECK(!acceptableTypes.isEmpty(), );

    ProjectViewModel *model = qobject_cast<ProjectViewModel *>(treeView->model());
    CHECK_SET_ERR(NULL != model, "Invalid view model detected");

    const int rowCount = model->rowCount(parent);
    for (int i = 0; i < rowCount; i++) {
        const QModelIndex index = model->index(i, 0, parent);
        GObject *object = model->toObject(index);
        if (NULL != object && Qt::NoItemFlags != model->flags(index) && !acceptableTypes.contains(object->getGObjectType()))
            CHECK_SET_ERR(NULL == object || Qt::NoItemFlags == model->flags(index) || acceptableTypes.contains(object->getGObjectType()), "Object has unexpected type");

        if (NULL == object) {
            checkObjectTypes(os, treeView, acceptableTypes, index);
            CHECK_OP_BREAK(os);
        }
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSelectedItem"
QString GTUtilsProjectTreeView::getSelectedItem(U2OpStatus &os)
{
    QTreeView* treeView = getTreeView(os);
    GT_CHECK_RESULT(treeView != NULL, "tree view is NULL", NULL);

    QModelIndexList list = treeView->selectionModel()->selectedIndexes();
    GT_CHECK_RESULT(list.size() != 0, QString("there are no selected items"), "");
    GT_CHECK_RESULT(list.size() == 1, QString("there are %1 selected items").arg(list.size()), "");

    QModelIndex index = list.at(0);
    QString result = index.data(Qt::DisplayRole).toString();

    return result;

}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getFont"
QFont GTUtilsProjectTreeView::getFont(U2OpStatus &os, QModelIndex index){
    QTreeView* treeView = getTreeView(os);
    QAbstractItemModel* model = treeView->model();
    QFont result = qvariant_cast<QFont>(model->data(index, Qt::FontRole));
    return result;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getIcon"
QIcon GTUtilsProjectTreeView::getIcon(U2OpStatus &os, QModelIndex index){
    QTreeView* treeView = getTreeView(os);
    QAbstractItemModel* model = treeView->model();
    QIcon result = qvariant_cast<QIcon>(model->data(index, Qt::DecorationRole));
    return result;
}
#undef GT_METHOD_NAME


#define GT_METHOD_NAME "itemModificationCheck"
void GTUtilsProjectTreeView::itemModificationCheck(U2OpStatus &os, QModelIndex index, bool modified ){
    GT_CHECK(index.isValid(), "item is valid");
    QVariant data = index.data(Qt::TextColorRole);
    bool modState = !(QVariant() == data);
    GT_CHECK(modState == modified, "Document's " + index.data(Qt::DisplayRole).toString() + " modification state not equal with expected");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "itemActiveCheck"
void GTUtilsProjectTreeView::itemActiveCheck(U2OpStatus &os, QModelIndex index, bool active) {
    GT_CHECK(index.isValid(), "item is NULL");
    QVariant data = index.data(Qt::FontRole);

    bool modState = !(QVariant() == data);
    GT_CHECK(modState == active, "Document's " + index.data(Qt::FontRole).toString() + " active state not equal with expected");
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

void GTUtilsProjectTreeView::dragAndDrop(U2OpStatus &os, QModelIndex from, QModelIndex to){
    QTreeView* tree = getTreeView(os);
    QAbstractItemModel *model = tree->model();
    QMimeData *mimeData = model->mimeData(QModelIndexList()<<from);

    QPoint enterPos = getItemCenter(os, from);
    QPoint dropPos = getItemCenter(os, to);

    sendDragAndDrop(os, mimeData, enterPos, dropPos);
}

void GTUtilsProjectTreeView::dragAndDrop(U2OpStatus &os, QModelIndex from, QWidget *to){
    QAbstractItemModel *model = getTreeView(os)->model();
    QMimeData *mimeData = model->mimeData(QModelIndexList()<<from);

    QPoint enterPos = getItemCenter(os, from);

    sendDragAndDrop(os, mimeData, enterPos, to);
}

void GTUtilsProjectTreeView::dragAndDropSeveralElements(U2OpStatus &os, QModelIndexList from, QModelIndex to){
    QTreeView *treeView = getTreeView(os);
#ifdef Q_OS_MAC
    int key = GTKeyboardDriver::key["cmd"];
#else
    int key = GTKeyboardDriver::key["ctrl"];
#endif
    GTKeyboardDriver::keyPress(os, key);
    foreach (QModelIndex index, from){
        treeView->scrollTo(index);
        GTMouseDriver::moveTo(os, getItemCenter(os, index));
        GTMouseDriver::click(os);
    }
    GTKeyboardDriver::keyRelease(os, key);

    QModelIndexList selected = treeView->selectionModel()->selectedIndexes();
    QMimeData* mimeData = treeView->model()->mimeData(selected);

    QPoint enterPos = getItemCenter(os, from.at(0));
    QPoint dropPos = getItemCenter(os, to);

    sendDragAndDrop(os, mimeData, enterPos, dropPos);
}

void GTUtilsProjectTreeView::sendDragAndDrop(U2OpStatus &os, QMimeData *mimeData, QPoint enterPos, QPoint dropPos){
    QTreeView *treeView = getTreeView(os);
    QAbstractItemModel *model = treeView->model();
    QWidget* veiwPort = treeView->findChild<QWidget*>("qt_scrollarea_viewport");

    QPoint localEnterPos = treeView->mapFromGlobal(enterPos);
    QPoint localDropPos = treeView->mapFromGlobal(dropPos);

    GTMouseDriver::moveTo(os, enterPos);
    Qt::DropActions dropActions = model->supportedDropActions();

    QDragEnterEvent* dragEnterEvent = new QDragEnterEvent(localEnterPos, dropActions, mimeData, Qt::LeftButton, 0);
    GTGlobals::sendEvent(veiwPort, dragEnterEvent);


    GTMouseDriver::moveTo(os, dropPos);


    QDragMoveEvent* dragmoveEvent = new QDragMoveEvent(localDropPos, dropActions, mimeData, Qt::LeftButton, 0);
    GTGlobals::sendEvent(veiwPort, dragmoveEvent );

    QDropEvent* dropEvent = new QDropEvent(localDropPos, dropActions, mimeData, Qt::LeftButton, 0);
    GTGlobals::sendEvent(veiwPort, dropEvent);
}

void GTUtilsProjectTreeView::sendDragAndDrop(U2OpStatus &os, QMimeData *mimeData, QPoint enterPos, QWidget *dropWidget){
    QTreeView *treeView = getTreeView(os);
    QAbstractItemModel *model = treeView->model();
    treeView->findChild<QWidget*>("qt_scrollarea_viewport");

    treeView->mapFromGlobal(enterPos);

    GTMouseDriver::moveTo(os, enterPos);
    Qt::DropActions dropActions = model->supportedDropActions();

    QDragEnterEvent* dragEnterEvent = new QDragEnterEvent(dropWidget->geometry().center(), dropActions, mimeData, Qt::LeftButton, 0);
    GTGlobals::sendEvent(dropWidget, dragEnterEvent);


    GTMouseDriver::moveTo(os, dropWidget->mapToGlobal(dropWidget->geometry().center()));

    QDragMoveEvent* dragmoveEvent = new QDragMoveEvent(dropWidget->geometry().center(), dropActions, mimeData, Qt::LeftButton, 0);
    GTGlobals::sendEvent(dropWidget, dragmoveEvent );

    QDropEvent* dropEvent = new QDropEvent(dropWidget->geometry().center(), dropActions, mimeData, Qt::LeftButton, 0);
    GTGlobals::sendEvent(dropWidget, dropEvent);
}

#undef GT_CLASS_NAME

}
