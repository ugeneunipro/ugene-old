/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <U2Core/L10n.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SqlHelpers.h>

#include "SqliteUpgraderFrom_0_To_1_13.h"
#include "../SQLiteDbi.h"
#include "../SQLiteAssemblyDbi.h"
#include "../SQLiteObjectRelationsDbi.h"

namespace U2 {

SqliteUpgraderFrom_0_To_1_13::SqliteUpgraderFrom_0_To_1_13(SQLiteDbi *dbi) :
    SqliteUpgrader(Version::parseVersion("0.0.0"), Version::parseVersion("1.13.0"), dbi)
{
}

void SqliteUpgraderFrom_0_To_1_13::upgrade(U2OpStatus &os) const {
    SQLiteTransaction t(dbi->getDbRef(), os);
    Q_UNUSED(t);

    upgradeObjectDbi(os);
    CHECK_OP(os, );

    upgradeObjectRelationsDbi(os);
    CHECK_OP(os, );

    upgradeAssemblyDbi(os);
    CHECK_OP(os, );

    dbi->setProperty(U2DbiOptions::APP_MIN_COMPATIBLE_VERSION, "1.13.0", os);
}

void SqliteUpgraderFrom_0_To_1_13::upgradeObjectDbi(U2OpStatus &os) const {
    SQLiteQuery q("PRAGMA table_info(Object)", dbi->getDbRef(), os);
    CHECK_OP(os, );

    bool hasModTrack = false;
    while (q.step()) {
        QString colName = q.getString(1);
        if ("trackMod" == colName) {
            hasModTrack = true;
            break;
        }
    }
    CHECK(!hasModTrack, );

    SQLiteQuery("ALTER TABLE Object ADD trackMod INTEGER NOT NULL DEFAULT 0", dbi->getDbRef(), os).execute();
}

void SqliteUpgraderFrom_0_To_1_13::upgradeObjectRelationsDbi(U2OpStatus &os) const {
    SQLiteObjectRelationsDbi *objectRelationsDbi = dbi->getSQLiteObjectRelationsDbi();
    SAFE_POINT_EXT(NULL != objectRelationsDbi, os.setError(L10N::nullPointerError("SQLite object relation dbi")), );
    objectRelationsDbi->initSqlSchema(os);
}

void SqliteUpgraderFrom_0_To_1_13::upgradeAssemblyDbi(U2OpStatus &os) const {
    DbRef *db = dbi->getDbRef();
    SQLiteQuery q("PRAGMA foreign_key_list(Assembly)", db, os);
    SAFE_POINT_OP(os, );

    bool referenceIsObject = false;
    while (q.step()) {
        const QString sourceColumn = q.getString(3);
        if ("reference" == sourceColumn && "Object" == q.getString(2)) {
            referenceIsObject = true;
            break;
        }
    }
    if (referenceIsObject) {
        return;
    }

    const QString newTableName = "Assembly_new";

    SQLiteQuery(SQLiteAssemblyDbi::getCreateAssemblyTableQuery(newTableName), db, os).execute();
    SAFE_POINT_OP(os,);

    SQLiteQuery assemblyFetch("SELECT object, reference, imethod, cmethod, idata, cdata FROM Assembly", db, os);
    SAFE_POINT_OP(os, );

    SQLiteQuery assemblyInsert(QString("INSERT INTO %1 (object, reference, imethod, cmethod, idata, cdata) VALUES(?1, ?2, ?3, ?4, ?5, ?6)")
        .arg(newTableName), db, os);
    SAFE_POINT_OP(os, );
    while (assemblyFetch.step()) {
        assemblyInsert.bindDataId(1, assemblyFetch.getDataId(0, U2Type::Assembly));
        const U2DataId refId = assemblyFetch.getDataId(1, U2Type::CrossDatabaseReference);
        const qint64 dbiRefId = U2DbiUtils::toDbiId(refId);
        if (0 == dbiRefId) {
            assemblyInsert.bindNull(2);
        } else {
            assemblyInsert.bindDataId(2, refId);
        }
        assemblyInsert.bindString(3, assemblyFetch.getString(2));
        assemblyInsert.bindString(4, assemblyFetch.getString(3));
        assemblyInsert.bindBlob(5, assemblyFetch.getBlob(4));
        assemblyInsert.bindBlob(6, assemblyFetch.getBlob(5));
        assemblyInsert.insert();
        SAFE_POINT_OP(os, );
        assemblyInsert.reset();
    }

    SQLiteQuery("DROP TABLE Assembly", db, os).execute();
    SAFE_POINT_OP(os, );

    SQLiteQuery(QString("ALTER TABLE %1 RENAME TO Assembly").arg(newTableName), db, os).execute();
}

}   // namespace U2
