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

#ifndef _U2_FILE_STORAGE_UTILS_
#define _U2_FILE_STORAGE_UTILS_

#include <U2Core/AppFileStorage.h>

namespace U2 {

class U2CORE_EXPORT FileStorageUtils {
public:
    /**
     * Returns the url to the sorted BAM file for the source @bamUrl.
     * If this BAM file is not in the file storage then returns empty string.
     */
    static QString getSortedBamUrl(const QString &bamUrl, FileStorage::WorkflowProcess &process);

    /**
     * Adds to the file storage information about sorted BAM: url and hash
     */
    static void addSortedBamUrl(const QString &bamUrl, const QString &sortedBamUrl, FileStorage::WorkflowProcess &process);

    /**
     * Returns the url to the converted BAM file for the source @samUrl.
     * If this BAM file is not in the file storage then returns empty string.
     */
    static QString getSamToBamConvertInfo(const QString &samUrl, FileStorage::WorkflowProcess &process);
    /**
     * Adds to the file storage information about converted SAM file: url and hash
     */
    static void addSamToBamConvertInfo(const QString &samUrl, const QString &bamUrl, FileStorage::WorkflowProcess &process);
};

} // U2

#endif // _U2_FILE_STORAGE_UTILS_
