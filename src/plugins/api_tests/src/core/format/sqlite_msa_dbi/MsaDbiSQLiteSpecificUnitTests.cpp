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
    CHECK_EQUAL(msaObj.visualName, newName, "name");

    // Verify the version has been incremented
    int versionAfterUpdate = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(objVersion + 1, versionAfterUpdate, "version");

    // Verify that there is no modification steps
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

    // Verify the object name and version has been restored
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

    // Verify the object name and version has been restored
    U2Msa msaObjAfterUndo = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newName, msaObjAfterUndo.visualName, "name after undo/redo");
    CHECK_EQUAL(objVersion + 1, msaObjAfterUndo.version, "version after undo/redo");
}

} // namespace
