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

#include "CAP3SupportTask.h"
#include "CAP3Support.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Counter.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/MAlignmentObject.h>

#include <U2Core/AddDocumentTask.h>
#include <U2Gui/OpenViewTask.h>

namespace U2 {


//////////////////////////////////////////////////////////////////////////
////CAP3SupportTask

CAP3SupportTask::CAP3SupportTask(const CAP3SupportTaskSettings& _settings) :
        Task("CAP3SupportTask", TaskFlags_NR_FOSCOE),
        settings(_settings)
{
   GCOUNTER( cvar, tvar, "CAP3SupportTask" );
   setMaxParallelSubtasks(1);
   newDoc = NULL;
   logParser = NULL;
   loadTmpDocumentTask = NULL;
}


void CAP3SupportTask::prepare(){
    //Add new subdir for temporary files
    //Directory name is ExternalToolName + CurrentDate + CurrentTime
    /*
    QString tmpDirName = "CAP3_"+QString::number(this->getTaskId())+"_"+
                         QDate::currentDate().toString("dd.MM.yyyy")+"_"+
                         QTime::currentTime().toString("hh.mm.ss.zzz")+"_"+
                         QString::number(QCoreApplication::applicationPid())+"/";
    //Check and remove subdir for temporary files
    
    QDir tmpDir(AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath() + "/" + tmpDirName);
    if(tmpDir.exists()){
        foreach(const QString& file, tmpDir.entryList()){
            tmpDir.remove(file);
        }
        if(!tmpDir.rmdir(tmpDir.absolutePath())){
            stateInfo.setError(tr("Subdirectory for temporary files exists. Can not remove this directory."));
            return;
        }
    }

    if(!tmpDir.mkpath(AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath()+"/"+tmpDirName)){
        stateInfo.setError(tr("Can not create directory for temporary files."));
        return;
    }
    */
    //url = AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath() + "/" + tmpDirName + "tmp.out";
    //saveTemporaryDocumentTask = new SaveAlignmentTask(mAObject->getMAlignment(), url, BaseDocumentFormats::CLUSTAL_ALN);
    //saveTemporaryDocumentTask->setSubtaskProgressWeight(5);
    //addSubTask(saveTemporaryDocumentTask);
    
    QStringList arguments;
    arguments << settings.inputFilePath;
    outputUrl = settings.outputFilePath;
    logParser = new CAP3LogParser();
    cap3Task = new ExternalToolRunTask(CAP3_TOOL_NAME, arguments, logParser);
    cap3Task->setSubtaskProgressWeight(95);
    addSubTask(cap3Task);

}



QList<Task*> CAP3SupportTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    
    propagateSubtaskError();
    
    if(hasError() || isCanceled()) {
        return res;
    }
    
    if (subTask == cap3Task){
        if(!QFileInfo(outputUrl).exists()){
            if(AppContext::getExternalToolRegistry()->getByName(CAP3_TOOL_NAME)->isValid()){
                stateInfo.setError(tr("Output file not found"));
            }else{
                stateInfo.setError(tr("Output file not found. May be %1 tool path '%2' not valid?")
                                   .arg(AppContext::getExternalToolRegistry()->getByName(CAP3_TOOL_NAME)->getName())
                                   .arg(AppContext::getExternalToolRegistry()->getByName(CAP3_TOOL_NAME)->getPath()));
            }
            return res;
        }
        loadTmpDocumentTask=
                new LoadDocumentTask(BaseDocumentFormats::ACE,
                                     outputUrl,
                                     AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE));
        loadTmpDocumentTask->setSubtaskProgressWeight(5);
        res.append(loadTmpDocumentTask);

    }else  if( subTask == loadTmpDocumentTask ) {
        Document* doc = loadTmpDocumentTask->takeDocument();
        assert(doc != NULL);
        // clone doc because it was created in another thread
        const GUrl& fullPath = doc->getURL();
        DocumentFormat* format = doc->getDocumentFormat();
        IOAdapterFactory * iof = doc->getIOAdapterFactory();
        Document* clonedDoc = new Document(format, iof, fullPath);
        clonedDoc->loadFrom(doc);
        assert(!clonedDoc->isTreeItemModified());
        assert(clonedDoc->isLoaded());
        res.append(new AddDocumentTask(clonedDoc));
        res.append(new OpenViewTask(clonedDoc));     
        
    }

    return res;

}


Task::ReportResult CAP3SupportTask::report(){
    //Remove subdir for temporary files, that created in prepare
//     if(!url.isEmpty()){
//         QDir tmpDir(QFileInfo(url).absoluteDir());
//         foreach(QString file, tmpDir.entryList()){
//             tmpDir.remove(file);
//         }
//         if(!tmpDir.rmdir(tmpDir.absolutePath())){
//             stateInfo.setError(tr("Can not remove directory for temporary files."));
//             emit si_stateChanged();
//         }
//     }

    return ReportResult_Finished;
}

//////////////////////////////////////////
////CAP3LogParser

CAP3LogParser::CAP3LogParser()
{

}

int CAP3LogParser::getProgress()
{
    return 0;
}


//////////////////////////////////////////
////PrepareInput

PrepareInputForCAP3Task::PrepareInputForCAP3Task( const QStringList&, const QString&)
:Task("PrepareInputForCAP3Task", TaskFlags_NR_FOSCOE)
{

}

void PrepareInputForCAP3Task::prepare() {
    //TODO:
}

QList<Task*> PrepareInputForCAP3Task::onSubTaskFinished(Task* ) {
    QList<Task*> res;
    //TODO:
    return res;

}







}//namespace
