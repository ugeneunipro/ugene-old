#ifndef _U2_WORKFLOW_CMDLINE_TASKS_H_
#define _U2_WORKFLOW_CMDLINE_TASKS_H_

#include <U2Core/Task.h>
#include <U2Remote/RemoteMachine.h>
#include <U2Lang/WorkflowIOTasks.h>

namespace U2 {

class WorkflowRunFromCMDLineBase : public Task {
    Q_OBJECT
public:
    WorkflowRunFromCMDLineBase();
    virtual ~WorkflowRunFromCMDLineBase();
    QList<Task*> onSubTaskFinished( Task* subTask );
    
protected:
    virtual Task * getWorkflowRunTask() const = 0;
    
private:
    LoadWorkflowTask * prepareLoadSchemaTask( const QString & schemaName );
    void processLoadSchemaTask( const QString & schemaName, int optionIdx );
    
protected:
    Schema*             schema;
    int                 optionsStartAt;
    LoadWorkflowTask *  loadTask;
    QString             schemaName;
    QMap<ActorId, ActorId> remapping;
    
}; // WorkflowRunFromCMDLineBase

class WorkflowRunFromCMDLineTask : public WorkflowRunFromCMDLineBase {
    Q_OBJECT
public:
    virtual Task * getWorkflowRunTask() const;
}; // WorkflowRunFromCMDLineTask

class WorkflowRemoteRunFromCMDLineTask : public WorkflowRunFromCMDLineBase {
    Q_OBJECT
public:
    WorkflowRemoteRunFromCMDLineTask();
    virtual Task * getWorkflowRunTask() const;
    
private:
    RemoteMachineSettings * settings;
    
}; // WorkflowRemoteRunFromCMDLineTask

} // U2

#endif // _U2_WORKFLOW_CMDLINE_TASKS_H_
