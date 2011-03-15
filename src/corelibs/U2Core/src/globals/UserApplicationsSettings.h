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

#ifndef _U2_USER_APPLICATIONS_SETTINGS_H_
#define _U2_USER_APPLICATIONS_SETTINGS_H_

#include <U2Core/global.h>

namespace U2 {

class U2CORE_EXPORT UserAppsSettings : public QObject {
    Q_OBJECT
public:
    QString getWebBrowserURL() const;
    void setWebBrowserURL(const QString& url);

    bool useDefaultWebBrowser() const;
    void setUseDefaultWebBrowser(bool state);

    QString getTranslationFile() const;
    void setTranslationFile(const QString& fn);

    bool openLastProjectAtStartup() const;
    void setOpenLastProjectAtStartup(bool v);

    //a style selected by user
    QString getVisualStyle() const;
    void setVisualStyle(const QString& newStyle);
    
    // specify dir for downloaded files
    QString getDownloadDirPath() const;
    void setDownloadDirPath(const QString& newPath) const;

    // recently downloaded file names
    QStringList getRecentlyDownloadedFileNames() const;
    void setRecentlyDownloadedFileNames(const QStringList& fileNames) const;

    // specify dir for downloaded files
    QString getTemporaryDirPath() const;
    void setTemporaryDirPath(const QString& newPath);

    bool enableCollectingStatistics() const;
    void setEnableCollectingStatistics(bool b);

    bool tabbedWindowLayout() const;
    void setTabbedWindowLayout(bool b);

signals:
    void si_temporaryPathChanged();
    void si_windowLayoutChanged();
};

}//namespace

#endif
