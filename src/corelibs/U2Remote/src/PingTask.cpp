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
