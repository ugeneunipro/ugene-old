#include "UserApplicationsSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>

#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtGui/QDesktopServices>
#include <QtGui/QApplication>
#include <QtGui/QStyle>
#include <QtGui/QStyleFactory>

namespace U2 {
    
#define SETTINGS_ROOT   QString("/user_apps/")
#define WEB_BROWSER     QString("web_browser")
#define USE_DEFAULT_WEB_BROWSER     QString("use_default_web_browser")
#define TRANSLATION     QString("translation_file")
#define LAST_PROJECT_FLAG QString("open_last_project")
#define VISUAL_STYLE    QString("style")
#define DOWNLOAD_DIR    QString("download_file")
#define RECENTLY_DOWNLOADED QString("recently_downloaded")
#define TEMPORARY_DIR QString("temporary_dir")
#define COLLECTING_STATISTICS QString("collecting_statistics")
#define WINDOW_LAYOUT  QString("tabbed_windows")

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

QString UserAppsSettings::getDownloadDirPath() const
{
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + DOWNLOAD_DIR, QDir::homePath()+"/.UGENE_downloaded").toString();
}

void UserAppsSettings::setDownloadDirPath(const QString& newPath) const
{
    AppContext::getSettings()->setValue(SETTINGS_ROOT + DOWNLOAD_DIR, newPath);
}

QStringList UserAppsSettings::getRecentlyDownloadedFileNames() const
{
    QStringList empty;
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + RECENTLY_DOWNLOADED, empty).toStringList();
}

void UserAppsSettings::setRecentlyDownloadedFileNames(const QStringList& fileNames) const
{
    AppContext::getSettings()->setValue(SETTINGS_ROOT + RECENTLY_DOWNLOADED, fileNames);
}

QString UserAppsSettings::getTemporaryDirPath() const
{
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + TEMPORARY_DIR, QDesktopServices::storageLocation(QDesktopServices::TempLocation)).toString();
}

void UserAppsSettings::setTemporaryDirPath(const QString& newPath)
{
    AppContext::getSettings()->setValue(SETTINGS_ROOT + TEMPORARY_DIR, newPath);
    emit si_temporaryPathChanged();
}

bool UserAppsSettings::enableCollectingStatistics() const {
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + COLLECTING_STATISTICS).toBool();
}

void UserAppsSettings::setEnableCollectingStatistics(bool b) {
    AppContext::getSettings()->setValue(SETTINGS_ROOT + COLLECTING_STATISTICS, b);
}

bool UserAppsSettings::tabbedWindowLayout() const
{
    return AppContext::getSettings()->getValue(SETTINGS_ROOT + WINDOW_LAYOUT).toBool();
}

void UserAppsSettings::setTabbedWindowLayout(bool b)
{
    AppContext::getSettings()->setValue(SETTINGS_ROOT + WINDOW_LAYOUT, b);
    emit si_windowLayoutChanged();
}

}//namespace
