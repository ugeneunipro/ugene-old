/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "LastReadyScheduler.h"

namespace U2 {

namespace LocalWorkflow {

LastReadyScheduler::LastReadyScheduler(Schema *sh)
: schema(sh), lastWorker(NULL), lastTask(NULL)
{
    
}

LastReadyScheduler::~LastReadyScheduler() {

}

void LastReadyScheduler::init() {
    foreach(Actor* a, schema->getProcesses()) {
        a->castPeer<BaseWorker>()->init();
    }

    topologicSortedGraph = schema->getActorBindingsGraph()->getTopologicalSortedGraph(schema->getProcesses());
}

bool LastReadyScheduler::isReady() {
    foreach(Actor* a, schema->getProcesses()) {
        if (a->castPeer<BaseWorker>()->isReady()) {
            return true;
        }
    }
    return false;
}

Task *LastReadyScheduler::tick() {
    for (int vertexLabel=0; vertexLabel<topologicSortedGraph.size(); vertexLabel++) {
        foreach (Actor *a, topologicSortedGraph.value(vertexLabel)) {
            if (a->castPeer<BaseWorker>()->isReady()) {
                lastWorker = a->castPeer<BaseWorker>();
                return lastTask = lastWorker->tick();
            }
        }
    }
    assert(0);
    return NULL;
}

bool LastReadyScheduler::isDone() {
    foreach(Actor* a, schema->getProcesses()) {
        if (!a->castPeer<BaseWorker>()->isDone()) {
            return false;
        }
    }
    return true;
}

void LastReadyScheduler::cleanup() {
    foreach(Actor* a, schema->getProcesses()) {
        a->castPeer<BaseWorker>()->cleanup();
    }
}

WorkerState LastReadyScheduler::getWorkerState(ActorId id) {
    Actor* a = schema->actorById(id);
    assert(a);
    BaseWorker* w = a->castPeer<BaseWorker>();
    if (lastWorker == w) {
        Task* t = lastTask;
        if (w->isDone() && t && t->isFinished()) {
            return WorkerDone;
        }
        return WorkerRunning;
    }
    if (w->isDone()) {
        return WorkerDone;
    } else if (w->isReady()) {
        return WorkerReady;
    }
    return WorkerWaiting;
}

} // LocalWorkflow

} // U2
