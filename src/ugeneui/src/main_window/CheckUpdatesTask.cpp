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

#include "CheckUpdatesTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Settings.h>
#include <U2Core/NetworkConfiguration.h>
#include <U2Core/U2SafePoints.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/GUIUtils.h>
#include <U2Remote/SynchHttp.h>

#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QMainWindow>
#include <QtNetwork/QNetworkReply>

namespace U2 {


CheckUpdatesTask::CheckUpdatesTask(bool startUp) 
:Task(tr("Check for updates"), TaskFlag_None)
{
    runOnStartup = startUp;
    setVerboseLogMode(true);
    startError = false;
}

#define SITE_URL  QString("ugene.unipro.ru")
#define PAGE_NAME QString("/current_version.html")

void CheckUpdatesTask::run() {
    stateInfo.setDescription(tr("Connecting to updates server"));
    NetworkConfiguration* nc = AppContext::getAppSettings()->getNetworkConfiguration();
    SAFE_POINT(nc != NULL, "Network configuration is null", );

    bool isProxy = nc->isProxyUsed(QNetworkProxy::HttpProxy);
    bool isException = nc->getExceptionsList().contains(SITE_URL);
    SyncHTTP http;
    if (isProxy && !isException) {
        http.setProxy(nc->getProxy(QNetworkProxy::HttpProxy));
    }
    QString siteVersionText = http.syncGet(QUrl("http://"+SITE_URL + PAGE_NAME));
    if (siteVersionText.isEmpty()){
        if(!runOnStartup){
            stateInfo.setError(  tr("Cannot load the current version."));
        }else{
            uiLog.error(tr("Cannot load the current version."));
            startError = true;
        }
    }
    siteVersion = Version::parseVersion(siteVersionText);
    stateInfo.setDescription(QString());

    if (http.error() != QNetworkReply::NoError) {
        if(!runOnStartup){
            stateInfo.setError(  tr("Connection error while checking for updates: %1").arg(http.errorString()) );
        }else{
            uiLog.error(tr("Connection error while checking for updates: %1").arg(http.errorString()));
            startError = true;
        }
        return;
    }
}

Task::ReportResult CheckUpdatesTask::report() {
    if (hasError() || startError) {
        return ReportResult_Finished;
    }

    Version thisVersion = Version::appVersion();

    if(runOnStartup) {
        if (siteVersion > thisVersion) {
            QString message = tr("Newer version available. You can download it from our site.");
            QMessageBox box(QMessageBox::Information, tr("Version information"), message, QMessageBox::NoButton, 
                AppContext::getMainWindow()->getQMainWindow());
            box.addButton(QMessageBox::Cancel);
            QPushButton *siteButton = box.addButton(tr("Visit web site"), QMessageBox::ActionRole);
            QPushButton *dontAsk = box.addButton(tr("Don't ask again"), QMessageBox::ActionRole);

            box.exec();

            if (box.clickedButton() == siteButton) {
                GUIUtils::runWebBrowser("http://ugene.unipro.ru/download.html");
            } else if(box.clickedButton() == dontAsk) {
                AppContext::getSettings()->setValue(ASK_VESRION_SETTING, false);
            }
        }
    } else {    
        QString message = "<table><tr><td>"+tr("Your version:") + "</td><td><b>" + thisVersion.text + "</b></td></tr>";
        message += "<tr><td>" + tr("Latest version:") + "</td><td><b>" + siteVersion.text + "</b></td></tr></table>";
        bool needUpdate = thisVersion != siteVersion;
        if (!needUpdate) {
            message += "<p>" + tr("You have the latest version");
        }
        
        QWidget *p = (QWidget*)(AppContext::getMainWindow()->getQMainWindow());
        QMessageBox box(QMessageBox::Information, tr("Version information"), message, QMessageBox::NoButton, p);
        box.addButton(QMessageBox::Ok);
        QAbstractButton* updateButton = NULL;
        
        if (needUpdate) {
            updateButton = box.addButton(tr("Visit web site"), QMessageBox::ActionRole);
        }
        box.exec();

        if (box.clickedButton() == updateButton) {
            GUIUtils::runWebBrowser("http://ugene.unipro.ru/download.html");
        }
    }
    return ReportResult_Finished;    
}


} //namespace
