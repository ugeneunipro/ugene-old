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

#ifndef _U2_MOD_DBI_SQLITE_SPECIFIC_UNIT_TESTS_H_
#define _U2_MOD_DBI_SQLITE_SPECIFIC_UNIT_TESTS_H_

#include "core/dbi/DbiTest.h"

#include <unittest.h>


namespace U2 {

class SQLiteDbi;
class U2MsaDbi;
class U2SequenceDbi;

class ModSQLiteSpecificTestData {
public:
    static void init();
    static void shutdown();

    static SQLiteDbi* getSQLiteDbi();

    static qint64 getModStepsNum(const U2DataId& objId, U2OpStatus& os);
    static U2ModStep getLastModStep(const U2DataId& objId, U2OpStatus& os);
    static QList<U2ModStep> getAllModSteps(const U2DataId& objId, U2OpStatus& os);

    static U2DataId createTestMsa(bool enableModTracking, U2OpStatus& os);

    static const QString TEST_MSA_NAME;

private:
    static U2MsaRow addRow(const U2DataId &msaId, const QByteArray &name, const QByteArray &seq, const QList<U2MsaGap> &gaps, U2OpStatus &os);

    static TestDbiProvider dbiProvider;
    static const QString& SQLITE_MSA_DB_URL;
    static SQLiteDbi* sqliteDbi;
};

///////////////////////////////////////////////////////////////
// Tests for U2ModDbi that assume that SQLiteDbi was used.
// The tests verify the correct structure of the SQLiteDbi
// after some operations (undo/redo).
///////////////////////////////////////////////////////////////


/**
 * Some updates of the MSA.
 *   ^ noModTrack               - modifications tracking is not enabled for the MSA.
 *   ^ severalSteps             - modifications tracking is enabled, check several "undo/redo" steps.
 *   ^ severalUndoThenAction    - modifications tracking is enabled, check several "undo" steps, then one action.
 */
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateMsaName_noModTrack);
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateMsaName_severalSteps);
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateMsaName_severalUndoThenAction);

/** Update a MSA alphabet */
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateMsaAlphabet_noModTrack);
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateMsaAlphabet_severalSteps);
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateMsaAlphabet_severalUndoThenAction);

/** Update gap model */
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateGapModel_noModTrack);
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateGapModel_severalSteps);
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateGapModel_severalUndoThenAction);

/** Update row content */
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateRowContent_noModTrack);
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateRowContent_severalSteps);
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateRowContent_severalUndoThenAction);

/** Set new row order of the MSA */
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, setNewRowsOrder_noModTrack);
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, setNewRowsOrder_severalSteps);
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, setNewRowsOrder_severalUndoThenAction);

/** Update row name */
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateRowName_noModTrack);
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateRowName_severalSteps);
DECLARE_TEST(ModDbiSQLiteSpecificUnitTests, updateRowName_severalUndoThenAction);

} // namespace

DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateMsaName_noModTrack);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateMsaName_severalSteps);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateMsaName_severalUndoThenAction);

DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateMsaAlphabet_noModTrack);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateMsaAlphabet_severalSteps);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateMsaAlphabet_severalUndoThenAction);

DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateGapModel_noModTrack);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateGapModel_severalSteps);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateGapModel_severalUndoThenAction);

DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateRowContent_noModTrack);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateRowContent_severalSteps);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateRowContent_severalUndoThenAction);

DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, setNewRowsOrder_noModTrack);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, setNewRowsOrder_severalSteps);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, setNewRowsOrder_severalUndoThenAction);

DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateRowName_noModTrack);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateRowName_severalSteps);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, updateRowName_severalUndoThenAction);

#endif
