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

#include <U2Core/IOAdapterUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/U2Assembly.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SafePoints.h>

#include "ExportCoverageTask.h"

namespace U2 {

ExportCoverageTask::ExportCoverageTask(const U2DbiRef &dbiRef, const U2DataId &assemblyId, const ExportCoverageSettings &settings) :
Task(tr("Export coverage per base for %1"), TaskFlags_NR_FOSE_COSC),
    dbiRef(dbiRef),
    assemblyId(assemblyId),
    settings(settings),
    alreadyProcessed(0),
    calculateTask(NULL)
{
    SAFE_POINT_EXT(dbiRef.isValid(), setError(tr("Invalid database reference")), );
    SAFE_POINT_EXT(!assemblyId.isEmpty(), setError(tr("Invalid assembly ID")), );
    SAFE_POINT_EXT(!settings.url.isEmpty(), setError(tr("Invalid destination url")), );

    DbiConnection con(dbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    U2AssemblyDbi *assemblyDbi = con.dbi->getAssemblyDbi();
    SAFE_POINT_EXT(NULL != assemblyDbi, setError(tr("Assembly DBI is NULL")), );

    const U2Assembly assembly = assemblyDbi->getAssemblyObject(assemblyId, stateInfo);
    CHECK_OP(stateInfo, );
    setTaskName(getTaskName().arg(assembly.visualName));
}

void ExportCoverageTask::prepare() {
    QDir().mkpath(QFileInfo(settings.url).absoluteDir().absolutePath());
    if (settings.compress) {
        IOAdapterFactory *ioAdapterFactory = IOAdapterUtils::get(BaseIOAdapters::GZIPPED_LOCAL_FILE);
        SAFE_POINT_EXT(NULL != ioAdapterFactory, setError(tr("Can't write the compressed file: IOAdapterFactory is NULL")), );
        ioAdapter.reset(ioAdapterFactory->createIOAdapter());
        bool isSuccess = ioAdapter->open(settings.url, IOAdapterMode_Write);
        CHECK_EXT(isSuccess, setError(L10N::errorOpeningFileWrite(settings.url)), );
    } else {
        IOAdapterFactory *ioAdapterFactory = IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE);
        SAFE_POINT_EXT(NULL != ioAdapterFactory, setError(tr("Can't write the file: IOAdapterFactory is NULL")), );
        ioAdapter.reset(ioAdapterFactory->createIOAdapter());
        bool isSuccess = ioAdapter->open(settings.url, IOAdapterMode_Write);
        CHECK_EXT(isSuccess, setError(L10N::errorOpeningFileWrite(settings.url)), );
    }
    CHECK_OP(stateInfo, );

    writeComments();

    calculateTask = new CalculateCoveragePerBaseTask(dbiRef, assemblyId);
    connect(calculateTask, SIGNAL(si_regionIsProcessed(qint64)), SLOT(sl_regionIsProcessed(qint64)), Qt::DirectConnection);
    addSubTask(calculateTask);
}

Task::ReportResult ExportCoverageTask::report() {
    if (NULL != calculateTask) {
        SAFE_POINT_EXT(!calculateTask->areThereUnprocessedResults(), setError(tr("Not all regions were processed")), ReportResult_Finished);
    }
    return ReportResult_Finished;
}

const QString &ExportCoverageTask::getUrl() const {
    return settings.url;
}

void ExportCoverageTask::sl_regionIsProcessed(qint64 startPos) {
    if (alreadyProcessed == startPos) {
        QVector<CoveragePerBaseInfo> *regionCoverage = calculateTask->takeResult(startPos);
        exportToTabDelimitedPileup(regionCoverage);
        delete regionCoverage;

        CHECK_OP(stateInfo, );

        if (calculateTask->isResultReady(alreadyProcessed)) {
            sl_regionIsProcessed(alreadyProcessed);
        }
    }
}

void ExportCoverageTask::writeComments() {
    QByteArray comments = "#position";

    if (settings.exportCoverage) {
        comments += QByteArray("\t") + "coverage";
    }

    if (settings.exportBasesCount) {
        comments += QByteArray("\t") + "A";
        comments += QByteArray("\t") + "C";
        comments += QByteArray("\t") + "G";
        comments += QByteArray("\t") + "T";
    }

    ioAdapter->writeBlock(comments + "\n");
}

void ExportCoverageTask::exportToTabDelimitedPileup(const QVector<CoveragePerBaseInfo> *data) {
    CHECK(NULL != data, );

    foreach (const CoveragePerBaseInfo &info, *data) {
        alreadyProcessed++;
        if (settings.threshold > info.coverage) {
            continue;
        }

        const QByteArray dataToWrite = info.toByteArray(alreadyProcessed, settings.exportCoverage, settings.exportBasesCount, "\t") + "\n";
        qint64 bytesWritten = ioAdapter->writeBlock(dataToWrite);
        CHECK_EXT(bytesWritten == dataToWrite.length(), setError(L10N::errorWritingFile(ioAdapter->getURL())), );
    }
}

}   // namespace U2
