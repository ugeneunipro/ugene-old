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

#include "SendReportDialog.h"
#include <QMessageBox>
#include <QFile>
#define HOST_URL "http://ugene.unipro.ru"
//#define HOST_URL "http://127.0.0.1"
#define DESTINATION_URL_KEEPER_PAGE "/crash_reports_dest.html"


SendReportDialog::SendReportDialog(const QString &report, QDialog *d): QDialog(d){
    setupUi(this);
    if(report.isEmpty()) {
        htmlReport = "Unhandled error occurred while running UGENE";
    } else {
        parse(report);
    }
    errorEdit->setText(htmlReport);
    connect(additionalInfoTextEdit,SIGNAL(textChanged()), SLOT(sl_onMaximumMessageSizeReached()));
    connect(sendButton, SIGNAL(clicked()), SLOT(sl_onOKclicked()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
}
void SendReportDialog::sl_onMaximumMessageSizeReached(){
    if(additionalInfoTextEdit->toPlainText().length() > 500 ){
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Warning"));
        msgBox.setText(tr("The \"Additional information\" message is too long."));
        msgBox.setInformativeText(tr("You can also send the description of the problem to UGENE team"
                                     "by e-mail <a href=\"mailto:ugene@unipro.ru\">ugene@unipro.ru</a>."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        additionalInfoTextEdit->undo();
    }
}
void SendReportDialog::sl_onOKclicked() {

    if(!emailLineEdit->text().isEmpty()){
        htmlReport += "\nUser email: ";
        htmlReport += emailLineEdit->text() + "\n";
    }

    if(!additionalInfoTextEdit->toPlainText().isEmpty()){
        htmlReport += "\nAdditional info: \n";
        htmlReport += additionalInfoTextEdit->toPlainText() + "\n";
    }

    SyncHTTP http(QUrl(HOST_URL).host());
    QString reportsPath = http.syncGet( DESTINATION_URL_KEEPER_PAGE );
    if( reportsPath.isEmpty() ) {
        return;
    } 
    if( QHttp::NoError != http.error() ) {
        return;
    }

    //Checking proxy again, for the new url
    SyncHTTP http2( QUrl(reportsPath).host() );
    htmlReport.replace(' ', "_");
    htmlReport.replace('\n', "|");
    htmlReport.replace('\t', "<t>");
    htmlReport.replace("#", "");
    htmlReport.replace("*", "<p>");
    htmlReport.replace("?", "-");
    htmlReport.replace("~", "%7E");
    htmlReport.replace("&", "<amp>");
    QString fullPath = reportsPath;
    fullPath += "?data=";
    fullPath += htmlReport.toUtf8();
    QString res = http2.syncGet(fullPath); //TODO: consider using POST method?
    if( QHttp::NoError != http.error() ) {
        return;
    }
    accept();
}

void SendReportDialog::parse(const QString &report) {
    htmlReport = "Exception with code ";
    
    QStringList list = report.split("|");
    if(list.size()== 6) {
        htmlReport += list.takeFirst() + " - ";
        htmlReport += list.takeFirst() + "\n\n";

        htmlReport += "Operation system: ";
        htmlReport += getOSVersion() + "\n\n";

        htmlReport += "UGENE version: ";
        htmlReport += list.takeFirst() + "\n\n";

        htmlReport += "ActiveWindow: ";
        htmlReport += list.takeFirst() + "\n\n";

        htmlReport += "TaskLog:\n";
        htmlReport += list.takeFirst() + "\n";

        htmlReport += "Task tree:\n";
        htmlReport += list.takeFirst();
    }
    QFile fp("/tmp/UGENEstacktrace.txt");
    if(fp.open(QIODevice::ReadOnly)) {
        QByteArray stacktrace = fp.readAll();
        htmlReport += "Stack trace:\n";
        htmlReport += stacktrace.data();
    }
}

QString SendReportDialog::getOSVersion() {
    QString result;
#if defined(Q_OS_WIN32)
    result = "Windows x86";
#elif defined(Q_OS_WIN64)
    result = "Windows x64";
#elif defined(Q_OS_LINUX)
    result = "Linux";
#elif defined(Q_OS_MAC)
    result = "MACOS";
#else
    result = "Unsupported OS";
#endif
    return result;
}


SyncHTTP::SyncHTTP(const QString& hostName, quint16 port, QObject* parent)
: QHttp(hostName,port,parent), requestID(-1)
{
    connect(this,SIGNAL(requestFinished(int,bool)),SLOT(finished(int,bool)));
}

QString SyncHTTP::syncGet(const QString& path) {
    QBuffer to;
    requestID = get(path, &to);
    loop.exec();
    return QString(to.data());
}

QString SyncHTTP::syncPost(const QString& path, const QByteArray& data) {
    QBuffer to;
    requestID = post(path, data, &to);
    loop.exec();
    return QString(to.data());
}


void SyncHTTP::finished(int idx, bool err) {
    loop.exit();
}
