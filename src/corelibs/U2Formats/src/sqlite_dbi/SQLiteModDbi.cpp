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

#include <QCoreApplication>

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SqlHelpers.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/SQLiteObjectDbi.h>


namespace U2 {

/************************************************************************/
/* U2UseCommonMultiModStep                                              */
/************************************************************************/
U2UseCommonMultiModStep::U2UseCommonMultiModStep(SQLiteDbi *_sqliteDbi, const U2DataId &_masterObjId, U2OpStatus& os)
: sqliteDbi(_sqliteDbi),
  valid(false)
{
    SAFE_POINT(NULL != sqliteDbi, "NULL sqliteDbi!", );
    QMutexLocker m(&sqliteDbi->getDbRef()->lock);

    sqliteDbi->getSQLiteModDbi()->startCommonMultiModStep(_masterObjId, os);
    if (!os.hasError()) {
        valid = true;
    }
}

U2UseCommonMultiModStep::~U2UseCommonMultiModStep() {
    SAFE_POINT(NULL != sqliteDbi, "NULL sqliteDbi!", );
    QMutexLocker m(&sqliteDbi->getDbRef()->lock);
    if (valid) {
        U2OpStatus2Log os;
        sqliteDbi->getSQLiteModDbi()->endCommonMultiModStep(os);
    }
}


/************************************************************************/
/* SQLiteModDbi                                                         */
/************************************************************************/
SQLiteModDbi::SQLiteModDbi(SQLiteDbi *dbi) : U2ModDbi(dbi), SQLiteChildDBICommon(dbi) {
}

void SQLiteModDbi::initSqlSchema(U2OpStatus &os) {
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
    //   version                - master object was modified from this version
    SQLiteQuery("CREATE TABLE UserModStep (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
        " object INTEGER NOT NULL,"
        " otype INTEGER NOT NULL,"
        " oextra BLOB NOT NULL,"
        " version INTEGER NOT NULL)", db, os).execute();
}

U2SingleModStep SQLiteModDbi::getModStep(const U2DataId &objectId, qint64 trackVersion, U2OpStatus &os) {
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

QList<U2SingleModStep> SQLiteModDbi::getModSteps(const U2DataId &masterObjId, qint64 version, U2OpStatus &os) {
    QList<U2SingleModStep> steps;
    SQLiteTransaction t(db, os);
    qint64 userStepId = -1;
    SQLiteQuery qGetUserStepId("SELECT id FROM UserModStep WHERE object = ?1 AND version = ?2", db, os);
    SAFE_POINT_OP(os, QList<U2SingleModStep>());

    qGetUserStepId.bindDataId(1, masterObjId);
    qGetUserStepId.bindInt64(2, version);

    if (qGetUserStepId.step()) {
        userStepId = qGetUserStepId.getInt64(0);
        qGetUserStepId.ensureDone();
    }
    else if (!os.hasError()) {
        os.setError("Failed to find user step ID!");
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

void SQLiteModDbi::createModStep(const U2DataId &masterObjId, U2SingleModStep &step, U2OpStatus &os) {
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
        endCommonMultiModStep(os);
    }
}

void SQLiteModDbi::removeModsWithGreaterVersion(const U2DataId &masterObjId, qint64 masterObjVersion, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    // Get user step IDs
    QList<qint64> userStepIds;
    SQLiteQuery qSelectUserSteps("SELECT id FROM UserModStep WHERE object = ?1 AND version >= ?2", db, os);
    SAFE_POINT_OP(os, );

    qSelectUserSteps.bindDataId(1, masterObjId);
    qSelectUserSteps.bindInt64(2, masterObjVersion);

    while (qSelectUserSteps.step()) {
        qint64 userStepId = qSelectUserSteps.getInt64(0);
        userStepIds.append(userStepId);
    }
    SAFE_POINT_OP(os, );

    // Remove all affected steps (user, multi, single)
    removeSteps(userStepIds, os);
    SAFE_POINT_OP(os, );
}

void SQLiteModDbi::removeSteps(QList<qint64> userStepIds, U2OpStatus &os) {
    if (userStepIds.isEmpty()) {
        return;
    }

    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    // Get multiple steps IDs
    QList<qint64> multiStepIds;
    SQLiteQuery qSelectMultiSteps("SELECT id FROM MultiModStep WHERE userStepId = ?1", db, os);
    SAFE_POINT_OP(os, );
    foreach (qint64 userStepId, userStepIds) {
        qSelectMultiSteps.reset();
        qSelectMultiSteps.bindInt64(1, userStepId);

        while (qSelectMultiSteps.step()) {
            qint64 multiStepId = qSelectMultiSteps.getInt64(0);
            multiStepIds.append(multiStepId);
        }
    }

    // Remove single steps
    SQLiteQuery qDeleteSingleSteps("DELETE FROM SingleModStep WHERE multiStepId = ?1", db, os);
    SAFE_POINT_OP(os, );
    foreach (qint64 multiStepId, multiStepIds) {
        qDeleteSingleSteps.reset();
        qDeleteSingleSteps.bindInt64(1, multiStepId);
        qDeleteSingleSteps.execute();
    }
    SAFE_POINT_OP(os, );

    // Remove multiple steps
    SQLiteQuery qDeleteMultiSteps("DELETE FROM MultiModStep WHERE id = ?1", db, os);
    SAFE_POINT_OP(os, );
    foreach (qint64 multiStepId, multiStepIds) {
        qDeleteMultiSteps.reset();
        qDeleteMultiSteps.bindInt64(1, multiStepId);
        qDeleteMultiSteps.execute();
    }

    // Remove user steps
    SQLiteQuery qDeleteUserSteps("DELETE FROM UserModStep WHERE id = ?1", db, os);
    foreach (qint64 userStepId, userStepIds) {
        qDeleteUserSteps.reset();
        qDeleteUserSteps.bindInt64(1, userStepId);
        qDeleteUserSteps.execute();
    }
}

void SQLiteModDbi::removeObjectMods(const U2DataId &objectId, U2OpStatus &os) {
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    // Get user step IDs
    QList<qint64> userStepIds;
    SQLiteQuery qSelectUserSteps("SELECT id FROM UserModStep WHERE object = ?1", db, os);
    SAFE_POINT_OP(os, );

    qSelectUserSteps.bindDataId(1, objectId);

    while (qSelectUserSteps.step()) {
        qint64 userStepId = qSelectUserSteps.getInt64(0);
        userStepIds.append(userStepId);
    }
    SAFE_POINT_OP(os, );

    // Remove all affected steps (user, multi, single)
    removeSteps(userStepIds, os);
    SAFE_POINT_OP(os, );
}

void SQLiteModDbi::cleanUpAllStepsOnError() {
    U2OpStatus2Log os;
    SQLiteTransaction t(db, os);
    Q_UNUSED(t);

    SQLiteQuery("DELETE FROM SingleModStep", db, os).execute();
    SQLiteQuery("DELETE FROM MultiModStep", db, os).execute();
    SQLiteQuery("DELETE FROM UserModStep", db, os).execute();
}

qint64 SQLiteModDbi::currentUserModStepId = -1;
qint64 SQLiteModDbi::currentMultiModStepId = -1;
U2DataId SQLiteModDbi::currentMasterObjId;
bool SQLiteModDbi::removeUserStepWithMulti = true;

static void checkMainThread(U2OpStatus &os) {
    QThread *mainThread = QCoreApplication::instance()->thread();
    QThread *thisThread = QThread::currentThread();

    if (mainThread != thisThread) {
        os.setError("Not main thread");
    }
}

void SQLiteModDbi::startCommonUserModStep(const U2DataId &masterObjId, U2OpStatus &os) {
    checkMainThread(os);
    CHECK_OP(os, );
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

void SQLiteModDbi::endCommonUserModStep(U2OpStatus &os) {
    checkMainThread(os);
    CHECK_OP(os, );
    currentMultiModStepId = -1;
    currentUserModStepId = -1;
    currentMasterObjId = U2DataId();
}

void SQLiteModDbi::startCommonMultiModStep(const U2DataId &userMasterObjId, U2OpStatus &os) {
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
        U2OpStatus2Log innerOs;
        endCommonUserModStep(innerOs);
        return;
    }

    // Create a new multiple modifications step in the database
    createMultiModStep(os);
    SAFE_POINT_OP(os, );
}

void SQLiteModDbi::endCommonMultiModStep(U2OpStatus &os) {
    if (removeUserStepWithMulti) {
        endCommonUserModStep(os);
    }
    else {
        currentMultiModStepId = -1;
    }
}


void SQLiteModDbi::createUserModStep(const U2DataId &masterObjId, U2OpStatus &os) {
    qint64 masterObjVersion = dbi->getSQLiteObjectDbi()->getObjectVersion(masterObjId, os);
    SAFE_POINT_OP(os, );

    SQLiteQuery qUser("INSERT INTO UserModStep(object, otype, oextra, version) VALUES(?1, ?2, ?3, ?4)", db, os);
    SAFE_POINT_OP(os, );

    qUser.bindDataId(1, masterObjId);
    qUser.bindType(2, SQLiteUtils::toType(masterObjId));
    qUser.bindBlob(3, SQLiteUtils::toDbExtra(masterObjId));
    qUser.bindInt64(4, masterObjVersion);

    currentUserModStepId = qUser.insert();
    if (-1 == currentUserModStepId) {
        os.setError("Failed to create a common user modifications step!");
        return;
    }
}

void SQLiteModDbi::createMultiModStep(U2OpStatus &os) {
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

bool SQLiteModDbi::canUndo(const U2DataId &objectId, U2OpStatus &os) {
    SQLiteTransaction t(db, os);

    // Get current object version
    qint64 objVersion = dbi->getSQLiteObjectDbi()->getObjectVersion(objectId, os);
    SAFE_POINT_OP(os, false);

    // Verify if there are steps
    SQLiteQuery q("SELECT id FROM UserModStep WHERE object = ?1 AND version < ?2", db, os);
    SAFE_POINT_OP(os, false);

    q.bindDataId(1, objectId);
    q.bindInt64(2, objVersion);

    if (q.step()) {
        return true;
    }
    
    return false;
}

bool SQLiteModDbi::canRedo(const U2DataId &objectId, U2OpStatus &os) {
    SQLiteTransaction t(db, os);

    // Get current object version
    qint64 objVersion = dbi->getSQLiteObjectDbi()->getObjectVersion(objectId, os);
    SAFE_POINT_OP(os, false);

    // Verify if there are steps
    SQLiteQuery q("SELECT id FROM UserModStep WHERE object = ?1 AND version >= ?2", db, os);
    SAFE_POINT_OP(os, false);

    q.bindDataId(1, objectId);
    q.bindInt64(2, objVersion);

    if (q.step()) {
        return true;
    }

    return false;
}

} // namespace
