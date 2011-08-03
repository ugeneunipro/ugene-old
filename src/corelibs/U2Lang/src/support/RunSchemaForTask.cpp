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

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/WorkflowRunTask.h>
#include "RunSchemaForTask.h"

namespace U2 {

using namespace Workflow;

/***************************
 * WorkflowRunSchemaForTask
 ***************************/
static QString SCHEMA_DIR_PATH = QString("%1:schemas_private/").arg(PATH_PREFIX_DATA);

static QString findSchemaPath(const QString & schemaName) {
    foreach(const QString & ext, WorkflowUtils::WD_FILE_EXTENSIONS) {
        QString candidate = SCHEMA_DIR_PATH + schemaName + "." + ext;
        if(QFile::exists(candidate)) {
            return candidate;
        }
    }
    return QString();
}

WorkflowRunSchemaForTask::WorkflowRunSchemaForTask(const QString & scName, WorkflowRunSchemaForTaskCallback * c, const QVariantMap& _resultDocHints)
: DocumentProviderTask(tr("Run workflow schema for task %1").arg(scName), TaskFlag_NoRun), callback(c), 
  loadSchemaTask(NULL), inputDocument(NULL), saveInputTask(NULL), runSchemaTask(NULL), loadResultTask(NULL), schemaName(scName) 
{
    assert(callback != NULL);
    resultDocHints = _resultDocHints;
    if (callback->saveInput()) {
        DocumentFormat * inputDf = AppContext::getDocumentFormatRegistry()->getFormatById(callback->inputFileFormat());
        assert(inputDf != NULL);
        saveInputTmpFile.setFileTemplate(QString("%1/XXXXXX.%2").arg(QDir::tempPath()).arg(inputDf->getSupportedDocumentFileExtensions().first()));
        if(!saveInputTmpFile.open()) {
            setError(tr("Cannot create temporary file for writing"));
            return;
        }
        saveInputTmpFilename = saveInputTmpFile.fileName();
        saveInputTmpFile.close();
    }
    
    if (callback->saveOutput()) {
        DocumentFormat * outputDf = AppContext::getDocumentFormatRegistry()->getFormatById(callback->outputFileFormat());
        assert(outputDf != NULL);
        resultTmpFile.setFileTemplate(QString("%1/XXXXXX.%2").arg(QDir::tempPath()).arg(outputDf->getSupportedDocumentFileExtensions().first()));
        if(!resultTmpFile.open()) {
            setError(tr("Cannot create temporary file for reading and writing"));
            return;
        }
        resultTmpFilename = resultTmpFile.fileName();
        resultTmpFile.close();
    }
    
    QString schemaPath = findSchemaPath(schemaName);
    if (schemaPath.isEmpty()) {
        assert(false);
        setError(tr("Internal error: cannot find schema %1").arg(schemaName));
        return;
    }
    
    loadSchemaTask = new LoadWorkflowTask(&schema, NULL, schemaPath);
    addSubTask(loadSchemaTask);
}

WorkflowRunSchemaForTask::~WorkflowRunSchemaForTask() {
    delete inputDocument;
}

static const QString INPUT_FILE_ALIAS("in");
static const QString OUTPUT_FILE_ALIAS("out");
static const QString OUTPUT_FILE_FORMAT_ALIAS("format");

void WorkflowRunSchemaForTask::setSchemaSettings() {
    QVariantMap data(callback->getSchemaData());
    if(callback->saveInput()) {
        data[INPUT_FILE_ALIAS] = qVariantFromValue(saveInputTmpFilename);
    }
    if(callback->saveOutput()) {
        data[OUTPUT_FILE_ALIAS] = qVariantFromValue(resultTmpFilename);
        data[OUTPUT_FILE_FORMAT_ALIAS] = qVariantFromValue(callback->outputFileFormat());    
    }
    setSchemaSettings(data);
}

void WorkflowRunSchemaForTask::setSchemaSettings(const QVariantMap & data) {
    foreach(const QString & key, data.keys()) {
        QString attrName;
        Actor * actor = WorkflowUtils::findActorByParamAlias(schema.getProcesses(), key, attrName, false);
        if(actor == NULL) {
            assert(false);
            setError(tr("Internal error: cannot find alias '%1' for schema '%2'").arg(key).arg(schemaName));
            return;
        }
        Attribute * attr = actor->getParameter(attrName);
        if(attr == NULL) {
            assert(false);
            setError(tr("Internal error: cannot find attribute '%1' for schema '%2'").arg(attrName).arg(schemaName));
        }
        attr->setAttributeValue(data.value(key));
    }
}

QList<Task*> WorkflowRunSchemaForTask::onSubTaskFinished(Task* subTask) {
    propagateSubtaskError();
    QList<Task*> res;
    if(hasError() || isCanceled()) {
        return res;
    }
    
    enum WhatNext {
        SAVE_INPUT,
        RUN_SCHEMA,
        LOAD_RESULT,
        NOTHING
    };
    
    WhatNext what = NOTHING;
    if(loadSchemaTask == subTask) {
        if(callback->saveInput()) {
            what = SAVE_INPUT;
        } else {
            assert(callback->inputFileFormat().isEmpty());
            assert(callback->createInputData().isEmpty());
            what = RUN_SCHEMA;
        }
    } else if(saveInputTask == subTask) {
        what = RUN_SCHEMA;
    } else if(runSchemaTask == subTask && callback->saveOutput()) { // result saved to temp file: load result from it
        what = LOAD_RESULT;
    }
    
    switch(what) {
    case SAVE_INPUT:
        {
            assert(inputDocument == NULL);
            assert(!saveInputTmpFilename.isEmpty());
            IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(saveInputTmpFilename));
            assert(iof != NULL);
            DocumentFormat * df = AppContext::getDocumentFormatRegistry()->getFormatById(callback->inputFileFormat());
            assert(df != NULL);
            inputDocument = new Document(df, iof, saveInputTmpFilename, callback->createInputData());
            saveInputTask = new SaveDocumentTask(inputDocument);
            res << saveInputTask;
        }
        break;
    case RUN_SCHEMA:
        {
            setSchemaSettings();
            if(hasError()) {
                return res;
            }
            runSchemaTask = new WorkflowRunInProcessTask(schema, schema.getIterations());
            res << runSchemaTask;
        }
        break;
    case LOAD_RESULT:
        {
            IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(resultTmpFilename));
            assert(iof != NULL);
            loadResultTask = new LoadDocumentTask(callback->outputFileFormat(), resultTmpFilename, iof, resultDocHints);
            res << loadResultTask;
        }
        break;
    case NOTHING:
        if (loadResultTask != NULL && loadResultTask->getDocument() != NULL) {
            Document* doc = loadResultTask->getDocument();
            resultDocument = doc->clone();
        }
        break;
    default:
        assert(false);
    }
    return res;
}

Task::ReportResult WorkflowRunSchemaForTask::report() {
    propagateSubtaskError();
    return ReportResult_Finished;
}

}    // namespace U2
