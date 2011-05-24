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

#include "MAFFTSupportTask.h"
#include "MAFFTSupport.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Counter.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MSAUtils.h>

#include <U2Core/AddDocumentTask.h>
#include <U2Gui/OpenViewTask.h>

namespace U2 {

void MAFFTSupportTaskSettings::reset() {
    gapExtenstionPenalty = -1;
    gapOpenPenalty = -1;
    maxNumberIterRefinement=0;
    inputFilePath="";
}

MAFFTSupportTask::MAFFTSupportTask(MAlignmentObject* _mAObject, const MAFFTSupportTaskSettings& _settings) :
        Task("Run MAFFT alignment task", TaskFlags_NR_FOSCOE),
        mAObject(_mAObject), settings(_settings)
{
    GCOUNTER( cvar, tvar, "MAFFTSupportTask" );
    currentDocument = mAObject->getDocument();
    saveTemporaryDocumentTask=NULL;
    loadTemporyDocumentTask=NULL;
    mAFFTTask=NULL;
    newDocument=NULL;
    logParser=NULL;
}

void MAFFTSupportTask::prepare(){
    //Add new subdir for temporary files
    //Directory name is ExternalToolName + CurrentDate + CurrentTime

    QString tmpDirName = "MAFFT_"+QString::number(this->getTaskId())+"_"+
                         QDate::currentDate().toString("dd.MM.yyyy")+"_"+
                         QTime::currentTime().toString("hh.mm.ss.zzz")+"_"+
                         QString::number(QCoreApplication::applicationPid())+"/";
    //Check and remove subdir for temporary files
    QDir tmpDir(AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath()+"/"+tmpDirName);
    if(tmpDir.exists()){
        foreach(QString file, tmpDir.entryList()){
            tmpDir.remove(file);
        }
        if(!tmpDir.rmdir(tmpDir.absolutePath())){
            stateInfo.setError(tr("Subdir for temporary files exists. Can not remove this directory."));
            emit si_stateChanged();
        }
    }
    if(!tmpDir.mkpath(AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath()+"/"+tmpDirName)){
        stateInfo.setError(tr("Can not create directory for temporary files."));
        emit si_stateChanged();
        return;
    }
    url=AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath() + "/" + tmpDirName + "tmp.fa";

    saveTemporaryDocumentTask=new SaveMSA2SequencesTask(mAObject->getMAlignment(), url, false, BaseDocumentFormats::PLAIN_FASTA);
    saveTemporaryDocumentTask->setSubtaskProgressWeight(5);
    addSubTask(saveTemporaryDocumentTask);
}
QList<Task*> MAFFTSupportTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if(subTask->hasError()) {
        if(subTask==loadTemporyDocumentTask){
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
    if(subTask==saveTemporaryDocumentTask){
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
        logParser=new MAFFTLogParser(mAObject->getMAlignment().getNumRows(), settings.maxNumberIterRefinement, url+".out.fa");
        mAFFTTask=new ExternalToolRunTask(MAFFT_TOOL_NAME, arguments, logParser);
        mAFFTTask->setSubtaskProgressWeight(95);
        res.append(mAFFTTask);
    }else if(subTask==mAFFTTask){
        assert(logParser);
        delete logParser;
        if(!QFileInfo(url+".out.fa").exists()){
            if(AppContext::getExternalToolRegistry()->getByName(MAFFT_TOOL_NAME)->isValid()){
                stateInfo.setError(tr("Output file not found"));
            }else{
                stateInfo.setError(tr("Output file not found. May be %1 tool path '%2' not valid?")
                                   .arg(AppContext::getExternalToolRegistry()->getByName(MAFFT_TOOL_NAME)->getName())
                                   .arg(AppContext::getExternalToolRegistry()->getByName(MAFFT_TOOL_NAME)->getPath()));
            }
            emit si_stateChanged();
            return res;
        }
        loadTemporyDocumentTask=
                new LoadDocumentTask(BaseDocumentFormats::PLAIN_FASTA,
                                     url+".out.fa",
                                     AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE));
        loadTemporyDocumentTask->setSubtaskProgressWeight(5);
        res.append(loadTemporyDocumentTask);
    }else if(subTask==loadTemporyDocumentTask){
        newDocument=loadTemporyDocumentTask->takeDocument();
        assert(newDocument!=NULL);

        //move MAlignment from new alignment to old document
        assert(newDocument->getObjects().length()!=0);
        QString err;
        resultMA = MSAUtils::seq2ma(newDocument->getObjects(), err);
        if (!err.isEmpty()) {
            stateInfo.setError(tr("Can not get multiple alignment from temporary file.")+err);
            emit si_stateChanged();
            return res;
        }
        mAObject->setMAlignment(resultMA);
        if(currentDocument != NULL){
            currentDocument->setModified(true);
        }
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
}

void MAFFTWithExtFileSpecifySupportTask::prepare(){
    DocumentFormatConstraints c;
    c.checkRawData = true;
    c.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;
    c.rawData = BaseIOAdapters::readFileHeader(settings.inputFilePath);
    QList<DocumentFormatId> formats = AppContext::getDocumentFormatRegistry()->selectFormats(c);
    if (formats.isEmpty()) {
        stateInfo.setError(  tr("input_format_error") );
        return;
    }

    DocumentFormatId alnFormat = formats.first();
    loadDocumentTask=
            new LoadDocumentTask(alnFormat,
                                 settings.inputFilePath,
                                 AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(settings.inputFilePath)));
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
    if(subTask==loadDocumentTask){
        currentDocument=loadDocumentTask->takeDocument();
        assert(currentDocument!=NULL);
        assert(currentDocument->getObjects().length()==1);
        mAObject=qobject_cast<MAlignmentObject*>(currentDocument->getObjects().first());
        assert(mAObject!=NULL);
        mAFFTSupportTask=new MAFFTSupportTask(mAObject,settings);
        res.append(mAFFTSupportTask);
    }else if(subTask == mAFFTSupportTask){
        saveDocumentTask = new SaveDocumentTask(currentDocument,AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(settings.inputFilePath)),settings.inputFilePath);
        res.append(saveDocumentTask);
    }else if(subTask==saveDocumentTask){
        Project* proj = AppContext::getProject();
        if (proj == NULL) {
            res.append(AppContext::getProjectLoader()->openProjectTask(currentDocument->getURLString(), false));
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
Task::ReportResult MAFFTWithExtFileSpecifySupportTask::report(){
    return ReportResult_Finished;
}

////////////////////////////////////////
//ClustalWLogParser
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
        if(buf.contains(QRegExp("\\d+ / ?\\d+"))
            ||buf.contains("WARNING")
            ||buf.contains(QRegExp("STEP \\d+"))
            ||buf.contains("rejected.")
            ||buf.contains("identical.")
            ||buf.contains("accepted.")){
            algoLog.trace(buf);
        }else{
            algoLog.info(buf);
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
