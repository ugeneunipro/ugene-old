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

#ifndef _SEND_REPORT_DIALOG_H_
#define _SEND_REPORT_DIALOG_H_

#include <QtNetwork/QHttp>
#include <QtCore/QEventLoop>
#include <QtCore/QBuffer>
#include <QtCore/QUrl>
#include <QtCore/QByteArray>

#include <QtCore/QTime>
#include <QtCore/QDate>

#include "../_tmp/ui/ui_SendReportDialog.h"




class SyncHTTP : public QHttp {
    Q_OBJECT
public:
    SyncHTTP(const QString& hostName, quint16 port=80, QObject* parent=0);
    QString syncGet(const QString& path);
    QString syncPost(const QString& path, const QByteArray &data);
    protected slots:
        virtual void finished(int idx, bool err);

private:
    int requestID;
    QEventLoop loop;
};

class ReportSender {
public:
    ReportSender(): report("") {}
    void parse(const QString &str);
    bool send(const QString &additionalInfo);
    QString getOSVersion();
    QString getReport() const {return report;}

private:
    QString report;
};

class SendReportDialog:public QDialog, public Ui_Dialog{
    Q_OBJECT
public:
    SendReportDialog(const QString &report, QDialog *d = NULL);

private slots:
        void sl_onOKclicked();
        void sl_onMaximumMessageSizeReached();

public:
    ReportSender sender;
};



#endif
