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


/** Cleans up all modifications steps on set up */
#define DECLARE_MOD_TEST(suite, name) \
class TEST_CLASS(suite, name) : public UnitTest { \
public: \
    virtual void Test(); \
    virtual void SetUp(); \
}


namespace U2 {

class SQLiteDbi;
class U2MsaDbi;
class U2SequenceDbi;

class U2MultiModStep4Test {
public:
    U2MultiModStep4Test() : id(-1), userStepId(-1) {}
    U2MultiModStep4Test(qint64 _id, qint64 _userStepId) : id(_id), userStepId(_userStepId) {}
    qint64      id;
    qint64      userStepId;
};

class U2UserModStep4Test {
public:
    U2UserModStep4Test() : id(-1), masterObjId("") {}
    U2UserModStep4Test(qint64 _id, const U2DataId& _masterObjId) : id(_id), masterObjId(_masterObjId) {}
    qint64      id;
    U2DataId    masterObjId;
};


class ModSQLiteSpecificTestData {
public:
    static void init();
    static void shutdown();

    static SQLiteDbi* getSQLiteDbi();

    static void getAllSteps(QList<U2SingleModStep>& singleSteps,
        QList<U2MultiModStep4Test>& multiSteps,
        QList<U2UserModStep4Test>& userSteps,
        U2OpStatus& os);

    static qint64 getModStepsNum(const U2DataId& objId, U2OpStatus& os);
    static U2SingleModStep getLastModStep(const U2DataId& objId, U2OpStatus& os);

    static QList<U2SingleModStep> getAllModSteps(const U2DataId& objId, U2OpStatus& os);

    static U2SingleModStep prepareSingleStep(qint64 modVersion, U2OpStatus& os);

    static U2DataId createObject(U2OpStatus& os);

    static U2DataId createTestMsa(bool enableModTracking, U2OpStatus& os);

    static const QString TEST_MSA_NAME;

    static void cleanUpAllModSteps();

private:
    static U2MsaRow addRow(const U2DataId &msaId, const QByteArray &name, const QByteArray &seq, const QList<U2MsaGap> &gaps, U2OpStatus &os);

    static TestDbiProvider dbiProvider;
    static const QString& SQLITE_MSA_DB_URL;
    static SQLiteDbi* sqliteDbi;
};


/**
 * Verify creation of a single, multiple and user modifications steps.
 *   ^ noMultiAndUser       - creates a step when there are no common multi and user modifications steps,
 *                            created multi and user steps must be ended after creation of the single step.
 *   ^ noMultiAndUser2Steps - creates 2 steps one after another, multi and user step is created for each.
 *   ^ startMulti           - creates 2 single steps with one multi, "U2UseCommonMultiModStep" is used to start/end multi step
 *   ^ start2MultiNoUser    - creates 2 single steps in one multi, and one more single step in another multi,
                              there must be 2 different user steps.
 *   ^ startUser            - create a user step using "U2UseCommonUserStep"
 *   ^ oneUser2Multi        -
 *   ^ severalUser          -
 */
DECLARE_MOD_TEST(ModDbiSQLiteSpecificUnitTests, createStep_noMultiAndUser);
DECLARE_MOD_TEST(ModDbiSQLiteSpecificUnitTests, createStep_noMultiAndUser2Steps);
DECLARE_MOD_TEST(ModDbiSQLiteSpecificUnitTests, createStep_startMulti);
DECLARE_MOD_TEST(ModDbiSQLiteSpecificUnitTests, createStep_start2MultiNoUser);
DECLARE_MOD_TEST(ModDbiSQLiteSpecificUnitTests, createStep_startUser);
DECLARE_MOD_TEST(ModDbiSQLiteSpecificUnitTests, createStep_oneUser2Multi);
DECLARE_MOD_TEST(ModDbiSQLiteSpecificUnitTests, createStep_severalUser);
DECLARE_MOD_TEST(ModDbiSQLiteSpecificUnitTests, createStep_separateThread);



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

DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, createStep_noMultiAndUser);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, createStep_noMultiAndUser2Steps);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, createStep_startMulti);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, createStep_start2MultiNoUser);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, createStep_startUser);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, createStep_oneUser2Multi);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, createStep_severalUser);
DECLARE_METATYPE(ModDbiSQLiteSpecificUnitTests, createStep_separateThread);





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
