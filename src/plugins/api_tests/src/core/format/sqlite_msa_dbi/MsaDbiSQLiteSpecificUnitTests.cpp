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

U2DataId MsaSQLiteSpecificTestData::createNotSoSmallTestMsa(bool enableModTracking, U2OpStatus& os) {
    // Create an alignment
    U2AlphabetId alphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
    U2DataId msaId = sqliteDbi->getMsaDbi()->createMsaObject("", TEST_MSA_NAME, alphabet, os);
    CHECK_OP(os, U2DataId());

    // The alignment has the following rows:
    // T-AAGAC-TTCTA--ATTTATTCGGGG-CTAGCTAGCA--TTTCAGCTAGCT---ACGGG
    // TAAGC--TACTAGGGA-CTGAG-CTAG---CATTCAGCTACGAGGGGGC--AAACTACTG
    // TTAGCTAGCTTAGCATCT--TAGCTATG--ACGTACAGCGATT-CGAC-GTA-TCAG-CT
    // ACTTTGAC---TCGAGCGAGC---TTATTTTACGATCAC--GACCCATAAGAATA--GAGCTAT
    // ACTGGC-TACAGCGA-TCTAGG-GGAAATC-TACTATC-T-ACGAT-CTAGTATCACGAT
    // AC--TAGGTTCAGCTAG--CGGATCGACATTTTGTAGGACGACGT--ACTTTCCCA--CGCTA
    // TTT--C-G-T-GTGTCACATA-C----GTCAGT----------TAACGATCGATCACACAC
    addRow(msaId, "1", "TAAGACTTCTAATTTATTCGGGGCTAGCTAGCATTTCAGCTAGCTACGGG", QList<U2MsaGap>() << U2MsaGap(1, 1) << U2MsaGap(7, 1) << U2MsaGap(13, 2) << U2MsaGap(27, 1) << U2MsaGap(38, 2) << U2MsaGap(52, 3), os);
    addRow(msaId, "2", "TAAGCTACTAGGGACTGAGCTAGCATTCAGCTACGAGGGGGCAAACTACTG", QList<U2MsaGap>() << U2MsaGap(5, 2) << U2MsaGap(16, 1) << U2MsaGap(22, 1) << U2MsaGap(27, 1) << U2MsaGap(49, 2), os);
    addRow(msaId, "3", "TTAGCTAGCTTAGCATCTTAGCTATGACGTACAGCGATTCGACGTATCAGCT", QList<U2MsaGap>() << U2MsaGap(18, 2) << U2MsaGap(28, 2) << U2MsaGap(43, 1) << U2MsaGap(48, 1) << U2MsaGap(52, 1) << U2MsaGap(57, 1), os);
    addRow(msaId, "4", "ACTTTGACTCGAGCGAGCTTATTTTACGATCACGACCCATAAGAATAGAGCTAT", QList<U2MsaGap>() << U2MsaGap(8, 3) << U2MsaGap(21, 3) << U2MsaGap(39, 2) << U2MsaGap(55, 2), os);
    addRow(msaId, "5", "ACTGGCTACAGCGATCTAGGGGAAATCTACTATCTACGATCTAGTATCACGAT", QList<U2MsaGap>() << U2MsaGap(6, 1) << U2MsaGap(15, 1) << U2MsaGap(22, 1) << U2MsaGap(30, 1) << U2MsaGap(38, 1) << U2MsaGap(40, 1) << U2MsaGap(46, 1), os);
    addRow(msaId, "6", "ACTAGGTTCAGCTAGCGGATCGACATTTTGTAGGACGACGTACTTTCCCACGCTA", QList<U2MsaGap>() << U2MsaGap(2, 2) << U2MsaGap(17, 2) << U2MsaGap(45, 2) << U2MsaGap(56, 2), os);
    addRow(msaId, "7", "TTTCGTGTGTCACATACGTCAGTTAACGATCGATCACACAC", QList<U2MsaGap>() << U2MsaGap(3, 2) << U2MsaGap(6, 1) << U2MsaGap(8, 1) << U2MsaGap(10, 1) << U2MsaGap(21, 1) << U2MsaGap(23, 4) << U2MsaGap(33, 10), os);

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

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaName_severalSteps) {
    //5 changes, 4 undo steps, 2 redo steps, 1 undo step, 1 redo step
    U2OpStatusImpl os;
    SQLiteDbi* sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    QString msaName = MsaSQLiteSpecificTestData::TEST_MSA_NAME;
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Prepare value list
    QStringList newNames(sqliteDbi->getMsaDbi()->getMsaObject(msaId, os).visualName);
    CHECK_NO_ERROR(os);
    for (int i = 1; i < 6; ++i) {
        newNames << "Renamed alignment" + QString::number(i);
    }

    // Steps count
    int valuesCount = newNames.length();    // changes = valuesCount - 1;
    QList<int> steps;                       // negative - undo steps, positive - redo steps;
    steps << -4 << 2 << -1 << 1;
    int expectedIndex = valuesCount - 1;
    for (int i = 0; i < steps.length(); ++i) {
        expectedIndex += steps[i];
    }

    // Rename the msa (changesCount times)
    for (int i = 1; i < newNames.length(); ++i) {
        sqliteDbi->getMsaDbi()->updateMsaName(msaId, newNames[i], os);
        CHECK_NO_ERROR(os);
    }

    // Undo and redo msa renaming
    int totalUndo = 0;
    int totalRedo = 0;
    for (int i  = 0; i < steps.length(); ++i) {
        if (steps[i] < 0) {
            for (int j = 0; j < -steps[i]; ++j) {
                sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
                CHECK_NO_ERROR(os);
                totalUndo++;
            }
        } else {
            for (int j = 0; j < steps[i]; ++j) {
                sqliteDbi->getSQLiteObjectDbi()->redo(msaId, os);
                CHECK_NO_ERROR(os);
                totalRedo++;
            }
        }
    }

    // Verify the object name and version
    U2Msa msaObjAfterUndo = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newNames[expectedIndex], msaObjAfterUndo.visualName, QString("name after %1 changes, %2 undo steps, %3 redo steps").arg(valuesCount - 1, totalUndo, totalRedo));
    CHECK_EQUAL(objVersion + expectedIndex, msaObjAfterUndo.version, "version after undo/redo");
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

    // Undo alphabet updating
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

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateMsaAlphabet_severalSteps) {
    //3 changes, 3 undo steps, 2 redo steps, 2 undo steps, 1 redo step
    U2OpStatusImpl os;
    SQLiteDbi* sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    QString msaName = MsaSQLiteSpecificTestData::TEST_MSA_NAME;
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Prepare value list
    QList<U2AlphabetId> newAlphabets;
    newAlphabets << sqliteDbi->getMsaDbi()->getMsaObject(msaId, os).alphabet;
    CHECK_NO_ERROR(os);
    newAlphabets << BaseDNAAlphabetIds::NUCL_DNA_EXTENDED();
    newAlphabets << BaseDNAAlphabetIds::NUCL_RNA_DEFAULT();
    newAlphabets << BaseDNAAlphabetIds::NUCL_RNA_EXTENDED();

    // Steps count
    int valuesCount = newAlphabets.length();    // changes = valuesCount - 1;
    QList<int> steps;                           // negative - undo steps, positive - redo steps;
    steps << -3 << 2 << -2 << 1;
    int expectedIndex = valuesCount - 1;
    for (int i = 0; i < steps.length(); ++i) {
        expectedIndex += steps[i];
    }

    // Update the msa alphabet
    for (int i = 1; i < newAlphabets.length(); ++i) {
        sqliteDbi->getMsaDbi()->updateMsaAlphabet(msaId, newAlphabets[i], os);
        CHECK_NO_ERROR(os);
    }

    // Undo and redo alphabet updating
    int totalUndo = 0;
    int totalRedo = 0;
    for (int i  = 0; i < steps.length(); ++i) {
        if (steps[i] < 0) {
            for (int j = 0; j < -steps[i]; ++j) {
                sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
                CHECK_NO_ERROR(os);
                totalUndo++;
            }
        } else {
            for (int j = 0; j < steps[i]; ++j) {
                sqliteDbi->getSQLiteObjectDbi()->redo(msaId, os);
                CHECK_NO_ERROR(os);
                totalRedo++;
            }
        }
    }

    // Verify the object alphabet and version
    U2Msa msaObjAfterUndo = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newAlphabets[expectedIndex].id, msaObjAfterUndo.alphabet.id, QString("alphabet after %1 changes, %2 undo steps, %3 redo steps").arg(valuesCount - 1, totalUndo, totalRedo));
    CHECK_EQUAL(objVersion + expectedIndex, msaObjAfterUndo.version, "version after undo/redo");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateGapModel_noModTrack) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(false, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update gaps
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

    // Update gaps
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

    // Update gaps
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

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateGapModel_severalSteps) {
    //6 changes, 4 undo steps, 2 redo steps, 1 undo step, 3 redo steps
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Get rows
    QList<U2MsaRow> oldRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);

    // Prepare value list
    QList<QList<U2MsaGap> > gapModels;
    gapModels << oldRows[0].gaps;  // base value

    QList<U2MsaGap> gapModel;
    for (int i = 0; i < 6; ++i) {
        gapModel.clear();
        gapModel << U2MsaGap(0, i + 1) << U2MsaGap(i + 3, 2);
        gapModels << gapModel;
    }

    // Steps count
    int valuesCount = gapModels.length();    // changes = valuesCount - 1;
    QList<int> steps;                        // negative - undo steps, positive - redo steps;
    steps << -4 << 2 << -1 << 3;
    int expectedIndex = valuesCount - 1;
    for (int i = 0; i < steps.length(); ++i) {
        expectedIndex += steps[i];
    }

    //Expected msa length
    qint64 expectedMsaLength = oldRows[0].gend - oldRows[1].gstart;
    CHECK_NO_ERROR(os);
    foreach (U2MsaGap gap, gapModels[expectedIndex]) {
        expectedMsaLength += gap.gap;
    }
    expectedMsaLength = qMax(expectedMsaLength, oldRows[1].length);

    // Expected modDetails
    QList<QByteArray> expectedModDetails;
    expectedModDetails << "0&" + QByteArray::number(oldRows[0].rowId) + "&\"1,1;7,1\"&\"\"";
    expectedModDetails << "0&" + QByteArray::number(oldRows[0].rowId) + "&\"0,1;3,2\"&\"\"";
    expectedModDetails << "0&" + QByteArray::number(oldRows[0].rowId) + "&\"0,2;4,2\"&\"\"";
    expectedModDetails << "0&" + QByteArray::number(oldRows[0].rowId) + "&\"0,3;5,2\"&\"\"";
    expectedModDetails << "0&" + QByteArray::number(oldRows[0].rowId) + "&\"0,4;6,2\"&\"\"";
    expectedModDetails << "0&" + QByteArray::number(oldRows[0].rowId) + "&\"0,5;7,2\"&\"\"";
    expectedModDetails << "0&" + QByteArray::number(oldRows[0].rowId) + "&\"0,6;8,2\"&\"\"";

    // Update the msa gap model
    for (int i = 1; i < gapModels.length(); ++i) {
        sqliteDbi->getMsaDbi()->updateGapModel(msaId, 0, gapModels[i], os);
        CHECK_NO_ERROR(os);
    }

    // Undo and redo gap model updating
    int totalUndo = 0;
    int totalRedo = 0;
    for (int i  = 0; i < steps.length(); ++i) {
        if (steps[i] < 0) {
            for (int j = 0; j < -steps[i]; ++j) {
                sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
                CHECK_NO_ERROR(os);
                totalUndo++;
            }
        } else {
            for (int j = 0; j < steps[i]; ++j) {
                sqliteDbi->getSQLiteObjectDbi()->redo(msaId, os);
                CHECK_NO_ERROR(os);
                totalRedo++;
            }
        }
    }

    // Verify gaps
    U2MsaRow finalRow = sqliteDbi->getMsaDbi()->getRow(msaId, oldRows[0].rowId, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(gapModels[expectedIndex] == finalRow.gaps, "final gaps");

    // Verify msa length
    U2Msa finalMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_EQUAL(expectedMsaLength, finalMsa.length, "length");

    // Verify version
    int finalVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(objVersion + expectedIndex, finalVersion, "final version");

    // Verify the modification step
    U2ModStep modStep = sqliteDbi->getModDbi()->getModStep(msaId, finalVersion, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(msaId, modStep.objectId, "object id");
    CHECK_EQUAL(finalVersion, modStep.version, "version in mod step");
    CHECK_EQUAL(U2ModType::msaUpdatedGapModel, modStep.modType, "mod step type");
    CHECK_EQUAL(expectedModDetails[expectedIndex], QString(modStep.details), "mod step details");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowContent_noModTrack) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(false, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int oldVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update row content
    U2MsaRow oldRow = sqliteDbi->getMsaDbi()->getRow(msaId, 0, os);
    CHECK_NO_ERROR(os);
    QByteArray oldSeq = sqliteDbi->getSequenceDbi()->getSequenceData(oldRow.sequenceId, U2_REGION_MAX, os);
    CHECK_NO_ERROR(os);
    QByteArray newSeq = "AAAAGGGGCCCCTTTT";
    QList<U2MsaGap> newGaps; newGaps << U2MsaGap(4, 4) << U2MsaGap(20, 4); // AAAA----GGGGCCCCTTTT----
    sqliteDbi->getMsaDbi()->updateRowContent(msaId, 0, newSeq, newGaps, os);
    CHECK_NO_ERROR(os);

    // Verify gaps
    U2MsaRow newRow = sqliteDbi->getMsaDbi()->getRow(msaId, 0, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(newGaps == newRow.gaps, "gaps");

    // Verify seq
    QByteArray newestSeq = sqliteDbi->getSequenceDbi()->getSequenceData(newRow.sequenceId, U2_REGION_MAX, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newSeq, newestSeq, "sequence");

    // Verify msa length
    U2Msa newMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_EQUAL(20, newMsa.length, "length");

    // Verify version
    int newVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion + 1, newVersion, "version");

    // Verify no modification steps
    qint64 modStepsNum = MsaSQLiteSpecificTestData::getModStepsNum(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(0, modStepsNum, "mod steps num");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowContent_undo) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int oldVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update row content
    U2MsaRow oldRow = sqliteDbi->getMsaDbi()->getRow(msaId, 1, os);
    CHECK_NO_ERROR(os);
    QByteArray oldSeq = sqliteDbi->getSequenceDbi()->getSequenceData(oldRow.sequenceId, U2_REGION_MAX, os);
    CHECK_NO_ERROR(os);
    QByteArray newSeq = "GGGGGTTTTTCCCCCAAAAA";
    QList<U2MsaGap> newGaps; newGaps << U2MsaGap(0, 1) << U2MsaGap(2, 1); // -G-GGGGTTTTTCCCCCAAAAA
    sqliteDbi->getMsaDbi()->updateRowContent(msaId, 1, newSeq, newGaps, os);
    CHECK_NO_ERROR(os);

    // Verify gaps
    U2MsaRow newRow = sqliteDbi->getMsaDbi()->getRow(msaId, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(newGaps == newRow.gaps, "gaps");

    // Verify seq
    QByteArray newestSeq = sqliteDbi->getSequenceDbi()->getSequenceData(newRow.sequenceId, U2_REGION_MAX, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newSeq, newestSeq, "sequence");

    // Verify msa length
    U2Msa newMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_EQUAL(22, newMsa.length, "length");

    // Verify version
    int newVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion + 1, newVersion, "version");

    // Verify the modification step
    QString expectedModDetails = "0&" + QByteArray::number(newRow.rowId) + "&TAAGCTACTA&\"5,2\"&GGGGGTTTTTCCCCCAAAAA&\"0,1;2,1\"";
    U2ModStep modStep = sqliteDbi->getModDbi()->getModStep(msaId, oldVersion, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(msaId, modStep.objectId, "object id");
    CHECK_EQUAL(oldVersion, modStep.version, "version in mod step");
    CHECK_EQUAL(U2ModType::msaUpdatedRowContent, modStep.modType, "mod step type");
    CHECK_EQUAL(expectedModDetails, QString(modStep.details), "mod step details");

    // Undo
    sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify gaps
    U2MsaRow undoRow = sqliteDbi->getMsaDbi()->getRow(msaId, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(oldRow.gaps == undoRow.gaps, "gaps after undo");

    // Verify seq
    QByteArray undoSeq = sqliteDbi->getSequenceDbi()->getSequenceData(undoRow.sequenceId, U2_REGION_MAX, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldSeq, undoSeq, "sequence");

    // Verify msa length
    U2Msa undoMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_EQUAL(13, undoMsa.length, "length");

    // Verify version
    int undoVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion, undoVersion, "version after undo");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowContent_redo) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int oldVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update row content
    U2MsaRow oldRow = sqliteDbi->getMsaDbi()->getRow(msaId, 0, os);
    CHECK_NO_ERROR(os);
    QByteArray oldSeq = sqliteDbi->getSequenceDbi()->getSequenceData(oldRow.sequenceId, U2_REGION_MAX, os);
    CHECK_NO_ERROR(os);
    QByteArray newSeq = "";
    QList<U2MsaGap> newGaps;
    sqliteDbi->getMsaDbi()->updateRowContent(msaId, 0, newSeq, newGaps, os);
    CHECK_NO_ERROR(os);

    // Undo
    sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
    CHECK_NO_ERROR(os);

    // Redo
    sqliteDbi->getSQLiteObjectDbi()->redo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify gaps
    U2MsaRow redoRow = sqliteDbi->getMsaDbi()->getRow(msaId, 0, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(newGaps == redoRow.gaps, "gaps after undo");

    // Verify seq
    QByteArray redoSeq = sqliteDbi->getSequenceDbi()->getSequenceData(redoRow.sequenceId, U2_REGION_MAX, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newSeq, redoSeq, "sequence");

    // Verify msa length
    U2Msa redoMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_EQUAL(12, redoMsa.length, "length");

    // Verify version
    int redoVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion + 1, redoVersion, "version after undo");

    // Verify the modification step
    QString expectedModDetails = "0&" + QByteArray::number(redoRow.rowId) + "&TAAGACTTCTA&\"1,1;7,1\"&&\"\"";
    U2ModStep modStep = sqliteDbi->getModDbi()->getModStep(msaId, oldVersion, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(msaId, modStep.objectId, "object id");
    CHECK_EQUAL(oldVersion, modStep.version, "version in mod step");
    CHECK_EQUAL(U2ModType::msaUpdatedRowContent, modStep.modType, "mod step type");
    CHECK_EQUAL(expectedModDetails, QString(modStep.details), "mod step details");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowContent_severalSteps) {
    //6 changes, 6 undo steps, 4 redo steps, 3 undo step, 2 redo steps
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int oldVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Get rows
    QList<U2MsaRow> oldRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);

    // Prepare value list
    QList<QPair<QByteArray, QList<U2MsaGap> > > rowContents;
    rowContents << QPair<QByteArray, QList<U2MsaGap> >(sqliteDbi->getSequenceDbi()->getSequenceData(oldRows[0].sequenceId, U2_REGION_MAX, os), oldRows[0].gaps);
    CHECK_NO_ERROR(os);
    for (int i = 0; i < 6; ++i) {
        QByteArray firstPart((i + 1) * 2, 'A');
        QByteArray secondPart((i + 3) * 2, 'C');
        QList<U2MsaGap> gapModel = QList<U2MsaGap>() << U2MsaGap((i + 2) * 2, 5);
        rowContents << QPair<QByteArray, QList<U2MsaGap> >(firstPart + secondPart, gapModel);
    }

    // Steps count
    int valuesCount = rowContents.length();    // changes = valuesCount - 1;
    QList<int> steps;                          // negative - undo steps, positive - redo steps;
    steps << -4 << 2 << -1 << 3;
    int expectedIndex = valuesCount - 1;
    for (int i = 0; i < steps.length(); ++i) {
        expectedIndex += steps[i];
    }

    //Expected msa length
    qint64 expectedMsaLength = rowContents[expectedIndex].first.length();
    CHECK_NO_ERROR(os);
    foreach (U2MsaGap gap, rowContents[expectedIndex].second) {
        expectedMsaLength += gap.gap;
    }
    expectedMsaLength = qMax(expectedMsaLength, oldRows[1].length);

    // Expected modDetails
    QList<QByteArray> expectedModDetails;
    for (int i = 0; i < rowContents.length() - 1; ++i) {
        QPair<QByteArray, QList<U2MsaGap> > rowContentFirst = rowContents[i];
        QPair<QByteArray, QList<U2MsaGap> > rowContentSecond = rowContents[i + 1];
        QByteArray gapsToByteArrayFirst;
        QByteArray gapsToByteArraySecond;
        gapsToByteArrayFirst += "\"";
        foreach(U2MsaGap gap, rowContentFirst.second) {
            if (false == gapsToByteArrayFirst.isEmpty()) {
                gapsToByteArrayFirst += ";";
            }
            gapsToByteArrayFirst += QByteArray::number(gap.offset) + "," + QByteArray::number(gap.gap);
        }
        gapsToByteArrayFirst += "\"";
        gapsToByteArraySecond += "\"";
        foreach(U2MsaGap gap, rowContentSecond.second) {
            if (false == gapsToByteArraySecond.isEmpty()) {
                gapsToByteArraySecond += ";";
            }
            gapsToByteArraySecond += QByteArray::number(gap.offset) + "," + QByteArray::number(gap.gap);
        }
        gapsToByteArraySecond += "\"";

        expectedModDetails << "0&" + QByteArray::number(oldRows[0].rowId) + "&" + rowContentFirst.first + "&" + gapsToByteArrayFirst + "&" + rowContentSecond.first + "&" + gapsToByteArraySecond;
    }

    // Update row content
    for (int i = 1; i < rowContents.length(); ++i) {
        sqliteDbi->getMsaDbi()->updateRowContent(msaId, oldRows[0].rowId, rowContents[i].first, rowContents[i].second, os);
        CHECK_NO_ERROR(os);
    }

    // Undo and redo gap model updating
    int totalUndo = 0;
    int totalRedo = 0;
    for (int i  = 0; i < steps.length(); ++i) {
        if (steps[i] < 0) {
            for (int j = 0; j < -steps[i]; ++j) {
                sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
                CHECK_NO_ERROR(os);
                totalUndo++;
            }
        } else {
            for (int j = 0; j < steps[i]; ++j) {
                sqliteDbi->getSQLiteObjectDbi()->redo(msaId, os);
                CHECK_NO_ERROR(os);
                totalRedo++;
            }
        }
    }

    // Verify gaps
    U2MsaRow finalRow = sqliteDbi->getMsaDbi()->getRow(msaId, 0, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(rowContents[expectedIndex].second == finalRow.gaps, "final gaps");

    // Verify seq
    QByteArray finalSeq = sqliteDbi->getSequenceDbi()->getSequenceData(finalRow.sequenceId, U2_REGION_MAX, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(rowContents[expectedIndex].first, finalSeq, "sequence");

    // Verify msa length
    U2Msa finalMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_EQUAL(expectedMsaLength, finalMsa.length, "length");

    // Verify version
    int finalVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion + expectedIndex, finalVersion, "final version");

    // Verify the modification step
    U2ModStep modStep = sqliteDbi->getModDbi()->getModStep(msaId, finalVersion, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(msaId, modStep.objectId, "object id");
    CHECK_EQUAL(finalVersion, modStep.version, "version in mod step");
    CHECK_EQUAL(U2ModType::msaUpdatedGapModel, modStep.modType, "mod step type");
    CHECK_EQUAL(expectedModDetails[expectedIndex], QString(modStep.details), "mod step details");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, setNewRowsOrder_noModTrack) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createNotSoSmallTestMsa(false, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int oldVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Get current row order
    QList<qint64> oldOrder = sqliteDbi->getMsaDbi()->getRowsOrder(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_NOT_EQUAL(0, oldOrder.length(), "incorrect oreder list length");

    // Get current rows
    QList<U2MsaRow> oldRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);

    // Get current sequences
    QList<QByteArray> oldSequences;
    foreach(U2MsaRow row, oldRows) {
        oldSequences << sqliteDbi->getSequenceDbi()->getSequenceData(row.sequenceId, U2_REGION_MAX, os);
        CHECK_NO_ERROR(os);
    }

    // Get current U2Msa
    U2Msa oldMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);

    // Set new row order
    //Expected order (indexes): 4, 5, 1, 6, 0, 2, 3
    QList<qint64> newOrder = oldOrder;
    newOrder.swap(0, 4);
    newOrder.swap(3, 6);
    newOrder.swap(2, 5);
    newOrder.swap(1, 2);

    sqliteDbi->getMsaDbi()->setNewRowsOrder(msaId, newOrder, os);
    CHECK_NO_ERROR(os);

    // Verify order
    QList<qint64> newestOrder = sqliteDbi->getMsaDbi()->getRowsOrder(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newOrder.length(), newestOrder.length(), "row order length");
    for (int i = 0; i < newOrder.length(); ++i) {
        CHECK_EQUAL(newOrder[i], newestOrder[i],  "row order");
    }

    // Verify gaps and IDs
    // Order of the list has changed
    QList<U2MsaRow> newRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldRows.length(), newRows.length(), "rows length");
    foreach (U2MsaRow newRow, newRows) {
        bool ok = false;
        foreach (U2MsaRow oldRow, oldRows) {
            if (newRow.gaps == oldRow.gaps && newRow.gend == oldRow.gend &&
                    newRow.gstart == oldRow.gstart && newRow.length == oldRow.length &&
                    newRow.rowId == oldRow.rowId && newRow.sequenceId == oldRow.sequenceId) {
                ok = true;
                break;
            }
        }
        CHECK_TRUE(ok, "rows");
    }

    // Verify seq
    // Order of the list has changed
    foreach (U2MsaRow row, newRows) {
        QByteArray sequence = sqliteDbi->getSequenceDbi()->getSequenceData(row.sequenceId, U2_REGION_MAX, os);
        CHECK_NO_ERROR(os);
        CHECK_TRUE(oldSequences.contains(sequence), "sequence");
    }

    // Verify U2Msa
    U2Msa newMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(oldMsa.alphabet == newMsa.alphabet, "msa");
    CHECK_TRUE(oldMsa.length == newMsa.length, "msa");

    // Verify version
    int newVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion + 1, newVersion, "version");

    // Verify no modification steps
    qint64 modStepsNum = MsaSQLiteSpecificTestData::getModStepsNum(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(0, modStepsNum, "mod steps num");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, setNewRowsOrder_undo) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createNotSoSmallTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int oldVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Get current row order
    QList<qint64> oldOrder = sqliteDbi->getMsaDbi()->getRowsOrder(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_NOT_EQUAL(0, oldOrder.length(), "incorrect oreder list length");

    // Get current rows
    QList<U2MsaRow> oldRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);

    // Get current sequences
    QList<QByteArray> oldSequences;
    foreach(U2MsaRow row, oldRows) {
        oldSequences << sqliteDbi->getSequenceDbi()->getSequenceData(row.sequenceId, U2_REGION_MAX, os);
        CHECK_NO_ERROR(os);
    }

    // Get current U2Msa
    U2Msa oldMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);

    // Set new row order
    //Expected order (indexes): 4, 5, 1, 6, 0, 2, 3
    QList<qint64> newOrder = oldOrder;
    newOrder.swap(0, 4);
    newOrder.swap(3, 6);
    newOrder.swap(2, 5);
    newOrder.swap(1, 2);

    sqliteDbi->getMsaDbi()->setNewRowsOrder(msaId, newOrder, os);
    CHECK_NO_ERROR(os);

    // Verify order
    QList<qint64> newestOrder = sqliteDbi->getMsaDbi()->getRowsOrder(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newOrder.length(), newestOrder.length(), "row order length");
    for (int i = 0; i < newOrder.length(); ++i) {
        CHECK_EQUAL(newOrder[i], newestOrder[i],  "row order");
    }

    // Verify gaps and IDs
    // Order of the list has changed
    QList<U2MsaRow> newRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldRows.length(), newRows.length(), "rows length");
    foreach (U2MsaRow newRow, newRows) {
        bool ok = false;
        foreach (U2MsaRow oldRow, oldRows) {
            if (newRow.gaps == oldRow.gaps && newRow.gend == oldRow.gend &&
                    newRow.gstart == oldRow.gstart && newRow.length == oldRow.length &&
                    newRow.rowId == oldRow.rowId && newRow.sequenceId == oldRow.sequenceId) {
                ok = true;
                break;
            }
        }
        CHECK_TRUE(ok, "rows");
    }

    // Verify seq
    // Order of the list has changed
    foreach (U2MsaRow row, newRows) {
        QByteArray sequence = sqliteDbi->getSequenceDbi()->getSequenceData(row.sequenceId, U2_REGION_MAX, os);
        CHECK_NO_ERROR(os);
        CHECK_TRUE(oldSequences.contains(sequence), "sequence");
    }

    // Verify U2Msa
    U2Msa newMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(oldMsa.alphabet == newMsa.alphabet, "msa");
    CHECK_TRUE(oldMsa.length == newMsa.length, "msa");

    // Verify version
    int newVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion + 1, newVersion, "version");

    // Verify no modification steps
    qint64 modStepsNum = MsaSQLiteSpecificTestData::getModStepsNum(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(1, modStepsNum, "mod steps num");

    // Undo
    sqliteDbi->getObjectDbi()->undo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify order
    newestOrder = sqliteDbi->getMsaDbi()->getRowsOrder(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldOrder.length(), newestOrder.length(), "row order length");
    for (int i = 0; i < oldOrder.length(); ++i) {
        CHECK_EQUAL(oldOrder[i], newestOrder[i],  "row order");
    }

    // Verify gaps and IDs
    // Order of the list has changed
    newRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldRows.length(), newRows.length(), "rows length");
    foreach (U2MsaRow newRow, newRows) {
        bool ok = false;
        foreach (U2MsaRow oldRow, oldRows) {
            if (newRow.gaps == oldRow.gaps && newRow.gend == oldRow.gend &&
                    newRow.gstart == oldRow.gstart && newRow.length == oldRow.length &&
                    newRow.rowId == oldRow.rowId && newRow.sequenceId == oldRow.sequenceId);
            ok = true;
            break;
        }
        CHECK_TRUE(ok, "rows");
    }

    // Verify seq
    // Order of the list has changed
    foreach (U2MsaRow row, newRows) {
        QByteArray sequence = sqliteDbi->getSequenceDbi()->getSequenceData(row.sequenceId, U2_REGION_MAX, os);
        CHECK_TRUE(oldSequences.contains(sequence), "sequence");
    }

    // Verify U2Msa
    newMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(oldMsa.alphabet == newMsa.alphabet, "msa");
    CHECK_TRUE(oldMsa.length == newMsa.length, "msa");

    // Verify version
    newVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion, newVersion, "version");

    // Verify modification steps
    modStepsNum = MsaSQLiteSpecificTestData::getModStepsNum(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(1, modStepsNum, "mod steps num");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, setNewRowsOrder_redo) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createNotSoSmallTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int oldVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Get current row order
    QList<qint64> oldOrder = sqliteDbi->getMsaDbi()->getRowsOrder(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_NOT_EQUAL(0, oldOrder.length(), "incorrect oreder list length");

    // Get current rows
    QList<U2MsaRow> oldRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);

    // Get current sequences
    QList<QByteArray> oldSequences;
    foreach(U2MsaRow row, oldRows) {
        oldSequences << sqliteDbi->getSequenceDbi()->getSequenceData(row.sequenceId, U2_REGION_MAX, os);
        CHECK_NO_ERROR(os);
    }

    // Get current U2Msa
    U2Msa oldMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);

    // Set new row order
    //Expected order (indexes): 4, 5, 1, 6, 0, 2, 3
    QList<qint64> newOrder = oldOrder;
    newOrder.swap(0, 4);
    newOrder.swap(3, 6);
    newOrder.swap(2, 5);
    newOrder.swap(1, 2);

    sqliteDbi->getMsaDbi()->setNewRowsOrder(msaId, newOrder, os);
    CHECK_NO_ERROR(os);

    // Verify order
    QList<qint64> newestOrder = sqliteDbi->getMsaDbi()->getRowsOrder(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newOrder.length(), newestOrder.length(), "row order length");
    for (int i = 0; i < newOrder.length(); ++i) {
        CHECK_EQUAL(newOrder[i], newestOrder[i],  "row order");
    }

    // Verify gaps and IDs
    // Order of the list has changed
    QList<U2MsaRow> newRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldRows.length(), newRows.length(), "rows length");
    foreach (U2MsaRow newRow, newRows) {
        bool ok = false;
        foreach (U2MsaRow oldRow, oldRows) {
            if (newRow.gaps == oldRow.gaps && newRow.gend == oldRow.gend &&
                    newRow.gstart == oldRow.gstart && newRow.length == oldRow.length &&
                    newRow.rowId == oldRow.rowId && newRow.sequenceId == oldRow.sequenceId);
            ok = true;
            break;
        }
        CHECK_TRUE(ok, "rows");
    }

    // Verify seq
    // Order of the list has changed
    foreach (U2MsaRow row, newRows) {
        QByteArray sequence = sqliteDbi->getSequenceDbi()->getSequenceData(row.sequenceId, U2_REGION_MAX, os);
        CHECK_TRUE(oldSequences.contains(sequence), "sequence");
    }

    // Verify U2Msa
    U2Msa newMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(oldMsa.alphabet == newMsa.alphabet, "msa");
    CHECK_TRUE(oldMsa.length == newMsa.length, "msa");

    // Verify version
    int newVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion + 1, newVersion, "version");

    // Verify no modification steps
    qint64 modStepsNum = MsaSQLiteSpecificTestData::getModStepsNum(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(1, modStepsNum, "mod steps num");

    // Undo
    sqliteDbi->getObjectDbi()->undo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify order
    newestOrder = sqliteDbi->getMsaDbi()->getRowsOrder(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldOrder.length(), newestOrder.length(), "row order length");
    for (int i = 0; i < oldOrder.length(); ++i) {
        CHECK_EQUAL(oldOrder[i], newestOrder[i],  "row order");
    }

    // Verify gaps and IDs
    // Order of the list has changed
    newRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldRows.length(), newRows.length(), "rows length");
    foreach (U2MsaRow newRow, newRows) {
        bool ok = false;
        foreach (U2MsaRow oldRow, oldRows) {
            if (newRow.gaps == oldRow.gaps && newRow.gend == oldRow.gend &&
                    newRow.gstart == oldRow.gstart && newRow.length == oldRow.length &&
                    newRow.rowId == oldRow.rowId && newRow.sequenceId == oldRow.sequenceId) {
                ok = true;
                break;
            }
        }
        CHECK_TRUE(ok, "rows");
    }

    // Verify seq
    // Order of the list has changed
    foreach (U2MsaRow row, newRows) {
        QByteArray sequence = sqliteDbi->getSequenceDbi()->getSequenceData(row.sequenceId, U2_REGION_MAX, os);
        CHECK_TRUE(oldSequences.contains(sequence), "sequence");
    }

    // Verify U2Msa
    newMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(oldMsa.alphabet == newMsa.alphabet, "msa");
    CHECK_TRUE(oldMsa.length == newMsa.length, "msa");

    // Verify version
    newVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion, newVersion, "version");

    // Verify modification steps
    modStepsNum = MsaSQLiteSpecificTestData::getModStepsNum(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(1, modStepsNum, "mod steps num");

    // Redo
    sqliteDbi->getObjectDbi()->redo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify order
    newestOrder = sqliteDbi->getMsaDbi()->getRowsOrder(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newOrder.length(), newestOrder.length(), "row order length");
    for (int i = 0; i < newOrder.length(); ++i) {
        CHECK_EQUAL(newOrder[i], newestOrder[i],  "row order");
    }

    // Verify gaps and IDs
    // Order of the list has changed
    newRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldRows.length(), newRows.length(), "rows length");
    foreach (U2MsaRow newRow, newRows) {
        bool ok = false;
        foreach (U2MsaRow oldRow, oldRows) {
            if (newRow.gaps == oldRow.gaps && newRow.gend == oldRow.gend &&
                    newRow.gstart == oldRow.gstart && newRow.length == oldRow.length &&
                    newRow.rowId == oldRow.rowId && newRow.sequenceId == oldRow.sequenceId);
            ok = true;
            break;
        }
        CHECK_TRUE(ok, "rows");
    }

    // Verify seq
    // Order of the list has changed
    foreach (U2MsaRow row, newRows) {
        QByteArray sequence = sqliteDbi->getSequenceDbi()->getSequenceData(row.sequenceId, U2_REGION_MAX, os);
        CHECK_NO_ERROR(os);
        CHECK_TRUE(oldSequences.contains(sequence), "sequence");
    }

    // Verify U2Msa
    newMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(oldMsa.alphabet == newMsa.alphabet, "msa");
    CHECK_TRUE(oldMsa.length == newMsa.length, "msa");

    // Verify version
    newVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion + 1, newVersion, "version");

    // Verify no modification steps
    modStepsNum = MsaSQLiteSpecificTestData::getModStepsNum(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(1, modStepsNum, "mod steps num");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, setNewRowsOrder_severalSteps) {
    //6 changes, 5 undo steps, 3 redo steps, 4 undo step, 3 redo steps
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Get rows
    QList<U2MsaRow> oldRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);

    // Get current sequences
    QList<QByteArray> oldSequences;
    foreach(U2MsaRow row, oldRows) {
        oldSequences << sqliteDbi->getSequenceDbi()->getSequenceData(row.sequenceId, U2_REGION_MAX, os);
        CHECK_NO_ERROR(os);
    }

    // Get current U2Msa
    U2Msa oldMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);

    // Prepare value list
    QList<QList<qint64> > rowOrders;
    rowOrders << sqliteDbi->getMsaDbi()->getRowsOrder(msaId, os);  // base value
    CHECK_NO_ERROR(os);

    QList<qint64> rowOrder;
    for (int i = 0; i < 6; ++i) {
        rowOrder.clear();
        for (int j = 1; j <= oldRows.length(); ++j) {
            // On every step:
            // 0 1 2 3 4 5 6
            // 4 1 5 2 6 3 0
            rowOrder << rowOrders[i][(j * 11) % oldRows.length()];
        }
        rowOrders << rowOrder;
    }

    // Steps count
    int valuesCount = rowOrders.length();    // changes = valuesCount - 1;
    QList<int> steps;                        // negative - undo steps, positive - redo steps;
    steps << -5 << 3 << -4 << 3;
    int expectedIndex = valuesCount - 1;
    for (int i = 0; i < steps.length(); ++i) {
        expectedIndex += steps[i];
    }

    // Update the row order
    for (int i = 1; i < rowOrders.length(); ++i) {
        sqliteDbi->getMsaDbi()->setNewRowsOrder(msaId, rowOrders[i], os);
        CHECK_NO_ERROR(os);
    }

    // Undo and redo row order updating
    int totalUndo = 0;
    int totalRedo = 0;
    for (int i  = 0; i < steps.length(); ++i) {
        if (steps[i] < 0) {
            for (int j = 0; j < -steps[i]; ++j) {
                sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
                CHECK_NO_ERROR(os);
                totalUndo++;
            }
        } else {
            for (int j = 0; j < steps[i]; ++j) {
                sqliteDbi->getSQLiteObjectDbi()->redo(msaId, os);
                CHECK_NO_ERROR(os);
                totalRedo++;
            }
        }
    }

    // Verify order
    QList<qint64> finalOrder = sqliteDbi->getMsaDbi()->getRowsOrder(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(rowOrders[expectedIndex].length(), finalOrder.length(), "row order length");
    for (int i = 0; i < finalOrder.length(); ++i) {
        CHECK_EQUAL(rowOrders[expectedIndex][i], finalOrder[i],  "row order");
    }

    // Verify gaps and IDs
    // Order of the list has changed
    QList<U2MsaRow> newRows = sqliteDbi->getMsaDbi()->getRows(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldRows.length(), newRows.length(), "rows length");
    foreach (U2MsaRow newRow, newRows) {
        bool ok = false;
        foreach (U2MsaRow oldRow, oldRows) {
            if (newRow.gaps == oldRow.gaps && newRow.gend == oldRow.gend &&
                    newRow.gstart == oldRow.gstart && newRow.length == oldRow.length &&
                    newRow.rowId == oldRow.rowId && newRow.sequenceId == oldRow.sequenceId) {
                ok = true;
                break;
            }
        }
        CHECK_TRUE(ok, "rows");
    }

    // Verify seq
    // Order of the list has changed
    foreach (U2MsaRow row, newRows) {
        QByteArray sequence = sqliteDbi->getSequenceDbi()->getSequenceData(row.sequenceId, U2_REGION_MAX, os);
        CHECK_NO_ERROR(os);
        CHECK_TRUE(oldSequences.contains(sequence), "sequence");
    }

    // Verify U2Msa
    U2Msa newMsa = sqliteDbi->getMsaDbi()->getMsaObject(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_TRUE(oldMsa.alphabet == newMsa.alphabet, "msa");
    CHECK_TRUE(oldMsa.length == newMsa.length, "msa");

    // Verify version
    qint64 newVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(objVersion + expectedIndex, newVersion, "version");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowName_noModTrack) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(false, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int oldVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update row name 
    U2MsaRow oldRow = sqliteDbi->getMsaDbi()->getRow(msaId, 0, os);
    CHECK_NO_ERROR(os);
    QString oldName = sqliteDbi->getSequenceDbi()->getSequenceObject(oldRow.sequenceId, os).visualName;
    CHECK_NO_ERROR(os);
    QString newName = oldName + "_new";
    sqliteDbi->getMsaDbi()->updateRowName(msaId, 0, newName, os);
    CHECK_NO_ERROR(os);

    // Verify name
    U2MsaRow newRow = sqliteDbi->getMsaDbi()->getRow(msaId, 0, os);
    CHECK_NO_ERROR(os);
    QString newestName = sqliteDbi->getSequenceDbi()->getSequenceObject(newRow.sequenceId, os).visualName;
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newName, newestName, "name");

    // Verify version
    int newVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion + 1, newVersion, "version");

    // Verify no modification steps
    qint64 modStepsNum = MsaSQLiteSpecificTestData::getModStepsNum(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(0, modStepsNum, "mod steps num");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowName_undo) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int oldVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update row name 
    U2MsaRow oldRow = sqliteDbi->getMsaDbi()->getRow(msaId, 1, os);
    CHECK_NO_ERROR(os);
    QString oldName = sqliteDbi->getSequenceDbi()->getSequenceObject(oldRow.sequenceId, os).visualName;
    CHECK_NO_ERROR(os);
    QString newName = oldName + "_new";
    sqliteDbi->getMsaDbi()->updateRowName(msaId, 1, newName, os);
    CHECK_NO_ERROR(os);

    // Verify name
    U2MsaRow newRow = sqliteDbi->getMsaDbi()->getRow(msaId, 1, os);
    CHECK_NO_ERROR(os);
    QString newestName = sqliteDbi->getSequenceDbi()->getSequenceObject(newRow.sequenceId, os).visualName;
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newName, newestName, "name");

    // Verify version
    int newVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion + 1, newVersion, "version");

    // Verify the modification step
    QString expectedModDetails = "0\t" + QByteArray::number(newRow.rowId) + "\t2\t2_new";
    U2ModStep modStep = sqliteDbi->getModDbi()->getModStep(msaId, oldVersion, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(msaId, modStep.objectId, "object id");
    CHECK_EQUAL(oldVersion, modStep.version, "version in mod step");
    CHECK_EQUAL(U2ModType::msaUpdatedRowName, modStep.modType, "mod step type");
    CHECK_EQUAL(expectedModDetails, QString(modStep.details), "mod step details");

    // Undo
    sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify name
    U2MsaRow undoRow = sqliteDbi->getMsaDbi()->getRow(msaId, 1, os);
    CHECK_NO_ERROR(os);
    QString undoName = sqliteDbi->getSequenceDbi()->getSequenceObject(undoRow.sequenceId, os).visualName;
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldName, undoName, "name after undo");

    // Verify version
    int undoVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion, undoVersion, "version after undo");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowName_redo) {
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int oldVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Update row name 
    U2MsaRow oldRow = sqliteDbi->getMsaDbi()->getRow(msaId, 1, os);
    CHECK_NO_ERROR(os);
    QString oldName = sqliteDbi->getSequenceDbi()->getSequenceObject(oldRow.sequenceId, os).visualName;
    CHECK_NO_ERROR(os);
    QString newName = oldName + "_new_new";
    sqliteDbi->getMsaDbi()->updateRowName(msaId, 1, newName, os);
    CHECK_NO_ERROR(os);

    // Undo
    sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
    CHECK_NO_ERROR(os);

    // Redo
    sqliteDbi->getSQLiteObjectDbi()->redo(msaId, os);
    CHECK_NO_ERROR(os);

    // Verify name
    U2MsaRow redoRow = sqliteDbi->getMsaDbi()->getRow(msaId, 1, os);
    CHECK_NO_ERROR(os);
    QString redoName = sqliteDbi->getSequenceDbi()->getSequenceObject(redoRow.sequenceId, os).visualName;
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(newName, redoName, "name after undo");

    // Verify version
    int redoVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(oldVersion + 1, redoVersion, "version after undo");

    // Verify the modification step
    QString expectedModDetails = "0\t" + QByteArray::number(redoRow.rowId) + "\t2\t2_new_new";
    U2ModStep modStep = sqliteDbi->getModDbi()->getModStep(msaId, oldVersion, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(msaId, modStep.objectId, "object id");
    CHECK_EQUAL(oldVersion, modStep.version, "version in mod step");
    CHECK_EQUAL(U2ModType::msaUpdatedRowName, modStep.modType, "mod step type");
    CHECK_EQUAL(expectedModDetails, QString(modStep.details), "mod step details");
}

IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, updateRowName_severalSteps) {
    //6 changes, 3 undo steps, 1 redo steps, 4 undo step, 3 redo steps
    U2OpStatusImpl os;
    SQLiteDbi *sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();
    U2DataId msaId = MsaSQLiteSpecificTestData::createTestMsa(true, os);
    CHECK_NO_ERROR(os);

    // Get current version
    int objVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);

    // Get row ids
    QList<qint64> rowIds = sqliteDbi->getMsaDbi()->getRowsOrder(msaId, os);
    CHECK_NO_ERROR(os);

    // Prepare value list
    QStringList rowNames;
    U2MsaRow oldRow = sqliteDbi->getMsaDbi()->getRow(msaId, rowIds[1], os);
    CHECK_NO_ERROR(os);
    rowNames << sqliteDbi->getSequenceDbi()->getSequenceObject(oldRow.sequenceId, os).visualName;
    CHECK_NO_ERROR(os);

    for (int i = 0; i < 6; ++i) {
        rowNames << rowNames[i] + "test";
    }

    // Steps count
    int valuesCount = rowNames.length();     // changes = valuesCount - 1;
    QList<int> steps;                        // negative - undo steps, positive - redo steps;
    steps << -3 << 1 << -4 << 3;
    int expectedIndex = valuesCount - 1;
    for (int i = 0; i < steps.length(); ++i) {
        expectedIndex += steps[i];
    }

    // Update row name
    for (int i = 1; i < rowNames.length(); ++i) {
        sqliteDbi->getMsaDbi()->updateRowName(msaId, rowIds[1], rowNames[i], os);
        CHECK_NO_ERROR(os);
    }

    // Undo and redo row order updating
    int totalUndo = 0;
    int totalRedo = 0;
    for (int i  = 0; i < steps.length(); ++i) {
        if (steps[i] < 0) {
            for (int j = 0; j < -steps[i]; ++j) {
                sqliteDbi->getSQLiteObjectDbi()->undo(msaId, os);
                CHECK_NO_ERROR(os);
                totalUndo++;
            }
        } else {
            for (int j = 0; j < steps[i]; ++j) {
                sqliteDbi->getSQLiteObjectDbi()->redo(msaId, os);
                CHECK_NO_ERROR(os);
                totalRedo++;
            }
        }
    }

    // Verify name
    U2MsaRow finalRow = sqliteDbi->getMsaDbi()->getRow(msaId, rowIds[1], os);
    CHECK_NO_ERROR(os);
    QString finalName = sqliteDbi->getSequenceDbi()->getSequenceObject(finalRow.sequenceId, os).visualName;
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(rowNames[expectedIndex], finalName, "name after all");

    // Verify version
    int finalVersion = sqliteDbi->getObjectDbi()->getObjectVersion(msaId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(objVersion + expectedIndex, finalVersion, "version after all");
}

} // namespace
