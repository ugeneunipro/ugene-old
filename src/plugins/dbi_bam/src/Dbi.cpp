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
#include "BAMDbiPlugin.h"
#include "Dbi.h"
#include "BAMFormat.h"

#include <U2Core/AppContext.h>
#include <U2Core/U2OpStatusUtils.h>

#include <sqlite3.h>

namespace U2 {
namespace BAM {

// Dbi

Dbi::Dbi():
    assembliesCount(0)
{
}

void Dbi::init(const QHash<QString, QString> &properties, const QVariantMap & /*persistentData*/, U2OpStatus &os) {
    try {
        if(U2DbiState_Void != state) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        state = U2DbiState_Starting;
        if(properties.value("url").isEmpty()) {
            throw Exception(BAMDbiPlugin::tr("URL is not specified"));
        }
        url = GUrl(properties.value(U2_DBI_OPTION_URL));
        if(!url.isLocalFile()) {
            throw Exception(BAMDbiPlugin::tr("Non-local files are not supported")); // FIXME:
        }
        {
            IOAdapterFactory *factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(url));
            ioAdapter.reset(factory->createIOAdapter());
        }
        if(!ioAdapter->open(url, IOAdapterMode_Read)) {
            throw IOException(BAMDbiPlugin::tr("Can't open file '%1'").arg(url.getURLString()));
        }
        reader.reset(new Reader(*ioAdapter));
        sqliteUrl = GUrl(url.getURLString() + ".sqlite"); // FIXME: store .sqlite files in the user's home directory
        if(SQLITE_OK != sqlite3_open(sqliteUrl.getURLString().toUtf8().constData(), &dbRef.handle)) {
            throw IOException(BAMDbiPlugin::tr("Can't open index database"));
        }
        dbRef.useTransaction = true;
        assembliesCount = reader->getHeader().getReferences().size();
        objectRDbi.reset(new ObjectRDbi(*this, dbRef, assembliesCount));
        assemblyRDbi.reset(new AssemblyRDbi(*this, *reader, dbRef, assembliesCount));
        buildIndex(os);
        state = U2DbiState_Ready;
        initProps = properties;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        assemblyRDbi.reset();
        objectRDbi.reset();
        reader.reset();
        ioAdapter.reset();
        if(NULL != dbRef.handle) {
            sqlite3_close(dbRef.handle);
            dbRef.handle = NULL;
        }
        assert(sqliteUrl.isLocalFile());
        QFile::remove(sqliteUrl.getURLString());
        state = U2DbiState_Void;
    }
}

QVariantMap Dbi::shutdown(U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != state) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        state = U2DbiState_Stopping;
        assemblyRDbi.reset();
        objectRDbi.reset();
        reader.reset();
        ioAdapter.reset();
        if(NULL != dbRef.handle) {
            sqlite3_close(dbRef.handle);
            dbRef.handle = NULL;
        }
        state = U2DbiState_Void;
        return QVariantMap();
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return QVariantMap();
    }
}

QString Dbi::getDbiId() const {
    return url.getURLString();
}

QHash<QString, QString> Dbi::getDbiMetaInfo(U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != state) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        QHash<QString, QString> result;
        result["url"] = url.getURLString();
        return result;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return QHash<QString, QString>();
    }
}

U2DataType Dbi::getEntityTypeById(U2DataId id) const {
    if(0 == id) {
        return U2Type::Unknown;
    } else if((quint64)id <= (quint64)assembliesCount) {
        return U2Type::Assembly;
    } else {
        return U2Type::AssemblyRead;
    }
}

U2ObjectRDbi *Dbi::getObjectRDbi() {
    if(U2DbiState_Ready == state) {
        return objectRDbi.get();
    } else {
        return NULL;
    }
}

U2AssemblyRDbi *Dbi::getAssemblyRDbi() {
    if(U2DbiState_Ready == state) {
        return assemblyRDbi.get();
    } else {
        return NULL;
    }
}

void Dbi::buildIndex(U2OpStatus &os) {
    {
        U2OpStatusImpl opStatus;
        int tableCount = SQLiteQuery("SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND (name = 'assemblies' OR name = 'assemblyReads');", &dbRef, opStatus).selectInt64();
        if (opStatus.hasError()) {
            throw Exception(opStatus.getError());
        }
        if(2 == tableCount) {
            return;
        }
    }
    {
        U2OpStatusImpl opStatus;
        SQLiteQuery("DROP TABLE IF EXISTS assemblyReads;", &dbRef, opStatus).execute();
        if(opStatus.hasError()) {
            throw Exception(opStatus.getError());
        }
    }
    {
        U2OpStatusImpl opStatus;
        SQLiteQuery("DROP TABLE IF EXISTS assemblies;", &dbRef, opStatus).execute();
        if(opStatus.hasError()) {
            throw Exception(opStatus.getError());
        }
    }
    {
        U2OpStatusImpl transactionOpStatus;
        SQLiteTransaction transaction(&dbRef, transactionOpStatus);
        if (transactionOpStatus.hasError()) {
            throw Exception(transactionOpStatus.getError());
        }
        Q_UNUSED(transaction);

        try {
            {
                U2OpStatusImpl opStatus;
                SQLiteQuery("CREATE TABLE assemblies (id INTEGER PRIMARY KEY, maxEndPos INTEGER);",&dbRef, opStatus).execute();
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
            }
            {
                U2OpStatusImpl opStatus;
                SQLiteQuery("CREATE TABLE assemblyReads (id INTEGER PRIMARY KEY, assemblyId INTEGER REFERENCES assemblies, startPosition INTEGER, endPosition INTEGER, packedRow INTEGER);", &dbRef, opStatus).execute();
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
            }
            const Header &header = reader->getHeader();
            QList<qint64> maxEndPositions;
            QList<QList<qint64> > rowEnds;
            QList<qint64> columnStarts;
            QList<qint64> packedRows;
            for(int referenceId = 0;referenceId < header.getReferences().size();referenceId++) {
                {
                    U2OpStatusImpl opStatus;
                    SQLiteQuery insertAssemblyQ("INSERT INTO assemblies(id) VALUES (?1);", &dbRef, opStatus);
                    insertAssemblyQ.bindInt64(1, referenceId + 1);
                    insertAssemblyQ.execute();
                    if(opStatus.hasError()) {
                        throw Exception(opStatus.getError());
                    }
                }
                maxEndPositions.append(0);
                rowEnds.append(QList<qint64>());
                columnStarts.append(0);
                packedRows.append(0);
            }
            {
                U2OpStatusImpl insertReadOpStatus;
                SQLiteQuery insertReadQ("INSERT INTO assemblyReads(id, assemblyId, startPosition, endPosition, packedRow) VALUES (?1, ?2, ?3, ?4, ?5);", &dbRef, insertReadOpStatus);
                while(!reader->isEof()) {
                    VirtualOffset alignmentOffset = reader->getOffset();
                    Alignment alignment = reader->readAlignment();
                    if(-1 != alignment.getReferenceId()) {
                        qint64 endPosition = alignment.getPosition() + Alignment::computeLength(alignment);
                        if(alignment.getPosition() - columnStarts[alignment.getReferenceId()] > COLUMN_DISTANCE) {
                            packedRows[alignment.getReferenceId()] = 0;
                            columnStarts[alignment.getReferenceId()] = alignment.getPosition();
                        }
                        while(packedRows[alignment.getReferenceId()] < rowEnds[alignment.getReferenceId()].size() &&
                            rowEnds[alignment.getReferenceId()][packedRows[alignment.getReferenceId()]] >= alignment.getPosition()) {
                                packedRows[alignment.getReferenceId()]++;
                        }
                        if(rowEnds[alignment.getReferenceId()].size() == packedRows[alignment.getReferenceId()]) {
                            rowEnds[alignment.getReferenceId()].append(0);
                        }
                        rowEnds[alignment.getReferenceId()][packedRows[alignment.getReferenceId()]] = endPosition;

                        insertReadQ.reset();
                        insertReadQ.bindInt64(1, alignmentOffset.getPackedOffset());
                        insertReadQ.bindInt64(2, alignment.getReferenceId() + 1);
                        insertReadQ.bindInt64(3, alignment.getPosition());
                        insertReadQ.bindInt64(4, endPosition);
                        insertReadQ.bindInt64(5, packedRows[alignment.getReferenceId()]);
                        insertReadQ.execute();
                        if(insertReadOpStatus.hasError()) {
                            throw Exception(insertReadOpStatus.getError());
                        }
                        maxEndPositions[alignment.getReferenceId()] = qMax(maxEndPositions[alignment.getReferenceId()], endPosition);
                    }
                    if(os.isCanceled()) {
                        throw Exception(BAMDbiPlugin::tr("Operation was cancelled"));
                    }
                    os.setProgress(ioAdapter->getProgress());
                }
            }
            for(int referenceId = 0;referenceId < header.getReferences().size();referenceId++) {
                U2OpStatusImpl opStatus;
                SQLiteQuery updateEndQ("UPDATE assemblies SET maxEndPos = ?1 WHERE id = ?2;", &dbRef, opStatus);
                updateEndQ.reset();
                updateEndQ.bindInt64(1, maxEndPositions[referenceId]);
                updateEndQ.bindInt64(2, referenceId + 1);
                updateEndQ.execute();
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
            }
        } catch(const Exception &e) {
            transactionOpStatus.setError(e.getMessage());
            throw;
        }
    }
}

// DbiFactory

DbiFactory::DbiFactory():
    U2DbiFactory()
{
}

U2Dbi *DbiFactory::createDbi() {
    return new Dbi();
}

U2DbiFactoryId DbiFactory::getId()const {
    return ID;
}

const QString DbiFactory::ID = "BAMDbi";

bool DbiFactory::isValidDbi(const QHash<QString, QString>& properties, const QByteArray& rawData, U2OpStatus& os) const {
    BAMFormat f;
    return f.checkRawData(rawData, properties.value(U2_DBI_OPTION_URL));
}

// ObjectRDbi

ObjectRDbi::ObjectRDbi(Dbi &dbi, DbRef &dbRef, int assembliesCount):
    U2ObjectRDbi(&dbi),
    dbi(dbi),
    dbRef(dbRef),
    assembliesCount(assembliesCount)
{
}

qint64 ObjectRDbi::countObjects(U2OpStatus &os) {
    return countObjects(U2Type::Assembly, os);
}

qint64 ObjectRDbi::countObjects(U2DataType type, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if(U2Type::Assembly == type) {
            return assembliesCount;
        } else {
            return 0;
        }
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return 0;
    }
}

QList<U2DataId> ObjectRDbi::getObjects(qint64 offset, qint64 count, U2OpStatus &os) {
    return getObjects(U2Type::Assembly, offset, count, os);
}

QList<U2DataId> ObjectRDbi::getObjects(U2DataType type, qint64 offset, qint64 count, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if(U2Type::Assembly == type) {
            U2OpStatusImpl opStatus;
            SQLiteQuery q("SELECT id FROM assemblies;", offset, count, &dbRef, opStatus);
            QList<U2DataId> result = q.selectDataIds(0);
            return result;
        } else {
            return QList<U2DataId>();
        }
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return QList<U2DataId>();
    }
}

QList<U2DataId> ObjectRDbi::getParents(U2DataId /*entityId*/, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        return QList<U2DataId>();
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return QList<U2DataId>();
    }
}

QStringList ObjectRDbi::getFolders(U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        return QStringList("/");
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return QStringList();
    }
}

qint64 ObjectRDbi::countObjects(const QString &folder, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if("/" != folder) {
            throw Exception(BAMDbiPlugin::tr("No such folder: %1").arg(folder));
        }
        return countObjects(os);
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return 0;
    }
}

QList<U2DataId> ObjectRDbi::getObjects(const QString &folder, qint64 offset, qint64 count, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if("/" != folder) {
            throw Exception(BAMDbiPlugin::tr("No such folder: %1").arg(folder));
        }
        return getObjects(offset, count, os);
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return QList<U2DataId>();
    }
}

QStringList ObjectRDbi::getObjectFolders(U2DataId objectId, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if(U2Type::Assembly == dbi.getEntityTypeById(objectId)) {
            return QStringList("/");
        } else {
            return QStringList();
        }
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return QStringList();
    }
}

qint64 ObjectRDbi::getObjectVersion(U2DataId /*objectId*/, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        return 0;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return 0;
    }
}

qint64 ObjectRDbi::getFolderLocalVersion(const QString &folder, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if("/" != folder) {
            throw Exception(BAMDbiPlugin::tr("No such folder: %1").arg(folder));
        }
        return 0;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return 0;
    }
}

qint64 ObjectRDbi::getFolderGlobalVersion(const QString &folder, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if("/" != folder) {
            throw Exception(BAMDbiPlugin::tr("No such folder: %1").arg(folder));
        }
        return 0;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return 0;
    }
}

// AssemblyRDbi

AssemblyRDbi::AssemblyRDbi(Dbi &dbi, Reader &reader, DbRef &dbRef, int assembliesCount):
    U2AssemblyRDbi(&dbi),
    dbi(dbi),
    reader(reader),
    dbRef(dbRef),
    assembliesCount(assembliesCount)
{
}

U2Assembly AssemblyRDbi::getAssemblyObject(U2DataId id, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if(dbi.getEntityTypeById(id) != U2Type::Assembly) {
            throw Exception(BAMDbiPlugin::tr("The specified object is not an assembly"));
        }
        U2Assembly result;
        result.id = id;
        result.dbiId = dbi.getDbiId();
        return result;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return U2Assembly();
    }
}

qint64 AssemblyRDbi::countReadsAt(U2DataId assemblyId, const U2Region &r, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if(dbi.getEntityTypeById(assemblyId) != U2Type::Assembly) {
            throw Exception(BAMDbiPlugin::tr("The specified object is not an assembly"));
        }
        qint64 result;
        {
            U2OpStatusImpl opStatus;
            SQLiteQuery q("SELECT COUNT(*) FROM assemblyReads WHERE assemblyId = ?1 AND startPosition < ?2 AND endPosition > ?3;", &dbRef, opStatus);
            q.bindInt64(1, assemblyId);
            q.bindInt64(2, r.endPos());
            q.bindInt64(3, r.startPos);
            result = q.selectInt64();
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        return result;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return 0;
    }
}

QList<U2DataId> AssemblyRDbi::getReadIdsAt(U2DataId assemblyId, const U2Region &r, qint64 offset, qint64 count, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if(dbi.getEntityTypeById(assemblyId) != U2Type::Assembly) {
            throw Exception(BAMDbiPlugin::tr("The specified object is not an assembly"));
        }
        QList<U2DataId> result;
        {
            U2OpStatusImpl opStatus;
            SQLiteQuery q("SELECT id FROM assemblyReads WHERE assemblyId = ?1 AND startPosition < ?2 AND endPosition > ?3", offset, count, &dbRef, opStatus);
            q.bindDataId(1, assemblyId);
            q.bindInt64(2, r.startPos);
            q.bindInt64(3, r.endPos());
            result = q.selectDataIds(0);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        return result;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return QList<U2DataId>();
    }
}

QList<U2AssemblyRead> AssemblyRDbi::getReadsAt(U2DataId assemblyId, const U2Region &r, qint64 offset, qint64 count, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if(dbi.getEntityTypeById(assemblyId) != U2Type::Assembly) {
            throw Exception(BAMDbiPlugin::tr("The specified object is not an assembly"));
        }
        QList<U2DataId> rowIds;
        QList<qint64> packedRows;
        {
            U2OpStatusImpl opStatus;
            SQLiteQuery q("SELECT id, packedRow FROM assemblyReads WHERE assemblyId = ?1 AND startPosition < ?2 AND endPosition > ?3", offset, count, &dbRef, opStatus);
            q.bindInt64(1, assemblyId);
            q.bindInt64(2, r.endPos());
            q.bindInt64(3, r.startPos);

            while(q.step()) {
                rowIds.append(q.getInt64(0));
                packedRows.append(q.getInt64(1));
            }
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        QList<U2AssemblyRead> result;
        {
            U2OpStatusImpl opStatus;
            result = getReadsByIds(rowIds, packedRows, opStatus);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        return result;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return QList<U2AssemblyRead>();
    }
}

U2AssemblyRead AssemblyRDbi::getReadById(U2DataId rowId, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if(dbi.getEntityTypeById(rowId) != U2Type::AssemblyRead) {
            throw Exception(BAMDbiPlugin::tr("The specified object is not an assembly read"));
        }
        qint64 packedRow = 0;
        {
            U2OpStatusImpl opStatus;
            SQLiteQuery q("SELECT packedRow FROM assemblyReads WHERE id = ?1;", &dbRef, opStatus);
            q.bindInt64(1, rowId);
            packedRow = q.getInt64(0);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        U2AssemblyRead result;
        {
            U2OpStatusImpl opStatus;
            result = getReadById(rowId, packedRow, opStatus);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        return result;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return U2AssemblyRead();
    }
}

qint64 AssemblyRDbi::getMaxPackedRow(U2DataId assemblyId, const U2Region &r, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if(dbi.getEntityTypeById(assemblyId) != U2Type::Assembly) {
            throw Exception(BAMDbiPlugin::tr("The specified object is not an assembly"));
        }
        qint64 result = 0;
        {
            U2OpStatusImpl opStatus;
            SQLiteQuery q("SELECT MAX(packedRow) FROM assemblyReads WHERE assemblyId = ?1 AND startPosition < ?2 AND endPosition > ?3;", &dbRef, opStatus);
            q.bindInt64(1, assemblyId);
            q.bindInt64(2, r.endPos());
            q.bindInt64(3, r.startPos);
            result = q.selectInt64();
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        return result;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return 0;
    }
}

QList<U2AssemblyRead> AssemblyRDbi::getReadsByRow(U2DataId assemblyId, const U2Region &r, qint64 minRow, qint64 maxRow, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if(dbi.getEntityTypeById(assemblyId) != U2Type::Assembly) {
            throw Exception(BAMDbiPlugin::tr("The specified object is not an assembly"));
        }
        QList<U2DataId> rowIds;
        QList<qint64> packedRows;
        {
            U2OpStatusImpl opStatus;
            SQLiteQuery q("SELECT id, packedRow FROM assemblyReads WHERE assemblyId = ?1 AND startPosition < ?2 AND endPosition > ?3 AND packedRow >= ?4 AND packedRow <= ?5;", &dbRef, opStatus);
            q.bindInt64(1, assemblyId);
            q.bindInt64(2, r.endPos());
            q.bindInt64(3, r.startPos);
            q.bindInt64(4, minRow);
            q.bindInt64(5, maxRow);
            while(q.step()) {
                rowIds.append(q.getInt64(0));
                packedRows.append(q.getInt64(1));
            }
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        QList<U2AssemblyRead> result;
        {
            U2OpStatusImpl opStatus;
            result = getReadsByIds(rowIds, packedRows, opStatus);
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        return result;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return QList<U2AssemblyRead>();
    }
}

quint64 AssemblyRDbi::getMaxEndPos(U2DataId assemblyId, U2OpStatus &os) {
    try {
        if(U2DbiState_Ready != dbi.getState()) {
            throw Exception(BAMDbiPlugin::tr("Invalid DBI state"));
        }
        if(dbi.getEntityTypeById(assemblyId) != U2Type::Assembly) {
            throw Exception(BAMDbiPlugin::tr("The specified object is not an assembly"));
        }
        quint64 result = 0;
        {
            U2OpStatusImpl opStatus;
            SQLiteQuery q("SELECT maxEndPos FROM assemblies WHERE id = ?1;", &dbRef, opStatus);
            q.bindInt64(1, assemblyId);
            result = q.selectInt64();
            if(opStatus.hasError()) {
                throw Exception(opStatus.getError());
            }
        }
        return result;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return 0;
    }
}

U2AssemblyRead AssemblyRDbi::alignmentToRead(const Alignment &alignment) {
    U2AssemblyRead row;
    row.leftmostPos = alignment.getPosition();
    row.readSequence = alignment.getSequence();
    row.complementary = (alignment.getFlags() & Alignment::Reverse);
    foreach(const Alignment::CigarOperation &cigarOperation, alignment.getCigar()) {
        U2CigarOp cigarOp = U2CigarOp_Invalid;
        switch(cigarOperation.getOperation()) {
        case Alignment::CigarOperation::AlignmentMatch:
            cigarOp = U2CigarOp_M;
            break;
        case Alignment::CigarOperation::Insertion:
            cigarOp = U2CigarOp_I;
            break;
        case Alignment::CigarOperation::Deletion:
            cigarOp = U2CigarOp_D;
            break;
        case Alignment::CigarOperation::Skipped:
            cigarOp = U2CigarOp_N;
            break;
        case Alignment::CigarOperation::SoftClip:
            cigarOp = U2CigarOp_S;
            break;
        case Alignment::CigarOperation::HardClip:
            cigarOp = U2CigarOp_H;
            break;
        case Alignment::CigarOperation::Padding:
            cigarOp = U2CigarOp_P;
            break;
        case Alignment::CigarOperation::SequenceMatch:
            cigarOp = U2CigarOp_M;
            break;
        case Alignment::CigarOperation::SequenceMismatch:
            cigarOp = U2CigarOp_M;
            break;
        default:
            assert(false);
        }
        row.cigar.append(U2CigarToken(cigarOp, cigarOperation.getLength()));
    }
    return row;
}

U2AssemblyRead AssemblyRDbi::getReadById(U2DataId rowId, qint64 packedRow, U2OpStatus &os) {
    try {
        reader.seek(VirtualOffset((quint64)rowId));
        U2AssemblyRead row = alignmentToRead(reader.readAlignment());
        row.id = rowId;
        row.packedViewRow = packedRow;
        return row;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return U2AssemblyRead();
    }
}

QList<U2AssemblyRead> AssemblyRDbi::getReadsByIds(QList<U2DataId> rowIds, QList<qint64> packedRows, U2OpStatus &os) {
    try {
        QList<U2AssemblyRead> result;
        for(int index = 0;index < rowIds.size();index++) {
            assert(dbi.getEntityTypeById(rowIds[index]) == U2Type::AssemblyRead);
            U2AssemblyRead read;
            {
                U2OpStatusImpl opStatus;
                read = getReadById(rowIds[index], packedRows[index], opStatus);
                if(opStatus.hasError()) {
                    throw Exception(opStatus.getError());
                }
            }
            result.append(read);
        }
        return result;
    } catch(const Exception &e) {
        os.setError(e.getMessage());
        return QList<U2AssemblyRead>();
    }
}

} // namespace BAM
} // namespace U2
