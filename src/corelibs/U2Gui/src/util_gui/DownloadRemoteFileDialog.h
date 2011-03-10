#ifndef _U2_DOWNLOAD_REMOTE_FILE_DIALOG_H_
#define _U2_DOWNLOAD_REMOTE_FILE_DIALOG_H_

#include <U2Core/global.h>

#include <QtGui/QDialog>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QNetworkReply>
#include <QXmlSimpleReader>

class QNetworkReply;
class QNetworkAccessManager;
class Ui_DownloadRemoteFileDialog;

namespace U2 {


class U2GUI_EXPORT DownloadRemoteFileDialog : public QDialog
{
    Q_OBJECT
    bool isQueryDB;
    QString resUrl;
    static QString defaultDB;
    
public slots:
    void sl_updateHint(const QString& dbName);
    void sl_saveFilenameButtonClicked();
    
public:
    DownloadRemoteFileDialog(QWidget *p = NULL);  
    ~DownloadRemoteFileDialog();
    QString getDBName() const;
    QString getResourceId() const;
    QString getFullpath() const;
    virtual void accept();

private:
    void setSaveFilename();

    Ui_DownloadRemoteFileDialog* ui;
    
};

} // namespace

#endif //_U2_DOWNLOAD_REMOTE_FILE_DIALOG_H_
