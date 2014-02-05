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

#ifndef _U2_USER_APP_SETTINGS_GUI_CONTROLLER_H_
#define _U2_USER_APP_SETTINGS_GUI_CONTROLLER_H_

#include <ui/ui_UserApplicationsSettingsWidget.h>

#include <U2Core/NetworkConfiguration.h>
#include <U2Gui/AppSettingsGUI.h>

#include <QtCore/QUrl>

namespace U2 {


class UserApplicationsSettingsPageController : public AppSettingsGUIPageController {
    Q_OBJECT
public:
    UserApplicationsSettingsPageController(QObject* p = NULL);

    virtual AppSettingsGUIPageState* getSavedState();

    virtual void saveState(AppSettingsGUIPageState* s);

    virtual AppSettingsGUIPageWidget* createWidget(AppSettingsGUIPageState* data);
    
    QMap<QString, QString> translations;
};


class UserApplicationsSettingsPageState : public AppSettingsGUIPageState {
    Q_OBJECT
public:
    UserApplicationsSettingsPageState() : useDefaultWebBrowser (true), 
        openLastProjectFlag(false), askToSaveProject(0), enableStatistics(false), tabbedWindowLayout(false), resetSettings(false) {}

    QString webBrowserUrl;
    QString translFile;
    QString style;
    QString downloadsDirPath;
    QString temporaryDirPath;
    QString fileStorageDirPath;
    bool useDefaultWebBrowser;
    bool openLastProjectFlag;
    int askToSaveProject;
    bool enableStatistics;
    bool tabbedWindowLayout;
    bool resetSettings;
};


class UserApplicationsSettingsPageWidget: public AppSettingsGUIPageWidget, public Ui_UserApplicationsSettingsWidget {
    Q_OBJECT
public:
    UserApplicationsSettingsPageWidget(UserApplicationsSettingsPageController* ctrl);

    virtual void setState(AppSettingsGUIPageState* state);

    virtual AppSettingsGUIPageState* getState(QString& err) const;

private slots:
    void sl_wbURLClicked();
    void sl_transFileClicked();
    void sl_browseButtonClicked();
    void sl_browseTmpDirButtonClicked();
    void sl_browseFileStorageButtonClicked();
    void sl_cleanupStorage();
};

}//namespace

#endif
