#ifndef _U2_FLOWTASK_H_
#define _U2_FLOWTASK_H_

#include <U2Core/Task.h>
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

class WorkflowRunInProcessMonitorTask;

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
    WorkflowRunInProcessMonitorTask * monitor;
    QMap<ActorId, ActorId> rmap;
    
}; // WorkflowIterationRunInProcessTask

class WorkflowRunInProcessMonitorTask : public Task {
    Q_OBJECT
public:
    WorkflowRunInProcessMonitorTask(const QString & schemaPath);
    ~WorkflowRunInProcessMonitorTask();
    
    virtual ReportResult report();
    WorkerState getState(const ActorId & id);
    int getMsgNum(const QString & ids);
    int getMsgPassed(const QString & ids);
    
private slots:
    void sl_onError(QProcess::ProcessError);
    void sl_onReadStandardOutput();
    
private:
    QString schemaPath;
    QProcess * proc;
    QMap<ActorId, WorkerState> states;
    QMap<QString, int> msgNums;
    QMap<QString, int> msgPassed;
    
}; // WorkflowRunInProcessMonitorTask

class U2LANG_EXPORT CheckCmdlineUgeneUtils {
public:
    static void setCmdlineUgenePath();
    
}; // CheckCmdlineUgeneTask

} //namespace

#endif
