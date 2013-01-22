/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "SQLiteObjectDbiUnitTests.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Formats/SQLiteDbi.h>
#include <U2Formats/SQLiteObjectDbi.h>


namespace U2 {

TestDbiProvider SQLiteObjectDbiTestData::dbiProvider = TestDbiProvider();
const QString& SQLiteObjectDbiTestData::SQLITE_OBJ_DB_URL("sqlite-obj-dbi.ugenedb");
U2AttributeDbi* SQLiteObjectDbiTestData::attributeDbi = NULL;
U2MsaDbi* SQLiteObjectDbiTestData::msaDbi = NULL;
U2SequenceDbi* SQLiteObjectDbiTestData::sequenceDbi = NULL;
SQLiteDbi* SQLiteObjectDbiTestData::sqliteDbi = NULL;
SQLiteObjectDbi* SQLiteObjectDbiTestData::sqliteObjectDbi = NULL;

void SQLiteObjectDbiTestData::init() {
    SAFE_POINT(NULL == sqliteDbi, "sqliteDbi has already been initialized!", );

    // Get URL
    bool ok = dbiProvider.init(SQLITE_OBJ_DB_URL, false);
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

    sqliteObjectDbi = sqliteDbi->getSQLiteObjectDbi();
    SAFE_POINT(NULL != sqliteObjectDbi, "Failed to get sqliteObjectDbi!",);

    attributeDbi = sqliteDbi->getAttributeDbi();
    SAFE_POINT(NULL != attributeDbi, "Failed to get attributeDbi!",);

    msaDbi = sqliteDbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "Failed to get msaDbi!",);

    sequenceDbi = sqliteDbi->getSequenceDbi();
    SAFE_POINT(NULL != sequenceDbi, "Failed to get sequenceDbi!",);
}

void SQLiteObjectDbiTestData::shutdown() {
    if (NULL != sqliteDbi) {

        delete sqliteDbi;

        sqliteDbi = NULL;
        sqliteObjectDbi = NULL;
        attributeDbi = NULL;
        msaDbi = NULL;
        sequenceDbi = NULL;
    }
}

SQLiteDbi* SQLiteObjectDbiTestData::getSQLiteDbi() {
    if (NULL == sqliteDbi) {
        init();
    }
    return sqliteDbi;
}

SQLiteObjectDbi* SQLiteObjectDbiTestData::getSQLiteObjectDbi() {
    if (NULL == sqliteObjectDbi) {
        init();
    }
    return sqliteObjectDbi;
}

U2AttributeDbi* SQLiteObjectDbiTestData::getAttributeDbi() {
    if (NULL == attributeDbi) {
        init();
    }
    return attributeDbi;
}

U2MsaDbi* SQLiteObjectDbiTestData::getMsaDbi() {
    if (NULL == msaDbi) {
        init();
    }
    return msaDbi;
}

U2SequenceDbi* SQLiteObjectDbiTestData::getSequenceDbi() {
    if (NULL == sequenceDbi) {
        init();
    }
    return sequenceDbi;
}


IMPLEMENT_TEST(SQLiteObjectDbiUnitTests, removeMsaObject) {
    U2OpStatusImpl os;
    U2MsaDbi* msaDbi = SQLiteObjectDbiTestData::getMsaDbi();

    // FIRST ALIGNMENT
    // Create an alignment
    U2Msa al;
    al.alphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
    al.length = 5;
    msaDbi->createMsaObject(al, "", os);
    CHECK_NO_ERROR(os);

    // Add alignment info
    U2StringAttribute attr(al.id, "MSA1 info key", "MSA1 info value");
    U2AttributeDbi* attrDbi = SQLiteObjectDbiTestData::getAttributeDbi();
    attrDbi->createStringAttribute(attr, os);
    CHECK_NO_ERROR(os);

    // Create sequences
    U2SequenceDbi* sequenceDbi = SQLiteObjectDbiTestData::getSequenceDbi();
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

    // SECOND ALIGNMENT
    // Create an alignment
    U2Msa al2;
    al2.alphabet = BaseDNAAlphabetIds::AMINO_DEFAULT();
    al2.length = 6;
    msaDbi->createMsaObject(al2, "", os);
    CHECK_NO_ERROR(os);

    // Add alignment info
    U2StringAttribute attr2(al2.id, "MSA2 info key", "MSA2 info value");
    attrDbi->createStringAttribute(attr2, os);
    CHECK_NO_ERROR(os);

    // Create sequences
    U2Sequence al2Seq;
    sequenceDbi->createSequenceObject(al2Seq, "", os);
    CHECK_NO_ERROR(os);

    // Add rows
    U2MsaRow al2Row;
    al2Row.rowId = 0;
    al2Row.sequenceId = al2Seq.id;
    al2Row.gstart = 0;
    al2Row.gend = 15;

    U2MsaGap al2RowGap(1, 12);
    QList<U2MsaGap> al2RowGaps;
    al2RowGaps << al2RowGap;

    al2Row.gaps = al2RowGaps;

    QList<U2MsaRow> al2Rows;
    al2Rows << al2Row;

    msaDbi->addRows(al2.id, al2Rows, os);
    CHECK_NO_ERROR(os);

    // REMOVE THE FIRST ALIGNMENT OBJECT
    SQLiteObjectDbi* sqliteObjectDbi = SQLiteObjectDbiTestData::getSQLiteObjectDbi();
    sqliteObjectDbi->removeObject(al.id, "", os);

    // VERIFY THAT THERE IS ONLY THE SECOND ALIGNMENT'S RECORDS LEFT IN TABLES
    SQLiteDbi* sqliteDbi = SQLiteObjectDbiTestData::getSQLiteDbi();

    // "Attribute"
    SQLiteQuery qAttr("SELECT COUNT(*) FROM Attribute WHERE name = ?1", sqliteDbi->getDbRef(), os);
    qAttr.bindString(1, "MSA1 info key");
    qint64 msa1AttrNum = qAttr.selectInt64();
    CHECK_EQUAL(0, msa1AttrNum, "MSA1 attributes number");

    qAttr.reset(true);
    qAttr.bindString(1, "MSA2 info key");
    qint64 msa2AttrNum = qAttr.selectInt64();
    CHECK_EQUAL(1, msa2AttrNum, "MSA2 attributes number");

    // "StringAttribute"
    SQLiteQuery qStringAttr("SELECT COUNT(*) FROM StringAttribute WHERE value = ?1", sqliteDbi->getDbRef(), os);
    qStringAttr.bindString(1, "MSA1 info value");
    qint64 msa1StrAttrNum = qStringAttr.selectInt64();
    CHECK_EQUAL(0, msa1StrAttrNum, "MSA1 string attributes number");

    qStringAttr.reset(true);
    qStringAttr.bindString(1, "MSA2 info value");
    qint64 msa2StrAttrNum = qStringAttr.selectInt64();
    CHECK_EQUAL(1, msa2StrAttrNum, "MSA2 string attributes number");

    // "MsaRow"
    SQLiteQuery qMsaRow("SELECT COUNT(*) FROM MsaRow WHERE msa = ?1", sqliteDbi->getDbRef(), os);
    qMsaRow.bindDataId(1, al.id);
    qint64 msa1Rows = qMsaRow.selectInt64();
    CHECK_EQUAL(0, msa1Rows, "number of rows in MSA1");

    qMsaRow.reset(true);
    qMsaRow.bindDataId(1, al2.id);
    qint64 msa2Rows = qMsaRow.selectInt64();
    CHECK_EQUAL(1, msa2Rows, "number of rows in MSA2");

    // "MsaRowGap"
    SQLiteQuery qMsaRowGap("SELECT COUNT(*) FROM MsaRowGap WHERE msa = ?1", sqliteDbi->getDbRef(), os);
    qMsaRowGap.bindDataId(1, al.id);
    qint64 msa1Gaps = qMsaRowGap.selectInt64();
    CHECK_EQUAL(0, msa1Gaps, "number of gaps in MSA1 rows");

    qMsaRowGap.reset(true);
    qMsaRowGap.bindDataId(1, al2.id);
    qint64 msa2Gaps = qMsaRowGap.selectInt64();
    CHECK_EQUAL(1, msa2Gaps, "number of gaps in MSA2 rows");

    // "Sequence"
    SQLiteQuery qSeq("SELECT COUNT(*) FROM Sequence WHERE object = ?1", sqliteDbi->getDbRef(), os);
    qSeq.bindDataId(1, seq1.id);
    qint64 msa1seq1 = qSeq.selectInt64();
    CHECK_EQUAL(0, msa1seq1, "seq1 of msa1");

    qSeq.reset(true);
    qSeq.bindDataId(1, seq2.id);
    qint64 msa1seq2 = qSeq.selectInt64();
    CHECK_EQUAL(0, msa1seq2, "seq2 of msa1");

    qSeq.reset(true);
    qSeq.bindDataId(1, al2Seq.id);
    qint64 msa2seq = qSeq.selectInt64();
    CHECK_EQUAL(1, msa2seq, "seq of msa2");

    // "Msa"
    SQLiteQuery qMsa("SELECT COUNT(*) FROM Msa WHERE object = ?1", sqliteDbi->getDbRef(), os);
    qMsa.bindDataId(1, al.id);
    qint64 msa1records = qMsa.selectInt64();
    CHECK_EQUAL(0, msa1records, "number of MSA1 records");

    qMsa.reset(true);
    qMsa.bindDataId(1, al2.id);
    qint64 msa2records = qMsa.selectInt64();
    CHECK_EQUAL(1, msa2records, "number of MSA2 records");

    // "Object"
    SQLiteQuery qObj("SELECT COUNT(*) FROM Object WHERE id = ?1", sqliteDbi->getDbRef(), os);
    qObj.bindDataId(1, al.id);
    qint64 msa1objects = qObj.selectInt64();
    CHECK_EQUAL(0, msa1objects, "number of MSA1 objects");

    qObj.reset(true);
    qObj.bindDataId(1, al2.id);
    qint64 msa2objects = qObj.selectInt64();
    CHECK_EQUAL(1, msa2objects, "number of MSA2 objects");
}

} // namespace
