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

#include "DistributedComputingUtil.h"

#include <AppContextImpl.h>

#include <U2Gui/MainWindow.h>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QFile>

#include <U2Core/NetworkConfiguration.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Log.h>
#include <U2Remote/SynchHttp.h>
#include <U2Remote/PingTask.h>
#include <U2Remote/SerializeUtils.h>
#include <U2Remote/RemoteWorkflowRunTask.h>
#include "RemoteMachineMonitorDialogImpl.h"

#include <QtGui/QMenu>
#include <QtGui/QCheckBox>

#include <cassert>
#include <memory>

namespace U2 {

/*******************************************
 * DistributedComputingUtil
 *******************************************/

DistributedComputingUtil::DistributedComputingUtil() {
    AppContextImpl * appContext = AppContextImpl::getApplicationContext();
    assert( NULL != appContext );
    
    pir = new ProtocolInfoRegistry();
    appContext->setProtocolInfoRegistry( pir );
    rmm = new RemoteMachineMonitor();
    appContext->setRemoteMachineMonitor( rmm );
    
    if( NULL != AppContext::getMainWindow() ) { /* if not congene */
        QAction * showRemoteMachinesMonitor = new QAction( QIcon( ":core/images/remote_machine_monitor.png" ),
                                                           tr( "Remote machines monitor..." ), this );
        showRemoteMachinesMonitor->setObjectName("Remote machines monitor");
    }
}

DistributedComputingUtil::~DistributedComputingUtil() {
    delete rmm;
    delete pir;
}

void DistributedComputingUtil::sl_showRemoteMachinesMonitor() {
    RemoteMachineMonitorDialogImpl dlg( QApplication::activeWindow(), rmm );
    int ret = dlg.exec();
    if( QDialog::Rejected == ret ) {
        return;
    }
    assert( QDialog::Accepted == ret );
}

QStringList DistributedComputingUtil::filterRemoteMachineServices( const QStringList & services ) {
    QStringList res = services;
    return res;
}


/*******************************************
* UpdateActiveTasks
*******************************************/

UpdateActiveTasks::UpdateActiveTasks(const RemoteMachineSettingsPtr& s) :
 Task("UpdateActiveTasks", TaskFlags_FOSCOE), settings(s), machine(NULL) 
 {
     rsLog.details(tr("Updating active tasks..."));
     ProtocolInfo* pi = AppContext::getProtocolInfoRegistry()->getProtocolInfo( settings->getProtocolId() );
     machine =  pi->getRemoteMachineFactory()->createInstance( settings );
     if( NULL == machine ) {
         setError( tr( "Cannot create remote machine from remote machine settings: %1" ).arg( settings->getName() ) );
     }
}

UpdateActiveTasks::~UpdateActiveTasks() {
    delete machine;
    machine = NULL;
}

void UpdateActiveTasks::run() {
    if (hasError() || isCanceled()) {
        return;
    }
    
    QList<qint64> taskIds;
    taskIds = machine->getActiveTasks(stateInfo);
    if (hasError()) {
        return;
    }

    foreach(qint64 taskId, taskIds) {
        rsLog.details(tr("Found active task: %1").arg(taskId));
        addTaskToScheduler(taskId);
    }

    rsLog.details(tr("Active tasks update finished!"));
}

void UpdateActiveTasks::addTaskToScheduler( qint64 taskid ) {
    TaskScheduler* scheduler = AppContext::getTaskScheduler();
    QList<qint64> activeIds;
    QList<Task*> topLevelTasks = scheduler->getTopLevelTasks();
    foreach(Task* task, topLevelTasks) {
        RemoteWorkflowRunTask* workflowTask = qobject_cast<RemoteWorkflowRunTask*> (task);
        if (workflowTask == NULL) {
            continue;
        } 
        activeIds.append(workflowTask->getRemoteTaskId());
    }

    if (!activeIds.contains(taskid)) {
        Task* newTask = new RemoteWorkflowRunTask(settings, taskid);
        scheduler->registerTopLevelTask(newTask);
    }
}

} // U2
