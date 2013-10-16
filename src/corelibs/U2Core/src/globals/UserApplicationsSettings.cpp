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

#include "UserApplicationsSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/GUrlUtils.h>

#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QProcessEnvironment>

#include <QtGui/QDesktopServices>
#include <QtGui/QApplication>
#include <QtGui/QStyle>
#include <QtGui/QStyleFactory>
#include <QtGui/QDialogButtonBox>
#if (QT_VERSION >= 0x050000)
#include <QStandardPaths>
#endif


namespace U2 {
    
#define SETTINGS_ROOT   QString("/user_apps/")
#define WEB_BROWSER     QString("web_browser")
#define USE_DEFAULT_WEB_BROWSER     QString("use_default_web_browser")
#define TRANSLATION     QString("translation_file")
#define LAST_PROJECT_FLAG QString("open_last_project")
#define SAVE_PROJECT_STATE QString("save_project")
#define VISUAL_STYLE    QString("style")
#define DOWNLOAD_DIR    QString("download_file")
#define RECENTLY_DOWNLOADED QString("recently_downloaded")
#define TEMPORARY_DIR QString("temporary_dir")
#define DATA_DIR QString("data_dir")
#define COLLECTING_STATISTICS QString("collecting_statistics")
#define WINDOW_LAYOUT  QString("tabbed_windows")
#define RESET_SETTINGS_FLAG QString("reset_settings")
#define FILE_STORAGE_DIR    QString("file_storage_dir")

//TODO: create a special ENV header to keep all env-vars ugene depends
#define UGENE_SKIP_TMP_DIR_CLEANUP "UGENE_SKIP_TMP_DIR_CLEANUP"

UserAppsSettings::UserAppsSettings() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    cleanupTmpDir = !env.contains(UGENE_SKIP_TMP_DIR_CLEANUP);
}


UserAppsSettings::~UserAppsSettings() {
    if (cleanupTmpDir) {
        QString path = getCurrentProcessTemporaryDirPath();
        coreLog.trace(tr("Cleaning temp dir: %1").arg(path));
        U2OpStatus2Log os;
        GUrlUtils::removeDir(path, os);
    }
}


QString UserAppsSettings::getWebBrowserURL() const {
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + WEB_BROWSER, QString("")).toString();
}

bool UserAppsSettings::useDefaultWebBrowser() const {
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + USE_DEFAULT_WEB_BROWSER, true).toBool();
}


void UserAppsSettings::setWebBrowserURL(const QString& url) {
    return AppContext::getSettings()->setValue(SETTINGS_ROOT + WEB_BROWSER, url);
}
void UserAppsSettings::setUseDefaultWebBrowser(bool state) {
    return AppContext::getSettings()->setValue(SETTINGS_ROOT + USE_DEFAULT_WEB_BROWSER, state);
}

QString UserAppsSettings::getTranslationFile() const {
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + TRANSLATION, QString("")).toString();
}
void UserAppsSettings::setTranslationFile(const QString& fn) {
    return AppContext::getSettings()->setValue(SETTINGS_ROOT + TRANSLATION, fn);
}

bool UserAppsSettings::openLastProjectAtStartup() const {
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + LAST_PROJECT_FLAG, true).toBool();
}

void UserAppsSettings::setOpenLastProjectAtStartup(bool v) {
    return AppContext::getSettings()->setValue(SETTINGS_ROOT + LAST_PROJECT_FLAG, v);
}

int UserAppsSettings::getAskToSaveProject() const {
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + SAVE_PROJECT_STATE, QDialogButtonBox::NoButton).toInt();
}

void UserAppsSettings::setAskToSaveProject(int v) {
    AppContext::getSettings()->setValue(SETTINGS_ROOT + SAVE_PROJECT_STATE, v);
}

bool UserAppsSettings::resetSettings() const {
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + RESET_SETTINGS_FLAG, false).toBool();
}

void UserAppsSettings::setResetSettings(bool b){
    AppContext::getSettings()->setValue(SETTINGS_ROOT + RESET_SETTINGS_FLAG, b);
}

QString UserAppsSettings::getVisualStyle() const {
    QString defaultStyle = QApplication::style()->objectName();
#ifdef Q_OS_WIN
#define DEFAULT_STYLE_NAME ".NET"
    const char* version = qVersion();
    if (QString("4.4.0")!=version) {
        if (QStyleFactory::keys().contains(DEFAULT_STYLE_NAME)) {
            defaultStyle = DEFAULT_STYLE_NAME;
        }
    }
#endif

    return AppContext::getSettings()->getValue(SETTINGS_ROOT + VISUAL_STYLE, defaultStyle).toString();
}

void UserAppsSettings::setVisualStyle(const QString& newStyle) {
    return AppContext::getSettings()->setValue(SETTINGS_ROOT + VISUAL_STYLE, newStyle.toLower());
}

QString UserAppsSettings::getDownloadDirPath() const {
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + DOWNLOAD_DIR, QDir::homePath()+"/.UGENE_downloaded").toString();
}

void UserAppsSettings::setDownloadDirPath(const QString& newPath) const {
    AppContext::getSettings()->setValue(SETTINGS_ROOT + DOWNLOAD_DIR, newPath);
}

QStringList UserAppsSettings::getRecentlyDownloadedFileNames() const {
    QStringList empty;
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + RECENTLY_DOWNLOADED, empty).toStringList();
}

void UserAppsSettings::setRecentlyDownloadedFileNames(const QStringList& fileNames) const {
    AppContext::getSettings()->setValue(SETTINGS_ROOT + RECENTLY_DOWNLOADED, fileNames);
}

QString UserAppsSettings::getUserTemporaryDirPath() const {
#if (QT_VERSION >= 0x050000)
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + TEMPORARY_DIR, QStandardPaths::writableLocation(QStandardPaths::TempLocation)).toString();
#else
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + TEMPORARY_DIR, QDesktopServices::storageLocation(QDesktopServices::TempLocation)).toString();
#endif

}

void UserAppsSettings::setUserTemporaryDirPath(const QString& newPath) {
    AppContext::getSettings()->setValue(SETTINGS_ROOT + TEMPORARY_DIR, newPath);
    emit si_temporaryPathChanged();
}

QString UserAppsSettings::getDefaultDataDirPath() const{
#if (QT_VERSION >= 0x050000)
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + DATA_DIR, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/UGENE_Data").toString();
#else
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + DATA_DIR, QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)+"/UGENE_Data").toString();
#endif
}

void UserAppsSettings::setDefaultDataDirPath( const QString& newPath ){
    AppContext::getSettings()->setValue(SETTINGS_ROOT + DATA_DIR, newPath);
}

bool UserAppsSettings::isStatisticsCollectionEnabled() const {
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + COLLECTING_STATISTICS).toBool();
}

void UserAppsSettings::setEnableCollectingStatistics(bool b) {
    AppContext::getSettings()->setValue(SETTINGS_ROOT + COLLECTING_STATISTICS, b);
}

bool UserAppsSettings::tabbedWindowLayout() const {
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + WINDOW_LAYOUT, false, true).toBool();
}

void UserAppsSettings::setTabbedWindowLayout(bool b) {
    AppContext::getSettings()->setValue(SETTINGS_ROOT + WINDOW_LAYOUT, b, true);
    emit si_windowLayoutChanged();
}


QString UserAppsSettings::getCurrentProcessTemporaryDirPath(const QString& domain) const {
    qint64 pid = QCoreApplication::applicationPid();
    QString tmpDirPath = getUserTemporaryDirPath() + "/" +  QString("ugene_tmp/p%1").arg(pid);
    if (!domain.isEmpty()) {
        tmpDirPath += "/" + domain;
    }
    return tmpDirPath;
}

static const int MAX_ATTEMPTS = 500;

QString UserAppsSettings::createCurrentProcessTemporarySubDir(U2OpStatus &os, const QString &domain) const {
    QDir baseDir(getCurrentProcessTemporaryDirPath(domain));
    if (!baseDir.exists()) {
        bool created = baseDir.mkpath(baseDir.absolutePath());
        if (!created) {
            os.setError(QString("Can not create the directory: %1").arg(baseDir.absolutePath()));
            return "";
        }
    }

    uint time = QDateTime::currentDateTime().toTime_t();
    QString baseDirName = QByteArray::number(time);

    // create sub dir
    int idx = 0;
    QString result;
    bool created = false;
    do {
        result = baseDirName + "_" + QByteArray::number(idx);
        created = baseDir.mkdir(result);
        idx++;

        if (idx > MAX_ATTEMPTS) {
            os.setError(QString("Can not create a sub-directory in: %1").arg(baseDir.absolutePath()));
            return "";
        }
    } while (!created);

    return baseDir.absolutePath() + "/" + result;
}

QString UserAppsSettings::getFileStorageDir() const {
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + FILE_STORAGE_DIR, QDir::homePath()+"/.UGENE_files").toString();
}

void UserAppsSettings::setFileStorageDir(const QString &newPath) {
    AppContext::getSettings()->setValue(SETTINGS_ROOT + FILE_STORAGE_DIR, newPath);
}

}//namespace
