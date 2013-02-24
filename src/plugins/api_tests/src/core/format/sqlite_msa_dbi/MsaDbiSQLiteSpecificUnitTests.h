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

    static qint64 getModStepsNum(const U2DataId& objId, U2OpStatus& os);

    static U2DataId createTestMsa(bool enableModTracking, U2OpStatus& os);
    static U2DataId createNotSoSmallTestMsa(bool enableModTracking, U2OpStatus& os);

    static const QString TEST_MSA_NAME;

private:
    static U2MsaRow addRow(const U2DataId &msaId, const QByteArray &name, const QByteArray &seq, const QList<U2MsaGap> &gaps, U2OpStatus &os);

    static TestDbiProvider dbiProvider;
    static const QString& SQLITE_MSA_DB_URL;
    static SQLiteDbi* sqliteDbi;
};

///////////////////////////////////////////////////////////////
// Tests for U2MsaDbi API that assume that SQLiteDbi was used.
// The tests verify the correct structure of the SQLiteDbi
// after some operations.
///////////////////////////////////////////////////////////////


/**
 * Update a MSA name.
 *   ^ noModTrack - modifications tracking is not enabled for the MSA.
 *   ^ undo       - modifications tracking is enabled, check the "undo" operation.
 *   ^ redo       - modifications tracking is enabled, check the "redo" operation.
 *   ^ severalSteps - modification tracking is enabled, check several "undo/redo" steps.
 */
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaName_noModTrack);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaName_undo);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaName_redo);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaName_severalSteps);

/** Update a MSA alphabet */
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaAlphabet_noModTrack);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaAlphabet_undo);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaAlphabet_redo);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaAlphabet_severalSteps);

/** Update gap model */
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateGapModel_noModTrack);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateGapModel_undo);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateGapModel_redo);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateGapModel_severalSteps);

/** Update row content */
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowContent_noModTrack);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowContent_undo);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowContent_redo);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowContent_severalSteps);

/**
 * Set new row order of the MSA.
 *   ^ noModTrack - modifications tracking is not enabled for the MSA.
 *   ^ undo       - modifications tracking is enabled, check the "undo" operation.
 *   ^ redo       - modifications tracking is enabled, check the "redo" operation.
 *   ^ severalSteps - modification tracking is enabled, check several "undo/redo" steps.
 */
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, setNewRowsOrder_noModTrack);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, setNewRowsOrder_undo);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, setNewRowsOrder_redo);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, setNewRowsOrder_severalSteps);

/** Update row name */
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowName_noModTrack);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowName_undo);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowName_redo);
DECLARE_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowName_severalSteps);

} // namespace

DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateMsaName_noModTrack);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateMsaName_undo);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateMsaName_redo);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateMsaName_severalSteps);

DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateMsaAlphabet_noModTrack);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateMsaAlphabet_undo);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateMsaAlphabet_redo);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateMsaAlphabet_severalSteps);

DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateGapModel_noModTrack);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateGapModel_undo);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateGapModel_redo);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateGapModel_severalSteps);

DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateRowContent_noModTrack);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateRowContent_undo);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateRowContent_redo);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateRowContent_severalSteps);

DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, setNewRowsOrder_noModTrack);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, setNewRowsOrder_undo);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, setNewRowsOrder_redo);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, setNewRowsOrder_severalSteps);

DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateRowName_noModTrack);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateRowName_undo);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateRowName_redo);
DECLARE_METATYPE(MsaDbiSQLiteSpecificUnitTests, updateRowName_severalSteps);

#endif
