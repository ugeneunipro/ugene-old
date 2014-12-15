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

#include <limits>
#include <time.h>

#include <QtCore/QFile>
#include <QtCore/QScopedPointer>

#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2CoreAttributes.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "BAMDbiPlugin.h"
#include "CancelledException.h"
#include "ConvertToSQLiteTask.h"
#include "Dbi.h"
#include "Index.h"
#include "IOException.h"
#include "LoadBamInfoTask.h"
#include "Reader.h"
#include "SamReader.h"

namespace U2 {
namespace BAM {

ConvertToSQLiteTask::ConvertToSQLiteTask(const GUrl &_sourceUrl, const U2DbiRef &dstDbiRef, BAMInfo& _bamInfo, bool _sam):
    Task(tr("Convert BAM to UGENE database (%1)").arg(_sourceUrl.fileName()), TaskFlag_None),
    sourceUrl(_sourceUrl),
    dstDbiRef(dstDbiRef),
    bamInfo(_bamInfo),
    sam(_sam)
{
    GCOUNTER( cvar, tvar, "ConvertBamToUgenedb" );
    tpm = Progress_Manual;
}

static void flushReads(U2Dbi* dbi, QMap<int, U2Assembly>& assemblies, QMap<int, QList<U2AssemblyRead> >& reads) {
    foreach(int index, assemblies.keys()) {
        if(!reads[index].isEmpty()) {
            U2OpStatusImpl opStatus;
            BufferedDbiIterator<U2AssemblyRead> readsIterator(reads[index]);
            dbi->getAssemblyDbi()->addReads(assemblies[index].id, &readsIterator, opStatus);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
    }
    reads.clear();
}

static void enableCoverageOnImport(U2AssemblyCoverageImportInfo &cii, int referenceLength) {
    cii.computeCoverage = true;
    int coverageInfoSize = qMin(U2AssemblyUtils::MAX_COVERAGE_VECTOR_SIZE, referenceLength);
    cii.coverageBasesPerPoint = qMax(1.0, ((double)referenceLength)/coverageInfoSize);
    cii.coverage.coverage.resize(coverageInfoSize);
}

namespace {

class Iterator {
public:
    virtual ~Iterator() {}

    virtual bool hasNext() = 0;

    virtual U2AssemblyRead next() = 0;

    virtual void skip() = 0;

    virtual const U2AssemblyRead &peek() = 0;

    virtual int peekReferenceId() = 0;
};

class BamIterator : public Iterator {
public:
    BamIterator(BamReader &reader):
        reader(reader),
        alignmentReader(NULL, 0, 0),
        alignmentReaderValid(false),
        readValid(false)
    {
    }

    virtual bool hasNext() {
        return readValid || alignmentReaderValid || !reader.isEof();
    }

    virtual U2AssemblyRead next() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        if(!readValid) {
            if(!alignmentReaderValid) {
                alignmentReader = reader.getAlignmentReader();
            }
            alignmentReaderValid = false;
            read = AssemblyDbi::alignmentToRead(alignmentReader.read());
        }
        readValid = false;
        return read;
    }

    virtual void skip() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        if(!readValid) {
            if(!alignmentReaderValid) {
                alignmentReader = reader.getAlignmentReader();
            }
            alignmentReaderValid = false;
            alignmentReader.skip();
        }
        readValid = false;
    }

    virtual const U2AssemblyRead &peek() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        if(!readValid) {
            if(!alignmentReaderValid) {
                alignmentReader = reader.getAlignmentReader();
            }
            alignmentReaderValid = false;
            read = AssemblyDbi::alignmentToRead(alignmentReader.read());
            readValid = true;
        }
        return read;
    }

    virtual int peekReferenceId() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        if(!readValid) {
            if(!alignmentReaderValid) {
                alignmentReader = reader.getAlignmentReader();
                alignmentReaderValid = true;
            }
        }
        return alignmentReader.getId();
    }

private:
    BamReader &reader;
    BamReader::AlignmentReader alignmentReader;
    bool alignmentReaderValid;
    U2AssemblyRead read;
    bool readValid;
};

class SamIterator : public Iterator {
public:
    SamIterator(SamReader &reader):
        reader(reader),
        readValid(false)
    {
    }

    virtual bool hasNext() {
        return readValid || !reader.isEof();
    }

    virtual U2AssemblyRead next() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        if(!readValid) {
            bool eof = false;
            read = AssemblyDbi::alignmentToRead(reader.readAlignment(eof));
        }
        readValid = false;
        return read;
    }

    virtual void skip() {
        next();
    }

    virtual const U2AssemblyRead &peek() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        if(!readValid) {
            bool eof = false;
            Alignment alignemnt = reader.readAlignment(eof);
            readReferenceId = alignemnt.getReferenceId();
            read = AssemblyDbi::alignmentToRead(alignemnt);
            readValid = true;
        }
        return read;
    }

    virtual int peekReferenceId() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        if(!readValid) {
            bool eof = false;
            Alignment alignemnt = reader.readAlignment(eof);
            readReferenceId = alignemnt.getReferenceId();
            read = AssemblyDbi::alignmentToRead(alignemnt);
            readValid = true;
        }
        return readReferenceId;
    }

private:
    SamReader &reader;
    U2AssemblyRead read;
    int readReferenceId;
    bool readValid;
};

class ReferenceIterator : public Iterator {
public:
    ReferenceIterator(int referenceId, Iterator &iterator):
        referenceId(referenceId),
        iterator(iterator)
    {
    }

    virtual bool hasNext() {
        return iterator.hasNext() && (iterator.peekReferenceId() == referenceId);
    }

    virtual U2AssemblyRead next() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        return iterator.next();
    }

    virtual void skip() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        iterator.skip();
    }

    virtual const U2AssemblyRead &peek() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        return iterator.peek();
    }

    virtual int peekReferenceId() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        return iterator.peekReferenceId();
    }

private:
    int referenceId;
    Iterator &iterator;
};

class SkipUnmappedIterator : public Iterator {
public:
    SkipUnmappedIterator(Iterator &iterator):
        iterator(iterator)
    {
    }

    virtual bool hasNext() {
        skipUnmappedReads();
        return iterator.hasNext();
    }

    virtual U2AssemblyRead next() {
        skipUnmappedReads();
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        return iterator.next();
    }

    virtual void skip() {
        skipUnmappedReads();
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        iterator.skip();
    }

    virtual const U2AssemblyRead &peek() {
        skipUnmappedReads();
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        return iterator.peek();
    }

    virtual int peekReferenceId() {
        skipUnmappedReads();
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        return iterator.peekReferenceId();
    }

private:
    void skipUnmappedReads() {
        while(iterator.hasNext()) {
            if(-1 == iterator.peekReferenceId() ||
               ReadFlagsUtils::isUnmappedRead(iterator.peek()->flags) ||
               iterator.peek()->cigar.isEmpty()) {
                iterator.skip();
            } else {
                break;
            }
        }
    }

private:
    Iterator &iterator;
};

class DbiIterator : public U2DbiIterator<U2AssemblyRead> {
public:
    virtual ~DbiIterator() {}

    virtual bool hasNext() = 0;

    virtual U2AssemblyRead next() = 0;

    virtual U2AssemblyRead peek() = 0;

    virtual qint64 getReadsImported() = 0;
};

class SequentialDbiIterator : public DbiIterator {
public:
    SequentialDbiIterator(int referenceId, bool skipUnmapped, Iterator &inputIterator, TaskStateInfo &stateInfo, const IOAdapter &ioAdapter):
        referenceIterator(referenceId, inputIterator),
        skipUnmappedIterator(skipUnmapped? new SkipUnmappedIterator(referenceIterator):NULL),
        iterator(skipUnmapped? (Iterator *)skipUnmappedIterator.data():(Iterator *)&referenceIterator),
        readsImported(0),
        stateInfo(stateInfo),
        ioAdapter(ioAdapter)
    {
    }

    virtual bool hasNext() {
        if(stateInfo.isCanceled()) {
            throw CancelledException(BAMDbiPlugin::tr("Task was cancelled"));
        }
        return iterator->hasNext();
    }

    virtual U2AssemblyRead next() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        stateInfo.progress = ioAdapter.getProgress();
        readsImported++;
        return iterator->next();
    }

    virtual U2AssemblyRead peek() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        return iterator->peek();
    }

    virtual qint64 getReadsImported() {
        return readsImported;
    }

private:
    ReferenceIterator referenceIterator;
    QScopedPointer<SkipUnmappedIterator> skipUnmappedIterator;
    Iterator *iterator;
    qint64 readsImported;
    TaskStateInfo &stateInfo;
    const IOAdapter &ioAdapter;
};

class IndexedBamDbiIterator : public DbiIterator {
public:
    IndexedBamDbiIterator(int referenceId, bool skipUnmapped, BamReader &reader, const Index &index, TaskStateInfo &stateInfo, const IOAdapter &ioAdapter):
        iterator(reader),
        dbiIterator(referenceId, skipUnmapped, iterator, stateInfo, ioAdapter),
        hasReads(false)
    {
        {
            VirtualOffset minOffset = VirtualOffset(0xffffffffffffLL, 0xffff);
            foreach(const Index::ReferenceIndex::Bin &bin, index.getReferenceIndices()[referenceId].getBins()) {
                foreach(const Index::ReferenceIndex::Chunk &chunk, bin.getChunks()) {
                    if(minOffset > chunk.getStart()) {
                        minOffset = chunk.getStart();
                        hasReads = true;
                    }
                }
            }
            if(hasReads) {
                reader.seek(minOffset);
            }
        }
    }

    virtual bool hasNext() {
        return hasReads && dbiIterator.hasNext();
    }

    virtual U2AssemblyRead next() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        return dbiIterator.next();
    }

    virtual U2AssemblyRead peek() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        return dbiIterator.peek();
    }

    virtual qint64 getReadsImported() {
        return dbiIterator.getReadsImported();
    }

private:
    BamIterator iterator;
    SequentialDbiIterator dbiIterator;
    bool hasReads;
};

static const int READS_CHUNK_SIZE = 250*1000;

} // namespace

void ConvertToSQLiteTask::run() {
    try {

        taskLog.info(tr("Converting assembly from %1 to %2 started")
                     .arg(sourceUrl.fileName())
                     .arg(getDestinationUrl().fileName()));

        time_t startTime = time(0);

        QScopedPointer<IOAdapter> ioAdapter;
        {
            IOAdapterFactory *factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(sourceUrl));
            ioAdapter.reset(factory->createIOAdapter());
        }
        if(!ioAdapter->open(sourceUrl, IOAdapterMode_Read)) {
            throw IOException(BAMDbiPlugin::tr("Can't open file '%1'").arg(sourceUrl.getURLString()));
        }

        BamReader *bamReader = NULL;
        SamReader *samReader = NULL;
        QScopedPointer<Reader> reader(NULL);
        if (sam) {
            samReader = new SamReader(*ioAdapter);
            reader.reset(samReader);
        } else {
            bamReader = new BamReader(*ioAdapter);
            reader.reset(bamReader);
        }

        U2OpStatusImpl opStatus;

        DbiConnection dbiHandle(dstDbiRef, true, opStatus);
        if (opStatus.hasError()) {
            throw Exception(opStatus.getError());
        }
        U2Dbi* dbi = dbiHandle.dbi;

        DbiOperationsBlock opBlock(dstDbiRef, stateInfo);
        Q_UNUSED(opBlock);
        CHECK_OP(stateInfo, );

        QMap<int, U2AssemblyReadsImportInfo> importInfos;

        qint64 totalReadsImported = 0;

        stateInfo.setDescription("Importing reads");

        if((Header::Coordinate == reader->getHeader().getSortingOrder()) ||
           (Header::QueryName == reader->getHeader().getSortingOrder())) {

            QScopedPointer<Iterator> iterator;
            if(!bamInfo.hasIndex()) {
                if(sam) {
                    iterator.reset(new SamIterator(*samReader));
                } else {
                    iterator.reset(new BamIterator(*bamReader));
                }
            }

            const QList<Header::Reference> &references = reader->getHeader().getReferences();
            for(int referenceId = 0;referenceId < references.size(); referenceId++) {

                if(bamInfo.isReferenceSelected(referenceId)) {
                    U2Assembly assembly;
                    assembly.visualName = references[referenceId].getName();
                    taskLog.details(tr("Importing assembly '%1' (%2 of %3)")
                                    .arg(assembly.visualName)
                                    .arg(referenceId + 1)
                                    .arg(reader->getHeader().getReferences().size()));

                    U2AssemblyReadsImportInfo & importInfo = importInfos[referenceId];
                    enableCoverageOnImport(importInfo.coverageInfo, references[referenceId].getLength());
                    U2OpStatusImpl opStatus;
                    QScopedPointer<DbiIterator> dbiIterator;
                    if(bamInfo.hasIndex()) {
                        dbiIterator.reset(new IndexedBamDbiIterator(referenceId, !bamInfo.isUnmappedSelected(), *bamReader, bamInfo.getIndex(), stateInfo, *ioAdapter));
                    } else {
                        dbiIterator.reset(new SequentialDbiIterator(referenceId, !bamInfo.isUnmappedSelected(), *iterator, stateInfo, *ioAdapter));
                    }
                    dbi->getAssemblyDbi()->createAssemblyObject(assembly, U2ObjectDbi::ROOT_FOLDER, dbiIterator.data(), importInfo, opStatus);
                    if(opStatus.hasError()) {
                        throw Exception(opStatus.getError());
                    }
                    if(isCanceled()) {
                        throw CancelledException(BAMDbiPlugin::tr("Task was cancelled"));
                    }
                    totalReadsImported += dbiIterator->getReadsImported();
                    assemblies.insert(referenceId, assembly);
                    taskLog.details(tr("Successfully imported %1 reads for assembly '%2' (total %3 reads imported)")
                                    .arg(dbiIterator->getReadsImported())
                                    .arg(assembly.visualName)
                                    .arg(totalReadsImported));
                } else {
                    if(!bamInfo.hasIndex()) {
                        while(iterator->hasNext() && iterator->peekReferenceId() == referenceId) {
                            iterator->skip();
                        }
                        if(isCanceled()) {
                            throw CancelledException(BAMDbiPlugin::tr("Task was cancelled"));
                        }
                    }
                }
            }

            if(bamInfo.isUnmappedSelected()) {
                taskLog.details(tr("Importing unmapped reads"));
                if(bamInfo.hasIndex() && !reader->getHeader().getReferences().isEmpty()) {
                    const Index &index = bamInfo.getIndex();
                    bool maxOffsetFound = false;
                    VirtualOffset maxOffset = VirtualOffset(0, 0);
                    for(int refId = 0; refId < reader->getHeader().getReferences().size(); ++refId) {
                        foreach(const Index::ReferenceIndex::Bin &bin, index.getReferenceIndices()[refId].getBins()) {
                            foreach(const Index::ReferenceIndex::Chunk &chunk, bin.getChunks()) {
                                if(chunk.getStart() < chunk.getEnd() && maxOffset < chunk.getStart()) {
                                    maxOffset = chunk.getStart();
                                    maxOffsetFound = true;
                                }
                            }
                        }
                    }
                    if(maxOffsetFound) {
                        bamReader->seek(maxOffset);
                        iterator.reset(new BamIterator(*bamReader));
                        while(iterator->hasNext() && iterator->peekReferenceId() != -1) {
                            iterator->skip();
                        }
                    }else{
                        iterator.reset(new BamIterator(*bamReader));
                    }
                }
                SequentialDbiIterator dbiIterator(-1, false, *iterator, stateInfo, *ioAdapter);
                U2Assembly assembly;
                assembly.visualName = "Unmapped";
                U2AssemblyReadsImportInfo & importInfo = importInfos[-1];
                U2OpStatusImpl opStatus;
                dbi->getAssemblyDbi()->createAssemblyObject(assembly, U2ObjectDbi::ROOT_FOLDER, &dbiIterator, importInfo, opStatus);
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
                if(isCanceled()) {
                    throw CancelledException(BAMDbiPlugin::tr("Task was cancelled"));
                }
                totalReadsImported += dbiIterator.getReadsImported();
                assemblies.insert(-1, assembly);
            }
        } else {
            QScopedPointer<Iterator> inputIterator;
            if(sam) {
                inputIterator.reset(new SamIterator(*samReader));
            } else {
                inputIterator.reset(new BamIterator(*bamReader));
            }
            QScopedPointer<SkipUnmappedIterator> skipUnmappedIterator;
            Iterator *iterator;
            if(!bamInfo.isUnmappedSelected()) {
                skipUnmappedIterator.reset(new SkipUnmappedIterator(*inputIterator));
                iterator = skipUnmappedIterator.data();
            } else {
                iterator = inputIterator.data();
            }

            taskLog.details(tr("No bam index given, preparing sequential import"));
            for(int referenceId = 0;referenceId < reader->getHeader().getReferences().size(); referenceId++) {
                if(bamInfo.isReferenceSelected(referenceId)) {
                    U2Assembly assembly;
                    assembly.visualName = reader->getHeader().getReferences()[referenceId].getName();

                    U2AssemblyReadsImportInfo & importInfo = importInfos[referenceId];
                    U2OpStatusImpl opStatus;
                    dbi->getAssemblyDbi()->createAssemblyObject(assembly, U2ObjectDbi::ROOT_FOLDER, NULL, importInfo, opStatus);
                    if(opStatus.hasError()) {
                        throw Exception(opStatus.getError());
                    }
                    if(isCanceled()) {
                        throw CancelledException(BAMDbiPlugin::tr("Task was cancelled"));
                    }
                    importInfo.packed = false;
                    assemblies.insert(referenceId, assembly);
                }
            }

            if(bamInfo.isUnmappedSelected()) {
                U2Assembly assembly;
                assembly.visualName = "Unmapped";
                U2AssemblyReadsImportInfo & importInfo = importInfos[-1];
                U2OpStatusImpl opStatus;
                dbi->getAssemblyDbi()->createAssemblyObject(assembly, U2ObjectDbi::ROOT_FOLDER, NULL, importInfo, opStatus);
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
                assemblies.insert(-1, assembly);
                importInfo.packed = false;
            }

            taskLog.details(tr("Importing reads sequentially"));
            while(iterator->hasNext()) {
                QMap<int, QList<U2AssemblyRead> > reads;
                foreach(int index, assemblies.keys()) {
                    reads.insert(index, QList<U2AssemblyRead>());
                }
                int readCount = 0;
                while(iterator->hasNext() && (readCount < READS_CHUNK_SIZE)) {
                    int referenceId = iterator->peekReferenceId();
                    if((-1 == referenceId) || bamInfo.isReferenceSelected(referenceId)) {
                        U2AssemblyReadsImportInfo & importInfo = importInfos[referenceId];
                        reads[referenceId].append(iterator->next());
                        readCount++;
                        importInfo.nReads++;
                    } else {
                        iterator->skip();
                    }
                }
                if(isCanceled()) {
                    throw CancelledException(BAMDbiPlugin::tr("Task was cancelled"));
                }
                flushReads(dbi, assemblies, reads);
                totalReadsImported += readCount;
            }
        }

        stateInfo.setDescription("Packing reads");

        time_t packStart = time(0);
        foreach(int referenceId, assemblies.keys()) {
            U2AssemblyReadsImportInfo & importInfo = importInfos[referenceId];
            // Pack reads only if t were not packed on import
            if(!importInfo.packed) {
                taskLog.details(tr("Packing reads for assembly '%1' (%2 of %3)")
                                .arg(assemblies[referenceId].visualName)
                                .arg(referenceId + 1)
                                .arg(reader->getHeader().getReferences().size()));

                U2OpStatusImpl opStatus;
                U2AssemblyPackStat stat;
                dbi->getAssemblyDbi()->pack(assemblies[referenceId].id, stat, opStatus);
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
                importInfo.packStat = stat;
            }
        }
        time_t packTime = time(0) - packStart;

        U2AttributeDbi *attributeDbi = dbi->getAttributeDbi();

        if(NULL != attributeDbi) {
            foreach(int referenceId, assemblies.keys()) {
                const U2Assembly &assembly = assemblies[referenceId];
                if(-1 != referenceId) {
                    const Header::Reference &reference = reader->getHeader().getReferences()[referenceId];
                    {
                        U2IntegerAttribute lenAttr;
                        lenAttr.objectId = assembly.id;
                        lenAttr.name = U2BaseAttributeName::reference_length;
                        lenAttr.version = assembly.version;
                        lenAttr.value = reference.getLength();
                        U2OpStatusImpl status;
                        attributeDbi->createIntegerAttribute(lenAttr, status);
                        if(status.hasError()) {
                            throw Exception(status.getError());
                        }
                    }
                    if(!reference.getMd5().isEmpty()) {
                        U2ByteArrayAttribute md5Attr;
                        md5Attr.objectId = assembly.id;
                        md5Attr.name = U2BaseAttributeName::reference_md5;
                        md5Attr.version = assembly.version;
                        md5Attr.value = reference.getMd5();
                        U2OpStatusImpl status;
                        attributeDbi->createByteArrayAttribute(md5Attr, status);
                        if(status.hasError()) {
                            throw Exception(status.getError());
                        }
                    }
                    if(!reference.getSpecies().isEmpty()) {
                        U2ByteArrayAttribute speciesAttr;
                        speciesAttr.objectId = assembly.id;
                        speciesAttr.name = U2BaseAttributeName::reference_species;
                        speciesAttr.version = assembly.version;
                        speciesAttr.value = reference.getSpecies();
                        U2OpStatusImpl status;
                        attributeDbi->createByteArrayAttribute(speciesAttr, status);
                        if(status.hasError()) {
                            throw Exception(status.getError());
                        }
                    }
                    if(!reference.getUri().isEmpty()) {
                        U2StringAttribute uriAttr;
                        uriAttr.objectId = assembly.id;
                        uriAttr.name = U2BaseAttributeName::reference_uri;
                        uriAttr.version = assembly.version;
                        uriAttr.value = reference.getUri();
                        U2OpStatusImpl status;
                        attributeDbi->createStringAttribute(uriAttr, status);
                        if(status.hasError()) {
                            throw Exception(status.getError());
                        }
                    }
                }

                U2AssemblyReadsImportInfo & importInfo = importInfos[referenceId];
                qint64 maxProw = importInfo.packStat.maxProw;
                qint64 readsCount = importInfo.packStat.readsCount;
                const U2AssemblyCoverageStat & coverageStat = importInfo.coverageInfo.coverage;
                if(maxProw > 0)
                {
                    U2IntegerAttribute maxProwAttr;
                    maxProwAttr.objectId = assembly.id;
                    maxProwAttr.name = U2BaseAttributeName::max_prow;
                    maxProwAttr.version = assembly.version;
                    maxProwAttr.value = maxProw;
                    attributeDbi->createIntegerAttribute(maxProwAttr, opStatus);
                    if(opStatus.hasError()) {
                        throw Exception(opStatus.getError());
                    }
                } else if(readsCount > 0){
                    // if there are reads, but maxProw == 0 => error
                    taskLog.details(QString("Warning: incorrect maxProw == %1, probably packing was not done! Attribute was not set").arg(maxProw));
                }
                if(readsCount > 0)
                {
                    U2IntegerAttribute countReadsAttr;
                    countReadsAttr.objectId = assembly.id;
                    countReadsAttr.name = "count_reads_attribute";
                    countReadsAttr.version = assembly.version;
                    countReadsAttr.value = readsCount;
                    attributeDbi->createIntegerAttribute(countReadsAttr, opStatus);
                    if(opStatus.hasError()) {
                        throw Exception(opStatus.getError());
                    }
                }
                if(!coverageStat.coverage.isEmpty()) {
                    U2ByteArrayAttribute attribute;
                    attribute.objectId = assembly.id;
                    attribute.name = U2BaseAttributeName::coverage_statistics;
                    attribute.value = U2AssemblyUtils::serializeCoverageStat(coverageStat);
                    attribute.version = assembly.version;
                    attributeDbi->createByteArrayAttribute(attribute, opStatus);
                    if(opStatus.hasError()) {
                        throw Exception(opStatus.getError());
                    }
                }
            }
        }

        time_t totalTime = time(0) - startTime;

        taskLog.info(QString("Converting assembly from %1 to %2 succesfully finished: imported %3 reads, total time %4 s, pack time %5 s")
                     .arg(sourceUrl.fileName())
                     .arg(getDestinationUrl().fileName())
                     .arg(totalReadsImported)
                     .arg(totalTime)
                     .arg(packTime));

    } catch(const CancelledException & /*e*/) {
        if (getDestinationUrl().isLocalFile()) {
            QFile::remove(getDestinationUrl().getURLString());
        }
        taskLog.info(tr("Converting assembly from %1 to %2 cancelled")
                     .arg(sourceUrl.fileName())
                     .arg(getDestinationUrl().fileName()));
    } catch(const Exception &e) {
        setError(tr("Converting assembly from %1 to %2 failed: %3")
                 .arg(sourceUrl.fileName())
                 .arg(getDestinationUrl().fileName())
                 .arg(e.getMessage()));
        if (getDestinationUrl().isLocalFile()) {
            QFile::remove(getDestinationUrl().getURLString());
        }
    }
}

GUrl ConvertToSQLiteTask::getDestinationUrl() const {
    return GUrl(U2DbiUtils::ref2Url(dstDbiRef));
}

QList<U2Assembly> ConvertToSQLiteTask::getAssemblies() const {
    return assemblies.values();
}

} // namespace BAM
} // namespace U2
