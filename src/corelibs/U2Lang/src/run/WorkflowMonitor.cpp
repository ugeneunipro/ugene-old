/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <U2Core/U2OpStatusUtils.h>

#include <U2Lang/ActorModel.h>
#include <U2Lang/HRSchemaSerializer.h>
#include <U2Lang/WorkflowRunTask.h>
#include <U2Lang/WorkflowUtils.h>

#include "WorkflowMonitor.h"

namespace U2 {
namespace Workflow {
using namespace Monitor;

WorkerLogInfo::~WorkerLogInfo() {
    foreach(ExternalToolListener* listener, logs) {
        if(NULL != listener) {
            delete listener;
        }
    }
}

const QString WorkflowMonitor::WORKFLOW_FILE_NAME("workflow.uwl");

WorkflowMonitor::WorkflowMonitor(WorkflowAbstractIterationRunner *_task, Schema *_schema)
: QObject(), schema(_schema), task(_task), saveSchema(false), started(false)
{
    foreach (Actor *p, schema->getProcesses()) {
        procMap[p->getId()] = p;
        addTime(0, p->getId());
    }

    foreach (Actor *p, schema->getProcesses()){
        WorkerParamsInfo info;
        info.actor = p;
        info.workerName = p->getLabel();
        QMap<QString, Attribute *> params = p->getParameters();
        QMapIterator<QString, Attribute *> paramsIter(params);
        while (paramsIter.hasNext()) {
            paramsIter.next();
            Attribute *attr = paramsIter.value();
            SAFE_POINT(NULL != attr, "NULL attribute in params!", );

            info.parameters << attr;
        }
        workersParamsInfo << info;
    }

    connect(task.data(), SIGNAL(si_updateProducers()), SIGNAL(si_updateProducers()));
    connect(task.data(), SIGNAL(si_progressChanged()), SLOT(sl_progressChanged()));
    connect(task.data(), SIGNAL(si_stateChanged()), SLOT(sl_taskStateChanged()));
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

const QMap<QString, Monitor::WorkerLogInfo> & WorkflowMonitor::getWorkersLog() const {
    return workersLog;
}

QString WorkflowMonitor::actorName(const QString &id) const {
    SAFE_POINT(procMap.contains(id), QString("Unknown actor id %1").arg(id), "");
    return procMap[id]->getLabel();
}

void WorkflowMonitor::addOutputFile(const QString &url, const QString &producer, bool openBySystem) {
    CHECK(!url.isEmpty(), );
    FileInfo info(url, producer, openBySystem);

    CHECK(!outputFiles.contains(info), );

    outputFiles << info;
    emit si_newOutputFile(info);
}

void WorkflowMonitor::addError(const QString &message, const QString &actor, const QString &type) {
    addProblem(Problem(message, actor, type));
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
    emit si_workerInfoChanged(actor, info);
}

void WorkflowMonitor::addTick(qint64 timeMks, const QString &actor) {
    workers[actor].ticks += 1;
    addTime(timeMks, actor);
}

void WorkflowMonitor::start() {
    SAFE_POINT(!started, "The workflow is already started", );
    started = true;
    setRunState(false);
}

void WorkflowMonitor::pause() {
    SAFE_POINT(started, "The workflow is not started yet", );
    setRunState(true);
}

void WorkflowMonitor::resume() {
    SAFE_POINT(started, "The workflow is not started yet", );
    setRunState(false);
}

void WorkflowMonitor::registerTask(Task *task, const QString &actor) {
    SAFE_POINT(procMap.contains(actor), "Unknown actor id", );
    taskMap[task] = procMap[actor];
}

void WorkflowMonitor::setOutputDir(const QString &dir) {
    _outputDir = dir;
    emit si_dirSet(outputDir());

    if (saveSchema) {
        QString url = outputDir() + "report/" + WORKFLOW_FILE_NAME;
        U2OpStatus2Log os;
        HRSchemaSerializer::saveSchema(schema, meta.data(), url, os);
    }
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
        } else if (task->hasError()) {
            state = FAILED;
        } else if (!problems.isEmpty()) {
            if (!hasErrors()) {
                state = FINISHED_WITH_PROBLEMS;
            } else {
                state = FAILED;
            }
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
    emit si_runStateChanged(paused);
}

int WorkflowMonitor::getDataProduced(const QString &actor) const {
    CHECK(!task.isNull(), 0);
    return task->getDataProduced(actor);
}

void WorkflowMonitor::addProblem(const Problem &problem) {
    if (problems.isEmpty()) {
        emit si_firstProblem();
        emit si_taskStateChanged(RUNNING_WITH_PROBLEMS);
    }
    problems << problem;
    emit si_newProblem(problem);
}

bool WorkflowMonitor::hasErrors() const {
    foreach (Problem problem, problems) {
        CHECK(problem.type != Problem::U2_ERROR, true);
    }
    return false;
}

void WorkflowMonitor::setSaveSchema(const Metadata &_meta) {
    meta.reset(new Metadata(_meta));
    saveSchema = true;
}

QList<ExternalToolListener*> WorkflowMonitor::createWorkflowListeners(const QString& workerName, int listenersNumber) {
    QList<ExternalToolListener*> listeners;
    WorkerLogInfo& logInfo = workersLog[workerName];
    logInfo.runNumber++;
    for(int i = 0; i < listenersNumber; i++) {
        WDListener* newListener = new WDListener(this, workerName, logInfo.runNumber);
        listeners.append(newListener);
    }
    logInfo.logs.append(listeners);
    return listeners;
}
void WorkflowMonitor::onLogChanged(const WDListener* listener, int messageType, const QString& message) {
    U2::Workflow::Monitor::LogEntry entry;
    entry.toolName = listener->getToolName();
    entry.actorName = listener->getActorName();
    entry.runNumber = listener->getRunNumber();
    entry.logType = messageType;
    entry.lastLine = message;
    emit si_logChanged(entry);
}

/************************************************************************/
/* FileInfo */
/************************************************************************/
FileInfo::FileInfo( )
    : url( ), actor( ), openBySystem(false)
{

}

FileInfo::FileInfo(const QString &_url, const QString &_producer, bool _openBySystem)
: url(_url), actor(_producer), openBySystem(_openBySystem)
{

}

bool FileInfo::operator== (const FileInfo &other) const {
    return url == other.url;
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

QStringList MonitorUtils::sortedByAppearanceActorIds(const WorkflowMonitor *m) {
    QStringList result;
    foreach (const FileInfo &info, m->getOutputFiles()) {
        if (!result.contains(info.actor)) {
            result << info.actor;
        }
    }
    return result;
}

/************************************************************************/
/* WDListener */
/************************************************************************/
void WDListener::addNewLogMessage(const QString& message, int messageType) {
    monitor->onLogChanged(this, messageType, message);
}

} // Workflow
} // U2
