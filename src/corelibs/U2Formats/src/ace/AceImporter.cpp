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
#include <QtCore/QTemporaryFile>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AssemblyObject.h>
#include <U2Core/CloneObjectTask.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/Timer.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Formats/AceFormat.h>

#include "AceImporter.h"
#include "CloneAssemblyWithReferenceToDbiTask.h"
#include "ConvertAceToSqliteTask.h"

namespace U2 {

///////////////////////////////////
//// AceImporterTask
///////////////////////////////////

AceImporterTask::AceImporterTask(const GUrl& url, const QVariantMap& settings, const QVariantMap &hints) :
    DocumentProviderTask(tr("ACE file import: %1").arg(url.fileName()), TaskFlags_NR_FOSE_COSC),
    convertTask(NULL),
    loadDocTask(NULL),
    isSqliteDbTransit(false),
    settings(settings),
    hints(hints),
    srcUrl(url)
{
    if (settings.contains(AceImporter::DEST_URL)) {
        destUrl = GUrl(settings.value(AceImporter::DEST_URL).toString());
    }

    documentDescription = srcUrl.fileName();
}

void AceImporterTask::prepare() {
    startTime = GTimer::currentTimeMicros();

    hintedDbiRef = hints.value(DocumentFormat::DBI_REF_HINT).value<U2DbiRef>();
    isSqliteDbTransit = hintedDbiRef.isValid() && SQLITE_DBI_ID != hintedDbiRef.dbiFactoryId;

    if (destUrl.isEmpty()) {
        if (!isSqliteDbTransit) {
            localDbiRef = U2DbiRef(SQLITE_DBI_ID, srcUrl.dirPath() + QDir::separator() + srcUrl.fileName() + ".ugenedb");
        } else {
            const QString tmpDir = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath("assembly_conversion") + QDir::separator();
            QDir().mkpath(tmpDir);

            const QString pattern = tmpDir + "XXXXXX.ugenedb";
            QTemporaryFile *tempLocalDb = new QTemporaryFile(pattern, this);

            tempLocalDb->open();
            const QString filePath = tempLocalDb->fileName();
            tempLocalDb->close();

            SAFE_POINT_EXT(QFile::exists(filePath), setError(tr("Can't create a temporary database")), );

            localDbiRef = U2DbiRef(SQLITE_DBI_ID, filePath);
        }
    } else {
        localDbiRef = U2DbiRef(SQLITE_DBI_ID, destUrl.getURLString());
    }

    convertTask = new ConvertAceToSqliteTask(srcUrl, localDbiRef);
    addSubTask(convertTask);
}

QList<Task*> AceImporterTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    CHECK_OP(stateInfo, res);

    if (isSqliteDbTransit && convertTask == subTask) {
        initCloneObjectTasks();
        res << cloneTasks;
    }

    else if (isSqliteDbTransit && cloneTasks.contains(subTask)) {
        cloneTasks.removeOne(subTask);
        if (cloneTasks.isEmpty()) {
            initLoadDocumentTask();
            CHECK(NULL != loadDocTask, res);
            res << loadDocTask;
        }
    }

    else if (!isSqliteDbTransit && convertTask == subTask) {
        initLoadDocumentTask();
        CHECK(NULL != loadDocTask, res);
        res << loadDocTask;
    }

    if (loadDocTask == subTask) {
        resultDocument = loadDocTask->takeDocument();
    }

    return res;
}

Task::ReportResult AceImporterTask::report() {
    qint64 totalTime = GTimer::currentTimeMicros() - startTime;
    taskLog.info(QString("AceImporter task total time is %1 sec").arg((double)totalTime / 1000000));
    return ReportResult_Finished;
}

void AceImporterTask::initCloneObjectTasks() {
    const QMap<U2Sequence, U2Assembly> importedObjects = convertTask->getImportedObjects();
    foreach (const U2Sequence &reference, importedObjects.keys()) {
        cloneTasks << new CloneAssemblyWithReferenceToDbiTask(importedObjects[reference], reference, localDbiRef, hintedDbiRef, hints);
    }
}

void AceImporterTask::initLoadDocumentTask() {
    if (hints.value(AceImporter::LOAD_RESULT_DOCUMENT, true).toBool()) {
        loadDocTask = LoadDocumentTask::getDefaultLoadDocTask(convertTask->getDestinationUrl());
        if (loadDocTask == NULL) {
            setError(tr("Failed to get load task for : %1").arg(convertTask->getDestinationUrl().getURLString()));
        }
    }
}


///////////////////////////////////
//// AceImporter
///////////////////////////////////

const QString AceImporter::ID = "ace-importer";
const QString AceImporter::SRC_URL = "source_url";
const QString AceImporter::DEST_URL = "destination_url";

AceImporter::AceImporter() :
    DocumentImporter(ID, tr("ACE file importer")) {
    ACEFormat aceFormat(NULL);
    extensions << aceFormat.getSupportedDocumentFileExtensions();
    formatIds << aceFormat.getFormatId();
    importerDescription = tr("ACE files importer is used to convert conventional ACE files into UGENE database format." \
                             "Having ACE file converted into UGENE DB format you get an fast and efficient interface " \
                             "to your data with an option to change the content");
}

FormatCheckResult AceImporter::checkRawData(const QByteArray& rawData, const GUrl& url) {
    ACEFormat aceFormat(NULL);
    return aceFormat.checkRawData(rawData, url);
}

DocumentProviderTask* AceImporter::createImportTask(const FormatDetectionResult& res, bool showWizard, const QVariantMap &hints) {
    QVariantMap settings;
    AceImporterTask* task = NULL;
    settings.insert(SRC_URL, res.url.getURLString());

    if (showWizard && NULL != dialogFactory) {
        ImportDialog* dialog = dialogFactory->getDialog(settings);
        int result = dialog->exec();
        settings = dialog->getSettings();
        delete dialog;
        task = new AceImporterTask(res.url, settings, hints);
        if (result == QDialog::Rejected) {
            task->cancel();
        }
    } else {
        task = new AceImporterTask(res.url, settings, hints);
    }

    return task;
}

}   // namespace U2
