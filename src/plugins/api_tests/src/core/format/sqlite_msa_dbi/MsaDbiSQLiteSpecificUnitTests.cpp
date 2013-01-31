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
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Formats/SQLiteDbi.h>


namespace U2 {

TestDbiProvider MsaSQLiteSpecificTestData::dbiProvider = TestDbiProvider();
const QString& MsaSQLiteSpecificTestData::MSA_SQLITE_DB_URL("msa-sqlite-dbi.ugenedb");
U2MsaDbi* MsaSQLiteSpecificTestData::msaDbi = NULL;
U2SequenceDbi* MsaSQLiteSpecificTestData::sequenceDbi = NULL;
SQLiteDbi* MsaSQLiteSpecificTestData::sqliteDbi = NULL;

void MsaSQLiteSpecificTestData::init() {
    SAFE_POINT(NULL == sqliteDbi, "sqliteDbi has already been initialized!", );
    SAFE_POINT(NULL == msaDbi, "msaDbi has been already initialized!", );
    SAFE_POINT(NULL == sequenceDbi, "sequenceDbi has been already initialized!", );

    // Get URL
    bool ok = dbiProvider.init(MSA_SQLITE_DB_URL, false);
    SAFE_POINT(ok, "Dbi provider failed to initialize in MsaTestData::init()!",);

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

    msaDbi = sqliteDbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "Failed to get msaDbi!",);

    sequenceDbi = sqliteDbi->getSequenceDbi();
    SAFE_POINT(NULL != sequenceDbi, "Failed to get sequenceDbi!",);
}

void MsaSQLiteSpecificTestData::shutdown() {
    if (NULL != sqliteDbi) {
        SAFE_POINT(NULL != msaDbi, "msaDbi must also be not NULL on this step!", );
        SAFE_POINT(NULL != sequenceDbi, "sequenceDbi must also be not NULL on this step!", );

        delete sqliteDbi;

        sqliteDbi = NULL;
        msaDbi = NULL;
        sequenceDbi = NULL;
    }
}

SQLiteDbi* MsaSQLiteSpecificTestData::getSQLiteDbi() {
    if (NULL == sqliteDbi) {
        init();
    }
    return sqliteDbi;
}

U2MsaDbi* MsaSQLiteSpecificTestData::getMsaDbi() {
    if (NULL == msaDbi) {
        init();
    }
    return msaDbi;
}

U2SequenceDbi* MsaSQLiteSpecificTestData::getSequenceDbi() {
    if (NULL == sequenceDbi) {
        init();
    }
    return sequenceDbi;
}


IMPLEMENT_TEST(MsaDbiSQLiteSpecificUnitTests, removeRow) {
    U2OpStatusImpl os;
    U2MsaDbi* msaDbi = MsaSQLiteSpecificTestData::getMsaDbi();

    // Create an alignment
    U2Msa al;
    al.alphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
    al.length = 5;
    msaDbi->createMsaObject(al, "", os);
    CHECK_NO_ERROR(os);

    // Create sequences
    U2SequenceDbi* sequenceDbi = MsaSQLiteSpecificTestData::getSequenceDbi();
    U2Sequence seq1;
    U2Sequence seq2;
    sequenceDbi->createSequenceObject(seq1, "", os);
    CHECK_NO_ERROR(os);
    sequenceDbi->createSequenceObject(seq2, "", os);
    CHECK_NO_ERROR(os);

    // Add rows
    U2MsaRow row1;
    row1.rowId = 0;
    row1.sequenceId = seq1.id;
    row1.gstart = 0;
    row1.gend = 5;

    U2MsaGap row1gap1(0, 2);
    U2MsaGap row1gap2(3, 1);
    QList<U2MsaGap> row1gaps;
    row1gaps << row1gap1 << row1gap2;

    row1.gaps = row1gaps;

    U2MsaRow row2;
    row2.rowId = 1;
    row2.sequenceId = seq2.id;
    row2.gstart = 2;
    row2.gend = 4;

    U2MsaGap row2gap(1, 2);
    QList<U2MsaGap> row2gaps;
    row2gaps << row2gap;

    row2.gaps = row2gaps;

    QList<U2MsaRow> rows;
    rows << row1 << row2;

    msaDbi->addRows(al.id, rows, os);
    CHECK_NO_ERROR(os);

    // Remove the first row
    QList<U2MsaRow> rowsToRemove;
    rowsToRemove << row1;

    msaDbi->removeRows(al.id, rowsToRemove, os);
    CHECK_NO_ERROR(os);

    // Get the number of rows
    qint64 actualNumOfRows = msaDbi->getNumOfRows(al.id, os);
    CHECK_EQUAL(1, actualNumOfRows, "number of rows");

    // Get the rows
    QList<U2MsaRow> actualRows = msaDbi->getRows(al.id, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(1, actualRows.count(), "number of rows");

    const U2MsaRow& actualRow = actualRows[0];
    CHECK_EQUAL(1, actualRow.rowId, "row id");
    CHECK_EQUAL(seq2.id, actualRow.sequenceId, "row sequence id");
    CHECK_EQUAL(2, actualRow.gstart, "row global start");
    CHECK_EQUAL(4, actualRow.gend, "row global end");
    CHECK_EQUAL(1, actualRow.gaps.count(), "row gaps");
    U2MsaGap actualRowGap = actualRow.gaps[0];
    CHECK_EQUAL(1, actualRowGap.offset, "row gap offset");
    CHECK_EQUAL(2, actualRowGap.gap, "row gap length");

    // Verify SQLite structure is correct
    //
    SQLiteDbi* sqliteDbi = MsaSQLiteSpecificTestData::getSQLiteDbi();

    // Verify that gaps of the removed row were removed
    SQLiteQuery qGaps("SELECT COUNT(*) FROM MsaRowGap WHERE msa = ?1 AND rowId = ?2", sqliteDbi->getDbRef(), os);
    qGaps.bindDataId(1, al.id);
    qGaps.bindInt64(2, row1.rowId);
    qint64 actualGapsNumber = qGaps.selectInt64();
    CHECK_EQUAL(0, actualGapsNumber, "removed row gaps number");

    // Verify that sequence of the removed row was removed
    SQLiteQuery qSeq("SELECT COUNT(*) FROM Sequence WHERE object = ?1", sqliteDbi->getDbRef(), os);
    qSeq.bindDataId(1, row1.sequenceId);
    qint64 actualSeqNumber = qSeq.selectInt64();
    CHECK_EQUAL(0, actualSeqNumber, "sequence");

    // Verify that the sequence object was also removed
    SQLiteQuery qSeqObj("SELECT COUNT(*) FROM Object WHERE id = ?1", sqliteDbi->getDbRef(), os);
    qSeqObj.bindDataId(1, row1.sequenceId);
    qint64 actualSeqObjNumber = qSeqObj.selectInt64();
    CHECK_EQUAL(0, actualSeqObjNumber, "sequence object");
}

} // namespace
