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

#include <QtCore/QSysInfo>
#include <QtCore/QDate>
#include <QtCore/QBuffer>
#include <QtCore/QProcess>
#include <QtCore/QUrl>

#include <QtGui/QMessageBox>
#include <QtGui/QApplication>

#include <QtNetwork/QNetworkReply>

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/Version.h>
#include <U2Core/Log.h>
#include <U2Core/NetworkConfiguration.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Counter.h>
#include <U2Gui/MainWindow.h>

#include <U2Remote/SynchHttp.h>

#include <U2Core/UserApplicationsSettings.h>
#include "Shtirlitz.h"

const static char * SETTINGS_NOT_FIRST_LAUNCH     = "shtirlitz/not_first_launch";
const static char * SETTINGS_PREVIOUS_REPORT_DATE = "shtirlitz/previous_report_date";
const static char * SETTINGS_COUNTERS             = "shtirlitz/counters";
const static char * SETTINGS_UGENE_UID            = "shtirlitz/uid";

const static int DAYS_BETWEEN_REPORTS = 7;

//This file stores the actual location of reports-receiver script.
const static char * DESTINATION_URL_KEEPER_SRV = "http://ugene.unipro.ru";
const static char * DESTINATION_URL_KEEPER_PAGE = "/reports_dest.html";

namespace U2 {

//////////////////////////////////////////////////////////////////////////
// Shtirlitz itself

QUuid Shtirlitz::getUniqueUgeneId() {
    static QUuid uniqueUgeneId;
    if( uniqueUgeneId.isNull() ) {
        //try to load it from settings
        QVariant uuidQvar = AppContext::getSettings()->getValue( SETTINGS_UGENE_UID );
        if( uuidQvar.isNull() ) {
            uniqueUgeneId = QUuid::createUuid();
            AppContext::getSettings()->setValue( SETTINGS_UGENE_UID, uniqueUgeneId.toString() );
        } else {
            uniqueUgeneId = QUuid( uuidQvar.toString() );
        }
    }
    return uniqueUgeneId;
}

const static char * firstTimeNotice =
"Dear UGENE user!\n"
"UGENE has the possibility to collect statistical information about its usage and report it to our team. "
"Such information helps us a lot in making our software better and more functional than it is.\n"
"\n"
"Kinds of information we collect:\n"
"1. System info: UGENE version, OS name, Qt version and so on.\n"
"2. Counters info: number of launches of certain tasks (e. g. HMM search, MUSCLE align)\n"
"\n"
"We DO NOT collect any personal data.\n"
"If you want this feature enabled, press 'Yes'. Otherwise, press 'No'."
;

//Report about system is sent on the first launch of UGENE.
//Statistical reports are sent once per DAYS_BETWEEN_REPORTS.
QList<Task*> Shtirlitz::wakeup() {
    QList<Task*> result;
    Settings * s = AppContext::getSettings();
    bool thisVersionFirstLaunch, allVersionsFirstLaunch;
    getFirstLaunchInfo(thisVersionFirstLaunch, allVersionsFirstLaunch);

    QString allVersionsKey = SETTINGS_NOT_FIRST_LAUNCH;
    QString thisVersionsKey = s->toVersionKey(SETTINGS_NOT_FIRST_LAUNCH);
    if (allVersionsFirstLaunch) {
        s->setValue(allVersionsKey, QVariant(true));
    }
    if (thisVersionFirstLaunch) {
        s->setValue(thisVersionsKey, QVariant(true));
    }
    
    // Do nothing if Shtirlitz was disabled
     if (QProcess::systemEnvironment().contains(ENV_UGENE_DEV)) {
         return result;
     }

    bool enabledByUser = AppContext::getAppSettings()->getUserAppsSettings()->isStatisticsCollectionEnabled();
    
    // Check if this version of UGENE is launched for the first time 
    // and user did not enabled stats before -> ask to enable
    // Do not ask to enable it twice for different versions!
    if( thisVersionFirstLaunch && !enabledByUser) {
        QMessageBox::StandardButton answ = QMessageBox::question(QApplication::activeWindow(), tr("Statistical reports"), tr(firstTimeNotice), QMessageBox::Yes | QMessageBox::No );
        if( QMessageBox::Yes != answ ) {
             AppContext::getAppSettings()->getUserAppsSettings()->setEnableCollectingStatistics( false );
             return result;
         }
         AppContext::getAppSettings()->getUserAppsSettings()->setEnableCollectingStatistics( true );
        coreLog.details( tr("Shtirlitz is sending the first-time report") );
        result << sendSystemReport();
        //Leave a mark that the first-time report was sent
    } 

    // Check if previous report was sent more than a week ago
    if( !allVersionsFirstLaunch && enabledByUser ) {
        QVariant prevDateQvar = AppContext::getSettings()->getValue( SETTINGS_PREVIOUS_REPORT_DATE );
        QDate prevDate = prevDateQvar.toDate();
        int daysPassed = prevDate.isValid() ? prevDate.daysTo(QDate::currentDate()) : 0;

        if( !prevDate.isValid() || daysPassed > DAYS_BETWEEN_REPORTS ) {
            coreLog.details( tr("%1 days passed passed since previous Shtirlitz's report. Shtirlitz is sending the new one.") );
            result << sendCountersReport();
            //and save the new date
            s->setValue( SETTINGS_PREVIOUS_REPORT_DATE, QDate::currentDate() );
        }
    }
    return result;
}

Task * Shtirlitz::sendCustomReport( const QString & customReport ) {
    return new ShtirlitzTask(customReport);
}

void Shtirlitz::saveGatheredInfo() {
    if( !enabled() ) {
        return;
    }
    //1. Save counters
    Settings * s = AppContext::getSettings();
    QList<GCounter*> appCounters = GCounter::allCounters();
    foreach( GCounter * ctr, appCounters ) {
        if( qobject_cast<GReportableCounter*>(ctr) ) {
            QString ctrKey = ctr->name;
            double ctrVal = ctr->scaledTotal();

            QString curKey = QString(SETTINGS_COUNTERS) + "/" + ctrKey;
            QVariant lastValQvar = s->getValue( curKey, QVariant() );
            double lastVal = lastValQvar.canConvert<double>() ? lastValQvar.toDouble() : 0.;
            double newVal = (lastVal + ctrVal > lastVal) ? lastVal + ctrVal : lastVal; //overflow detection
            s->setValue( curKey, newVal );
        }
    }
}

bool Shtirlitz::enabled() {
    // Check environment variable for developers
    if( QProcess::systemEnvironment().contains(ENV_UGENE_DEV) ) {
        return false;
    }
    return AppContext::getAppSettings()->getUserAppsSettings()->isStatisticsCollectionEnabled();
}

Task * Shtirlitz::sendCountersReport() {
    QString countersReport = formCountersReport();
    return sendCustomReport(countersReport);
}

Task * Shtirlitz::sendSystemReport() {
    QString systemReport = formSystemReport();
    return sendCustomReport(systemReport);
}

//Tries to load saved counters from settings.
//Adds loaded counters to report, sets saved values to zero
QString Shtirlitz::formCountersReport() {
    Settings * s = AppContext::getSettings();
    QString countersReport;
    countersReport += "COUNTERS REPORT:\n";
    countersReport += "ID: " + getUniqueUgeneId() + "\n";
    QStringList savedCounters = s->getAllKeys( SETTINGS_COUNTERS );
    if( savedCounters.empty() ){
        countersReport += "NO INFO\n";
    } else {
        foreach( QString savedCtrName, savedCounters ) {
            QVariant savedCtrQvar = s->getValue( QString(SETTINGS_COUNTERS) + "/" + savedCtrName );
            double savedCtrVal = savedCtrQvar.canConvert<double>() ? savedCtrQvar.toDouble() : 0.;
            countersReport += savedCtrName + " : " + QString::number(savedCtrVal) + "\n";
            s->setValue( QString(SETTINGS_COUNTERS) + "/" + savedCtrName, 0. );
        }
    }
    countersReport += "ENDOF COUNTERS REPORT.\n";
    return countersReport;
}

QString Shtirlitz::formSystemReport() {
    QString dateAndTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString ugeneVer = Version::appVersion().text;
    QString qtVersion = qVersion();
    QString osName;
    QString osVersion;
    getOsNameAndVersion( osName, osVersion );
    
    QString systemReport;
    systemReport += "SYSTEM REPORT:\n";
    systemReport += "ID: " + getUniqueUgeneId() + "\n";
    systemReport += "Date and time: " + dateAndTime + "\n";
    systemReport += "Qt Version: " + qtVersion + "\n";
    systemReport += "UGENE version: " + ugeneVer + "\n";
    systemReport += "Word size: " + QString::number(QSysInfo::WordSize) + "\n";
    systemReport += "OS name: " + osName + "\n";
    systemReport += "OS version: " + osVersion + "\n";
    systemReport += "ENDOF SYSTEM REPORT.\n";

    return systemReport;
}

void Shtirlitz::getOsNameAndVersion( QString & name, QString & version ) {
#if defined(Q_OS_WIN)
    name = "Windows";
    version = QString::number(QSysInfo::WindowsVersion);
#elif defined(Q_OS_MAC) 
    name = "Mac";
    version = QString::number(QSysInfo::MacintoshVersion);
#elif defined(Q_OS_LINUX)
    name = "Linux";
    Q_UNUSED(version);//no version is available :(
#elif defined(Q_OS_FREEBSD)
    name = "FreeBSD";
    Q_UNUSED(version);//no version is available :(
#elif defined(Q_OS_UNIX)
    name = "Unix";
    Q_UNUSED(version);//no version is available :(
#else
    name = "Other";
#endif
}

void Shtirlitz::getFirstLaunchInfo(bool& thisVersion, bool& allVersions) {
    Settings* settings = AppContext::getSettings();
    
    QString allVersionsKey = SETTINGS_NOT_FIRST_LAUNCH;
    QString thisVersionsKey = settings->toVersionKey(SETTINGS_NOT_FIRST_LAUNCH);

    QVariant launchedAllQvar = settings->getValue( allVersionsKey);
    QVariant launchedThisQvar = settings->getValue( thisVersionsKey);
    
    allVersions = (!launchedAllQvar.isValid())  || launchedAllQvar.isNull();
    thisVersion = (!launchedThisQvar.isValid()) || launchedThisQvar.isNull();
}

QString Shtirlitz::tr( const char * str ) {
    return QObject::tr(str);
}

//////////////////////////////////////////////////////////////////////////
// Shtirlitz Task

ShtirlitzTask::ShtirlitzTask( const QString & _report ) :
Task("Shtirlitz task", TaskFlag_None), report(_report) {
}

void ShtirlitzTask::run() {
    stateInfo.setDescription( tr("Connecting to remote server") );

    //Creating SyncHttp object and enabling proxy if needed.
    SyncHTTP http(this);
    NetworkConfiguration * nc = AppContext::getAppSettings()->getNetworkConfiguration();
    bool isProxy = nc->isProxyUsed( QNetworkProxy::HttpProxy );
    bool isException = nc->getExceptionsList().contains( QUrl(DESTINATION_URL_KEEPER_SRV).host() );
    
    if (isProxy && !isException) {
        http.setProxy(nc->getProxy(QNetworkProxy::HttpProxy));
    }

    QByteArray preparedReport("data=");
    preparedReport += QUrl::toPercentEncoding(report);

    // Get actual location of the reports receiver
    //FIXME: error handling
    QString reportsPath = http.syncGet( QUrl(QString(DESTINATION_URL_KEEPER_SRV) + QString(DESTINATION_URL_KEEPER_PAGE)) );
    if( reportsPath.isEmpty() ) {
        stateInfo.setError( tr("Cannot resolve destination path for statistical reports") );
        return;
    } 
    if( QNetworkReply::NoError != http.error() ) {
        stateInfo.setError( tr("Network error while resolving destination URL: ") + http.errorString() );
        return;
    }

    //Checking proxy again, for the new url
    SyncHTTP http2(this);
    isException = nc->getExceptionsList().contains( QUrl(reportsPath).host() );
    if( isProxy && !isException ) {
        http2.setProxy( nc->getProxy(QNetworkProxy::HttpProxy) );
    }
    QString fullPath = reportsPath + "?" + preparedReport;
    http2.syncGet(fullPath); //TODO: consider using POST method?
    if( QNetworkReply::NoError != http.error() ) {
        stateInfo.setError( tr("Network error while sending report: ") + http2.errorString() );
        return;
    }
}

}// U2
