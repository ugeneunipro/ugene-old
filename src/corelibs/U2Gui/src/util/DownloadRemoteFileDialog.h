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


class U2GUI_EXPORT DownloadRemoteFileDialog : public QDialog{
    Q_OBJECT
    bool isQueryDB;
    QString resUrl;
    QString resUrls;
    static QString defaultDB;

public slots:
    void sl_onDbChanged(int idx);
    void sl_saveFilenameButtonClicked();
    void sl_linkActivated(const QString& link);

public:
    DownloadRemoteFileDialog(QWidget *p = NULL);
    DownloadRemoteFileDialog(const QString& ids, const QString& dbId, QWidget *p = NULL);
    ~DownloadRemoteFileDialog();
    QString getDBId() const;
    QString getResourceId() const;
    QString getFullpath() const;
    virtual void accept();

private:
    void setSaveFilename();
    bool isDefaultDb(const QString& dbId);
    void setupHintText( const QString &sampleText = QString( ) );

    Ui_DownloadRemoteFileDialog *ui;
};

} // namespace

#endif //_U2_DOWNLOAD_REMOTE_FILE_DIALOG_H_
