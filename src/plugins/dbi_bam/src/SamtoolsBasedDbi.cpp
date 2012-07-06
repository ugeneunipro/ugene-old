/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "BAMDbiPlugin.h"
#include "BAMFormat.h"
#include "Exception.h"
#include "IOException.h"

#include "SamtoolsBasedDbi.h"

namespace U2 {
namespace BAM {

/************************************************************************/
/* SamtoolsBasedDbi */
/************************************************************************/
SamtoolsBasedDbi::SamtoolsBasedDbi()
: U2AbstractDbi(SamtoolsBasedDbiFactory::ID), assembliesCount(0), bamHandler(NULL), header(NULL), index(NULL)
{

}

SamtoolsBasedDbi::~SamtoolsBasedDbi() {
    this->cleanup();
}

QVariantMap SamtoolsBasedDbi::shutdown(U2OpStatus &os) {
    cleanup();
    return QVariantMap();
}

void SamtoolsBasedDbi::init(const QHash<QString, QString> &properties, const QVariantMap & /*persistentData*/, U2OpStatus &os) {
    try {
        if(U2DbiState_Void != state) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        state = U2DbiState_Starting;
        if(properties.value(U2_DBI_OPTION_URL).isEmpty()) {
            throw Exception(BAMDbiPlugin::tr("URL is not specified"));
        }
        url = GUrl(properties.value(U2_DBI_OPTION_URL));
        if(!url.isLocalFile()) {
            throw Exception(BAMDbiPlugin::tr("Non-local files are not supported"));
        }
        QByteArray urlBA = url.getURLString().toAscii();
        bamHandler = bam_open(urlBA.constData(), "r");
        if (NULL == bamHandler) {
            throw IOException(BAMDbiPlugin::tr("Can't open file '%1'").arg(url.getURLString()));
        }

        index = bam_index_load(urlBA.constData());
        if (NULL == index) {
            bam_index_build(urlBA.constData());
            index = bam_index_load(urlBA.constData());
        }
        if (NULL == index) {
            throw IOException(BAMDbiPlugin::tr("Can't load or build index file for '%1'").arg(url.getURLString()));
        }

        header = bam_header_read(bamHandler);
        if (NULL == header) {
            throw IOException(BAMDbiPlugin::tr("Can't read header from file '%1'").arg(url.getURLString()));
        }

        assembliesCount = header->n_targets;
        assemblyDbi.reset(new SamtoolsBasedAssemblyDbi(*this));
        createObjectDbi();

        initProperties = properties;
        features.insert(U2DbiFeature_ReadSequence);
        features.insert(U2DbiFeature_ReadAssembly);
        dbiId = url.getURLString();
        state = U2DbiState_Ready;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        this->cleanup();
    }
}

void SamtoolsBasedDbi::cleanup() {
    assemblyDbi.reset();
    objectDbi.reset();
    if (NULL != header) {
        bam_header_destroy(header);
        header = NULL;
    }
    if (NULL != index) {
        bam_index_destroy(index);
        index = NULL;
    }
    if (NULL != bamHandler) {
        bam_close(bamHandler);
        bamHandler = NULL;
    }
    state = U2DbiState_Void;
}

void SamtoolsBasedDbi::createObjectDbi() {
    QList<U2DataId> assemblyObjectIds;
    for (int i=0; i<header->n_targets; i++) {
        assemblyObjectIds << QByteArray::number(i);
    }
    objectDbi.reset(new SamtoolsBasedObjectDbi(*this, assemblyObjectIds));
}

U2DataType SamtoolsBasedDbi::getEntityTypeById(const U2DataId &id) const {
    CHECK(!id.isEmpty(), U2Type::Unknown);
    bool ok = true;
    int dbId = id.toInt(&ok);
    CHECK(ok, U2Type::Unknown);

    if(dbId <= assembliesCount) {
        return U2Type::Assembly;
    } else {
        return U2Type::Unknown;
    }
}

const bamFile SamtoolsBasedDbi::getBamFile() const {
    return bamHandler;
}

const bam_header_t *SamtoolsBasedDbi::getHeader() const {
    return header;
}

const bam_index_t *SamtoolsBasedDbi::getIndex() const {
    return index;
}

U2AssemblyDbi *SamtoolsBasedDbi::getAssemblyDbi() {
    if(U2DbiState_Ready == state) {
        return assemblyDbi.get();
    } else {
        return NULL;
    }
}

U2ObjectDbi *SamtoolsBasedDbi::getObjectDbi() {
    if(U2DbiState_Ready == state) {
        return objectDbi.get();
    } else {
        return NULL;
    }
}

/************************************************************************/
/* SamtoolsBasedObjectDbi */
/************************************************************************/
SamtoolsBasedObjectDbi::SamtoolsBasedObjectDbi(SamtoolsBasedDbi &dbi, const QList<U2DataId> &assemblyObjectIds)
: U2SimpleObjectDbi(&dbi), dbi(dbi), assemblyObjectIds(assemblyObjectIds)
{

}

qint64 SamtoolsBasedObjectDbi::countObjects(U2OpStatus &os) {
    return countObjects(U2Type::Assembly, os);
}

qint64 SamtoolsBasedObjectDbi::countObjects(U2DataType type, U2OpStatus &os) {
    CHECK_EXT(U2DbiState_Ready == dbi.getState(),
              os.setError(BAMDbiPlugin::tr("Invalid samtools DBI state")), 0);

    if(U2Type::Assembly == type) {
        return assemblyObjectIds.size();
    } else {
        return 0;
    }
}

QList<U2DataId> SamtoolsBasedObjectDbi::getObjects(qint64 offset, qint64 count, U2OpStatus &os) {
    return getObjects(U2Type::Assembly, offset, count, os);
}

QList<U2DataId> SamtoolsBasedObjectDbi::getObjects(U2DataType type, qint64 offset, qint64 count, U2OpStatus &os) {
    CHECK_EXT(U2DbiState_Ready == dbi.getState(),
        os.setError(BAMDbiPlugin::tr("Invalid samtools DBI state")), QList<U2DataId>());

    if(U2Type::Assembly == type) {
        QList<U2DataId> result;
        qint64 lastExc = offset + count;
        if (U2_DBI_NO_LIMIT == count) {
            lastExc = assemblyObjectIds.size();
        }
        for (int i=offset; i<lastExc; i++) {
            result << assemblyObjectIds[i];
        }
        return result;
    } else {
        return QList<U2DataId>();
    }
}

QList<U2DataId> SamtoolsBasedObjectDbi::getParents(const U2DataId& /*entityId*/, U2OpStatus &os) {
    CHECK_EXT(U2DbiState_Ready == dbi.getState(),
        os.setError(BAMDbiPlugin::tr("Invalid samtools DBI state")), QList<U2DataId>());
    return QList<U2DataId>();
}

QStringList SamtoolsBasedObjectDbi::getFolders(U2OpStatus &os) {
    CHECK_EXT(U2DbiState_Ready == dbi.getState(),
        os.setError(BAMDbiPlugin::tr("Invalid samtools DBI state")), QStringList());
    return QStringList("/");
}

qint64 SamtoolsBasedObjectDbi::countObjects(const QString &folder, U2OpStatus &os) {
    CHECK_EXT(U2DbiState_Ready == dbi.getState(),
        os.setError(BAMDbiPlugin::tr("Invalid samtools DBI state")), 0);

    CHECK_EXT("/" == folder,
        os.setError(BAMDbiPlugin::tr("No such folder: %1").arg(folder)), 0);

    return countObjects(os);
}

QList<U2DataId> SamtoolsBasedObjectDbi::getObjects(const QString &folder, qint64 offset, qint64 count, U2OpStatus &os) {
    CHECK_EXT(U2DbiState_Ready == dbi.getState(),
        os.setError(BAMDbiPlugin::tr("Invalid samtools DBI state")), QList<U2DataId>());

    CHECK_EXT("/" == folder,
        os.setError(BAMDbiPlugin::tr("No such folder: %1").arg(folder)), QList<U2DataId>());

    return getObjects(offset, count, os);
}

QStringList SamtoolsBasedObjectDbi::getObjectFolders(const U2DataId& objectId, U2OpStatus &os) {
    CHECK_EXT(U2DbiState_Ready == dbi.getState(),
        os.setError(BAMDbiPlugin::tr("Invalid samtools DBI state")), QStringList());

    if(U2Type::Assembly == dbi.getEntityTypeById(objectId)) {
        return QStringList("/");
    } else {
        return QStringList();
    }
}

qint64 SamtoolsBasedObjectDbi::getObjectVersion(const U2DataId& /*objectId*/, U2OpStatus &os) {
    CHECK_EXT(U2DbiState_Ready == dbi.getState(),
        os.setError(BAMDbiPlugin::tr("Invalid samtools DBI state")), 0);

    return 0;
}

qint64 SamtoolsBasedObjectDbi::getFolderLocalVersion(const QString &folder, U2OpStatus &os) {
    CHECK_EXT(U2DbiState_Ready == dbi.getState(),
        os.setError(BAMDbiPlugin::tr("Invalid samtools DBI state")), 0);

    CHECK_EXT("/" == folder,
        os.setError(BAMDbiPlugin::tr("No such folder: %1").arg(folder)), 0);

    return 0;
}

qint64 SamtoolsBasedObjectDbi::getFolderGlobalVersion(const QString &folder, U2OpStatus &os) {
    CHECK_EXT(U2DbiState_Ready == dbi.getState(),
        os.setError(BAMDbiPlugin::tr("Invalid samtools DBI state")), 0);

    CHECK_EXT("/" == folder,
        os.setError(BAMDbiPlugin::tr("No such folder: %1").arg(folder)), 0);

    return 0;
}

U2DbiIterator<U2DataId>* SamtoolsBasedObjectDbi::getObjectsByVisualName(const QString& , U2DataType , U2OpStatus& ) {
    // TODO:
    return NULL;
}

/************************************************************************/
/* SamtoolsBasedReadsIterator */
/************************************************************************/
const int SamtoolsBasedReadsIterator::BUFFERED_INTERVAL_SIZE = 1000;

SamtoolsBasedReadsIterator::SamtoolsBasedReadsIterator(int assemblyId, const U2Region &_r, SamtoolsBasedDbi &dbi)
: U2DbiIterator<U2AssemblyRead>(), assemblyId(assemblyId), dbi(dbi)
{
    bool errorRegion = false;
    qint64 startPos = _r.startPos;
    qint64 endPos = _r.endPos() - 1;

    // region must be between 0 and INT_MAX
    if (startPos < 0) {
        startPos = 0;
    } else if (startPos > INT_MAX) {
        startPos = INT_MAX;
        errorRegion = true;
    }
    if (endPos < 0) {
        endPos = 0;
        errorRegion = true;
    } else if (endPos > INT_MAX) {
        endPos = INT_MAX;
    }

    qint64 length = endPos - startPos + 1;
    r = U2Region(startPos, length);
    nextPosToRead = r.startPos;

    SAFE_POINT(!errorRegion, QString("Bad region for samtools reads fetching: %1 - %2").arg(_r.startPos).arg(_r.endPos()), );
}

bool SamtoolsBasedReadsIterator::hasNext() {
    bool fetch = false;
    if (reads.isEmpty()) {
        fetch = true;
    } else if (reads.end() == current) {
        fetch = true;
    }
    if (!fetch) {
        return true;
    }

    reads.clear();
    qint64 endPosExc = r.endPos();
    while (reads.isEmpty() && nextPosToRead < endPosExc) {
        fetchNextChunk();
    }

    if (!reads.isEmpty()) {
        return true;
    }
    return false;
}

U2AssemblyRead SamtoolsBasedReadsIterator::next() {
    if (this->hasNext()) {
        U2AssemblyRead res = *current;
        current++;
        return res;
    }
    return U2AssemblyRead();
}

U2AssemblyRead SamtoolsBasedReadsIterator::peek() {
    if (this->hasNext()) {
        U2AssemblyRead res = *current;
        return res;
    }
    return U2AssemblyRead();
}

static const int NAME_COL = 0;
static const int FLAGS_COL = 1;
static const int CIGAR_COL = 5;
static const int SEQ_COL = 9;
static const int QUAL_COL = 10;

int bamFetchFunction(const bam1_t *b, void *data) {
    SamtoolsBasedReadsIterator *it = (SamtoolsBasedReadsIterator*)data;
    QList<U2AssemblyRead> &reads = it->reads;
    SamtoolsBasedDbi &dbi = it->dbi;

    U2AssemblyRead read(new U2AssemblyReadData());
    {
        char *samStr = bam_format1(dbi.getHeader(), b);
        QByteArray samArr(samStr);
        QList<QByteArray> values = samArr.split('\t');

        read->name = values[NAME_COL];
        read->flags = values[FLAGS_COL].toLongLong();
        read->leftmostPos = b->core.pos;
        read->mappingQuality = b->core.qual;
        QString error;
        QList<U2CigarToken> tokens = U2AssemblyUtils::parseCigar(values[CIGAR_COL], error);
        if (error.isEmpty()) {
            read->cigar = tokens;
        }
        read->readSequence = values[SEQ_COL];
        read->quality = values[QUAL_COL];
        read->effectiveLen = bam_cigar2qlen(&(b->core), bam1_cigar(b));
        delete[] samStr;
        read->id = read->name
           + ";" + QByteArray::number(read->leftmostPos)
           + ";" + QByteArray::number(read->effectiveLen);
    }

    // add new border intersected reads
    qint64 endPos = read->leftmostPos + read->effectiveLen;
    if (endPos > (qint64)it->nextPosToRead) {
        it->newBorderReadIds << read->id;
    }

    if (!it->borderReadIds.contains(read->id)) {
        reads.append(read);
    }
    return 0;
}

void SamtoolsBasedReadsIterator::fetchNextChunk() {
    bamFile bam = dbi.getBamFile();
    const bam_index_t *idx = dbi.getIndex();
    SAFE_POINT_EXT(NULL != bam, nextPosToRead = INT_MAX, );
    SAFE_POINT_EXT(NULL != idx, nextPosToRead = INT_MAX, );

    void *data = (void*)(this);
    borderReadIds = newBorderReadIds;
    newBorderReadIds.clear();
    int startPos = (int)nextPosToRead;
    int endPos = (int)(nextPosToRead + BUFFERED_INTERVAL_SIZE);
    nextPosToRead += BUFFERED_INTERVAL_SIZE;
    bam_fetch(bam, idx, assemblyId, startPos, endPos, data, bamFetchFunction);

    current = reads.begin();
}

/************************************************************************/
/* SamtoolsBasedAssemblyDbi */
/************************************************************************/
SamtoolsBasedAssemblyDbi::SamtoolsBasedAssemblyDbi(SamtoolsBasedDbi &dbi)
: U2SimpleAssemblyDbi(&dbi), dbi(dbi)
{

}

U2Assembly SamtoolsBasedAssemblyDbi::getAssemblyObject(const U2DataId &id, U2OpStatus &os) {
    CHECK_EXT(U2DbiState_Ready == dbi.getState(),
        os.setError(BAMDbiPlugin::tr("Invalid samtools DBI state")), U2Assembly());

    const bam_header_t *header = dbi.getHeader();
    SAFE_POINT(NULL != header, "NULL BAM header", U2Assembly());

    CHECK_EXT(U2Type::Assembly == dbi.getEntityTypeById(id),
        os.setError(BAMDbiPlugin::tr("The specified object is not an assembly")), U2Assembly());

    int dbId = id.toInt();
    CHECK(dbId < header->n_targets, U2Assembly());

    U2Assembly result;
    result.id = id;
    result.dbiId = dbi.getDbiId();
    result.visualName = header->target_name[dbId];

    return result;
}

int bamCountFunction(const bam1_t *b, void *data) {
    qint64 *count = (qint64*)data;
    (*count)++;
    return 0;
}

qint64 SamtoolsBasedAssemblyDbi::countReads(const U2DataId &assemblyId, const U2Region &r, U2OpStatus &os) {
    bool ok = false;
    int id = assemblyId.toInt(&ok);
    CHECK_EXT(ok, os.setError("Assembly id error"), 0);

    qint64 result = 0;
    void *data = &result;
    U2Region targetReg = this->getCorrectRegion(assemblyId, r, os);
    CHECK_OP(os, 0);
    qint64 endPos = targetReg.endPos() - 1;
    bam_fetch(dbi.getBamFile(), dbi.getIndex(), id, (int)targetReg.startPos, (int)endPos, data, bamCountFunction);

    return result;
}

U2DbiIterator<U2AssemblyRead>* SamtoolsBasedAssemblyDbi::getReads(const U2DataId &assemblyId, const U2Region &r, U2OpStatus& os, bool /*sortedHint*/) {
    bool ok = false;
    int id = assemblyId.toInt(&ok);
    CHECK_EXT(ok, os.setError("Assembly id error"), 0);
    U2Region targetReg = this->getCorrectRegion(assemblyId, r, os);
    return new SamtoolsBasedReadsIterator(id, targetReg, dbi);
}

qint64 SamtoolsBasedAssemblyDbi::getMaxPackedRow(const U2DataId &, const U2Region &, U2OpStatus &os) {
    os.setError("Operation not supported: BAM::SamtoolsBasedAssemblyDbi::getMaxPackedRow");
    return 0;
}

U2DbiIterator<U2AssemblyRead>* SamtoolsBasedAssemblyDbi::getReadsByRow(const U2DataId &, const U2Region &, qint64, qint64, U2OpStatus &os) {
    os.setError("Operation not supported: BAM::SamtoolsBasedAssemblyDbi::getReadsByRow");
    return NULL;
}

U2DbiIterator<U2AssemblyRead>* SamtoolsBasedAssemblyDbi::getReadsByName(const U2DataId &, const QByteArray &, U2OpStatus &os) {
    os.setError("Operation not supported: BAM::SamtoolsBasedAssemblyDbi::getReadsByName");
    return NULL;
}

qint64 SamtoolsBasedAssemblyDbi::getMaxEndPos(const U2DataId &assemblyId, U2OpStatus &os) {
    bool ok = false;
    int id = assemblyId.toInt(&ok);
    CHECK_EXT(ok, os.setError("Assembly id error"), 0);

    const bam_header_t *header = dbi.getHeader();
    CHECK_EXT(NULL != header, os.setError("NULL header"), 0);
    CHECK_EXT(id < header->n_targets, os.setError("Unknown assembly id"), 0);

    return header->target_len[id] - 1;
}

U2Region SamtoolsBasedAssemblyDbi::getCorrectRegion(const U2DataId &assemblyId, const U2Region &r, U2OpStatus &os) {
    qint64 assemblyLength = getMaxEndPos(assemblyId, os) + 1;
    CHECK_OP(os, U2Region());
    qint64 startPos = r.startPos;
    qint64 endPos = r.endPos() - 1;

    U2Region outOfRangeRegion(assemblyLength + 1, 0);

    if (startPos < 0) {
        startPos = 0;
    } else if (startPos >= assemblyLength) {
        return outOfRangeRegion;
    }
    if (endPos < 0) {
        return outOfRangeRegion;
    } else if (endPos >= assemblyLength) {
        endPos = assemblyLength - 1;
    }

    qint64 length = endPos - startPos + 1;
    CHECK(length >= 0, outOfRangeRegion);

    U2Region result(startPos, length);
    return result;
}

/************************************************************************/
/* SamtoolsBasedDbiFactory */
/************************************************************************/
const QString SamtoolsBasedDbiFactory::ID = BAM_DBI_ID;

SamtoolsBasedDbiFactory::SamtoolsBasedDbiFactory()
: U2DbiFactory()
{

}

U2Dbi *SamtoolsBasedDbiFactory::createDbi() {
    return new SamtoolsBasedDbi();
}

U2DbiFactoryId SamtoolsBasedDbiFactory::getId()const {
    return ID;
}

FormatCheckResult SamtoolsBasedDbiFactory::isValidDbi(const QHash<QString, QString> &properties, const QByteArray &rawData, U2OpStatus & /*os*/) const {
    BAMFormat f;
    FormatCheckResult res = f.checkRawData(rawData, properties.value(U2_DBI_OPTION_URL));
    return res;
}


bool SamtoolsBasedDbiFactory::isDbiExists(const U2DbiId& id) const {
    return QFile::exists(id);
}

} // BAM
} // U2
