/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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


#ifndef _U2_PROTOCOL_INFO_H_
#define _U2_PROTOCOL_INFO_H_

#include <U2Core/global.h>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>

namespace U2 {

class RemoteTaskServer;
class ProtocolUI;
class RemoteMachineFactory;
class RemoteMachineScanner;

class U2REMOTE_EXPORT ProtocolInfo {
public:
    ProtocolInfo( const QString &id, ProtocolUI *protocolUI,
        RemoteMachineFactory *remoteMachineFactory);
    
    ProtocolUI *getProtocolUI() const;
    RemoteMachineFactory *getRemoteMachineFactory() const;
    // TODO: do we need version?
    QString getId() const;
private:
    RemoteMachineScanner *getRemoteMachineScanner() const;
    RemoteTaskServer *getRemoteTaskServer() const;
private:
    RemoteTaskServer *server;
    ProtocolUI *protocolUI;
    RemoteMachineFactory *remoteMachineFactory;
    RemoteMachineScanner *scanner;
    
    QString id;

}; // ProtocolInfo

class U2REMOTE_EXPORT ProtocolInfoRegistry {
public:
    void registerProtocolInfo( ProtocolInfo *info );
    bool unregisterProtocolInfo( const QString &id );
    bool isProtocolInfoRegistered( const QString &id ) const;
    ProtocolInfo *getProtocolInfo( const QString &id ) const;
    QList< ProtocolInfo * > getProtocolInfos() const;

private:
    QMap< QString, ProtocolInfo * > protocolInfos;

}; // ProtcolInfoRegistry

} // U2

#endif // _U2_PROTOCOL_INFO_H_
