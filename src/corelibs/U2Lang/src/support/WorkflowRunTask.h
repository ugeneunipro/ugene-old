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

#ifndef _U2_FLOWTASK_H_
#define _U2_FLOWTASK_H_

#include <U2Core/Task.h>

#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/DbiDataStorage.h>
#include <U2Lang/Schema.h>
#include <U2Lang/WorkflowManager.h>
#include <U2Lang/WorkflowIOTasks.h>
#include <U2Lang/WorkflowDebugStatus.h>

#include <QtCore/QUrl>
#include <QtCore/QTemporaryFile>
#include <QtCore/QProcess>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

namespace U2 {

namespace Workflow {
    class CommunicationChannel;
    class WorkflowMonitor;
}
using namespace Workflow;

class U2LANG_EXPORT WorkflowAbstractRunner : public Task {
    Q_OBJECT
public:
    WorkflowAbstractRunner(const QString &name, TaskFlags flags);
    virtual QList<WorkerState> getState(Actor*) = 0;
    virtual int getMsgNum(const Link*) = 0;
    virtual int getMsgPassed(const Link*) = 0;

    const QList<WorkflowMonitor*> & getMonitors() const;

protected:
    QList<WorkflowMonitor*> monitors;
}; // WorkflowAbstractRunner

class U2LANG_EXPORT WorkflowAbstractIterationRunner : public Task {
    Q_OBJECT
public:
    WorkflowAbstractIterationRunner(const QString &name, TaskFlags flags);
    virtual ~WorkflowAbstractIterationRunner() {}
    virtual WorkerState getState(const ActorId &actor) = 0;
    virtual int getMsgNum(const Link *l) = 0;
    virtual int getMsgPassed(const Link *l) = 0;

    virtual int getDataProduced(const ActorId &actor) = 0;

signals:
    void si_updateProducers();
};


typedef QMap<ActorId,ActorId> ActorMap;

class U2LANG_EXPORT WorkflowRunTask : public WorkflowAbstractRunner {
    Q_OBJECT
public:
    WorkflowRunTask(const Schema&, const ActorMap& rmap = ActorMap(),
        WorkflowDebugStatus *debugInfo = new WorkflowDebugStatus());
    virtual ReportResult report(); 
    virtual QList<WorkerState> getState(Actor*);
    virtual int getMsgNum(const Link*);
    virtual int getMsgPassed(const Link*);

signals:
    void si_ticked();

private slots:
    void sl_outputProgressAndState();
    
private:
    QMap<ActorId, ActorId> rmap;
    QList<Link*> flows;
    
}; // WorkflowRunTask

class WorkflowIterationRunTask : public WorkflowAbstractIterationRunner {
    Q_OBJECT
public:
    WorkflowIterationRunTask(const Schema&, WorkflowDebugStatus *initDebugInfo);
    ~WorkflowIterationRunTask();
    virtual void prepare();
    virtual ReportResult report();

    virtual WorkerState getState(const ActorId &actor);
    virtual int getMsgNum(const Link *l);
    virtual int getMsgPassed(const Link *l);
    virtual int getDataProduced(const ActorId &actor);

    WorkflowMonitor * getMonitor() const;

signals:
    void si_ticked();

protected:
    virtual QList<Task*> onSubTaskFinished(Task* subTask);

private slots:
    void sl_pauseStateChanged(bool isPaused);
    void sl_busInvestigationIsRequested(const Workflow::Link *bus, int messageNumber);
    void sl_busCountOfMessagesRequested(const Workflow::Link *bus);
    void sl_singleStepIsRequested(const ActorId &actor);
    void sl_convertMessages2Documents(const Workflow::Link *bus, const QString &messageType,
        int messageNumber, const QString &schemeName);

private:
    static TaskFlags getAdditionalFlags();

    QList<CommunicationChannel*> getActorLinks(const QString &actor);

    WorkflowContext *context;
    Schema* schema;
    Scheduler* scheduler;
    QMap<ActorId, ActorId> rmap;
    QMap<QString, CommunicationChannel*> lmap;
    
    WorkflowDebugStatus *debugInfo;
    bool isNextTickRestoring;
};

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
    void writeLog(QStringList &lines);
    QStringList getActorLinks(const ActorId &id);

signals:
    void si_logRead();

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

private:
    QString readStdout();
}; // RunCmdlineWorkflowTask


} //namespace U2

#endif
