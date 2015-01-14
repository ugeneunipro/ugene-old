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

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/Folder.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SafePoints.h>

#include "ProjectUtils.h"

namespace U2 {

const int ProjectUtils::MAX_OBJS_TO_SHOW_LOAD_PROGRESS = 100;
const QString ProjectUtils::RECYCLE_BIN_FOLDER_PATH = U2ObjectDbi::ROOT_FOLDER + U2ObjectDbi::RECYCLE_BIN_FOLDER;

bool ProjectUtils::isConnectedDatabaseDoc(const Document *doc) {
    CHECK(NULL != doc, false);
    return (BaseDocumentFormats::DATABASE_CONNECTION == doc->getDocumentFormatId()) && (doc->isLoaded());
}

bool ProjectUtils::isDatabaseDoc(const Document *doc) {
    CHECK(NULL != doc, false);
    return BaseDocumentFormats::DATABASE_CONNECTION == doc->getDocumentFormatId();
}

bool ProjectUtils::isFolderInRecycleBin(const QString &folderPath) {
    return folderPath.startsWith(RECYCLE_BIN_FOLDER_PATH + U2ObjectDbi::PATH_SEP);
}

bool ProjectUtils::isFolderInRecycleBinSubtree(const QString &folderPath) {
    return (folderPath == RECYCLE_BIN_FOLDER_PATH || isFolderInRecycleBin(folderPath));
}

bool ProjectUtils::isSystemFolder(const QString &folderPath) {
    return folderPath == U2ObjectDbi::ROOT_FOLDER || folderPath == RECYCLE_BIN_FOLDER_PATH;
}

} // U2
