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

#include "SQLiteDbi.h"
#include "SQLiteObjectDbi.h"
#include "SQLiteSequenceDbi.h"
#include "SQLiteMsaDbi.h"
#include "SQLiteAssemblyDbi.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/Log.h>
#include <U2Core/Version.h>
#include <U2Core/GUrl.h>

#include <sqlite3.h>

namespace U2 {


SQLiteDbi::SQLiteDbi() : U2AbstractDbi (SQLiteDbiFactory::ID){
    db = new DbRef();
    flags = 0;

    objectDbi = new SQLiteObjectDbi(this);
    sequenceDbi = new SQLiteSequenceDbi(this);
    msaRDbi = new SQLiteMsaRDbi(this);
    assemblyDbi = new SQLiteAssemblyDbi(this);
}

SQLiteDbi::~SQLiteDbi() {
    delete objectDbi;
    delete sequenceDbi;
    delete msaRDbi;
    delete assemblyDbi;
    if (db->handle != NULL) {
        sqlite3_close(db->handle);
    }
    delete db;
}


QString SQLiteDbi::getFlag(const QString& name, U2OpStatus& os) const {
    SQLiteQuery q("SELECT value FROM Meta WHERE name = ?1", db, os);
    q.bindText(1, name);
    q.step();
    return q.getCString(0);
}

void SQLiteDbi::addFlag(const QString& name, const QString& value, U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }
    SQLiteQuery q("INSERT INTO Meta(name, value) VALUES (?1, ?2)", db, os);
    q.bindText(1, name);
    q.bindText(2, value);
    q.execute();
}

static int isEmptyCallback(void *o, int argc, char **argv, char **column) {
    int* res = (int*)o;
    *res = argc;
    return 0;
}

static bool isEmpty(DbRef* db, U2OpStatus& os) {
    QByteArray showTablesQuery = "SELECT * FROM sqlite_master WHERE type='table';";
    int nTables = 0;
    char* err;
    int rc = sqlite3_exec(db->handle, showTablesQuery.constData(), isEmptyCallback, &nTables, &err);
    if (rc != SQLITE_OK) {
        os.setError(SQLiteL10N::tr("Error checking SQLite database: %1!").arg(err));
        sqlite3_free(err);
        return false;
    }
    return nTables == 0;
}

#define CT(table, fields) \
{ \
    char* err = NULL; \
    QByteArray query = QByteArray("CREATE TABLE ") + table + " (" + fields + ");";\
    int rc = sqlite3_exec(db->handle, query, NULL, NULL, &err); \
    if (rc != SQLITE_OK) { \
        os.setError(SQLiteL10N::tr("Error creating table: %1, error: %2").arg(table).arg(err)); \
        sqlite3_free(err); \
        return; \
    } \
}

void SQLiteDbi::populateDefaultSchema(U2OpStatus& os) {
    // meta table, stores general db info
    CT("Meta", "name TEXT NOT NULL, value TEXT NOT NULL"); 
    
    // objects table - stores IDs and types for all objects. It also stores 'top_level' flag to simplify queries
    // top_level = 1 if object is placed into some folder.
    // name is a visual name of the object shown to user.
    CT("Object", " id INTEGER PRIMARY KEY AUTOINCREMENT, type INTEGER NOT NULL, "
        "version INTEGER NOT NULL DEFAULT 1, top_level INTEGER NOT NULL, "
        "name TEXT NOT NULL");
    
    // parent-child object relation
    CT("Parent", "parent INTEGER, child INTEGER, "
                 "FOREIGN KEY(parent) REFERENCES Object(id), "
                 "FOREIGN KEY(child) REFERENCES Object(id) ");

    // folders 
    CT("Folder", "id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT UNIQUE NOT NULL,  "
                 "vlocal INTEGER NOT NULL DEFAULT 1, vglobal INTEGER NOT NULL DEFAULT 1");

    // folder-object relation
    CT("FolderContent", "folder INTEGER, object INTEGER, "
                        "FOREIGN KEY(folder) REFERENCES Folder(id),"
                        "FOREIGN KEY(object) REFERENCES Object(id) ");

    // sequence object
    CT("Sequence", "object INTEGER, length INTEGER NOT NULL DEFAULT 0, alphabet TEXT NOT NULL, "
                   "circular INTEGER NOT NULL DEFAULT 0, "
                   "FOREIGN KEY(object) REFERENCES Object(id) ");

    // part of the sequence, starting with 'sstart'(inclusive) and ending at 'send'(not inclusive)
    CT("SequenceData", "sequence INTEGER, sstart INTEGER NOT NULL, send INTEGER NOT NULL, data BLOB NOT NULL, "
                        "FOREIGN KEY(sequence) REFERENCES Sequence(object) ");

    // msa object
    CT("Msa", "object INTEGER, length INTEGER NOT NULL, alphabet TEXT NOT NULL, sequenceCount INTEGER NOT NULL, "
              " FOREIGN KEY(object) REFERENCES Object(id) ");

    // msa object row
    // msa      - msa object id
    // sequence - sequence object id
    // pos      - positional number of row in msa
    // gstart   - offset of the first non-gap sequence element. Same as gstart of the first MsaRowGap
    // gend     - offset of the last non-gap sequence element. Same as gend of the last MsaRowGap
    CT("MsaRow", "msa INTEGER, sequence INTEGER, pos INTEGER NOT NULL, gstart INTEGER NOT NULL, gend INTEGER NOT NULL, "
                " FOREIGN KEY(msa) REFERENCES Msa(object), "
                " FOREIGN KEY(sequence) REFERENCES Msa(object) ");

    // gap info for msa row: 
    //  gstart  - global (in msa) start of non-gap region
    //  gend    - global (in msa) end of non-gap region
    //  sstart  - local (in sequence) start of non-gap region
    //  send    - local (in sequence) end of non-gap region
    // Note! there is invariant: gend - gstart == send - sstart
    CT("MsaRowGap", "msa INTEGER, sequence INTEGER, gstart INTEGER NOT NULL, gend INTEGER NOT NULL, "
                    " sstart INTEGER NOT NULL, send INTEGER NOT NULL, "
                    " FOREIGN KEY(msa) REFERENCES MsaRow(msa), "
                    " FOREIGN KEY(sequence) REFERENCES MsaRow(sequence)");


    // assembly object
    // reference    - reference sequence id
    // lextra       - extra info about max read length in assembly. Different strategies can be used here. Depends on DBI version
    CT("Assembly", "object INTEGER, reference INTEGER, lextra INTEGER NOT NULL, "
                    " FOREIGN KEY(object) REFERENCES Object(id), "
                    " FOREIGN KEY(reference) REFERENCES Sequence(object)");

    addFlag(SQLITE_DBI_OPTION_UGENE_VERSION, Version::ugeneVersion().text, os);

    if (initProperties[SQLITE_DBI_OPTION_ASSEMBLY_READ_COMPRESSION1_FLAG] == U2_DBI_VALUE_ON) {
        addFlag(SQLITE_DBI_OPTION_ASSEMBLY_READ_COMPRESSION1_FLAG, U2_DBI_VALUE_ON, os);
    }

}

void SQLiteDbi::internalInit(U2OpStatus& os){
    QString dbUgeneVersionText = getFlag(SQLITE_DBI_OPTION_UGENE_VERSION, os);
    if (os.hasError()) {
        return;
    }
    if (dbUgeneVersionText.isEmpty()) {
        os.setError(SQLiteL10n::tr("Not a UGENE SQLite database: %1").arg(url));
        return;
    }
    Version dbUgeneVersion = Version::parseVersion(dbUgeneVersionText);
    Version currentVersion = Version::ugeneVersion();
    if (dbUgeneVersion > currentVersion) {
        coreLog.info(SQLiteL10n::tr("Warning! Database was created with a newer UGENE version: %2. Not all database features are supported!"));
    }
    QString readsCompression1 = getFlag(SQLITE_DBI_OPTION_ASSEMBLY_READ_COMPRESSION1_FLAG, os);
    if (readsCompression1 == U2_DBI_VALUE_ON) {
        coreLog.trace(QString("SQLiteDbi: enabling reads compression mode: %1").arg(url));
        flags|=SQLiteDbiFlag_AssemblyReadsCompression1;
    }

    // set up features list
    features.insert(U2DbiFeature_ReadSequence);
    features.insert(U2DbiFeature_ReadMsa);
    features.insert(U2DbiFeature_ReadAssembly);
    features.insert(U2DbiFeature_WriteSequence);
    //features.insert(U2DbiFeature_WriteMsa);
    features.insert(U2DbiFeature_WriteAssembly);
    features.insert(U2DbiFeature_AssemblyReadsPacking);
    features.insert(U2DbiFeature_RemoveObjects);
    features.insert(U2DbiFeature_ChangeFolders);
}


void SQLiteDbi::setState(U2DbiState s) {
    state = s;
}

void SQLiteDbi::init(const QHash<QString, QString>& props, const QVariantMap&, U2OpStatus& os) {
    if (db->handle != NULL) {
        os.setError(SQLiteL10N::tr("Database is already opened!"));
        return;
    }
    if (state != U2DbiState_Void) {
        os.setError(SQLiteL10N::tr("Illegal database state: %1").arg(state));
        return;
    }
    setState(U2DbiState_Starting);
    url = props.value(U2_DBI_OPTION_URL);
    if (url.isEmpty()) {
        os.setError(SQLiteL10N::tr("URL is not specified"));
        setState(U2DbiState_Void);
        return;
    }
    do {
        int flags = SQLITE_OPEN_READWRITE;
        bool create = props.value(U2_DBI_OPTION_CREATE, "0").toInt() > 0;
        if (create) {
            flags |= SQLITE_OPEN_CREATE;
        }
        QByteArray file = url.toUtf8();
        int rc = sqlite3_open_v2(file.constData(), &db->handle, flags, NULL);
        if (rc != SQLITE_OK) {
            QString err = db == NULL ? QString(" error-code: %1").arg(rc) : QString(sqlite3_errmsg(db->handle));
            os.setError(SQLiteL10N::tr("Error opening SQLite database: %1!").arg(err));
            break;
        }
 
        SQLiteQuery("PRAGMA synchronous = OFF", db, os).execute();
        SQLiteQuery("PRAGMA main.locking_mode = EXCLUSIVE", db, os).execute();
        SQLiteQuery("PRAGMA temp_store = MEMORY", db, os).execute();
        SQLiteQuery("PRAGMA journal_mode = MEMORY", db, os).execute();
        
        
        // check if the opened database is valid UGENE sqlite dbi
        initProperties = props;
        if (isEmpty(db, os)) {
            if (create) {
                populateDefaultSchema(os);
                if (os.hasError()) {
                    break;
                }
            }
        } 

        dbiId = url;
        internalInit(os);
        // OK, initialization complete

    } while (0);
    
    if (os.hasError()) {
        sqlite3_close(db->handle);
        db->handle = NULL;
        setState(U2DbiState_Void);
        return;
    }
    setState(U2DbiState_Ready);
}

QVariantMap SQLiteDbi::shutdown(U2OpStatus& os) {
    if (db == NULL) {
        os.setError(SQLiteL10N::tr("Database is already closed!"));
        return QVariantMap();
    }
    if (state != U2DbiState_Ready) {
        os.setError(SQLiteL10N::tr("Illegal database state %1!").arg(state));
        return QVariantMap();
    }
    
    setState(U2DbiState_Stopping);
    sqlite3_close(db->handle);
    db->handle = NULL;
    url.clear();
    initProperties.clear();

    setState(U2DbiState_Void);
    return QVariantMap();
}
    
bool SQLiteDbi::flush(U2OpStatus& os) {
    //todo:
    return true;
}

QString SQLiteDbi::getDbiId() const {
    return url;
}

QHash<QString, QString> SQLiteDbi::getDbiMetaInfo(U2OpStatus& os) {
    QHash<QString, QString> res;
    res["url"] = url;
    return res;

}

U2DataType SQLiteDbi::getEntityTypeById(U2DataId id) const {
    return SQLiteUtils::toType(id);
}

SQLiteObjectDbi* SQLiteDbi::getSQLiteObjectDbi() const {
    return static_cast<SQLiteObjectDbi*>(objectDbi);
}

// SQLiteDbiFactory

SQLiteDbiFactory::SQLiteDbiFactory() : U2DbiFactory()
{
}

U2Dbi *SQLiteDbiFactory::createDbi() {
    return new SQLiteDbi();
}

U2DbiFactoryId SQLiteDbiFactory::getId()const {
    return ID;
}

bool SQLiteDbiFactory::isValidDbi(const QHash<QString, QString>& properties, const QByteArray& rawData, U2OpStatus& os) const {
    QString surl  = properties.value(U2_DBI_OPTION_URL);
    GUrl url(surl);
    if (!url.isLocalFile()) {
        return false;
    }
    return rawData.startsWith("SQLite format 3");
}

const U2DbiFactoryId SQLiteDbiFactory::ID = "SQLiteDbi";

}//namespace
