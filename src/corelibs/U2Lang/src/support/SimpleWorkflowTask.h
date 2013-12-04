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


/** Set of wrappers for typical workflow tasks */

#ifndef _SIMPLE_WORKFLOW_TASK_H_
#define _SIMPLE_WORKFLOW_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/MAlignment.h>

#include <U2Lang/Schema.h>
#include <U2Lang/WorkflowIOTasks.h>
#include <U2Lang/WorkflowRunTask.h>

namespace U2 {

using namespace Workflow;

class MAlignmentObject;

class U2LANG_EXPORT SimpleInOutWorkflowTaskConfig {
public:
    QList<GObject*>     objects;
    DocumentFormatId    inFormat;
    QVariantMap         inDocHints;
    DocumentFormatId    outFormat;
    QVariantMap         outDocHints;
    QStringList         extraArgs;
    QString             schemaName;
};

/** 
    Runs workflow in a separate process and handles in-out parameters 
    The result is output document
*/
class U2LANG_EXPORT SimpleInOutWorkflowTask : public DocumentProviderTask {
    Q_OBJECT
public:
    SimpleInOutWorkflowTask(const SimpleInOutWorkflowTaskConfig& conf);
    void prepare();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    
private:
    void prepareTmpFile(QTemporaryFile& tmpFile, const QString& tmpl);

    SimpleInOutWorkflowTaskConfig        conf;
    
    Document*                           inDoc;
    
    SaveDocumentTask*                   saveInputTask;
    QTemporaryFile                      inputTmpFile;

    RunCmdlineWorkflowTask*             runWorkflowTask;

    QTemporaryFile                      resultTmpFile;
    LoadDocumentTask*                   loadResultTask;

    QString                             schemaPath;

};


class SimpleMSAWorkflowTaskConfig {
public:
    QString     schemaName;
    QStringList schemaArgs;
    QString     algoName;
    QVariantMap resultDocHints;
};

class U2LANG_EXPORT SimpleMSAWorkflow4GObjectTask : public Task {
    Q_OBJECT

public:
    SimpleMSAWorkflow4GObjectTask(const QString& taskName, MAlignmentObject* maObj, const SimpleMSAWorkflowTaskConfig& conf);
    ~SimpleMSAWorkflow4GObjectTask();
    
    void prepare();
    ReportResult report();
    MAlignment getResult();

private:
    QPointer<MAlignmentObject>  obj;
    StateLock*                  lock;
    QString                     docName;
    SimpleMSAWorkflowTaskConfig conf;
    SimpleInOutWorkflowTask*    runWorkflowTask;
    U2UseCommonUserModStep      *userModStep;
};

}    // namespace U2

#endif    // #ifndef _RUN_SCHEMA_FOR_TASK_H_
