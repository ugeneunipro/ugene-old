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

#include <QtCore/QDir>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/Counter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/TextObject.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include "Peak2GeneSupport.h"
#include "Peak2GeneTask.h"

namespace U2 {

const QString Peak2GeneTask::BASE_DIR_NAME("peak2gene_tmp");
const QString Peak2GeneTask::TREAT_NAME("treatment");

Peak2GeneTask::Peak2GeneTask(const Peak2GeneSettings& _settings, Workflow::DbiDataStorage *storage, const QList<Workflow::SharedDbiDataHandler>& _treatAnn)
: ExternalToolSupportTask("Peak2gene annotation", TaskFlag_None)
, settings(_settings)
, storage(storage)
, treatAnn(_treatAnn)
, treatDoc(NULL)
, geneDoc(NULL)
, peaksDoc(NULL)
, treatTask(NULL)
, geneTask(NULL)
, peaksTask(NULL)
, etTask(NULL)
, logParser(NULL)
{
    GCOUNTER(cvar, tvar, "NGS:Peak2GeneTask");
    SAFE_POINT_EXT(NULL != storage, setError(L10N::nullPointerError("workflow data storage")), );
}

Peak2GeneTask::~Peak2GeneTask() {
    cleanup();
}

void Peak2GeneTask::cleanup() {
    treatAnn.clear();

    delete treatDoc; treatDoc = NULL;
    delete logParser; logParser = NULL;
    delete peaksDoc; peaksDoc = NULL;
    delete geneDoc; geneDoc = NULL;

    //remove tmp files
    QString tmpDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(BASE_DIR_NAME);
    QDir tmpDir(tmpDirPath);
    if(tmpDir.exists()){
        foreach(QString file, tmpDir.entryList()){
            tmpDir.remove(file);
        }
    }
}

void Peak2GeneTask::prepare() {
    UserAppsSettings *appSettings = AppContext::getAppSettings()->getUserAppsSettings();
    workingDir = appSettings->createCurrentProcessTemporarySubDir(stateInfo, BASE_DIR_NAME);
    CHECK_OP(stateInfo, );

    treatDoc = createDoc(treatAnn, TREAT_NAME);
    CHECK_OP(stateInfo, );

    treatTask = new SaveDocumentTask(treatDoc);
    addSubTask(treatTask);
}

Document* Peak2GeneTask::createDoc( const QList<Workflow::SharedDbiDataHandler>& annData, const QString& name){
    Document* doc = NULL;

    QString docUrl = workingDir + "/" + name +".bed";

    DocumentFormat *bedFormat = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::BED);
    CHECK_EXT(NULL != bedFormat, stateInfo.setError("NULL bed format"), doc);

    doc = bedFormat->createNewLoadedDocument(
        IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), docUrl, stateInfo);
    CHECK_OP(stateInfo, doc);
    doc->setDocumentOwnsDbiResources(false);

    QList<AnnotationTableObject *> annTables = Workflow::StorageUtils::getAnnotationTableObjects(storage, annData);
    foreach (AnnotationTableObject *annTable, annTables) {
        doc->addObject(annTable);
    }

    return doc;
}

QList<Task*> Peak2GeneTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> result;
    CHECK(!subTask->isCanceled(), result);
    CHECK(!subTask->hasError(), result);

    if (treatTask == subTask) {
            QStringList args = settings.getArguments(treatDoc->getURLString());
            
            logParser = new Peak2GeneLogParser();
            etTask = new ExternalToolRunTask(ET_PEAK2GENE, args, logParser, workingDir);
            setListenerForTask(etTask);
            result << etTask;
    }
    else if(subTask == etTask){
        //read annotations
        QString geneName = workingDir + "/" + Peak2GeneSettings::DEFAULT_NAME+"_gene_annotation.txt";
        QString peakName = workingDir + "/" + Peak2GeneSettings::DEFAULT_NAME+"_peaks_annotation.txt";

        geneTask=
            new LoadDocumentTask(BaseDocumentFormats::BED,
            geneName,
            AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE));
        result.append(geneTask);
 
        peaksTask=
            new LoadDocumentTask(BaseDocumentFormats::BED,
            peakName,
            AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE));

        result.append(peaksTask);

    }else if(subTask == geneTask){
        geneDoc = geneTask->takeDocument();
        
    }else if(subTask == peaksTask){
        peaksDoc = peaksTask->takeDocument();
    }

    return result;
}

void Peak2GeneTask::run() {

}

const Peak2GeneSettings& Peak2GeneTask::getSettings(){
    return settings;
}

QList<AnnotationTableObject *> Peak2GeneTask::getGenes() const {
    QList<AnnotationTableObject *> res;

    if (geneDoc == NULL) {
        return res;
    }

    const QList<GObject*> objects = geneDoc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
    foreach(GObject* ao, objects) {
        res << qobject_cast<AnnotationTableObject *>(ao);
        geneDoc->removeObject(ao, DocumentObjectRemovalMode_Release);
    }

    return res;
}

QList<AnnotationTableObject *> Peak2GeneTask::getPeaks() const {
    QList<AnnotationTableObject *> res;

    if (peaksDoc == NULL){
        return res;
    }

    const QList<GObject*> objects = peaksDoc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
    foreach(GObject* ao, objects) {
        res << qobject_cast<AnnotationTableObject *>(ao);
        peaksDoc->removeObject(ao, DocumentObjectRemovalMode_Release);
    }

    return res;
}

//////////////////////////////////////////////////////////////////////////
//Peak2GeneLogParser
Peak2GeneLogParser::Peak2GeneLogParser()
:ExternalToolLogParser(){

}

} // U2
