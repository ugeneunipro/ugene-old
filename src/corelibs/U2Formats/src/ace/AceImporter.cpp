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

#include <U2Core/DocumentUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/Timer.h>

#include <U2Formats/AceFormat.h>

#include "AceImporter.h"
#include "ConvertAceToSqliteTask.h"

namespace U2 {

///////////////////////////////////
//// AceImporterTask
///////////////////////////////////

AceImporterTask::AceImporterTask(const GUrl& url, const QVariantMap& settings, const QVariantMap &hints) :
    DocumentProviderTask(tr("ACE file import: %1").arg(url.fileName()), TaskFlags_NR_FOSE_COSC),
    convertTask(NULL),
    loadDocTask(NULL),
    srcUrl(url),
    destUrl(settings.value(AceImporter::DEST_URL).toString()) {
    if (hints.contains(DocumentFormat::DBI_REF_HINT)) {
        U2DbiRef ref = hints.value(DocumentFormat::DBI_REF_HINT).value<U2DbiRef>();
        hintedDbiUrl = ref.dbiId;
    }

    documentDescription = srcUrl.fileName();
}

void AceImporterTask::prepare() {
    startTime = TimeCounter::getCounter();

    if (destUrl.isEmpty()) {
        if (hintedDbiUrl.isEmpty()) {
            destUrl = srcUrl.dirPath() + "/" + srcUrl.fileName() + ".ugenedb";
        } else {
            destUrl = hintedDbiUrl;
        }
    }

    convertTask = new ConvertAceToSqliteTask(srcUrl, destUrl);
    addSubTask(convertTask);
}

QList<Task*> AceImporterTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    CHECK_EXT(!subTask->hasError(), propagateSubtaskError(), res);
    CHECK_EXT(!subTask->isCanceled(), stateInfo.setCanceled(true), res);

    if (convertTask == subTask) {
        loadDocTask = LoadDocumentTask::getDefaultLoadDocTask(convertTask->getDestinationUrl());
        if (loadDocTask == NULL) {
            setError(tr("Failed to get load task for : %1").arg(convertTask->getDestinationUrl().getURLString()));
            return res;
        }
        res << loadDocTask;
    }

    if (loadDocTask == subTask) {
        resultDocument = loadDocTask->takeDocument();
    }

    return res;
}

Task::ReportResult AceImporterTask::report() {
    qint64 totalTime = TimeCounter::getCounter() - startTime;
    taskLog.info(QString("AceImporter task total time is %1 sec").arg(totalTime));
    return ReportResult_Finished;
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

    if (showWizard && dialogFactory) {
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
