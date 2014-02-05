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

#ifndef _U2_SQLITE_DBI_H_
#define _U2_SQLITE_DBI_H_

#include <U2Core/U2AbstractDbi.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2SqlHelpers.h>

struct sqlite3;

namespace U2 {

class SQLiteObjectDbi;
class SQLiteSequenceDbi;
class SQLiteMsaDbi;
class SQLiteAssemblyDbi;
class SQLiteCrossDatabaseReferenceDbi;
class SQLiteAttributeDbi;
class SQLiteVariantDbi;
class SQLiteFeatureDbi;
class SQLiteModDbi;
class SQLiteSNPTablesDbi;
class SQLiteKnownMutationsDbi;
class SQLiteUdrDbi;
class DbRef;


// Names of SQLiteDbi flags
#define SQLITE_DBI_OPTION_APP_VERSION    (U2_PRODUCT_KEY + QString("-version"))


/** Name of the init property used to indicate assembly reads storage method for all new assemblies */
#define SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_KEY "sqlite-assembly-reads-elen-method"
/** Stores all reads in a single table. Not optimal if read effective length varies */
#define SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_SINGLE_TABLE "single-table"
/** Store all reads in N tables sorted by effective read length */
#define SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_MULTITABLE_V1 "multi-table-v1"
/** Uses RTree index to store reads. This method is simple but not very efficient in terms of space/insert time */
#define SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_RTREE "rtree2d"

/** Name of the property used to indicate compression algorithm for reads data */
#define SQLITE_DBI_ASSEMBLY_READ_COMPRESSION_METHOD_KEY "sqlite-assembly-reads-compression-method"
/** No compression is applied. Best for manual DB browsing  (default)*/
#define SQLITE_DBI_ASSEMBLY_READ_COMPRESSION_METHOD_NO_COMPRESSION "no-compression"
/** CIGAR and sequence are packed using bits compression and stored as a single BLOB */
#define SQLITE_DBI_ASSEMBLY_READ_COMPRESSION_METHOD_BITS_1 "compress-bits-1"


/** Indicates object life-cycle and storage location */
enum SQLiteDbiObjectRank {
    /** Object  is stored in this database and is top-level (included into some folder) */
    SQLiteDbiObjectRank_TopLevel = 1,
    /** Object  is stored in this database and is not top-level, it is a child of another object */
    SQLiteDbiObjectRank_Child = 2,
    /** Object  is stored in another database, see CrossDbiReference table for details */
    SQLiteDbiObjectRank_Remote = 3
};

// Values of SQLiteDbi flags
#define SQLITE_DBI_VALUE_MEMORY_DB_URL ":memory:"


class U2FORMATS_EXPORT SQLiteDbi : public U2AbstractDbi {
public:
    SQLiteDbi();
    ~SQLiteDbi();

    /** 
    Boots the database up to functional state. 
    Can be called again after successful shutdown, to re-load the database.
    special flags 
    "assembly-reads-compression=1" - enables reads compression for assembly. Affects new DB only
    */
    virtual void init(const QHash<QString, QString>& properties, const QVariantMap& persistentData, U2OpStatus& os);

    /** Stops the database and frees up used resources. */
    virtual QVariantMap shutdown(U2OpStatus& os);

    /** 
    Ensures that dbi state is synchronized with storage 
    Return 'true' of operation is successful
    */
    virtual bool flush(U2OpStatus& os);

    /** Unique database id. Used for cross-database references. */
    virtual QString getDbiId() const;

    /** Returns database meta-info. Any set of properties to be shown to user */
    virtual QHash<QString, QString> getDbiMetaInfo(U2OpStatus& os) ;

    /** Returns type of the entity referenced by the given ID */    
    virtual U2DataType getEntityTypeById(const U2DataId& id) const;


    virtual U2ObjectDbi* getObjectDbi();

    virtual U2SequenceDbi* getSequenceDbi();

    virtual U2MsaDbi* getMsaDbi();

    virtual U2AssemblyDbi* getAssemblyDbi();
    
    virtual U2CrossDatabaseReferenceDbi* getCrossDatabaseReferenceDbi();

    virtual U2AttributeDbi* getAttributeDbi();

    virtual U2VariantDbi*   getVariantDbi();

    virtual U2FeatureDbi*   getFeatureDbi();

    virtual U2ModDbi*   getModDbi();

    virtual UdrDbi* getUdrDbi();


    virtual SNPTablesDbi* getSNPTableDbi();

    virtual KnownMutationsDbi* getKnownMutationsDbi();

    DbRef*    getDbRef() const {return db;}

    SQLiteObjectDbi* getSQLiteObjectDbi() const;

    SQLiteMsaDbi* getSQLiteMsaDbi() const;

    SQLiteSequenceDbi* getSQLiteSequenceDbi() const;

    SQLiteModDbi* getSQLiteModDbi() const;

    SQLiteUdrDbi* getSQLiteUdrDbi() const;

    /** Returns properties used to initialized the database */
    virtual QHash<QString, QString> getInitProperties() const {return initProperties;}

    virtual QString getProperty(const QString& name, const QString& defaultValue, U2OpStatus& os);

    virtual void setProperty(const QString& name, const QString& value, U2OpStatus& os);

    virtual void startOperationsBlock(U2OpStatus &os);

    virtual void stopOperationBlock();

private:
    QString getLastErrorMessage(int rc);

    void setState(U2DbiState state);

    void populateDefaultSchema(U2OpStatus& os);
    void internalInit(const QHash<QString, QString>& props, U2OpStatus& os);
    /**
     * If the database was created by a previous version of UGENE
     * then it must be upgraded up to the current version.
     */
    void upgrade(U2OpStatus &os);

    QString                             url;
    DbRef*                              db;

    SQLiteObjectDbi*                    objectDbi;
    SQLiteSequenceDbi*                  sequenceDbi;
    SQLiteMsaDbi*                       msaDbi;
    SQLiteAssemblyDbi*                  assemblyDbi;
    SQLiteCrossDatabaseReferenceDbi*    crossDbi;
    SQLiteAttributeDbi*                 attributeDbi;
    SQLiteVariantDbi*                   variantDbi;
    SQLiteFeatureDbi*                   featureDbi;
    SQLiteModDbi*                       modDbi;
    SQLiteTransaction*                  operationsBlockTransaction;
    SQLiteSNPTablesDbi*                 filterTableDbi;
    SQLiteKnownMutationsDbi*            knownMutationsDbi;
    SQLiteUdrDbi*                       udrDbi;

    friend class SQLiteObjectDbi;
    friend class SQLiteCrossDatabaseReferenceDbi;
    friend class SQLiteSequenceDbi;
    friend class SQLiteAssemblyDbi;
    friend class SQLiteMsaDbi;
    friend class SQLiteAttributeDbi;
};

class U2FORMATS_EXPORT SQLiteDbiFactory : public U2DbiFactory {
public:
    SQLiteDbiFactory();

    /** Creates new DBI instance */
    virtual U2Dbi *createDbi();

    /** Returns DBI type ID */
    virtual U2DbiFactoryId getId() const;

    /** Checks that data pointed by properties is a valid DBI resource */
    virtual FormatCheckResult isValidDbi(const QHash<QString, QString>& properties, const QByteArray& rawData, U2OpStatus& os) const;

    virtual GUrl id2Url(const U2DbiId& id) const {return GUrl(id, GUrl_File);}

    virtual bool isDbiExists(const U2DbiId& id) const;


public:
    static const U2DbiFactoryId ID;
};

/** helper class, used as a base for all SQLite<child>Dbis */
class SQLiteChildDBICommon {
public:
    SQLiteChildDBICommon(SQLiteDbi* dbi) : dbi(dbi), db (dbi->getDbRef()){}
    virtual ~SQLiteChildDBICommon(){}

    virtual void initSqlSchema(U2OpStatus& os) = 0;
    virtual void shutdown(U2OpStatus&) {};

protected:
    SQLiteDbi*  dbi;
    DbRef*      db;
};

class SQLiteL10N : public QObject {
    Q_OBJECT
};

} //namespace

#endif
