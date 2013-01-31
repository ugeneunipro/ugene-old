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

#ifndef _U2_MSA_DBI_SQLITE_SPECIFIC_UNIT_TESTS_H_
#define _U2_MSA_DBI_SQLITE_SPECIFIC_UNIT_TESTS_H_

#include "core/dbi/DbiTest.h"

#include <unittest.h>


namespace U2 {

class SQLiteDbi;
class U2MsaDbi;
class U2SequenceDbi;

class MsaSQLiteSpecificTestData {
public:
    static void init();

    static void shutdown();

    static SQLiteDbi* getSQLiteDbi();
    static U2MsaDbi* getMsaDbi();
    static U2SequenceDbi* getSequenceDbi();

private:
    static TestDbiProvider dbiProvider;
    static const QString& MSA_SQLITE_DB_URL;
    static U2MsaDbi* msaDbi;
    static U2SequenceDbi* sequenceDbi;
    static SQLiteDbi* sqliteDbi;
};

///////////////////////////////////////////////////////////////
// Tests for U2MsaDbi API that assume that SQLiteDbi was used.
// The tests verify the correct structure of the SQLiteDbi
// after some operations.
///////////////////////////////////////////////////////////////

/**
 * Remove a row from a MSA.
 * Verifies SQLiteDbi structure after a row was deleted:
 *   1) No gaps for the row
 *   2) The sequence was deleted
 *   3) The sequence object was deleted
 */
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, removeRow);

} // namespace


DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, removeRow);


#endif
