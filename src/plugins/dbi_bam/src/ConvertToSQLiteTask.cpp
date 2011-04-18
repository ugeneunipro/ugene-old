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

#include <memory>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/AppContext.h>
#include "IOException.h"
#include "Reader.h"
#include "Index.h"
#include "Dbi.h"
#include "BAMDbiPlugin.h"
#include "ConvertToSQLiteTask.h"
#include "LoadBamInfoTask.h"

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
    foreach(int index, assemblies.keys()) {
        reads.insert(index, QList<U2AssemblyRead>());            
    }
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
        std::auto_ptr<U2Dbi> sqliteDbi(AppContext::getDbiRegistry()->getDbiFactoryById("SQLiteDbi")->createDbi());
        {
            QHash<QString, QString> properties;
            properties["url"] = destinationUrl.getURLString();
            properties["create"] = "1";
            U2OpStatusImpl opStatus;
            sqliteDbi->init(properties, QVariantMap(), opStatus);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        {
            if(!append) {
                U2OpStatusImpl opStatus;
                sqliteDbi->getObjectDbi()->createFolder("/", opStatus);
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
            }
        }
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
                assemblies.insert(i, assembly);
            }
        }
        static const int FIRST_STAGE_PERCENT = 60;
        static const int SECOND_STAGE_PERCENT = 40;
        QMap<int, QList<U2AssemblyRead> > reads;
        foreach(int index, assemblies.keys()) {
            reads.insert(index, QList<U2AssemblyRead>());            
        }

        if(bamInfo.hasIndex()) {

            stateInfo.setStateDesc(BAMDbiPlugin::tr("Preparing BAM index"));

            const QList<Index::ReferenceIndex> &refIndices = bamInfo.getIndex().getReferenceIndices();
            QList<Index::ReferenceIndex::Chunk> sortedChunks;
            for(int i=0, i_c = refIndices.count(); i < i_c; i++) {
                if(bamInfo.isReferenceSelected(i)) {
                    const QList<Index::ReferenceIndex::Bin>& bins = refIndices.at(i).getBins();
                    unsigned int lastBin = bins.last().getBin();
                    unsigned int minBin = lastBin > 0 ? (lastBin > 8 ? (lastBin > 72 ? (lastBin > 584 ? (lastBin > 4680 ? 4681 : 585) : 73) : 9) : 1) : 0;
                    foreach(const Index::ReferenceIndex::Bin bin, bins) {
                        if(bin.getBin() >= minBin) {
                            sortedChunks.append(bin.getChunks());
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
            int totalReads = 0;
            qint64 readLength = 0;

            stateInfo.setStateDesc(BAMDbiPlugin::tr("Reading"));

            foreach(const Index::ReferenceIndex::Chunk& chunk, sortedChunks) {
                reader->seek(chunk.getStart());
                qint64 chunkLen = chunk.getEnd().getCoffset() - chunk.getStart().getCoffset();
                while(!reader->isEof() && reader->getOffset() < chunk.getEnd()) {
                    Alignment alignment = reader->readAlignment();
                    if(-1 != alignment.getReferenceId() && bamInfo.isReferenceSelected(alignment.getReferenceId())) {
                        reads[alignment.getReferenceId()].append(AssemblyDbi::alignmentToRead(alignment));
                        if(++readsCount >= 16384) {
                            readsCount = 0;
                            flushReads(sqliteDbi.get(), assemblies, reads);
                        }
                    }
                    if(++totalReads > 1000) {
                        
                        if(isCanceled()) {
                            throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
                        }
                        stateInfo.progress = FIRST_STAGE_PERCENT * (readLength + reader->getOffset().getCoffset() - chunk.getStart().getCoffset()) / totalChunksLength;
                        totalReads = 0;
                    }
                }
                readLength += chunkLen;
            }
            
            flushReads(sqliteDbi.get(), assemblies, reads);
            stateInfo.progress = FIRST_STAGE_PERCENT;
        } else {
            stateInfo.setStateDesc(BAMDbiPlugin::tr("Reading"));
            while(!reader->isEof()) {
                {
                    int readsCount = 0;
                    int totalReads = 0;
                    while(!reader->isEof()) {
                        Alignment alignment = reader->readAlignment();
                        if(-1 != alignment.getReferenceId() && bamInfo.isReferenceSelected(alignment.getReferenceId())) {
                            reads[alignment.getReferenceId()].append(AssemblyDbi::alignmentToRead(alignment));
                            readsCount++;
                        }
                        if(++totalReads > 1000) {
                            if(isCanceled()) {
                                throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
                            }
                            stateInfo.progress = ioAdapter->getProgress() * FIRST_STAGE_PERCENT / 100;
                            totalReads = 0;
                        }
                        if(readsCount >= 16384) {
                            break;
                        }
                    }
                }
                flushReads(sqliteDbi.get(), assemblies, reads);
                if(isCanceled()) {
                    throw Exception(BAMDbiPlugin::tr("Task was cancelled"));
                }
                stateInfo.progress = ioAdapter->getProgress()*FIRST_STAGE_PERCENT/100;
            }
        }

        stateInfo.setStateDesc(BAMDbiPlugin::tr("Packing assemblies"));

        {
            int i = 0;
            foreach(int index, assemblies.keys()) {
                {
                    U2OpStatusImpl opStatus;
                    sqliteDbi->getAssemblyDbi()->pack(assemblies[index].id, opStatus);
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
        {
            U2OpStatusImpl opStatus;
            sqliteDbi->shutdown(opStatus);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
            stateInfo.setStateDesc(BAMDbiPlugin::tr("Done. Releasing resources"));
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
