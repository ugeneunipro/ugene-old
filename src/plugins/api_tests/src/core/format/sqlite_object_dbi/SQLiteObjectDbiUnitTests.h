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

#ifndef _U2_SQLITE_OBJECT_DBI_UNIT_TESTS_H_
#define _U2_SQLITE_OBJECT_DBI_UNIT_TESTS_H_

#include "core/dbi/DbiTest.h"

#include <unittest.h>


namespace U2 {

class SQLiteDbi;
class SQLiteObjectDbi;
class U2AttributeDbi;
class U2MsaDbi;
class U2SequenceDbi;

class SQLiteObjectDbiTestData {
public:
    static void init();

    static void shutdown();

    static SQLiteDbi* getSQLiteDbi();
    static SQLiteObjectDbi* getSQLiteObjectDbi();
    static U2AttributeDbi* getAttributeDbi();
    static U2MsaDbi* getMsaDbi();
    static U2SequenceDbi* getSequenceDbi();

private:
    static TestDbiProvider dbiProvider;
    static const QString& SQLITE_OBJ_DB_URL;
    static U2AttributeDbi* attributeDbi;
    static U2MsaDbi* msaDbi;
    static U2SequenceDbi* sequenceDbi;
    static SQLiteObjectDbi* sqliteObjectDbi;
    static SQLiteDbi* sqliteDbi;
};

/**
 * Remove a MSA object from the database, the following must be removed:
 *   1) Records in "Attribute" table (MSA info)
 *   2) Records in "StringAttribute"
 *   3) Records in "MsaRow"
 *   4) Records in "MsaRowGap"
 *   5) Records in "Sequence" (not used by other objects)
 *   6) The record in "Msa"
 *   7) The record in "Object"
 * Records of another MSA object are not removed.
 */
DECLARE_TEST(SQLiteObjectDbiUnitTests, removeMsaObject);

} // namespace

DECLARE_METATYPE(SQLiteObjectDbiUnitTests, removeMsaObject);

#endif
