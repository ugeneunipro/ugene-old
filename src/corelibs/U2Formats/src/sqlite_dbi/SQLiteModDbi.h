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

class SQLiteModDbi : public U2ModDbi, public SQLiteChildDBICommon {
public:
    SQLiteModDbi(SQLiteDbi* dbi);

    /** Creates all required tables */
    virtual void initSqlSchema(U2OpStatus& os);

    /** Returns a modification step with the specified version for the object */
    virtual U2ModStep getModStep(const U2DataId& objectId, qint64 trackVersion, U2OpStatus& os);

    /**
     * Adds a record about a single object modification.
     * Parameter 'step' must contain valid object ID and info about the modification,
     * the parameter is modified to contain ID of the added record.
     */
    virtual void createModStep(U2ModStep& step, U2OpStatus& os);

    /** Removes modification steps for the object with version EQUAL or GREATER than the specified version */
    virtual void removeModsWithGreaterVersion(const U2DataId& objectId, qint64 version, U2OpStatus& os);

    /** Removes all modification tracks and steps for the object */
    virtual void removeObjectMods(const U2DataId& objectId, U2OpStatus& os);
};

} // namespace

#endif
