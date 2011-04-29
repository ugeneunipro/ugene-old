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


void ReportSender::parse(const QString &htmlReport) {
    report = "Exception with code ";

    QStringList list = htmlReport.split("|");
    if(list.size()== 7) {
        report += list.takeFirst() + " - ";
        report += list.takeFirst() + "\n\n";

        report += "Operation system: ";
        report += getOSVersion() + "\n\n";

        report += "UGENE version: ";
        report += list.takeFirst() + "\n\n";

        report += "ActiveWindow: ";
        report += list.takeFirst() + "\n\n";

        report += "TaskLog:\n";
        report += list.takeFirst() + "\n";

        report += "Task tree:\n";
        report += list.takeFirst() + "\n";

#if defined (Q_OS_WIN) 
        report += list.takeLast();
#endif
    } else {
        foreach(const QString& str, list) {
            report += str + "\n";
        }
    }

    QFile fp("/tmp/UGENEstacktrace.txt");
    if(fp.open(QIODevice::ReadOnly)) {
        QByteArray stacktrace = fp.readAll();
        report += "Stack trace:\n";
        report += stacktrace.data();
    }
}

bool ReportSender::send(const QString &additionalInfo) {
    report += additionalInfo;
    SyncHTTP http(QUrl(HOST_URL).host());
    QString reportsPath = http.syncGet( DESTINATION_URL_KEEPER_PAGE );
    if( reportsPath.isEmpty() ) {
        return false;
    }
    if( QHttp::NoError != http.error() ) {
        return false;
    }

    SyncHTTP http2( QUrl(reportsPath).host() );
    report.replace(' ', "_");
    report.replace('\n', "|");
    report.replace('\t', "<t>");
    report.replace("#", "");
    report.replace("*", "<p>");
    report.replace("?", "-");
    report.replace("~", "%7E");
    report.replace("&", "<amp>");
    QString fullPath = reportsPath;
    fullPath += "?data=";
    fullPath += report.toUtf8();
    QString res = http2.syncGet(fullPath); 
    if( QHttp::NoError != http.error() ) {
        return false;
    }

    return true;
}

SendReportDialog::SendReportDialog(const QString &report, QDialog *d): 
QDialog(d){
    setupUi(this);
    sender.parse(report);
    errorEdit->setText(sender.getReport());
    connect(additionalInfoTextEdit,SIGNAL(textChanged()), 
SLOT(sl_onMaximumMessageSizeReached()));
    connect(sendButton, SIGNAL(clicked()), SLOT(sl_onOKclicked()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
}
void SendReportDialog::sl_onMaximumMessageSizeReached(){
    if(additionalInfoTextEdit->toPlainText().length() > 500 ){
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Warning"));
        msgBox.setText(tr("The \"Additional information\" message is too long."));
        msgBox.setInformativeText(tr("You can also send the description of the problem to UGENE team"
                                     "by e-mail <ahref=\"mailto:ugene@unipro.ru\">ugene@unipro.ru</a>."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        additionalInfoTextEdit->undo();
    }
}
void SendReportDialog::sl_onOKclicked() {

    QString htmlReport = "";
    if(!emailLineEdit->text().isEmpty()){
        htmlReport += "\nUser email: ";
        htmlReport += emailLineEdit->text() + "\n";
    }

    if(!additionalInfoTextEdit->toPlainText().isEmpty()){
        htmlReport += "\nAdditional info: \n";
        htmlReport += additionalInfoTextEdit->toPlainText() + "\n";
    }
    if(sender.send(htmlReport)) {
        accept();
    }
}


QString ReportSender::getOSVersion() {
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


SyncHTTP::SyncHTTP(const QString& hostName, quint16 port, QObject* 
parent)
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

QString SyncHTTP::syncPost(const QString& path, const QByteArray& data) 
{
    QBuffer to;
    requestID = post(path, data, &to);
    loop.exec();
    return QString(to.data());
}


void SyncHTTP::finished(int idx, bool err) {
    loop.exit();
}