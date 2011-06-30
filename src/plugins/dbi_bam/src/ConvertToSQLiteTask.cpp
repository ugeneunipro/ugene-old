/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "IOException.h"
#include "CancelledException.h"
#include "Reader.h"
#include "SamReader.h"
#include "Index.h"
#include "Dbi.h"
#include "BAMDbiPlugin.h"
#include "ConvertToSQLiteTask.h"
#include "LoadBamInfoTask.h"

#include <U2Core/U2Dbi.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>

#include <limits>
#include <memory>
#include <time.h>

namespace U2 {
namespace BAM {

ConvertToSQLiteTask::ConvertToSQLiteTask(const GUrl &_sourceUrl, const GUrl &_destinationUrl, BAMInfo& _bamInfo, bool _sam):
    Task(tr("Convert BAM to UGENE database (%1)").arg(_destinationUrl.fileName()), TaskFlag_None),
    sourceUrl(_sourceUrl),
    destinationUrl(_destinationUrl),
    bamInfo(_bamInfo),
    sam(_sam)
{
    GCOUNTER( cvar, tvar, "ConvertBamToUgenedb" );
    tpm = Progress_Manual;
}

static void flushReads(U2Dbi* sqliteDbi, QMap<int, U2Assembly>& assemblies, QMap<int, QList<U2AssemblyRead> >& reads) {
    foreach(int index, assemblies.keys()) {
        if(!reads[index].isEmpty()) {
            U2OpStatusImpl opStatus;
            BufferedDbiIterator<U2AssemblyRead> readsIterator(reads[index]);
            sqliteDbi->getAssemblyDbi()->addReads(assemblies[index].id, &readsIterator, opStatus);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
    }
    reads.clear();
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

class ReferenceIterator {
public:
    ReferenceIterator(int referenceId, Iterator &iterator):
        referenceId(referenceId),
        iterator(iterator)
    {
    }

    U2AssemblyRead next() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        return iterator.next();
    }

    U2AssemblyRead peek() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        return iterator.peek();
    }

    bool hasNext() {
        return iterator.hasNext() && (iterator.peekReferenceId() == referenceId);
    }

private:
    int referenceId;
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
    SequentialDbiIterator(int referenceId, Iterator &iterator, TaskStateInfo &stateInfo, const IOAdapter &ioAdapter):
        referenceIterator(referenceId, iterator),
        readsImported(0),
        stateInfo(stateInfo),
        ioAdapter(ioAdapter)
    {
    }

    virtual bool hasNext() {
        if(stateInfo.isCanceled()) {
            return false;
        }
        return referenceIterator.hasNext();
    }

    virtual U2AssemblyRead next() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        stateInfo.progress = ioAdapter.getProgress();
        readsImported++;
        return referenceIterator.next();
    }

    virtual U2AssemblyRead peek() {
        if(!hasNext()) {
            throw Exception(BAMDbiPlugin::tr("The iteration has no next element"));
        }
        return referenceIterator.peek();
    }

    virtual qint64 getReadsImported() {
        return readsImported;
    }

private:
    ReferenceIterator referenceIterator;
    qint64 readsImported;
    TaskStateInfo &stateInfo;
    const IOAdapter &ioAdapter;
};

class IndexedBamDbiIterator : public DbiIterator {
public:
    IndexedBamDbiIterator(int referenceId, BamReader &reader, const Index &index, TaskStateInfo &stateInfo, const IOAdapter &ioAdapter):
        iterator(reader),
        dbiIterator(referenceId, iterator, stateInfo, ioAdapter)
    {
        {
            VirtualOffset minOffset = VirtualOffset(0xffffffffffffLL, 0xffff);
            foreach(const Index::ReferenceIndex::Bin &bin, index.getReferenceIndices()[referenceId].getBins()) {
                foreach(const Index::ReferenceIndex::Chunk &chunk, bin.getChunks()) {
                    if(minOffset > chunk.getStart()) {
                        minOffset = chunk.getStart();
                    }
                }
            }
            reader.seek(minOffset);
        }
    }

    virtual bool hasNext() {
        return dbiIterator.hasNext();
    }

    virtual U2AssemblyRead next() {
        return dbiIterator.next();
    }

    virtual U2AssemblyRead peek() {
        return dbiIterator.peek();
    }

    virtual qint64 getReadsImported() {
        return dbiIterator.getReadsImported();
    }

private:
    BamIterator iterator;
    SequentialDbiIterator dbiIterator;
};

static const int READS_CHUNK_SIZE = 250*1000;

} // namespace

void ConvertToSQLiteTask::run() {
    try {

        time_t startTime = time(0);

        if(!destinationUrl.isLocalFile()) {
            throw Exception(BAMDbiPlugin::tr("Non-local files are not supported"));
        }
        std::auto_ptr<IOAdapter> ioAdapter;
        {
            IOAdapterFactory *factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(sourceUrl));
            ioAdapter.reset(factory->createIOAdapter());
        }
        if(!ioAdapter->open(sourceUrl, IOAdapterMode_Read)) {
            throw IOException(BAMDbiPlugin::tr("Can't open file '%1'").arg(sourceUrl.getURLString()));
        }

        BamReader *bamReader = NULL;
        SamReader *samReader = NULL;
        std::auto_ptr<Reader> reader(NULL);
        if (sam) {
            samReader = new SamReader(*ioAdapter);
            reader.reset(samReader);
        } else {
            bamReader = new BamReader(*ioAdapter);
            reader.reset(bamReader);
        }

        assert(destinationUrl.isLocalFile());
        bool append = QFile::exists(destinationUrl.getURLString());

        U2OpStatusImpl opStatus;

        DbiHandle dbiHandle("SQLiteDbi", destinationUrl.getURLString(), true, opStatus);
        U2Dbi* sqliteDbi = dbiHandle.dbi;
        if(opStatus.hasError()) {
            throw Exception(opStatus.getError());
        }

        if(!append) {
            sqliteDbi->getObjectDbi()->createFolder("/", opStatus);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }

        QMap<int, U2Assembly> assemblies;

        bool needPacking = false;

        qint64 totalReadsImported = 0;

        stateInfo.setDescription("Importing reads");

        if(bamInfo.hasIndex() ||
           (Header::Coordinate == reader->getHeader().getSortingOrder()) ||
           (Header::QueryName == reader->getHeader().getSortingOrder())) {

            std::auto_ptr<Iterator> iterator;
            if(!bamInfo.hasIndex()) {
                if(sam) {
                    iterator.reset(new SamIterator(*samReader));
                } else {
                    iterator.reset(new BamIterator(*bamReader));
                }
            }

            for(int referenceId = 0;referenceId < reader->getHeader().getReferences().size(); referenceId++) {
                if(bamInfo.isReferenceSelected(referenceId)) {
                    U2Assembly assembly;
                    assembly.visualName = reader->getHeader().getReferences()[referenceId].getName();
                    U2AssemblyReadsImportInfo importInfo;
                    U2OpStatusImpl opStatus;
                    std::auto_ptr<DbiIterator> dbiIterator;
                    if(bamInfo.hasIndex()) {
                        dbiIterator.reset(new IndexedBamDbiIterator(referenceId, *bamReader, bamInfo.getIndex(), stateInfo, *ioAdapter));
                    } else {
                        dbiIterator.reset(new SequentialDbiIterator(referenceId, *iterator, stateInfo, *ioAdapter));
                    }
                    sqliteDbi->getAssemblyDbi()->createAssemblyObject(assembly, "/", dbiIterator.get(), importInfo, opStatus);
                    if(opStatus.hasError()) {
                        throw Exception(opStatus.getError());
                    }
                    if(isCanceled()) {
                        throw CancelledException(BAMDbiPlugin::tr("Task was cancelled"));
                    }
                    if(!importInfo.packed) {
                        needPacking = true;
                    }
                    totalReadsImported += dbiIterator->getReadsImported();
                    assemblies.insert(referenceId, assembly);
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
                SequentialDbiIterator dbiIterator(-1, *iterator, stateInfo, *ioAdapter);
                U2Assembly assembly;
                assembly.visualName = "Unmapped";
                U2AssemblyReadsImportInfo importInfo;
                U2OpStatusImpl opStatus;
                sqliteDbi->getAssemblyDbi()->createAssemblyObject(assembly, "/", &dbiIterator, importInfo, opStatus);
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
            needPacking = true;

            std::auto_ptr<Iterator> iterator;
            if(sam) {
                iterator.reset(new SamIterator(*samReader));
            } else {
                iterator.reset(new BamIterator(*bamReader));
            }

            for(int referenceId = 0;referenceId < reader->getHeader().getReferences().size(); referenceId++) {
                if(bamInfo.isReferenceSelected(referenceId)) {
                    U2Assembly assembly;
                    assembly.visualName = reader->getHeader().getReferences()[referenceId].getName();
                    U2AssemblyReadsImportInfo importInfo;
                    U2OpStatusImpl opStatus;
                    sqliteDbi->getAssemblyDbi()->createAssemblyObject(assembly, "/", NULL, importInfo, opStatus);
                    if(opStatus.hasError()) {
                        throw Exception(opStatus.getError());
                    }
                    if(isCanceled()) {
                        throw CancelledException(BAMDbiPlugin::tr("Task was cancelled"));
                    }
                    assemblies.insert(referenceId, assembly);
                }
            }

            if(bamInfo.isUnmappedSelected()) {
                U2Assembly assembly;
                assembly.visualName = "Unmapped";
                U2AssemblyReadsImportInfo importInfo;
                U2OpStatusImpl opStatus;
                sqliteDbi->getAssemblyDbi()->createAssemblyObject(assembly, "/", NULL, importInfo, opStatus);
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
                assemblies.insert(-1, assembly);
            }

            while(iterator->hasNext()) {
                QMap<int, QList<U2AssemblyRead> > reads;
                foreach(int index, assemblies.keys()) {
                    reads.insert(index, QList<U2AssemblyRead>());
                }
                int readCount = 0;
                while(iterator->hasNext() && (readCount < READS_CHUNK_SIZE)) {
                    if(((-1 == iterator->peekReferenceId()) && bamInfo.isUnmappedSelected()) ||
                        bamInfo.isReferenceSelected(iterator->peekReferenceId())) {
                        reads[iterator->peekReferenceId()].append(iterator->next());
                        readCount++;
                    } else {
                        iterator->skip();
                    }
                }
                if(isCanceled()) {
                    throw CancelledException(BAMDbiPlugin::tr("Task was cancelled"));
                }
                flushReads(sqliteDbi, assemblies, reads);
                totalReadsImported += readCount;
            }
        }

        U2AttributeDbi *attributeDbi = sqliteDbi->getAttributeDbi();

        if(NULL != attributeDbi) {
            foreach(int referenceId, assemblies.keys()) {
                if(-1 != referenceId) {
                    const Header::Reference &reference = reader->getHeader().getReferences()[referenceId];
                    const U2Assembly &assembly = assemblies[referenceId];
                    {
                        U2IntegerAttribute lenAttr;
                        lenAttr.objectId = assembly.id;
                        lenAttr.name = "reference_length_attribute";
                        lenAttr.version = 1;
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
                        md5Attr.name = "reference_md5_attribute";
                        md5Attr.version = 1;
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
                        speciesAttr.name = "reference_species_attribute";
                        speciesAttr.version = 1;
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
                        uriAttr.name = "reference_uri_attribute";
                        uriAttr.version = 1;
                        uriAttr.value = reference.getUri();
                        U2OpStatusImpl status;
                        attributeDbi->createStringAttribute(uriAttr, status);
                        if(status.hasError()) {
                            throw Exception(status.getError());
                        }
                    }
                }
            }
        }

        stateInfo.setDescription("Packing reads");

        time_t packStart = time(0);
        if(needPacking) {
            foreach(int referenceId, assemblies.keys()) {
                U2OpStatusImpl opStatus;
                U2AssemblyPackStat stat;
                sqliteDbi->getAssemblyDbi()->pack(assemblies[referenceId].id, stat, opStatus);
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
                if(NULL != attributeDbi) {
                    {
                        U2IntegerAttribute maxProwAttr;
                        maxProwAttr.objectId = assemblies[referenceId].id;
                        maxProwAttr.name = "max_prow_attribute";
                        maxProwAttr.version = 1;
                        maxProwAttr.value = stat.maxProw;
                        attributeDbi->createIntegerAttribute(maxProwAttr, opStatus);
                        if(opStatus.hasError()) {
                            throw Exception(opStatus.getError());
                        }
                    }
                    {
                        U2IntegerAttribute countReadsAttr;
                        countReadsAttr.objectId = assemblies[referenceId].id;
                        countReadsAttr.name = "count_reads_attribute";
                        countReadsAttr.version = 1;
                        countReadsAttr.value = stat.readsCount;
                        attributeDbi->createIntegerAttribute(countReadsAttr, opStatus);
                        if(opStatus.hasError()) {
                            throw Exception(opStatus.getError());
                        }
                    }
                }
            }
        }
        time_t packTime = time(0) - packStart;

        time_t totalTime = time(0) - startTime;
        
        ioLog.trace(QString("BAM %1: imported %2 reads, total time %3 s, pack time %4").arg(sourceUrl.fileName()).arg(QString::number(totalReadsImported)).arg(QString::number(totalTime)).arg(QString::number(packTime)));

    } catch(const CancelledException & /*e*/) {
        assert(destinationUrl.isLocalFile());
        QFile::remove(destinationUrl.getURLString());
    } catch(const Exception &e) {
        setError(e.getMessage());
        assert(destinationUrl.isLocalFile());
        QFile::remove(destinationUrl.getURLString());
    }
}

const GUrl &ConvertToSQLiteTask::getDestinationUrl() const {
    return destinationUrl;
}

} // namespace BAM
} // namespace U2
