
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
