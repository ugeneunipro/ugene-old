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
        " version INTEGER NOT NULL,"
        " modType TEXT NOT NULL,"
        " details TEXT NOT NULL,"
        " FOREIGN KEY(object) REFERENCES Object(id) )", db, os).execute();
    SQLiteQuery("CREATE INDEX ModStep_object ON ModStep(object)", db, os).execute();
}

U2ModStep SQLiteModDbi::getModStep(qint64 id, U2OpStatus& os) {
    U2ModStep res;
    SQLiteQuery q("SELECT object, version, modType, details FROM ModStep WHERE id = ?1", db, os);
    CHECK_OP(os, res);

    q.bindInt64(1, id);
    if (q.step()) {
        res.id = id;
        res.objectId = q.getDataIdExt(0);
        res.version = q.getInt64(1);
        res.modType = q.getBlob(2);
        res.details = q.getBlob(3);
        q.ensureDone();
    }
    else if (!os.hasError()) {
        os.setError(SQLiteL10N::tr("An object modification step not found!"));
    }

    return res;
}

void SQLiteModDbi::createModStep(U2ModStep& step, U2OpStatus& os) {
    SQLiteQuery q("INSERT INTO ModStep(object, version, modType, details) VALUES(?1, ?2, ?3, ?4)", db, os);
    CHECK_OP(os, );

    q.bindDataId(1, step.objectId);
    q.bindInt64(2, step.version);
    q.bindString(3, step.modType);
    q.bindString(4, step.details);
    step.id = q.insert();
}

void SQLiteModDbi::removeObjectMods(const U2DataId& objectId, U2OpStatus& os) {
    SQLiteQuery q("DELETE FROM ModStep WHERE object = ?1", db, os);
    CHECK_OP(os, );

    q.bindDataId(1, objectId);
    q.execute();
}

} // namespace
