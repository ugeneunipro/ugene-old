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

#include "ConductGOSupport.h"
#include "R/RSupport.h"

#include "ConductGOTask.h"

namespace U2 {

const QString ConductGOTask::BASE_DIR_NAME("ConductGO_tmp");
const QString ConductGOTask::BASE_SUBDIR_NAME("ConductGO");
const QString ConductGOTask::TREAT_NAME("treatment");

ConductGOTask::ConductGOTask(const ConductGOSettings& _settings, const QList<AnnotationData>& _treatAnn)
: ExternalToolSupportTask("ConductGO annotation", TaskFlag_None)
, settings(_settings)
, treatAnn(_treatAnn)
, treatDoc(NULL)
, treatTask(NULL)
, etTask(NULL)
, logParser(NULL)
{

}

ConductGOTask::~ConductGOTask() {
    cleanup();
}

void ConductGOTask::cleanup() {
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
    }
}

void ConductGOTask::prepare() {
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

Document* ConductGOTask::createDoc( const QList<AnnotationData>& annData, const QString& name){
    Document* doc = NULL;

    QString docUrl = workingDir + "/" + name +".bed";

    DocumentFormat *bedFormat = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::BED);
    CHECK_EXT(NULL != bedFormat, stateInfo.setError("NULL bed format"), doc);

    doc = bedFormat->createNewLoadedDocument(
        IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), docUrl, stateInfo);
    CHECK_OP(stateInfo, doc);
    doc->setDocumentOwnsDbiResources(false);

    AnnotationTableObject *ato = new AnnotationTableObject( name, doc->getDbiRef( ) );
    foreach (const AnnotationData &ad, annData) {
        ato->addAnnotation( ad, QString( ) );
    }
    doc->addObject(ato);

    return doc;
}

QList<Task*> ConductGOTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> result;
    CHECK(!subTask->isCanceled(), result);
    CHECK(!subTask->hasError(), result);

    if (treatTask == subTask) {
            QStringList args = settings.getArguments(treatDoc->getURLString());
            
            logParser = new ConductGOLogParser();

            ExternalTool* rTool = AppContext::getExternalToolRegistry()->getByName(ET_R);
            SAFE_POINT(NULL != rTool, "R script tool wasn't found in the registry", result);

            etTask = new ExternalToolRunTask(ET_GO_ANALYSIS, args, logParser, getSettings().outDir, QStringList() << rTool->getPath());
            setListenerForTask(etTask);
            result << etTask;
    }
    return result;
}

void ConductGOTask::run() {
    foreach(const QString& file, getResultFileNames()){
        copyFile(workingDir+"/"+ file, getSettings().outDir + "/" + file);
    }
}

const ConductGOSettings& ConductGOTask::getSettings(){
    return settings;
}

bool ConductGOTask::copyFile(const QString &src, const QString &dst) {
    if (!QFile::exists(src)) {
        coreLog.trace(tr("Conduct GO warning: Can not find a required output file %1.").arg(src));
        return false;
    }

    QSet<QString> excludeFileNames = DocumentUtils::getNewDocFileNameExcludesHint();
    if (!GUrlUtils::renameFileWithNameRoll(dst, stateInfo, excludeFileNames, &taskLog)) {
        return false;
    }

    bool copied = QFile::copy(src, dst);
    if (!copied) {
        setError(tr("Can not copy the result file to: %1").arg(dst));
        return false;
    }
    return true;
}


QStringList ConductGOTask::getResultFileNames(){
    QStringList result;

    QString current;

    current = getSettings().title + "_CC_RESULT.txt";
    if (QFile::exists(workingDir+"/"+current)){
        result << current;
    }
    current = getSettings().title + "_BP_RESULT.txt";
    if (QFile::exists(workingDir+"/"+current)){
        result << current;
    }

    current = getSettings().title + "_MF_RESULT.txt";
    if (QFile::exists(workingDir+"/"+current)){
        result << current;
    }

    current = getSettings().title + "_Conduct_GO_using_David.html";
    if (QFile::exists(workingDir+"/"+current)){
        result << current;
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////
//ConductGOLogParser
ConductGOLogParser::ConductGOLogParser()
:ExternalToolLogParser(){

}

} // U2
