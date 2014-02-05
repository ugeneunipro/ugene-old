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

#include "Task.h"

#include <U2Core/AppResources.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

static qint64 genTaskId() {
    //todo: add main-thread model check
    static qint64 id = 0;
    qint64 res = ++id;
    return res;
}

Task::Task(const QString& _name, TaskFlags f) {
    taskName    = _name;
    state       = State_New;
    tpm         = Progress_SubTasksBased;
    flags       = f;
    setVerboseOnTaskCancel(true);
    taskId      = genTaskId();
    parentTask  = NULL;
    progressWeightAsSubtask = 1;
    maxParallelSubtasks = MAX_PARALLEL_SUBTASKS_SERIAL;
    insidePrepare = false;
}

void Task::setMaxParallelSubtasks(int n) {
    SAFE_POINT(n >= 0, QString("max parallel subtasks must be >=0, value passed: %1").arg(n),);
    maxParallelSubtasks = n;
}

void Task::setTaskName(const QString& _taskName) {
    SAFE_POINT(isNew(), "Can only change name for new tasks!",);
    taskName = _taskName;
}

void Task::cancel() { 
    foreach(Task* t, subtasks) {
        if (!t->isFinished()) {
            t->cancel();
        }
    }
    stateInfo.cancelFlag = true;
}


void Task::addSubTask(Task* sub) {
    SAFE_POINT(sub != NULL, "Trying to add NULL subtask",);
    SAFE_POINT(sub->parentTask==NULL, "Task already has a parent!",);
    SAFE_POINT(state == State_New, "Parents can be assigned to tasks in NEW state only!",);
    
    sub->parentTask = this;
    subtasks.append(sub); 
    emit si_subtaskAdded(sub);
}


void Task::cleanup()    {
    assert(isFinished());
    foreach(Task* sub, getSubtasks()) {
        sub->cleanup();
    }
}

bool Task::propagateSubtaskError() {
    if (hasError()) {
        return true;
    }
    Task* badChild = getSubtaskWithErrors();
    if (badChild) {
        stateInfo.setError(stateInfo.getError() + badChild->getError());
    }
    return stateInfo.hasError();
}

Task* Task::getSubtaskWithErrors() const  {
    foreach(Task* sub, getSubtasks()) {
        if (sub->hasError()) {
            return sub;
        }
    }
    return NULL;
}

QList<Task*> Task::onSubTaskFinished(Task*){ 
    static QList<Task*> stub; 
    return stub; 
}


int Task::getNumParallelSubtasks() const {
    int nParallel = maxParallelSubtasks;
    assert(nParallel >=0 );
    if (nParallel == MAX_PARALLEL_SUBTASKS_AUTO) {
        nParallel = AppResourcePool::instance()->getIdealThreadCount();
    }
    assert(nParallel>=1);
    return nParallel;
}

void Task::setMinimizeSubtaskErrorText(bool v) {
    assert(flags.testFlag(TaskFlag_FailOnSubtaskError));
    setFlag(TaskFlag_MinimizeSubtaskErrorText, v);
}

void Task::addTaskResource(const TaskResourceUsage& r) {
    SAFE_POINT(state == Task::State_New, QString("Can't add task resource in current state: %1)").arg(getState()),);    
    SAFE_POINT(!insidePrepare || !r.prepareStageLock, "Can't add prepare-time resource from within prepare function call!",);    
    SAFE_POINT(!r.locked, QString("Resource is already locked, resource id: %1").arg(r.resourceId),);
    taskResources.append(r);
}

//////////////////////////////////////////////////////////////////////////
// task scheduler

void TaskScheduler::addSubTask(Task* t, Task* sub) {
    SAFE_POINT(t != NULL, "When adding subtask to TaskScheduler, the parent task is NULL",);
    SAFE_POINT(sub != NULL, "When adding subtask to TaskScheduler, the subtask is NULL",);
    SAFE_POINT(sub->getParentTask() == NULL, "Task already has a parent!",);

    sub->parentTask = t;
    t->subtasks.append(sub);
    emit t->si_subtaskAdded(sub);
}

void TaskScheduler::setTaskState(Task* t, Task::State newState) { 
    SAFE_POINT(t->getState() < newState, QString("Illegal task state change! Current state: %1, new state: %2").arg(t->getState()).arg(newState),);

    t->state = newState; 
    
    emit t->si_stateChanged();
    emit si_stateChanged(t);
}

void TaskScheduler::setTaskStateDesc(Task* t, const QString& desc) { 
    t->stateInfo.setDescription(desc);
}

void TaskScheduler::setTaskInsidePrepare(Task* t, bool val) {
    t->insidePrepare = val;
}

}//namespace
