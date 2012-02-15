/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

namespace U2 {

SyncHTTP::SyncHTTP(const QString& hostName, quint16 port, QObject* parent)
: QHttp(hostName,port,parent), requestID(-1)
{
    connect(this,SIGNAL(requestFinished(int,bool)),SLOT(finished(int,bool)));
}

QString SyncHTTP::syncGet(const QString& path) {
    assert(requestID == -1);
    QBuffer to;
    requestID = get(path, &to);
    loop.exec();
    return QString(to.data());
}

QString SyncHTTP::syncPost(const QString & path, QIODevice * data) {
    assert(requestID == -1);
    QBuffer to;
    requestID = post(path, data, &to);
    loop.exec();
    return QString(to.data());
}

void SyncHTTP::finished(int idx, bool err) {
    assert(idx = requestID);Q_UNUSED(err); Q_UNUSED(idx);
    loop.exit();
}

}  // U2
