/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "CufflinksSupport.h"
#include "CufflinksSupportTask.h"
#include "../ExternalToolSupportL10N.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AssemblyObject.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/GUrl.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Formats/GTFFormat.h>

#include <QCoreApplication>
#include <QDir>


namespace U2 {

CufflinksSupportTask::CufflinksSupportTask(const CufflinksSettings& _settings)
    : Task(tr("Running Cufflinks task"), TaskFlags_NR_FOSE_COSC),
      settings(_settings),
      logParser(NULL),
      cufflinksExtToolTask(NULL),
      tmpDoc(NULL)
{
}


CufflinksSupportTask::~CufflinksSupportTask()
{
    if (NULL != tmpDoc) {
        delete tmpDoc;
    }

    if (NULL != logParser) {
        delete logParser;
    }
}


void CufflinksSupportTask::prepare()
{
    // Add a new subdirectory for temporary files
    QString tmpDirName = "Cufflinks_"+QString::number(this->getTaskId())+"_"+
        QDate::currentDate().toString("dd.MM.yyyy")+"_"+
        QTime::currentTime().toString("hh.mm.ss.zzz")+"_"+
        QString::number(QCoreApplication::applicationPid())+"/";

    QString cufflinksTmpDirName =
        AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath(CUFFLINKS_TMP_DIR);

    QDir tmpDir(cufflinksTmpDirName + "/" + tmpDirName);

    if (tmpDir.exists()) {
        foreach (const QString& file, tmpDir.entryList()) {
            tmpDir.remove(file);
        }

        if (!tmpDir.rmdir(tmpDir.absolutePath())){
            stateInfo.setError(ExternalToolSupportL10N::errorRemovingTmpSubdir(tmpDir.absolutePath()));
            return;
        }
    }

    if (!tmpDir.mkpath(tmpDir.absolutePath())) {
        stateInfo.setError(ExternalToolSupportL10N::errorCreatingTmpSubrir(tmpDir.absolutePath()));
        return;
    }

    workingDirectory = tmpDir.absolutePath();
    url = workingDirectory + "/tmp.sam";

    DocumentFormat* docFormat = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::SAM);
    tmpDoc = docFormat->createNewLoadedDocument(IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), GUrl(url), stateInfo);
    CHECK_OP(stateInfo, );

    std::auto_ptr<AssemblyObject> assObj(
        Workflow::StorageUtils::getAssemblyObject(settings.storage, settings.assemblyId));

    if (NULL == assObj.get()) {
        stateInfo.setError(tr("Unable to get an assembly object."));
        return;
    }

    convertAssToSamTask = new ConvertAssemblyToSamTask(assObj->getEntityRef(), url);
    addSubTask(convertAssToSamTask);
}


QList<Task*> CufflinksSupportTask::onSubTaskFinished(Task* subTask)
{
    QList<Task*> result;

    if (subTask->hasError()) {
        stateInfo.setError(subTask->getError());
        return result;
    }

    if (hasError() || isCanceled()) {
        return result;
    }

    if (subTask == convertAssToSamTask) {

        // Init the arguments list
        QStringList arguments;

        arguments.prepend(url);

        if (!settings.referenceAnnotation.isEmpty()) {
            arguments << "-G" << settings.referenceAnnotation;
        }

        if (!settings.rabtAnnotation.isEmpty()) {
            arguments << "-g" << settings.rabtAnnotation;
        }

        arguments << "--library-type" << settings.libraryType.getLibraryTypeAsStr();

        if (!settings.maskFile.isEmpty()) {
            arguments << "-M" << settings.maskFile;
        }

        if (true == settings.multiReadCorrect) {
            arguments << "--multi-read-correct";
        }

        arguments << "--min-isoform-fraction" << QString::number(settings.minIsoformFraction);

        if (!settings.fragBiasCorrect.isEmpty()) {
            arguments << "--frag-bias-correct" << settings.fragBiasCorrect;
        }

        arguments << "--pre-mrna-fraction" << QString::number(settings.preMrnaFraction);


        // Create a log parser
        logParser = new ExternalToolLogParser();

        // Create the Cufflinks task
        cufflinksExtToolTask = new ExternalToolRunTask(CUFFLINKS_TOOL_NAME,
            arguments,
            logParser,
            workingDirectory);

        result.append(cufflinksExtToolTask);
    }
    else if (subTask == cufflinksExtToolTask) {
        transcriptGtfResultToAnnotations();
    }

    return result;
}


void CufflinksSupportTask::transcriptGtfResultToAnnotations()
{
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    if (NULL == iof) {
        ioLog.error(tr("Can't create IOAdapterFactory for reading transcripts.gtf for Cufflinks!"));
        return;
    }

    QString transcriptGtfPath = workingDirectory + "/transcripts.gtf";

    if(!QFile::exists(transcriptGtfPath)){
        stateInfo.setError(tr("Cufflinks output file '%1' is not found!"));
        return;
    }

    std::auto_ptr<IOAdapter> io(iof->createIOAdapter());
    if (!io.get()->open(GUrl(transcriptGtfPath), IOAdapterMode_Read)) {
        stateInfo.setError(L10N::errorOpeningFileRead(transcriptGtfPath));
        return;
    }

    transcriptGtfAnnots = GTFFormat::getAnnotData(io.get(), stateInfo);
}


QList<SharedAnnotationData> CufflinksSupportTask::getTranscriptGtfAnnots() const
{
    return transcriptGtfAnnots;
}


} // namespace U2
