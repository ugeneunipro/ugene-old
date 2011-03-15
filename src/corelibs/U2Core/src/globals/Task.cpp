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

#include "Task.h"
#include <U2Core/AppResources.h>

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
    taskId      = genTaskId();
    parentTask  = NULL;
    progressWeightAsSubtask = 1;
    maxParallelSubtasks = MAX_PARALLEL_SUBTASKS_SERIAL;
    timeOut = 0;
}

void Task::setMaxParallelSubtasks(int n) {
    assert(n>=0);
    maxParallelSubtasks = n;
}

void Task::setTaskName(const QString& _taskName) {
    assert(isNew());
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
    assert(sub->parentTask==NULL);
    assert(state == State_New);
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
    if (hasErrors()) {
        return true;
    }
    Task* badChild = getSubtaskWithErrors();
    if (badChild) {
        stateInfo.setError(stateInfo.getError() + badChild->getError());
    }
    return stateInfo.hasErrors();
}

Task* Task::getSubtaskWithErrors() const  {
    foreach(Task* sub, getSubtasks()) {
        if (sub->hasErrors()) {
            return sub;
        }
    }
    return NULL;
}

QList<Task*> Task::onSubTaskFinished(Task* subTask){ 
    Q_UNUSED(subTask); 
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

//////////////////////////////////////////////////////////////////////////
// task scheduler

void TaskScheduler::addSubTask(Task* t, Task* sub) {
    assert(sub->getParentTask() == NULL);
    sub->parentTask = t;
    t->subtasks.append(sub);
    emit t->si_subtaskAdded(sub);
}

void TaskScheduler::setTaskState(Task* t, Task::State newState) { 
    assert(t->getState() < newState); 
    t->state = newState; 
    
    emit t->si_stateChanged();
    emit si_stateChanged(t);
}

void TaskScheduler::setTaskStateDesc(Task* t, const QString& desc) { 
    t->stateInfo.setStateDesc(desc);
}


}//namespace
