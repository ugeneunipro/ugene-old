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

#include "MsaDbiUtilsTests.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/MAlignmentExporter.h>

namespace U2 {

const QString MsaDbiUtilsTestUtils::alignmentName = QString("MsaDbiUtils test alignment");
TestDbiProvider MsaDbiUtilsTestUtils::dbiProvider = TestDbiProvider();
const QString& MsaDbiUtilsTestUtils::MSA_DB_URL("msa-dbi-utils.ugenedb");
U2MsaDbi* MsaDbiUtilsTestUtils::msaDbi = NULL;
U2SequenceDbi* MsaDbiUtilsTestUtils::sequenceDbi = NULL;


void MsaDbiUtilsTestUtils::init() {
    SAFE_POINT(NULL == msaDbi, "msaDbi has been already initialized!", );
    SAFE_POINT(NULL == sequenceDbi, "sequenceDbi has been already initialized!", );

    bool ok = dbiProvider.init(MSA_DB_URL, true);
    SAFE_POINT(ok, "Dbi provider failed to initialize in MsaTestData::init()!",);

    U2Dbi* dbi = dbiProvider.getDbi();
    msaDbi = dbi->getMsaDbi();
    SAFE_POINT(NULL != msaDbi, "Failed to get msaDbi!",);

    sequenceDbi = dbi->getSequenceDbi();
    SAFE_POINT(NULL != sequenceDbi, "Failed to get sequenceDbi!",);
}

void MsaDbiUtilsTestUtils::shutdown() {
    if (NULL != msaDbi) {
        SAFE_POINT(NULL != sequenceDbi, "sequenceDbi must also be not NULL on this step!", );

        U2OpStatusImpl os;
        dbiProvider.close();
        msaDbi = NULL;
        sequenceDbi = NULL;
        SAFE_POINT_OP(os, );
    }
}

U2MsaDbi* MsaDbiUtilsTestUtils::getMsaDbi() {
    if (NULL == msaDbi) {
        init();
    }
    return msaDbi;
}

U2SequenceDbi* MsaDbiUtilsTestUtils::getSequenceDbi() {
    if (NULL == sequenceDbi) {
        init();
    }
    return sequenceDbi;
}


U2EntityRef MsaDbiUtilsTestUtils::initTestAlignment(const qint64 rowCount) {
    getMsaDbi();
    getSequenceDbi();

    SAFE_POINT(NULL != msaDbi, "MsaDbi is NULL", U2EntityRef());
    SAFE_POINT(NULL != sequenceDbi, "SequenceDbi is NULL", U2EntityRef());

    U2OpStatusImpl os;
    U2DataId msaId = msaDbi->createMsaObject("", MsaDbiUtilsTestUtils::alignmentName, BaseDNAAlphabetIds::NUCL_DNA_DEFAULT(), os);
    CHECK_OP(os, U2EntityRef());

    QList<U2MsaRow> rows;
    for (int i = 0; i < rowCount; ++i) {
        U2Sequence sequence;
        sequence.alphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
        sequence.visualName = QString::number(i);
        sequenceDbi->createSequenceObject(sequence, "", os);
        CHECK_OP(os, U2EntityRef());

        U2MsaRow row;
        row.rowId = i;
        row.sequenceId = sequence.id;
        row.gstart = 0;
        row.gend = 5;

        rows << row;
    }

    msaDbi->addRows(msaId, rows, os);
    CHECK_OP(os, U2EntityRef());

    U2EntityRef msaRef(msaDbi->getRootDbi()->getDbiRef(), msaId);
    return msaRef;
}

U2EntityRef MsaDbiUtilsTestUtils::initTestAlignment(const QStringList& rowsData = QStringList()) {
    getMsaDbi();
    getSequenceDbi();

    SAFE_POINT(NULL != msaDbi, "MsaDbi is NULL", U2EntityRef());
    SAFE_POINT(NULL != sequenceDbi, "SequenceDbi is NULL", U2EntityRef());

    U2OpStatusImpl os;
    U2DataId msaId = msaDbi->createMsaObject("", MsaDbiUtilsTestUtils::alignmentName, BaseDNAAlphabetIds::NUCL_DNA_DEFAULT(), os);
    CHECK_OP(os, U2EntityRef());

    qint64 rowCount = rowsData.length();

    QList<U2MsaRow> rows;
    for (int i = 0; i < rowCount; ++i) {
        QByteArray seqData;
        QList<U2MsaGap> gapModel;
        MsaDbiUtils::splitBytesToCharsAndGaps(rowsData[i].toLatin1(), seqData, gapModel);

        U2Sequence sequence;
        sequence.alphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
        sequence.visualName = QString::number(i);
        sequence.length = rowsData[i].length();
        sequenceDbi->createSequenceObject(sequence, "", os);
        CHECK_OP(os, U2EntityRef());
        sequenceDbi->updateSequenceData(sequence.id, U2Region(0, 0), seqData, QVariantMap(), os);
        CHECK_OP(os, U2EntityRef());

        U2MsaRow row;
        row.rowId = i;
        row.sequenceId = sequence.id;
        row.gstart = 0;
        row.gend = seqData.length();
        row.gaps = gapModel;

        rows << row;
    }

    msaDbi->addRows(msaId, rows, os);
    CHECK_OP(os, U2EntityRef());

    U2EntityRef msaRef(msaDbi->getRootDbi()->getDbiRef(), msaId);
    return msaRef;
}

U2EntityRef MsaDbiUtilsTestUtils::initTestAlignment(QList<U2MsaRow>& rows) {
    getMsaDbi();
    getSequenceDbi();

    SAFE_POINT(NULL != msaDbi, "MsaDbi is NULL", U2EntityRef());
    SAFE_POINT(NULL != sequenceDbi, "SequenceDbi is NULL", U2EntityRef());

    U2OpStatusImpl os;
    U2DataId msaId = msaDbi->createMsaObject("", MsaDbiUtilsTestUtils::alignmentName, BaseDNAAlphabetIds::NUCL_DNA_DEFAULT(), os);
    CHECK_OP(os, U2EntityRef());

    msaDbi->addRows(msaId, rows, os);
    CHECK_OP(os, U2EntityRef());

    U2EntityRef msaRef(msaDbi->getRootDbi()->getDbiRef(), msaId);
    return msaRef;
}

U2MsaRow MsaDbiUtilsTestUtils::addRow(const U2DataId &msaId, qint64 num, const QByteArray &name, const QByteArray &seq, const QList<U2MsaGap> &gaps, U2OpStatus &os) {
    U2Sequence sequence;
    sequence.alphabet = BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
    sequence.visualName = name;
    sequenceDbi->createSequenceObject(sequence, "", os);
    CHECK_OP(os, U2MsaRow());

    U2Region reg(0, 0);
    sequenceDbi->updateSequenceData(sequence.id, reg, seq, QVariantMap(), os);
    CHECK_OP(os, U2MsaRow());

    U2MsaRow row;
    row.rowId = num;
    row.sequenceId = sequence.id;
    row.gstart = 0;
    row.gend = seq.length();
    row.gaps = gaps;
    return row;
}

U2EntityRef MsaDbiUtilsTestUtils::removeRegionTestAlignment(U2OpStatus &os) {
    U2MsaDbi *msaDbi = getMsaDbi();
    U2SequenceDbi *sequenceDbi = getSequenceDbi();
    SAFE_POINT(NULL != msaDbi, "MsaDbi is NULL", U2EntityRef());
    SAFE_POINT(NULL != sequenceDbi, "SequenceDbi is NULL", U2EntityRef());

    U2DataId msaId = msaDbi->createMsaObject("", MsaDbiUtilsTestUtils::alignmentName, BaseDNAAlphabetIds::NUCL_DNA_DEFAULT(), os);
    CHECK_OP(os, U2EntityRef());

    QList<U2MsaRow> rows;
    rows << addRow(msaId, 0, "1", "TAAGACTTCTAA", QList<U2MsaGap>() << U2MsaGap(12, 2), os);
    rows << addRow(msaId, 1, "2", "TAAGCTTACTA", QList<U2MsaGap>() << U2MsaGap(11, 3), os);
    rows << addRow(msaId, 2, "3", "TTAGTTTATTA", QList<U2MsaGap>() << U2MsaGap(11, 3), os);
    rows << addRow(msaId, 3, "4", "TCAGTCTATTA", QList<U2MsaGap>() << U2MsaGap(1, 2) << U2MsaGap(5, 1), os);
    rows << addRow(msaId, 4, "5", "TCAGTTTATTA", QList<U2MsaGap>() << U2MsaGap(1, 2) << U2MsaGap(5, 1), os);
    rows << addRow(msaId, 5, "6", "TTAGTCTACTA", QList<U2MsaGap>() << U2MsaGap(1, 2) << U2MsaGap(5, 1), os);
    rows << addRow(msaId, 6, "7", "TCAGATTATTA", QList<U2MsaGap>() << U2MsaGap(1, 2) << U2MsaGap(5, 1), os);
    rows << addRow(msaId, 7, "8", "TTAGATTGCTA", QList<U2MsaGap>() << U2MsaGap(1, 1) << U2MsaGap(12, 2), os);
    rows << addRow(msaId, 8, "9", "TTAGATTATTA", QList<U2MsaGap>() << U2MsaGap(11, 3), os);
    rows << addRow(msaId, 9, "10", "", QList<U2MsaGap>() << U2MsaGap(0, 14), os);
    rows << addRow(msaId, 10, "11", "", QList<U2MsaGap>() << U2MsaGap(0, 14), os);
    rows << addRow(msaId, 11, "12", "", QList<U2MsaGap>() << U2MsaGap(0, 14), os);
    rows << addRow(msaId, 12, "13", "", QList<U2MsaGap>() << U2MsaGap(0, 14), os);
    CHECK_OP(os, U2EntityRef());

    msaDbi->addRows(msaId, rows, os);
    CHECK_OP(os, U2EntityRef());

    U2EntityRef msaRef(msaDbi->getRootDbi()->getDbiRef(), msaId);
    return msaRef;
}

QStringList MsaDbiUtilsTestUtils::getRowNames(U2EntityRef msaRef) {
    SAFE_POINT(msaRef.isValid(), "msaRef no valid", QStringList())
    U2OpStatusImpl os;
    QStringList res;

    QList<U2MsaRow> rows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_OP(os, QStringList());
    foreach (U2MsaRow row, rows) {
        res << sequenceDbi->getSequenceObject(row.sequenceId, os).visualName;
        CHECK_OP(os, QStringList());
    }

    return res;
}


IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_oneLineInMiddle) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(3, 2);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId;

    int delta = -1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step up");

    expected.move(3, 4);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId;

    delta = 1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step down");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_oneLineInMiddleToTheTop) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(3, 0);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId;

    int delta = -3;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step up to i");

    expected.move(3, 0);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId;

    delta = -4;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step up to i+1");

    expected.move(3, 1);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId;

    delta = -2;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step up to i-1");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_oneLineInMiddleToTheBottom) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(3, 6);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId;

    int delta = 3;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step down to i");

    expected.move(3, 6);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId;

    delta = 4;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step down to i+1");

    expected.move(3, 5);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId;

    delta = 2;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step down to i-1");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_oneBlockInMiddle) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(3, 2);
    expected.move(4, 3);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId << allRows[4].rowId;

    int delta = -1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step up");

    expected.move(4, 5);
    expected.move(3, 4);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId << allRows[4].rowId;

    delta = 1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step down");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_oneBlockInMiddleToTheOutside) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(3, 0);
    expected.move(4, 1);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId << allRows[4].rowId;

    int delta = -9;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step up");

    expected.move(4, 6);
    expected.move(3, 5);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId << allRows[4].rowId;

    delta = 9;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step down");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_twoBlocksInMiddleWithoutGluing) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(2, 1);
    expected.move(4, 3);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[2].rowId << allRows[4].rowId;

    int delta = -1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step up");

    expected.move(4, 5);
    expected.move(2, 3);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[2].rowId << allRows[4].rowId;

    delta = 1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step down");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_twoBlocksInMiddleWithGluing) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(2, 0);
    expected.move(4, 1);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[2].rowId << allRows[4].rowId;

    int delta = -9;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step up");

    expected.move(4, 6);
    expected.move(2, 5);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[2].rowId << allRows[4].rowId;

    delta = 9;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step down");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_twoBlocksOnTopWithoutGluing) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(0, 0);
    expected.move(3, 2);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[0].rowId << allRows[3].rowId;

    int delta = -1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step up");

    expected.move(0, 1);
    expected.move(3, 4);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[0].rowId << allRows[3].rowId;

    delta = 1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step down");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_twoBlocksOnTopWithGluing) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(0, 0);
    expected.move(3, 1);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[0].rowId << allRows[3].rowId;

    int delta = -9;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step up");

    expected.move(3, 6);
    expected.move(0, 5);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[0].rowId << allRows[3].rowId;

    delta = 9;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step down");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_twoBlocksOnBottomWithoutGluing) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(6, 6);
    expected.move(3, 4);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId << allRows[6].rowId;

    int delta = 1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step down");

    expected.move(6, 5);
    expected.move(3, 2);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId << allRows[6].rowId;

    delta = -1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step up");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_twoBlocksOnBottomWithGluing) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(6, 6);
    expected.move(3, 5);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId << allRows[6].rowId;

    int delta = 9;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step down");

    expected.move(3, 0);
    expected.move(6, 1);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[3].rowId << allRows[6].rowId;

    delta = -9;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step up");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_threeBlocksWithoutGluing) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(0, 0);
    expected.move(4, 3);
    expected.move(6, 5);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[0].rowId << allRows[4].rowId << allRows[6].rowId;

    int delta = -1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step up");

    expected.move(0, 1);
    expected.move(2, 3);
    expected.move(6, 6);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[0].rowId << allRows[2].rowId << allRows[6].rowId;

    delta = 1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step down");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_threeBlocksWithOnceGluing) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(0, 0);
    expected.move(2, 1);
    expected.move(6, 3);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[0].rowId << allRows[2].rowId << allRows[6].rowId;

    int delta = -3;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step up");

    expected.move(6, 6);
    expected.move(2, 5);
    expected.move(0, 3);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[0].rowId << allRows[2].rowId << allRows[6].rowId;

    delta = 3;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step down");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_threeBlocksWithTwiceGluing) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);
    QStringList expected = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    expected.move(0, 0);
    expected.move(2, 1);
    expected.move(6, 2);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[0].rowId << allRows[2].rowId << allRows[6].rowId;

    int delta = -9;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    QStringList actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    if (expected.length() != rowCount) {
        SetError("Unexpected count of rows");
        return;
    }
    CHECK_TRUE(expected == actual, "step up");

    expected.move(6, 6);
    expected.move(2, 5);
    expected.move(0, 4);
    rowsToMove.clear();
    allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[0].rowId << allRows[2].rowId << allRows[6].rowId;

    delta = 9;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    CHECK_NO_ERROR(os);

    actual = MsaDbiUtilsTestUtils::getRowNames(msaRef);
    CHECK_TRUE(expected == actual, "step down");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_UnorderedList) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);

    QList<qint64> rowsToMove;
    QList<U2MsaRow> allRows = msaDbi->getRows(msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    rowsToMove << allRows[5].rowId << allRows[3].rowId;

    int delta = 1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    if (!(!os.isCanceled() && os.hasError() && os.getError() == "List of rows to move is not ordered")) {
        CHECK_TRUE(false, "List not ordered");
    }
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, moveRows_InvalidRowList) {
    U2MsaDbi* msaDbi = MsaDbiUtilsTestUtils::getMsaDbi();
    U2OpStatusImpl os;
    int rowCount = 7;

    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(rowCount);

    QList<qint64> rowsToMove;
    rowsToMove << -1;

    int delta = 1;
    MsaDbiUtils::moveRows(msaRef, rowsToMove, delta, os);
    if (!(!os.isCanceled() && os.hasError() && os.getError() == "Invalid row list")) {
        CHECK_TRUE(false, "Invalid row list");
    }
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, trim_noGaps) {
    //Init test data
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(QStringList() << "AACCGGTT" << "CCGGTTAA" << "GGTTAACC");

    //Prepare expected state
    QStringList expected = QStringList() << "AACCGGTT" << "CCGGTTAA" << "GGTTAACC";

    //Call test function
    MsaDbiUtils::trim(msaRef, os);

    //Check actual state
    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(8, al.getLength(), "Wrong msa length.");
    CHECK_EQUAL(expected.length(), al.getNumRows(), "Wrong rows count.");

    QStringList actual;
    actual << al.getRow(0).toByteArray(al.getLength(), os);
    actual << al.getRow(1).toByteArray(al.getLength(), os);
    actual << al.getRow(2).toByteArray(al.getLength(), os);
    for (int i = 0; i < expected.length(); ++i) {
        CHECK_EQUAL(expected[i], actual[i], "Wrong msa data.");
    }
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, trim_leadingGaps) {
    //Init test data
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(QStringList() << "--AACCGGTT" << "--CCGGTTAA" << "--GGTTA--C");

    //Prepare expected state
    QStringList expected = QStringList() << "AACCGGTT" << "CCGGTTAA" << "GGTTA--C";

    //Call test function
    MsaDbiUtils::trim(msaRef, os);

    //Check actual state
    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(8, al.getLength(), "Wrong msa length.");
    CHECK_EQUAL(expected.length(), al.getNumRows(), "Wrong rows count.");

    QStringList actual;
    actual << al.getRow(0).toByteArray(al.getLength(), os);
    actual << al.getRow(1).toByteArray(al.getLength(), os);
    actual << al.getRow(2).toByteArray(al.getLength(), os);
    for (int i = 0; i < expected.length(); ++i) {
        CHECK_EQUAL(expected[i], actual[i], "Wrong msa data.");
    }
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, trim_trailingGaps) {
    //Init test data
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(QStringList() << "AACCGGTT--" << "CCG--TAA--" << "GGTTAACC--");

    //Prepare expected state
    QStringList expected = QStringList() << "AACCGGTT" << "CCG--TAA" << "GGTTAACC";

    //Call test function
    MsaDbiUtils::trim(msaRef, os);

    //Check actual state
    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(8, al.getLength(), "Wrong msa length.");
    CHECK_EQUAL(expected.length(), al.getNumRows(), "Wrong rows count.");

    QStringList actual;
    actual << al.getRow(0).toByteArray(al.getLength(), os);
    actual << al.getRow(1).toByteArray(al.getLength(), os);
    actual << al.getRow(2).toByteArray(al.getLength(), os);
    for (int i = 0; i < expected.length(); ++i) {
        CHECK_EQUAL(expected[i], actual[i], "Wrong msa data.");
    }
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, trim_leadingGapsCutOff) {
    //Init test data
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(QStringList() << "---TAACCGG" << "--CCGGTTAA" << "--GGTTAACC");

    //Prepare expected state
    QStringList expected = QStringList() << "-TAACCGG" << "CCGGTTAA" << "GGTTAACC";

    //Call test function
    MsaDbiUtils::trim(msaRef, os);

    //Check actual state
    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(8, al.getLength(), "Wrong msa length.");
    CHECK_EQUAL(expected.length(), al.getNumRows(), "Wrong rows count.");

    QStringList actual;
    actual << al.getRow(0).toByteArray(al.getLength(), os);
    actual << al.getRow(1).toByteArray(al.getLength(), os);
    actual << al.getRow(2).toByteArray(al.getLength(), os);
    for (int i = 0; i < expected.length(); ++i) {
        CHECK_EQUAL(expected[i], actual[i], "Wrong msa data.");
    }
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, trim_trailingGapsCutOff) {
    //Init test data
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(QStringList() << "AACCGGT---" << "CCGGTTAA--" << "GGTTAACC--");

    //Prepare expected state
    QStringList expected = QStringList() << "AACCGGT-" << "CCGGTTAA" << "GGTTAACC";

    //Call test function
    MsaDbiUtils::trim(msaRef, os);

    //Check actual state
    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(8, al.getLength(), "Wrong msa length.");
    CHECK_EQUAL(expected.length(), al.getNumRows(), "Wrong rows count.");

    QStringList actual;
    actual << al.getRow(0).toByteArray(al.getLength(), os);
    actual << al.getRow(1).toByteArray(al.getLength(), os);
    actual << al.getRow(2).toByteArray(al.getLength(), os);
    for (int i = 0; i < expected.length(); ++i) {
        CHECK_EQUAL(expected[i], actual[i], "Wrong msa data.");
    }
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, trim_gapsOnly) {
    //Init test data
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::initTestAlignment(QStringList() << "------" << "----" << "-----");

    //Prepare expected state
    QStringList expected = QStringList() << "" << "" << "";

    //Call test function
    MsaDbiUtils::trim(msaRef, os);

    //Check actual state
    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(0, al.getLength(), "Wrong msa length.");
    CHECK_EQUAL(expected.length(), al.getNumRows(), "Wrong rows count.");

    QStringList actual;
    actual << al.getRow(0).toByteArray(al.getLength(), os);
    actual << al.getRow(1).toByteArray(al.getLength(), os);
    actual << al.getRow(2).toByteArray(al.getLength(), os);
    for (int i = 0; i < expected.length(); ++i) {
        CHECK_EQUAL(expected[i], actual[i], "Wrong msa data.");
    }
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, removeRegion_oneRow) {
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::removeRegionTestAlignment(os);
    CHECK_NO_ERROR(os);

    QList<qint64> rowIds;
    rowIds << 1;
    MsaDbiUtils::removeRegion(msaRef, rowIds, 8, 3, os);
    CHECK_NO_ERROR(os);

    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(14, al.getLength(), "Wrong msa length");

    QByteArray row0 = al.getRow(0).toByteArray(al.getLength(), os);
    QByteArray row1 = al.getRow(1).toByteArray(al.getLength(), os);
    QByteArray row2 = al.getRow(2).toByteArray(al.getLength(), os);
    CHECK_EQUAL(QString("TAAGACTTCTAA--"), QString(row0), "Wrong msa row");
    CHECK_EQUAL(QString("TAAGCTTA------"), QString(row1), "Wrong msa row");
    CHECK_EQUAL(QString("TTAGTTTATTA---"), QString(row2), "Wrong msa row");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, removeRegion_threeRows) {
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::removeRegionTestAlignment(os);
    CHECK_NO_ERROR(os);

    QList<qint64> rowIds;
    rowIds << 1 << 8 << 5;
    MsaDbiUtils::removeRegion(msaRef, rowIds, 2, 8, os);
    CHECK_NO_ERROR(os);

    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(14, al.getLength(), "Wrong msa length");

    QByteArray row1 = al.getRow(1).toByteArray(al.getLength(), os);
    QByteArray row5 = al.getRow(5).toByteArray(al.getLength(), os);
    QByteArray row8 = al.getRow(8).toByteArray(al.getLength(), os);
    CHECK_EQUAL(QString("TAA-----------"), QString(row1), "Wrong msa row");
    CHECK_EQUAL(QString("T-ACTA--------"), QString(row5), "Wrong msa row");
    CHECK_EQUAL(QString("TTA-----------"), QString(row8), "Wrong msa row");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, removeRegion_lengthChange) {
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::removeRegionTestAlignment(os);
    CHECK_NO_ERROR(os);

    QList<qint64> rowIds;
    rowIds << 3 << 4 << 5 << 6;
    MsaDbiUtils::removeRegion(msaRef, rowIds, 5, 1, os);
    CHECK_NO_ERROR(os);

    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(13, al.getLength(), "Wrong msa length");

    QByteArray row3 = al.getRow(3).toByteArray(al.getLength(), os);
    QByteArray row4 = al.getRow(4).toByteArray(al.getLength(), os);
    QByteArray row5 = al.getRow(5).toByteArray(al.getLength(), os);
    QByteArray row6 = al.getRow(6).toByteArray(al.getLength(), os);
    CHECK_EQUAL(QString("T--CAGTCTATTA"), QString(row3), "Wrong msa row");
    CHECK_EQUAL(QString("T--CAGTTTATTA"), QString(row4), "Wrong msa row");
    CHECK_EQUAL(QString("T--TAGTCTACTA"), QString(row5), "Wrong msa row");
    CHECK_EQUAL(QString("T--CAGATTATTA"), QString(row6), "Wrong msa row");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, removeRegion_allRows) {
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::removeRegionTestAlignment(os);
    CHECK_NO_ERROR(os);

    QList<qint64> rowIds;
    rowIds << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10 << 11 << 12;
    MsaDbiUtils::removeRegion(msaRef, rowIds, 0, 3, os);
    CHECK_NO_ERROR(os);

    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(11, al.getLength(), "Wrong msa length");

    QByteArray row0 = al.getRow(0).toByteArray(al.getLength(), os);
    QByteArray row1 = al.getRow(1).toByteArray(al.getLength(), os);
    QByteArray row2 = al.getRow(2).toByteArray(al.getLength(), os);
    QByteArray row3 = al.getRow(3).toByteArray(al.getLength(), os);
    QByteArray row4 = al.getRow(4).toByteArray(al.getLength(), os);
    QByteArray row5 = al.getRow(5).toByteArray(al.getLength(), os);
    QByteArray row6 = al.getRow(6).toByteArray(al.getLength(), os);
    QByteArray row7 = al.getRow(7).toByteArray(al.getLength(), os);
    QByteArray row8 = al.getRow(8).toByteArray(al.getLength(), os);
    QByteArray row9 = al.getRow(9).toByteArray(al.getLength(), os);
    QByteArray row10 = al.getRow(10).toByteArray(al.getLength(), os);
    QByteArray row11 = al.getRow(11).toByteArray(al.getLength(), os);
    QByteArray row12 = al.getRow(12).toByteArray(al.getLength(), os);
    CHECK_EQUAL(QString("GACTTCTAA--"), QString(row0), "Wrong msa row");
    CHECK_EQUAL(QString("GCTTACTA---"), QString(row1), "Wrong msa row");
    CHECK_EQUAL(QString("GTTTATTA---"), QString(row2), "Wrong msa row");
    CHECK_EQUAL(QString("CA-GTCTATTA"), QString(row3), "Wrong msa row");
    CHECK_EQUAL(QString("CA-GTTTATTA"), QString(row4), "Wrong msa row");
    CHECK_EQUAL(QString("TA-GTCTACTA"), QString(row5), "Wrong msa row");
    CHECK_EQUAL(QString("CA-GATTATTA"), QString(row6), "Wrong msa row");
    CHECK_EQUAL(QString("AGATTGCTA--"), QString(row7), "Wrong msa row");
    CHECK_EQUAL(QString("GATTATTA---"), QString(row8), "Wrong msa row");
    CHECK_EQUAL(QString("-----------"), QString(row9), "Wrong msa row");
    CHECK_EQUAL(QString("-----------"), QString(row10), "Wrong msa row");
    CHECK_EQUAL(QString("-----------"), QString(row11), "Wrong msa row");
    CHECK_EQUAL(QString("-----------"), QString(row12), "Wrong msa row");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, removeRegion_all) {
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::removeRegionTestAlignment(os);
    CHECK_NO_ERROR(os);

    QList<qint64> rowIds;
    rowIds << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10 << 11 << 12;
    MsaDbiUtils::removeRegion(msaRef, rowIds, 0, 14, os);
    CHECK_NO_ERROR(os);

    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(0, al.getLength(), "Wrong msa length");

    QByteArray row0 = al.getRow(0).toByteArray(al.getLength(), os);
    QByteArray row12 = al.getRow(12).toByteArray(al.getLength(), os);
    CHECK_EQUAL(QString(""), QString(row0), "Wrong msa row");
    CHECK_EQUAL(QString(""), QString(row12), "Wrong msa row");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, removeRegion_negativePos) {
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::removeRegionTestAlignment(os);
    CHECK_NO_ERROR(os);

    QList<qint64> rowIds;
    rowIds << 0;
    U2OpStatusImpl tmpOs;
    MsaDbiUtils::removeRegion(msaRef, rowIds, -1, 14, tmpOs);
    CHECK_TRUE(tmpOs.hasError(), "No error occurred for negative pos");

    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(14, al.getLength(), "Wrong msa length");

    QByteArray row0 = al.getRow(0).toByteArray(al.getLength(), os);
    CHECK_EQUAL(QString("TAAGACTTCTAA--"), QString(row0), "Wrong msa row");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, removeRegion_wrongId) {
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::removeRegionTestAlignment(os);
    CHECK_NO_ERROR(os);

    QList<qint64> rowIds;
    rowIds << 20;
    U2OpStatusImpl tmpOs;
    MsaDbiUtils::removeRegion(msaRef, rowIds, 0, 5, tmpOs);
    CHECK_TRUE(tmpOs.hasError(), "No error occurred for negative pos");
}

IMPLEMENT_TEST(MsaDbiUtilsUnitTests, removeRegion_wrongCount) {
    U2OpStatusImpl os;
    U2EntityRef msaRef = MsaDbiUtilsTestUtils::removeRegionTestAlignment(os);
    CHECK_NO_ERROR(os);

    QList<qint64> rowIds;
    rowIds << 0;
    U2OpStatusImpl tmpOs;
    MsaDbiUtils::removeRegion(msaRef, rowIds, 0, 0, tmpOs);
    CHECK_TRUE(tmpOs.hasError(), "No error occurred for negative pos");

    MAlignmentExporter ex;
    MAlignment al = ex.getAlignment(msaRef.dbiRef, msaRef.entityId, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(14, al.getLength(), "Wrong msa length");

    QByteArray row0 = al.getRow(0).toByteArray(al.getLength(), os);
    CHECK_EQUAL(QString("TAAGACTTCTAA--"), QString(row0), "Wrong msa row");
}

}   // namespace
