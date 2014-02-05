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

#ifndef __SYNC_HTTP_H__
#define __SYNC_HTTP_H__

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QAuthenticator>
#include <QtCore/QEventLoop>

#include <U2Core/global.h>

namespace U2 {

class U2REMOTE_EXPORT SyncHTTP : public QNetworkAccessManager {
    Q_OBJECT
public:
    SyncHTTP(QObject* parent=0);
    ~SyncHTTP();
    QString syncGet(const QUrl& url);
    QString syncPost(const QUrl & url, QIODevice * data);
    QNetworkReply::NetworkError error() {return err;}
    QString errorString() {return errString;}
protected slots:
    virtual void finished(QNetworkReply*);
    virtual void onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*);

private:
    QEventLoop* loop;
    QNetworkReply::NetworkError err;
    QString errString;
};

} // U2

#endif
