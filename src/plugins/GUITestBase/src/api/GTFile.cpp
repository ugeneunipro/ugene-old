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

#include "GTFile.h"
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

namespace U2 {

PermissionsSetter::PermissionsSetter() {
}

PermissionsSetter::~PermissionsSetter() {
    foreach (const QString& path, previousState.keys()) {
        QFile file(path);
        QFile::Permissions p = file.permissions();

        p = previousState.value(path, p);
        file.setPermissions(p);
    }
}

bool PermissionsSetter::setPermissions(const QString& path, QFile::Permissions perm, bool recursive) {
    if (recursive) {
        return setRecursive(path, perm);
    } else {
        return setOnce(path, perm);
    }
}

bool PermissionsSetter::setRecursive(const QString& path, QFile::Permissions perm) {
    QFileInfo fileInfo(path);
    CHECK(fileInfo.exists(), false);
    CHECK(!fileInfo.isSymLink(), false);

    if (fileInfo.isDir()) {
        QDir dir(path);
        foreach (const QString& entryPath, dir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
            bool res = setRecursive(path + "/" + entryPath, perm);
            CHECK(res, res);
        }
    }

    bool res = setOnce(path, perm);

    return res;
}

bool PermissionsSetter::setOnce(const QString& path, QFile::Permissions perm) {
    QFileInfo fileInfo(path);
    CHECK(fileInfo.exists(), false);
    CHECK(!fileInfo.isSymLink(), false);

    QFile file(path);
    QFile::Permissions p = file.permissions();
    previousState.insert(path, p);

    p &= perm;
    return file.setPermissions(p);
}

#define GT_CLASS_NAME "GTFile"

const QString GTFile::backupPostfix = "_GT_backup";

#define GT_METHOD_NAME "equals"
bool GTFile::equals(U2OpStatus &os, const QString& path1, const QString& path2) {

    QFile f1(path1);
    QFile f2(path2);

    bool ok = f1.open(QIODevice::ReadOnly) && f2.open(QIODevice::ReadOnly);
    GT_CHECK_RESULT(ok, f1.errorString() + " " + f2.errorString(), false);

    QByteArray byteArray1 = f1.readAll();
    QByteArray byteArray2 = f2.readAll();

    GT_CHECK_RESULT((f1.error() == QFile::NoError) && (f2.error() == QFile::NoError), f1.errorString() + " " + f2.errorString(), false);

    return byteArray1 == byteArray2;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSize"
qint64 GTFile::getSize(U2OpStatus &os, const QString &path){
    QFile f(path);
    bool ok = f.open(QIODevice::ReadOnly);
    GT_CHECK_RESULT(ok, "file " + path + "not found",-1);

    int size = f.size();
    f.close();
    return size;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "copy"
void GTFile::copy(U2OpStatus &os, const QString& from, const QString& to) {

    QFile f2(to);
    bool ok = f2.open(QIODevice::ReadOnly);
    if (ok) {
        f2.remove();
    }

    bool copied = QFile::copy(from, to);
    GT_CHECK(copied == true, "can't copy <" + from + "> to <" + to + ">");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "copyDir"
void GTFile::copyDir(U2OpStatus &os, const QString& dirToCopy, const QString& dirToPaste) {

    QDir from;
    from.setFilter(QDir::Hidden | QDir::AllDirs | QDir::Files);
    from.setPath(dirToCopy);

    QString pastePath = dirToPaste;
    bool ok = QDir().mkpath(pastePath);
    GT_CHECK(ok, "could not create directory: " + pastePath);

    QFileInfoList list = from.entryInfoList();
    foreach(QFileInfo info, list){
        if(info.fileName()=="." || info.fileName()==".."){
            continue;
        }
        if (info.isFile()){
            copy(os, info.filePath(), pastePath  + '/' + info.fileName());
        }else if(info.isDir()){
            copyDir(os, info.filePath(), pastePath  + '/' + info.fileName());
        }

    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "removeDir"
void GTFile::removeDir(QString dirName)
{
    QDir dir(dirName);
    dir.setFilter(QDir::Hidden | QDir::AllDirs | QDir::Files);

    foreach (QFileInfo fileInfo, dir.entryInfoList()) {
        QString fileName = fileInfo.fileName();
        QString filePath = fileInfo.filePath();
        if (fileName != "." && fileName != "..") {
            QFile file(filePath);
            file.setPermissions(QFile::ReadOther | QFile::WriteOther);
            if(!file.remove(filePath)){
                QDir dir(filePath);
                if(!dir.rmdir(filePath)){
                    removeDir(filePath);
                }
            }
        }
    }
    dir.rmdir(dir.absoluteFilePath(dirName));
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "backup"
void GTFile::backup(U2OpStatus &os, const QString& path) {

    copy(os, path, path + backupPostfix);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "backupDir"
void GTFile::backupDir(U2OpStatus &os, const QString& path) {

    copyDir(os, path, path + backupPostfix);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "restore"
void GTFile::restore(U2OpStatus &os, const QString& path) {

    QFile backupFile(path + backupPostfix);

    bool ok = backupFile.open(QIODevice::ReadOnly);
    GT_CHECK(ok, "There is no backup file for <" + path + ">");

    QFile file(path);
    ok = file.open(QIODevice::ReadOnly);
    if (ok) {
        file.remove();
    }

    bool renamed = backupFile.rename(path);
    GT_CHECK(renamed == true, "restore of <" + path + "> can't be done");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "restoreDir"
void GTFile::restoreDir(U2OpStatus &os, const QString& path) {
    QDir backupDir(path + backupPostfix);
    bool exists = backupDir.exists();
    if(!exists){
        return;
    }

    QDir dir(path);
    exists = dir.exists();
    if (exists) {
        removeDir(dir.absolutePath());
    }

    bool renamed = backupDir.rename(path + backupPostfix, path);
    GT_CHECK(renamed == true, "restore of <" + path + "> can't be done");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "check"
bool GTFile::check(U2OpStatus &/*os*/, const QString& path) {
    QFile file(path);
    return file.exists();
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

} //namespace
