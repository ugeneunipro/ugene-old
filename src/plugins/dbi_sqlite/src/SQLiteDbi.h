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

#ifndef _U2_SQLITE_DBI_H_
#define _U2_SQLITE_DBI_H_

#include <U2Core/U2AbstractDbi.h>
#include <U2Core/U2DbiRegistry.h>

struct sqlite3;

namespace U2 {

class SQLiteObjectDbi;
class DbRef;


// Names of SQLiteDbi flags
#define SQLITE_DBI_OPTION_UGENE_VERSION    "ugene-version"


/** Name of the init property used to indicate assembly reads storage method for all new assemblies */
#define SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_KEY "assembly-reads-elen-method"
/** Asks to store all reads in a single table. Not optimal if read effective length varies */
#define SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_SINGLE_TABLE "single-table"
/** Asks to store all reads in 6 tables. Number is a maximum effective length of the read to be stored in the table (default)*/
#define SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_MULTITABLE_4 "50-100-700-U"
/** Asks to use RTree index to store reads. This method is simple but not very efficient in terms of space/insert time */
#define SQLITE_DBI_ASSEMBLY_READ_ELEN_METHOD_RTREE "rtree"

/** Name of the property used to indicate compression algorithm for reads data */
#define SQLITE_DBI_ASSEMBLY_READ_COMPRESSION_METHOD_KEY "assembly-reads-compression-method"
/** No compression is applied. Best for manual DB browsing  (default)*/
#define SQLITE_DBI_ASSEMBLY_READ_COMPRESSION_METHOD_NO_COMPRESSION "no-compression"
/** CIGAR and sequence are packed using bits compression and stored as a single BLOB */
#define SQLITE_DBI_ASSEMBLY_READ_COMPRESSION_METHOD_BITS_1 "compress-bits-1"

// Values of SQLiteDbi flags
#define SQLITE_DBI_VALUE_MEMORY_DB_URL ":memory:"


class SQLiteDbi : public U2AbstractDbi {
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


    virtual U2ObjectDbi* getObjectDbi()  {return objectDbi;}

    virtual U2SequenceDbi* getSequenceDbi()  {return sequenceDbi;}

    virtual U2AssemblyDbi* getAssemblyDbi()  {return assemblyDbi;}

    DbRef*    getDbRef() const {return db;}

    SQLiteObjectDbi* getSQLiteObjectDbi() const;

    /** Returns properties used to initialized the database */
    virtual QHash<QString, QString> getInitProperties() const {return initProperties;}

private:
    void setState(U2DbiState state);

    QString getProperty(const QString& name, const QString& defaultValue, U2OpStatus& os) const;
    void setProperty(const QString& name, const QString& value, U2OpStatus& os);

    void populateDefaultSchema(U2OpStatus& os);
    void internalInit(U2OpStatus& os);

    QString             url;
    DbRef*              db;

    U2ObjectDbi*        objectDbi;
    U2SequenceDbi*      sequenceDbi;
    U2MsaDbi*           msaRDbi;
    U2AssemblyDbi*      assemblyDbi;
    
    friend class SQLiteObjectDbi;
    friend class SQLiteSequenceDbi;
    friend class SQLiteAssemblyDbi;
    friend class SQLiteMsaDbi;
};

class SQLiteDbiFactory : public U2DbiFactory {
public:
    SQLiteDbiFactory();

    /** Creates new DBI instance */
    virtual U2Dbi *createDbi();

    /** Returns DBI type ID */
    virtual U2DbiFactoryId getId() const;

    /** Checks that data pointed by properties is a valid DBI resource */
    virtual bool isValidDbi(const QHash<QString, QString>& properties, const QByteArray& rawData, U2OpStatus& os) const;

public:
    static const U2DbiFactoryId ID;
};

/** helper class, used as a base for all SQLite<child>Dbis */
class SQLiteChildDBICommon {
public:
    SQLiteChildDBICommon(SQLiteDbi* dbi) : dbi(dbi), db (dbi->getDbRef()){}
    virtual ~SQLiteChildDBICommon(){}

protected:
    SQLiteDbi*  dbi;
    DbRef*      db;
};

class SQLiteL10N : public QObject {
    Q_OBJECT
};

} //namespace

#endif
