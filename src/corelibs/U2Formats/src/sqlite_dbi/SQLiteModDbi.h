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

#ifndef _U2_SQLITE_MOD_DBI_H_
#define _U2_SQLITE_MOD_DBI_H_

#include "SQLiteDbi.h"

#include <U2Core/U2AbstractDbi.h>


namespace U2 {

class U2FORMATS_EXPORT U2UseCommonMultiModStep {
public:
    /**
     * Master object ID refers to the user modifications step.
     * WARNING!: If a user step has been already created, the master object must be the same!
     */
    U2UseCommonMultiModStep(SQLiteDbi* _sqliteDbi, const U2DataId& _masterObjId, U2OpStatus& os);
    ~U2UseCommonMultiModStep();
private:
    SQLiteDbi* sqliteDbi;
    bool valid;
};


class U2FORMATS_EXPORT SQLiteModDbi : public U2ModDbi, public SQLiteChildDBICommon {
public:
    SQLiteModDbi(SQLiteDbi* dbi);

    /** Creates all required tables */
    virtual void initSqlSchema(U2OpStatus& os);

    /** Returns a modification step with the specified version for the object */
    virtual U2SingleModStep getModStep(const U2DataId& objectId, qint64 trackVersion, U2OpStatus& os);

    /** Returns all single steps of the master object ID of the specified version */
    QList<U2SingleModStep> getModSteps(const U2DataId& masterObjId, qint64 version, U2OpStatus& os);

    /**
     * Adds a modification step into the database.
     * The step must contain valid object ID and info about modifications,
     * the method sets correct ID and multiStepId for the step.
     */
    void createModStep(const U2DataId& masterObjId, U2SingleModStep& step, U2OpStatus& os);

    /**
     * Removes user steps for the object with version EQUAL or GREATER than the specified version.
     * Removes all multi and single steps of the user steps.
     */
    void removeModsWithGreaterVersion(const U2DataId &masterObjId, qint64 masterObjVersion, U2OpStatus &os);

    /** Removes all modification tracks and steps for the object */
    virtual void removeObjectMods(const U2DataId& objectId, U2OpStatus& os);

    /**
     * Starts a common user modifications step.
     * Do not use this method, create a "U2UseCommonUserModStep" instance instead!
     */
    virtual void startCommonUserModStep(const U2DataId& masterObjId, U2OpStatus& os);

    /**
     * Ends a common user modifications step.
     * Do not use this method, use "U2UseCommonUserModStep" instead!
     */
    virtual void endCommonUserModStep(U2OpStatus &os);

    /**
     * Starts a common multiple modifications step.
     * If there is a common user modifications step, verifies that the passed object ID is the same. 
     * Creates a common user modifications step, if it doesn't exist.
     * Do not use this method, create a "U2UseCommonMultiModStep" instance instead!
     */
    void startCommonMultiModStep(const U2DataId& userMasterObjId, U2OpStatus& os);

    /**
     * Ends a common multiple modifications step.
     * Do not use this method, use "U2UseCommonMultiModStep" instead!
     */
    void endCommonMultiModStep(U2OpStatus &os);

    /** Specifies whether a step has been started */
    static bool isUserStepStarted() { return currentUserModStepId != -1; }
    static bool isMultiStepStarted() { return currentMultiModStepId != -1; }

    /** Specifies whether there are user steps that can be undone/redone */
    bool canUndo(const U2DataId& objectId, U2OpStatus& os);
    bool canRedo(const U2DataId& objectId, U2OpStatus& os);

private:
    /**
     * Create a record in the UserModStep table.
     * Sets "currentUserModStepId" to the added record ID.
     */
    void createUserModStep(const U2DataId& masterObjId, U2OpStatus& os);

    /**
     * Creates a record in the MultiModStep table.
     * Sets "currentMultiModStepId" to the added record ID.
     * Warning: it is assumed that a user modification step has been started!
     */
    void createMultiModStep(U2OpStatus& os);

    /** User modifications step ID if it has been started, or -1 otherwise */
    static qint64 currentUserModStepId;

    /** Multiple modifications step ID if it has been started, or -1  otherwise */
    static qint64 currentMultiModStepId;

    /** ID of the master object of the current user modifications step */
    static U2DataId currentMasterObjId;

    /** Specifies whether user step was created for multiple step only */
    static bool removeUserStepWithMulti;
};

} // namespace

#endif
