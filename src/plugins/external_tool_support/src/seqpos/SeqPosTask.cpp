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

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/TextObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include "SeqPosSupport.h"
#include "R/RSupport.h"

#include "SeqPosTask.h"

namespace U2 {

const QString SeqPosTask::BASE_DIR_NAME("SeqPos_tmp");
const QString SeqPosTask::BASE_SUBDIR_NAME("SeqPos");
const QString SeqPosTask::TREAT_NAME("treatment");

SeqPosTask::SeqPosTask(const SeqPosSettings& _settings, const QList<SharedAnnotationData>& _treatAnn)
: ExternalToolSupportTask("SeqPos annotation", TaskFlag_None)
, settings(_settings)
, treatAnn(_treatAnn)
, treatDoc(NULL)
, treatTask(NULL)
, etTask(NULL)
, logParser(NULL)
{

}

SeqPosTask::~SeqPosTask() {
    cleanup();
}

void SeqPosTask::cleanup() {
    treatAnn.clear();

    delete treatDoc; treatDoc = NULL;
    delete logParser; logParser = NULL;

    //remove tmp files
    QString tmpDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(BASE_DIR_NAME);
    QDir tmpDir(tmpDirPath);
    if(tmpDir.exists()){
        foreach(QString file, tmpDir.entryList()){
            tmpDir.remove(file);
        }
        if(!tmpDir.rmdir(tmpDir.absolutePath())){
            //stateInfo.setError(tr("Subdir for temporary files exists. Can not remove this directory."));
            //return;
        }
    }
}

void SeqPosTask::prepare() {
    UserAppsSettings *appSettings = AppContext::getAppSettings()->getUserAppsSettings();
    workingDir = appSettings->createCurrentProcessTemporarySubDir(stateInfo, BASE_DIR_NAME);
    CHECK_OP(stateInfo, );

    settings.outDir = GUrlUtils::createDirectory(
        settings.outDir + "/" + BASE_SUBDIR_NAME,
        "_", stateInfo);
    CHECK_OP(stateInfo, );


    treatDoc = createDoc(treatAnn, TREAT_NAME);
    CHECK_OP(stateInfo, );

    treatTask = new SaveDocumentTask(treatDoc);
    addSubTask(treatTask);
    
}


Document* SeqPosTask::createDoc( const QList<SharedAnnotationData>& annData, const QString& name){
    Document* doc = NULL;

    QString docUrl = workingDir + "/" + name +".bed";

    DocumentFormat *bedFormat = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::BED);
    CHECK_EXT(NULL != bedFormat, stateInfo.setError("NULL bed format"), doc);

    doc = bedFormat->createNewLoadedDocument(
        IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), docUrl, stateInfo);
    CHECK_OP(stateInfo, doc);
    doc->setDocumentOwnsDbiResources(false);

    AnnotationTableObject *ato = new AnnotationTableObject(name);
    foreach (const SharedAnnotationData &sad, annData) {
        ato->addAnnotation(new Annotation(sad), QString());
    }
    doc->addObject(ato);   

    return doc;
}


QList<Task*> SeqPosTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> result;
    CHECK(!subTask->isCanceled(), result);
    CHECK(!subTask->hasError(), result);

    if (treatTask == subTask) {
            QStringList args = settings.getArguments(treatDoc->getURLString());
            
            logParser = new SeqPosLogParser();
            ExternalTool* rTool = AppContext::getExternalToolRegistry()->getByName(ET_R);
            SAFE_POINT(NULL != rTool, "R script tool wasn't found in the registry", result);


            etTask = new ExternalToolRunTask(ET_SEQPOS, args, logParser, getSettings().outDir, QStringList() << rTool->getPath());
            setListenerForTask(etTask);
            result << etTask;
    }
    return result;
}

void SeqPosTask::run() {

}

const SeqPosSettings& SeqPosTask::getSettings(){
    return settings;
}

QStringList SeqPosTask::getOutputFiles(){
    QStringList result;

    QString current;

    current = getSettings().outDir + "/results/" + "mdseqpos_index.html";
    if (QFile::exists(current)){
        result << current;
    }
    current = getSettings().outDir + "/results/" + "table.html";
    if (QFile::exists(current)){
        result << current;
    }


    return result;
}

//////////////////////////////////////////////////////////////////////////
//SeqPosLogParser
SeqPosLogParser::SeqPosLogParser()
:ExternalToolLogParser(){

}
} // U2
