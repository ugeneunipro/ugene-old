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
