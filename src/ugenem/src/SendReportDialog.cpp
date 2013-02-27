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

#include "SendReportDialog.h"

#include <QtCore/QBuffer>
#include <QtCore/QUrl>
#include <QtCore/QTime>
#include <QtCore/QDate>
#include <QtCore/QEventLoop>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

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

        report += "Log:\n";
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
        if(stacktrace.isEmpty()) {
            QFile err("/tmp/UGENEerror");
            if(err.open(QIODevice::ReadOnly)) {
                stacktrace = fp.readAll();
                report += stacktrace.data();
                err.close();
            }
        }
        fp.close();
    }
}

bool ReportSender::send(const QString &additionalInfo) {
    report += additionalInfo;

    QNetworkAccessManager* netManager=new QNetworkAccessManager(this);
    QNetworkProxy proxy = QNetworkProxy::applicationProxy ();
    netManager->setProxy(proxy);

    connect(netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(sl_replyFinished(QNetworkReply*)));
    //check destination availability

    QNetworkReply *reply = netManager->get(QNetworkRequest(QString(HOST_URL)+QString(DESTINATION_URL_KEEPER_PAGE)));
    loop.exec();
    QString reportsPath = QString(reply->readAll());
    if( reportsPath.isEmpty() ) {
        return false;
    }
    if( reply->error() != QNetworkReply::NoError ) {
        return false;
    }

    //prepare report
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
    //send report
    QUrl url=QUrl(fullPath);
    reply = netManager->post(QNetworkRequest(url),url.encodedQuery());
    loop.exec();
    if( reply->error() != QNetworkReply::NoError ) {
        return false;
    }
    return true;
}
void ReportSender::sl_replyFinished(QNetworkReply *){
    loop.exit();
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
    result = "Windows";
#elif defined(Q_OS_LINUX)
    result = "Linux";
#elif defined(Q_OS_MAC)
    result = "MACOS";
#else
    result = "Unsupported OS";
#endif

#if defined(UGENE_X86_64)
    result += " x64";
#else
    result += " x86";
#endif

    return result;
}
