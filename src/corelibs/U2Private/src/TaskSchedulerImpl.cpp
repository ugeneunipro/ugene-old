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

#include "TaskSchedulerImpl.h"

#include <U2Core/Log.h>
#include <U2Core/AppResources.h>
#include <U2Core/Timer.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/L10n.h>

#include <QtCore/QCoreApplication>

/* TRANSLATOR U2::TaskSchedulerImpl */

namespace U2 {

#define UPDATE_TIMEOUT 100

TaskSchedulerImpl::TaskSchedulerImpl(AppResourcePool* rp) {
    resourcePool = rp;

    stateNames << tr("New") <<  tr("Prepared") <<  tr("Running") << tr("Finished");
    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
    timer.start(UPDATE_TIMEOUT);

    stateChangesObserved = false;
    threadsResource = resourcePool->getResource(RESOURCE_THREAD);

    schedulerLog = new LogCache();
    schedulerLog->filter.filters.append(LogFilterItem(ULOG_CAT_TASKS, LogLevel_TRACE));
    schedulerLog->filter.filters.append(LogFilterItem(ULOG_CAT_CORE_SERVICES, LogLevel_TRACE));
    schedulerLog->filter.filters.append(LogFilterItem(ULOG_CAT_IO, LogLevel_TRACE));
    schedulerLog->filter.filters.append(LogFilterItem(ULOG_CAT_USER_INTERFACE, LogLevel_ERROR));
    schedulerLog->filter.filters.append(LogFilterItem(ULOG_CAT_ALGORITHM, LogLevel_TRACE));
    schedulerLog->filter.filters.append(LogFilterItem(ULOG_CAT_CONSOLE, LogLevel_ERROR));
}

TaskSchedulerImpl::~TaskSchedulerImpl() {
    assert(topLevelTasks.empty());
    assert(priorityQueue.isEmpty());
}


void TaskSchedulerImpl::cancelTask(Task* task) {
    if (task->getState() < Task::State_Finished) {
        taskLog.info(tr("Canceling task: %1").arg(task->getTaskName()));
        getTaskStateInfo(task).cancelFlag = true;
        foreach(Task* t, task->getSubtasks()) {
            cancelTask(t);
        }
    }
}

void TaskSchedulerImpl::cancelAllTasks() {
    foreach(Task* t, topLevelTasks) {
        cancelTask(t);
    }
}


void TaskSchedulerImpl::propagateStateToParent(Task* task) {
    Task* parentTask = task->getParentTask();
    if (parentTask == NULL) {
        return;
    }
    if (parentTask->hasError() || parentTask->isCanceled()) {
        return;
    }
    if (task->isCanceled() && parentTask->getFlags().testFlag(TaskFlag_FailOnSubtaskCancel)) {
        TaskStateInfo& tsi = getTaskStateInfo(parentTask);
        tsi.setError( tr("Subtask {%1} is canceled %2").arg(task->getTaskName()).arg(task->getError()) );
        cancelTask(parentTask);
    } else if (task->hasError() && parentTask->getFlags().testFlag(TaskFlag_FailOnSubtaskError)) {
        TaskStateInfo& tsi = getTaskStateInfo(parentTask);
        if (parentTask->getFlags().testFlag(TaskFlag_MinimizeSubtaskErrorText)) {
            tsi.setError(task->getError());
        } else {
            tsi.setError( tr("Subtask {%1} is failed: %2").arg(task->getTaskName()).arg(task->getError()));
        }
    }
}

bool TaskSchedulerImpl::processFinishedTasks() {
    bool hasFinished = false;
    for (int i = priorityQueue.size(); --i>=0;) {
        TaskInfo* ti = priorityQueue[i];
        TaskInfo* pti = ti->parentTaskInfo;
        Task::State state = ti->task->getState();
        assert(state != Task::State_Finished);

        if (ti->task->getTimeOut() > 0){
            int secsPassed = GTimer::secsBetween(ti->task->getTimeInfo().startTime, GTimer::currentTimeMicros());
            if(ti->task->getTimeOut() < secsPassed){
                QString msg = QString("Timeout error, running %1 sec expected %2 sec.").arg(secsPassed).arg(ti->task->getTimeOut());
                taskLog.error(QString("Task {%1} %2 Cancelling task...").arg(ti->task->getTaskName()).arg(msg));

                ti->task->setError(msg);
                ti->task->cancel();
            }

        }
        //if the task was canceled -> cancel subtasks too
        if (ti->task->isCanceled() && !ti->subtasksWereCanceled) {
            ti->subtasksWereCanceled = true;
            foreach(Task* t, ti->task->getSubtasks()) {
                cancelTask(t);
            }
        }
        if (state != Task::State_Running) {
            continue;
        }

        if (ti->selfRunFinished && ti->hasLockedRunResources) {
            releaseResources(ti, false); //release resources for RUN stage
        }

        //update state desc
        updateTaskProgressAndDesc(ti);
        if (!readyToFinish(ti)) { //the task can be finished only after all its subtasks finished and its run finished
            continue;
        }

        if (ti->wasPrepared) {
            Task::ReportResult res = ti->task->report();
            if (res == Task::ReportResult_CallMeAgain) {
                continue;
            }
        }

        hasFinished = true;
        promoteTask(ti, Task::State_Finished);

        releaseResources(ti, true); //release resources for PREPARE stage

        Task* task = ti->task;
        priorityQueue.removeAt(i);
        delete ti; //task is removed from priority queue

        // notify parent that subtask finished, check if there are new subtasks from parent
        if (pti != NULL) {
            Task* parentTask = pti->task;
            propagateStateToParent(task);
            QList<Task*> newSubTasks = onSubTaskFinished(parentTask, task);
            if (!newSubTasks.isEmpty() || !pti->newSubtasks.isEmpty()) {
                if (!tasksWithNewSubtasks.contains(pti)) {
                    tasksWithNewSubtasks.append(pti);
                }
            }
            foreach(Task* newSub, newSubTasks) {
                pti->newSubtasks.append(newSub);
                addSubTask(parentTask, newSub);
            }
        }
    }
    return hasFinished;
}

void TaskSchedulerImpl::unregisterFinishedTopLevelTasks() {
    QList<Task*> tasksToDelete;
    foreach(Task* task, topLevelTasks) {
        if (task->getState() == Task::State_Finished) {
            tasksToDelete.append(task);
        }
    }
    foreach(Task* task, tasksToDelete) {
        unregisterTopLevelTask(task);
    }
}

void TaskSchedulerImpl::processNewSubtasks() {
    for (int i=0, n = tasksWithNewSubtasks.size();i<n; i++) {
        TaskInfo* ti = tasksWithNewSubtasks[i];
        assert(ti->newSubtasks.size() > 0);

        int nParallel = ti->task->getNumParallelSubtasks();
        int nNew = ti->newSubtasks.size();
        int nToRun = qMin(nParallel - ti->numActiveSubtasks(), nNew);
        int nRun = 0;
        for (int j=0; j < nToRun; j++) {
            Task* newSub = ti->newSubtasks[j];
            bool ok = addToPriorityQueue(newSub, ti);
            if (ok) {
                ti->newSubtasks[j] = NULL;
                nRun++;
            }
        }
        if (nRun == nNew) {
            ti->newSubtasks.clear();
            tasksWithNewSubtasks[i] = NULL;
        } else if (nRun > 0) {
            ti->newSubtasks.removeAll(NULL);
        }
    }
    tasksWithNewSubtasks.removeAll(NULL);
}


void TaskSchedulerImpl::runReady() {
    foreach(TaskInfo* ti, priorityQueue) {
        Task* task = ti->task;
        Task::State state = task->getState();
        //Note: task is running if any of its subtasks is running
        assert(state == Task::State_Prepared || state == Task::State_Running);
        if (task->getFlags().testFlag(TaskFlag_NoRun) || task->isCanceled() || task->hasError()) {
            if (state == Task::State_Prepared) {
                promoteTask(ti, Task::State_Running);
            }
            if (ti->thread == NULL) {
                ti->selfRunFinished = true;
            }
            continue;
        }
        if (ti->thread!=NULL) { //task is already running in a separate thread
            assert(state == Task::State_Running);
            continue;
        }
        bool ready = task->hasFlags(TaskFlag_RunBeforeSubtasksFinished) || ti->numFinishedSubtasks == task->getSubtasks().size();
        if (!ready) {
            continue;
        }
        QString noResMessage = tryLockResources(ti->task, false, ti->hasLockedRunResources);
        if (!noResMessage.isEmpty()) {
            setTaskStateDesc(ti->task, noResMessage);
            continue;
        }
        if (state == Task::State_Prepared) {
            promoteTask(ti, Task::State_Running);
        }
        setTaskStateDesc(ti->task, "");
        runThread(ti);
    }
}


void TaskSchedulerImpl::runThread(TaskInfo* ti) {
#ifdef _DEBUG
    assert(ti->task->getState() == Task::State_Running);
    assert(!ti->task->getFlags().testFlag(TaskFlag_NoRun));
    assert(ti->task->hasFlags(TaskFlag_RunBeforeSubtasksFinished) || ti->numFinishedSubtasks == ti->task->getSubtasks().size());
    assert(!ti->task->isCanceled());
    assert(!ti->task->hasError());
    assert(!ti->selfRunFinished);
#endif
    ti->thread = new TaskThread(ti);
    connect(ti->thread, SIGNAL(finished()), SLOT(sl_threadFinished()));
    ti->thread->start();
}


QString TaskSchedulerImpl::tryLockResources(Task* task, bool prepareStage, bool& hasLockedResourcesAfterCall) {
    if (prepareStage) { //task must be New
        SAFE_POINT(task->getState() == Task::State_New, "Attempt to lock prepare-stage resources for non-NEW task!", L10N::internalError());
    } else { //task must be Prepared or Running. Task can be 'Running' if it has subtasks
        SAFE_POINT(task->getState() == Task::State_Running || task->getState() == Task::State_Prepared, QString("Attempt to lock run-stage for task in state: %1!").arg(task->getState()), L10N::internalError());
    }
    if (!prepareStage && !threadsResource->isAvailable()) {
        return tr("Waiting for resource '%1', count: %2").arg(threadsResource->name).arg(1);
    }
    TaskResources& tres = getTaskResources(task);
    for (int i=0, n = tres.size(); i<n; i++) {
        const TaskResourceUsage& taskRes = tres[i];
        if (taskRes.prepareStageLock != prepareStage) {
            assert(prepareStage ? !taskRes.locked : taskRes.locked);
            continue;
        }
        assert(!prepareStage || taskRes.resourceId!=RESOURCE_THREAD);
        AppResource* appRes = resourcePool->getResource(taskRes.resourceId);
        if (!appRes->isAvailable(taskRes.resourceUse)) {
            if (appRes->maxUse < taskRes.resourceUse) {
                task->setError(tr("Not enough resources for the task, resource name: '%1' max: %2%3 requested: %4%5")
                    .arg(appRes->name).arg(appRes->maxUse).arg(appRes->suffix).arg(taskRes.resourceUse).arg(appRes->suffix));
            }
            return tr("Waiting for resource '%1', count: %2%3").arg(appRes->name).arg(taskRes.resourceUse).arg(appRes->suffix);
        }
    }
    //ok there are all task resources available -> lock it
    int nLocked = 0;
    if (!prepareStage) {
        threadsResource->acquire();
        taskLog.trace(QString("acquiring resource: '%1':%2, state: %3/%4").arg(threadsResource->name).arg(1).arg(threadsResource->currentUse).arg(threadsResource->maxUse));
        nLocked++;
    }
    for (int i=0, n = tres.size(); i<n; i++) {
        TaskResourceUsage& taskRes = tres[i];
        if (taskRes.prepareStageLock != prepareStage) {
            continue;
        }
        AppResource* appRes = resourcePool->getResource(taskRes.resourceId);
        appRes->acquire(taskRes.resourceUse);
        taskRes.locked = true;
        nLocked++;
        taskLog.trace(QString("acquiring resource: '%1':%2, state: %3/%4").arg(appRes->name).arg(taskRes.resourceUse).arg(appRes->currentUse).arg(appRes->maxUse));
    }
    hasLockedResourcesAfterCall = nLocked > 0;
    return QString::null;
}

void TaskSchedulerImpl::releaseResources(TaskInfo* ti, bool prepareStage) {
    SAFE_POINT(ti->task->getState() == (prepareStage ? Task::State_Finished : Task::State_Running), "Releasing task resources in illegal state!",);
    if (!(prepareStage ? ti->hasLockedPrepareResources : ti->hasLockedRunResources)) {
        return;
    }
    if (!prepareStage) {
        threadsResource->release();
    }
    taskLog.trace(QString("releasing resource: '%1':%2, state: %3/%4").arg(threadsResource->name).arg(1).arg(threadsResource->currentUse).arg(threadsResource->maxUse));
    TaskResources& tres = getTaskResources(ti->task);
    for (int i=0, n = tres.size(); i<n; i++) {
        TaskResourceUsage& taskRes = tres[i];
        if (taskRes.prepareStageLock != prepareStage) {
            assert(prepareStage ? !taskRes.locked : taskRes.locked);
            continue;
        }
        AppResource* appRes = resourcePool->getResource(taskRes.resourceId);
        appRes->release(taskRes.resourceUse);
        taskRes.locked = false;
        taskLog.trace(QString("releasing resource: '%1':%2, state: %3/%4").arg(appRes->name).arg(taskRes.resourceUse).arg(appRes->currentUse).arg(appRes->maxUse));
    }
    if (prepareStage) {
        ti->hasLockedPrepareResources = false;
    } else {
        ti->hasLockedRunResources = false;
    }
}

void TaskSchedulerImpl::update() {
    static bool recursion = false;

    if (recursion) {
        return;
    }
    recursion = true;
    stateChangesObserved = false;

    bool finishedFound = processFinishedTasks();
    if (finishedFound) {
        unregisterFinishedTopLevelTasks();
    }
    processNewSubtasks();

    prepareNewTasks();

    runReady();

    updateOldTasksPriority();

    if (stateChangesObserved) {
        stateChangesObserved = false;
        timer.setInterval(0);
    } else if (timer.interval()!=UPDATE_TIMEOUT){
        timer.setInterval(UPDATE_TIMEOUT);
    }

    recursion = false;
}

void TaskSchedulerImpl::prepareNewTasks() {
    if (newTasks.empty()) {
        return;
    }
    QList<Task*> newCopy = newTasks;
    newTasks.clear();
    foreach(Task* task, newCopy) {
        if (task->hasError() || task->isCanceled()) {//check if its canceled or has errors
            propagateStateToParent(task);
            //forget about this task
            TaskInfo ti(task, 0);
            promoteTask(&ti, Task::State_Finished);
            if (task->isTopLevelTask()) {
                unregisterTopLevelTask(task);
            }
            continue;
        }
        bool ok = addToPriorityQueue(task, NULL);
        if (!ok) {
            newTasks.append(task);
        }
    }
}

void TaskSchedulerImpl::registerTopLevelTask(Task* task) {
    SAFE_POINT(task, QString("Trying to register NULL task"),);

#ifdef _DEBUG
    QThread* appThread = QCoreApplication::instance()->thread();
    QThread* thisThread = QThread::currentThread();
    QThread* taskThread = task->thread();
    assert(appThread == thisThread);
    assert(taskThread == thisThread);
#endif

    SAFE_POINT(task->getState() == Task::State_New, QString("Trying to register task in not NEW state. State: %1").arg(task->getState()),);
    SAFE_POINT(!topLevelTasks.contains(task), QString("Task is already registered: %1").arg(task->getTaskName()),);    
    
    taskLog.details(tr("Registering new task: %1").arg(task->getTaskName()));
    topLevelTasks.append(task);
    emit si_topLevelTaskRegistered(task);
    newTasks.append(task);
}

bool TaskSchedulerImpl::addToPriorityQueue(Task* task, TaskInfo* pti) {
    if (pti!=NULL && (pti->task->isCanceled() || pti->task->hasError())) { //canceled tasks are not processed
        task->cancel();
    }

    //check if there are enough resources;
    bool runPrepare = !task->isCanceled() && !task->hasError();
    bool lr = false;
    if (runPrepare) {
        QString noResMessage = tryLockResources(task, true, lr);
        if (!noResMessage.isEmpty()) {
            setTaskStateDesc(task, noResMessage);
            if (!task->hasError()) {
                return false; //call again
            } else {
                runPrepare = false; // resource lock error
            }
        }
    }

    TaskInfo* ti = new TaskInfo(task, pti);
    ti->hasLockedPrepareResources = lr;
    priorityQueue.append(ti);
    if (runPrepare) {
        setTaskInsidePrepare(task, true);
        task->prepare();
        setTaskInsidePrepare(task, false);
        ti->wasPrepared = true;
    }
    promoteTask(ti, Task::State_Prepared);

    int nParallel = task->getNumParallelSubtasks();
    const QList<Task*>& subtasks = task->getSubtasks();
    for (int i = 0, n = subtasks.size(); i < n; i++) {
        Task* sub = subtasks[i];
        bool ok = i < nParallel && addToPriorityQueue(sub, ti);
        if (!ok && !sub->hasError()) { //if task got err on resource allocation -> its not new now, but failed
            ti->newSubtasks.append(sub);
            if( !tasksWithNewSubtasks.contains(ti) ){
                tasksWithNewSubtasks.append(ti);
            }
        }
    }
    return true;
}

void TaskSchedulerImpl::unregisterTopLevelTask(Task* task) {
    assert(topLevelTasks.contains(task));
    taskLog.trace(tr("Unregistering task: %1").arg(task->getTaskName()));
    stopTask(task);
    topLevelTasks.removeOne(task);

    emit si_topLevelTaskUnregistered(task);

    if (!task->hasFlags(TaskFlag_NoAutoDelete)) {
        deleteTask(task);
    }
}

void TaskSchedulerImpl::stopTask(Task* task) {
    foreach(Task* sub, task->getSubtasks()) {
        stopTask(sub);
    }

    foreach(TaskInfo* ti,  priorityQueue) { //stop task if its running
        if (ti->task == task) {
            cancelTask(task);
            if (ti->thread!=NULL && !ti->thread->isFinished()) {
                ti->thread->wait();//TODO: try avoid blocking here
            }
            assert(readyToFinish(ti));
            break;
        }
    }
}


bool TaskSchedulerImpl::readyToFinish(TaskInfo* ti) {
    if (ti->task->getState() == Task::State_Finished) {
        return true;
    }
    if (ti->task->getState() != Task::State_Running) {
        return false;
    }
    if (ti->numFinishedSubtasks < ti->task->getSubtasks().size()) {
        return false;
    }
    if (!ti->selfRunFinished) {
        return false;
    }
#ifdef _DEBUG
    foreach(Task* sub, ti->task->getSubtasks()) { //must be true because of 'numFinishedSubtasks' check above
        assert(sub->getState() == Task::State_Finished);
    }
    assert(ti->newSubtasks.isEmpty());
#endif
    return true;
}

QString TaskSchedulerImpl::getStateName(Task* t) const {
    Task::State s = t->getState();
    return stateNames[s];
}


QDateTime TaskSchedulerImpl::estimatedFinishTime(Task* task) const {
    SAFE_POINT(task->getState() == Task::State_Running, "Method is valid for running tasks only", QDateTime());

    const TaskTimeInfo& tti = task->getTimeInfo();
    int secsPassed = GTimer::secsBetween(tti.startTime, GTimer::currentTimeMicros());
    float percentInSecs = task->getProgress() / (float)secsPassed;
    int secsTotal = int(percentInSecs * 100);
    int secsLeft = secsTotal - secsPassed;

    QDateTime res = QDateTime::currentDateTime();
    res.addSecs(secsLeft);
    return res;
}

static QString state2String(Task::State state) {
    switch(state) {
        case Task::State_New:
            return TaskSchedulerImpl::tr("New");
        case Task::State_Prepared:
            return TaskSchedulerImpl::tr("Prepared");
        case Task::State_Running:
            return TaskSchedulerImpl::tr("Running");
        case Task::State_Finished:
            return TaskSchedulerImpl::tr("Finished");
        default: assert(0);
    }
    return TaskSchedulerImpl::tr("Invalid name");
}

void TaskSchedulerImpl::checkSerialPromotion(TaskInfo* pti, Task* subtask) {
#ifdef _DEBUG
    assert(!subtask->isNew()); //must be promoted at this point -> check algorithm depends requirement
    Task* task = pti == NULL ? NULL : pti->task;
    if (task == NULL) {
        return;
    }
    int nParallel = task->getNumParallelSubtasks();
    bool before = true;
    int numActive = 0;
    const QList<Task*>& subs = task->getSubtasks();
    for (int i=0, n = subs.size(); i < n; i++) {
        Task* sub = subs[i];
        if (!sub->isNew() && !sub->isFinished()) {
            numActive++;
            assert(numActive <= nParallel);
        }
        if (sub == subtask) {
            before = false;
        } else if (before) {
            Task::State subState = sub->getState();
            assert(subState!=Task::State_New);
        }
    }
#else
    Q_UNUSED(pti);
    Q_UNUSED(subtask);
#endif
}

static void checkFinishedState(TaskInfo* ti) {
#ifdef _DEBUG
    foreach(Task* sub, ti->task->getSubtasks()) {
        assert(sub->getState() == Task::State_Finished);
    }
    assert(ti->newSubtasks.empty());
    assert(ti->task->getSubtasks().size() == ti->numFinishedSubtasks);
    assert(ti->numRunningSubtasks  == 0);
    assert(ti->numPreparedSubtasks  == 0);
#else
    Q_UNUSED(ti);
#
#endif
}

void TaskSchedulerImpl::promoteTask(TaskInfo* ti, Task::State newState) {
    stateChangesObserved = true;

    Task* task = ti->task;
    assert(newState > task->getState());

    setTaskState(task, newState); //emits signals

    TaskStateInfo& tsi = getTaskStateInfo(task);
    TaskTimeInfo& tti = getTaskTimeInfo(task);
    TaskInfo* pti = ti->parentTaskInfo;

    if (!tsi.hasError()) {
        taskLog.trace(tr("promoting task {%1} to '%2'").arg(task->getTaskName()).arg(state2String(newState)));
    } else {
        taskLog.trace(tr("promoting task {%1} to '%2', error '%3'").arg(task->getTaskName()).arg(state2String(newState)).arg(tsi.getError()));
    }

    checkSerialPromotion(pti, ti->task);
    switch(newState) {
        case Task::State_Prepared:
            if (pti != NULL) {
                pti->numPreparedSubtasks++;
            }
            if (ti->task->isTopLevelTask() && ti->task->isVerboseLogMode()) {
                taskLog.info(tr("Starting {%1} task").arg(ti->task->getTaskName()));
            }
            break;
        case Task::State_Running:
            tti.startTime = GTimer::currentTimeMicros();
            if (pti != NULL) {
                pti->numPreparedSubtasks--;
                pti->numRunningSubtasks++;
                if (pti->task->getState() < Task::State_Running) {
                    assert(pti->task->getState() == Task::State_Prepared);
                    promoteTask(pti, Task::State_Running);
                }
            }
            break;
        case Task::State_Finished:
            checkFinishedState(ti);
            tti.finishTime = GTimer::currentTimeMicros();
            tsi.setDescription(QString());
            if (pti != NULL) {
                pti->numRunningSubtasks--;
                assert(pti->numRunningSubtasks>=0);
                pti->numFinishedSubtasks++;
                assert(pti->numFinishedSubtasks <= pti->task->getSubtasks().size());
            }
            if (ti->task->isTopLevelTask()) {
                if (tsi.hasError() && !tsi.cancelFlag) {
                    taskLog.error(tr("Task {%1} finished with error: %2").arg(task->getTaskName()).arg(tsi.getError()));
                } else if (tsi.cancelFlag) {
                    taskLog.info(tr("Task {%1} canceled").arg(ti->task->getTaskName()));
                } else if (ti->task->isVerboseLogMode()) {
                    taskLog.info(tr("Task {%1} finished").arg(ti->task->getTaskName()));
                }
            }
            break;
        default: assert(0);
    }
#ifdef _DEBUG
    Task* parentTask = task->getParentTask();
    if (parentTask!=NULL){
        int localPreparedSubs, localRunningSubs, localFinishedSubs, localNewSubs, localTotalSubs;
        localTotalSubs = localNewSubs = localPreparedSubs = localRunningSubs = localFinishedSubs = 0;
        foreach (Task* sub, parentTask->getSubtasks()){
            switch (sub->getState()){
                case Task::State_New:
                    localNewSubs++;
                    break;
                case Task::State_Prepared:
                    localPreparedSubs++;
                    break;
                case Task::State_Running:
                    localRunningSubs++;
                    break;
                case Task::State_Finished:
                    localFinishedSubs++;
                    break;
            }
            localTotalSubs++;
        }
        assert(localPreparedSubs == pti->numPreparedSubtasks);
        assert(localRunningSubs == pti->numRunningSubtasks);
        assert(localPreparedSubs+localRunningSubs == pti->numActiveSubtasks());
        assert(localFinishedSubs == pti->numFinishedSubtasks);
        assert(localTotalSubs == localNewSubs+localPreparedSubs+localRunningSubs+localFinishedSubs);
    }
#endif
    updateTaskProgressAndDesc(ti);
}


void TaskSchedulerImpl::updateTaskProgressAndDesc(TaskInfo* ti) {
    Task* task = ti->task;
    TaskStateInfo& tsi = getTaskStateInfo(task);


    //update desc
    if (ti->task->useDescriptionFromSubtask()) {
        const QList<Task*>& subs = task->getSubtasks();
        if (!subs.isEmpty()) {
            Task* sub = subs.last();
            tsi.setDescription(sub->getStateInfo().getDescription());
        }
    }
    QString currentDesc = tsi.getDescription();
    if (currentDesc!= ti->prevDesc) {
        ti->prevDesc = currentDesc;
        emit_taskDescriptionChanged(task);
    }

    //update progress
    int newProgress = tsi.progress;
    bool updateProgress = false;
    if (task->isFinished()) {
        newProgress = 100;
        updateProgress = tsi.progress != newProgress;
    } else if (task->getProgressManagementType() == Task::Progress_Manual) {
        int prevProgress = ti->prevProgress;
        if (tsi.progress != prevProgress) {
            ti->prevProgress = tsi.progress;
            emit_taskProgressChanged(task);
        }
    }  else {
        assert(task->getProgressManagementType() == Task::Progress_SubTasksBased);
        const QList<Task*>& subs = task->getSubtasks();
        int nsubs = subs.size();
        if (nsubs > 0) {
            float sum = 0;
            float maxSum = 0.001F;
            foreach(Task* sub, subs) {
                float w = sub->getSubtaskProgressWeight();
                sum += sub->getProgress() * w;
                maxSum += w;
            }
            if (maxSum > 1.0f) {
                newProgress = qRound(sum / maxSum);
            } else {
                newProgress = qRound(sum);
            }
        }
        updateProgress = tsi.progress != newProgress;
    }

    if (updateProgress) {
        tsi.progress = newProgress;
        emit_taskProgressChanged(task);
    }
}

void TaskSchedulerImpl::deleteTask(Task* task) {
    foreach(Task* sub, task->getSubtasks()) {
        //todo: check subtask autodelete ??
        deleteTask(sub);
    }
    taskLog.trace(tr("Deleting task: %1").arg(task->getTaskName()));
    delete task;
}

#define MAX_SECS_TO_LOWER_PRIORITY 60
static QThread::Priority getThreadPriority(Task* t) {
    assert(t->isTopLevelTask());
    assert(t->isRunning());
    int secsPassed = GTimer::secsBetween(t->getTopLevelParentTask()->getTimeInfo().startTime, GTimer::currentTimeMicros());
    if (secsPassed > MAX_SECS_TO_LOWER_PRIORITY) {
        return QThread::LowestPriority;
    }
    return QThread::LowPriority;
}

static void updateThreadPriority(TaskInfo* ti) {
    assert(ti->thread!=NULL);
    QThread::Priority tp = getThreadPriority(ti->task->getTopLevelParentTask());
    if (ti->thread->priority() != tp && ti->thread->isRunning()) {
        ti->thread->setPriority(tp);
    }
}

void TaskSchedulerImpl::updateOldTasksPriority() {
//work every N-th tick
#define UPDATE_GRAN 10
    static int n = UPDATE_GRAN;
    if (--n !=0) {
        return;
    }
    n = UPDATE_GRAN;

    foreach(TaskInfo* ti, priorityQueue) {
        if (!ti->task->isRunning() || ti->thread == NULL || !ti->thread->isRunning()) {
            continue;
        }
        updateThreadPriority(ti);
    }
}

void TaskSchedulerImpl::sl_threadFinished() {
    timer.setInterval(0);
}

Task * TaskSchedulerImpl::getTopLevelTaskById( qint64 id ) const {
    Task * ret = NULL;
    foreach( Task * task, topLevelTasks ) {
        assert( NULL != task );
        if( id == task->getTaskId() ) {
            ret = task;
            break;
        }
    }
    return ret;
}

static QMutex lock;

void TaskThread::run() {
   // try {
    Qt::HANDLE handle= QThread::currentThreadId();
    lock.lock();
    AppContext::getTaskScheduler()->addThreadId(ti->task->getTaskId(), handle);
    lock.unlock();

        assert(!ti->selfRunFinished);
        assert(ti->task->getState()== Task::State_Running);

        updateThreadPriority(ti);
        ti->task->run();
        assert(ti->task->getState()== Task::State_Running);
        ti->selfRunFinished = true;

        lock.lock();
    AppContext::getTaskScheduler()->removeThreadId(ti->task->getTaskId());
    lock.unlock();
    //delete handle;
    /*}
    catch(...) {
        if(CrashHandler::buffer) {
            CrashHandler::releaseReserve();
        }
        CrashHandler::runMonitorProcess("|Unhandled exception");
    }*/
}

TaskInfo::~TaskInfo() {
    if (thread!=NULL) {
        if (!thread->isFinished()) {
            taskLog.trace("TaskScheduler: Waiting for the thread before delete");
            thread->wait();
            taskLog.trace("TaskScheduler: Wait finished");
        }
        delete thread;
    }
}

}//namespace
