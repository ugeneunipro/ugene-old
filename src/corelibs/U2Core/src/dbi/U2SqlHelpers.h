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

#ifndef _U2_SQL_HELPERS_H_
#define _U2_SQL_HELPERS_H_

#include <U2Core/U2OpStatus.h>
#include <U2Core/U2Type.h>

#include <QtCore/QMutex>
#include <QtCore/QStringList>

struct sqlite3;
struct sqlite3_stmt;

namespace U2 {

class SQLiteQuery;

class U2CORE_EXPORT DbRef {
public:
    DbRef() : handle(NULL), lock(QMutex::Recursive), useTransaction(true), transactionDepth(0){}
    DbRef(sqlite3* db) : handle(db), lock(QMutex::Recursive), useTransaction(true), transactionDepth(0) {}

    sqlite3*    handle;
    QMutex      lock;
    bool        useTransaction;
    int         transactionDepth;
};

class U2CORE_EXPORT SQLiteUtils {
public:
    
    /** 
        Removes from the table all records with 'field' == id 
        Checks 'expectedRowCount' the same way as 'update' method
    */
    static qint64 remove(const QString& table, const QString& field, U2DataId id, qint64 expectedRows, DbRef* db, U2OpStatus& os);
    
    /** Converts internal database id to U2DataId*/
    static U2DataId toU2DataId(qint64 id, U2DataType type);

    /** Converts U2DataId to internal database id*/
    static quint64 toDbiId(U2DataId id);

    /** Extracts type info from U2DataId */
    static U2DataType toType(U2DataId id);

    /** Adds limit operator to the sql query */
    static void addLimit(QString& sql, qint64 offset, qint64 count);
};

/** Common localization messages for SQLiteDBI*/
class U2CORE_EXPORT SQLiteL10n : public QObject {
public:
    static QString queryError(const QString& err);
    static QString tooManyResults();
};


/** 
    SQLite query wrapper. Uses prepared statement internally
    An optimized and simplified interface for U2DBI needs.
*/
class U2CORE_EXPORT SQLiteQuery  {
private:
    
public:
    /** 
        Constructs prepared statement for SQLiteDB 
        If failed the error message is written to 'os'
    */
    SQLiteQuery(const QString& sql, DbRef* d, U2OpStatus& os);
    SQLiteQuery(const QString& sql, qint64 offset, qint64 count, DbRef* d, U2OpStatus& os);

    /** Releases all resources associated with the statement */
    ~SQLiteQuery();

    //////////////////////////////////////////////////////////////////////////
    // Statement/query state manipulation methods

    /** Clears all bindings and resets statement */
    bool reset(bool clearBindings = true);

    /** 
        Executes next step of the statement 
        Returns true there are more results to fetch and no error occurs
    */
    bool step();

    /** 
        Ensures that there are no more results in result set
        Sets error message if more results are available
    */
    void ensureDone();

    
    //////////////////////////////////////////////////////////////////////////
    // param binding methods

    /** Binds U2DataId  */
    void bindDataId(int idx, U2DataId val);

    /** Binds U2DataType */
    void bindType(int idx, U2DataType type);

    /** Binds 32bit integer value*/
    void bindInt32(int idx, qint32 val);

    /** Binds 64bit integer value*/
    void bindInt64(int idx, qint64 val);

    /** Binds bool value */
    void bindBool(int idx, bool val);

    /** Binds text string */
    void bindText(int idx, const QString& val);

    /** Binds BLOB */
    void bindBlob(int idx, const QByteArray& blob, bool transient = true);

    
    //////////////////////////////////////////////////////////////////////////
    // result retrieval methods

    U2DataId getDataId(int column, U2DataType type) const;

    U2DataType getDataType(int column) const;

    int getInt32(int column) const;

    qint64 getInt64(int column) const;

    QString getString(int column) const;

    QByteArray getCString(int column) const;

    QByteArray getBlob(int column) const;

    bool getBool(int column) const {return getInt32(column) != 0;}

    //////////////////////////////////////////////////////////////////////////
    // Utility methods

    /** Executes update and returns last row id converted to U2DataId using type info*/
    U2DataId insert(U2DataType type);

    /** Executes query */
    void execute();

    /** Executes update and returns number of rows affected. 
        Fails if result count != expectedRowCount
        'expectedRowCount' == 1 disables row-count check
    */
    qint64 update(qint64 expectedRows = -1);


    /** Selects a single int64 value */
    qint64 selectInt64();

    /** Selects a single int64 value, if no results found returns default value */
    qint64 selectInt64(qint64 defaultValue);

    /** Selects a single U2DataId value */
    U2DataId selectDataId(U2DataType type);

    /** Select list of ids and adds 'type' parameter to construct U2DataId */
    QList<U2DataId> selectDataIds(U2DataType type);

    /** Select id(col=0), type(col=1) pairs  and constructs U2DataId */
    QList<U2DataId> selectDataIdsExt();

    /** Select list of strings */
    QStringList selectStrings();


        
    //////////////////////////////////////////////////////////////////////////
    // Query info methods
    const QString& getQueryText() const {return sql;}

    void setError(const QString& err) {os.setError(err);}
    
    bool hasError() const {return os.hasError();}
    
    DbRef*          getDb() const {return db;}

private:
    /** Returns last insert row id converted to U2DataId using type info */
    U2DataId getLastRowId(U2DataType type);
    void prepare();


    DbRef*          db;
    U2OpStatus&  os;
    sqlite3_stmt*   st;
    QString         sql;
};

/** Helper class to mark transaction regions */
class U2CORE_EXPORT SQLiteTransaction {
public:
    SQLiteTransaction(DbRef* db, U2OpStatus& os);
    virtual ~SQLiteTransaction();

private:
    DbRef* db;
    U2OpStatus& os;
};

} //namespace

#endif
