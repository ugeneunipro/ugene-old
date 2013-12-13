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

#include "Utils.h"

#include <QtCore/QBuffer>
#include <QtCore/QUrl>
#include <QtCore/QTime>
#include <QtCore/QDate>
#include <QtCore/QEventLoop>
#include <QtCore/QProcess>
#include <QtCore/QSysInfo>
#include <QtCore/QFile>
#include <QtCore/QProcess>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtGui/QMessageBox>

#ifdef Q_OS_WIN
#include <intrin.h>
#include <windows.h>
#include <Psapi.h>
#include <Winbase.h> //for IsProcessorFeaturePresent
#endif

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
        report += "CPU Info: ";
        report += getCPUInfo() + "\n\n";

        report += "Memory Info: ";
        report += QString::number(getTotalPhysicalMemory()) + "Mb\n\n";

        report += "UGENE version: ";
#ifdef UGENE_VERSION_SUFFIX
        //Example of usage on linux: DEFINES+='UGENE_VERSION_SUFFIX=\\\"-ppa\\\"'
        report += list.takeFirst() + QString(UGENE_VERSION_SUFFIX) +"\n\n";
#else
        report += list.takeFirst() + "\n\n";
#endif

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
void ReportSender::sl_replyFinished(QNetworkReply *) {
    loop.exit();
}

SendReportDialog::SendReportDialog(const QString &report, QDialog *d): 
QDialog(d) {
    setupUi(this);
    sender.parse(report);
    errorEdit->setText(sender.getReport());
    connect(additionalInfoTextEdit,SIGNAL(textChanged()), 
SLOT(sl_onMaximumMessageSizeReached()));
    connect(sendButton, SIGNAL(clicked()), SLOT(sl_onClicked()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(sl_onCancelClicked()));

    QFile file(getUgeneName());
    if (!file.exists()) {
        checkBox->hide();
        checkBox->setChecked(false);
    }
}

void SendReportDialog::sl_onCancelClicked() {
    if (checkBox->isChecked()) {
        openUgene();
    }
    this->reject();
}

void SendReportDialog::sl_onMaximumMessageSizeReached() {
    if(additionalInfoTextEdit->toPlainText().length() > 500 ) {
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

QString SendReportDialog::getCommandForRunUgene() const {
    QString command = getUgeneName();
#if defined Q_OS_LINUX || defined Q_OS_MAC
    command.prepend("./");
#endif
    return command;
}

QString SendReportDialog::getUgeneName() const {
    QString name;
    bool isWin = false;
#ifdef Q_OS_LINUX
    name = "ugene";
#endif
#ifdef Q_OS_WIN32
    isWin = true;
    name = "ugene.exe";
#endif
#ifdef Q_OS_MAC
    name = "ugeneui";
#endif
#ifdef QT_DEBUG
    if (isWin) {
        name = "ugened.exe";
    }else{
        name.append("d");
    }
#endif
    return name;
}

QStringList SendReportDialog::getParameters() const {
    QStringList parameters;
#ifdef Q_OS_LINUX
    parameters << "-ui";
#endif
    if (Utils::hasDatabaseUrl()) {
        parameters << Utils::SESSION_DB_UGENE_ARG + "\"" + Utils::getDatabaseUrl() + "\"";
    }
    return parameters;
}

void SendReportDialog::openUgene() const {
    QString command = getCommandForRunUgene();
    QStringList parameters = getParameters();
    QProcess::startDetached(command, parameters);
}

void SendReportDialog::sl_onOkClicked() {
    sendButton->setEnabled(false);
    cancelButton->setEnabled(false);
    checkBox->setEnabled(false);
    QString htmlReport = "";
    if(!emailLineEdit->text().isEmpty()) {
        htmlReport += "\nUser email: ";
        htmlReport += emailLineEdit->text() + "\n";
    }

    if(!additionalInfoTextEdit->toPlainText().isEmpty()) {
        htmlReport += "\nAdditional info: \n";
        htmlReport += additionalInfoTextEdit->toPlainText() + "\n";
    }

    if (checkBox->isChecked()) {
        openUgene();
    }

    if(sender.send(htmlReport)) {
        accept();
    }
}


QString ReportSender::getOSVersion() {
    QString result;
#if defined(Q_OS_WIN32)
    result = "Windows ";
    switch (QSysInfo::WindowsVersion) {
    case QSysInfo::WV_32s:
        result += "3.1 with Win 32s";
        break;
    case QSysInfo::WV_95:
        result += "95";
        break;
    case QSysInfo::WV_98:
        result += "98";
        break;
    case QSysInfo::WV_Me:
        result += "Me";
        break;
    case QSysInfo::WV_NT:
        result += "NT (operating system version 4.0)";
        break;
    case QSysInfo::WV_2000:
        result += "2000 (operating system version 5.0)";
        break;
    case QSysInfo::WV_XP:
        result += "XP (operating system version 5.1)";
        break;
    case QSysInfo::WV_2003:
        result += "Server 2003, Server 2003 R2, Home Server, XP Professional x64 Edition (operating system version 5.2)";
        break;
    case QSysInfo::WV_VISTA:
        result += "Vista, Server 2008 (operating system version 6.0)";
        break;
    case QSysInfo::WV_WINDOWS7:
        result += "7, Server 2008 R2 (operating system version 6.1)";
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    case QSysInfo::WV_WINDOWS8:
        result += "8 (operating system version 6.2)";
        break;
#endif
    default:
        result += "unknown";
        break;
    }

#elif defined(Q_OS_LINUX)
    result = "Linux";
#elif defined(Q_OS_MAC)
    result = "Mac ";
    switch (QSysInfo::MacintoshVersion) {
    case QSysInfo::MV_9:
        result += "Mac OS 9 (unsupported)";
        break;
    case QSysInfo::MV_10_0:
        result += "OS X 10.0 (unsupported)";
        break;
    case QSysInfo::MV_10_1:
        result += "OS X 10.1 (unsupported)";
        break;
    case QSysInfo::MV_10_2:
        result += "OS X 10.2 (unsupported)";
        break;
    case QSysInfo::MV_10_3:
        result += "OS X 10.3";
        break;
    case QSysInfo::MV_10_4:
        result += "OS X 10.4";
        break;
    case QSysInfo::MV_10_5:
        result += "OS X 10.5";
        break;
    case QSysInfo::MV_10_6:
        result += "OS X 10.6";
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
    case QSysInfo::MV_10_7:
        result += "OS X 10.7";
        break;
    case QSysInfo::MV_10_8:
        result += "OS X 10.8";
        break;
#endif
    default:
        result += "unknown";
        break;
    }
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

int ReportSender::getTotalPhysicalMemory() {
    int totalPhysicalMemory = 0;

#if defined(Q_OS_WIN32)
    MEMORYSTATUSEX memory_status;
    ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
    memory_status.dwLength = sizeof(memory_status);
    if (GlobalMemoryStatusEx(&memory_status)) {
        totalPhysicalMemory = memory_status.ullTotalPhys / (1024 * 1024);
    }

#elif defined(Q_OS_LINUX)
    QProcess p;
    p.start("awk", QStringList() << "/MemTotal/ {print $2}" << "/proc/meminfo");
    p.waitForFinished();
    QString memory = p.readAllStandardOutput();
    p.close();
    bool ok = false;
    qlonglong output_mem = memory.toLongLong(&ok);
    if (ok) {
        totalPhysicalMemory = output_mem / 1024;
    }

#elif defined(Q_OS_MAC)
// TODO
     QProcess p;
     p.start("sh", QStringList() << "-c" << "sysctl hw.memsize | awk -F ' ' '{print $2}'");
     p.waitForFinished();
     QString system_info = p.readAllStandardOutput();
     p.close();
     bool ok = false;
     qlonglong output_mem = system_info.toLongLong(&ok);
     if (ok) {
         totalPhysicalMemory = output_mem / (1024 * 1024);
     }
#endif

    return totalPhysicalMemory;
}

#ifndef Q_OS_MAC
void cpuID(unsigned i, unsigned regs[4]) {
#ifdef _WIN32
  __cpuid((int *)regs, (int)i);

#else
  asm volatile
    ("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
     : "a" (i), "c" (0));
  // ECX is set to zero for CPUID function 4
#endif
}
#endif

QString ReportSender::getCPUInfo() {
    QString result;
#ifndef Q_OS_MAC
    unsigned regs[4];

    // Get vendor
    char vendor[12];
    cpuID(0, regs);
    ((unsigned *)vendor)[0] = regs[1]; // EBX
    ((unsigned *)vendor)[1] = regs[3]; // EDX
    ((unsigned *)vendor)[2] = regs[2]; // ECX
    QString cpuVendor=QString(vendor);
    result+= "\n  Vendor :"+ cpuVendor;

    // Get CPU features
    cpuID(1, regs);
    unsigned cpuFeatures = regs[3]; // EDX

    // Logical core count per CPU
    cpuID(1, regs);
    unsigned logical = (regs[1] >> 16) & 0xff; // EBX[23:16]

    result+= "\n  logical cpus: " + QString::number(logical);
    unsigned cores = logical;

    if (cpuVendor == "GenuineIntel") {
      // Get DCP cache info
      cpuID(4, regs);
      cores = ((regs[0] >> 26) & 0x3f) + 1; // EAX[31:26] + 1

    } else if (cpuVendor == "AuthenticAMD") {
      // Get NC: Number of CPU cores - 1
      cpuID(0x80000008, regs);
      cores = ((unsigned)(regs[2] & 0xff)) + 1; // ECX[7:0] + 1
    }

    result+= "\n  cpu cores: " + QString::number(cores);

    // Detect hyper-threads..
    bool hyperThreads = cpuFeatures & (1 << 28) && cores < logical;

    result+= "\n  hyper-threads: " + QString(hyperThreads ? "true" : "false");
#else
    result="unknown";
#endif
    return result;
}
