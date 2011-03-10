
#include <memory>
#include <cstdio>

#include <U2Core/Log.h>
#include <U2Core/Counter.h>
#include <U2Core/AppContext.h>

#include <U2Remote/RemoteMachine.h>


#include "PingTask.h"

namespace U2 {

 
/************************************
* PingTask
***********************************/
 
PingTask::PingTask(RemoteMachine* m)
 : Task( tr( "PingTask" ), TaskFlag_None), machine(m) 
{
    assert(machine!=NULL);
    GCOUNTER( cvar, tvar, "PingTask" );
}

void PingTask::run() {
    rsLog.details(tr("Starting remote service ping task, task-id: %1" ).arg( QString::number( getTaskId() ) ) );
    machine->ping(stateInfo);
    rsLog.details(tr("Remote service ping task finished, task-id: %1").arg( QString::number( getTaskId() ) ) );
}




} // U2
