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

#ifndef _U2_BAM_DBI_H_
#define _U2_BAM_DBI_H_

#include <U2Core/U2Dbi.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2DbiRegistry.h>

#include "Reader.h"

#include <memory>

namespace U2 {
namespace BAM {

class ObjectRDbi;
class AssemblyRDbi;

class Dbi : public U2AbstractDbi {
public:
    Dbi();

    virtual void init(const QHash<QString, QString> &properties, const QVariantMap &persistentData, U2OpStatus &os);

    virtual QVariantMap shutdown(U2OpStatus &os);
    
    virtual QString getDbiId() const;

    virtual QHash<QString, QString> getDbiMetaInfo(U2OpStatus &);

    virtual U2DataType getEntityTypeById(U2DataId id) const;

    virtual U2ObjectRDbi *getObjectRDbi();

    virtual U2AssemblyRDbi *getAssemblyRDbi();

    virtual QHash<QString, QString> getInitProperties() const {return initProps;}

private:
    void buildIndex(U2OpStatus &os);

private:
    static const int COLUMN_DISTANCE = 100;

    GUrl url;
    GUrl sqliteUrl;
    DbRef dbRef;
    int assembliesCount;
    std::auto_ptr<IOAdapter> ioAdapter;
    std::auto_ptr<Reader> reader;
    std::auto_ptr<ObjectRDbi> objectRDbi;
    std::auto_ptr<AssemblyRDbi> assemblyRDbi;
    QHash<QString, QString> initProps;
};

class DbiFactory : public U2DbiFactory {
public:
    DbiFactory();

    virtual U2Dbi *createDbi();

    virtual U2DbiFactoryId getId()const;

    /** Checks that data pointed by properties is a valid DBI resource */
    virtual bool isValidDbi(const QHash<QString, QString>& properties, const QByteArray& rawData, U2OpStatus& os) const;

public:
    static const QString ID;
};

class ObjectRDbi : public U2ObjectRDbi {
public:
    ObjectRDbi(Dbi &dbi, DbRef &dbRef, int assembliesCount);

    virtual qint64 countObjects(U2OpStatus &os);

    virtual qint64 countObjects(U2DataType type, U2OpStatus &os);

    virtual QList<U2DataId> getObjects(qint64 offset, qint64 count, U2OpStatus &os);

    virtual QList<U2DataId> getObjects(U2DataType type, qint64 offset, qint64 count, U2OpStatus &os);

    virtual QList<U2DataId> getParents(U2DataId entityId, U2OpStatus &os);

    virtual QStringList getFolders(U2OpStatus &os);

    virtual qint64 countObjects(const QString &folder, U2OpStatus &os);

    virtual QList<U2DataId> getObjects(const QString &folder, qint64 offset, qint64 count, U2OpStatus &os);

    virtual QStringList getObjectFolders(U2DataId objectId, U2OpStatus &os);

    virtual qint64 getObjectVersion(U2DataId objectId, U2OpStatus& os);

    virtual qint64 getFolderLocalVersion(const QString& folder, U2OpStatus& os);

    virtual qint64 getFolderGlobalVersion(const QString& folder, U2OpStatus& os);

private:
    Dbi &dbi;
    DbRef &dbRef;
    int assembliesCount;
};

class AssemblyRDbi : public U2AssemblyRDbi {
public:
    AssemblyRDbi(Dbi &dbi, Reader &reader, DbRef &dbRef, int assembliesCount, QList<qint64> maxReadLengths);

    virtual U2Assembly getAssemblyObject(U2DataId id, U2OpStatus &os);

    virtual qint64 countReadsAt(U2DataId assemblyId, const U2Region &r, U2OpStatus &os);

    virtual QList<U2DataId> getReadIdsAt(U2DataId assemblyId, const U2Region &r, qint64 offset, qint64 count, U2OpStatus &os);

    virtual QList<U2AssemblyRead> getReadsAt(U2DataId assemblyId, const U2Region &r, qint64 offset, qint64 count, U2OpStatus &os);

    virtual U2AssemblyRead getReadById(U2DataId rowId, U2OpStatus &os);

    virtual qint64 getMaxPackedRow(U2DataId assemblyId, const U2Region &r, U2OpStatus &os);

    virtual QList<U2AssemblyRead> getReadsByRow(U2DataId assemblyId, const U2Region &r, qint64 minRow, qint64 maxRow, U2OpStatus &os);

    virtual quint64 getMaxEndPos(U2DataId assemblyId, U2OpStatus &os);

    static U2AssemblyRead alignmentToRead(const Alignment &alignment);
private:
    qint64 getMaxReadLength(U2DataId assemblyId, const U2Region &r);
    U2AssemblyRead getReadById(U2DataId rowId, qint64 packedRow, U2OpStatus &os);
    QList<U2AssemblyRead> getReadsByIds(QList<U2DataId> rowIds, QList<qint64> packedRows, U2OpStatus &os);

    Dbi &dbi;
    Reader &reader;
    DbRef &dbRef;
    int assembliesCount;
    QList<qint64> maxReadLengths;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_DBI_H_
