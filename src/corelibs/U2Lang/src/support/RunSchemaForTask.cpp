#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
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

WorkflowRunSchemaForTask::WorkflowRunSchemaForTask(const QString & scName, WorkflowRunSchemaForTaskCallback * c)
: Task(tr("Run workflow schema for task %1").arg(scName), TaskFlag_NoRun), callback(c), 
  loadSchemaTask(NULL), inputDocument(NULL), saveInputTask(NULL), runSchemaTask(NULL), loadResultTask(NULL), schemaName(scName) {
    assert(callback != NULL);
    
    DocumentFormat * inputDf = AppContext::getDocumentFormatRegistry()->getFormatById(callback->getInputFileFormat());
    assert(inputDf != NULL);
    saveInputTmpFile.setFileTemplate(QString("%1/XXXXXX.%2").arg(QDir::tempPath()).arg(inputDf->getSupportedDocumentFileExtensions().first()));
    if(!saveInputTmpFile.open()) {
        setError(tr("Cannot create temporary file for writing"));
        return;
    }
    saveInputTmpFilename = saveInputTmpFile.fileName();
    saveInputTmpFile.close();
    
    DocumentFormat * outputDf = AppContext::getDocumentFormatRegistry()->getFormatById(callback->getOutputFileFormat());
    assert(outputDf != NULL);
    resultTmpFile.setFileTemplate(QString("%1/XXXXXX.%2").arg(QDir::tempPath()).arg(outputDf->getSupportedDocumentFileExtensions().first()));
    if(!resultTmpFile.open()) {
        setError(tr("Cannot create temporary file for reading and writing"));
        return;
    }
    resultTmpFilename = resultTmpFile.fileName();
    resultTmpFile.close();
    
    QString schemaPath = findSchemaPath(schemaName);
    if(schemaPath.isEmpty()) {
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
    data[INPUT_FILE_ALIAS] = qVariantFromValue(saveInputTmpFilename);
    data[OUTPUT_FILE_ALIAS] = qVariantFromValue(resultTmpFilename);
    data[OUTPUT_FILE_FORMAT_ALIAS] = qVariantFromValue(callback->getOutputFileFormat());
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
    if(hasErrors() || isCanceled()) {
        return res;
    }
    
    if(loadSchemaTask == subTask) {
        assert(inputDocument == NULL);
        assert(!saveInputTmpFilename.isEmpty());
        IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(saveInputTmpFilename));
        assert(iof != NULL);
        DocumentFormat * df = AppContext::getDocumentFormatRegistry()->getFormatById(callback->getInputFileFormat());
        assert(df != NULL);
        inputDocument = new Document(df, iof, saveInputTmpFilename, callback->createInputData());
        saveInputTask = new SaveDocumentTask(inputDocument);
        res << saveInputTask;
    } else if(saveInputTask == subTask) {
        setSchemaSettings();
        if(hasErrors()) {
            return res;
        }
        runSchemaTask = new WorkflowRunInProcessTask(schema, schema.getIterations());
        res << runSchemaTask;
    } else if(runSchemaTask == subTask) {
        IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(resultTmpFilename));
        assert(iof != NULL);
        loadResultTask = new LoadDocumentTask(callback->getOutputFileFormat(), resultTmpFilename, iof);
        res << loadResultTask;
    }
    return res;
}

Document * WorkflowRunSchemaForTask::getResult() {
    if(loadResultTask != NULL) {
        return loadResultTask->getDocument();
    } else {
        assert(false);
        return NULL;
    }
}

}    // namespace U2
