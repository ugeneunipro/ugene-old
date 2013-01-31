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

#include <memory>

#include "MuscleTask.h"
#include "MuscleParallel.h"
#include "MuscleAdapter.h"
#include "MuscleConstants.h"
#include "TaskLocalStorage.h"

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
#include <U2Core/DocumentUtils.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Lang/WorkflowSettings.h>
#include <U2Lang/SimpleWorkflowTask.h>

#include <U2Gui/OpenViewTask.h>

#include "muscle/muscle.h" 
#include "muscle/muscle_context.h" 

#define MUSCLE_LOCK_REASON "Muscle lock"

namespace U2 {

void MuscleTaskSettings::reset() {
    nThreads = 0; 
    op = MuscleTaskOp_Align;
    maxIterations = 8;
    maxSecs = 0;
    stableMode = true;
    regionToAlign.startPos = regionToAlign.length = 0;
    profile.clear();
    alignRegion = false;
    inputFilePath = "";
    mode = Default;
}

MuscleTask::MuscleTask(const MAlignment& ma, const MuscleTaskSettings& _config) 
:Task(tr("MUSCLE alignment"), TaskFlags_FOSCOE), config(_config), inputMA(ma)
{
    GCOUNTER( cvar, tvar, "MuscleTask" );
    config.nThreads = (config.nThreads == 0) ? AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount():config.nThreads;
    assert(config.nThreads > 0);
    setMaxParallelSubtasks(config.nThreads);

    algoLog.info(tr("MUSCLE alignment started"));

    ctx = new MuscleContext(config.nThreads);
    ctx->params.g_bStable = config.stableMode;
    ctx->params.g_uMaxIters = config.maxIterations;
    ctx->params.g_ulMaxSecs = config.maxSecs;
    parallelSubTask = NULL;

    //todo: make more precise estimation, use config.op mode
    int aliLen = ma.getLength();
    int nSeq = ma.getNumRows();
    int memUseMB = qint64(aliLen) * qint64(nSeq) * 200 / (1024 * 1024); //200x per char in alignment
    TaskResourceUsage tru(RESOURCE_MEMORY, memUseMB);

    QString inputAlName = inputMA.getName();
    resultMA.setName(inputAlName);
    resultSubMA.setName(inputAlName);
    
    inputSubMA = inputMA;
    if (config.alignRegion && config.regionToAlign.length != inputMA.getLength()) {
        assert(config.regionToAlign.length > 0);
        inputSubMA = inputMA.mid(config.regionToAlign.startPos, config.regionToAlign.length);
    }
    
    ctx->input_uIds = new unsigned[inputSubMA.getNumRows()];
    ctx->tmp_uIds = new unsigned[inputSubMA.getNumRows()];
	for(unsigned i=0, n = inputSubMA.getNumRows(); i<n; i++) {
        ctx->input_uIds[i] = i;
	}

    if (config.nThreads == 1 || (config.op != MuscleTaskOp_Align)) {
        tpm = Task::Progress_Manual;
    } else {
        setUseDescriptionFromSubtask(true);
        parallelSubTask = new MuscleParallelTask(inputSubMA, resultSubMA, config, ctx);
        addSubTask(parallelSubTask);
        tru.prepareStageLock = true;
    }
    addTaskResource(tru);
}

void MuscleTask::run() {
    TaskLocalData::bindToMuscleTLSContext(ctx);

    assert(!hasError());

    algoLog.details(tr("Performing MUSCLE alignment..."));

    switch(config.op) {
        case MuscleTaskOp_Align:
            doAlign(false); 
            break;
        case MuscleTaskOp_Refine: 
            doAlign(true); 
            break;
        case MuscleTaskOp_AddUnalignedToProfile: 
            doAddUnalignedToProfile();
            break;
        case MuscleTaskOp_ProfileToProfile: 
            doProfile2Profile();
            break;
    }
    if (!hasError() && !isCanceled()) {
        assert(resultMA.getAlphabet() != NULL);
    }
    TaskLocalData::detachMuscleTLSContext();

    if(!stateInfo.isCoR()) {
        algoLog.info(tr("MUSCLE alignment successfully finished"));
    }
}

void MuscleTask::doAlign(bool refine) {
    if (parallelSubTask == NULL) { //align in this thread
        assert(resultSubMA.isEmpty());
        if (refine) {
            MuscleAdapter::refine(inputSubMA, resultSubMA, stateInfo);
        } else {
            MuscleAdapter::align(inputSubMA, resultSubMA, stateInfo, true);
        }
        if (hasError()) {
            return;
        }
    } 
    assert(!hasError());
    U2OpStatus2Log os;

    if(!isCanceled()) {
        assert(!resultSubMA.isEmpty());
        resultMA.setAlphabet(inputMA.getAlphabet());
        QByteArray emptySeq;
        const int nSeq = inputMA.getNumRows();
        int *ids = new int[nSeq];

        const int resNSeq = resultSubMA.getNumRows();
        bool *existID = new bool[nSeq];
        memset(existID,0,sizeof(bool)*nSeq);
        for(int i=0, n = resNSeq; i < n; i++) {
            ids[i] = ctx->output_uIds ? ctx->output_uIds[i] : i;
            existID[ids[i]] = true;
        }
        if(config.stableMode) {
            for(int i = 0; i<nSeq;i++) {
                ids[i] = i;
            }
        }
        int j = resNSeq;
        QByteArray gapSeq(resultSubMA.getLength(),MAlignment_GapChar);
        for(int i=0, n = nSeq; i < n; i++) {
            if(!existID[i]) {
                QString rowName = inputMA.getRow(i).getName();
                if(config.stableMode) {
                    resultSubMA.addRow(rowName, gapSeq, i, os);
                } else {
                    ids[j] = i;
                    resultSubMA.addRow(rowName, gapSeq, os);
                }
                j++;
            }
        }
        delete[] existID;

        assert(resultSubMA.getNumRows() == inputMA.getNumRows());

        if (config.alignRegion && config.regionToAlign.length != inputMA.getLength()) {                        

            for(int i=0, n = inputMA.getNumRows(); i < n; i++) {
                const MAlignmentRow& row= inputMA.getRow(ids[i]);
                resultMA.addRow(row.getName(), emptySeq, os);
            }
            if (config.regionToAlign.startPos != 0) {
				for(int i=0; i < nSeq; i++)  {
					int regionLen = config.regionToAlign.startPos;
					MAlignmentRow inputRow = inputMA.getRow(ids[i]).mid(0,regionLen, os);
					resultMA.appendChars(i, inputRow.toByteArray(regionLen, os).constData(), regionLen);
				}
            }
            resultMA += resultSubMA;
            if (config.regionToAlign.endPos() != inputMA.getLength()) {
                int subStart = config.regionToAlign.endPos();
                int subLen = inputMA.getLength() - config.regionToAlign.endPos();
				for(int i = 0; i < nSeq; i++) {
					MAlignmentRow inputRow = inputMA.getRow(ids[i]).mid(subStart, subLen, os);
					resultMA.appendChars(i, inputRow.toByteArray(subLen, os).constData(), subLen);
				}
            }
            delete[] ids;
            //TODO: check if there are GAP columns on borders and remove them        
        } else {
            resultMA = resultSubMA;
        }
    }
}

void MuscleTask::doAddUnalignedToProfile() {
    MuscleAdapter::addUnalignedSequencesToProfile(inputMA, config.profile, resultMA, stateInfo);
}

void MuscleTask::doProfile2Profile() {
    MuscleAdapter::align2Profiles(inputMA, config.profile, resultMA, stateInfo);
}

Task::ReportResult MuscleTask::report() {
    delete ctx;
    ctx = NULL;
    return ReportResult_Finished;
}

//////////////////////////////////////////////////////////////////////////
// MuscleAddSequencesToProfileTask

MuscleAddSequencesToProfileTask::MuscleAddSequencesToProfileTask(MAlignmentObject* _obj, const QString& fileWithSequencesOrProfile, MMode _mode) 
: Task("", TaskFlags_NR_FOSCOE), maObj(_obj), mode(_mode)
{
    setUseDescriptionFromSubtask(true);
    setVerboseLogMode(true);
    
    QString aliName = maObj->getDocument()->getName();
    QString fileName = QFileInfo(fileWithSequencesOrProfile).fileName();
    QString tn;
    if (mode == Profile2Profile) {
        tn = tr("MUSCLE align profiles '%1' vs '%2'").arg(aliName).arg(fileName);
    } else {
        tn = tr("MUSCLE align '%2' by profile '%1'").arg(aliName).arg(fileName);
    }
    setTaskName(tn);


    //todo: create 'detect file format task'
//     DocumentFormatConstraints c;
//     c.checkRawData = true;
//     c.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT; //MA here comes first because for a sequence format raw sequence can be used by default
//     c.rawData = IOAdapterUtils::readFileHeader(fileWithSequencesOrProfile);
//     QList<DocumentFormatId> formats = AppContext::getDocumentFormatRegistry()->selectFormats(c);
//     if (formats.isEmpty()) {
//         c.supportedObjectTypes.clear();
//         c.supportedObjectTypes += GObjectTypes::SEQUENCE;
//         formats = AppContext::getDocumentFormatRegistry()->selectFormats(c);
//         if (formats.isEmpty()) {
//             stateInfo.setError(tr("input_format_error"));
//             return;
//         }
//     }
    QList<FormatDetectionResult> detectedFormats = DocumentUtils::detectFormat(fileWithSequencesOrProfile);    
    if (detectedFormats.isEmpty()) {
        setError("Unknown format");
    }
    DocumentFormat* format = detectedFormats.first().format;
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(fileWithSequencesOrProfile));
    loadTask = new LoadDocumentTask(format->getFormatId(), fileWithSequencesOrProfile, iof);
    loadTask->setSubtaskProgressWeight(0.01f);
    addSubTask(loadTask);
}

QList<Task*> MuscleAddSequencesToProfileTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;

    if (subTask != loadTask || isCanceled() || hasError()) {
        return res;
    }

    propagateSubtaskError();
    if (hasError()) {
        return res;
    }

    MuscleTaskSettings s;
    s.op = mode == Sequences2Profile ? MuscleTaskOp_AddUnalignedToProfile : MuscleTaskOp_ProfileToProfile;

    QList<GObject*> seqObjects = loadTask->getDocument()->findGObjectByType(GObjectTypes::SEQUENCE);
    //todo: move to utility alphabet reduction
    DNAAlphabet* al = NULL;
    foreach(GObject* obj, seqObjects) {
        U2SequenceObject* dnaObj = qobject_cast<U2SequenceObject*>(obj);
        DNAAlphabet* objAl = dnaObj->getAlphabet();
        if (al == NULL) {
            al = objAl;
        } else if (al != objAl) {
            al = U2AlphabetUtils::deriveCommonAlphabet(al, objAl);
            CHECK_EXT(al != NULL, setError(tr("Sequences in file have different alphabets %1").arg(loadTask->getDocument()->getURLString())), res);
        }
        U2OpStatus2Log os;
        s.profile.addRow(dnaObj->getSequenceName(), dnaObj->getWholeSequenceData(), os);
    }
    if(!seqObjects.isEmpty()) {
        s.profile.setAlphabet(al);
    }

    if (seqObjects.isEmpty()) {
        QList<GObject*> maObjects = loadTask->getDocument()->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
        if (!maObjects.isEmpty()) {
            MAlignmentObject* maObj = qobject_cast<MAlignmentObject*>(maObjects.first());
            s.profile = maObj->getMAlignment();
        }
    }
    
    if (s.profile.isEmpty()) {
        if (mode == Sequences2Profile) {
            stateInfo.setError(tr("No sequences found in file %1").arg(loadTask->getDocument()->getURLString()));
        } else {
            stateInfo.setError(tr("No alignment found in file %1").arg(loadTask->getDocument()->getURLString()));
        }
        return res;
    }
    res.append(new MuscleGObjectTask(maObj, s));
    return res;
}

Task::ReportResult MuscleAddSequencesToProfileTask::report() {
    if (!hasError()) {
        propagateSubtaskError();
    }
    return ReportResult_Finished;
}

//////////////////////////////////////////////////////////////////////////
// MuscleGObjectTask

MuscleGObjectTask::MuscleGObjectTask(MAlignmentObject* _obj, const MuscleTaskSettings& _config) 
: AlignGObjectTask("", TaskFlags_NR_FOSCOE,_obj), lock(NULL), muscleTask(NULL), config(_config)
{
    QString aliName;
    if (NULL == obj->getDocument()) {
        aliName = MA_OBJECT_NAME;
    } else {
        aliName = obj->getDocument()->getName();
    }
    QString tn;
    switch(config.op) {
        case MuscleTaskOp_Align:
            tn = tr("MUSCLE align '%1'").arg(aliName);
            break;
        case MuscleTaskOp_Refine: 
            tn = tr("MUSCLE refine '%1'").arg(aliName);
            break;
        case MuscleTaskOp_AddUnalignedToProfile: 
            tn = tr("MUSCLE add to profile '%1'").arg(aliName);
            break;
        case MuscleTaskOp_ProfileToProfile: 
            tn = tr("MUSCLE align profiles");
            break;
        default: assert(0);
    }
    setTaskName(tn);
    setUseDescriptionFromSubtask(true);
    setVerboseLogMode(true);
}

MuscleGObjectTask::~MuscleGObjectTask() {
    assert(lock == NULL);
}

void MuscleGObjectTask::prepare() {
    if (obj.isNull()) {
        stateInfo.setError(tr("object_removed"));
        return;
    }
    if (obj->isStateLocked()) {
        stateInfo.setError(tr("object_is_state_locked"));
        return;
    }

    lock = new StateLock(MUSCLE_LOCK_REASON, StateLockFlag_LiveLock);
    obj->lockState(lock);
    muscleTask = new MuscleTask(obj->getMAlignment(), config);

    addSubTask(muscleTask);
}

Task::ReportResult MuscleGObjectTask::report() {
    if (lock!=NULL) {
        obj->unlockState(lock);
        delete lock;
        lock = NULL;
    }
    propagateSubtaskError();
    if (hasError() || isCanceled()) {
        return ReportResult_Finished;
    }
    assert(!obj.isNull());
    if (obj->isStateLocked()) {
        stateInfo.setError(tr("object_is_state_locked"));
        return ReportResult_Finished;
    }
    if (config.op == MuscleTaskOp_AddUnalignedToProfile) {
        assert((muscleTask->inputMA.getNumRows() + config.profile.getNumRows()) == muscleTask->resultMA.getNumRows());
        obj->setMAlignment(muscleTask->resultMA);    
    }
    else if (config.op == MuscleTaskOp_Align || config.op == MuscleTaskOp_Refine) {
        QList<qint64> rowsOrder = MSAUtils::compareRowsAfterAlignment(muscleTask->inputMA, muscleTask->resultMA, stateInfo);
        CHECK_OP(stateInfo, ReportResult_Finished);

        if (rowsOrder.count() != muscleTask->inputMA.getNumRows()) {
            stateInfo.setError("Unexpected number of rows in the result multiple alignment!");
            return ReportResult_Finished;
        }

        QMap<qint64, QList<U2MsaGap> > rowsGapModel;
        for (int i = 0, n = muscleTask->resultMA.getNumRows(); i < n; ++i) {            
            qint64 rowId = muscleTask->resultMA.getRow(i).getRowDBInfo().rowId;
            const QList<U2MsaGap>& newGapModel = muscleTask->resultMA.getRow(i).getGapModel();
            rowsGapModel.insert(rowId, newGapModel);
        }

        obj->updateGapModel(rowsGapModel, stateInfo);

        if (rowsOrder != muscleTask->inputMA.getRowsIds()) {
            obj->updateRowsOrder(rowsOrder, stateInfo);
        }
    }
    else if (config.op == MuscleTaskOp_ProfileToProfile) {
        assert(muscleTask->inputMA.getNumRows() + config.profile.getNumRows() == muscleTask->resultMA.getNumRows());
        obj->setMAlignment(muscleTask->resultMA);    
    }

    return ReportResult_Finished;
}


////////////////////////////////////////
//MuscleWithExtFileSpecifySupportTask
MuscleWithExtFileSpecifySupportTask::MuscleWithExtFileSpecifySupportTask(const MuscleTaskSettings& _config) :
        Task("Run Muscle alignment task", TaskFlags_NR_FOSCOE),
        config(_config)
{
    mAObject = NULL;
    currentDocument = NULL;
    saveDocumentTask = NULL;
    loadDocumentTask = NULL;
    muscleGObjectTask = NULL;
    cleanDoc = true;
}

MuscleWithExtFileSpecifySupportTask::~MuscleWithExtFileSpecifySupportTask() {
    if (cleanDoc) {
        delete currentDocument;
    }
}

void MuscleWithExtFileSpecifySupportTask::prepare(){
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
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(config.inputFilePath));
    QVariantMap hints;
    if(alnFormat == BaseDocumentFormats::FASTA){
        hints[DocumentReadingMode_SequenceAsAlignmentHint] = true;
    }
    loadDocumentTask = new LoadDocumentTask(alnFormat, config.inputFilePath, iof, hints);
    addSubTask(loadDocumentTask);
}

QList<Task*> MuscleWithExtFileSpecifySupportTask::onSubTaskFinished(Task* subTask) {
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
        if (config.alignRegion) {
            if((config.regionToAlign.startPos > mAObject->getLength())
                || ((config.regionToAlign.startPos + config.regionToAlign.length) > mAObject->getLength()))
                {
                    config.alignRegion=false;
                    config.regionToAlign = U2Region(0, mAObject->getLength());
                }
        } else {
            config.regionToAlign = U2Region(0, mAObject->getLength());
        }

        if (WorkflowSettings::runInSeparateProcess()) {
            muscleGObjectTask = new MuscleGObjectRunFromSchemaTask(mAObject, config);
        } else {
            muscleGObjectTask = new MuscleGObjectTask(mAObject, config);
        }
        assert(muscleGObjectTask != NULL);
        res.append(muscleGObjectTask);
    } else if (subTask == muscleGObjectTask){
        saveDocumentTask = new SaveDocumentTask(currentDocument,AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(config.outputFilePath)),config.outputFilePath);
        res.append(saveDocumentTask);
    } else if (subTask == saveDocumentTask){
        //Project* proj = AppContext::getProject();
        //if (proj == NULL) {
        //    res.append(AppContext::getProjectLoader()->openWithProjectTask(currentDocument->getURL(), currentDocument->getGHintsMap()));
        //} else {
        //    Document* projDoc = proj->findDocumentByURL(currentDocument->getURL());
        //    if (projDoc) {
        //        projDoc->setLastUpdateTime();
        //        res.append(new LoadUnloadedDocumentAndOpenViewTask(projDoc));
        //    } else {
        //        // Add document to project
        //        res.append(new AddDocumentAndOpenViewTask(currentDocument));
        //        cleanDoc = false;
        //    }
        //}
        Task* openTask = AppContext::getProjectLoader()->openWithProjectTask(config.outputFilePath);
        res << openTask;
    }
    return res;
}

Task::ReportResult MuscleWithExtFileSpecifySupportTask::report(){
    return ReportResult_Finished;
}


//////////////////////////////////
//MuscleGObjectRunFromSchemaTask
MuscleGObjectRunFromSchemaTask::MuscleGObjectRunFromSchemaTask(MAlignmentObject * o, const MuscleTaskSettings & c) 
: AlignGObjectTask("", TaskFlags_NR_FOSCOE, o), config(c)
{
    assert(config.op == MuscleTaskOp_Align || config.op == MuscleTaskOp_Refine);
    assert(config.profile.isEmpty());
    setUseDescriptionFromSubtask(true);
    setVerboseLogMode(true);
    
    QString objName = o->getDocument()->getName();
    assert(!objName.isEmpty());
    
    QString tName;
    switch(config.op) {
        case MuscleTaskOp_Align:
            tName = tr("MUSCLE align '%1'").arg(objName);
            break;
        case MuscleTaskOp_Refine: 
            tName = tr("MUSCLE refine '%1'").arg(objName);
            break;
        default: 
            assert(false);
    }
    setTaskName(tName);

    SimpleMSAWorkflowTaskConfig conf;
    conf.algoName = "Muscle";
    conf.schemaName = "align";
    conf.schemaArgs << QString("--mode=%1").arg(config.mode);
    conf.schemaArgs << QString("--max-iterations=%1").arg(config.maxIterations);
    conf.schemaArgs << QString("--stable=%1").arg(config.stableMode);
    if (config.alignRegion) {
        conf.schemaArgs << QString("--range=%1").arg(QString("%1..%2").arg(config.regionToAlign.startPos + 1).arg(config.regionToAlign.endPos()));
    }
    addSubTask(new SimpleMSAWorkflow4GObjectTask(QString("Workflow wrapper '%1'").arg(tName), o, conf));
}

} //namespace
