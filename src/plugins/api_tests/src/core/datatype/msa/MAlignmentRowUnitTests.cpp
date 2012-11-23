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

#include "MAlignmentRowUnitTests.h"

#include <U2Core/DNASequence.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2OpStatusUtils.h>


namespace U2 {

const int MAlignmentRowTestUtils::rowWithGapsLength = 7;
const int MAlignmentRowTestUtils::rowWithGapsInMiddleLength = 8;
const int MAlignmentRowTestUtils::rowWithoutGapsLength = 5;

const QString MAlignmentRowTestUtils::rowWithGapsName = "Row with gaps name";

MAlignmentRow MAlignmentRowTestUtils::initTestRowWithGaps() {
    DNASequence sequence(rowWithGapsName, "AGT");
    QList<U2MsaGap> gaps;
    U2MsaGap gapBeginning(0, 3);
    U2MsaGap gapMiddle(5, 1);
    gaps << gapBeginning << gapMiddle;
    U2OpStatusImpl opStatus;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, opStatus);
    return row; // "---AG-T"
}

MAlignmentRow MAlignmentRowTestUtils::initTestRowWithGapsInMiddle() {
    DNASequence sequence("Test sequence", "GGTAT");
    QList<U2MsaGap> gaps;
    U2MsaGap gapMiddle1(2, 1);
    U2MsaGap gapMiddle2(4, 2);
    gaps << gapMiddle1 << gapMiddle2;
    U2OpStatusImpl opStatus;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, opStatus);
    return row; // "GG-T--AT"
}

MAlignmentRow MAlignmentRowTestUtils::initTestRowWithTrailingGaps() {
    DNASequence sequence("Row with trailing gaps", "CAGTT");
    QList<U2MsaGap> gaps;
    U2MsaGap gapMiddle1(2, 1);
    U2MsaGap gapMiddle2(5, 2);
    U2MsaGap gapTrailing(8, 2);
    gaps << gapMiddle1 << gapMiddle2 << gapTrailing;
    U2OpStatusImpl opStatus;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, opStatus);
    return row; // "CA-GT--T--"
}

MAlignmentRow MAlignmentRowTestUtils::initTestRowWithoutGaps() {
    DNASequence sequence("For a row without gaps", "ACGTA");
    QList<U2MsaGap> gaps;
    U2OpStatusImpl opStatus;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, opStatus);
    return row; // "ACGTA"
}

MAlignmentRow MAlignmentRowTestUtils::initEmptyRow() {
    DNASequence sequence("Empty", "");
    QList<U2MsaGap> gaps;
    U2OpStatusImpl opStatus;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, opStatus);
    return row; // ""
}

MAlignmentRow MAlignmentRowTestUtils::initTestRowForModification() {
    DNASequence sequence("Test sequence", "AACGGTTACG");
    QList<U2MsaGap> gaps;
    U2MsaGap gapMiddle1(1, 3);
    U2MsaGap gapMiddle2(7, 2);
    U2MsaGap gapMiddle3(12, 1);
    U2MsaGap gapMiddle4(14, 1);
    U2MsaGap gapMiddle5(16, 3);
    gaps << gapMiddle1 << gapMiddle2 << gapMiddle3 << gapMiddle4 << gapMiddle5;
    U2OpStatusImpl opStatus;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, opStatus);
    return row; // "A---ACG--GTT-A-C---G"
}

QString MAlignmentRowTestUtils::getRowData(MAlignmentRow row) {
    U2OpStatusImpl os;
    QString result = row.toByteArray(row.getRowLength(), os).data();
    SAFE_POINT_OP(os, QString());
    return result;
}


/** Tests createRow */
IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_empty) {
    MAlignmentRow row;
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(0, row.getCoreEnd(), "core end");
    CHECK_EQUAL(0, row.getCoreLength(), "core length");
    CHECK_EQUAL(0, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_fromBytes) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "--GG-A---T", os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("--GG-A---T", QString(row.getCore()), "core data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(10, row.getCoreEnd(), "core end");
    CHECK_EQUAL(10, row.getCoreLength(), "core length");
    CHECK_EQUAL(10, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_fromBytesTrailing) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "--GG-A---T--", os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("--GG-A---T--", QString(row.getCore()), "core data");
    CHECK_EQUAL(4, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(12, row.getCoreEnd(), "core end");
    CHECK_EQUAL(12, row.getCoreLength(), "core length");
    CHECK_EQUAL(12, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_fromBytesGaps) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "----", os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("----", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("----", QString(row.getCore()), "core data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(4, row.getCoreEnd(), "core end");
    CHECK_EQUAL(4, row.getCoreLength(), "core length");
    CHECK_EQUAL(4, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_oneTrailing) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "A-", os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("A-", QString(row.getCore()), "core data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(2, row.getCoreEnd(), "core end");
    CHECK_EQUAL(2, row.getCoreLength(), "core length");
    CHECK_EQUAL(2, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_twoTrailing) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "A--", os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("A--", QString(row.getCore()), "core data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(3, row.getCoreEnd(), "core end");
    CHECK_EQUAL(3, row.getCoreLength(), "core length");
    CHECK_EQUAL(3, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_oneMiddleGap) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "AC-GT", os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("AC-GT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("AC-GT", QString(row.getCore()), "core data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(5, row.getCoreEnd(), "core end");
    CHECK_EQUAL(5, row.getCoreLength(), "core length");
    CHECK_EQUAL(5, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_noGaps) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "ACGT", os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("ACGT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("ACGT", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(4, row.getCoreEnd(), "core end");
    CHECK_EQUAL(4, row.getCoreLength(), "core length");
    CHECK_EQUAL(4, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_offsetNoGap) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "AG-TC", 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("-AG-TC", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("-AG-TC", QString(row.getCore()), "core data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(6, row.getCoreEnd(), "core end");
    CHECK_EQUAL(6, row.getCoreLength(), "core length");
    CHECK_EQUAL(6, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_offsetGap) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "---AG-TC", 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("-----AG-TC", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("-----AG-TC", QString(row.getCore()), "core data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(10, row.getCoreEnd(), "core end");
    CHECK_EQUAL(10, row.getCoreLength(), "core length");
    CHECK_EQUAL(10, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_offsetOnlyGaps) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "---", 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("-----", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("-----", QString(row.getCore()), "core data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(5, row.getCoreEnd(), "core end");
    CHECK_EQUAL(5, row.getCoreLength(), "core length");
    CHECK_EQUAL(5, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_emptyAndOffset) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "", 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("Test row", row.getName(), "row name");
    CHECK_EQUAL("-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("-", QString(row.getCore()), "core data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(1, row.getCoreEnd(), "core end");
    CHECK_EQUAL(1, row.getCoreLength(), "core length");
    CHECK_EQUAL(1, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_fromSeq) {
    DNASequence sequence("Test sequence", "GGAT");
    QList<U2MsaGap> gaps;
    U2MsaGap gapBeginning(0, 2);
    U2MsaGap gapMiddle1(4, 1);
    U2MsaGap gapMiddle2(6, 3);
    gaps << gapBeginning << gapMiddle1 << gapMiddle2;
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("--GG-A---T", QString(row.getCore()), "core data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(10, row.getCoreEnd(), "core end");
    CHECK_EQUAL(10, row.getCoreLength(), "core length");
    CHECK_EQUAL(10, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_fromSeqTrailing) {
    DNASequence sequence("Test sequence", "GGAT");
    QList<U2MsaGap> gaps;
    U2MsaGap gapBeginning(0, 2);
    U2MsaGap gapMiddle1(4, 1);
    U2MsaGap gapMiddle2(6, 3);
    U2MsaGap gapTrailing(10, 2);
    gaps << gapBeginning << gapMiddle1 << gapMiddle2 << gapTrailing;
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("--GG-A---T--", QString(row.getCore()), "core data");
    CHECK_EQUAL(4, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(12, row.getCoreEnd(), "core end");
    CHECK_EQUAL(12, row.getCoreLength(), "core length");
    CHECK_EQUAL(12, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_fromSeqWithGaps) {
    DNASequence sequence("Test sequence", "GG-AT");
    QList<U2MsaGap> gaps;
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, os);
    CHECK_EQUAL("Failed to create a multiple alignment row!", os.getError(), "opStatus");
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(0, row.getCoreEnd(), "core end");
    CHECK_EQUAL(0, row.getCoreLength(), "core length");
    CHECK_EQUAL(0, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_gapPositionTooBig) {
    DNASequence sequence("Test sequence", "GGAT");
    QList<U2MsaGap> gaps;
    U2MsaGap gapBeginning(0, 2);
    U2MsaGap gapMiddle1(4, 1);
    U2MsaGap gapMiddle2(8, 3);
    gaps << gapBeginning << gapMiddle1 << gapMiddle2;
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, os);
    CHECK_EQUAL("Failed to create a multiple alignment row!", os.getError(), "opStatus");
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(0, row.getCoreEnd(), "core end");
    CHECK_EQUAL(0, row.getCoreLength(), "core length");
    CHECK_EQUAL(0, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_negativeGapPos) {
    DNASequence sequence("Test sequence", "ACGT");
    QList<U2MsaGap> gaps;
    U2MsaGap invalidGap(-1, 2);
    gaps << invalidGap;
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, os);
    CHECK_EQUAL("Failed to create a multiple alignment row!", os.getError(), "opStatus");
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(0, row.getCoreEnd(), "core end");
    CHECK_EQUAL(0, row.getCoreLength(), "core length");
    CHECK_EQUAL(0, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_negativeGapOffset) {
    DNASequence sequence("Test sequence", "ACGT");
    QList<U2MsaGap> gaps;
    U2MsaGap invalidGap(0, -1);
    gaps << invalidGap;
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, os);
    CHECK_EQUAL("Failed to create a multiple alignment row!", os.getError(), "opStatus");
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number"); 
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(0, row.getCoreEnd(), "core end");
    CHECK_EQUAL(0, row.getCoreLength(), "core length");
    CHECK_EQUAL(0, row.getRowLength(), "row length");
}

/** Tests rowName */
IMPLEMENT_TEST(MAlignmentRowUnitTests, rowName_rowFromBytes) {
    QString rowName = "Test sequence";
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow(rowName, "AG-T" , os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(rowName, row.getName(), "name of the row");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowName_rowFromSeq) {
    QString rowName = "Test sequence";
    DNASequence sequence(rowName, "AGT");
    QList<U2MsaGap> gaps;
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(rowName, row.getName(), "name of the row");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowName_setName) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    QString rowName = "New row name";
    row.setName(rowName);
    CHECK_EQUAL(rowName, row.getName(), "name of the row");
}

/** Tests toByteArray */
IMPLEMENT_TEST(MAlignmentRowUnitTests, toByteArray_noGaps) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithoutGaps();
    U2OpStatusImpl os;
    QByteArray bytes = row.toByteArray(MAlignmentRowTestUtils::rowWithoutGapsLength, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("ACGTA", QString(bytes), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, toByteArray_gapsInBeginningAndMiddle) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    QByteArray bytes = row.toByteArray(MAlignmentRowTestUtils::rowWithGapsLength, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---AG-T", QString(bytes), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, toByteArray_incorrectLength) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    QByteArray bytes = row.toByteArray(MAlignmentRowTestUtils::rowWithGapsLength - 1, os);
    CHECK_EQUAL("Failed to get row data!", os.getError(), "opStatus");
    CHECK_EQUAL("", QString(bytes), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, toByteArray_greaterLength) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    QByteArray bytes = row.toByteArray(MAlignmentRowTestUtils::rowWithGapsLength + 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---AG-T-", QString(bytes), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, toByteArray_trailing) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "--GG-A---T--", os);
    CHECK_NO_ERROR(os);
    QByteArray bytes = row.toByteArray(12, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T--", QString(bytes), "row data");
}

/** Tests simplify */
IMPLEMENT_TEST(MAlignmentRowUnitTests, simplify_gaps) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "--GG-A---T--", os);
    CHECK_NO_ERROR(os);
    bool result = row.simplify();
    CHECK_TRUE(result, "simplify() must have returned 'true'!");
    CHECK_EQUAL("GGAT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("GGAT", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(4, row.getCoreEnd(), "core end");
    CHECK_EQUAL(4, row.getCoreLength(), "core length");
    CHECK_EQUAL(4, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, simplify_nothingToRemove) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithoutGaps();
    bool result = row.simplify();
    CHECK_FALSE(result, "simplify() must have returned 'false'!");
    CHECK_EQUAL("ACGTA", MAlignmentRowTestUtils::getRowData(row), "row data");
}

/** Tests append */
IMPLEMENT_TEST(MAlignmentRowUnitTests, append_noGapBetweenRows) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    MAlignmentRow anotherRow = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.append(anotherRow, row.getRowLength(), os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---AG-TGG-T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL("GG-T--AT", MAlignmentRowTestUtils::getRowData(anotherRow), "another row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, append_gapBetweenRows) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    MAlignmentRow anotherRow = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.append(anotherRow, row.getRowLength() + 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---AG-T-GG-T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, append_offsetInAnotherRow) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    MAlignmentRow anotherRow = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.append(anotherRow, row.getRowLength() + 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG-T--AT-----AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, append_trailingInFirst) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithTrailingGaps();
    MAlignmentRow anotherRow = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.append(anotherRow, row.getRowLength() + 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("CA-GT--T---GG-T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, append_trailingAndOffset) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithTrailingGaps();
    MAlignmentRow anotherRow = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.append(anotherRow, row.getRowLength(), os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("CA-GT--T-----AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, append_invalidLength) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    MAlignmentRow anotherRow = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.append(anotherRow, row.getRowLength() - 1, os);
    CHECK_EQUAL("Failed to append one row to another!", os.getError(), "opStatus");
    CHECK_EQUAL("---AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
}

/** Tests setRowContent */
IMPLEMENT_TEST(MAlignmentRowUnitTests, setRowContent_empty) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.setRowContent("", 0, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(MAlignmentRowTestUtils::rowWithGapsName, row.getName(), "row name");
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(0, row.getCoreEnd(), "core end");
    CHECK_EQUAL(0, row.getCoreLength(), "core length");
    CHECK_EQUAL(0, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, setRowContent_trailingGaps) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.setRowContent("--GG-A---T--", 0, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(MAlignmentRowTestUtils::rowWithGapsName, row.getName(), "row name");
    CHECK_EQUAL("--GG-A---T--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("--GG-A---T--", QString(row.getCore()), "core data");
    CHECK_EQUAL(4, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(12, row.getCoreEnd(), "core end");
    CHECK_EQUAL(12, row.getCoreLength(), "core length");
    CHECK_EQUAL(12, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, setRowContent_offsetNoGap) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.setRowContent("AC-GT", 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(MAlignmentRowTestUtils::rowWithGapsName, row.getName(), "row name");
    CHECK_EQUAL("-AC-GT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("-AC-GT", QString(row.getCore()), "core data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(6, row.getCoreEnd(), "core end");
    CHECK_EQUAL(6, row.getCoreLength(), "core length");
    CHECK_EQUAL(6, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, setRowContent_offsetGap) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.setRowContent("--GG", 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(MAlignmentRowTestUtils::rowWithGapsName, row.getName(), "row name");
    CHECK_EQUAL("---GG", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("---GG", QString(row.getCore()), "core data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(5, row.getCoreEnd(), "core end");
    CHECK_EQUAL(5, row.getCoreLength(), "core length");
    CHECK_EQUAL(5, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, setRowContent_emptyAndOffset) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.setRowContent("", 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(MAlignmentRowTestUtils::rowWithGapsName, row.getName(), "row name");
    CHECK_EQUAL("-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("-", QString(row.getCore()), "core data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(1, row.getCoreEnd(), "core end");
    CHECK_EQUAL(1, row.getCoreLength(), "core length");
    CHECK_EQUAL(1, row.getRowLength(), "row length");
}

/** Tests insertGaps */
IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_empty) {
    MAlignmentRow row = MAlignmentRowTestUtils::initEmptyRow();
    U2OpStatusImpl os;
    row.insertGaps(0, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toGapPosLeft) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.insertGaps(3, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG--T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");

    row.insertGaps(7, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG--T----AT", MAlignmentRowTestUtils::getRowData(row), "row data (second insertion)");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toGapPosRight) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.insertGaps(2, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG--T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");

    row.insertGaps(5, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG--T----AT", MAlignmentRowTestUtils::getRowData(row), "row data (second insertion)");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toGapPosInside) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.insertGaps(1, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("-----AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_insideChars) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.insertGaps(4, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---A--G-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toZeroPosNoGap) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.insertGaps(0, 3, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---GG-T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toZeroPosGap) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.insertGaps(0, 3, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("------AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toLastPosNoGap) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.insertGaps(7, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG-T--A--T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toLastPosGap) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithTrailingGaps();
    U2OpStatusImpl os;
    row.insertGaps(9, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("CA-GT--T---", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toLastPosOneGap) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "A-", os);
    CHECK_NO_ERROR(os);
    row.insertGaps(1, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_noGapsYet) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithoutGaps();
    U2OpStatusImpl os;
    row.insertGaps(4, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("ACGT-A", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_onlyGaps) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "--", os);
    CHECK_NO_ERROR(os);
    row.insertGaps(1, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("----", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_oneChar) {
    DNASequence sequence("One-char sequence", "A");
    QList<U2MsaGap> gaps;
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, os);
    CHECK_NO_ERROR(os);
    row.insertGaps(0, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--A", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_tooBigPosition) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.insertGaps(7, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_negativePosition) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.insertGaps(-1, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_negativeNumOfChars) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.insertGaps(1, -1, os);
    CHECK_EQUAL("Failed to insert gaps into a row!", os.getError(), "opStatus");
    CHECK_EQUAL("---AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
}

/** Tests removeChars */
IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_empty) {
    MAlignmentRow row = MAlignmentRowTestUtils::initEmptyRow();
    U2OpStatusImpl os;
    row.removeChars(0, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideGap1) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(2, 15, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideGap2) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(3, 15, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_leftGapSide) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(7, 9, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---ACG---G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_rightGapSide) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(4, 11, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideSeq1) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(5, 6, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---AT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideSeq2) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(6, 4, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---ACTT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_fromZeroPosGap) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.removeChars(0, 4, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("G-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_fromZeroPosChar) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(0, 17, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_lastPosExactly) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.removeChars(7, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG-T--A", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_fromLastPos) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.removeChars(7, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG-T--A", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideOneGap1) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(2, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A-ACG--GTT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideOneGap2) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(2, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A--ACG--GTT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideOneGapLong) {
    DNASequence sequence("Test sequence", "AGTCT");
    QList<U2MsaGap> gaps;
    U2MsaGap gap1(1, 6);
    U2MsaGap gap2(9, 2);
    U2MsaGap gap3(12, 1);
    gaps << gap1 << gap2 << gap3;
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, os);
    CHECK_EQUAL("A------GT--C-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_NO_ERROR(os);
    row.removeChars(2, 3, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---GT--C-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideTrailingGap) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "AC-GT----", os);
    CHECK_NO_ERROR(os);
    row.removeChars(5, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("AC-GT--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideCharsOne) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(5, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---AG--GTT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_negativePosition) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(-1, 1, os);
    CHECK_EQUAL("Can't remove chars from a row!", os.getError(), "opStatus");
    CHECK_EQUAL("A---ACG--GTT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_negativeNumOfChars) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(1, -1, os);
    CHECK_EQUAL("Can't remove chars from a row!", os.getError(), "opStatus");
    CHECK_EQUAL("A---ACG--GTT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_gapsAtRowEnd1) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(9, 12, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---ACG--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_gapsAtRowEnd2) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(3, 21, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_onlyGapsAfterRemove) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.removeChars(2, 9, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_emptyAfterRemove) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(0, 21, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_oneCharInGaps) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.removeChars(13, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---ACG--GTT--C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "number of gaps");
}

/** Tests charAt */
IMPLEMENT_TEST(MAlignmentRowUnitTests, charAt_allCharsNoOffset) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    char ch = row.charAt(-1);
    CHECK_EQUAL('-', ch, "char -1");
    
    ch = row.charAt(0);
    CHECK_EQUAL('G', ch, "char 0");
    
    ch = row.charAt(1);
    CHECK_EQUAL('G', ch, "char 1");
    
    ch = row.charAt(2);
    CHECK_EQUAL('-', ch, "char 2");
    
    ch = row.charAt(3);
    CHECK_EQUAL('T', ch, "char 3");
    
    ch = row.charAt(4);
    CHECK_EQUAL('-', ch, "char 4");
    
    ch = row.charAt(5);
    CHECK_EQUAL('-', ch, "char 5");
    
    ch = row.charAt(6);
    CHECK_EQUAL('A', ch, "char 6");
    
    ch = row.charAt(7);
    CHECK_EQUAL('T', ch, "char 7");
    
    ch = row.charAt(8);
    CHECK_EQUAL('-', ch, "char 8");
    
    ch = row.charAt(9);
    CHECK_EQUAL('-', ch, "char 9");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, charAt_offsetAndTrailing) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "-AC-", os);
    CHECK_NO_ERROR(os);

    char ch = row.charAt(-1);
    CHECK_EQUAL('-', ch, "char -1");

    ch = row.charAt(0);
    CHECK_EQUAL('-', ch, "char 0");
    
    ch = row.charAt(1);
    CHECK_EQUAL('A', ch, "char 1");
    
    ch = row.charAt(2);
    CHECK_EQUAL('C', ch, "char 2");

    ch = row.charAt(3);
    CHECK_EQUAL('-', ch, "char 3");

    ch = row.charAt(4);
    CHECK_EQUAL('-', ch, "char 4");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, charAt_onlyCharsInRow) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "ACG", os);
    CHECK_NO_ERROR(os);

    char ch = row.charAt(-1);
    CHECK_EQUAL('-', ch, "char -1");

    ch = row.charAt(0);
    CHECK_EQUAL('A', ch, "char 0");

    ch = row.charAt(1);
    CHECK_EQUAL('C', ch, "char 1");

    ch = row.charAt(2);
    CHECK_EQUAL('G', ch, "char 2");

    ch = row.charAt(3);
    CHECK_EQUAL('-', ch, "char 3");
}


/** Tests rowEqual */
IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_sameContent) {
    MAlignmentRow firstRow = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    MAlignmentRow secondRow = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_TRUE(result, "The first and the second rows are NOT equal unexpectedly!");

    CHECK_TRUE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_FALSE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_noGaps) {
    U2OpStatusImpl os;
    MAlignmentRow firstRow = MAlignmentRow::createRow("First", "ACT", os);
    CHECK_NO_ERROR(os);
    MAlignmentRow secondRow = MAlignmentRow::createRow("Second", "ACT", os);
    CHECK_NO_ERROR(os);

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_TRUE(result, "The first and the second rows are NOT equal unexpectedly!");

    CHECK_TRUE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_FALSE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_trailingInFirst) {
    U2OpStatusImpl os;
    MAlignmentRow firstRow = MAlignmentRow::createRow("First", "AC-GT-", os);
    CHECK_NO_ERROR(os);
    MAlignmentRow secondRow = MAlignmentRow::createRow("Second", "AC-GT", os);
    CHECK_NO_ERROR(os);

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_TRUE(result, "The first and the second rows are NOT equal unexpectedly!");

    CHECK_TRUE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_FALSE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_trailingInSecond) {
    U2OpStatusImpl os;
    MAlignmentRow firstRow = MAlignmentRow::createRow("First", "AC-GT", os);
    CHECK_NO_ERROR(os);
    MAlignmentRow secondRow = MAlignmentRow::createRow("Second", "AC-GT--", os);
    CHECK_NO_ERROR(os);

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_TRUE(result, "The first and the second rows are NOT equal unexpectedly!");

    CHECK_TRUE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_FALSE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_trailingInBoth) {
    U2OpStatusImpl os;
    MAlignmentRow firstRow = MAlignmentRow::createRow("First", "AC-GT---", os);
    CHECK_NO_ERROR(os);
    MAlignmentRow secondRow = MAlignmentRow::createRow("Second", "AC-GT--", os);
    CHECK_NO_ERROR(os);

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_TRUE(result, "The first and the second rows are NOT equal unexpectedly!");

    CHECK_TRUE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_FALSE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_diffGapModelsGap) {
    DNASequence sequence("Test sequence", "GGAT");
    
    U2MsaGap gapBeginning(0, 2);
    U2MsaGap gapMiddle1ForRow1(4, 1);
    U2MsaGap gapMiddle1ForRow2(4, 2);
    U2MsaGap gapMiddle2ForRow1(6, 3);
    U2MsaGap gapMiddle2ForRow2(7, 3);

    QList<U2MsaGap> gapsForRow1;
    QList<U2MsaGap> gapsForRow2;
    gapsForRow1 << gapBeginning << gapMiddle1ForRow1 << gapMiddle2ForRow1;
    gapsForRow2 << gapBeginning << gapMiddle1ForRow2 << gapMiddle2ForRow2;

    U2OpStatusImpl os;
    MAlignmentRow firstRow = MAlignmentRow::createRow(sequence, gapsForRow1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T", MAlignmentRowTestUtils::getRowData(firstRow), "first row data");
    MAlignmentRow secondRow = MAlignmentRow::createRow(sequence, gapsForRow2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG--A---T", MAlignmentRowTestUtils::getRowData(secondRow), "second row data");

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_FALSE(result, "The first and the second rows are EQUAL unexpectedly!");

    CHECK_FALSE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_TRUE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_diffGapModelsOffset) {
    DNASequence sequence("Test sequence", "GGAT");

    U2MsaGap gapBeginning(0, 2);
    U2MsaGap gapMiddle1ForRow1(4, 1);
    U2MsaGap gapMiddle1ForRow2(3, 1);
    U2MsaGap gapMiddle2(6, 3);

    QList<U2MsaGap> gapsForRow1;
    QList<U2MsaGap> gapsForRow2;
    gapsForRow1 << gapBeginning << gapMiddle1ForRow1 << gapMiddle2;
    gapsForRow2 << gapBeginning << gapMiddle1ForRow2 << gapMiddle2;

    U2OpStatusImpl os;
    MAlignmentRow firstRow = MAlignmentRow::createRow(sequence, gapsForRow1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T", MAlignmentRowTestUtils::getRowData(firstRow), "first row data");
    MAlignmentRow secondRow = MAlignmentRow::createRow(sequence, gapsForRow2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--G-GA---T", MAlignmentRowTestUtils::getRowData(secondRow), "second row data");

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_FALSE(result, "The first and the second rows are EQUAL unexpectedly!");

    CHECK_FALSE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_TRUE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_diffNumOfGaps) {
    DNASequence sequence("Test sequence", "GGAT");

    U2MsaGap gapBeginning(0, 2);
    U2MsaGap gapMiddle1(4, 1);
    U2MsaGap gapMiddle2(6, 3);

    QList<U2MsaGap> gapsForRow1;
    QList<U2MsaGap> gapsForRow2;
    gapsForRow1 << gapBeginning << gapMiddle1 << gapMiddle2;
    gapsForRow2 << gapBeginning << gapMiddle1;

    U2OpStatusImpl os;
    MAlignmentRow firstRow = MAlignmentRow::createRow(sequence, gapsForRow1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T", MAlignmentRowTestUtils::getRowData(firstRow), "first row data");
    MAlignmentRow secondRow = MAlignmentRow::createRow(sequence, gapsForRow2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-AT", MAlignmentRowTestUtils::getRowData(secondRow), "second row data");

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_FALSE(result, "The first and the second rows are EQUAL unexpectedly!");

    CHECK_FALSE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_TRUE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_diffSequences) {
    DNASequence firstSequence("First sequence", "GGAT");
    DNASequence secondSequence("Second sequence", "GGCT");

    QList<U2MsaGap> gaps;
    U2MsaGap gapBeginning(0, 2);
    U2MsaGap gapMiddle1(4, 1);
    U2MsaGap gapMiddle2(6, 3);
    gaps << gapBeginning << gapMiddle1 << gapMiddle2;

    U2OpStatusImpl os;
    MAlignmentRow firstRow = MAlignmentRow::createRow(firstSequence, gaps, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T", MAlignmentRowTestUtils::getRowData(firstRow), "first row data");
    MAlignmentRow secondRow = MAlignmentRow::createRow(secondSequence, gaps, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-C---T", MAlignmentRowTestUtils::getRowData(secondRow), "second row data");

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_FALSE(result, "The first and the second rows are EQUAL unexpectedly!");

    CHECK_FALSE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_TRUE(firstRow != secondRow, "Incorrect 'operator!='!");
}

/** Tests ungapped */
IMPLEMENT_TEST(MAlignmentRowUnitTests, ungapped_rowWithoutOffset) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    CHECK_EQUAL(5, row.getUngappedLength(), "ungapped length");
    CHECK_EQUAL(-1, row.getUngappedPosition(-1), "pos -1");
    CHECK_EQUAL(0,  row.getUngappedPosition(0),  "pos 0");
    CHECK_EQUAL(1,  row.getUngappedPosition(1),  "pos 1");
    CHECK_EQUAL(-1, row.getUngappedPosition(2),  "pos 2");
    CHECK_EQUAL(2,  row.getUngappedPosition(3),  "pos 3");
    CHECK_EQUAL(-1, row.getUngappedPosition(4),  "pos 4");
    CHECK_EQUAL(-1, row.getUngappedPosition(5),  "pos 5");
    CHECK_EQUAL(3,  row.getUngappedPosition(6),  "pos 6");
    CHECK_EQUAL(4,  row.getUngappedPosition(7),  "pos 7");
    CHECK_EQUAL(-1, row.getUngappedPosition(8),  "pos 8");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, ungapped_offsetTrailing) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Test row", "---AG-T-", os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(3,  row.getUngappedLength(), "ungapped length");
    CHECK_EQUAL(-1, row.getUngappedPosition(-1), "pos -1");
    CHECK_EQUAL(-1, row.getUngappedPosition(0),  "pos 0");
    CHECK_EQUAL(-1, row.getUngappedPosition(1),  "pos 1");
    CHECK_EQUAL(-1, row.getUngappedPosition(2),  "pos 2");
    CHECK_EQUAL(0,  row.getUngappedPosition(3),  "pos 3");
    CHECK_EQUAL(1,  row.getUngappedPosition(4),  "pos 4");
    CHECK_EQUAL(-1, row.getUngappedPosition(5),  "pos 5");
    CHECK_EQUAL(2,  row.getUngappedPosition(6),  "pos 6");
    CHECK_EQUAL(-1, row.getUngappedPosition(7),  "pos 7");
    CHECK_EQUAL(-1, row.getUngappedPosition(8),  "pos 8");
}

/** Tests crop */
IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_empty) {
    MAlignmentRow row = MAlignmentRowTestUtils::initEmptyRow();
    U2OpStatusImpl os;
    row.crop(0, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideGap1) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(2, 15, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--ACG--GTT-A-C-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideGap2) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(3, 15, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("-ACG--GTT-A-C--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_leftGapSide) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(7, 9, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GTT-A-C", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_rightGapSide) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(4, 11, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("ACG--GTT-A-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideSeq1) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(5, 6, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("CG--GT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideSeq2) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(6, 4, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("G--G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_fromZeroPosGap) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.crop(0, 4, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---A", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_fromZeroPosChar) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(0, 17, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---ACG--GTT-A-C-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_lastPosExactly) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.crop(7, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_fromLastPos) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.crop(7, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideOneGap1) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(2, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideOneGap2) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(2, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideOneGapLong) {
    DNASequence sequence("Test sequence", "AGTCT");
    QList<U2MsaGap> gaps;
    U2MsaGap gap1(1, 6);
    U2MsaGap gap2(9, 2);
    U2MsaGap gap3(12, 1);
    gaps << gap1 << gap2 << gap3;
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow(sequence, gaps, os);
    CHECK_EQUAL("A------GT--C-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_NO_ERROR(os);
    row.crop(2, 3, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideCharsOne) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(5, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("C", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_negativePosition) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(-1, 1, os);
    CHECK_EQUAL("Can't crop a row!", os.getError(), "opStatus");
    CHECK_EQUAL("A---ACG--GTT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_negativeNumOfChars) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(1, -1, os);
    CHECK_EQUAL("Can't crop a row!", os.getError(), "opStatus");
    CHECK_EQUAL("A---ACG--GTT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_trailing) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithTrailingGaps();
    U2OpStatusImpl os;
    row.crop(2, 8, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("-GT--T--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_trailingToGaps) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithTrailingGaps();
    U2OpStatusImpl os;
    row.crop(0, 9, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("CA-GT--T-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_cropTrailing) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithTrailingGaps();
    U2OpStatusImpl os;
    row.crop(9, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_oneCharInGaps) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    row.crop(13, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_posMoreThanLength) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps();
    U2OpStatusImpl os;
    row.crop(13, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(0, row.getRowLength(), "row length");
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

/** Tests mid */
IMPLEMENT_TEST(MAlignmentRowUnitTests, mid_general) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification();
    U2OpStatusImpl os;
    MAlignmentRow result = row.mid(4, 8, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("ACG--GTT", MAlignmentRowTestUtils::getRowData(result), "row data");
    CHECK_EQUAL(1, result.getGapModel().count(), "number of gaps");
}

/** Tests upperCase */
IMPLEMENT_TEST(MAlignmentRowUnitTests, upperCase_general) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Row name", "avn-*y-s", os);
    CHECK_NO_ERROR(os);
    row.toUpperCase();
    CHECK_EQUAL("AVN-*Y-S", MAlignmentRowTestUtils::getRowData(row), "row data");

    QString actualRowName = row.getName();
    CHECK_EQUAL("Row name", actualRowName, "row name");
}

/** Tests replaceChars */
IMPLEMENT_TEST(MAlignmentRowUnitTests, replaceChars_charToChar) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.replaceChars('T', 'C', os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG-C--AC", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, replaceChars_nothingToReplace) {
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle();
    U2OpStatusImpl os;
    row.replaceChars('~', '-', os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG-T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, replaceChars_tildasToGapsNoGaps) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Row name", "A~~CC~~~AG~AC~TG", os);
    CHECK_NO_ERROR(os);
    row.replaceChars('~', '-', os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A--CC---AG-AC-TG", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, replaceChars_tildasToGapsWithGaps) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Row name", "A~-CC~-~AG~AC-TG", os);
    CHECK_NO_ERROR(os);
    row.replaceChars('~', '-', os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A--CC---AG-AC-TG", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, replaceChars_trailingGaps) {
    U2OpStatusImpl os;
    MAlignmentRow row = MAlignmentRow::createRow("Row name", "A~~CC~~~AG~AC~TG~", os);
    CHECK_NO_ERROR(os);
    row.replaceChars('~', '-', os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A--CC---AG-AC-TG-", MAlignmentRowTestUtils::getRowData(row), "row data");
}


} // namespace
