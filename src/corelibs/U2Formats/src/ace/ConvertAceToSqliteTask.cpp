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

#include <QtCore/QFile>

#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/Timer.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2CoreAttributes.h>
#include <U2Core/U2CrossDatabaseReferenceDbi.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2SequenceUtils.h>

#include "ace/ConvertAceToSqliteTask.h"

namespace U2 {

ConvertAceToSqliteTask::ConvertAceToSqliteTask(const GUrl &_sourceUrl, const GUrl &_destinationUrl) :
    Task(tr("Convert ACE to UGENE database (%1)").arg(_destinationUrl.fileName()), TaskFlag_None),
    append(false),
    sourceUrl(_sourceUrl),
    destinationUrl(_destinationUrl),
    dbi(NULL)
{
    GCOUNTER(cvar, tvar, "ConvertAceToUgenedb");
    tpm = Progress_Manual;
}

void ConvertAceToSqliteTask::run() {
    taskLog.info(tr("Converting assembly from %1 to %2 started")
                 .arg(sourceUrl.fileName())
                 .arg(destinationUrl.fileName()));

    qint64 startTime = TimeCounter::getCounter();

    CHECK_EXT(destinationUrl.isLocalFile(), setError(tr("Non-local files are not supported")), );

    QScopedPointer<IOAdapter> ioAdapter;
    IOAdapterFactory *factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(sourceUrl));
    SAFE_POINT_EXT(factory, setError(tr("IOAdapterFactory is NULL")), );
    ioAdapter.reset(factory->createIOAdapter());

    if (!ioAdapter->open(sourceUrl, IOAdapterMode_Read)) {
        setError(tr("Can't open file '%1'").arg(sourceUrl.getURLString()));
        return;
    }

    append = QFile::exists(destinationUrl.getURLString());

    DbiConnection dbiHandle(U2DbiRef(SQLITE_DBI_ID, destinationUrl.getURLString()), true, stateInfo);
    CHECK_OP(stateInfo, );

    dbi = dbiHandle.dbi;
    SAFE_POINT(dbi, tr("DBI is NULL"), );
    U2ObjectDbi* objDbi = dbi->getObjectDbi();
    SAFE_POINT(objDbi, tr("Object DBI is NULL"), );

    QStringList folders = objDbi->getFolders(stateInfo);
    CHECK_OP(stateInfo, );

    if (!append || !folders.contains("/")) {
        objDbi->createFolder("/", stateInfo);
        CHECK_OP(stateInfo, );
    }

    stateInfo.setDescription("Importing");
    taskLog.details(tr("Importing"));

    qint64 totalReadsImported = importAssemblies(*ioAdapter);
    CHECK_OP(stateInfo, );

    stateInfo.setDescription("Packing reads");
    qint64 packTime = packReads();
    CHECK_OP(stateInfo, );

    stateInfo.setDescription("Updating attributes");
    updateAttributeDbi();
    CHECK_OP(stateInfo, );

    qint64 totalTime = TimeCounter::getCounter() - startTime;
    taskLog.info(QString("Converting assembly from %1 to %2 succesfully finished: imported %3 reads, total time %4 s, pack time %5 s")
                 .arg(sourceUrl.fileName())
                 .arg(destinationUrl.fileName())
                 .arg(totalReadsImported)
                 .arg(totalTime)
                 .arg(packTime));
}

Task::ReportResult ConvertAceToSqliteTask::report() {
    if (stateInfo.isCoR() && destinationUrl.isLocalFile()) {
        if (!append) {
            QFile::remove(destinationUrl.getURLString());
        } else {
            // Do not remove file if it was appended.
            removeCorruptedAssembly();
        }
    }

    return ReportResult_Finished;
}

const GUrl & ConvertAceToSqliteTask::getDestinationUrl() const {
    return destinationUrl;
}

qint64 ConvertAceToSqliteTask::importAssemblies(IOAdapter &ioAdapter) {
    int assemblyNum = 0;
    qint64 totalReadsImported = 0;

    U2AssemblyDbi* assDbi = dbi->getAssemblyDbi();
    SAFE_POINT(assDbi, tr("Assembly DBI is NULL"), totalReadsImported);
    U2CrossDatabaseReferenceDbi* crossDbi = dbi->getCrossDatabaseReferenceDbi();
    SAFE_POINT(crossDbi, tr("Cross DBI is NULL"), totalReadsImported);
    U2SequenceDbi* seqDbi = dbi->getSequenceDbi();
    SAFE_POINT(seqDbi, tr("Sequence DBI is NULL"), totalReadsImported);

    U2OpStatusChildImpl os(&stateInfo, U2OpStatusMapping(0, 50));
    QScopedPointer<AceReader> aceReader;
    aceReader.reset(new AceReader(ioAdapter, os));
    CHECK_OP(os, totalReadsImported);

    QScopedPointer<AceIterator> iterator;
    iterator.reset(new AceIterator(*aceReader, stateInfo));

    while (iterator->hasNext()) {
        CHECK(!isCanceled(), totalReadsImported);

        U2Assembly assembly;

        Assembly aceAssembly = iterator->next();
        CHECK_OP(stateInfo, totalReadsImported);
        CHECK_EXT(aceAssembly.isValid(), setError(tr("Invalid source file")), totalReadsImported);
        Assembly::Sequence aceReference = aceAssembly.getReference();
        references.insert(assemblyNum, aceReference);

        U2Sequence reference;
        reference.length = aceReference.data.length();
        reference.visualName = aceReference.name;
        reference.alphabet = U2AlphabetUtils::findBestAlphabet(aceReference.data)->getId();

        assembly.visualName = aceAssembly.getName();

        U2CrossDatabaseReference crossDbRef;
        crossDbRef.dataRef.dbiRef.dbiId = dbi->getDbiId();
        crossDbRef.dataRef.dbiRef.dbiFactoryId = dbi->getFactoryId();
        crossDbRef.dataRef.entityId = reference.visualName.toAscii();
        crossDbRef.dataRef.version = 1;
        crossDbi->createCrossReference(crossDbRef, stateInfo);
        CHECK_OP(stateInfo, totalReadsImported);
        assembly.referenceId = crossDbRef.id;

        corruptedAssembly = assembly;

        U2AssemblyReadsImportInfo & importInfo = importInfos[assemblyNum];
        assDbi->createAssemblyObject(assembly, "/", NULL, importInfo, stateInfo);
        CHECK_OP(stateInfo, totalReadsImported);

        importInfo.packed = false;
        importInfo.nReads = aceAssembly.getReadsCount();
        assemblies.insert(assemblyNum, assembly);

        QList<U2AssemblyRead> reads = aceAssembly.getReads();

        BufferedDbiIterator<U2AssemblyRead> readsIterator(reads);
        assDbi->addReads(assembly.id, &readsIterator, stateInfo);
        CHECK_OP(stateInfo, totalReadsImported);

        seqDbi->createSequenceObject(reference, "/", stateInfo);
        CHECK_OP(stateInfo, totalReadsImported);

        QVariantMap hints;
        hints[U2SequenceDbiHints::EMPTY_SEQUENCE] = true;
        hints[U2SequenceDbiHints::UPDATE_SEQUENCE_LENGTH] = true;
        seqDbi->updateSequenceData(reference.id, U2_REGION_MAX, aceReference.data, hints, stateInfo);
        CHECK_OP(stateInfo, totalReadsImported);

        totalReadsImported += aceAssembly.getReadsCount();
        assemblyNum++;

        corruptedAssembly = U2Assembly();
    }
    CHECK_EXT( aceReader->getContigsCount() == assemblyNum, setError(tr("Invalid source file")), totalReadsImported);

    return totalReadsImported;
}

qint64 ConvertAceToSqliteTask::packReads() {
    qint64 packStart = TimeCounter::getCounter();
    int progressStep;
    if (assemblies.count() > 0) {
        progressStep = 40 / assemblies.count();
    } else {
        progressStep = 40;
    }


    U2AssemblyDbi* assDbi = dbi->getAssemblyDbi();
    SAFE_POINT(assDbi, tr("Assembly DBI is NULL"), 0);

    foreach (int assemblyNum, assemblies.keys()) {
        U2AssemblyReadsImportInfo & importInfo = importInfos[assemblyNum];
        // Pack reads only if it were not packed on import
        if (!importInfo.packed) {
            taskLog.details(tr("Packing reads for assembly '%1' (%2 of %3)")
                            .arg(assemblies[assemblyNum].visualName)
                            .arg(assemblyNum + 1)
                            .arg(assemblies.keys().count()));

            U2AssemblyPackStat stat;
            assDbi->pack(assemblies[assemblyNum].id, stat, stateInfo);
            CHECK_OP(stateInfo, 0);

            importInfo.packStat = stat;
        }
        stateInfo.setProgress(stateInfo.getProgress() + progressStep);
    }

    return TimeCounter::getCounter() - packStart;
}

void ConvertAceToSqliteTask::updateAttributeDbi() {
    int progressStep;
    if (assemblies.count() > 0) {
        progressStep = 10 / assemblies.count();
    } else {
        progressStep = 10;
    }

    U2AttributeDbi* attrDbi = dbi->getAttributeDbi();
    SAFE_POINT(attrDbi, tr("Attribute DBI is NULL"), );

    foreach (int assemblyNum, assemblies.keys()) {
        const U2Assembly &assembly = assemblies[assemblyNum];
        const Assembly::Sequence &reference = references[assemblyNum];
        {
            U2IntegerAttribute lenAttr;
            lenAttr.objectId = assembly.id;
            lenAttr.name = U2BaseAttributeName::reference_length;
            lenAttr.version = assembly.version;
            lenAttr.value = reference.data.length();
            attrDbi->createIntegerAttribute(lenAttr, stateInfo);
            CHECK_OP(stateInfo, );
        }

        U2AssemblyReadsImportInfo & importInfo = importInfos[assemblyNum];
        qint64 maxProw = importInfo.packStat.maxProw;
        qint64 readsCount = importInfo.packStat.readsCount;
        const U2AssemblyCoverageStat & coverageStat = importInfo.coverageInfo.coverage;
        if (maxProw > 0) {
            U2IntegerAttribute maxProwAttr;
            maxProwAttr.objectId = assembly.id;
            maxProwAttr.name = U2BaseAttributeName::max_prow;
            maxProwAttr.version = assembly.version;
            maxProwAttr.value = maxProw;
            attrDbi->createIntegerAttribute(maxProwAttr, stateInfo);
            CHECK_OP(stateInfo, );
        } else if (readsCount > 0) {
            // if there are reads, but maxProw == 0 => error
            taskLog.details(tr("Warning: incorrect maxProw == %1, probably packing was not done! Attribute was not set").arg(maxProw));
        }

        if (readsCount > 0) {
            U2IntegerAttribute countReadsAttr;
            countReadsAttr.objectId = assembly.id;
            countReadsAttr.name = "count_reads_attribute";
            countReadsAttr.version = assembly.version;
            countReadsAttr.value = readsCount;
            attrDbi->createIntegerAttribute(countReadsAttr, stateInfo);
            CHECK_OP(stateInfo, );
        }
        if (!coverageStat.coverage.isEmpty()) {
            U2ByteArrayAttribute attribute;
            attribute.objectId = assembly.id;
            attribute.name = U2BaseAttributeName::coverage_statistics;
            attribute.value = U2AssemblyUtils::serializeCoverageStat(coverageStat);
            attribute.version = assembly.version;
            attrDbi->createByteArrayAttribute(attribute, stateInfo);
            CHECK_OP(stateInfo, );
        }
        stateInfo.setProgress(stateInfo.getProgress() + progressStep);
    }
}

void ConvertAceToSqliteTask::removeCorruptedAssembly() {
    if (corruptedAssembly.id.isEmpty()) {
        return;
    }

    // stateInfo can already have error
    U2OpStatusImpl os;

    U2ObjectDbi* objDbi = dbi->getObjectDbi();
    SAFE_POINT(NULL != objDbi, tr("Object DBI is NULL"), );
    U2AssemblyDbi* assDbi = dbi->getAssemblyDbi();
    SAFE_POINT(NULL != assDbi, tr("Assembly DBI is NULL"), );
    U2CrossDatabaseReferenceDbi* crossDbi = dbi->getCrossDatabaseReferenceDbi();
    SAFE_POINT(NULL != crossDbi, tr("Cross reference DBI is NULL"), );

    // remove the assembly object
    objDbi->removeObject(corruptedAssembly.id, os);
    CHECK_OP(os, );

    // remove reads of the corrupted assembly
    QScopedPointer<U2DbiIterator<U2AssemblyRead> > readsIterator(assDbi->getReads(corruptedAssembly.id, U2_REGION_MAX, os));
    CHECK_OP(os, );
    QList<U2DataId> readIds;
    while (readsIterator->hasNext()) {
        readIds << readsIterator->peek()->id;
    }

    assDbi->removeReads(corruptedAssembly.id, readIds, os);
    CHECK_OP(os, );

    // remove the reference sequence
    U2CrossDatabaseReference crossRef = crossDbi->getCrossReference(corruptedAssembly.referenceId, os);
    CHECK_OP(os, );
    objDbi->removeObject(crossRef.dataRef.entityId, os);
    CHECK_OP(os, );

    // remove cross reference object
    objDbi->removeObject(corruptedAssembly.referenceId, os);
    CHECK_OP(os, );
}

}   // namespace U2
