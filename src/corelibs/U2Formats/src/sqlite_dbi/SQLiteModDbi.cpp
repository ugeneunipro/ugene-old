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

    // SingleModStep - single modification of a dbi object
    //   id       - id of the modification
    //   object   - id of the object that was modified
    //   version  - this is a modification from 'version' to 'version + 1' of the object
    //   modType  - type of the object modification
    //   details  - detailed description of the object modification
    SQLiteQuery("CREATE TABLE SingleModStep (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
        " object INTEGER NOT NULL,"
        " otype INTEGER NOT NULL,"
        " oextra BLOB NOT NULL,"
        " version INTEGER NOT NULL,"
        " modType INTEGER NOT NULL,"
        " details TEXT NOT NULL,"
        " multiStepId INTEGER NOT NULL, "
        " FOREIGN KEY(object) REFERENCES Object(id), "
        " FOREIGN KEY(multiStepId) REFERENCES MultiModStep(id) )", db, os).execute();
    SQLiteQuery("CREATE INDEX SingleModStep_object ON SingleModStep(object)", db, os).execute();
    SQLiteQuery("CREATE INDEX SingleModStep_object_version ON SingleModStep(object, version)", db, os).execute();

    // MultiModStep - multiple modifications step with reference to a user modifications step
    //   id          - id of the multiple modifications step
    //   userStepId  - id of the user modifications step
    SQLiteQuery("CREATE TABLE MultiModStep (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
        " userStepId INTEGER NOT NULL,"
        " FOREIGN KEY(userStepId) REFERENCES UserModStep(id) )", db, os).execute();

    // UserModStep - user modification steps
    //   id          - id of the user modifications step
    SQLiteQuery("CREATE TABLE UserModStep (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
        " completeState INTEGER NOT NULL )", db, os).execute();
}

U2SingleModStep SQLiteModDbi::getModStep(const U2DataId& objectId, qint64 trackVersion, U2OpStatus& os) {
    U2SingleModStep res;
    SQLiteQuery q("SELECT id, object, otype, oextra, version, modType, details, multiStepId FROM SingleModStep WHERE object = ?1 AND version = ?2", db, os);
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
        os.setError(SQLiteL10N::tr("An object single modification step not found!"));
    }

    return res;
}

void SQLiteModDbi::createMultiModStep(U2MultiModStep& multiStep, U2OpStatus& os) {
    SQLiteTransaction t(db, os);

    // UserModStep
    SQLiteQuery qUser("INSERT INTO UserModStep(completeState) VALUES(?1)", db, os);
    SAFE_POINT_OP(os, );

    qUser.bindInt64(1, 1);
    qint64 userModStepId = qUser.insert();

    // MultiModSteps
    SQLiteQuery qMulti("INSERT INTO MultiModStep(userStepId) VALUES (?1)", db, os);
    SAFE_POINT_OP(os, );

    qMulti.bindInt64(1, userModStepId);
    qint64 multiModStepId = qMulti.insert();

    // SingleModSteps
    SQLiteQuery qSingle("INSERT INTO SingleModStep(object, otype, oextra, version, modType, details, multiStepId) VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7)", db, os);
    SAFE_POINT_OP(os, );

    for (int i = 0, n = multiStep.singleSteps.count(); i < n; ++i) {
        U2SingleModStep& singleModStep = multiStep.singleSteps[i];
        qSingle.reset();
        qSingle.bindInt64(1, multiModStepId);

        qSingle.bindDataId(1, singleModStep.objectId);
        qSingle.bindType(2, SQLiteUtils::toType(singleModStep.objectId));
        qSingle.bindBlob(3, SQLiteUtils::toDbExtra(singleModStep.objectId));
        qSingle.bindInt64(4, singleModStep.version);
        qSingle.bindInt64(5, singleModStep.modType);
        qSingle.bindString(6, singleModStep.details);
        qSingle.bindInt64(7, multiModStepId);

        singleModStep.id = qSingle.insert();
    }
}

void SQLiteModDbi::removeModsWithGreaterVersion(const U2DataId& objectId, qint64 version, U2OpStatus& os) {
    SQLiteQuery q("DELETE FROM SingleModStep WHERE object = ?1 AND version >= ?2", db, os);
    CHECK_OP(os, );

    q.bindDataId(1, objectId);
    q.bindInt64(2, version);
    q.execute();
}

void SQLiteModDbi::removeObjectMods(const U2DataId& objectId, U2OpStatus& os) {
    SQLiteQuery q("DELETE FROM SingleModStep WHERE object = ?1", db, os);
    CHECK_OP(os, );

    q.bindDataId(1, objectId);
    q.execute();
}

} // namespace
