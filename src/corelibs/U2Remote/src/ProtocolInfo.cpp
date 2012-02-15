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


#include "ProtocolInfo.h"

namespace U2 {

/***********************************************
* ProtocolInfo
***********************************************/

    ProtocolInfo::ProtocolInfo( const QString &id, ProtocolUI *protocolUI,
        RemoteMachineFactory *remoteMachineFactory )
: server(NULL), protocolUI(protocolUI), remoteMachineFactory(remoteMachineFactory), scanner(NULL), id(id) {
}

RemoteTaskServer * ProtocolInfo::getRemoteTaskServer() const {
    return server;
}

ProtocolUI * ProtocolInfo::getProtocolUI() const {
    return protocolUI;
}

RemoteMachineFactory * ProtocolInfo::getRemoteMachineFactory() const {
    return remoteMachineFactory;
}

RemoteMachineScanner * ProtocolInfo::getRemoteMachineScanner() const {
    return scanner;
}

QString ProtocolInfo::getId() const {
    return id;
}

/***********************************************
 * ProtocolInfoRegistry
 ***********************************************/

void ProtocolInfoRegistry::registerProtocolInfo( ProtocolInfo *info ) {
    protocolInfos.insert( info->getId(), info );
}

bool ProtocolInfoRegistry::unregisterProtocolInfo( const QString &id ) {
    if( !protocolInfos.contains( id ) ) {
        return false;
    }
    protocolInfos.remove( id );
    return true;
}

bool ProtocolInfoRegistry::isProtocolInfoRegistered( const QString &id ) const {
    return protocolInfos.contains( id );
}

ProtocolInfo *ProtocolInfoRegistry::getProtocolInfo( const QString &id ) const {
    if( !protocolInfos.contains( id ) ) {
        return NULL;
    }
    return protocolInfos.value( id );
}

QList< ProtocolInfo *> ProtocolInfoRegistry::getProtocolInfos() const {
    return protocolInfos.values();
}

} // U2
