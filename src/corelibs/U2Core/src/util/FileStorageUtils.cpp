/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <QFileInfo>

#include <U2Core/AppContext.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "FileStorageUtils.h"

namespace U2 {

using namespace FileStorage;

static QString getCommonHashForFile(const QString &url) {
    QFileInfo info(url);
    uint modified = info.lastModified().toTime_t();

    return QByteArray::number(modified);
}

/**
 * Finds the destination file by @srcUrl and @role.
 * Checks hash for both destination and source files.
 * If hashes are ok then returns the destination file.
 */
static QString getFileToFileInfo(const QString &srcUrl, const QString &role, WorkflowProcess &process) {
    AppFileStorage *fileStorage = AppContext::getAppFileStorage();

    if (NULL != fileStorage) {
        U2OpStatus2Log os;
        QString dstUrl = fileStorage->getFileInfo(srcUrl, role, process, os);
        CHECK_OP(os, "");

        if (!dstUrl.isEmpty() && QFile::exists(dstUrl)) {
            QString srcHash = fileStorage->getFileInfo(srcUrl, StorageRoles::HASH, process, os);
            CHECK_OP(os, "");

            QString dstHash = fileStorage->getFileInfo(dstUrl, StorageRoles::HASH, process, os);
            CHECK_OP(os, "");

            if ((getCommonHashForFile(dstUrl) == dstHash) &&
                (getCommonHashForFile(srcUrl) == srcHash)) {
                FileInfo fToFInfo(srcUrl, role, dstUrl);
                fileStorage->addFileOwner(fToFInfo, process, os);
                CHECK_OP(os, "");

                return dstUrl;
            }
        }
    }

    return "";
}

void addFileToFileInfo(const FileInfo &fToFInfo, WorkflowProcess &process) {
    CHECK(fToFInfo.isFileToFileInfo(), );
    AppFileStorage *fileStorage = AppContext::getAppFileStorage();
    CHECK(NULL != fileStorage, );

    U2OpStatus2Log os;
    fileStorage->addFileInfo(fToFInfo, process, os);
    CHECK_OP(os, );

    FileInfo srcHashInfo(fToFInfo.getFile(), StorageRoles::HASH, getCommonHashForFile(fToFInfo.getFile()));
    fileStorage->addFileInfo(srcHashInfo, process, os);
    CHECK_OP(os, );

    FileInfo dstHashInfo(fToFInfo.getInfo(), StorageRoles::HASH, getCommonHashForFile(fToFInfo.getInfo()));
    fileStorage->addFileInfo(dstHashInfo, process, os);
    CHECK_OP(os, );
}

QString FileStorageUtils::getSortedBamUrl(const QString &bamUrl, WorkflowProcess &process) {
    return getFileToFileInfo(bamUrl, StorageRoles::SORTED_BAM, process);
}

QString FileStorageUtils::getSamToBamConvertInfo(const QString &samUrl, WorkflowProcess &process) {
    return getFileToFileInfo(samUrl, StorageRoles::SAM_TO_BAM, process);
}

void FileStorageUtils::addSortedBamUrl(const QString &bamUrl, const QString &sortedBamUrl, WorkflowProcess &process) {
    FileInfo fToFInfo(bamUrl, StorageRoles::SORTED_BAM, sortedBamUrl);
    addFileToFileInfo(fToFInfo, process);
}

void FileStorageUtils::addSamToBamConvertInfo(const QString &samUrl, const QString &bamUrl, WorkflowProcess &process) {
    FileInfo fToFInfo(samUrl, StorageRoles::SAM_TO_BAM, bamUrl);
    addFileToFileInfo(fToFInfo, process);
}

} // U2
