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

#include "RemoteServicePingTask.h"

#include <U2Remote/SerializeUtils.h>
#include <U2Core/Log.h>

namespace U2 {

#define PING_REMOTE_SERVICE_LOG "ping-remote-server"

RemoteServicePingTask::RemoteServicePingTask( const QString& url )
:Task( tr( "RemoteServicePingTask" ), TaskFlags( TaskFlags_FOSCOE ) ),
machinePath(url), machine(NULL),  machineFactory(new RemoteServiceMachineFactory())
{

}

void RemoteServicePingTask::prepare() {

    if( machinePath.isEmpty() ) {
        setError("Path to remote server settings file is not set");
        return;
    }

    RemoteMachineSettingsPtr machineSettings = RemoteMachineSettingsPtr();
    if (QFile::exists(machinePath)) {
        machineSettings = SerializeUtils::deserializeRemoteMachineSettingsFromFile(machinePath);
    } else {
        machineSettings = SerializeUtils::deserializeRemoteMachineSettings(machinePath);
    }
    if( machineSettings == NULL ) {
        setError( tr("Can not parse remote server settings file %1").arg(machinePath) );
        return;
    }

    RemoteMachine* m = machineFactory->createInstance(machineSettings);
    machine.reset( static_cast<RemoteServiceMachine*> (m) );

}

void RemoteServicePingTask::run()
{
    SAFE_POINT(!machine.isNull(), "Invalid remote machine detected!", );
    if (isCanceled() || hasError()) {
        return;
    }

    machine->initSession(stateInfo);
    if (hasError()) {
        return;
    }

    QString hostName = machine->getServerName(stateInfo);
}

} // namespace U2


