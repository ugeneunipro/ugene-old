#include "CheckUpdatesTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/Version.h>
#include <U2Core/AppSettings.h>
#include <U2Core/NetworkConfiguration.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/GUIUtils.h>
#include <U2Remote/SynchHttp.h>

#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

namespace U2 {

CheckUpdatesTask::CheckUpdatesTask() 
:Task(tr("Check for updates"), TaskFlag_None)
{
    setVerboseLogMode(true);
}

#define SITE_URL  QString("ugene.unipro.ru")
#define PAGE_NAME QString("/current_version.html")

void CheckUpdatesTask::run() {
    stateInfo.setStateDesc(tr("Connecting to updates server"));
    NetworkConfiguration* nc = AppContext::getAppSettings()->getNetworkConfiguration();
    bool isProxy = nc->isProxyUsed(QNetworkProxy::HttpProxy);
    bool isException = nc->getExceptionsList().contains(SITE_URL);
    SyncHTTP http(SITE_URL);
    if (isProxy && !isException) {
        http.setProxy(nc->getProxy(QNetworkProxy::HttpProxy));
    }
    siteVersion = http.syncGet(PAGE_NAME);
    stateInfo.setStateDesc(QString());

    if (http.error() != QHttp::NoError) {
        stateInfo.setError(  tr("Connection error: %1").arg(http.errorString()) );
        return;
    }
}

Task::ReportResult CheckUpdatesTask::report() {
    if (hasErrors()) {
        return ReportResult_Finished;
    }
    Version v = Version::ugeneVersion();
    QString thisVersion = v.text;
    QString message = "<table><tr><td>"+tr("Your version:") + "</td><td><b>"+thisVersion+"</b></td></tr>";
    message+="<tr><td>" + tr("Latest version:") + "</td><td><b>"+siteVersion+"</b></td></tr></table>";
    bool needUpdate = thisVersion != siteVersion;
    if (!needUpdate) {
        message += "<p>" + tr("You have the latest  version");
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
    return ReportResult_Finished;    
}


} //namespace
