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

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AssemblyObject.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/GUrl.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Formats/DifferentialFormat.h>
#include <U2Formats/FpkmTrackingFormat.h>
#include <U2Formats/GTFFormat.h>

#include "CufflinksSupport.h"
#include "../ExternalToolSupportL10N.h"
#include "tophat/TopHatSettings.h"
#include "CufflinksSupportTask.h"

namespace U2 {

const QString CufflinksSupportTask::outSubDirBaseName("cufflinks_out");

CufflinksSupportTask::CufflinksSupportTask(const CufflinksSettings& _settings)
    : ExternalToolSupportTask(tr("Running Cufflinks task"), TaskFlags_NR_FOSE_COSC),
      settings(_settings),
      logParser(NULL),
      tmpDoc(NULL),
      convertAssToSamTask(NULL),
      cufflinksExtToolTask(NULL)
{

}

CufflinksSupportTask::~CufflinksSupportTask()
{
    delete tmpDoc;
    delete logParser;
}

QString CufflinksSupportTask::initTmpDir() {
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
            return "";
        }
    }

    if (!tmpDir.mkpath(tmpDir.absolutePath())) {
        stateInfo.setError(ExternalToolSupportL10N::errorCreatingTmpSubrir(tmpDir.absolutePath()));
        return "";
    }
    return tmpDir.absolutePath();
}

void CufflinksSupportTask::prepare()
{
    settings.outDir = GUrlUtils::createDirectory(
        settings.outDir + "/" + outSubDirBaseName,
        "_", stateInfo);
    CHECK_OP(stateInfo, );

    workingDirectory = initTmpDir();
    CHECK_OP(stateInfo, );

    if (settings.fromFile) {
        cufflinksExtToolTask = runCufflinks();
        addSubTask(cufflinksExtToolTask);
        return;
    }

    settings.url = workingDirectory + "/tmp.sam";
    DocumentFormat* docFormat = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::SAM);
    tmpDoc = docFormat->createNewLoadedDocument(IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), GUrl(settings.url), stateInfo);
    CHECK_OP(stateInfo, );

    std::auto_ptr<AssemblyObject> assObj(
        Workflow::StorageUtils::getAssemblyObject(settings.storage, settings.assemblyId));

    if (NULL == assObj.get()) {
        stateInfo.setError(tr("Unable to get an assembly object."));
        return;
    }

    convertAssToSamTask = new ConvertAssemblyToSamTask(assObj->getEntityRef(), settings.url);
    addSubTask(convertAssToSamTask);
}

ExternalToolRunTask * CufflinksSupportTask::runCufflinks() {
    // Init the arguments list
    QStringList arguments;

    arguments << "-p" << QString::number(TopHatSettings::getThreadsCount());
    arguments << "--output-dir" << settings.outDir;

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

    arguments << settings.url;

    // Create a log parser
    logParser = new ExternalToolLogParser();

    // Create the Cufflinks task
    
    ExternalToolRunTask* runTask = new ExternalToolRunTask(ET_CUFFLINKS,
        arguments,
        logParser,
        workingDirectory);
    setListenerForTask(runTask);
    return runTask;
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
        cufflinksExtToolTask = runCufflinks();
        result.append(cufflinksExtToolTask);
    }
    else if (subTask == cufflinksExtToolTask) {
        ExternalToolSupportUtils::appendExistingFile(settings.outDir + "/transcripts.gtf", outputFiles);
        ExternalToolSupportUtils::appendExistingFile(settings.outDir + "/isoforms.fpkm_tracking", outputFiles);
        ExternalToolSupportUtils::appendExistingFile(settings.outDir + "/genes.fpkm_tracking", outputFiles);
        isoformLevelAnnots = getAnnotationsFromFile("transcripts.gtf", CufflinksOutputGtf);
    }

    return result;
}


QList<AnnotationData> CufflinksSupportTask::getAnnotationsFromFile( QString fileName,
    CufflinksOutputFormat format )
{
    const QString filePath = settings.outDir + "/" + fileName;
    DocumentFormatId formatId;
    switch ( format ) {
    case CufflinksOutputFpkm :
        formatId = BaseDocumentFormats::FPKM_TRACKING_FORMAT;
        break;
    case CufflinksOutputGtf :
        formatId = BaseDocumentFormats::GTF;
        break;
    default:
        FAIL( "Internal error: unexpected format of the Cufflinks output!", QList<AnnotationData>( ) );
    }

    return getAnnotationsFromFile(filePath, formatId, ET_CUFFLINKS, stateInfo);
}

Task::ReportResult CufflinksSupportTask::report()
{
    if (settings.url.isEmpty()) {
        return ReportResult_Finished;
    }

    GUrlUtils::removeDir(workingDirectory, stateInfo);
    return ReportResult_Finished;
}

QStringList CufflinksSupportTask::getOutputFiles() const {
    return outputFiles;
}

QList<AnnotationData> CufflinksSupportTask::getAnnotationsFromFile(const QString &filePath,
    const DocumentFormatId &format, const QString &toolName, U2OpStatus &os)
{
    QList<AnnotationData> result;

    IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    if (NULL == iof) {
        os.setError(QObject::tr("An internal error occurred during getting annotations from a %1 output file!").arg(toolName));
        return result;
    }

    if(!QFile::exists(filePath)){
        os.setError(tr("%1 output file '%2' is not found!").arg(toolName).arg(filePath));
        return result;
    }

    QScopedPointer<IOAdapter> io(iof->createIOAdapter());
    if (!io.data()->open(GUrl(filePath), IOAdapterMode_Read)) {
        os.setError(L10N::errorOpeningFileRead(filePath));
        return result;
    }

    if ( BaseDocumentFormats::FPKM_TRACKING_FORMAT == format ) {
        result = FpkmTrackingFormat::getAnnotData(io.data(), os);
    } else if ( BaseDocumentFormats::GTF == format ) {
        result = GTFFormat::getAnnotData(io.data(), os);
    } else if ( BaseDocumentFormats::DIFF == format ) {
        result = DifferentialFormat::getAnnotationData(io.data(), os);
    } else {
        FAIL(QObject::tr("Internal error: unexpected format of the %1 output!").arg(toolName), result);
    }

    return result;
}

} // namespace U2
