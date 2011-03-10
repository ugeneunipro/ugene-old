#ifndef _SEND_REPORT_DIALOG_H_
#define _SEND_REPORT_DIALOG_H_

#include <QtNetwork/QHttp>
#include <QtCore/QEventLoop>
#include <QtCore/QBuffer>
#include <QtCore/QUrl>

#include <QtCore/QTime>
#include <QtCore/QDate>

#include "../_tmp/ui/ui_SendReportDialog.h"




class SyncHTTP : public QHttp {
    Q_OBJECT
public:
    SyncHTTP(const QString& hostName, quint16 port=80, QObject* parent=0);
    QString syncGet(const QString& path);
protected slots:
    virtual void finished(int idx, bool err);

private:
    int requestID;
    QEventLoop loop;
};


class SendReportDialog:public QDialog, public Ui_Dialog{
    Q_OBJECT
public:
    SendReportDialog(const QString &report, QDialog *d = NULL);
    void parse(const QString &report);

private slots:
    void sl_onOKclicked();
    void sl_onMaximumMessageSizeReached();

private:
    QString getOSVersion();
    QString htmlReport;
};



#endif
