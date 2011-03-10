#ifndef _U2_REMOTE_WORKFLOW_RUN_TASK_H_
#define _U2_REMOTE_WORKFLOW_RUN_TASK_H_

#include <U2Core/VirtualFileSystem.h>
#include <U2Remote/RemoteMachine.h>

#include <U2Lang/Schema.h>
#include <U2Lang/RunSchemaForTask.h>

#include <QtCore/QEventLoop>


namespace U2 {

using namespace Workflow;

class U2REMOTE_EXPORT RemoteWorkflowRunTask : public Task {
    Q_OBJECT
public:
    RemoteWorkflowRunTask( RemoteMachineSettings *m, const Schema & sc, const QList<Iteration> & its );
    RemoteWorkflowRunTask( RemoteMachineSettings *m, qint64 remoteTaskId);
    ~RemoteWorkflowRunTask();

    static const int TIMER_UPDATE_TIME = 2000; /* 2 seconds */
    static const int REMOTE_TASK_TIMEOUT = 30000;

    virtual void prepare();
    virtual void run();
    virtual ReportResult report();
    qint64 getRemoteTaskId() { return taskId; }

private:
    void preprocessSchema();
    static void dumpSchema(const QString& fileName, const QByteArray& schema);

private slots:
    void sl_remoteTaskTimerUpdate();

private:
    RemoteMachineSettings*  machineSettings;
    RemoteMachine *         machine;
    Schema                  schema;
    QList<Iteration>        iterations;
    qint64                  taskId;
    QEventLoop*             eventLoop;
    QStringList             outputUrls;
    QVariantMap             taskSettings;
    // Task is running or enqueued on remote machine already
    bool                    taskIsActive;

}; // RemoteWorkflowRunTask

} // U2

#endif // _U2_REMOTE_WORKFLOW_RUN_TASK_H_
