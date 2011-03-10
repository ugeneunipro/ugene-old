#include "RemoteMachine.h"

namespace U2 {

RemoteMachine::~RemoteMachine() {
}

RemoteMachineSettings::RemoteMachineSettings( ProtocolInfo * proto, RemoteMachineType type ) : machineType(type), credentials(NULL) {
    assert( NULL != proto ); 
    protoId = proto->getId();
}

RemoteMachineSettings::~RemoteMachineSettings() {
    flushCredentials();
}

void RemoteMachineSettings::flushCredentials() {
    delete credentials;
    credentials = NULL;
}

void RemoteMachineSettings::setupCredentials( const QString& userName, const QString& passwd, bool permanent ) {
    flushCredentials();
    credentials = new UserCredentials;
    credentials->name = userName;
    credentials->passwd = passwd;
    credentials->permanent = permanent;
}

RemoteMachineFactory::~RemoteMachineFactory() {
}

} // U2
