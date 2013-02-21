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

#include "MsaDbiSQLiteSpecificUnitTests.h"
#include "core/util/MsaDbiUtilsUnitTests.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Formats/SQLiteDbi.h>
#include <U2Formats/SQLiteObjectDbi.h>


namespace U2 {

TestDbiProvider MsaSQLiteSpecificTestData::dbiProvider = TestDbiProvider();
const QString& MsaSQLiteSpecificTestData::SQLITE_MSA_DB_URL("sqlite-msa-dbi.ugenedb");
SQLiteDbi* MsaSQLiteSpecificTestData::sqliteDbi = NULL;

const QString MsaSQLiteSpecificTestData::TEST_MSA_NAME = "Test alignment";

void MsaSQLiteSpecificTestData::init() {
    SAFE_POINT(NULL == sqliteDbi, "sqliteDbi has already been initialized!", );

    // Get URL
    bool ok = dbiProvider.init(SQLITE_MSA_DB_URL, false);
    SAFE_POINT(ok, "Dbi provider failed to initialize!",);

    U2Dbi* dbi = dbiProvider.getDbi();
    QString url = dbi->getDbiRef().dbiId;
    dbiProvider.close();

    // Init DBI
    sqliteDbi = new SQLiteDbi();
    QHash<QString, QString> initProperties;
    initProperties[U2_DBI_OPTION_URL] = url;
    U2OpStatusImpl os;
    sqliteDbi->init(initProperties, QVariantMap(), os);
    SAFE_POINT_OP(os, );

    // Get msa IDs
    QList<U2DataId> ids = sqliteDbi->getObjectDbi()->getObjects(U2Type::Msa, 0, U2_DBI_NO_LIMIT, os);
    SAFE_POINT_OP(os,);
}

void MsaSQLiteSpecificTestData::shutdown() {
    if (NULL != sqliteDbi) {
        U2OpStatusImpl os;
        sqliteDbi->shutdown(os);
        SAFE_POINT_OP(os, );
        delete sqliteDbi;
        sqliteDbi = NULL;
    }
}

SQLiteDbi* MsaSQLiteSpecificTestData::getSQLiteDbi() {
    if (NULL == sqliteDbi) {
        init();
    }
    return sqliteDbi;
}

qint64 MsaSQLiteSpecificTestData::getModStepsNum(const U2DataId& objId, U2OpStatus& os) {
    SQLiteQuery qModSteps("SELECT COUNT(*) FROM ModStep WHERE object = ?1", sqliteDbi->getDbRef(), os);
    qModSteps.bindDataId(1, objId);
    return qModSteps.selectInt64();
}

U2MsaRow MsaSQLiteSpecificTestData::addRow(const U2DataId &msaId, const QByteArray &name, const QByteArray &seq, const QList<U2MsaGap> &gaps, U2OpStatus &os) {
    U2Sequence sequence;
    sequence.alphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
    sequence.visualName = name;
    sqliteDbi->getSequenceDbi()->createSequenceObject(sequence, "", os);
    CHECK_OP(os, U2MsaRow());

    U2Region reg(0, 0);
    sqliteDbi->getSequenceDbi()->updateSequenceData(sequence.id, reg, seq, QVariantMap(), os);
    CHECK_OP(os, U2MsaRow());

    U2MsaRow row;
    row.rowId = -1;
    row.sequenceId = sequence.id;
    row.gstart = 0;
    row.gend = seq.length();
    row.gaps = gaps;

    sqliteDbi->getMsaDbi()->addRow(msaId, -1, row, os);
    CHECK_OP(os, U2MsaRow());
    return row;
}

U2DataId MsaSQLiteSpecificTestData::createTestMsa(bool enableModTracking, U2OpStatus& os) {
    // Create an alignment
    U2AlphabetId alphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
    U2DataId msaId = sqliteDbi->getMsaDbi()->createMsaObject("", TEST_MSA_NAME, alphabet, os);
    CHECK_OP(os, U2DataId());

    // The alignment has the following rows:
    // T-AAGAC-TTCTA
    // TAAGC--TACTA
    addRow(msaId, "1", "TAAGACTTCTA", QList<U2MsaGap>() << U2MsaGap(1, 1) << U2MsaGap(7, 1), os);
    addRow(msaId, "2", "TAAGCTACTA", QList<U2MsaGap>() << U2MsaGap(5, 2), os);

    if (enableModTracking) {
        sqliteDbi->getObjectDbi()->setTrackModType(msaId, TrackOnUpdate, os);
        CHECK_OP(os, U2DataId());
    }

    return msaId;
}


IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaName_noModTrack) {
    U2OpStatusImpl os;
    SQLiteDbi* sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(false, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Rename the msa
    QString newName = "Renamed alignment";
    sqliteDbi->getMsaDbi()->updateMsaName(msaId, newName, os);
    CHECK_NO_ERROR(os);

    // Verify the msa has been renamed
    U2Msa msaObj = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newName, msaObj.visualName, "name");

    // Verify the version has been incremented
    int versionAfterUpdate = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(objVersion + 1, versionAfterUpdate, "version");

    // Verify that there are no modification steps
    qint64 actualModStepsNum = MsaSQLiteSpecificTestData::getModStepsNum(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(0, actualModStepsNum, "mod steps num");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaName_undo) {
    U2OpStatusImpl os;
    SQLiteDbi* sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    QString msaName = MsaSQLiteSpecificTestData::TEST_MSA_NAME;
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Rename the msa
    QString newName = "Renamed alignment";
    sqliteDbi->getMsaDbi()->updateMsaName(msaId, newName, os);
    CHECK_NO_ERROR(os);

    // Verify the msa has been renamed
    U2Msa msaObj = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(msaObj.visualName, newName, "name");

    // Verify the version has been incremented
    int versionAfterUpdate = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(objVersion + 1, versionAfterUpdate, "version");

    // Verify the modification step
    U2ModStep modStep = sqliteDbi->getModDbi()->getModStep(msaId, objVersion, os);
    QString expectedModDetails = "0&" + msaName + "&" + newName;
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(msaId, modStep.objectId, "object id");
    CHECK_EQUAL(objVersion, modStep.version, "version in mod step");
    CHECK_EQUAL(U2ModType::objUpdatedName, modStep.modType, "mod step type");
    CHECK_EQUAL(expectedModDetails, QString(modStep.details), "mod step details");

    // Undo msa renaming
    sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify the object name and version have been restored
    U2Msa msaObjAfterUndo = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(msaName, msaObjAfterUndo.visualName, "name after undo");
    CHECK_EQUAL(objVersion, msaObjAfterUndo.version, "version after undo");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaName_redo) {
    U2OpStatusImpl os;
    SQLiteDbi* sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    QString msaName = MsaSQLiteSpecificTestData::TEST_MSA_NAME;
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Rename the msa
    QString newName = "Renamed alignment";
    sqliteDbi->getMsaDbi()->updateMsaName(msaId, newName, os);
    CHECK_NO_ERROR(os);

    // Undo msa renaming
    sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
    CHECK_NO_ERROR(os);

    // Redo msa renaming
    sqliteDbi->getSQLiteObjectDbi()->redo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify the object name and version
    U2Msa msaObjAfterUndo = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newName, msaObjAfterUndo.visualName, "name after undo/redo");
    CHECK_EQUAL(objVersion + 1, msaObjAfterUndo.version, "version after undo/redo");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaAlphabet_noModTrack) {
    U2OpStatusImpl os;
    SQLiteDbi* sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(false, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update the msa alphabet
    U2AlphabetId oldAlphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
    U2AlphabetId newAlphabet = BaseDNAAlphabetIds::NUCL_DNA_EXTENDED();
    sqliteDbi->getMsaDbi()->updateMsaAlphabet(msaId, newAlphabet, os);
    CHECK_NO_ERROR(os);

    // Verify the alphabet has been changed
    U2Msa msaObj = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newAlphabet.id, msaObj.alphabet.id, "alphabet");

    // Verify the version has been incremented
    int versionAfterUpdate = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(objVersion + 1, versionAfterUpdate, "version");

    // Verify that there are no modification steps
    qint64 actualModStepsNum = MsaSQLiteSpecificTestData::getModStepsNum(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(0, actualModStepsNum, "mod steps num");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaAlphabet_undo) {
    U2OpStatusImpl os;
    SQLiteDbi* sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update the msa alphabet
    U2AlphabetId oldAlphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
    U2AlphabetId newAlphabet = BaseDNAAlphabetIds::NUCL_DNA_EXTENDED();
    sqliteDbi->getMsaDbi()->updateMsaAlphabet(msaId, newAlphabet, os);
    CHECK_NO_ERROR(os);

    // Verify the alphabet has been changed
    U2Msa msaObj = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newAlphabet.id, msaObj.alphabet.id, "alphabet");

    // Verify the version has been incremented
    int versionAfterUpdate = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(objVersion + 1, versionAfterUpdate, "version");

    // Verify the modification step
    U2ModStep modStep = sqliteDbi->getModDbi()->getModStep(msaId, objVersion, os);
    QString expectedModDetails = "0&" + oldAlphabet.id + "&" + newAlphabet.id;
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(msaId, modStep.objectId, "object id");
    CHECK_EQUAL(objVersion, modStep.version, "version in mod step");
    CHECK_EQUAL(U2ModType::msaUpdatedAlphabet, modStep.modType, "mod step type");
    CHECK_EQUAL(expectedModDetails, QString(modStep.details), "mod step details");

    // Undo alphabet updating
    sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify the object alphabet and version have been restored
    U2Msa msaObjAfterUndo = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldAlphabet.id, msaObjAfterUndo.alphabet.id, "name after undo");
    CHECK_EQUAL(objVersion, msaObjAfterUndo.version, "version after undo");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaAlphabet_redo) {
    U2OpStatusImpl os;
    SQLiteDbi* sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    QString msaName = MsaSQLiteSpecificTestData::TEST_MSA_NAME;
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update the msa alphabet
    U2AlphabetId oldAlphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
    U2AlphabetId newAlphabet = BaseDNAAlphabetIds::NUCL_DNA_EXTENDED();
    sqliteDbi->getMsaDbi()->updateMsaAlphabet(msaId, newAlphabet, os);
    CHECK_NO_ERROR(os);

    // Undo alphabet updatig
    sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
    CHECK_NO_ERROR(os);

    // Redo alphabet updating
    sqliteDbi->getSQLiteObjectDbi()->redo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify the object alphabet and version
    U2Msa msaObjAfterUndo = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newAlphabet.id, msaObjAfterUndo.alphabet.id, "alphabet after undo/redo");
    CHECK_EQUAL(objVersion + 1, msaObjAfterUndo.version, "version after undo/redo");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateGapModel_noModTrack) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(false, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update the msa alphabet
    QList<U2MsaGap> oldGaps = sqliteDbi->getMsaDbi()->getRow(msaId, 0, os).gaps;
    QList<U2MsaGap> newGaps; newGaps << U2MsaGap(4, 3) << U2MsaGap(11, 3); // TAAG---ACTT---CTA
    CHECK_NO_ERROR(os);
    sqliteDbi->getMsaDbi()->updateGapModel(msaId, 0, newGaps, os);
    CHECK_NO_ERROR(os);

    // Verify gaps
    U2MsaRow row = sqliteDbi->getMsaDbi()->getRow(msaId, 0, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(newGaps == row.gaps, "gaps");

    // Verify msa length
    U2Msa msaAfterUpdate = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_EQUAL(17, msaAfterUpdate.length, "length");

    // Verify version
    int versionAfterUpdate = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(objVersion + 1, versionAfterUpdate, "version");

    // Verify no modification steps
    qint64 actualModStepsNum = MsaSQLiteSpecificTestData::getModStepsNum(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(0, actualModStepsNum, "mod steps num");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateGapModel_undo) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update the msa alphabet
    QList<U2MsaGap> oldGaps = sqliteDbi->getMsaDbi()->getRow(msaId, 1, os).gaps;
    QList<U2MsaGap> newGaps; newGaps << U2MsaGap(1, 4) << U2MsaGap(14, 11); // T----AAGCTACTA-----------
    CHECK_NO_ERROR(os);
    sqliteDbi->getMsaDbi()->updateGapModel(msaId, 1, newGaps, os);
    CHECK_NO_ERROR(os);

    // Verify gaps
    U2MsaRow rowAfterUpdate = sqliteDbi->getMsaDbi()->getRow(msaId, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(newGaps == rowAfterUpdate.gaps, "gaps");

    // Verify msa length
    U2Msa msaAfterUpdate = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_EQUAL(14, msaAfterUpdate.length, "length");

    // Verify version
    int versionAfterUpdate = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(objVersion + 1, versionAfterUpdate, "version");

    // Verify the modification step
    QString expectedModDetails = "0&" + QByteArray::number(rowAfterUpdate.rowId) + "&\"5,2\"&\"1,4;14,11\"";
    U2ModStep modStep = sqliteDbi->getModDbi()->getModStep(msaId, objVersion, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(msaId, modStep.objectId, "object id");
    CHECK_EQUAL(objVersion, modStep.version, "version in mod step");
    CHECK_EQUAL(U2ModType::msaUpdatedGapModel, modStep.modType, "mod step type");
    CHECK_EQUAL(expectedModDetails, QString(modStep.details), "mod step details");

    // Undo
    sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify gaps
    U2MsaRow rowAfterUndo = sqliteDbi->getMsaDbi()->getRow(msaId, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(oldGaps == rowAfterUndo.gaps, "gaps after undo");

    // Verify msa length
    U2Msa msaAfterUndo = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_EQUAL(13, msaAfterUndo.length, "length");

    // Verify version
    int versionAfterUndo = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(objVersion, versionAfterUndo, "version after undo");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateGapModel_redo) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update the msa alphabet
    QList<U2MsaGap> oldGaps = sqliteDbi->getMsaDbi()->getRow(msaId, 0, os).gaps;
    QList<U2MsaGap> newGaps; // TAAGACTTCTA
    CHECK_NO_ERROR(os);
    sqliteDbi->getMsaDbi()->updateGapModel(msaId, 0, newGaps, os);
    CHECK_NO_ERROR(os);

    // Undo
    sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
    CHECK_NO_ERROR(os);

    // Redo
    sqliteDbi->getSQLiteObjectDbi()->redo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify gaps
    U2MsaRow rowAfterRedo = sqliteDbi->getMsaDbi()->getRow(msaId, 0, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(newGaps == rowAfterRedo.gaps, "gaps after undo");

    // Verify msa length
    U2Msa msaAfterRedo = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_EQUAL(12, msaAfterRedo.length, "length");

    // Verify version
    int versionAfterRedo = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(objVersion + 1, versionAfterRedo, "version after undo");

    // Verify the modification step
    QString expectedModDetails = "0&" + QByteArray::number(rowAfterRedo.rowId) + "&\"1,1;7,1\"&\"\"";
    U2ModStep modStep = sqliteDbi->getModDbi()->getModStep(msaId, objVersion, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(msaId, modStep.objectId, "object id");
    CHECK_EQUAL(objVersion, modStep.version, "version in mod step");
    CHECK_EQUAL(U2ModType::msaUpdatedGapModel, modStep.modType, "mod step type");
    CHECK_EQUAL(expectedModDetails, QString(modStep.details), "mod step details");
}

} // namespace
