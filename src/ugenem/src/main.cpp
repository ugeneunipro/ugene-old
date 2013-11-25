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

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtCore/QTextStream>
#include "SendReportDialog.h"
#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#endif

namespace {
    const QString REPORT_FILE_ARG("-f");
    const int MAX_FILE_SEZE = 512000; // 500 Kb

    bool isLoadFromFile(int argc, char *argv[]) {
        if (argc != 3) {
            return false;
        }
        return (REPORT_FILE_ARG == argv[1]);
    }

    QString getFilePath(int argc, char *argv[]) {
        if (argc != 3) {
            return "";
        }
        return argv[2];
    }

    QString loadDataFromFile(const QString &filePath) {
        QFile file(filePath);
        if (!file.exists()) {
            return "";
        }
        bool opened = file.open(QIODevice::ReadOnly);
        if (!opened) {
            return "";
        }
        QByteArray data = file.read(MAX_FILE_SEZE);
        file.close();
        file.remove();
        return QString::fromUtf8(data);
    }

    QString loadData(int argc, char *argv[]) {
        if (isLoadFromFile(argc, argv)) {
            return loadDataFromFile(getFilePath(argc, argv));
        } else if (argc > 1) {
            return QString::fromUtf8(QByteArray::fromBase64(argv[1]));
        }
        return "";
    }
}

int main(int argc, char *argv[]){
    QString message = loadData(argc, argv);
#ifdef Q_OS_LINUX
    if(XOpenDisplay(NULL) == NULL) {
        QCoreApplication a(argc, argv);
        QTextStream stream(stdin);
        QTextStream cout(stdout);
        printf("UGENE crashed. Would you like to send crash report to developer team? (y/n)\n");
        QString str = stream.readLine();
        printf("\n%s", str.toUtf8().data());
        if(str == "y" || str == "Y") {
            ReportSender sender;
            sender.parse(message);
            sender.send("");
        }
    } else {
        QApplication a(argc, argv);
        SendReportDialog dlg(message);
        dlg.setWindowIcon(QIcon(":ugenem/images/crash_icon.png"));
        dlg.exec();
    }
#else
    QApplication a(argc, argv);
    SendReportDialog dlg(message);
    dlg.setWindowIcon(QIcon(":ugenem/images/crash_icon.png"));
    dlg.exec();
#endif
    return 0;

}
