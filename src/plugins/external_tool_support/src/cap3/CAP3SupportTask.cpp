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

#include "CAP3SupportTask.h"
#include "CAP3Support.h"

#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/Counter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/CopyDataTask.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/IOAdapter.h>

#include <U2Formats/DNAQualityIOUtils.h>

#include <U2Gui/OpenViewTask.h>

namespace U2 {


//////////////////////////////////////////////////////////////////////////
////CAP3SupportTask

CAP3SupportTask::CAP3SupportTask(const CAP3SupportTaskSettings& _settings) :
        ExternalToolSupportTask("CAP3SupportTask", TaskFlags_NR_FOSCOE),
        settings(_settings)
{
    GCOUNTER( cvar, tvar, "CAP3SupportTask" );
    setMaxParallelSubtasks(1);
    newDoc = NULL;
    logParser = NULL;
    loadTmpDocumentTask = NULL;
    copyResultTask = NULL;
    cap3Task = NULL;
    prepareDataForCAP3Task = NULL;
    maObject = NULL;
}


void CAP3SupportTask::prepare(){
    
    
    //Add new subdir for temporary files
    
    tmpDirUrl = ExternalToolSupportUtils::createTmpDir(CAP3_TMP_DIR, stateInfo);
    CHECK_OP(stateInfo, );
    
    prepareDataForCAP3Task = new PrepareInputForCAP3Task(settings.inputFiles, tmpDirUrl);
    addSubTask(prepareDataForCAP3Task);

}


#define CAP3_EXT ".cap.ace"

QList<Task*> CAP3SupportTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    
    propagateSubtaskError();
    
    if(hasError() || isCanceled()) {
        return res;
    }
    
    if (subTask == prepareDataForCAP3Task) {
        assert(!prepareDataForCAP3Task->getPreparedPath().isEmpty());
        GUrl inputUrl = prepareDataForCAP3Task->getPreparedPath();
        if (prepareDataForCAP3Task->onlyCopyInputFiles()) {
            tmpOutputUrl = inputUrl.getURLString()+ CAP3_EXT;     
        } else {
            tmpOutputUrl = inputUrl.dirPath() + "/" + inputUrl.baseFileName() + CAP3_EXT; 
        }

        QStringList arguments = settings.getArgumentsList();
        arguments.prepend( inputUrl.getURLString() );
        logParser = new CAP3LogParser();
        cap3Task = new ExternalToolRunTask(ET_CAP3, arguments, logParser);
        setListenerForTask(cap3Task);
        cap3Task->setSubtaskProgressWeight(95);
        res.append(cap3Task);   
    } else if (subTask == cap3Task){
        if(!QFile::exists(tmpOutputUrl)){
            if(AppContext::getExternalToolRegistry()->getByName(ET_CAP3)->isValid()){
                stateInfo.setError(tr("Output file not found"));
            }else{
                stateInfo.setError(tr("Output file not found. May be %1 tool path '%2' not valid?")
                                   .arg(AppContext::getExternalToolRegistry()->getByName(ET_CAP3)->getName())
                                   .arg(AppContext::getExternalToolRegistry()->getByName(ET_CAP3)->getPath()));
            }
            return res;
        }
        
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        copyResultTask = new CopyDataTask(iof, tmpOutputUrl, iof, settings.outputFilePath );
        res.append(copyResultTask);
        
    } else if (subTask == copyResultTask) {
        
        if( !QFile::exists(settings.outputFilePath)) {
            stateInfo.setError(tr("Output file not found: copy from tmp dir failed."));
            return res;
        }

        Project * p = AppContext::getProject();
        Document * oldDoc = NULL;
        // If document already in project, just open view if needed, it will be updated automatically
        if(p != NULL && (oldDoc = p->findDocumentByURL(settings.outputFilePath)) != NULL) {
            if(settings.openView) {
                if(oldDoc->isLoaded()) {
                    res.append(new OpenViewTask(oldDoc));
                } else {
                    res.append(new LoadUnloadedDocumentAndOpenViewTask(oldDoc));
                }
            }
            return res;
        }

        loadTmpDocumentTask=
                new LoadDocumentTask(BaseDocumentFormats::ACE,
                                     settings.outputFilePath,
                                     AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE));
        loadTmpDocumentTask->setSubtaskProgressWeight(5);
        res.append(loadTmpDocumentTask);

    } else if( subTask == loadTmpDocumentTask  ) {
        
        Document* doc = loadTmpDocumentTask->takeDocument();
        SAFE_POINT(doc != NULL, "Failed loading result document", res);
        
        if (doc->getObjects().size() == 0) {
            QFile::remove(doc->getURLString());
            delete doc;
            setError(tr("No assembly is found for provided reads"));
            return res;
        }
        
        maObject = qobject_cast<MAlignmentObject*>( doc->getObjects().first() );
        if (settings.openView) {
            if (AppContext::getProject() == NULL) {
                res.append( AppContext::getProjectLoader()->createNewProjectTask() );
            }

            res.append(new AddDocumentAndOpenViewTask(doc));
        }
        
    }

    return res;

}


Task::ReportResult CAP3SupportTask::report() {
    U2OpStatus2Log os;
    ExternalToolSupportUtils::removeTmpDir(tmpDirUrl,os);
    return ReportResult_Finished;
}

//////////////////////////////////////////
////CAP3LogParser

CAP3LogParser::CAP3LogParser() {
}

int CAP3LogParser::getProgress() {
    return 0;
}


//////////////////////////////////////////
////PrepareInput

PrepareInputForCAP3Task::PrepareInputForCAP3Task( const QStringList& inputFiles, const QString& outputDirPath)
:Task("PrepareInputForCAP3Task", TaskFlags_FOSCOE), inputUrls(inputFiles), outputDir(outputDirPath), onlyCopyFiles(false)
{


}

void PrepareInputForCAP3Task::prepare() {
    
    if (inputUrls.size() == 1) {
        const QString& inputFileUrl = inputUrls.first();
        
        QList<FormatDetectionResult> results = DocumentUtils::detectFormat(inputFileUrl);
        
        if (!results.isEmpty()) {
            DocumentFormat* format = results.first().format;
            if (format->getFormatId() == BaseDocumentFormats::FASTA) {
                onlyCopyFiles = true;
            }
        }
    } 
    
    
    if (onlyCopyFiles) {    
        // short path: copy single FASTA file along with quality and constraints to target dir
        QString inputFileUrl = inputUrls.first();
        filesToCopy.append(inputFileUrl);
        QString inputFileUrlBase = GUrl(inputFileUrl).baseFileName();
        QString inputFileDir = GUrl(inputFileUrl).dirPath();
        QString qualFileUrl = inputFileDir + "/" + inputFileUrlBase + ".qual";
        if (QFile::exists(qualFileUrl)) {
            filesToCopy.append(qualFileUrl);        
        }
        QString constraintsFileUrl = inputFileDir+ "/" + inputFileUrlBase + ".con";
        if (QFile::exists(constraintsFileUrl)) {
            filesToCopy.append(qualFileUrl);
        }
        foreach (const QString& fileName, filesToCopy) {
            IOAdapterFactory* iof = 
                AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
            CopyDataTask* copyTask = new CopyDataTask(iof, fileName, iof, outputDir + "/" + GUrl(fileName).fileName());
            addSubTask(copyTask);
        }
        preparedPath = outputDir + "/" + GUrl(inputFileUrl).fileName();

    } else {
        // long path: load each file, save sequences and qualities to output dir
        
        QList<GUrl> inputGUrls;
        foreach( const QString& url, inputUrls) {
            inputGUrls.append(url);
        }
        
        if (!seqReader.init(inputGUrls)) {
            setError(seqReader.getErrorMessage());
            return;
        }
        
        QString outPath = outputDir + "/" + QString("%1_misc").arg(inputGUrls.first().baseFileName());
        qualityFilePath = outPath + ".qual";

        if (!seqWriter.init(outPath)) {
            setError(tr("Failed to initialize sequence writer."));
            return;
        }
        
    } 

}

void PrepareInputForCAP3Task::run()
{
    if (hasError() || onlyCopyFiles) {
        return;
    }
    
    while (seqReader.hasNext()) {
        if (isCanceled()) {
            return;
        }
        DNASequence* seq = seqReader.getNextSequenceObject();
        if (seq == NULL) {
            setError(seqReader.getErrorMessage());
            return;
        }
        // avoid names dublicating
        QByteArray seqName = seq->getName().toLatin1();
        seqName.replace(' ','_');
        seq->setName(seqName);
        bool ok = seqWriter.writeNextSequence(*seq);
        if (!ok) {
            setError(tr("Failed to write sequence %1").arg(seq->getName()));
            return;
        }
        
        if (!seq->quality.isEmpty()) {
            DNAQualityIOUtils::writeDNAQuality(seqName, seq->quality, qualityFilePath, true /*append*/, true /*decode*/, stateInfo );
            if (stateInfo.hasError()) {
                return;
            }
        }
    }
    
    preparedPath = seqWriter.getOutputPath().getURLString();
    seqWriter.close();


}


QStringList CAP3SupportTaskSettings::getArgumentsList()
{
    QStringList res;
    res += "-a"; res += QString("%1").arg(bandExpansionSize);
    res += "-b"; res += QString("%1").arg(baseQualityDiffCutoff);
    res += "-c"; res += QString("%1").arg(baseQualityClipCutoff);
    res += "-d"; res += QString("%1").arg(maxQScoreSum);
    res += "-f"; res += QString("%1").arg(maxGapLength);
    res += "-g"; res += QString("%1").arg(gapPenaltyFactor);
    res += "-m"; res += QString("%1").arg(matchScoreFactor);
    res += "-n"; res += QString("%1").arg(mismatchScoreFactor);
    res += "-o"; res += QString("%1").arg(overlapLengthCutoff);
    res += "-p"; res += QString("%1").arg(overlapPercentIdentetyCutoff);
    res += "-s"; res += QString("%1").arg(overlapSimilarityScoreCutoff);
    res += "-t"; res += QString("%1").arg(maxNumberOfWordMatches);
    res += "-y"; res += QString("%1").arg(clippingRange);
    
    return res;
}

}//namespace
