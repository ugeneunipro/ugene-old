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
#include <U2Core/Counter.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Gui/OpenViewTask.h>
#include <U2Lang/WorkflowSettings.h>

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
            ids[i] = ctx->output_uIds[i];
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
                MAlignmentRow row(inputMA.getRow(i).getName(), gapSeq);
                if(config.stableMode) {
                    resultSubMA.addRow(row,i);
                } else {
                    ids[j] = i;
                    resultSubMA.addRow(row);
                }
                j++;
            }
        }
        delete[] existID;

        assert(resultSubMA.getNumRows() == inputMA.getNumRows());

        if (config.alignRegion && config.regionToAlign.length != inputMA.getLength()) {                        

            for(int i=0, n = inputMA.getNumRows(); i < n; i++) {
                const MAlignmentRow& row= inputMA.getRow(ids[i]);
                resultMA.addRow(MAlignmentRow(row.getName(), emptySeq));
            }
            if (config.regionToAlign.startPos != 0) {
				for(int i=0; i < nSeq; i++)  {
					int regionLen = config.regionToAlign.startPos;
					MAlignmentRow inputRow = inputMA.getRow(ids[i]).mid(0,regionLen);
					resultMA.appendChars(i, inputRow.toByteArray(regionLen).constData(), regionLen);
				}
            }
            resultMA += resultSubMA;
            if (config.regionToAlign.endPos() != inputMA.getLength()) {
                int subStart = config.regionToAlign.endPos();
                int subLen = inputMA.getLength() - config.regionToAlign.endPos();
				for(int i = 0; i < nSeq; i++) {
					MAlignmentRow inputRow = inputMA.getRow(ids[i]).mid(subStart,subLen);
					resultMA.appendChars(i, inputRow.toByteArray(subLen).constData(), subLen);
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
//     c.rawData = BaseIOAdapters::readFileHeader(fileWithSequencesOrProfile);
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
    QList<DocumentFormat*> detectedFormats = DocumentUtils::detectFormat(fileWithSequencesOrProfile);    
    if (detectedFormats.isEmpty()) {
        setError("Unknown format");
    }
    DocumentFormat* format = detectedFormats.first();
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(fileWithSequencesOrProfile));
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
        DNASequenceObject* dnaObj = qobject_cast<DNASequenceObject*>(obj);
        DNAAlphabet* objAl = dnaObj->getAlphabet();
        if (al == NULL) {
            al = objAl;
        } else if (al != objAl) {
            al = DNAAlphabet::deriveCommonAlphabet(al, objAl);
            if (al == NULL) {
                stateInfo.setError(tr("Sequences in file have different alphabets %1").arg(loadTask->getDocument()->getURLString()));
                return res;
            }
        }
        s.profile.addRow(MAlignmentRow(dnaObj->getGObjectName(), dnaObj->getSequence()));
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
: MAlignmentGObjectTask("", TaskFlags_NR_FOSCOE,_obj), lock(NULL), muscleTask(NULL), config(_config)
{
    QString aliName = obj->getDocument()->getName();
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
    } else if (config.op == MuscleTaskOp_Align) {
        assert(muscleTask->inputMA.getNumRows() == muscleTask->resultMA.getNumRows());
    } else if (config.op == MuscleTaskOp_ProfileToProfile) {
        assert(muscleTask->inputMA.getNumRows() + config.profile.getNumRows() == muscleTask->resultMA.getNumRows());
    }
    obj->setMAlignment(muscleTask->resultMA);    

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
}

void MuscleWithExtFileSpecifySupportTask::prepare(){
    DocumentFormatConstraints c;
    c.checkRawData = true;
    c.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;
    c.rawData = BaseIOAdapters::readFileHeader(config.inputFilePath);
    QList<DocumentFormatId> formats = AppContext::getDocumentFormatRegistry()->selectFormats(c);
    if (formats.isEmpty()) {
        stateInfo.setError(  tr("input_format_error") );
        return;
    }

    DocumentFormatId alnFormat = formats.first();
    loadDocumentTask=
            new LoadDocumentTask(alnFormat,
                                 config.inputFilePath,
                                 AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(config.inputFilePath)));
    addSubTask(loadDocumentTask);
}

QList<Task*> MuscleWithExtFileSpecifySupportTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if(subTask->hasError()) {
        stateInfo.setError(subTask->getError());
        return res;
    }
    if(hasError() || isCanceled()) {
        return res;
    }
    if(subTask==loadDocumentTask){
        currentDocument=loadDocumentTask->takeDocument();
        assert(currentDocument!=NULL);
        assert(currentDocument->getObjects().length()==1);
        mAObject=qobject_cast<MAlignmentObject*>(currentDocument->getObjects().first());
        assert(mAObject!=NULL);
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

#ifndef RUN_WORKFLOW_IN_THREADS
        if(WorkflowSettings::runInSeparateProcess() && !WorkflowSettings::getCmdlineUgenePath().isEmpty()) {
            muscleGObjectTask = new MuscleGObjectRunFromSchemaTask(mAObject, config);
        } else {
            muscleGObjectTask = new MuscleGObjectTask(mAObject, config);
        }
#else
        muscleGObjectTask = new MuscleGObjectTask(mAObject, config);
#endif // RUN_WORKFLOW_IN_THREADS
        assert(muscleGObjectTask != NULL);
        res.append(muscleGObjectTask);
    }else if(subTask == muscleGObjectTask){
        saveDocumentTask = new SaveDocumentTask(currentDocument,AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(config.inputFilePath)),config.inputFilePath);
        res.append(saveDocumentTask);
    }else if(subTask==saveDocumentTask){
        Project* proj = AppContext::getProject();
        if (proj == NULL) {
            res.append(AppContext::getProjectLoader()->openWithProjectTask(currentDocument->getURLString()));
        } else {
            bool docAlreadyInProject=false;
            foreach(Document* doc, proj->getDocuments()){
                if(doc->getURL() == currentDocument->getURL()){
                    docAlreadyInProject=true;
                }
            }
            if (docAlreadyInProject) {
                res.append(new LoadUnloadedDocumentAndOpenViewTask(currentDocument));
            } else {
                // Add document to project
                res.append(new AddDocumentTask(currentDocument));
                res.append(new LoadUnloadedDocumentAndOpenViewTask(currentDocument));
            }
        }
    }
    return res;
}

Task::ReportResult MuscleWithExtFileSpecifySupportTask::report(){
    return ReportResult_Finished;
}

#ifndef RUN_WORKFLOW_IN_THREADS

//////////////////////////////////
//MuscleGObjectRunFromSchemaTask
static const QString LOCK_NAME("muscle state lock");
static const QString MUSCLE_SCHEMA_NAME("muscle");

MuscleGObjectRunFromSchemaTask::MuscleGObjectRunFromSchemaTask(MAlignmentObject * o, const MuscleTaskSettings & c) :
MAlignmentGObjectTask("", TaskFlags_NR_FOSCOE, o), objName(o->getDocument()->getName()), config(c), runSchemaTask(NULL), lock(NULL){
    assertConfig();
    assert(!objName.isEmpty());
    
    QString name;
    switch(config.op) {
        case MuscleTaskOp_Align:
            name = tr("MUSCLE run in separate process align '%1'").arg(objName);
            break;
        case MuscleTaskOp_Refine: 
            name = tr("MUSCLE run in separate process refine '%1'").arg(objName);
            break;
        default: 
            assert(false);
    }
    setTaskName(name);
    setUseDescriptionFromSubtask(true);
    setVerboseLogMode(true);
}

MuscleGObjectRunFromSchemaTask::~MuscleGObjectRunFromSchemaTask() {
    assert(lock == NULL);
}

void MuscleGObjectRunFromSchemaTask::assertConfig() {
    assert(config.op == MuscleTaskOp_Align || config.op == MuscleTaskOp_Refine);
    assert(config.profile.isEmpty());
}

void MuscleGObjectRunFromSchemaTask::prepare() {
    if (obj.isNull()) {
        stateInfo.setError(tr("Object '%1' removed").arg(objName));
        return;
    }
    if (obj->isStateLocked()) {
        stateInfo.setError(tr("Object '%1' is locked").arg(objName));
        return;
    }
    
    lock = new StateLock(MUSCLE_LOCK_REASON, StateLockFlag_LiveLock);
    obj->lockState(lock);
    runSchemaTask = new WorkflowRunSchemaForTask(MUSCLE_SCHEMA_NAME, this);
    addSubTask(runSchemaTask);
}

Task::ReportResult MuscleGObjectRunFromSchemaTask::report() {
    if (lock!=NULL) {
        obj->unlockState(lock);
        delete lock;
        lock = NULL;
    }
    
    propagateSubtaskError();
    if(hasError() || isCanceled()) {
        return ReportResult_Finished;
    }
    
    if (obj->isStateLocked()) {
        setError(tr("Object '%1' is locked").arg(objName));
        return ReportResult_Finished;
    }
    
    std::auto_ptr<Document> result(runSchemaTask->getResult());
    QList<GObject*> objs = result->getObjects();
    assert(objs.size() == 1);
    const QString MUSCLE_TASK_NO_RESULT_ERROR(tr("Undefined error: muscle task did not produced result"));
    if( objs.isEmpty() ) {
        setError(MUSCLE_TASK_NO_RESULT_ERROR);
            return ReportResult_Finished;
    }
    MAlignmentObject * maObj = qobject_cast<MAlignmentObject*>(objs.first());
    if(maObj == NULL) {
        setError(MUSCLE_TASK_NO_RESULT_ERROR);
            return ReportResult_Finished;
    }
    obj->setMAlignment(maObj->getMAlignment());
    return ReportResult_Finished;
}

bool MuscleGObjectRunFromSchemaTask::saveInput() const {
    return true;
}

QList<GObject*> MuscleGObjectRunFromSchemaTask::createInputData() const {
    QList<GObject*> objs;
    objs << obj.data()->clone();
    return objs;
}

QVariantMap MuscleGObjectRunFromSchemaTask::getSchemaData() const {
    QVariantMap res;
    res["mode"] = qVariantFromValue((int)config.mode);
    res["max-iterations"] = qVariantFromValue(config.maxIterations);
    res["stable"] = qVariantFromValue(config.stableMode);
    if(!config.alignRegion) {
        res["range"] = qVariantFromValue(QString("Whole alignment"));
    } else {
        res["range"] = qVariantFromValue(QString("%1..%2").arg(config.regionToAlign.startPos + 1).arg(config.regionToAlign.endPos()));
    }
    return res;
}

DocumentFormatId MuscleGObjectRunFromSchemaTask::outputFileFormat() const {
    return inputFileFormat();
}

DocumentFormatId MuscleGObjectRunFromSchemaTask::inputFileFormat() const {
    if(obj != NULL && obj->getDocument() != NULL && obj->getDocument()->getDocumentFormat() != NULL) {
        return obj->getDocument()->getDocumentFormat()->getFormatId();
    } else {
        return BaseDocumentFormats::CLUSTAL_ALN;
    }
}

bool MuscleGObjectRunFromSchemaTask::saveOutput() const {
    return true;
}

#endif // RUN_WORKFLOW_IN_THREADS

} //namespace
