#ifndef _U2_REMOTE_MACHINE_MONITOR_DIALOG_CTRL_H_
#define _U2_REMOTE_MACHINE_MONITOR_DIALOG_CTRL_H_

#include <U2Remote/RemoteMachine.h>
#include <U2Remote/RemoteMachineMonitor.h>

namespace U2 {

    class U2REMOTE_EXPORT RemoteMachineMonitorDialogController {

    public:
        static RemoteMachineSettings* selectRemoteMachine(RemoteMachineMonitor* monitor, bool runTaskMode = false );
    };

} // U2

#endif
