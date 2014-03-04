/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <memory>

#include "KalignTask.h"
#include "KalignAdapter.h"
#include "KalignConstants.h"

extern "C" {
#include "kalign2/kalign2_context.h"
}

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/StateLockableDataModel.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/Counter.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2Mod.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/DNAAlphabet.h>

#include <U2Lang/WorkflowSettings.h>
#include <U2Lang/SimpleWorkflowTask.h>

#include <U2Gui/OpenViewTask.h>

extern "C" kalign_context *getKalignContext() {
    U2::KalignContext* ctx = static_cast<U2::KalignContext*>(U2::TLSUtils::current(KALIGN_CONTEXT_ID));
    assert(ctx->d != NULL);
    return ctx->d;
}

namespace U2 {

static const QString KALIGN_LOCK_REASON("Kalign lock");

void KalignTaskSettings::reset() {
    gapExtenstionPenalty = -1;
    gapOpenPenalty = -1;
    termGapPenalty = -1;
    secret = -1;
    inputFilePath="";
}

KalignTask::KalignTask(const MAlignment& ma, const KalignTaskSettings& _config) 
:TLSTask(tr("KALIGN alignment"), TaskFlags_FOSCOE), config(_config), inputMA(ma)
{
    GCOUNTER( cvar, tvar, "KalignTask" );
    inputSubMA = inputMA;
    resultSubMA.setAlphabet(inputSubMA.getAlphabet());
    QString inputMAName = inputMA.getName();
    resultMA.setName(inputMAName);
    resultSubMA.setName(inputMAName);
    tpm = Task::Progress_Manual;
    quint64 mem = inputMA.getNumRows() * sizeof(float);
    addTaskResource(TaskResourceUsage(RESOURCE_MEMORY,  (mem * mem + 3 * mem) / (1024 * 1024)));
}

void KalignTask::_run() {
    if (inputMA.getAlphabet()->getId() == BaseDNAAlphabetIds::RAW() ||
            inputMA.getAlphabet()->getId() == BaseDNAAlphabetIds::AMINO_EXTENDED()) {
        setError(tr("Unsupported alphabet: %1").arg(inputMA.getAlphabet()->getName()));
        return;
    }
    algoLog.info(tr("Kalign alignment started"));
    assert(!hasError());
    doAlign(); 
    if (!hasError() && !isCanceled()) {
        assert(resultMA.getAlphabet()!=NULL);
        algoLog.info(tr("Kalign alignment successfully finished"));
    }
}

void KalignTask::doAlign() {
    assert(resultSubMA.isEmpty());
    KalignAdapter::align(inputSubMA, resultSubMA, stateInfo);
    if (hasError()) {
        return;
    }
    resultMA = resultSubMA;
    MSAUtils::compareRowsAfterAlignment(inputMA, resultMA, stateInfo);
}

Task::ReportResult KalignTask::report() {
    KalignContext* ctx = static_cast<KalignContext*>(taskContext);
    delete ctx->d;
    return ReportResult_Finished;
}

TLSContext* KalignTask::createContextInstance()
{
    kalign_context* ctx = new kalign_context;
    init_context(ctx, &stateInfo);
    if(config.gapOpenPenalty != -1) {
        ctx->gpo = config.gapOpenPenalty;
    }
    if(config.gapExtenstionPenalty != -1) {
        ctx->gpe = config.gapExtenstionPenalty;
    }
    if(config.termGapPenalty != -1) {
        ctx->tgpe = config.termGapPenalty;
    }
    if(config.secret != -1) {
        ctx->secret = config.secret;
    }
    return new KalignContext(ctx);
}

//////////////////////////////////////////////////////////////////////////
// KalignGObjectTask

KalignGObjectTask::KalignGObjectTask(MAlignmentObject* _obj, const KalignTaskSettings& _config) 
: AlignGObjectTask("", TaskFlags_NR_FOSCOE, _obj), lock(NULL), kalignTask(NULL), config(_config)
{
    QString aliName = obj->getDocument()->getName();
    QString tn;
    tn = tr("KALIGN align '%1'").arg(aliName);
    setTaskName(tn);
    setUseDescriptionFromSubtask(true);
    setVerboseLogMode(true);
}

KalignGObjectTask::~KalignGObjectTask() {
    assert(lock == NULL);
}

void KalignGObjectTask::prepare() {
    CHECK_EXT(!obj.isNull(), stateInfo.setError(tr("Object is removed!")), );
    CHECK_EXT(!obj->isStateLocked(), stateInfo.setError(tr("Object is state-locked!")), );

    lock = new StateLock(KALIGN_LOCK_REASON, StateLockFlag_LiveLock);
    obj->lockState(lock);
    kalignTask = new KalignTask(obj->getMAlignment(), config);
    addSubTask(kalignTask);
}

Task::ReportResult KalignGObjectTask::report() {
    if (lock != NULL) {
        obj->unlockState(lock);
        delete lock;
        lock = NULL;
    }
    propagateSubtaskError();
    CHECK_OP(stateInfo, ReportResult_Finished);
    
    SAFE_POINT(!obj.isNull(), "Object was removed?!", ReportResult_Finished);
    CHECK_EXT(!obj->isStateLocked(), stateInfo.setError(tr("object_is_state_locked")), ReportResult_Finished);
    
    // Apply the result
    const MAlignment& inputMA = kalignTask->inputMA;
    MAlignment resultMA = kalignTask->resultMA;

    QList<qint64> rowsOrder = MSAUtils::compareRowsAfterAlignment(inputMA, resultMA, stateInfo);
    CHECK_OP(stateInfo, ReportResult_Finished);

    if (rowsOrder.count() != inputMA.getNumRows()) {
        stateInfo.setError("Unexpected number of rows in the result multiple alignment!");
        return ReportResult_Finished;
    }

    QMap<qint64, QList<U2MsaGap> > rowsGapModel;
    for (int i = 0, n = resultMA.getNumRows(); i < n; ++i) {
        qint64 rowId = resultMA.getRow(i).getRowDBInfo().rowId;
        const QList<U2MsaGap>& newGapModel = resultMA.getRow(i).getGapModel();
        rowsGapModel.insert(rowId, newGapModel);
    }

    // Save data to the database
    {
        U2OpStatus2Log os;
        U2UseCommonUserModStep userModStep(obj->getEntityRef(), os);
        if (os.hasError()) {
            stateInfo.setError("Failed to apply the result of the alignment!");
            return ReportResult_Finished;
        }

        obj->updateGapModel(rowsGapModel, stateInfo);
        SAFE_POINT_OP(stateInfo, ReportResult_Finished);

        if (rowsOrder != inputMA.getRowsIds()) {
            obj->updateRowsOrder(rowsOrder, stateInfo);
            SAFE_POINT_OP(stateInfo, ReportResult_Finished);
        }
    }

    return ReportResult_Finished;
}


///////////////////////////////////
//KalignGObjectRunFromSchemaTask


KalignGObjectRunFromSchemaTask::KalignGObjectRunFromSchemaTask(MAlignmentObject * o, const KalignTaskSettings & c) 
: AlignGObjectTask("", TaskFlags_NR_FOSCOE,o), config(c)
{
    QString tName = tr("KAlign align '%1'").arg(o->getDocument()->getName());
    setTaskName(tName);
    setUseDescriptionFromSubtask(true);
    setVerboseLogMode(true);

    SimpleMSAWorkflowTaskConfig conf;
    conf.algoName = "KAlign";
    conf.schemaName = "align-kalign";
    conf.schemaArgs << QString("--bonus-score=%1").arg(config.secret);
    conf.schemaArgs<< QString("--gap-ext-penalty=%1").arg(config.gapExtenstionPenalty);
    conf.schemaArgs<< QString("--gap-open-penalty=%1").arg(config.gapOpenPenalty);
    conf.schemaArgs<< QString("--gap-terminal-penalty=%1").arg(config.termGapPenalty);

    addSubTask(new SimpleMSAWorkflow4GObjectTask(QString("Workflow wrapper '%1'").arg(tName), o, conf));
}


//////////////////////////////////////////////////////////////////////////
/// KalignWithExtFileSpecifySupportTask

KalignWithExtFileSpecifySupportTask::KalignWithExtFileSpecifySupportTask(const KalignTaskSettings& _config )
:  Task("Run KAlign alignment task on external file", TaskFlags_NR_FOSCOE), config(_config)
{
    mAObject = NULL;
    currentDocument = NULL;
    cleanDoc = true;
    saveDocumentTask = NULL;
    loadDocumentTask = NULL;
    kalignGObjectTask = NULL;
}

KalignWithExtFileSpecifySupportTask::~KalignWithExtFileSpecifySupportTask() {
    if (cleanDoc) {
        delete currentDocument;
    }
}

void KalignWithExtFileSpecifySupportTask::prepare() {

    DocumentFormatConstraints c;
    c.checkRawData = true;
    c.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;
    c.rawData = IOAdapterUtils::readFileHeader(config.inputFilePath);
    QList<DocumentFormatId> formats = AppContext::getDocumentFormatRegistry()->selectFormats(c);
    if (formats.isEmpty()) {
        stateInfo.setError(  tr("input_format_error") );
        return;
        }
    DocumentFormatId alnFormat = formats.first();
    QVariantMap hints;
    if(alnFormat == BaseDocumentFormats::FASTA){
        hints[DocumentReadingMode_SequenceAsAlignmentHint] = true;
        }
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(config.inputFilePath));
    loadDocumentTask = new LoadDocumentTask(alnFormat, config.inputFilePath, iof, hints);
    addSubTask(loadDocumentTask);
    }        

QList<Task*> KalignWithExtFileSpecifySupportTask::onSubTaskFinished( Task* subTask ) {
    QList<Task*> res;
    if (subTask->hasError()) {
        stateInfo.setError(subTask->getError());
        return res;
    }
    if (hasError() || isCanceled()) {
        return res;

    }
        if (subTask == loadDocumentTask){
        currentDocument = loadDocumentTask->takeDocument();
        SAFE_POINT(currentDocument != NULL, QString("Failed loading document: %1").arg(loadDocumentTask->getURLString()), res);
        SAFE_POINT(currentDocument->getObjects().length() == 1, QString("Number of objects != 1 : %1").arg(loadDocumentTask->getURLString()), res);
        mAObject=qobject_cast<MAlignmentObject*>(currentDocument->getObjects().first());
        SAFE_POINT(mAObject != NULL, QString("MA object not found!: %1").arg(loadDocumentTask->getURLString()), res);

        kalignGObjectTask = new KalignGObjectRunFromSchemaTask(mAObject, config);
        res.append(kalignGObjectTask);
    } else if (subTask == kalignGObjectTask){
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(config.outputFilePath));
        saveDocumentTask = new SaveDocumentTask(currentDocument,iof,config.outputFilePath);
        res.append(saveDocumentTask);
    } else if (subTask == saveDocumentTask){
        Task* openTask = AppContext::getProjectLoader()->openWithProjectTask(config.outputFilePath);
        if (openTask != NULL) {
            res << openTask;
        }
    }
    return res;
}

} //namespace
