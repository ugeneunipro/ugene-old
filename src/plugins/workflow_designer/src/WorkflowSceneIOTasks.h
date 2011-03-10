#ifndef _U2_WORKFLOW_SCENE_IO_TASK_H_
#define _U2_WORKFLOW_SCENE_IO_TASK_H_

#include <QtCore/QPointer>

#include <U2Core/Task.h>
#include <U2Lang/Schema.h>
#include <U2Lang/WorkflowIOTasks.h>

namespace U2 {

class WorkflowScene;
using namespace Workflow;

class SaveWorkflowSceneTask : public Task {
    Q_OBJECT
public:
    static const QString SCHEMA_PATHS_SETTINGS_TAG;
    
public:
    SaveWorkflowSceneTask(WorkflowScene* scene, const Metadata& meta);
    Task::ReportResult report();
    virtual void run();
    
private:
    QPointer<WorkflowScene> scene;
    Metadata meta;
    QString rawData;
};

class LoadWorkflowSceneTask : public Task {
    Q_OBJECT
public:
    LoadWorkflowSceneTask(WorkflowScene* scene, Metadata* meta, const QString& url);
    virtual void run();
    virtual Task::ReportResult report();
    
private:
    WorkflowScene* scene;
    Metadata * meta;
    QString url;
    QString rawData;
    LoadWorkflowTask::FileFormat format;
};

} //namespace
#endif
