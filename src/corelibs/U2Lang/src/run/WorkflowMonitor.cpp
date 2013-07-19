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

#include <U2Lang/ActorModel.h>
#include <U2Lang/WorkflowRunTask.h>
#include <U2Lang/WorkflowUtils.h>

#include "SeparateProcessMonitor.h"

#include "WorkflowMonitor.h"

namespace U2 {
namespace Workflow {
using namespace Monitor;

WorkflowMonitor::WorkflowMonitor(WorkflowAbstractIterationRunner *_task, const QList<Actor*> &procs)
: QObject(), task(_task)
{
    foreach (Actor *p, procs) {
        procMap[p->getId()] = p;
        addTime(0, p->getId());
    }

    foreach (Actor *p, procs){
        WorkerParamsInfo info;
        info.workerName = p->getLabel();
        QMap<QString, Attribute *> params = p->getParameters();
        QMapIterator<QString, Attribute *> paramsIter(params);
        while (paramsIter.hasNext()) {
            paramsIter.next();
            Attribute *attr = paramsIter.value();
            SAFE_POINT(NULL != attr, "NULL attribute in params!", );

            info.paramsWithValues.insert(attr->getDisplayName(),
                WorkflowUtils::getStringForParameterDisplayRole(attr->getAttributeValueWithoutScript<QVariant>()));
        }
        workersParamsInfo << info;
    }

    connect(task.data(), SIGNAL(si_updateProducers()), SIGNAL(si_updateProducers()));
    connect(task.data(), SIGNAL(si_progressChanged()), SLOT(sl_progressChanged()));
    connect(task.data(), SIGNAL(si_stateChanged()), SLOT(sl_taskStateChanged()));
}

QString WorkflowMonitor::getName() const {
    return name;
}

const QList<FileInfo> & WorkflowMonitor::getOutputFiles() const {
    return outputFiles;
}

const QList<Problem> & WorkflowMonitor::getProblems() const {
    return problems;
}

const QMap<QString, WorkerInfo> & WorkflowMonitor::getWorkersInfo() const {
    return workers;
}

const QList<WorkerParamsInfo> & WorkflowMonitor::getWorkersParameters() const {
    return workersParamsInfo;
}

QString WorkflowMonitor::actorName(const QString &id) const {
    SAFE_POINT(procMap.contains(id), "Unknown actor id", "");
    return procMap[id]->getLabel();
}

void WorkflowMonitor::addOutputFile(const QString &url, const QString &producer) {
    CHECK(!url.isEmpty(), );
    FileInfo info(url, producer);
    CHECK(!outputFiles.contains(info), );

    outputFiles << info;
    cmdLog.trace(PackUtils::packFileInfo(info));
    emit si_newOutputFile(info);
}

void WorkflowMonitor::addError(const QString &message, const QString &actor) {
    addProblem(Problem(message, actor));
    coreLog.error(message);
}

void WorkflowMonitor::addTaskError(Task *task, const QString &message) {
    SAFE_POINT(taskMap.contains(task), "Unregistered task", );
    CHECK(!errorTasks.contains(task), );
    QString error = message.isEmpty() ? task->getError() : message;
    addError(error, taskMap[task]->getId());
    errorTasks << task;
}

void WorkflowMonitor::addTime(qint64 timeMks, const QString &actor) {
    WorkerInfo &info = workers[actor];
    info.timeMks += timeMks;
    cmdLog.trace(PackUtils::packWorkerInfo(actor, info));
    emit si_workerInfoChanged(actor, info);
}

void WorkflowMonitor::addTick(qint64 timeMks, const QString &actor) {
    workers[actor].ticks += 1;
    addTime(timeMks, actor);
}

void WorkflowMonitor::start() {
    setRunState(false);
}

void WorkflowMonitor::pause() {
    setRunState(true);
}

void WorkflowMonitor::registerTask(Task *task, const QString &actor) {
    SAFE_POINT(procMap.contains(actor), "Unknown actor id", );
    taskMap[task] = procMap[actor];
}

void WorkflowMonitor::setOutputDir(const QString &dir) {
    _outputDir = dir;
    emit si_dirSet(outputDir());
}

QString WorkflowMonitor::outputDir() const {
    return _outputDir;
}

void WorkflowMonitor::sl_progressChanged() {
    CHECK(!task.isNull(), );
    emit si_progressChanged(task->getProgress());
}

void WorkflowMonitor::sl_taskStateChanged() {
    CHECK(!task.isNull(), );
    if (task->isFinished()) {
        TaskState state = SUCCESS;
        if (task->isCanceled()) {
            state = CANCELLED;
        } else if (task->hasError() || !problems.isEmpty()) {
            state = FAILED;
        }
        emit si_taskStateChanged(state);
        emit si_report();
    }
}

void WorkflowMonitor::setWorkerInfo(const QString &actorId, const WorkerInfo &info) {
    workers[actorId] = info;
    emit si_workerInfoChanged(actorId, info);
}

void WorkflowMonitor::setRunState(bool paused) {
    cmdLog.trace(PackUtils::packRunState(paused));
    emit si_runStateChanged(paused);
}

int WorkflowMonitor::getDataProduced(const QString &actor) const {
    CHECK(!task.isNull(), 0);
    return task->getDataProduced(actor);
}

void WorkflowMonitor::addProblem(const Monitor::Problem &problem) {
    if (problems.isEmpty()) {
        emit si_firstProblem();
        emit si_taskStateChanged(RUNNING_WITH_PROBLEMS);
    }
    problems << problem;
    emit si_newProblem(problem);
    cmdLog.trace(PackUtils::packProblem(problem));
}

/************************************************************************/
/* FileInfo */
/************************************************************************/
FileInfo::FileInfo( )
    : url( ), actor( )
{

}

FileInfo::FileInfo(const QString &_url, const QString &_producer)
: url(_url), actor(_producer)
{

}

bool FileInfo::operator== (const FileInfo &other) const {
    return url == other.url;
}

/************************************************************************/
/* Problem */
/************************************************************************/
Problem::Problem( )
    : message( ), actor( )
{

}

Problem::Problem(const QString &_message, const QString &_actor)
: message(_message), actor(_actor)
{

}

bool Problem::operator== (const Problem &other) const {
    return (actor == other.actor) && (message == other.message);
}

/************************************************************************/
/* WorkerInfo */
/************************************************************************/
WorkerInfo::WorkerInfo()
: ticks(0), timeMks(0)
{

}

/************************************************************************/
/* WorkerParamsInfo */
/************************************************************************/
WorkerParamsInfo::WorkerParamsInfo()
{

}

/************************************************************************/
/* MonitorUtils */
/************************************************************************/
QMap< QString, QList<FileInfo> > MonitorUtils::filesByActor(const WorkflowMonitor *m) {
    QMap< QString, QList<FileInfo> > result;
    foreach (const FileInfo &info, m->getOutputFiles()) {
        result[info.actor] << info;
    }
    return result;
}

} // Workflow
} // U2
