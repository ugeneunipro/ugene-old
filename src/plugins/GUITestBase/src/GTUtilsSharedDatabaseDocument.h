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

#ifndef _U2_GUI_SHARED_DATABASE_DOCUMENT_UTILS_H_
#define _U2_GUI_SHARED_DATABASE_DOCUMENT_UTILS_H_

#include <QtCore/QModelIndex>

#include "api/GTGlobals.h"

namespace U2 {

class Document;

class GTUtilsSharedDatabaseDocument {
public:
    static Document* getDatabaseDocumentByName(U2OpStatus &os, const QString& name);

    static void disconnectDatabase(U2OpStatus& os, Document* databaseDoc);
    static void disconnectDatabase(U2OpStatus& os, const QString& name);

    static QModelIndex getItemIndex(U2OpStatus &os, Document* databaseDoc, const QString& itemPath, bool mustExist = true);

    static void createFolder(U2OpStatus &os, Document* databaseDoc, const QString& parentFolderPath, const QString& newFolderName);
    static void createPath(U2OpStatus &os, Document* databaseDoc, const QString& path);

    static QString getItemPath(U2OpStatus &os, const QModelIndex& itemIndex);

    static void expantToItem(U2OpStatus &os, Document* databaseDoc, const QString& itemPath);
    static void expantToItem(U2OpStatus &os, Document* databaseDoc, const QModelIndex& itemIndex);

    static void doubleClickItem(U2OpStatus &os, Document* databaseDoc, const QString& itemPath);
    static void doubleClickItem(U2OpStatus &os, Document* databaseDoc, const QModelIndex& itemIndex);

    static void openView(U2OpStatus &os, Document* databaseDoc, const QString& itemPath);
    static void openView(U2OpStatus &os, Document* databaseDoc, const QModelIndex& itemIndex);

    static void callImportDialog(U2OpStatus &os, Document* databaseDoc, const QString& itemPath);
    static void callImportDialog(U2OpStatus &os, Document* databaseDoc, const QModelIndex& itemIndex);

    static void ensureItemExists(U2OpStatus &os, Document* databaseDoc, const QString& itemPath);
    static void ensureItemsExist(U2OpStatus &os, Document* databaseDoc, const QStringList& itemsPaths);
    static void ensureThereAreNoItemsExceptListed(U2OpStatus &os, Document* databaseDoc, const QString &parentPath, const QStringList& itemsPaths);

    static void importFiles(U2OpStatus &os, Document* databaseDoc, const QString& dstFolderPath, const QStringList& filesPaths, const QVariantMap& options = QVariantMap());
    static void importDirs(U2OpStatus &os, Document* databaseDoc, const QString& dstFolderPath, const QStringList& dirsPaths, const QVariantMap& options = QVariantMap());
    static void importProjectItems(U2OpStatus &os, Document* databaseDoc, const QString& dstFolderPath, const QMap<QString, QStringList>& projectItems, const QVariantMap& options = QVariantMap());

private:
    static const QVariant convertProjectItemsPaths(const QMap<QString, QStringList>& projectItems);
};

}   // namespace U2

#endif // _U2_GUI_SHARED_DATABASE_DOCUMENT_UTILS_H_
