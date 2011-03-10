#ifndef _U2_BAM_DBI_H_
#define _U2_BAM_DBI_H_

#include <U2Core/U2Dbi.h>
#include <U2Core/U2DbiUtils.h>
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
    void buildIndex();

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
    AssemblyRDbi(Dbi &dbi, Reader &reader, DbRef &dbRef, int assembliesCount);

    virtual U2Assembly getAssemblyObject(U2DataId id, U2OpStatus &os);

    virtual qint64 countReadsAt(U2DataId assemblyId, const U2Region &r, U2OpStatus &os);

    virtual QList<U2DataId> getReadIdsAt(U2DataId assemblyId, const U2Region &r, qint64 offset, qint64 count, U2OpStatus &os);

    virtual QList<U2AssemblyRead> getReadsAt(U2DataId assemblyId, const U2Region &r, qint64 offset, qint64 count, U2OpStatus &os);

    virtual U2AssemblyRead getReadById(U2DataId rowId, U2OpStatus &os);

    virtual qint64 getMaxPackedRow(U2DataId assemblyId, const U2Region &r, U2OpStatus &os);

    virtual QList<U2AssemblyRead> getReadsByRow(U2DataId assemblyId, const U2Region &r, qint64 minRow, qint64 maxRow, U2OpStatus &os);

    virtual quint64 getMaxEndPos(U2DataId assemblyId, U2OpStatus &os);
private:
    U2AssemblyRead getReadById(U2DataId rowId, qint64 packedRow, U2OpStatus &os);
    QList<U2AssemblyRead> getReadsByIds(QList<U2DataId> rowIds, QList<qint64> packedRows, U2OpStatus &os);

    Dbi &dbi;
    Reader &reader;
    DbRef &dbRef;
    int assembliesCount;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_DBI_H_
