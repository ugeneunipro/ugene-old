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

#include "ClustalWSupportTask.h"
#include "ClustalWSupport.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Counter.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/U2Mod.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/DNAAlphabet.h>

#include <U2Gui/OpenViewTask.h>

namespace U2 {

void ClustalWSupportTaskSettings::reset() {
    gapExtenstionPenalty = -1;
    gapOpenPenalty = -1;
    endGaps=false;
    noPGaps=false;
    noHGaps=false;
    gapDist=-1;
    iterationType="";
    numIterations=-1;
    matrix="";
    inputFilePath="";
    outOrderInput=true;
}

ClustalWSupportTask::ClustalWSupportTask(const MAlignment& _inputMsa, const GObjectReference& _objRef, const ClustalWSupportTaskSettings& _settings)
    : ExternalToolSupportTask("Run ClustalW alignment task", TaskFlags_NR_FOSCOE),
      inputMsa(_inputMsa),
      objRef(_objRef),
      settings(_settings),
      lock(NULL)
{
    GCOUNTER( cvar, tvar, "ClustalWSupportTask" );
    saveTemporaryDocumentTask=NULL;
    loadTemporyDocumentTask=NULL;
    clustalWTask=NULL;
    tmpDoc=NULL;
    logParser=NULL;
    resultMA.setAlphabet(inputMsa.getAlphabet());
    resultMA.setName(inputMsa.getName());
}

ClustalWSupportTask::~ClustalWSupportTask() {
    if (NULL != tmpDoc) {
        delete tmpDoc;
    }
    //Unlock the alignment object if the task has been failed
    if(!lock.isNull()) {
        if (objRef.isValid()) {
            GObject* obj = GObjectUtils::selectObjectByReference(objRef, UOF_LoadedOnly);
            if (NULL != obj) {
                MAlignmentObject* alObj = dynamic_cast<MAlignmentObject*>(obj);
                CHECK(NULL != alObj, );
                if(alObj->isStateLocked()) {
                    alObj->unlockState(lock);
                }
                delete lock;
                lock = NULL;
            }
        }
    }
}

void ClustalWSupportTask::prepare(){
    SAFE_POINT_EXT(NULL != inputMsa.getAlphabet(), "The alphabet is NULL",);
    if (inputMsa.getAlphabet()->getId() == BaseDNAAlphabetIds::RAW() ||
            inputMsa.getAlphabet()->getId() == BaseDNAAlphabetIds::AMINO_EXTENDED()) {
        setError(tr("Unsupported alphabet: %1").arg(inputMsa.getAlphabet()->getName()));
        return;
    }
    algoLog.info(tr("ClustalW alignment started"));

    if (objRef.isValid()) {
        GObject* obj = GObjectUtils::selectObjectByReference(objRef, UOF_LoadedOnly);
        if (NULL != obj) {
            MAlignmentObject* alObj = dynamic_cast<MAlignmentObject*>(obj);
            SAFE_POINT(NULL != alObj, "Failed to convert GObject to MAlignmentObject during applying ClustalW results!",);
            lock = new StateLock("ClustalWAligment");
            alObj->lockState(lock);
        }
    }


    //Add new subdir for temporary files
    //Directory name is ExternalToolName + CurrentDate + CurrentTime

    QString tmpDirName = "ClustalW_"+QString::number(this->getTaskId())+"_"+
                         QDate::currentDate().toString("dd.MM.yyyy")+"_"+
                         QTime::currentTime().toString("hh.mm.ss.zzz")+"_"+
                         QString::number(QCoreApplication::applicationPid())+"/";
    QString tmpDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(CLUSTAL_TMP_DIR) + "/" + tmpDirName;
    url= tmpDirPath + "tmp.aln";
    ioLog.details(tr("Saving data to temporary file '%1'").arg(url));

    //Check and remove subdir for temporary files
    QDir tmpDir(tmpDirPath);
    if(tmpDir.exists()){
        foreach(const QString& file, tmpDir.entryList()){
            tmpDir.remove(file);
        }
        if(!tmpDir.rmdir(tmpDir.absolutePath())){
            stateInfo.setError("Subdirectory for temporary files exists. Can not remove this directory.");
            return;
        }
    }
    if(!tmpDir.mkpath(tmpDirPath)){
        stateInfo.setError("Can not create directory for temporary files.");
        return;
    }

    saveTemporaryDocumentTask = new SaveAlignmentTask( MSAUtils::setUniqueRowNames(inputMsa), url, BaseDocumentFormats::CLUSTAL_ALN);
    saveTemporaryDocumentTask->setSubtaskProgressWeight(5);
    addSubTask(saveTemporaryDocumentTask);
}

QList<Task*> ClustalWSupportTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if(subTask->hasError()) {
        stateInfo.setError(subTask->getError());
        return res;
    }
    if(hasError() || isCanceled()) {
        return res;
    }
    QString outputUrl = url+".out.aln";
    if(subTask==saveTemporaryDocumentTask){
        QStringList arguments;
        arguments <<"-ALIGN"<< "-INFILE="+url;
        if(settings.gapOpenPenalty != -1) {
            arguments <<"-GAPOPEN="+QString::number(settings.gapOpenPenalty);
        }
        if(settings.gapExtenstionPenalty != -1) {
            arguments <<"-GAPEXT="+QString::number(settings.gapExtenstionPenalty);
        }
        if(settings.gapDist != -1) {
            arguments <<"-GAPDIST="+QString::number(settings.gapDist);
        }
        if(!settings.iterationType.isEmpty()){
            arguments<<"-ITERATION="+settings.iterationType;
            if(settings.numIterations != -1){
                arguments <<"-NUMITER="+QString::number(settings.numIterations);
            }
        }
        if(settings.outOrderInput){
            arguments << "-OUTORDER=INPUT";
        }else{
            arguments << "-OUTORDER=ALIGNED"; //this is default value in ClustalW, may be not needed set this option
        }
        if(!settings.matrix.isEmpty()){
            if((settings.matrix == "IUB")||(settings.matrix == "CLUSTALW")){
                arguments <<"-DNAMATRIX="+settings.matrix;
            }else{
                arguments <<"-MATRIX="+settings.matrix;
            }
        }
        if(settings.endGaps) arguments<<"-ENDGAPS";
        if(settings.noPGaps) arguments<<"-NOPGAP";
        if(settings.noHGaps) arguments<<"-NOHGAP";
        arguments << "-OUTFILE="+outputUrl;
        logParser=new ClustalWLogParser(inputMsa.getNumRows());
        clustalWTask=new ExternalToolRunTask(ET_CLUSTAL,arguments, logParser);
        setListenerForTask(clustalWTask);
        clustalWTask->setSubtaskProgressWeight(95);
        res.append(clustalWTask);
    }else if(subTask==clustalWTask){
        assert(logParser);
        delete logParser;
        if(!QFileInfo(outputUrl).exists()){
            if(AppContext::getExternalToolRegistry()->getByName(ET_CLUSTAL)->isValid()){
                stateInfo.setError(tr("Output file %1 not found").arg(outputUrl));
            }else{
                stateInfo.setError(tr("Output file %3 not found. May be %1 tool path '%2' not valid?")
                                   .arg(AppContext::getExternalToolRegistry()->getByName(ET_CLUSTAL)->getName())
                                   .arg(AppContext::getExternalToolRegistry()->getByName(ET_CLUSTAL)->getPath())
                                   .arg(outputUrl));
            }
            emit si_stateChanged();
            return res;
        }
        ioLog.details(tr("Loading output file '%1'").arg(outputUrl));
        loadTemporyDocumentTask=
                new LoadDocumentTask(BaseDocumentFormats::CLUSTAL_ALN,
                                     outputUrl,
                                     AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE));
        loadTemporyDocumentTask->setSubtaskProgressWeight(5);
        res.append(loadTemporyDocumentTask);
    }
    else if (subTask == loadTemporyDocumentTask) {
        tmpDoc = loadTemporyDocumentTask->takeDocument();
        SAFE_POINT(tmpDoc!=NULL, QString("output document '%1' not loaded").arg(tmpDoc->getURLString()), res);
        SAFE_POINT(tmpDoc->getObjects().length()==1, QString("no objects in output document '%1'").arg(tmpDoc->getURLString()), res);

        //move MAlignment from new alignment to old document
        MAlignmentObject* newMAligmentObject=qobject_cast<MAlignmentObject*>(tmpDoc->getObjects().first());
        SAFE_POINT(newMAligmentObject!=NULL, "newDocument->getObjects().first() is not a MAlignmentObject", res);

        resultMA=newMAligmentObject->getMAlignment();
        bool renamed = MSAUtils::restoreRowNames(resultMA, inputMsa.getRowNames());
        SAFE_POINT( renamed, "Failed to restore initial row names!", res);

        // If an alignment object has been specified, save the result to it
        if (objRef.isValid()) {
            GObject* obj = GObjectUtils::selectObjectByReference(objRef, UOF_LoadedOnly);
            if (NULL != obj) {
                MAlignmentObject* alObj = dynamic_cast<MAlignmentObject*>(obj);
                SAFE_POINT(NULL != alObj, "Failed to convert GObject to MAlignmentObject during applying ClustalW results!", res);

                if(!lock.isNull()) {
                    if(alObj->isStateLocked()) {
                        alObj->unlockState(lock);
                    }
                    delete lock;
                    lock = NULL;
                }
                else {
                    stateInfo.setError("MAlignment object has been changed");
                    return res;
                }
                
                QList<qint64> rowsOrder = MSAUtils::compareRowsAfterAlignment(inputMsa, resultMA, stateInfo);
                CHECK_OP(stateInfo, res);

                if (rowsOrder.count() != inputMsa.getNumRows()) {
                    stateInfo.setError("Unexpected number of rows in the result multiple alignment!");
                    return res;
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
                    U2UseCommonUserModStep userModStep(alObj->getEntityRef(), os);
                    if (os.hasError()) {
                        stateInfo.setError("Failed to apply the result of the alignment!");
                        return res;
                    }

                    alObj->updateGapModel(rowsGapModel, stateInfo);
                    SAFE_POINT_OP(stateInfo, res);

                    if (rowsOrder != inputMsa.getRowsIds()) {
                        alObj->updateRowsOrder(rowsOrder, stateInfo);
                        SAFE_POINT_OP(stateInfo, res);
                    }
                }

                Document* currentDocument = alObj->getDocument();
                SAFE_POINT(NULL != currentDocument, "Document is NULL!", res);
                currentDocument->setModified(true);
            }
            else {
                algoLog.error(tr("Failed to apply the result of ClustalW: alignment object is not available!"));
                return res;
            }
        }

        algoLog.info(tr("ClustalW alignment successfully finished"));
        //new document deleted in destructor of LoadDocumentTask
    }
    return res;
}
Task::ReportResult ClustalWSupportTask::report(){
    //Remove subdir for temporary files, that created in prepare
    if(!url.isEmpty()){
        QDir tmpDir(QFileInfo(url).absoluteDir());
        foreach(QString file, tmpDir.entryList()){
            tmpDir.remove(file);
        }
        if(!tmpDir.rmdir(tmpDir.absolutePath())){
            stateInfo.setError(tr("Can not remove directory for temporary files."));
            emit si_stateChanged();
        }
    }
    return ReportResult_Finished;
}

////////////////////////////////////////
//ClustalWWithExtFileSpecifySupportTask
ClustalWWithExtFileSpecifySupportTask::ClustalWWithExtFileSpecifySupportTask(const ClustalWSupportTaskSettings& _settings) :
        Task("Run ClustalW alignment task", TaskFlags_NR_FOSCOE),
        settings(_settings)
{
    GCOUNTER( cvar, tvar, "ClustalWSupportTask" );
    mAObject = NULL;
    currentDocument = NULL;
    saveDocumentTask = NULL;
    loadDocumentTask = NULL;
    clustalWSupportTask = NULL;
    cleanDoc = true;
}

ClustalWWithExtFileSpecifySupportTask::~ClustalWWithExtFileSpecifySupportTask(){
    if (cleanDoc) {
        delete currentDocument;
    }
}

void ClustalWWithExtFileSpecifySupportTask::prepare(){
    DocumentFormatConstraints c;
    c.checkRawData = true;
    c.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;
    c.rawData = IOAdapterUtils::readFileHeader(settings.inputFilePath);
    QList<DocumentFormatId> formats = AppContext::getDocumentFormatRegistry()->selectFormats(c);
    if (formats.isEmpty()) {
        stateInfo.setError(tr("input_format_error"));
        return;
    }

    DocumentFormatId alnFormat = formats.first();
    QVariantMap hints;
    if(alnFormat == BaseDocumentFormats::FASTA){
        hints[DocumentReadingMode_SequenceAsAlignmentHint] = true;
    }
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(settings.inputFilePath));
    loadDocumentTask = new LoadDocumentTask(alnFormat, settings.inputFilePath,iof, hints);

    addSubTask(loadDocumentTask);
}
QList<Task*> ClustalWWithExtFileSpecifySupportTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if(subTask->hasError()) {
        stateInfo.setError(subTask->getError());
        return res;
    }
    if (hasError() || isCanceled()) {
        return res;
    }
    if (subTask == loadDocumentTask){
        currentDocument=loadDocumentTask->takeDocument();
        SAFE_POINT(currentDocument != NULL, QString("Failed loading document: %1").arg(loadDocumentTask->getURLString()), res);
        SAFE_POINT(currentDocument->getObjects().length() == 1, QString("Number of objects != 1 : %1").arg(loadDocumentTask->getURLString()), res);
        mAObject = qobject_cast<MAlignmentObject*>(currentDocument->getObjects().first());
        SAFE_POINT(mAObject != NULL, QString("MA object not found!: %1").arg(loadDocumentTask->getURLString()), res);

        // Launch the task, objRef is empty - the input document maybe not in project
        clustalWSupportTask = new ClustalWSupportTask(mAObject->getMAlignment(), GObjectReference(), settings);
        res.append(clustalWSupportTask);
    }
    else if (subTask == clustalWSupportTask) {
        // Set the result alignment to the alignment object of the current document
        mAObject=qobject_cast<MAlignmentObject*>(currentDocument->getObjects().first());
        SAFE_POINT(mAObject != NULL, QString("MA object not found!: %1").arg(loadDocumentTask->getURLString()), res);
        mAObject->copyGapModel(clustalWSupportTask->resultMA.getRows());

        // Save the current document
        saveDocumentTask = new SaveDocumentTask(currentDocument,
            AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(settings.outputFilePath)),
            settings.outputFilePath);
        res.append(saveDocumentTask);
    }
    else if (subTask == saveDocumentTask) {
        Task* openTask = AppContext::getProjectLoader()->openWithProjectTask(settings.outputFilePath);
        if (openTask != NULL) {
            res << openTask;
        }
    }
    return res;
}
Task::ReportResult ClustalWWithExtFileSpecifySupportTask::report(){
    return ReportResult_Finished;
}

////////////////////////////////////////
//ClustalWLogParser
ClustalWLogParser::ClustalWLogParser(int _countSequencesInMSA) : ExternalToolLogParser(), countSequencesInMSA(_countSequencesInMSA){

}
int ClustalWLogParser::getProgress(){
    if(countSequencesInMSA <= 0){
        return -1;
    }
    if(!lastPartOfLog.isEmpty()){
        QString lastMessage=lastPartOfLog.last();
        //0..10% progresss
        if(lastMessage.contains(QRegExp("Sequence \\d+:"))){
            QRegExp rx("Sequence (\\d+):");
            rx.indexIn(lastMessage);
            CHECK(rx.captureCount() > 0, 0);
            return rx.cap(1).toInt()*10/countSequencesInMSA;
        }
        //10..90% progresss
        if(lastMessage.contains(QRegExp("Sequences \\(\\d+:\\d+\\)"))){
            QRegExp rx("Sequences \\((\\d+):\\d+\\)");
            rx.indexIn(lastMessage);
            CHECK(rx.captureCount() > 0, 0);
            return rx.cap(1).toInt()*80/countSequencesInMSA+10;
        }
        //90..100% progresss
        if(lastMessage.contains(QRegExp("Group \\d+:"))){
            QRegExp rx("Group (\\d+):");
            rx.indexIn(lastMessage);
            CHECK(rx.captureCount() > 0, 0);
            return rx.cap(1).toInt()*10/countSequencesInMSA+90;
        }

    }
    return 0;
}
}//namespace
