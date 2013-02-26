/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "SQLiteModDbi.h"

#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2SafePoints.h>


namespace U2 {

SQLiteModDbi::SQLiteModDbi(SQLiteDbi* dbi) : U2ModDbi(dbi), SQLiteChildDBICommon(dbi) {
}

void SQLiteModDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }

    // ModStep - single modification of a dbi object
    //   id       - id of the modification
    //   object   - id of the object that was modified
    //   version  - this is a modification from 'version' to 'version + 1' of the object
    //   modType  - type of the object modification
    //   details  - detailed description of the object modification
    SQLiteQuery("CREATE TABLE ModStep (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
        " object INTEGER NOT NULL,"
        " otype INTEGER NOT NULL,"
        " oextra BLOB NOT NULL,"
        " version INTEGER NOT NULL,"
        " modType INTEGER NOT NULL,"
        " details TEXT NOT NULL,"
        " FOREIGN KEY(object) REFERENCES Object(id) )", db, os).execute();
    SQLiteQuery("CREATE INDEX ModStep_object ON ModStep(object)", db, os).execute();
    SQLiteQuery("CREATE INDEX ModStep_object_version ON ModStep(object, version)", db, os).execute();
}

U2ModStep SQLiteModDbi::getModStep(const U2DataId& objectId, qint64 trackVersion, U2OpStatus& os) {
    U2ModStep res;
    SQLiteQuery q("SELECT id, object, otype, oextra, version, modType, details FROM ModStep WHERE object = ?1 AND version = ?2", db, os);
    CHECK_OP(os, res);

    q.bindDataId(1, objectId);
    q.bindInt64(2, trackVersion);

    if (q.step()) {
        res.id = q.getInt32(0);
        res.objectId = q.getDataIdExt(1);
        res.version = q.getInt64(4);
        res.modType = q.getInt64(5);
        res.details = q.getBlob(6);
        q.ensureDone();
    }
    else if (!os.hasError()) {
        os.setError(SQLiteL10N::tr("An object modification step not found!"));
    }

    return res;
}

void SQLiteModDbi::createModStep(U2ModStep& step, U2OpStatus& os) {
    SQLiteQuery q("INSERT INTO ModStep(object, otype, oextra, version, modType, details) VALUES(?1, ?2, ?3, ?4, ?5, ?6)", db, os);
    CHECK_OP(os, );

    q.bindDataId(1, step.objectId);
    q.bindType(2, SQLiteUtils::toType(step.objectId));
    q.bindBlob(3, SQLiteUtils::toDbExtra(step.objectId));
    q.bindInt64(4, step.version);
    q.bindInt64(5, step.modType);
    q.bindString(6, step.details);
    step.id = q.insert();
}

void SQLiteModDbi::removeModsWithGreaterVersion(const U2DataId& objectId, qint64 version, U2OpStatus& os) {
    SQLiteQuery q("DELETE FROM ModStep WHERE object = ?1 AND version >= ?2", db, os);
    CHECK_OP(os, );

    q.bindDataId(1, objectId);
    q.bindInt64(2, version);
    q.execute();
}

void SQLiteModDbi::removeObjectMods(const U2DataId& objectId, U2OpStatus& os) {
    SQLiteQuery q("DELETE FROM ModStep WHERE object = ?1", db, os);
    CHECK_OP(os, );

    q.bindDataId(1, objectId);
    q.execute();
}

} // namespace
