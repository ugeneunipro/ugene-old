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

#include "MAFFTSupportTask.h"
#include "MAFFTSupport.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Counter.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/OpenViewTask.h>

namespace U2 {

void MAFFTSupportTaskSettings::reset() {
    gapExtenstionPenalty = -1;
    gapOpenPenalty = -1;
    maxNumberIterRefinement=0;
    inputFilePath="";
}

MAFFTSupportTask::MAFFTSupportTask(const MAlignment& _inputMsa, const GObjectReference& _objRef, const MAFFTSupportTaskSettings& _settings)
    : Task("Run MAFFT alignment task", TaskFlags_NR_FOSCOE),
      inputMsa(_inputMsa),
      objRef(_objRef),
      settings(_settings)
{
    GCOUNTER( cvar, tvar, "MAFFTSupportTask" );
    saveTemporaryDocumentTask=NULL;
    loadTmpDocumentTask=NULL;
    mAFFTTask=NULL;
    tmpDoc=NULL;
    logParser=NULL;
    resultMA.setAlphabet(inputMsa.getAlphabet());
    resultMA.setName(inputMsa.getName());
}

MAFFTSupportTask::~MAFFTSupportTask() {
    if (NULL != tmpDoc) {
        delete tmpDoc;
    }
}

void MAFFTSupportTask::prepare(){
    algoLog.info(tr("MAFFT alignment started"));

    //Add new subdir for temporary files
    //Directory name is ExternalToolName + CurrentDate + CurrentTime

    QString tmpDirName = "MAFFT_"+QString::number(this->getTaskId())+"_"+
                         QDate::currentDate().toString("dd.MM.yyyy")+"_"+
                         QTime::currentTime().toString("hh.mm.ss.zzz")+"_"+
                         QString::number(QCoreApplication::applicationPid())+"/";
    QString tmpDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(MAFFT_TMP_DIR) + "/" + tmpDirName;
    url= tmpDirPath + "tmp.fa";
    ioLog.details(tr("Saving data to temporary file '%1'").arg(url));

    //Check and remove subdir for temporary files
    QDir tmpDir(tmpDirPath);
    if(tmpDir.exists()){
        foreach(QString file, tmpDir.entryList()){
            tmpDir.remove(file);
        }
        if(!tmpDir.rmdir(tmpDir.absolutePath())){
            stateInfo.setError(tr("Subdir for temporary files exists. Can not remove this directory."));
            return;
        }
    }
    if (!tmpDir.mkpath(tmpDirPath)){
        stateInfo.setError(tr("Can not create directory for temporary files."));
        return;
    }

    saveTemporaryDocumentTask=new SaveMSA2SequencesTask(inputMsa, url, false, BaseDocumentFormats::FASTA);
    saveTemporaryDocumentTask->setSubtaskProgressWeight(5);
    addSubTask(saveTemporaryDocumentTask);
}
QList<Task*> MAFFTSupportTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if(subTask->hasError()) {
        if(subTask==loadTmpDocumentTask){
            if(AppContext::getExternalToolRegistry()->getByName(MAFFT_TOOL_NAME)->isValid()){
                stateInfo.setError(tr("Can not open output file: ")+subTask->getError());
            }else{
                stateInfo.setError(tr("Can not open output file: ")+subTask->getError()
                                   +tr(" May be %1 tool path '%2' not valid?")
                                   .arg(AppContext::getExternalToolRegistry()->getByName(MAFFT_TOOL_NAME)->getName())
                                   .arg(AppContext::getExternalToolRegistry()->getByName(MAFFT_TOOL_NAME)->getPath()));
            }

        }else{
            stateInfo.setError(subTask->getError());
        }
        return res;
    }
    if(hasError() || isCanceled()) {
        return res;
    }
    QString outputUrl = url+".out.fa";
    if (subTask == saveTemporaryDocumentTask){
        QStringList arguments;
        if(settings.gapOpenPenalty != -1) {
            arguments <<"--op" << QString::number(settings.gapOpenPenalty);
        }
        if(settings.gapExtenstionPenalty != -1) {
            arguments <<"--ep"<<QString::number(settings.gapExtenstionPenalty);
        }
        if(settings.maxNumberIterRefinement!= -1) {
            arguments <<"--maxiterate"<<QString::number(settings.maxNumberIterRefinement);
        }
        arguments <<url;
        logParser = new MAFFTLogParser(inputMsa.getNumRows(), settings.maxNumberIterRefinement, outputUrl);
        mAFFTTask = new ExternalToolRunTask(MAFFT_TOOL_NAME, arguments, logParser);
        mAFFTTask->setSubtaskProgressWeight(95);
        res.append(mAFFTTask);
    } else if (subTask == mAFFTTask) {
        assert(logParser);
        delete logParser;
        if (!QFileInfo(outputUrl).exists()) {
            if (AppContext::getExternalToolRegistry()->getByName(MAFFT_TOOL_NAME)->isValid()){
                stateInfo.setError(tr("Output file '%1' not found").arg(outputUrl));
            } else {
                stateInfo.setError(tr("Output file '%3' not found. May be %1 tool path '%2' not valid?")
                                   .arg(AppContext::getExternalToolRegistry()->getByName(MAFFT_TOOL_NAME)->getName())
                                   .arg(AppContext::getExternalToolRegistry()->getByName(MAFFT_TOOL_NAME)->getPath())
                                   .arg(outputUrl));
            }
            emit si_stateChanged();
            return res;
        }
        ioLog.details(tr("Loading output file '%1'").arg(outputUrl));
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        loadTmpDocumentTask= new LoadDocumentTask(BaseDocumentFormats::FASTA, outputUrl, iof);
        loadTmpDocumentTask->setSubtaskProgressWeight(5);
        res.append(loadTmpDocumentTask);
    } else if (subTask == loadTmpDocumentTask) {
        tmpDoc=loadTmpDocumentTask->takeDocument();
        SAFE_POINT(tmpDoc!=NULL, QString("output document '%1' not loaded").arg(tmpDoc->getURLString()), res);
        SAFE_POINT(tmpDoc->getObjects().length()!=0, QString("no objects in output document '%1'").arg(tmpDoc->getURLString()), res);

        // Get the result alignment
        resultMA = MSAUtils::seq2ma(tmpDoc->getObjects(), stateInfo);
        if (hasError()) {
            emit si_stateChanged(); //TODO: task can't emit this signal!
            return res;
        }

        // If an alignment object has been specified, save the result to it
        if (objRef.isValid()) {
            GObject* obj = GObjectUtils::selectObjectByReference(objRef, UOF_LoadedOnly);
            if (NULL != obj) {
                MAlignmentObject* alObj = dynamic_cast<MAlignmentObject*>(obj);
                SAFE_POINT(NULL != alObj, "Failed to convert GObject to MAlignmentObject during applying MAFFT results!", res);

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

                alObj->updateGapModel(rowsGapModel, stateInfo);

                if (rowsOrder != inputMsa.getRowsIds()) {
                    alObj->updateRowsOrder(rowsOrder, stateInfo);
                }

                Document* currentDocument = alObj->getDocument();
                SAFE_POINT(NULL != currentDocument, "Document is NULL!", res);
                currentDocument->setModified(true);
            }
            else {
                algoLog.error(tr("Failed to apply the result of aligning with MAFFT: alignment object is not available!"));
                return res;
            }
        }

        algoLog.info(tr("MAFFT alignment successfully finished"));
        //new document deleted in destructor of LoadDocumentTask
    }
    return res;
}
Task::ReportResult MAFFTSupportTask::report(){
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
//MAFFTWithExtFileSpecifySupportTask
MAFFTWithExtFileSpecifySupportTask::MAFFTWithExtFileSpecifySupportTask(const MAFFTSupportTaskSettings& _settings) :
        Task("Run MAFFT alignment task", TaskFlags_NR_FOSCOE),
        settings(_settings)
{
    GCOUNTER( cvar, tvar, "MAFFTSupportTask" );
    mAObject = NULL;
    currentDocument = NULL;
    saveDocumentTask = NULL;
    loadDocumentTask = NULL;
    mAFFTSupportTask = NULL;
    cleanDoc = true;
}
MAFFTWithExtFileSpecifySupportTask::~MAFFTWithExtFileSpecifySupportTask(){
    if (cleanDoc){
        delete currentDocument;
    }
}
void MAFFTWithExtFileSpecifySupportTask::prepare(){
    DocumentFormatConstraints c;
    c.checkRawData = true;
    c.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;
    c.rawData = IOAdapterUtils::readFileHeader(settings.inputFilePath);
    QList<DocumentFormatId> formats = AppContext::getDocumentFormatRegistry()->selectFormats(c);
    if (formats.isEmpty()) {
        stateInfo.setError(  tr("input_format_error") );
        return;
    }

    DocumentFormatId alnFormat = formats.first();
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(settings.inputFilePath));
    QVariantMap hints;
    if(alnFormat == BaseDocumentFormats::FASTA){
        hints[DocumentReadingMode_SequenceAsAlignmentHint] = true;
    }
    loadDocumentTask = new LoadDocumentTask(alnFormat, settings.inputFilePath, iof, hints);
    addSubTask(loadDocumentTask);
}
QList<Task*> MAFFTWithExtFileSpecifySupportTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if(subTask->hasError()) {
        stateInfo.setError(subTask->getError());
        return res;
    }
    if(hasError() || isCanceled()) {
        return res;
    }
    if (subTask == loadDocumentTask) {
        currentDocument = loadDocumentTask->takeDocument();
        SAFE_POINT(currentDocument != NULL, QString("Failed loading document: %1").arg(loadDocumentTask->getURLString()), res);
        SAFE_POINT(currentDocument->getObjects().length() == 1, QString("Number of objects != 1 : %1").arg(loadDocumentTask->getURLString()), res);
        mAObject=qobject_cast<MAlignmentObject*>(currentDocument->getObjects().first());
        SAFE_POINT(mAObject != NULL, QString("MA object not found!: %1").arg(loadDocumentTask->getURLString()), res);

        // Launch the task, objRef is empty - the input document maybe not in project
        mAFFTSupportTask = new MAFFTSupportTask(mAObject->getMAlignment(), GObjectReference(), settings);
        res.append(mAFFTSupportTask);
    }
    else if (subTask == mAFFTSupportTask) {
        // Set the result alignment to the alignment object of the current document
        mAObject=qobject_cast<MAlignmentObject*>(currentDocument->getObjects().first());
        SAFE_POINT(mAObject != NULL, QString("MA object not found!: %1").arg(loadDocumentTask->getURLString()), res);
        mAObject->copyGapModel(mAFFTSupportTask->resultMA.getRows());

        // Save the current document
        saveDocumentTask = new SaveDocumentTask(currentDocument,
            AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(settings.outputFilePath)),
            settings.outputFilePath);
        res.append(saveDocumentTask);
    }
    else if (subTask == saveDocumentTask) {
        Task* openTask = AppContext::getProjectLoader()->openWithProjectTask(settings.outputFilePath);
        res << openTask;
    }
    return res;
}
Task::ReportResult MAFFTWithExtFileSpecifySupportTask::report(){
    return ReportResult_Finished;
}

////////////////////////////////////////
//MAFFTLogParser
MAFFTLogParser::MAFFTLogParser(int _countSequencesInMSA, int _countRefinementIter, const QString& _outputFileName) :
        countSequencesInMSA(_countSequencesInMSA),
        countRefinementIter(_countRefinementIter),
        outputFileName(_outputFileName)
{
    isOutputFileCreated=false;
    firstDistanceMatrix=false;
    firstUPGMATree=false;
    firstProAlign=false;
    outFile.setFileName(outputFileName);
    if(outFile.open(QIODevice::WriteOnly)){
        isOutputFileCreated=true;
    }
}

void MAFFTLogParser::parseOutput(const QString& partOfLog){
    outFile.write(partOfLog.toAscii());
}

void MAFFTLogParser::parseErrOutput(const QString& partOfLog){
    lastPartOfLog=partOfLog.split(QRegExp("(\n|\r)"));
    lastPartOfLog.first()=lastErrLine+lastPartOfLog.first();
    lastErrLine=lastPartOfLog.takeLast();
    foreach(QString buf, lastPartOfLog){
        if(buf.contains("WARNING")
            ||buf.contains("rejected.")
            ||buf.contains("identical.")
            ||buf.contains("accepted.")){
            algoLog.info("MAFFT: " + buf);
        }else if(!buf.isEmpty()){
            algoLog.trace("MAFFT: " + buf);
        }
    }
}

int MAFFTLogParser::getProgress(){
    /*
     Making a distance matrix - 5%
     Constructing a UPGMA tree - 10%
     Progressive alignment - 15%
     Making a distance matrix - 20%
     Constructing a UPGMA tree - 25%
     Progressive alignment - 30-80%
     STEP 001-002-3 - 80-100%
    */
    if(!lastPartOfLog.isEmpty()){
        foreach(QString buf, lastPartOfLog){
            if(buf.contains("Making")){
                if(firstDistanceMatrix){
                    progress=20;
                }else{
                    firstDistanceMatrix=true;
                    progress=5;
                }
            }
            if(buf.contains("Constructing")){
                if(firstUPGMATree){
                    progress=25;
                }else{
                    firstUPGMATree=true;
                    progress=10;
                }
            }
            if(buf.contains("Progressive")){
                if(!firstProAlign){
                    firstProAlign=true;
                    progress=15;
                }
            }
        }
        if(firstProAlign&&firstUPGMATree&&firstDistanceMatrix){
            QString lastMessage=lastPartOfLog.last();
            if(lastMessage.contains(QRegExp("STEP +\\d+ /"))){
                QRegExp rx("STEP +(\\d+) /");
                assert(rx.indexIn(lastMessage)>-1);
                rx.indexIn(lastMessage);
                progress=rx.cap(1).toInt()*50/countSequencesInMSA+30;
            }else if(lastMessage.contains(QRegExp("STEP +\\d+-"))){
                QRegExp rx("STEP +(\\d+)-");
                assert(rx.indexIn(lastMessage)>-1);
                rx.indexIn(lastMessage);
                progress=rx.cap(1).toInt()*20/countRefinementIter+80;
            }
        }
    }
    return progress;
}
}//namespace
