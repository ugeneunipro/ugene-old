/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "SettingsImpl.h"

#include <U2Core/AppContext.h>
#include <U2Core/Version.h>

#include <U2Core/CMDLineRegistry.h>
#include <U2Core/CMDLineCoreOptions.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>

namespace U2 {

static QString findKey(const QStringList& envList, const QString& key) {
    QString prefix = key + "=";
    QString result;
    foreach(const QString& var, envList) {
        if (var.startsWith(prefix)) {
            result = var.mid(prefix.length());
            break;
        }
    }
    return result;
}

static QString preparePath(const QString& pathName) {
    QString result = pathName.trimmed().replace("//", "/").replace("\\", "/");
    if (result.endsWith("/")) {
        result = result.left(result.length() - 1) ;
    }
    return result;
}



SettingsImpl::SettingsImpl(QSettings::Scope scope) {
    QString fileName;
    QStringList envList = QProcess::systemEnvironment();
    static const QString configFileName = QString("%1.ini").arg(U2_PRODUCT_NAME);

    if (scope == QSettings::UserScope) {
        // check for local cfg file -> portable UGENE
        bool localCfg = false;
        QDir dir(QDir::current());
        QFileInfoList dirEntries = dir.entryInfoList();
        foreach(const QFileInfo& entry, dirEntries) {
            if (entry.fileName() == configFileName) {

                fileName = entry.filePath();
                localCfg = true;
                break;
            }
        }
        if (!localCfg) {
            fileName = AppContext::getCMDLineRegistry()->getParameterValue( CMDLineCoreOptions::INI_FILE );
            if (fileName.isEmpty()) {
                fileName = findKey(envList, "UGENE_USER_INI");
            }
        }
    } else {
        fileName = findKey(envList, "UGENE_SYSTEM_INI");
    }
    if (fileName.isEmpty()) {
#ifdef Q_WS_MAC
        QSettings::Format format = QSettings::NativeFormat;
#else
        QSettings::Format format = QSettings::IniFormat;
#endif
        settings = new QSettings(format, scope, "Unipro", U2_PRODUCT_NAME, this);
    }  else {
        settings = new QSettings(fileName, QSettings::IniFormat, this);
    }
}

SettingsImpl::~SettingsImpl() {
    settings->sync();
}

bool SettingsImpl::contains(const QString& pathName) const {
    QMutexLocker lock(&threadSafityLock);

    QString key = preparePath(pathName);
    return settings->contains(key);
}

void SettingsImpl::remove(const QString& pathName) {
    QMutexLocker lock(&threadSafityLock);

    QString key = preparePath(pathName);
    settings->remove(key);
}


QVariant SettingsImpl::getValue(const QString& pathName, const QVariant& defaultValue) const {
    QMutexLocker lock(&threadSafityLock);

    QString path = pathName;
    QString key = preparePath(path);
    return settings->value(key, defaultValue);
}

void SettingsImpl::setValue(const QString& pathName, const QVariant& value) {
    QMutexLocker lock(&threadSafityLock);

    QString path = pathName;
    QString key = preparePath(path);
    settings->setValue(key, value);
}

QString SettingsImpl::toVersionKey(const QString& key) const {
    static QString VERSION_KEY_SUFFIX = "/" + Version::appVersion().text;
    
    if (key.endsWith("/")) {
        return key + VERSION_KEY_SUFFIX + "/";
    }
    return key + VERSION_KEY_SUFFIX;
}

QStringList SettingsImpl::getAllKeys(const QString& path) {
    QMutexLocker lock(&threadSafityLock);

    QString key = preparePath(path);
    settings->beginGroup(key);
    QStringList allKeys = settings->allKeys();
    settings->endGroup();
    return allKeys;
}


void SettingsImpl::sync() {
    QMutexLocker lock(&threadSafityLock);

    settings->sync();
}

QString SettingsImpl::fileName() const  {
    return settings->fileName();
}

}//namespace
