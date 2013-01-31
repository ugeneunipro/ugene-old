/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "RemoteMachineTasks.h"

#include "PingTask.h"
#include "SynchHttp.h"

#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QFile>

#include <U2Core/NetworkConfiguration.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Log.h>

#include "SerializeUtils.h"

#include <cassert>
#include <memory>

namespace U2 {

/*******************************************
* RetrieveRemoteMachineInfoTask
*******************************************/

RetrieveRemoteMachineInfoTask::RetrieveRemoteMachineInfoTask( RemoteMachineSettingsPtr s ) 
: Task( tr( "Retrieve remote machine info task" ), TaskFlags_FOSCOE ), pingTask(NULL), pingOK(false), machine( NULL ), settings(s)
{
    setVerboseLogMode(true);
    setErrorNotificationSuppression(true);

}

RetrieveRemoteMachineInfoTask::~RetrieveRemoteMachineInfoTask() {
    delete machine;
    machine = NULL;
}

void RetrieveRemoteMachineInfoTask::prepare() {
    rsLog.details(tr("Retrieving remomote machine info..." ));

    ProtocolInfo* pi = AppContext::getProtocolInfoRegistry()->getProtocolInfo( settings->getProtocolId() );
    machine = pi->getRemoteMachineFactory()->createInstance(settings);
    if( NULL == machine ) {
        setError( tr( "Cannot create remote machine from remote machine settings: %1" ).arg( settings->getName() ) );
        return;
    }

    pingTask = new PingTask(machine);
    addSubTask( pingTask );
}


void RetrieveRemoteMachineInfoTask::run() {
    if( hasError() || isCanceled() ) {
        return;
    }
    assert( NULL != machine );
    if( isCanceled() ) { 
        return; 
    }
    hostname = machine->getServerName(stateInfo);
}

Task::ReportResult RetrieveRemoteMachineInfoTask::report() {

    if( pingTask->isCanceled() ) {
        pingTask->setError( tr( "Ping task is canceled by user" ) );
    }
    if( pingTask->hasError() ) {
        setError( tr( "Ping task finished with error: " ) + pingTask->getError() );
        pingOK = false;
        return ReportResult_Finished;
    }

    pingOK = true;
    if( isCanceled() ) {
        setError( tr( "Task is canceled by user" ) );
        return ReportResult_Finished;
    }

    return ReportResult_Finished;
}

QStringList RetrieveRemoteMachineInfoTask::getServicesList() const {
    return services;
}

QString RetrieveRemoteMachineInfoTask::getHostName() const {
    return hostname;
}

bool RetrieveRemoteMachineInfoTask::isPingOk() const {
    return pingOK;
}



/*******************************************
* RetrievePublicMachinesTask
*******************************************/

const QString RetrievePublicMachinesTask::PUBLIC_MACHINES_KEEPER_SERVER = "http://ugene.unipro.ru";
const QString RetrievePublicMachinesTask::PUBLIC_MACHINES_KEEPER_PAGE   = "/public_machines.html";
const QString RetrievePublicMachinesTask::PUBLIC_MACHINES_STR_SEPARATOR = "<br>";

RetrievePublicMachinesTask::RetrievePublicMachinesTask() : Task( "Retrieve public remote machines", TaskFlag_None ) {
    setVerboseLogMode(true);
    setErrorNotificationSuppression(true);
}


RetrievePublicMachinesTask::~RetrievePublicMachinesTask() {

}

void RetrievePublicMachinesTask::run() {
    rsLog.details(tr("Retrieving public machines..."));

    SyncHTTP http( QUrl( PUBLIC_MACHINES_KEEPER_SERVER ).host() );
    NetworkConfiguration * nc = AppContext::getAppSettings()->getNetworkConfiguration();
    assert( NULL != nc );
    bool proxyUsed = nc->isProxyUsed( QNetworkProxy::HttpProxy );
    bool srvIsException = nc->getExceptionsList().contains( QUrl( PUBLIC_MACHINES_KEEPER_SERVER ).host() );

    if( proxyUsed && !srvIsException ) {
        http.setProxy( nc->getProxy( QNetworkProxy::HttpProxy ) );
    }
    processEncodedMachines( http.syncGet( PUBLIC_MACHINES_KEEPER_PAGE ) );

    if (hasError()) {
        rsLog.error(tr("Failed to retrieve public machines, error: %1").arg(getError()));
    } else {
        rsLog.info(tr("Found %1 public machines").arg(publicMachines.size()));
    }
}

void RetrievePublicMachinesTask::processEncodedMachines( const QString & encodedMachinesStr ) {
    QStringList encodedMachines = encodedMachinesStr.split( PUBLIC_MACHINES_STR_SEPARATOR, QString::SkipEmptyParts  );
    foreach( const QString & encodedMachine, encodedMachines ) {
        RemoteMachineSettingsPtr settings = SerializeUtils::deserializeRemoteMachineSettings( encodedMachine.trimmed() );
        if (settings == NULL) {
            setError( tr( "Illegal server response" ) );
            break;
        }
        publicMachines << settings;
    }
}

QList< RemoteMachineSettingsPtr > RetrievePublicMachinesTask::getPublicMachines() const {
    return publicMachines;
}

QList< RemoteMachineSettingsPtr > RetrievePublicMachinesTask::takePublicMachines() {
    QList< RemoteMachineSettingsPtr > res = publicMachines;
    publicMachines.clear();
    return res;
}

/*******************************************
* SaveRemoteMachineSettings
*******************************************/
SaveRemoteMachineSettings::SaveRemoteMachineSettings(const RemoteMachineSettingsPtr& machineSettings, const QString& file)
: Task(tr("Save remote machine settings task"), TaskFlag_None), filename(file) {
    if(filename.isEmpty()) {
        setError(tr("Output file not set"));
        return;
    }
    if( machineSettings == NULL ) {
        setError(tr("Nothing to write: empty remote machine settings"));
        return;
    }
    data = SerializeUtils::serializeRemoteMachineSettings(machineSettings).toAscii();
}

void SaveRemoteMachineSettings::run() {
    if(hasError() || isCanceled()) {
        return;
    }
    QFile out(filename);
    if( !out.open(QIODevice::WriteOnly) ) {
        setError(tr("Cannot open %1 file").arg(filename));
        return;
    }
    out.write(data);
    out.close();
}

} // U2
