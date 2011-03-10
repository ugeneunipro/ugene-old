#ifndef _RUN_SCHEMA_FOR_TASK_H_
#define _RUN_SCHEMA_FOR_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Lang/Schema.h>
#include <U2Lang/WorkflowIOTasks.h>
#include <U2Lang/WorkflowRunTask.h>

namespace U2
{

using namespace Workflow;

class U2LANG_EXPORT WorkflowRunSchemaForTaskCallback {
public:
    virtual QList<GObject*> createInputData() const = 0; // for saving input data (sequence, msa ...)
    virtual DocumentFormatId getInputFileFormat() const = 0; // format of input file to save
    virtual QVariantMap getSchemaData() const = 0; // pairs (alias, value)
    virtual DocumentFormatId getOutputFileFormat() const = 0; // will set --format alias
    
}; // WorkflowRunSchemaForTaskCallback

class U2LANG_EXPORT WorkflowRunSchemaForTask : public Task {
    Q_OBJECT
public:
    WorkflowRunSchemaForTask(const QString & schemaName, WorkflowRunSchemaForTaskCallback * callback);
    ~WorkflowRunSchemaForTask();
    
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    Document * getResult();
    
private:
    void setSchemaSettings();
    void setSchemaSettings(const QVariantMap & data);
    
private:
    WorkflowRunSchemaForTaskCallback * callback;
    LoadWorkflowTask * loadSchemaTask;
    Schema schema;
    Document * inputDocument;
    SaveDocumentTask * saveInputTask;
    QTemporaryFile saveInputTmpFile;
    QString saveInputTmpFilename;
    WorkflowRunInProcessTask * runSchemaTask;
    QTemporaryFile resultTmpFile;
    QString resultTmpFilename;
    LoadDocumentTask * loadResultTask;
    QString schemaName;
    
}; // WorkflowRunSchemaForTask

}    // namespace U2

#endif    // #ifndef _RUN_SCHEMA_FOR_TASK_H_
