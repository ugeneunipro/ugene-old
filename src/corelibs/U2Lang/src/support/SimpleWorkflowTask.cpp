/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "SimpleWorkflowTask.h"

#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2DbiUtils.h>
#include <U2core/U2OpStatusUtils.h>

#include <U2Formats/DocumentFormatUtils.h>

#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/WorkflowRunTask.h>


namespace U2 {

using namespace Workflow;

/***************************
 * WorkflowRunSchemaForTask
 ***************************/
static QString SCHEMA_DIR_PATH = QString("%1:cmdline/").arg(PATH_PREFIX_DATA);

static QString findWorkflowPath(const QString & schemaName) {
    foreach(const QString & ext, WorkflowUtils::WD_FILE_EXTENSIONS) {
        QString candidate = SCHEMA_DIR_PATH + schemaName + "." + ext;
        if (QFile::exists(candidate)) {
            return candidate;
        }
    }
    return QString();
}

/*static QString getFormatExt(const QString& docFormatId) {
    DocumentFormat* df = BaseDocumentFormats::get(docFormatId);
    SAFE_POINT(df, QString("Document format is not found: %1").arg(docFormatId) ,"");
    return df->getSupportedDocumentFileExtensions().first();
}*/


SimpleInOutWorkflowTask::SimpleInOutWorkflowTask(const SimpleInOutWorkflowTaskConfig& _conf)
: DocumentProviderTask(tr("Run workflow: %1").arg(_conf.schemaName), TaskFlags_NR_FOSCOE), conf(_conf)
{
    inDoc = new Document(BaseDocumentFormats::get(BaseDocumentFormats::FASTA), IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), 
                        GUrl("unused"), U2DbiRef(), conf.objects, conf.inDocHints);
    inDoc->setParent(this);
}

void SimpleInOutWorkflowTask::prepareTmpFile(QTemporaryFile& tmpFile, const QString& tmpl) {
    tmpFile.setFileTemplate(tmpl);
    if (!tmpFile.open()) {
        setError(tr("Cannot create temporary file for writing"));
        return;
    }
#ifdef _DEBUG
    tmpFile.setAutoRemove(false);
#endif
    tmpFile.close();
}

void SimpleInOutWorkflowTask::prepare() {
    prepareTmpFile(inputTmpFile, QString("%1/XXXXXX.%2").arg(QDir::tempPath()).arg(conf.inFormat));
    CHECK_OP(stateInfo, );
        
    prepareTmpFile(resultTmpFile, QString("%1/XXXXXX.%2").arg(QDir::tempPath()).arg(conf.outFormat));
    CHECK_OP(stateInfo, );

    schemaPath = findWorkflowPath(conf.schemaName);
    CHECK_EXT(!schemaPath.isEmpty(), setError(tr("Internal error: cannot find workflow %1").arg(conf.schemaName)), );
    
    saveInputTask = new SaveDocumentTask(inDoc, IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), inputTmpFile.fileName());
    addSubTask(saveInputTask);
}

QList<Task*> SimpleInOutWorkflowTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    CHECK_OP(stateInfo, res);
    
    if (subTask == saveInputTask) {
        // run workflow
        conf.extraArgs << "--in=" + inputTmpFile.fileName();        
        conf.extraArgs << "--out=" + resultTmpFile.fileName();
        conf.extraArgs << "--format=" + conf.outFormat;
        RunCmdlineWorkflowTaskConfig monitorConf(schemaPath, conf.extraArgs);
#ifdef _DEBUG
        monitorConf.logLevel2Commute = LogLevel_TRACE;
#else
        monitorConf.logLevel2Commute = LogLevel_DETAILS;
#endif 
        runWorkflowTask = new RunCmdlineWorkflowTask(monitorConf);
        res << runWorkflowTask;
    } else if (subTask == runWorkflowTask) {
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        ioLog.details(tr("Loading result file '%1'").arg(resultTmpFile.fileName()));
        loadResultTask = new LoadDocumentTask(conf.outFormat, resultTmpFile.fileName(), iof, conf.outDocHints);
        res << loadResultTask;
    } else {
        assert(subTask == loadResultTask);
        resultDocument = loadResultTask->takeDocument();
    }

    return res;
}

//////////////////////////////////////////////////////////////////////////
// RunSimpleMSAWorkflow4GObject
SimpleMSAWorkflow4GObjectTask::SimpleMSAWorkflow4GObjectTask(const QString& taskName, MAlignmentObject* _maObj, const SimpleMSAWorkflowTaskConfig& _conf) 
: Task(taskName, TaskFlags_NR_FOSCOE),
  conf(_conf),
  obj(_maObj),
  lock(NULL)
{
    SAFE_POINT(NULL != obj, "NULL MAlignmentObject!",);

    U2OpStatus2Log os;
    MAlignment al = obj->getMAlignment();
    U2EntityRef msaRef = MAlignmentImporter::createAlignment(obj->getEntityRef().dbiRef, al, os);
    SAFE_POINT_OP(os,);

    SimpleInOutWorkflowTaskConfig sioConf;
    sioConf.objects << new MAlignmentObject(al.getName(), msaRef);
    sioConf.inFormat = BaseDocumentFormats::FASTA;
    sioConf.outFormat = BaseDocumentFormats::FASTA;
    sioConf.outDocHints = conf.resultDocHints;
    sioConf.outDocHints[DocumentReadingMode_SequenceAsAlignmentHint] = true;
    sioConf.extraArgs = conf.schemaArgs;
    sioConf.schemaName = conf.schemaName;

    runWorkflowTask = new SimpleInOutWorkflowTask(sioConf);
    addSubTask(runWorkflowTask);

    setUseDescriptionFromSubtask(true);
    setVerboseLogMode(true);
    docName = obj->getDocument()->getName();
}

SimpleMSAWorkflow4GObjectTask::~SimpleMSAWorkflow4GObjectTask() {
    SAFE_POINT(lock == NULL, "Lock is not deallocated!",);
}

void SimpleMSAWorkflow4GObjectTask::prepare() {
    CHECK_EXT(!obj.isNull(), setError(tr("Object '%1' removed").arg(docName)), );

    lock = new StateLock(getTaskName(), StateLockFlag_LiveLock);
    obj->lockState(lock);
}


Task::ReportResult SimpleMSAWorkflow4GObjectTask::report() {
    if (lock != NULL) {
        if (!obj.isNull()) {
            obj->unlockState(lock);
        }
        delete lock;
        lock = NULL;
    }
    CHECK_OP(stateInfo, ReportResult_Finished);
    CHECK_EXT(!obj.isNull(), setError(tr("Object '%1' removed").arg(docName)), ReportResult_Finished);
    CHECK_EXT(!obj->isStateLocked(), setError(tr("Object '%1' is locked").arg(docName)), ReportResult_Finished);

    obj->setMAlignment(getResult());
    return ReportResult_Finished;
}

MAlignment SimpleMSAWorkflow4GObjectTask::getResult() {
    MAlignment res;
    CHECK_OP(stateInfo, res);

    Document* d = runWorkflowTask->getDocument();
    CHECK_EXT(d!=NULL, setError(tr("Result document not found!")), res);
    CHECK_EXT(d->getObjects().size() == 1, setError(tr("Result document content not matched! %1").arg(d->getURLString())), res);
    MAlignmentObject* maObj = qobject_cast<MAlignmentObject*>(d->getObjects().first());
    CHECK_EXT(maObj!=NULL, setError(tr("Result document contains no MSA! %1").arg(d->getURLString())), res);
    return maObj->getMAlignment();
}


}    // namespace U2
