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

#include "TCoffeeSupportTask.h"
#include "TCoffeeSupport.h"

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
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/OpenViewTask.h>

namespace U2 {

void TCoffeeSupportTaskSettings::reset() {
    gapExtenstionPenalty = -1;
    gapOpenPenalty = -1;
    numIterations=0;
    inputFilePath="";
}

TCoffeeSupportTask::TCoffeeSupportTask(MAlignmentObject* _mAObject, const TCoffeeSupportTaskSettings& _settings) :
        Task("Run T-Coffee alignment task", TaskFlags_NR_FOSCOE),
        mAObject(_mAObject), settings(_settings)
{
    GCOUNTER( cvar, tvar, "TCoffeeSupportTask" );
    currentDocument = mAObject->getDocument();
    saveTemporaryDocumentTask=NULL;
    loadTmpDocumentTask=NULL;
    tCoffeeTask=NULL;
    newDocument=NULL;
    logParser=NULL;
}

void TCoffeeSupportTask::prepare(){
    algoLog.info(tr("T-Coffee alignment started"));

    //Add new subdir for temporary files
    //Directory name is ExternalToolName + CurrentDate + CurrentTime

    QString tmpDirName = "TCoffee_"+QString::number(this->getTaskId())+"_"+
                         QDate::currentDate().toString("dd.MM.yyyy")+"_"+
                         QTime::currentTime().toString("hh.mm.ss.zzz")+"_"+
                         QString::number(QCoreApplication::applicationPid())+"/";
    url=AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath() + "/" + tmpDirName + "tmp.fa";
    ioLog.details(tr("Saving data to temporary file '%1'").arg(url));

    //Check and remove subdir for temporary files
    QDir tmpDir(AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath()+"/"+tmpDirName);
    if(tmpDir.exists()){
        foreach(QString file, tmpDir.entryList()){
            tmpDir.remove(file);
        }
        if(!tmpDir.rmdir(tmpDir.absolutePath())){
            stateInfo.setError(tr("Subdir for temporary files exists. Can not remove this directory."));
            return;
        }
    }
    if(!tmpDir.mkpath(AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath()+"/"+tmpDirName)){
        stateInfo.setError(tr("Can not create directory for temporary files."));
        return;
    }

    saveTemporaryDocumentTask=new SaveMSA2SequencesTask(mAObject->getMAlignment(), url, false, BaseDocumentFormats::PLAIN_FASTA);
    saveTemporaryDocumentTask->setSubtaskProgressWeight(5);
    addSubTask(saveTemporaryDocumentTask);
}
QList<Task*> TCoffeeSupportTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if(subTask->hasError()) {
        stateInfo.setError(subTask->getError());
        return res;
    }
    if(hasError() || isCanceled()) {
        return res;
    }
    QString outputUrl = url+".msf";
    if(subTask==saveTemporaryDocumentTask){
        QStringList arguments;
        if(url.contains(" ")){
            stateInfo.setError("Temporary directory path have space(s). Try select any other directory without spaces.");
            return res;
        }
        arguments <<url;
        arguments <<"-output"<<"msf";
        if(settings.gapOpenPenalty != -1) {
            arguments <<"-gapopen" << QString::number(settings.gapOpenPenalty);
        }
        if(settings.gapExtenstionPenalty != -1) {
            arguments <<"-gapext"<<QString::number(settings.gapExtenstionPenalty);
        }
        if(settings.numIterations!= -1) {
            arguments <<"-iterate"<<QString::number(settings.numIterations);
        }
        arguments <<"-outfile"<<outputUrl;
        logParser=new TCoffeeLogParser();
        tCoffeeTask=new ExternalToolRunTask(TCOFFEE_TOOL_NAME, arguments, logParser);
        tCoffeeTask->setSubtaskProgressWeight(95);
        res.append(tCoffeeTask);
    }else if(subTask==tCoffeeTask){
        assert(logParser);
        delete logParser;
        if(!QFileInfo(outputUrl).exists()){
            if(AppContext::getExternalToolRegistry()->getByName(TCOFFEE_TOOL_NAME)->isValid()){
                stateInfo.setError(tr("Output file %1 not found").arg(outputUrl));
            }else{
                stateInfo.setError(tr("Output file %3 not found. May be %1 tool path '%2' not valid?")
                                   .arg(AppContext::getExternalToolRegistry()->getByName(TCOFFEE_TOOL_NAME)->getName())
                                   .arg(AppContext::getExternalToolRegistry()->getByName(TCOFFEE_TOOL_NAME)->getPath())
                                   .arg(outputUrl));
            }
            emit si_stateChanged();
            return res;
        }
        ioLog.details(tr("Loading output file '%1'").arg(outputUrl));
        
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        QVariantMap hints;
        hints[DocumentReadingMode_SequenceAsAlignmentHint] = true;
        loadTmpDocumentTask = new LoadDocumentTask(BaseDocumentFormats::MSF, outputUrl, iof, hints);
                        
        loadTmpDocumentTask->setSubtaskProgressWeight(5);
        res.append(loadTmpDocumentTask);
    } else if (subTask==loadTmpDocumentTask) {
        newDocument=loadTmpDocumentTask->takeDocument();
        SAFE_POINT(newDocument!=NULL, QString("output document '%1' not loaded").arg(newDocument->getURLString()), res);
        SAFE_POINT(newDocument->getObjects().length()!=0, QString("no objects in output document '%1'").arg(newDocument->getURLString()), res);

        //move MAlignment from new alignment to old document
        MAlignmentObject* newMAligmentObject=qobject_cast<MAlignmentObject*>(newDocument->getObjects().first());
        assert(newMAligmentObject!=NULL);
        resultMA=newMAligmentObject->getMAlignment();
        mAObject->setMAlignment(resultMA);
        if (currentDocument != NULL){
            currentDocument->setModified(true);
        }
        algoLog.info(tr("T-Coffee alignment successfully finished"));
        //new document deleted in destructor of LoadDocumentTask
    }
    return res;
}
Task::ReportResult TCoffeeSupportTask::report(){
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
//TCoffeeWithExtFileSpecifySupportTask
TCoffeeWithExtFileSpecifySupportTask::TCoffeeWithExtFileSpecifySupportTask(const TCoffeeSupportTaskSettings& _settings) :
        Task("Run MAFFT alignment task", TaskFlags_NR_FOSCOE),
        settings(_settings)
{
    GCOUNTER( cvar, tvar, "TCoffeeSupportTask" );
    mAObject = NULL;
    currentDocument = NULL;
    saveDocumentTask = NULL;
    loadDocumentTask = NULL;
    tCoffeeSupportTask = NULL;
    cleanDoc = true;
}
TCoffeeWithExtFileSpecifySupportTask::~TCoffeeWithExtFileSpecifySupportTask(){
    if (cleanDoc){
        delete currentDocument;
    }
}
void TCoffeeWithExtFileSpecifySupportTask::prepare(){
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
    hints[DocumentReadingMode_SequenceAsAlignmentHint] = true;
    loadDocumentTask = new LoadDocumentTask(alnFormat, settings.inputFilePath, iof, hints);
    addSubTask(loadDocumentTask);
}
QList<Task*> TCoffeeWithExtFileSpecifySupportTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if (subTask->hasError()) {
        stateInfo.setError(subTask->getError());
        return res;
    }
    if (hasError() || isCanceled()) {
        return res;
    }
    if (subTask==loadDocumentTask){
        currentDocument=loadDocumentTask->getDocument()->clone();
        assert(currentDocument!=NULL);
        assert(currentDocument->getObjects().length()==1);
        mAObject=qobject_cast<MAlignmentObject*>(currentDocument->getObjects().first());
        assert(mAObject!=NULL);
        tCoffeeSupportTask=new TCoffeeSupportTask(mAObject,settings);
        res.append(tCoffeeSupportTask);
    } else if (subTask == tCoffeeSupportTask){
        saveDocumentTask = new SaveDocumentTask(currentDocument,AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(settings.inputFilePath)),settings.inputFilePath);
        res.append(saveDocumentTask);
    } else if (subTask==saveDocumentTask){
        Project* proj = AppContext::getProject();
        if (proj == NULL) {
            res.append(AppContext::getProjectLoader()->openWithProjectTask(currentDocument->getURL(), currentDocument->getGHintsMap()));
        } else {
            Document* projDoc = proj->findDocumentByURL(currentDocument->getURL());
            if (projDoc != NULL) {
                projDoc->setLastUpdateTime();
                res.append(new LoadUnloadedDocumentAndOpenViewTask(projDoc));
            } else {
                // Add document to project
                res.append(new AddDocumentAndOpenViewTask(currentDocument));
                cleanDoc = false;
            }
        }
    }
    return res;
}
Task::ReportResult TCoffeeWithExtFileSpecifySupportTask::report(){
    return ReportResult_Finished;
}

////////////////////////////////////////
//TCoffeeLogParser
TCoffeeLogParser::TCoffeeLogParser()
{
    progress=0;
}

void TCoffeeLogParser::parseOutput(const QString& partOfLog){
    Q_UNUSED(partOfLog)
}

void TCoffeeLogParser::parseErrOutput(const QString& partOfLog){
    lastPartOfLog=partOfLog.split(QRegExp("(\n|\r)"));
    lastPartOfLog.first()=lastErrLine+lastPartOfLog.first();
    lastErrLine=lastPartOfLog.takeLast();
    foreach(QString buf, lastPartOfLog){
        if(buf.contains("WARNING")){
            algoLog.info("MAFFT: " + buf);
        }else{
            algoLog.trace(buf);
        }
    }
}

int TCoffeeLogParser::getProgress(){
    /* parse progress string:
        [Submit   Job][TOT= 18][ 22 %][ELAPSED TIME: 0 sec.]
    */
    if(!lastPartOfLog.isEmpty()){
        QString lastMessage=lastPartOfLog.last();
        if(lastMessage.contains(QRegExp("\\[Submit +Job\\]\\[TOT="))){
            QRegExp rx("(.*)\\[ +(\\d+) %\\](.*)");
            assert(rx.indexIn(lastMessage)>-1);
            rx.indexIn(lastMessage);
            return rx.cap(2).toInt();
        }
    }
    return progress;
}
}//namespace
