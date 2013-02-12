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

namespace U2 {

const QString MsaDbiUtilsTestUtils::alignmentName = QString("MsaDbiUtils test alignment");
TestDbiProvider MsaDbiUtilsTestUtils::dbiProvider = TestDbiProvider();
const QString& MsaDbiUtilsTestUtils::MSA_DB_URL("msa-dbi.ugenedb");
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


U2EntityRef MsaDbiUtilsTestUtils::initTestAlignment(const int rowCount) {
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

}   // namespace
