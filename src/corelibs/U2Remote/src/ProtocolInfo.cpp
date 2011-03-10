
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
