/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _WORKFLOW_LAST_READY_SCHEDULER_H_
#define _WORKFLOW_LAST_READY_SCHEDULER_H_

#include <U2Lang/LocalDomain.h>

namespace U2 {

namespace LocalWorkflow {

/**
 * more smart scheduler than a SimplestSequentialScheduler
 * seeks the nearest to the end of the workflow worker and ticks it
 * uses workflow's actors bindings graph
 */
class U2LANG_EXPORT LastReadyScheduler : public Scheduler {
public:
    LastReadyScheduler(Schema *sh);
    virtual ~LastReadyScheduler();

    // reimplemented from Worker
    virtual void init();
    virtual bool isReady();
    virtual Task *tick();
    virtual bool isDone();
    virtual void cleanup();

    virtual WorkerState getWorkerState(ActorId);

private:
    Schema *schema;
    QMap<int, QList<Actor*> > topologicSortedGraph;
    BaseWorker *lastWorker;
    Task *lastTask;

    WorkerState getWorkerState(Actor* a);
};

} // LocalWorkflow

} // U2

#endif // _WORKFLOW_LAST_READY_SCHEDULER_H_
