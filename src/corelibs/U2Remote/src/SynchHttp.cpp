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

#include "SynchHttp.h"
#include <U2Core/U2SafePoints.h>
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QTimer>

namespace U2 {

SyncHTTP::SyncHTTP(U2OpStatus &os, QObject* parent)
: QNetworkAccessManager(parent)
,loop(NULL)
,errString(""),
os(os)
{
    connect(this,SIGNAL(finished(QNetworkReply*)),SLOT(finished(QNetworkReply*)));
}

QString SyncHTTP::syncGet(const QUrl& url) {
    connect(this,SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), this, SLOT(onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
    QNetworkRequest request(url);
    QNetworkReply *reply = get(request);
    SAFE_POINT(reply != NULL, "SyncHTTP::syncGet no reply is created", "");
    runTimer();
    if (loop == NULL){
        loop = new QEventLoop();
    }
    CHECK_OP(os, QString());
    loop->exec();
    err=reply->error();
    errString=reply->errorString();
    return QString(reply->readAll());
}

QString SyncHTTP::syncPost(const QUrl & url, QIODevice * data) {
    connect(this,SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), this, SLOT(onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
    QNetworkRequest request(url);
    QNetworkReply *reply = post(request, data);
    SAFE_POINT(reply != NULL, "SyncHTTP::syncGet no reply is created", "");
    runTimer();
    if (loop == NULL){
        loop = new QEventLoop();
    }
    CHECK_OP(os, QString());
    loop->exec();
    err=reply->error();
    errString=reply->errorString();
    return QString(reply->readAll());
}

void SyncHTTP::finished(QNetworkReply*) {
    SAFE_POINT(loop != NULL, "SyncHTTP::finished no event loop", );
    loop->exit();
}

void SyncHTTP::onProxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth){
    auth->setUser(proxy.user());
    auth->setPassword(proxy.password());
    disconnect(this, SLOT(onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
}

SyncHTTP::~SyncHTTP(){
    delete loop;
    loop = NULL;
}
void SyncHTTP::runTimer() {
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(sl_taskCancellingCheck()));
    timer->start(500);
}

void SyncHTTP::sl_taskCancellingCheck() {
    if (loop != NULL && os.isCanceled()) {
        loop->exit();
    }
}

}  // U2
