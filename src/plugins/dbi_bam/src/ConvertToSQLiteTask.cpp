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

#include <memory>

#define READS_CHUNK_SIZE (250*1000)
namespace U2 {
namespace BAM {

ConvertToSQLiteTask::ConvertToSQLiteTask(const GUrl &_sourceUrl, const GUrl &_destinationUrl, BAMInfo& _bamInfo):
    Task(tr("Convert BAM to UGENE database (%1)").arg(_destinationUrl.fileName()), TaskFlag_None),
    sourceUrl(_sourceUrl),
    destinationUrl(_destinationUrl),
    bamInfo(_bamInfo)
{
    tpm = Progress_Manual;
}

static void flushReads(U2Dbi* sqliteDbi, QMap<int, U2Assembly>& assemblies, QMap<int, QList<U2AssemblyRead> >& reads) {
    foreach(int index, assemblies.keys()) {
        if(!reads[index].isEmpty()) {
            U2OpStatusImpl opStatus;
            sqliteDbi->getAssemblyDbi()->addReads(assemblies[index].id, reads[index], opStatus);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
    }
    reads.clear();
}

static bool chunkLessThan(const Index::ReferenceIndex::Chunk &c1, const Index::ReferenceIndex::Chunk &c2) {
    return c1.getStart() < c2.getStart();
}

void ConvertToSQLiteTask::run() {
    try {
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

        U2AttributeDbi * attributeDbi = sqliteDbi->getAttributeDbi();
        QMap<int, U2Assembly> assemblies;
        for(int i=0; i < reader->getHeader().getReferences().count(); i++) {
            if(bamInfo.isReferenceSelected(i)) {
                const Header::Reference &reference = reader->getHeader().getReferences().at(i);
                U2Assembly assembly;
                assembly.visualName = reference.getName();
                {
                    U2OpStatusImpl opStatus;
                    sqliteDbi->getAssemblyDbi()->createAssemblyObject(assembly, "/", NULL, opStatus);
                    if(opStatus.hasError()) {
                        throw Exception(opStatus.getError());
                    }
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
        if(bamInfo.isUnmappedSelected()) {
            U2Assembly assembly;
            assembly.visualName = QString("Unmapped");
            {
                U2OpStatusImpl opStatus;
                sqliteDbi->getAssemblyDbi()->createAssemblyObject(assembly, "/", NULL, opStatus);
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
            }
            assemblies.insert(-1, assembly);
        }
        static const int FIRST_STAGE_PERCENT = 60;
        static const int SECOND_STAGE_PERCENT = 40;
        QMap<int, QList<U2AssemblyRead> > reads;
        foreach(int index, assemblies.keys()) {
            reads.insert(index, QList<U2AssemblyRead>());            
        }

        QStringList progressIndicators = QStringList() << ".  " << ".. " << "...";
        if(bamInfo.hasIndex() && !bamInfo.isUnmappedSelected()) {

            stateInfo.setStateDesc(BAMDbiPlugin::tr("Preparing BAM index"));

            const QList<Index::ReferenceIndex> &refIndices = bamInfo.getIndex().getReferenceIndices();
            QList<Index::ReferenceIndex::Chunk> sortedChunks;
            for(int i=0, i_c = refIndices.count(); i < i_c; i++) {
                if(bamInfo.isReferenceSelected(i)) {
                    const QList<Index::ReferenceIndex::Bin>& bins = refIndices.at(i).getBins();
                    if(bins.isEmpty()) {
                        continue;
                    }
                    unsigned int lastBin = bins.last().getBin();
                    unsigned int minBin = lastBin > 0 ? (lastBin > 8 ? (lastBin > 72 ? (lastBin > 584 ? (lastBin > 4680 ? 4681 : 585) : 73) : 9) : 1) : 0;
                    foreach(const Index::ReferenceIndex::Bin bin, bins) {
                        if(bin.getBin() >= minBin) {
                            foreach(const Index::ReferenceIndex::Chunk& chunk, bin.getChunks()) {
                                if(chunk.getStart() < chunk.getEnd()) {
                                    sortedChunks.append(chunk);
                                } else {
                                    coreLog.error(BAMDbiPlugin::tr("Skip invalid chunk: bin %1, chunk begin %2, chunk end %3").arg(bin.getBin())
                                        .arg(chunk.getStart().getPackedOffset())
                                        .arg(chunk.getEnd().getPackedOffset()));
                                }
                            }
                            
                        }
                    }
                }
            }

            qSort(sortedChunks.begin(), sortedChunks.end(), chunkLessThan);

            for(int i = 0; i < sortedChunks.count() - 1; ) {
                const Index::ReferenceIndex::Chunk& left = sortedChunks.at(i);
                const Index::ReferenceIndex::Chunk& right = sortedChunks.at(i + 1);
                if(left.getEnd() >= right.getStart()) {
                    // merge
                    Index::ReferenceIndex::Chunk chunk(left.getStart(), right.getEnd());
                    sortedChunks.replace(i, chunk);
                    sortedChunks.removeAt(i + 1);
                } else {
                    i++;
                }
            }

            qint64 totalChunksLength = 0;
            foreach(const Index::ReferenceIndex::Chunk& chunk, sortedChunks) {
                totalChunksLength += chunk.getEnd().getCoffset() - chunk.getStart().getCoffset();
            }

#ifdef _DEBUG
            for(int i = 0; i < sortedChunks.count() - 1; i++) {
                const Index::ReferenceIndex::Chunk& left = sortedChunks.at(i);
                const Index::ReferenceIndex::Chunk& right = sortedChunks.at(i + 1);
                assert(left.getStart() < left.getEnd());
                assert(right.getStart() < right.getEnd());
                assert(left.getEnd() < right.getStart());
            }
#endif
            int readsCount = 0;
            int progressUpdateCounter = 0;
            int progressUpdates = 0;
            qint64 readLength = 0;            
            stateInfo.setStateDesc(BAMDbiPlugin::tr("Reading"));
            foreach(const Index::ReferenceIndex::Chunk& chunk, sortedChunks) {
                reader->seek(chunk.getStart());
                qint64 chunkLen = chunk.getEnd().getCoffset() - chunk.getStart().getCoffset();
                while(!reader->isEof() && reader->getOffset() < chunk.getEnd()) {
                    Alignment alignment = reader->readAlignment();
                    if(bamInfo.isReferenceSelected(alignment.getReferenceId())) {
                        reads[alignment.getReferenceId()].append(AssemblyDbi::alignmentToRead(alignment));
                        if(++readsCount >= READS_CHUNK_SIZE) {
                            readsCount = 0;
                            stateInfo.setStateDesc(BAMDbiPlugin::tr("Saving reads"));
                            flushReads(sqliteDbi, assemblies, reads);
                            stateInfo.setStateDesc(BAMDbiPlugin::tr("Reading"));
                        }
                    }
                    if(++progressUpdateCounter > 1000) {                        
                        if(isCanceled()) {
                            throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
                        }
                        stateInfo.progress = FIRST_STAGE_PERCENT * (readLength + reader->getOffset().getCoffset() - chunk.getStart().getCoffset()) / totalChunksLength;
                        stateInfo.setStateDesc(BAMDbiPlugin::tr("Reading %1").arg(progressIndicators.at(progressUpdates++ % 3)));
                        progressUpdateCounter = 0;
                    }
                }
                readLength += chunkLen;
            }
            stateInfo.setStateDesc(BAMDbiPlugin::tr("Saving reads"));
            flushReads(sqliteDbi, assemblies, reads);
            stateInfo.progress = FIRST_STAGE_PERCENT;
        } else {
            while(!reader->isEof()) {
                stateInfo.setStateDesc(BAMDbiPlugin::tr("Reading"));
                int readsCount = 0;
                int progressUpdateCounter = 0;
                int progressUpdates = 0;
                while(!reader->isEof() && readsCount < READS_CHUNK_SIZE) {
                    Alignment alignment = reader->readAlignment();
                    if(bamInfo.isReferenceSelected(alignment.getReferenceId())) {
                        reads[alignment.getReferenceId()].append(AssemblyDbi::alignmentToRead(alignment));
                        readsCount++;
                    }
                    if (++progressUpdateCounter > 1000) {
                        if (isCanceled()) {
                            throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
                        }
                        stateInfo.progress = ioAdapter->getProgress() * FIRST_STAGE_PERCENT / 100;
                        stateInfo.setStateDesc(BAMDbiPlugin::tr("Reading %1").arg(progressIndicators.at(progressUpdates++ % 3)));
                        progressUpdateCounter = 0;
                    }
                }
                stateInfo.setStateDesc(BAMDbiPlugin::tr("Saving reads"));
                flushReads(sqliteDbi, assemblies, reads);
                if (isCanceled()) {
                    throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
                }
                stateInfo.progress = ioAdapter->getProgress()*FIRST_STAGE_PERCENT/100;
            }
        }

        stateInfo.setStateDesc(BAMDbiPlugin::tr("Packing reads"));

        {
            int i = 0;
            foreach(int index, assemblies.keys()) {
                {
                    U2OpStatusImpl opStatus;
                    U2AssemblyPackStat stat;
                    sqliteDbi->getAssemblyDbi()->pack(assemblies[index].id, stat, opStatus);
                    if(opStatus.hasError()) {
                        throw Exception(opStatus.getError());
                    }
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
                if(isCanceled()) {
                    throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
                }
                stateInfo.progress = FIRST_STAGE_PERCENT + (++i)*SECOND_STAGE_PERCENT/assemblies.size();
            }
        }
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
