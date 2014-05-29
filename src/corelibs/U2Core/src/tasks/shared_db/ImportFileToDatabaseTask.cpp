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

#include <QtCore/QFileInfo>

#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrl.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/ProjectView.h>

#include "ImportFileToDatabaseTask.h"

namespace U2 {

ImportFileToDatabaseTask::ImportFileToDatabaseTask(const QString& srcUrl, const U2DbiRef& dstDbiRef, const QString& dstFolder, const ImportToDatabaseOptions& options) :
    Task(tr("Import file %1 to the database").arg(QFileInfo(srcUrl).fileName()), TaskFlags_FOSE_COSC),
    srcUrl(srcUrl),
    dstDbiRef(dstDbiRef),
    dstFolder(dstFolder),
    options(options)
{
    CHECK_EXT(QFileInfo(srcUrl).isFile(), setError(tr("It is not a file: ") + srcUrl), );
    CHECK_EXT(dstDbiRef.isValid(), setError(tr("Invalid database reference")), );
}

void ImportFileToDatabaseTask::run() {
    GCOUNTER(cvar, tvar, "ImportFileToDatabaseTask");
    FormatDetectionConfig detectionConfig;
    detectionConfig.useImporters = true;
    QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(GUrl(srcUrl), detectionConfig);
    CHECK_EXT(!formats.isEmpty(), setError(tr("File is not recognized")), );

    // TODO: use objects relations
    DocumentProviderTask* task = NULL;
    DocumentFormat* format = formats.first().format;
    DocumentImporter* importer = formats.first().importer;

    const QVariantMap hints = prepareHints();

    if (NULL != format) {
        IOAdapterFactory* ioFactory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(GUrl(srcUrl)));
        CHECK_EXT(NULL != ioFactory, setError(tr("Unrecognized url: ") + srcUrl), );

        CHECK_OP(stateInfo, );
        Document* loadedDoc = format->loadDocument(ioFactory, srcUrl, hints, stateInfo);
        CHECK_OP(stateInfo, );
        loadedDoc->setDocumentOwnsDbiResources(false);
        delete loadedDoc;
    }

    // TODO: do something if format is not recognized
    // options.importUnknownAsUdr

    // TODO: do something with import
//    else {
//        CHECK_EXT(NULL != importer, setError(tr("File can't be neither loaded nor imported")), );
//        task = importer->createImportTask(formats.first(), true, hints);
//    }
}

Task::ReportResult ImportFileToDatabaseTask::report() {
    return ReportResult_Finished;
}

const QString &ImportFileToDatabaseTask::getFilePath() const {
    return srcUrl;
}

QVariantMap ImportFileToDatabaseTask::prepareHints() const {
    QVariantMap hints;
    hints[DocumentReadingMode_DontMakeUniqueNames] = 1;
    hints[DocumentFormat::DBI_REF_HINT] = qVariantFromValue(dstDbiRef);
    hints[DocumentFormat::DBI_FOLDER_HINT] = dstFolder + (options.createSubfolderForEachFile ?
                                                              U2ObjectDbi::PATH_SEP + QFileInfo(srcUrl).fileName() :
                                                              "");

    switch (options.multiSequencePolicy) {
        case ImportToDatabaseOptions::SEPARATE:
            // do nothing, it is normal behavior
            break;
        case ImportToDatabaseOptions::MERGE:
            hints[DocumentReadingMode_SequenceAsAlignmentHint] = true;
            break;
        case ImportToDatabaseOptions::MALIGNEMNT:
            hints[DocumentReadingMode_SequenceMergeGapSize] = options.mergeMultiSequencePolicySeparatorSize;
            break;
    };

    return hints;
}

}   // namespace U2
