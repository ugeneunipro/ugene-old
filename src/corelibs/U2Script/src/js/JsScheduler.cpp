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

#include <U2Core/MultiTask.h>
#include <U2Lang/WorkflowDebugStatus.h>

#include "JsScheduler.h"

namespace U2 {

namespace Js {

JsScheduler::JsScheduler( Workflow::Schema *scheme, const Local<Function> &callback )
    : ScriptableScheduler( scheme ), schedulerCallback( callback )
{

}

JsScheduler::~JsScheduler( ) {

}

Task * JsScheduler::tick( ) {
    Local<Value> argv;
    schedulerCallback->Call( Context::GetCurrent( )->Global( ), 0, &argv );
    QList<Task *> tickTasks;
    foreach ( ActorId actor, nextTicks ) {
        LocalWorkflow::BaseWorker *worker
            = schema->actorById( actor )->castPeer<LocalWorkflow::BaseWorker>( );
        worker->deleteBackupMessagesFromPreviousTick( );
        Task *newTask = worker->tick( );
        debugInfo->checkActorForBreakpoint( schema->actorById( actor ) );
        if ( NULL != newTask ) {
            tickTasks.append( newTask );
        }
    }
    Task *result = ( tickTasks.isEmpty( ) ) ? NULL
        : new MultiTask( "Js-driven worker tasks", tickTasks );
    if ( NULL != result ) {
        result->setMaxParallelSubtasks( tickTasks.size() );
    }
    nextTicks.clear();
    return result;
}

} // namespace Js

} // namespace U2