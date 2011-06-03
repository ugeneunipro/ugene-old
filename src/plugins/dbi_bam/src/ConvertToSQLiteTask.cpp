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
#include "Reader.h"
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

#include <memory>
#include <time.h>

namespace U2 {
namespace BAM {

#define READS_CHUNK_SIZE (250*1000)
static const int FIRST_STAGE_PERCENT = 60;
static const int SECOND_STAGE_PERCENT = 40;

ConvertToSQLiteTask::ConvertToSQLiteTask(const GUrl &_sourceUrl, const GUrl &_destinationUrl, BAMInfo& _bamInfo):
    Task(tr("Convert BAM to UGENE database (%1)").arg(_destinationUrl.fileName()), TaskFlag_None),
    sourceUrl(_sourceUrl),
    destinationUrl(_destinationUrl),
    bamInfo(_bamInfo)
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

class BAMDbiIterator : public U2DbiIterator<U2AssemblyRead> {
public:
    BAMDbiIterator(int referenceId, Reader* _reader, QList<Index::ReferenceIndex::Chunk> _chunks, qint64 _totalChunksLength, qint64& _readLength, TaskStateInfo& _ti)
        : refId(referenceId), reader(_reader), chunks(_chunks), totalChunksLength(_totalChunksLength), readLength(_readLength), chunksLength(0), ti(_ti),
        bufferCount(0), bufferIndex(0), chunkIndex(0), offset(0, 0), readsCount(0), readTime(0)
    {
        foreach(const Index::ReferenceIndex::Chunk& chunk, chunks) {
            chunksLength += chunk.getEnd().getCoffset() - chunk.getStart().getCoffset();
        }
    }

    bool hasNext() {
        if(bufferIndex >= bufferCount) {
            fill();
        }
        return bufferIndex < bufferCount;                   
    }

    U2AssemblyRead next() {
        assert(hasNext());
        return buffer[bufferIndex++];
    }

    U2AssemblyRead peek() {
        assert(bufferIndex < bufferCount);
        return buffer[bufferIndex];
    }

    qint64 getImportedCount() {
        return readsCount;
    }

    time_t getReadTime() {
        return readTime;
    }

private:

    void fill() {
        bufferCount = 0;
        bufferIndex = 0;
        int statusUpdateCount = 0;
        ti.setDescription(BAMDbiPlugin::tr("Reading"));
        time_t startTime = time(0);
        while(chunkIndex < chunks.count() && bufferCount < BUFFER_SIZE) {
            const Index::ReferenceIndex::Chunk& chunk = chunks.at(chunkIndex);
            VirtualOffset start = offset <= chunk.getStart() ? chunk.getStart() : offset;
            reader->seek(start);
            while(reader->getOffset() < chunk.getEnd() && bufferCount < BUFFER_SIZE) {
                if(reader->isEof()) {
                    throw IOException(BAMDbiPlugin::tr("Unexpected end of file"));
                }
                
                Reader::AlignmentReader aReader = reader->getAlignmentReader();
                if(aReader.getId() == refId) {
                    buffer[bufferCount++] = AssemblyDbi::alignmentToRead(aReader.read());
                } else {
                    aReader.skip();
                }

                if(++statusUpdateCount > 1000) {
                    if(ti.cancelFlag) {
                        throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
                    }
                    ti.progress = FIRST_STAGE_PERCENT * (readLength + reader->getOffset().getCoffset() - start.getCoffset()) / totalChunksLength;
                    ti.setDescription(BAMDbiPlugin::tr("Reading (%1 reads)").arg(QString::number(bufferCount)));
                }
            }
            offset = reader->getOffset();
            if(offset == chunk.getEnd()) {
                chunkIndex++;
                offset = VirtualOffset(0);
            }
            readLength += reader->getOffset().getCoffset() - start.getCoffset();        
        }
        readTime += time(0) - startTime;
        readsCount += bufferCount;
        ti.setDescription(BAMDbiPlugin::tr("Saving reads"));
    }

    static const int BUFFER_SIZE = 100 * 1000;
    int refId;
    Reader* reader;
    QList<Index::ReferenceIndex::Chunk> chunks;
    qint64 totalChunksLength;
    qint64& readLength;
    qint64 chunksLength;
    TaskStateInfo& ti;
    U2AssemblyRead buffer[BUFFER_SIZE];
    int bufferCount;
    int bufferIndex;
    int chunkIndex;
    VirtualOffset offset;
    qint64 readsCount;
    time_t readTime;
};

static bool chunkLessThan(const Index::ReferenceIndex::Chunk &c1, const Index::ReferenceIndex::Chunk &c2) {
    return c1.getStart() < c2.getStart();
}

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
        std::auto_ptr<Reader> reader(new Reader(*ioAdapter));

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
        QMap<int, QList<Index::ReferenceIndex::Chunk> > chunks;

        //Prepare index chunks

        qint64 totalChunksLength = 0;
        qint64 readLength = 0;
        if(bamInfo.hasIndex() && !bamInfo.isUnmappedSelected()) {
            for(int i=0; i < reader->getHeader().getReferences().count(); i++) {
                if(bamInfo.isReferenceSelected(i)) {
                    const QList<Index::ReferenceIndex::Bin>& bins = bamInfo.getIndex().getReferenceIndices().at(i).getBins();
                    if(bins.isEmpty()) {
                        continue;
                    }
                    //unsigned int lastBin = bins.last().getBin();
                    //unsigned int minBin = lastBin > 0 ? (lastBin > 8 ? (lastBin > 72 ? (lastBin > 584 ? (lastBin > 4680 ? 4681 : 585) : 73) : 9) : 1) : 0;
                    QList<Index::ReferenceIndex::Chunk> refChunks;
                    foreach(const Index::ReferenceIndex::Bin bin, bins) {
                        //if(bin.getBin() >= minBin) {
                            foreach(const Index::ReferenceIndex::Chunk& chunk, bin.getChunks()) {
                                if(chunk.getStart() < chunk.getEnd()) {
                                    refChunks.append(chunk);
                                    totalChunksLength += chunk.getEnd().getCoffset() - chunk.getStart().getCoffset();
                                } else {
                                    coreLog.error(BAMDbiPlugin::tr("Skip invalid chunk: bin %1, chunk begin %2, chunk end %3").arg(bin.getBin())
                                        .arg(chunk.getStart().getPackedOffset())
                                        .arg(chunk.getEnd().getPackedOffset()));
                                }
                            }
                        //}
                    }

                    qSort(refChunks.begin(), refChunks.end(), chunkLessThan);

                    for(int j = 0; j < refChunks.count() - 1; ) {
                        const Index::ReferenceIndex::Chunk& left = refChunks.at(j);
                        const Index::ReferenceIndex::Chunk& right = refChunks.at(j + 1);
                        if(left.getEnd() >= right.getStart()) {
                            // merge
                            Index::ReferenceIndex::Chunk chunk(left.getStart(), right.getEnd());
                            refChunks.replace(j, chunk);
                            refChunks.removeAt(j + 1);
                        } else {
                            j++;
                        }
                    }
                    chunks.insert(i, refChunks);
                }
            }
        }

        U2AttributeDbi * attributeDbi = sqliteDbi->getAttributeDbi();
        qint64 totalReadsImported = 0;
        time_t totalReadTime = 0;
        bool pack = true;
        for(int i=0; i < reader->getHeader().getReferences().count(); i++) {
            if(bamInfo.isReferenceSelected(i)) {
                
                const Header::Reference &reference = reader->getHeader().getReferences().at(i);
                U2Assembly assembly;
                assembly.visualName = reference.getName();
                U2OpStatusImpl opStatus;
                U2AssemblyReadsImportInfo importInfo;
                if(chunks.isEmpty()) {
                    sqliteDbi->getAssemblyDbi()->createAssemblyObject(assembly, "/", NULL, importInfo, opStatus);
                } else {                    
                    BAMDbiIterator iter(i, reader.get(), chunks[i], totalChunksLength, readLength, stateInfo);
                    sqliteDbi->getAssemblyDbi()->createAssemblyObject(assembly, "/", &iter, importInfo, opStatus);
                    totalReadsImported += iter.getImportedCount();
                    totalReadTime += iter.getReadTime();
                    pack = !importInfo.packed;
                    assert(bamInfo.hasIndex());
                }
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
                // set contig's properties to attributes
                if(attributeDbi != NULL) {
                    U2OpStatusImpl status;
                    {
                        U2IntegerAttribute lenAttr;
                        lenAttr.objectId = assembly.id;
                        lenAttr.name = "reference_length_attribute";
                        lenAttr.version = 1;
                        lenAttr.value = reference.getLength();
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
                        attributeDbi->createStringAttribute(uriAttr, status);
                        if(status.hasError()) {
                            throw Exception(status.getError());
                        }
                    }
                }
                assemblies.insert(i, assembly);
            }
        }

        // Import without index
        
        if(bamInfo.isUnmappedSelected()) {
            U2Assembly assembly;
            assembly.visualName = QString("Unmapped");
            {
                U2OpStatusImpl opStatus;
                U2AssemblyReadsImportInfo importInfo;
                sqliteDbi->getAssemblyDbi()->createAssemblyObject(assembly, "/", NULL, importInfo, opStatus);
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
            }
            assemblies.insert(-1, assembly);
        }
        
        if(!bamInfo.hasIndex() || bamInfo.isUnmappedSelected()) {
            QMap<int, QList<U2AssemblyRead> > reads;        
            foreach(int index, assemblies.keys()) {
                reads.insert(index, QList<U2AssemblyRead>());            
            }

            while(!reader->isEof()) {
                stateInfo.setDescription(BAMDbiPlugin::tr("Reading"));
                int readsCount = 0;
                int progressUpdateCounter = 0;
                while(!reader->isEof() && readsCount < READS_CHUNK_SIZE) {
                    Reader::AlignmentReader aReader = reader->getAlignmentReader();
                    if(bamInfo.isReferenceSelected(aReader.getId())) {
                        reads[aReader.getId()].append(AssemblyDbi::alignmentToRead(aReader.read()));
                        readsCount++;
                    } else {
                        aReader.skip();
                    }
                    if (++progressUpdateCounter > 1000) {
                        if (isCanceled()) {
                            throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
                        }
                        stateInfo.progress = ioAdapter->getProgress() * FIRST_STAGE_PERCENT / 100;
                        stateInfo.setDescription(readsCount == 0 ? BAMDbiPlugin::tr("Positioning...")  : BAMDbiPlugin::tr("Reading (%1 reads)").arg(QString::number(readsCount)));
                        progressUpdateCounter = 0;
                    }
                }
                totalReadsImported += readsCount;
                stateInfo.setDescription(BAMDbiPlugin::tr("Saving reads"));
                flushReads(sqliteDbi, assemblies, reads);
                if (isCanceled()) {
                    throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
                }
                stateInfo.progress = ioAdapter->getProgress()*FIRST_STAGE_PERCENT/100;
            }
        }        

        //Packing
        time_t packStart = time(0);
        if (pack) {
            stateInfo.setDescription(BAMDbiPlugin::tr("Packing reads"));
            int i = 0;
            foreach(int index, assemblies.keys()) {
                {
                    U2OpStatusImpl opStatus;
                    U2AssemblyPackStat stat;
                    sqliteDbi->getAssemblyDbi()->pack(assemblies[index].id, stat, opStatus);
                    if(opStatus.hasError()) {
                        throw Exception(opStatus.getError());
                    }
                    if(attributeDbi != NULL) {
                        {
                            U2IntegerAttribute maxProwAttr;
                            maxProwAttr.objectId = assemblies[index].id;
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
                            countReadsAttr.objectId = assemblies[index].id;
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
                if(isCanceled()) {
                    throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
                }
                stateInfo.progress = FIRST_STAGE_PERCENT + (++i)*SECOND_STAGE_PERCENT/assemblies.size();
            }
        }
        time_t packTime = time(0) - packStart;
        time_t totalTime = time(0) - startTime;
        
        bamLog.trace(QString("BAM %1: imported %2 reads, total time %3 s, read time %4 s, pack time %5").arg(sourceUrl.fileName()).arg(QString::number(totalReadsImported)).arg(QString::number(totalTime)).arg(QString::number(totalReadTime)).arg(QString::number(packTime)));

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
