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

#include "MACSSupport.h"

#include "MACSTask.h"

namespace U2 {

const QString MACSTask::BASE_DIR_NAME("macs_tmp");
const QString MACSTask::BASE_SUBDIR_NAME("macs");

MACSTask::MACSTask(const MACSSettings &_settings, const GUrl &_treatUrl, const GUrl &_conUrl)
    : ExternalToolSupportTask(tr("MACS peak calling"), TaskFlag_None)
    , settings(_settings)
    , treatUrl(_treatUrl)
    , conUrl(_conUrl)
    , peaksDoc(NULL)
    , summitsDoc(NULL)
    , peaksTask(NULL)
    , summitsTask(NULL)
    , etTask(NULL)
    , logParser(NULL)
{

}

MACSTask::~MACSTask() {
    cleanup();
}

void MACSTask::cleanup() {

    delete logParser;
    logParser = NULL;
    delete peaksDoc;
    peaksDoc = NULL;
    delete summitsDoc;
    summitsDoc = NULL;

    //remove tmp files
    QString tmpDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(BASE_DIR_NAME);
    QDir tmpDir(tmpDirPath);
    if(tmpDir.exists()){
        foreach(QString file, tmpDir.entryList()){
            tmpDir.remove(file);
        }
    }
}

void MACSTask::prepare() {
    UserAppsSettings *appSettings = AppContext::getAppSettings()->getUserAppsSettings();
    workingDir = appSettings->createCurrentProcessTemporarySubDir(stateInfo, BASE_DIR_NAME);
    CHECK_OP(stateInfo, );

    settings.outDir = GUrlUtils::createDirectory(
        settings.outDir + QDir::separator() + BASE_SUBDIR_NAME,
        "_", stateInfo);
    CHECK_OP(stateInfo, );

    QStringList args = settings.getArguments(treatUrl.getURLString(), conUrl.isEmpty() ? "" : conUrl.getURLString());

    logParser = new MACSLogParser();
    etTask = new ExternalToolRunTask(ET_MACS, args, logParser, getSettings().outDir);
    setListenerForTask(etTask);
    addSubTask(etTask);
}

QList<Task*> MACSTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> result;
    CHECK(!subTask->isCanceled(), result);
    CHECK(!subTask->hasError(), result);

    if(subTask == etTask){
        //read annotations
        QString peaksName = getSettings().outDir + QDir::separator() + getSettings().fileNames+"_peaks.bed";
        QString summitName = getSettings().outDir + QDir::separator() + getSettings().fileNames+"_summits.bed";

        peaksTask=
            new LoadDocumentTask(BaseDocumentFormats::BED,
            peaksName,
            AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE));
        result.append(peaksTask);

        summitsTask=
            new LoadDocumentTask(BaseDocumentFormats::BED,
            summitName,
            AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE));

        result.append(summitsTask);
    }else if(subTask == peaksTask){
        peaksDoc = peaksTask->takeDocument();
        
    }else if(subTask == summitsTask){
        summitsDoc = summitsTask->takeDocument();
    }

    return result;
}

void MACSTask::run() {

}

const MACSSettings& MACSTask::getSettings(){
    return settings;
}

QList<AnnotationData> MACSTask::getPeaks( ) {
    QList<AnnotationData> res;

    if ( NULL == peaksDoc ) {
        return res;
    }

    const QList<GObject *> objects = peaksDoc->getObjects( );

    foreach ( GObject *ao, objects ) {
        if ( ao->getGObjectType( ) == GObjectTypes::ANNOTATION_TABLE ) {
            AnnotationTableObject *aobj = qobject_cast<AnnotationTableObject *>( ao );
            if ( NULL != ao ) {
                const QList<Annotation> annots = aobj->getAnnotations( );
                foreach ( const Annotation &a, annots ) {
                    res << a.getData( );
                }
            }
        }
    }

    return res;
}

QList<AnnotationData> MACSTask::getPeakSummits( ) {
    QList<AnnotationData> res;

    if ( NULL == summitsDoc ) {
        return res;
    }
    const QList<GObject *> objects = summitsDoc->getObjects( );

    foreach ( GObject *ao, objects ) {
        if ( ao->getGObjectType( ) == GObjectTypes::ANNOTATION_TABLE ) {
            AnnotationTableObject *aobj = qobject_cast<AnnotationTableObject *>( ao );
            if ( NULL != ao ) {
                const QList<Annotation> &annots = aobj->getAnnotations( );
                foreach ( const Annotation &a, annots ) {
                    res << a.getData( );
                }
            }
        }
    }

    return res;
}

QString MACSTask::getWiggleUrl(){
    QString res = "";
    
    if (settings.wiggleOut){
        res = getSettings().outDir + QDir::separator()
            + getSettings().fileNames +"_MACS_wiggle" + QDir::separator()
            + "treat" + QDir::separator()
            + getSettings().fileNames+"_treat_afterfiting_all.wig";
    }

    return res;
}

QStringList MACSTask::getOutputFiles(){
    QStringList result;

    QString current;

    current = getSettings().fileNames + "_peaks.bed";
    if (QFile::exists(getSettings().outDir + QDir::separator() + current)){
        result << current;
    }
    current = getSettings().fileNames + "_summits.bed";
    if (QFile::exists(getSettings().outDir + QDir::separator() + current)){
        result << current;
    }


    return result;
}

//////////////////////////////////////////////////////////////////////////
//MACSLogParser
MACSLogParser::MACSLogParser()
    :ExternalToolLogParser(), progress(-1) {

}

int MACSLogParser::getProgress(){
    //parsing INFO  @ Fri, 07 Dec 2012 19:30:16: #1 read tag files...
    int max_step = 5;
    if(!lastPartOfLog.isEmpty()){
        QString lastMessage=lastPartOfLog.last();
        QRegExp rx(" #(\\d+) \\w");
        if(lastMessage.contains(rx)){
            SAFE_POINT(rx.indexIn(lastMessage) > -1, "bad progress index", 0);
            int step = rx.cap(1).toInt();
            return  progress = (100 * step)/ float(qMax(step, max_step));
        }
    }
    return progress;
}

void MACSLogParser::parseOutput( const QString& partOfLog ){
    ExternalToolLogParser::parseOutput(partOfLog);
}

void MACSLogParser::parseErrOutput( const QString& partOfLog ){
    lastPartOfLog=partOfLog.split(QRegExp("(\n|\r)"));
    lastPartOfLog.first()=lastErrLine+lastPartOfLog.first();
    lastErrLine=lastPartOfLog.takeLast();
    foreach(QString buf, lastPartOfLog){
        if(buf.contains("ERROR", Qt::CaseInsensitive)
            || buf.contains("CRITICAL", Qt::CaseInsensitive)){
                coreLog.error("MACS: " + buf);
        }else if (buf.contains("WARNING", Qt::CaseInsensitive)){
            algoLog.info(buf);
        }else {
            algoLog.trace(buf);
        }
    }
}

} // U2
