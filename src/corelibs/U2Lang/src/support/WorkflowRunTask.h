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

#ifndef _U2_FLOWTASK_H_
#define _U2_FLOWTASK_H_

#include <U2Core/Task.h>
#include <U2Lang/DbiDataStorage.h>
#include <U2Lang/Schema.h>
#include <U2Lang/WorkflowManager.h>
#include <U2Lang/WorkflowIOTasks.h>
#include <U2Lang/CoreLibConstants.h>

#include <QtCore/QUrl>
#include <QtCore/QTemporaryFile>
#include <QtCore/QProcess>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

namespace U2 {

namespace Workflow {
    class CommunicationChannel;
}
using namespace Workflow;

class U2LANG_EXPORT WorkflowAbstractRunner : public Task {
    Q_OBJECT
public:
    WorkflowAbstractRunner(const QString& n, TaskFlags f) : Task(n, f) {}
    virtual QList<WorkerState> getState(Actor*) = 0;
    virtual int getMsgNum(Link*) = 0;
    virtual int getMsgPassed(Link*) = 0;
}; // WorkflowAbstractRunner


typedef QMap<ActorId,ActorId> ActorMap;

class U2LANG_EXPORT WorkflowRunTask : public WorkflowAbstractRunner {
    Q_OBJECT
public:
    WorkflowRunTask(const Schema&, QList<Iteration>, const ActorMap& rmap = ActorMap());
    virtual QString generateReport() const;
    virtual ReportResult report(); 
    virtual QList<WorkerState> getState(Actor*);
    virtual int getMsgNum(Link*);
    virtual int getMsgPassed(Link*);
    
signals:
    void si_ticked();

private slots:
    void sl_outputProgressAndState();
    
private:
    QMap<ActorId, ActorId> rmap;
    QList<Link*> flows;
    
}; // WorkflowRunTask

class WorkflowIterationRunTask : public Task {
    Q_OBJECT
public:
    WorkflowIterationRunTask(const Schema&, const Iteration&);
    ~WorkflowIterationRunTask();
    virtual void prepare();
    virtual ReportResult report();

signals:
    void si_ticked();
public:
    WorkerState getState(const ActorId& id);
    int getMsgNum(Link*);
    int getMsgPassed(Link*);
    QStringList getFiles() const;

protected:
    virtual QList<Task*> onSubTaskFinished(Task* subTask);

private:
    WorkflowContext *context;
    Schema* schema;
    Scheduler* scheduler;
    QMap<ActorId, ActorId> rmap;
    QMap<QString, CommunicationChannel*> lmap;
    QStringList fileLinks;
};

class U2LANG_EXPORT WorkflowRunInProcessTask : public WorkflowAbstractRunner {
    Q_OBJECT
public:
    WorkflowRunInProcessTask(const Schema & sc, const QList<Iteration> & its);
    virtual QString generateReport() const;
    virtual ReportResult report();
    virtual QList<WorkerState> getState(Actor*);
    virtual int getMsgNum(Link*);
    virtual int getMsgPassed(Link*);
    
signals:
    void si_ticked();
    
}; // WorkflowRunInProcessTask

class RunCmdlineWorkflowTask;

class WorkflowIterationRunInProcessTask : public Task {
    Q_OBJECT
public:
    WorkflowIterationRunInProcessTask(const Schema & sc, const Iteration & it);
    ~WorkflowIterationRunInProcessTask();
    
    virtual ReportResult report();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    
    WorkerState getState(Actor*);
    int getMsgNum(Link * l);
    int getMsgPassed(Link * l);
    QStringList getFiles();
    
private:
    Schema* schema;
    QTemporaryFile tempFile;
    SaveWorkflowTask * saveSchemaTask;
    RunCmdlineWorkflowTask * monitor;
    QMap<ActorId, ActorId> rmap;
    
}; // WorkflowIterationRunInProcessTask

class RunCmdlineWorkflowTaskConfig {
public:
    RunCmdlineWorkflowTaskConfig(const QString& _schemaPath = QString(), const QStringList& _args = QStringList()) 
        : schemaPath(_schemaPath), args(_args), logLevel2Commute(LogLevel_TRACE) {}

    QString         schemaPath;
    QStringList     args;
    LogLevel        logLevel2Commute;
};

class RunCmdlineWorkflowTask : public Task {
    Q_OBJECT
public:
    RunCmdlineWorkflowTask(const RunCmdlineWorkflowTaskConfig& conf);
    
    void prepare();
    virtual ReportResult report();
    WorkerState getState(const ActorId & id);
    int getMsgNum(const QString & ids);
    int getMsgPassed(const QString & ids);
    void writeLog(const QString& message);
    
private slots:
    void sl_onError(QProcess::ProcessError);
    void sl_onReadStandardOutput();
    
private:
    RunCmdlineWorkflowTaskConfig   conf;
    QProcess*                               proc;
    QMap<ActorId, WorkerState>              states;
    QMap<QString, int>                      msgNums;
    QMap<QString, int>                      msgPassed;
    QString                                 processLogPrefix;
    
}; // WorkflowRunInProcessMonitorTask


} //namespace

#endif
