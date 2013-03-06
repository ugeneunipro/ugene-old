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

/************************************************************************/
/* U2UseCommonMultiModStep                                              */
/************************************************************************/
U2UseCommonMultiModStep::U2UseCommonMultiModStep(SQLiteDbi* _sqliteDbi, const U2DataId& _masterObjId, U2OpStatus& os)
: sqliteDbi(_sqliteDbi),
  valid(false)
{
    SAFE_POINT(NULL != sqliteDbi, "NULL sqliteDbi!", );

    sqliteDbi->getSQLiteModDbi()->startCommonMultiModStep(_masterObjId, os);
    if (!os.hasError()) {
        valid = true;
    }
}

U2UseCommonMultiModStep::~U2UseCommonMultiModStep() {
    if (valid) {
        sqliteDbi->getSQLiteModDbi()->endCommonMultiModStep();
    }
}


/************************************************************************/
/* SQLiteModDbi                                                         */
/************************************************************************/
SQLiteModDbi::SQLiteModDbi(SQLiteDbi* dbi) : U2ModDbi(dbi), SQLiteChildDBICommon(dbi) {
}

void SQLiteModDbi::initSqlSchema(U2OpStatus& os) {
    if (os.hasError()) {
        return;
    }

    // SingleModStep - single modification of a dbi object
    //   id                    - id of the modification
    //   object, otype, oextra - data id of the object that was modified
    //   version               - this is a modification from 'version' to 'version + 1' of the object
    //   modType               - type of the object modification
    //   details               - detailed description of the object modification
    //   multiStepId           - id of the multiModStep
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
    //   id                     - id of the user modifications step
    //   object, otype, oextra  - data id of the master object (i.e. object for which "undo/redo" was initiated)
    SQLiteQuery("CREATE TABLE UserModStep (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
        " object INTEGER NOT NULL,"
        " otype INTEGER NOT NULL,"
        " oextra BLOB NOT NULL)", db, os).execute();
}

U2SingleModStep SQLiteModDbi::getModStep(const U2DataId& objectId, qint64 trackVersion, U2OpStatus& os) {
    U2SingleModStep res;
    SQLiteQuery q("SELECT id, object, otype, oextra, version, modType, details, multiStepId FROM SingleModStep WHERE object = ?1 AND version = ?2 ORDER BY id", db, os);
    SAFE_POINT_OP(os, res);

    q.bindDataId(1, objectId);
    q.bindInt64(2, trackVersion);

    if (q.step()) {
        res.id = q.getInt64(0);
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

QList<U2SingleModStep> SQLiteModDbi::getModSteps(const U2DataId& objectId, qint64 version, U2OpStatus& os) {
    QList<U2SingleModStep> steps;
    SQLiteTransaction t(db, os);
    qint64 userStepId = -1;
    SQLiteQuery qGetUserStepId("SELECT userStepId FROM MultiModStep WHERE id = (SELECT multiStepId FROM SingleModStep WHERE object = ?1 AND version = ?2)", db, os);
    SAFE_POINT_OP(os, QList<U2SingleModStep>());

    qGetUserStepId.bindDataId(1, objectId);
    qGetUserStepId.bindInt64(2, version);

    if (qGetUserStepId.step()) {
        userStepId = qGetUserStepId.getInt64(0);
        qGetUserStepId.ensureDone();
    }
    else if (!os.hasError()) {
        os.setError("Failed to find user step ID by single step ID!");
        return steps;
    }

    SQLiteQuery qMultiStepId("SELECT id FROM MultiModStep WHERE userStepId = ?1", db, os);
    qMultiStepId.bindInt64(1, userStepId);

    SQLiteQuery qSingleStep("SELECT id, object, otype, oextra, version, modType, details, multiStepId FROM SingleModStep WHERE multiStepId = ?1", db, os);
    while (qMultiStepId.step()) {
        qint64 multiStepId = qMultiStepId.getInt64(0);

        qSingleStep.reset();
        qSingleStep.bindInt64(1, multiStepId);

        while (qSingleStep.step()) {
            U2SingleModStep step;
            step.id = qSingleStep.getInt64(0);
            step.objectId = qSingleStep.getDataIdExt(1);
            step.version = qSingleStep.getInt64(4);
            step.modType = qSingleStep.getInt64(5);
            step.details = qSingleStep.getBlob(6);

            SAFE_POINT_OP(os, QList<U2SingleModStep>());
            steps.append(step);
        }
    }
    return steps;
}

void SQLiteModDbi::createModStep(const U2DataId& masterObjId, U2SingleModStep& step, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    bool closeMultiStep = false;
    if (!isMultiStepStarted()) {
        startCommonMultiModStep(masterObjId, os);
        SAFE_POINT_OP(os, );
        SAFE_POINT(isMultiStepStarted(), "A multiple modifications step must have been started!", );
        closeMultiStep = true;
    }
    
    SQLiteQuery qSingle("INSERT INTO SingleModStep(object, otype, oextra, version, modType, details, multiStepId) VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7) ", db, os);
    SAFE_POINT_OP(os, );

    qSingle.bindDataId(1, step.objectId);
    qSingle.bindType(2, SQLiteUtils::toType(step.objectId));
    qSingle.bindBlob(3, SQLiteUtils::toDbExtra(step.objectId));
    qSingle.bindInt64(4, step.version);
    qSingle.bindInt64(5, step.modType);
    qSingle.bindString(6, step.details);
    qSingle.bindInt64(7, currentMultiModStepId);

    step.id = qSingle.insert();
    step.multiStepId = currentMultiModStepId;

    if (closeMultiStep) {
        endCommonMultiModStep();
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

qint64 SQLiteModDbi::currentUserModStepId = -1;
qint64 SQLiteModDbi::currentMultiModStepId = -1;
U2DataId SQLiteModDbi::currentMasterObjId;
bool SQLiteModDbi::removeUserStepWithMulti = true;


void SQLiteModDbi::startCommonUserModStep(const U2DataId& masterObjId, U2OpStatus& os) {
    SQLiteTransaction t(db, os);

    // Only one common step at a time
    if (isUserStepStarted()) {
        os.setError("Can't create a common user modifications step, previous one is not complete!");
        return;
    }

    // Create a new user modifications step in the database
    createUserModStep(masterObjId, os);
    SAFE_POINT_OP(os, );

    currentMasterObjId = masterObjId;
}

void SQLiteModDbi::endCommonUserModStep() {
    currentMultiModStepId = -1;
    currentUserModStepId = -1;
    currentMasterObjId = U2DataId();
}

void SQLiteModDbi::startCommonMultiModStep(const U2DataId& userMasterObjId, U2OpStatus& os) {
    SQLiteTransaction t(db, os);
    if (!isUserStepStarted()) {
        startCommonUserModStep(userMasterObjId, os);
        SAFE_POINT_OP(os, );
        SAFE_POINT(isUserStepStarted(), "A user modifications step must have been started!", );
        removeUserStepWithMulti = true;
    }
    else {
        removeUserStepWithMulti = false;

        // Verify that object ID of the user mod step is the same
        SAFE_POINT(currentMasterObjId == userMasterObjId, "Incorrect user step master object ID!", );
    }

    
    if (isMultiStepStarted()) {
        os.setError("Can't create a common multiple modifications step, previous one is not complete!");
        endCommonUserModStep();
        return;
    }

    // Create a new multiple modifications step in the database
    createMultiModStep(os);
    SAFE_POINT_OP(os, );
}

void SQLiteModDbi::endCommonMultiModStep() {
    if (removeUserStepWithMulti) {
        endCommonUserModStep();
    }
    else {
        currentMultiModStepId = -1;
    }
}


void SQLiteModDbi::createUserModStep(const U2DataId& masterObjId, U2OpStatus& os) {
    SQLiteQuery qUser("INSERT INTO UserModStep(object, otype, oextra) VALUES(?1, ?2, ?3)", db, os);
    SAFE_POINT_OP(os, );

    qUser.bindDataId(1, masterObjId);
    qUser.bindType(2, SQLiteUtils::toType(masterObjId));
    qUser.bindBlob(3, SQLiteUtils::toDbExtra(masterObjId));

    currentUserModStepId = qUser.insert();
    if (-1 == currentUserModStepId) {
        os.setError("Failed to create a common user modifications step!");
        return;
    }
}

void SQLiteModDbi::createMultiModStep(U2OpStatus& os) {
    SAFE_POINT(isUserStepStarted(), "A user modifications step must have been started!", );

    SQLiteQuery qMulti("INSERT INTO MultiModStep(userStepId) VALUES(?1)", db, os);
    SAFE_POINT_OP(os, );

    qMulti.bindInt64(1, currentUserModStepId);

    currentMultiModStepId = qMulti.insert();

    if (-1 == currentMultiModStepId) {
        os.setError("Failed to create a common multiple modifications step!");
        return;
    }
}

} // namespace
