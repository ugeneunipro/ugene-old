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

#include <QtGui/QTreeView>

#include <U2Core/U2ObjectDbi.h>

#include <U2Gui/MainWindow.h>

#include "GTUtilsDialog.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsSharedDatabaseDocument.h"
#include "GTUtilsTaskTreeView.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/corelibs/U2Gui/AddFolderDialogFiller.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsSharedDatabaseDocument"

#define GT_METHOD_NAME "getDatabaseDocumentByName"
Document *GTUtilsSharedDatabaseDocument::getDatabaseDocumentByName(U2OpStatus &os, const QString &name) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(!name.isEmpty(), "Name is empty", NULL);

    const QModelIndex databaseDocIndex = GTUtilsProjectTreeView::findIndex(os, name);
    GT_CHECK_RESULT(databaseDocIndex.isValid(), QString("Can't find the document with name '%1'").arg(name), NULL);

    return ProjectViewModel::toDocument(databaseDocIndex);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "disconnectDatabase"
void GTUtilsSharedDatabaseDocument::disconnectDatabase(U2OpStatus &os, Document *databaseDoc) {
    Q_UNUSED(os);
    GT_CHECK(NULL != databaseDoc, "databaseDoc is NULL");
    disconnectDatabase(os, databaseDoc->getName());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "disconnectDatabase"
void GTUtilsSharedDatabaseDocument::disconnectDatabase(U2OpStatus &os, const QString &name) {
    Q_UNUSED(os);
    GTUtilsDocument::removeDocument(os, name);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getFolderItem"
QModelIndex GTUtilsSharedDatabaseDocument::getItemIndex(U2OpStatus &os, Document *databaseDoc, const QString &itemPath) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(NULL != databaseDoc, "databaseDoc is NULL", QModelIndex());
    GT_CHECK_RESULT(!itemPath.isEmpty(), "Folder path is empty", QModelIndex());

    const QStringList folders = itemPath.split(U2ObjectDbi::PATH_SEP, QString::SkipEmptyParts);

    QModelIndex itemIndex = GTUtilsProjectTreeView::findIndex(os, databaseDoc->getName());
    CHECK(!folders.isEmpty(), itemIndex);

    foreach (const QString& folder, folders) {
        GTGlobals::FindOptions options;
        options.depth = 1;
        itemIndex = GTUtilsProjectTreeView::findIndex(os, folder, itemIndex, options);
        CHECK_OP_BREAK(os);
    }

    return itemIndex;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "createFolder"
void GTUtilsSharedDatabaseDocument::createFolder(U2OpStatus &os, Document *databaseDoc, const QString &parentFolderPath, const QString &newFolderName) {
    Q_UNUSED(os);
    GT_CHECK(NULL != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(!parentFolderPath.isEmpty(), "Parent folder path is empty");
    GT_CHECK(!newFolderName.isEmpty(), "New folder's name is empty");
    GT_CHECK(!newFolderName.contains(U2ObjectDbi::PATH_SEP), "New folder's name contains invalid characters");

    QModelIndex parentFolderIndex = getItemIndex(os, databaseDoc, parentFolderPath);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__ADD_MENU << ACTION_PROJECT__CREATE_FOLDER, GTGlobals::UseMouse));
    GTUtilsDialog::waitForDialog(os, new AddFolderDialogFiller(os, newFolderName, GTGlobals::UseMouse));

    GTMouseDriver::moveTo(os, GTUtilsProjectTreeView::getItemCenter(os, parentFolderIndex));
    GTMouseDriver::click(os, Qt::RightButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getItemPath"
QString GTUtilsSharedDatabaseDocument::getItemPath(U2OpStatus &os, const QModelIndex &itemIndex) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(itemIndex.isValid(), "Item index is invalid", QString());

    ProjectViewModel::Type itemType = ProjectViewModel::itemType(itemIndex);
    switch (itemType) {
    case ProjectViewModel::DOCUMENT:
        return U2ObjectDbi::ROOT_FOLDER;

    case ProjectViewModel::FOLDER: {
        Folder* folder = ProjectViewModel::toFolder(itemIndex);
        GT_CHECK_RESULT(NULL != folder, "Can't convert item to folder", QString());
        return folder->getFolderPath();
    }

    case ProjectViewModel::OBJECT: {
        QString folderPath;
        const QModelIndex parentItemIndex = itemIndex.parent();
        GT_CHECK_RESULT(parentItemIndex.isValid(), "Parent item index of the object item is invalid", QString());

        ProjectViewModel::Type parentItemType = ProjectViewModel::itemType(parentItemIndex);
        if (ProjectViewModel::DOCUMENT == parentItemType) {
            folderPath = U2ObjectDbi::ROOT_FOLDER;
        } else if (ProjectViewModel::FOLDER == parentItemType) {
            Folder* folder = ProjectViewModel::toFolder(parentItemIndex);
            GT_CHECK_RESULT(NULL != folder, "Can't convert parent item to folder", QString());
            folderPath = folder->getFolderPath();
        } else {
            GT_CHECK_RESULT(false, "Can't recognize the parent item", QString());
        }

        GObject* object = ProjectViewModel::toObject(itemIndex);
        GT_CHECK_RESULT(NULL != object, "Can't convert item to object", QString());
        return folderPath + U2ObjectDbi::PATH_SEP + object->getGObjectName();
    }

    default:
        GT_CHECK_RESULT(false, "Can't recognize the item", QString());
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "expantToItem"
void GTUtilsSharedDatabaseDocument::expantToItem(U2OpStatus &os, Document *databaseDoc, const QString &itemPath) {
    Q_UNUSED(os);
    GT_CHECK(NULL != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(!itemPath.isEmpty(), "Item path is empty");

    QStringList folders = itemPath.split(U2ObjectDbi::PATH_SEP, QString::SkipEmptyParts);
    GTGlobals::FindOptions findOptions;
    findOptions.depth = 1;
    const QModelIndex databaseDocIndex = GTUtilsProjectTreeView::findIndex(os, databaseDoc->getName(), findOptions);

    QTreeView* projectTreeView = GTUtilsProjectTreeView::getTreeView(os);
    GT_CHECK(NULL != projectTreeView, "Project tree view not found");
    projectTreeView->expand(databaseDocIndex);

    CHECK(!folders.isEmpty(), );
    folders.pop_back();

    QModelIndex prevFolderIndex = databaseDocIndex;
    foreach (const QString& folder, folders) {
        const QModelIndex folderIndex = GTUtilsProjectTreeView::findIndex(os, folder, prevFolderIndex, findOptions);
        GTUtilsProjectTreeView::doubleClickItem(os, folderIndex);
        prevFolderIndex = folderIndex;
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "expantToItem"
void GTUtilsSharedDatabaseDocument::expantToItem(U2OpStatus &os, Document *databaseDoc, const QModelIndex &itemIndex) {
    Q_UNUSED(os);
    GT_CHECK(NULL != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(itemIndex.isValid(), "Item index is invalid");

    const QString itemPath = getItemPath(os, itemIndex);
    expantToItem(os, databaseDoc, itemPath);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "doubleClickItem"
void GTUtilsSharedDatabaseDocument::doubleClickItem(U2OpStatus &os, Document *databaseDoc, const QString &itemPath) {
    Q_UNUSED(os);
    GT_CHECK(NULL != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(!itemPath.isEmpty(), "Item path is empty");

    expantToItem(os, databaseDoc, itemPath);
    const QModelIndex itemIndex = getItemIndex(os, databaseDoc, itemPath);
    GTUtilsProjectTreeView::doubleClickItem(os, itemIndex);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "doubleClickItem"
void GTUtilsSharedDatabaseDocument::doubleClickItem(U2OpStatus &os, Document *databaseDoc, const QModelIndex &itemIndex) {
    Q_UNUSED(os);
    GT_CHECK(NULL != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(itemIndex.isValid(), "Item index is invalid");

    expantToItem(os, databaseDoc, itemIndex);
    GTUtilsProjectTreeView::doubleClickItem(os, itemIndex);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openView"
void GTUtilsSharedDatabaseDocument::openView(U2OpStatus &os, Document *databaseDoc, const QString &itemPath) {
    Q_UNUSED(os);
    GT_CHECK(NULL != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(!itemPath.isEmpty(), "Item path is empty");

    doubleClickItem(os, databaseDoc, itemPath);
    GTGlobals::sleep(100);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(100);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openView"
void GTUtilsSharedDatabaseDocument::openView(U2OpStatus &os, Document *databaseDoc, const QModelIndex &itemIndex) {
    Q_UNUSED(os);
    GT_CHECK(NULL != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(itemIndex.isValid(), "Item index is invalid");

    doubleClickItem(os, databaseDoc, itemIndex);
    GTGlobals::sleep(100);
    GTUtilsTaskTreeView::waitTaskFinished(os);
    GTGlobals::sleep(100);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "callImportDialog"
void GTUtilsSharedDatabaseDocument::callImportDialog(U2OpStatus &os, Document *databaseDoc, const QString &itemPath) {
    Q_UNUSED(os);
    GT_CHECK(NULL != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(!itemPath.isEmpty(), "Item path is empty");

    const QModelIndex itemIndex = getItemIndex(os, databaseDoc, itemPath);
    callImportDialog(os, databaseDoc, itemIndex);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "callImportDialog"
void GTUtilsSharedDatabaseDocument::callImportDialog(U2OpStatus &os, Document *databaseDoc, const QModelIndex &itemIndex) {
    Q_UNUSED(os);
    GT_CHECK(NULL != databaseDoc, "databaseDoc is NULL");
    GT_CHECK(itemIndex.isValid(), "Item index is invalid");

    expantToItem(os, databaseDoc, itemIndex);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << ACTION_PROJECT__ADD_MENU << ACTION_PROJECT__IMPORT_TO_DATABASE, GTGlobals::UseMouse));

    const QPoint itemCenter = GTUtilsProjectTreeView::getItemCenter(os, itemIndex);
    GTMouseDriver::moveTo(os, itemCenter);
    GTMouseDriver::click(os, Qt::RightButton);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "ensureItemExists"
void GTUtilsSharedDatabaseDocument::ensureItemExists(U2OpStatus &os, Document *databaseDoc, const QString &itemPath) {
    Q_UNUSED(os);

    const QModelIndex itemIndex = getItemIndex(os, databaseDoc, itemPath);
    GT_CHECK(itemIndex.isValid(), QString("Item is invalid, item's path: '%1'").arg(itemPath));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "ensureItemsExist"
void GTUtilsSharedDatabaseDocument::ensureItemsExist(U2OpStatus &os, Document *databaseDoc, const QStringList &itemsPaths) {
    Q_UNUSED(os);

    foreach (const QString& itemPath, itemsPaths) {
        ensureItemExists(os, databaseDoc, itemPath);
        CHECK_OP(os, );
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "ensureThereAraNoItemsExceptListed"
void GTUtilsSharedDatabaseDocument::ensureThereAreNoItemsExceptListed(U2OpStatus &os, Document *databaseDoc, const QString& parentPath, const QStringList &itemsPaths) {
    Q_UNUSED(os);

    ensureItemsExist(os, databaseDoc, itemsPaths);
    CHECK_OP(os, );

    const QModelIndex parentIndex = getItemIndex(os, databaseDoc, parentPath);
    const QModelIndexList subIndecies = GTUtilsProjectTreeView::findIndecies(os, "", parentIndex);
    GT_CHECK(subIndecies.size() == itemsPaths.size(), QString("Parent item contains %1 subitems, expected % subitems").arg(subIndecies.size()).arg(itemsPaths.size()));
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
