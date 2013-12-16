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

#ifndef _U2_GT_FILE_H_
#define _U2_GT_FILE_H_

#include "api/GTGlobals.h"
#include <QtCore/QFile>

namespace U2 {

class PermissionsSetter {
    // If UGENE crash in the test, permissions won't be restored.
public:
    PermissionsSetter();
    ~PermissionsSetter();

    bool setPermissions(const QString& path, QFile::Permissions perm, bool recursive = true);

private:
    bool setRecursive(const QString& path, QFile::Permissions perm);
    bool setOnce(const QString& path, QFile::Permissions perm);

    QMap<QString, QFile::Permissions> previousState;
};

class GTFile {
public:
    static bool equals(U2OpStatus &os, const QString&, const QString&);

    static qint64 getSize(U2OpStatus &os, const QString&);

    // backup file to the file with backupPostfix. fail the given file can't be opened
    static void backup(U2OpStatus &os, const QString&);

    // backup directory to the file with backupPostfix
    static void backupDir(U2OpStatus &os, const QString&);

    // copy file removing target file if exist
    static void copy(U2OpStatus &os, const QString& from, const QString& to);

    // copy directory removing target file if exist
    static void copyDir(U2OpStatus &os, const QString& dirToCopy, const QString& diToPaste);

    // copy file removing target file if exist
    static void removeDir(QString dirName);

    // restores file and deletes backup. fail if there is no backup or can't write to the given file
    static void restore(U2OpStatus &os, const QString&);

    // restores file and deletes backup. fail if there is no backup or can't write to the given file
    static void restoreDir(U2OpStatus &os, const QString&);

    // checks if file exists
    static bool check(U2OpStatus &os, const QString&);

    static const QString backupPostfix;
};

} //namespace

#endif
