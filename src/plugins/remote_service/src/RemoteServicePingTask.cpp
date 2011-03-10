#include <U2Remote/SerializeUtils.h>
#include <U2Core/Log.h>

#include "RemoteServicePingTask.h"
#include "RemoteServiceMachine.h"

namespace U2 {

#define PING_REMOTE_SERVICE_LOG "ping-remote-server"

static Logger log(PING_REMOTE_SERVICE_LOG);


RemoteServicePingTask::RemoteServicePingTask( const QString& url )
:Task( tr( "RemoteServicePingTask" ), TaskFlags( TaskFlags_FOSCOE ) ), 
machinePath(url), machine(NULL),  machineFactory(new RemoteServiceMachineFactory()) 
{

}

void RemoteServicePingTask::prepare() {
 
    RemoteMachineSettings* machineSettings;
    
    if( machinePath.isEmpty() ) { 
        setError("Path to remote server settings file is not set");
        return;
    }

    if( !SerializeUtils::deserializeRemoteMachineSettingsFromFile( machinePath, &machineSettings ) ) {
        setError( tr("Can not parse remote server settings file %1").arg(machinePath) );
        return;
    }
    
    RemoteMachine* m = machineFactory->createInstance(machineSettings); 
    machine.reset( static_cast<RemoteServiceMachine*> (m) );
    
}

void RemoteServicePingTask::run()
{
    assert(machine.get() != NULL);
    if (isCanceled() || hasErrors()) {
        return;
    }

    machine->initSession(stateInfo);
    if (hasErrors()) {
        return;
    }

    QString hostName = machine->getServerName(stateInfo);
}

} // namespace U2


