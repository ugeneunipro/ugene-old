/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

MAlignmentRow MAlignmentRowTestUtils::initTestRowWithGaps(MAlignment& almnt) {
    U2OpStatusImpl opStatus;
    almnt.setName("For row with gaps");
    almnt.addRow(rowWithGapsName, "---AG-T", opStatus);
    return almnt.getRow(0); // "---AG-T"
}

MAlignmentRow MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(MAlignment& almnt) {
    U2OpStatusImpl opStatus;
    almnt.setName("For row with gaps in middle");
    almnt.addRow("Test sequence", "GG-T--AT", opStatus);
    return almnt.getRow(0); // "GG-T--AT"
}

MAlignmentRow MAlignmentRowTestUtils::initTestRowWithTrailingGaps(MAlignment& almnt) {
    U2OpStatusImpl opStatus;
    almnt.setName("For row with trailing gaps");
    almnt.addRow("Row with trailing gaps", "CA-GT--T--", opStatus);
    return almnt.getRow(0); // "CA-GT--T--"
}

MAlignmentRow MAlignmentRowTestUtils::initTestRowWithoutGaps(MAlignment& almnt) {
    U2OpStatusImpl opStatus;
    almnt.setName("For a row without gaps");
    almnt.addRow("Row without gaps", "ACGTA", opStatus);
    return almnt.getRow(0); // "ACGTA"
}

MAlignmentRow MAlignmentRowTestUtils::initEmptyRow(MAlignment& almnt) {
    U2OpStatusImpl opStatus;
    almnt.setName("For empty row");
    almnt.addRow("Empty", "", opStatus);
    return almnt.getRow(0); // ""
}

MAlignmentRow MAlignmentRowTestUtils::initTestRowForModification(MAlignment& almnt) {
    U2OpStatusImpl opStatus;
    almnt.setName("For row for modifications");
    almnt.addRow("Test sequence", "A---ACG--GTT-A-C---G", opStatus);
    return almnt.getRow(0); // "A---ACG--GTT-A-C---G"
}

QString MAlignmentRowTestUtils::getRowData(MAlignmentRow row) {
    U2OpStatusImpl os;
    QString result = row.toByteArray(row.getRowLength(), os).data();
    SAFE_POINT_OP(os, QString());
    return result;
}


/** Tests createRow */
IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_fromBytes) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "--GG-A---T", os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("--GG-A---T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("GG-A---T", QString(row.getCore()), "core data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(2, row.getCoreStart(), "core start");
    CHECK_EQUAL(10, row.getCoreEnd(), "core end");
    CHECK_EQUAL(8, row.getCoreLength(), "core length");
    CHECK_EQUAL(10, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_fromBytesTrailing) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "--GG-A---T--", os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("--GG-A---T--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("GG-A---T", QString(row.getCore()), "core data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(2, row.getCoreStart(), "core start");
    CHECK_EQUAL(10, row.getCoreEnd(), "core end");
    CHECK_EQUAL(8, row.getCoreLength(), "core length");
    CHECK_EQUAL(12, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_fromBytesGaps) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "----", os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("----", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(0, row.getCoreEnd(), "core end");
    CHECK_EQUAL(0, row.getCoreLength(), "core length");
    CHECK_EQUAL(4, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_oneTrailing) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "A-", os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("A-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("A", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(1, row.getCoreEnd(), "core end");
    CHECK_EQUAL(1, row.getCoreLength(), "core length");
    CHECK_EQUAL(2, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_twoTrailing) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "A--", os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("A--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("A", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(1, row.getCoreEnd(), "core end");
    CHECK_EQUAL(1, row.getCoreLength(), "core length");
    CHECK_EQUAL(3, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_oneMiddleGap) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "AC-GT", os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
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
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "ACGT", os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("ACGT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("ACGT", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(4, row.getCoreEnd(), "core end");
    CHECK_EQUAL(4, row.getCoreLength(), "core length");
    CHECK_EQUAL(4, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_fromSeq) {
    MAlignment almnt("Test alignment");
    DNASequence sequence("Test sequence", "GGAT");
    QList<U2MsaGap> gaps;
    U2MsaGap gapBeginning(0, 2);
    U2MsaGap gapMiddle1(4, 1);
    U2MsaGap gapMiddle2(6, 3);
    gaps << gapBeginning << gapMiddle1 << gapMiddle2;
    U2OpStatusImpl os;
    almnt.addRow("Row", sequence, gaps, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("--GG-A---T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("GG-A---T", QString(row.getCore()), "core data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(2, row.getCoreStart(), "core start");
    CHECK_EQUAL(10, row.getCoreEnd(), "core end");
    CHECK_EQUAL(8, row.getCoreLength(), "core length");
    CHECK_EQUAL(10, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_fromSeqTrailing) {
    MAlignment almnt("Test alignment");
    DNASequence sequence("Test sequence", "GGAT");
    QList<U2MsaGap> gaps;
    U2MsaGap gapBeginning(0, 2);
    U2MsaGap gapMiddle1(4, 1);
    U2MsaGap gapMiddle2(6, 3);
    U2MsaGap gapTrailing(10, 2);
    gaps << gapBeginning << gapMiddle1 << gapMiddle2 << gapTrailing;
    U2OpStatusImpl os;
    almnt.addRow("Row", sequence, gaps, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("--GG-A---T--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("GG-A---T", QString(row.getCore()), "core data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(2, row.getCoreStart(), "core start");
    CHECK_EQUAL(10, row.getCoreEnd(), "core end");
    CHECK_EQUAL(8, row.getCoreLength(), "core length");
    CHECK_EQUAL(12, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_fromSeqWithGaps) {
    MAlignment almnt("Test alignment");
    DNASequence sequence("Test sequence", "GG-AT");
    QList<U2MsaGap> gaps;
    U2OpStatusImpl os;
    almnt.addRow("Row", sequence, gaps, os);
    CHECK_EQUAL("Failed to create a multiple alignment row!", os.getError(), "opStatus");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_gapPositionTooBig) {
    MAlignment almnt("Test alignment");
    DNASequence sequence("Test sequence", "GGAT");
    QList<U2MsaGap> gaps;
    U2MsaGap gapBeginning(0, 2);
    U2MsaGap gapMiddle1(4, 1);
    U2MsaGap gapMiddle2(8, 3);
    gaps << gapBeginning << gapMiddle1 << gapMiddle2;
    U2OpStatusImpl os;
    almnt.addRow("Row", sequence, gaps, os);
    CHECK_EQUAL("Failed to create a multiple alignment row!", os.getError(), "opStatus");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_negativeGapPos) {
    MAlignment almnt("Test alignment");
    DNASequence sequence("Test sequence", "ACGT");
    QList<U2MsaGap> gaps;
    U2MsaGap invalidGap(-1, 2);
    gaps << invalidGap;
    U2OpStatusImpl os;
    almnt.addRow("Row", sequence, gaps, os);
    CHECK_EQUAL("Failed to create a multiple alignment row!", os.getError(), "opStatus");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, createRow_negativeGapOffset) {
    MAlignment almnt("Test alignment");
    DNASequence sequence("Test sequence", "ACGT");
    QList<U2MsaGap> gaps;
    U2MsaGap invalidGap(0, -1);
    gaps << invalidGap;
    U2OpStatusImpl os;
    almnt.addRow("Row", sequence, gaps, os);
    CHECK_EQUAL("Failed to create a multiple alignment row!", os.getError(), "opStatus");
}

/** Tests rowName */
IMPLEMENT_TEST(MAlignmentRowUnitTests, rowName_rowFromBytes) {
    QString rowName = "Test sequence";
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow(rowName, "AG-T" , os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(rowName, row.getName(), "name of the row");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowName_rowFromSeq) {
    QString rowName = "Test sequence";
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow(rowName, "AGT", os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(rowName, row.getName(), "name of the row");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowName_setName) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    QString rowName = "New row name";
    row.setName(rowName);
    CHECK_EQUAL(rowName, row.getName(), "name of the row");
}

/** Tests toByteArray */
IMPLEMENT_TEST(MAlignmentRowUnitTests, toByteArray_noGaps) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithoutGaps(almnt);
    U2OpStatusImpl os;
    QByteArray bytes = row.toByteArray(MAlignmentRowTestUtils::rowWithoutGapsLength, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("ACGTA", QString(bytes), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, toByteArray_gapsInBeginningAndMiddle) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    QByteArray bytes = row.toByteArray(MAlignmentRowTestUtils::rowWithGapsLength, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---AG-T", QString(bytes), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, toByteArray_incorrectLength) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    QByteArray bytes = row.toByteArray(MAlignmentRowTestUtils::rowWithGapsLength - 1, os);
    CHECK_EQUAL("Failed to get row data!", os.getError(), "opStatus");
    CHECK_EQUAL("", QString(bytes), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, toByteArray_greaterLength) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    QByteArray bytes = row.toByteArray(MAlignmentRowTestUtils::rowWithGapsLength + 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---AG-T-", QString(bytes), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, toByteArray_trailing) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "--GG-A---T--", os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    QByteArray bytes = row.toByteArray(12, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T--", QString(bytes), "row data");
}

/** Tests simplify */
IMPLEMENT_TEST(MAlignmentRowUnitTests, simplify_gaps) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "--GG-A---T--", os);
    CHECK_NO_ERROR(os);
    bool result = almnt.simplify();
    MAlignmentRow row = almnt.getRow(0);
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
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithoutGaps(almnt);
    bool result = almnt.simplify();
    CHECK_FALSE(result, "simplify() must have returned 'false'!");
    CHECK_EQUAL("ACGTA", MAlignmentRowTestUtils::getRowData(row), "row data");
}

/** Tests append */
IMPLEMENT_TEST(MAlignmentRowUnitTests, append_noGapBetweenRows) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    MAlignment almnt2;
    MAlignmentRow anotherRow = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt2);
    U2OpStatusImpl os;
    almnt.appendRow(0, row.getRowLength(), anotherRow, os);
    row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---AG-TGG-T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL("GG-T--AT", MAlignmentRowTestUtils::getRowData(anotherRow), "another row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, append_gapBetweenRows) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    MAlignment almnt2;
    MAlignmentRow anotherRow = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt2);
    U2OpStatusImpl os;
    almnt.appendRow(0, row.getRowLength() + 1, anotherRow, os);
    row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---AG-T-GG-T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, append_offsetInAnotherRow) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
    MAlignment almnt2;
    MAlignmentRow anotherRow = MAlignmentRowTestUtils::initTestRowWithGaps(almnt2);
    U2OpStatusImpl os;
    almnt.appendRow(0, row.getRowLength() + 2, anotherRow, os);
    row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG-T--AT-----AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, append_trailingInFirst) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithTrailingGaps(almnt);
    MAlignment almnt2;
    MAlignmentRow anotherRow = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt2);
    U2OpStatusImpl os;
    almnt.appendRow(0, row.getRowLength() + 1, anotherRow, os);
    row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("CA-GT--T---GG-T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, append_trailingAndOffset) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithTrailingGaps(almnt);
    MAlignment almnt2;
    MAlignmentRow anotherRow = MAlignmentRowTestUtils::initTestRowWithGaps(almnt2);
    U2OpStatusImpl os;
    almnt.appendRow(0, row.getRowLength(), anotherRow, os);
    row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("CA-GT--T-----AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, append_invalidLength) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    MAlignment almnt2;
    MAlignmentRow anotherRow = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt2);
    U2OpStatusImpl os;
    almnt.appendRow(0, row.getRowLength() - 1, anotherRow, os);
    row = almnt.getRow(0);
    CHECK_EQUAL("Failed to append one row to another!", os.getError(), "opStatus");
    CHECK_EQUAL("---AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
}

/** Tests setRowContent */
IMPLEMENT_TEST(MAlignmentRowUnitTests, setRowContent_empty) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.setRowContent(0, "");
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(MAlignmentRowTestUtils::rowWithGapsName, row.getName(), "row name");
    CHECK_EQUAL("-------", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(0, row.getCoreEnd(), "core end");
    CHECK_EQUAL(0, row.getCoreLength(), "core length");
    CHECK_EQUAL(7, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, setRowContent_trailingGaps) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.setRowContent(0, "--GG-A---T--");
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(MAlignmentRowTestUtils::rowWithGapsName, row.getName(), "row name");
    CHECK_EQUAL("--GG-A---T--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("GG-A---T", QString(row.getCore()), "core data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(2, row.getCoreStart(), "core start");
    CHECK_EQUAL(10, row.getCoreEnd(), "core end");
    CHECK_EQUAL(8, row.getCoreLength(), "core length");
    CHECK_EQUAL(12, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, setRowContent_offsetNoGap) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.setRowContent(0, "AC-GT", 1);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(MAlignmentRowTestUtils::rowWithGapsName, row.getName(), "row name");
    CHECK_EQUAL("-AC-GT-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("AC-GT", QString(row.getCore()), "core data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(1, row.getCoreStart(), "core start");
    CHECK_EQUAL(6, row.getCoreEnd(), "core end");
    CHECK_EQUAL(5, row.getCoreLength(), "core length");
    CHECK_EQUAL(7, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, setRowContent_offsetGap) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.setRowContent(0, "--GG", 1);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(MAlignmentRowTestUtils::rowWithGapsName, row.getName(), "row name");
    CHECK_EQUAL("---GG--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("GG", QString(row.getCore()), "core data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(3, row.getCoreStart(), "core start");
    CHECK_EQUAL(5, row.getCoreEnd(), "core end");
    CHECK_EQUAL(2, row.getCoreLength(), "core length");
    CHECK_EQUAL(7, row.getRowLength(), "row length");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, setRowContent_emptyAndOffset) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.setRowContent(0, "", 1);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(MAlignmentRowTestUtils::rowWithGapsName, row.getName(), "row name");
    CHECK_EQUAL("-------", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL("", QString(row.getCore()), "core data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number");
    CHECK_EQUAL(0, row.getCoreStart(), "core start");
    CHECK_EQUAL(0, row.getCoreEnd(), "core end");
    CHECK_EQUAL(0, row.getCoreLength(), "core length");
    CHECK_EQUAL(7, row.getRowLength(), "row length");
}

/** Tests insertGaps */
IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_empty) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initEmptyRow(almnt);
    U2OpStatusImpl os;
    almnt.insertGaps(0, 0, 2, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toGapPosLeft) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
    U2OpStatusImpl os;
    almnt.insertGaps(0, 3, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG--T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");

    almnt.insertGaps(0, 7, 2, os);
    row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG--T----AT", MAlignmentRowTestUtils::getRowData(row), "row data (second insertion)");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toGapPosRight) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
    U2OpStatusImpl os;
    almnt.insertGaps(0, 2, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG--T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");

    almnt.insertGaps(0, 5, 2, os);
    row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG--T----AT", MAlignmentRowTestUtils::getRowData(row), "row data (second insertion)");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toGapPosInside) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.insertGaps(0, 1, 2, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("-----AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_insideChars) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.insertGaps(0, 4, 2, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---A--G-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toZeroPosNoGap) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
    U2OpStatusImpl os;
    almnt.insertGaps(0, 0, 3, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---GG-T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toZeroPosGap) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.insertGaps(0, 0, 3, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("------AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toLastPosNoGap) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
    U2OpStatusImpl os;
    almnt.insertGaps(0, 7, 2, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG-T--A--T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toLastPosGap) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithTrailingGaps(almnt);
    U2OpStatusImpl os;
    almnt.insertGaps(0, 9, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("CA-GT--T---", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_toLastPosOneGap) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "A-", os);
    CHECK_NO_ERROR(os);
    almnt.insertGaps(0, 1, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_noGapsYet) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithoutGaps(almnt);
    U2OpStatusImpl os;
    almnt.insertGaps(0, 4, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("ACGT-A", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_onlyGaps) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "--", os);
    CHECK_NO_ERROR(os);
    almnt.insertGaps(0, 1, 2, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("----", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_oneChar) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("One-char sequence", "A", os);
    CHECK_NO_ERROR(os);
    almnt.insertGaps(0, 0, 2, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--A", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_tooBigPosition) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.insertGaps(0, 10, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("Failed to insert gaps into an alignment!", os.getError(), "opStatus");
    CHECK_EQUAL("---AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "gaps number");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_negativePosition) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    row.insertGaps(-1, 1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, insertGaps_negativeNumOfChars) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.insertGaps(0, 1, -1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("Failed to insert gaps into an alignment!", os.getError(), "opStatus");
    CHECK_EQUAL("---AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
}

/** Tests removeChars */
IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_empty) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initEmptyRow(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 0, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideGap1) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 2, 15, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---G---------------", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideGap2) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 3, 15, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---G---------------", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_leftGapSide) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 7, 9, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---ACG---G---------", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_rightGapSide) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 4, 11, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---C---G-----------", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideSeq1) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 5, 6, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---AT-A-C---G------", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideSeq2) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 6, 4, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---ACTT-A-C---G----", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_fromZeroPosGap) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 0, 4, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("G-T----", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_fromZeroPosChar) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 0, 17, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--G-----------------", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_lastPosExactly) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 7, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG-T--A-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_fromLastPos) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 7, 2, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG-T--A-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideOneGap1) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 2, 2, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A-ACG--GTT-A-C---G--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideOneGap2) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 2, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A--ACG--GTT-A-C---G-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideOneGapLong) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test sequence", "A------GT--C-T", os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("A------GT--C-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_NO_ERROR(os);
    almnt.removeChars(0, 2, 3, os);
    row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---GT--C-T---", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideTrailingGap) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "AC-GT----", os);
    CHECK_NO_ERROR(os);
    almnt.removeChars(0, 5, 2, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("AC-GT----", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_insideCharsOne) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 5, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---AG--GTT-A-C---G-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(5, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_negativePosition) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, -1, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("Failed to remove chars from an alignment!", os.getError(), "opStatus");
    CHECK_EQUAL("A---ACG--GTT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_negativeNumOfChars) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 1, -1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("Failed to remove chars from an alignment!", os.getError(), "opStatus");
    CHECK_EQUAL("A---ACG--GTT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_gapsAtRowEnd1) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 9, 12, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---ACG-------------", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_gapsAtRowEnd2) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 3, 21, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A-------------------", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_onlyGapsAfterRemove) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 2, 9, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("-------", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_emptyAfterRemove) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 0, 21, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--------------------", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, remove_oneCharInGaps) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.removeChars(0, 13, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A---ACG--GTT--C---G-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "number of gaps");
}

/** Tests charAt */
IMPLEMENT_TEST(MAlignmentRowUnitTests, charAt_allCharsNoOffset) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
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
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "-AC-", os);
    MAlignmentRow row = almnt.getRow(0);
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
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "ACG", os);
    MAlignmentRow row = almnt.getRow(0);
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
    MAlignment almnt;
    MAlignmentRow firstRow = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
    MAlignmentRow secondRow = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_TRUE(result, "The first and the second rows are NOT equal unexpectedly!");

    CHECK_TRUE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_FALSE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_noGaps) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("First", "ACT", os);
    MAlignmentRow firstRow = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second", "ACT", os);
    MAlignmentRow secondRow = almnt.getRow(1);
    CHECK_NO_ERROR(os);

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_TRUE(result, "The first and the second rows are NOT equal unexpectedly!");

    CHECK_TRUE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_FALSE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_trailingInFirst) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("First", "AC-GT-", os);
    MAlignmentRow firstRow = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    MAlignment almnt2("Test alignment");
    almnt2.addRow("Second", "AC-GT", os);
    MAlignmentRow secondRow = almnt2.getRow(0);
    CHECK_NO_ERROR(os);

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_TRUE(result, "The first and the second rows are NOT equal unexpectedly!");

    CHECK_TRUE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_FALSE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_trailingInSecond) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("First", "AC-GT", os);
    MAlignmentRow firstRow = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    MAlignment almnt2("Test alignment");
    almnt2.addRow("Second", "AC-GT--", os);
    MAlignmentRow secondRow = almnt2.getRow(0);
    CHECK_NO_ERROR(os);

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_TRUE(result, "The first and the second rows are NOT equal unexpectedly!");

    CHECK_TRUE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_FALSE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_trailingInBoth) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("First", "AC-GT---", os);
    MAlignmentRow firstRow = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    MAlignment almnt2("Test alignment");
    almnt2.addRow("Second", "AC-GT--", os);
    MAlignmentRow secondRow = almnt2.getRow(0);
    CHECK_NO_ERROR(os);

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_TRUE(result, "The first and the second rows are NOT equal unexpectedly!");

    CHECK_TRUE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_FALSE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_diffGapModelsGap) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test sequence", "--GG-A---T", os);
    MAlignmentRow firstRow = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T", MAlignmentRowTestUtils::getRowData(firstRow), "first row data");
    MAlignment almnt2("Test alignment");
    almnt2.addRow("Test sequence", "--GG--A---T", os);
    MAlignmentRow secondRow = almnt2.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG--A---T", MAlignmentRowTestUtils::getRowData(secondRow), "second row data");

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_FALSE(result, "The first and the second rows are EQUAL unexpectedly!");

    CHECK_FALSE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_TRUE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_diffGapModelsOffset) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test sequence", "--GG-A---T", os);
    MAlignmentRow firstRow = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T", MAlignmentRowTestUtils::getRowData(firstRow), "first row data");
    MAlignment almnt2("Test alignment");
    almnt2.addRow("Test sequence", "--G-GA---T", os);
    MAlignmentRow secondRow = almnt2.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--G-GA---T", MAlignmentRowTestUtils::getRowData(secondRow), "second row data");

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_FALSE(result, "The first and the second rows are EQUAL unexpectedly!");

    CHECK_FALSE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_TRUE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_diffNumOfGaps) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test sequence", "--GG-A---T", os);
    MAlignmentRow firstRow = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T", MAlignmentRowTestUtils::getRowData(firstRow), "first row data");
    MAlignment almnt2("Test alignment");
    almnt2.addRow("Test sequence", "--GG-AT", os);
    MAlignmentRow secondRow = almnt2.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-AT", MAlignmentRowTestUtils::getRowData(secondRow), "second row data");

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_FALSE(result, "The first and the second rows are EQUAL unexpectedly!");

    CHECK_FALSE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_TRUE(firstRow != secondRow, "Incorrect 'operator!='!");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, rowsEqual_diffSequences) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test sequence", "--GG-A---T", os);
    MAlignmentRow firstRow = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-A---T", MAlignmentRowTestUtils::getRowData(firstRow), "first row data");
    almnt.addRow("Test sequence", "--GG-C---T", os);
    MAlignmentRow secondRow = almnt.getRow(1);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("--GG-C---T", MAlignmentRowTestUtils::getRowData(secondRow), "second row data");

    bool result = firstRow.isRowContentEqual(secondRow);
    CHECK_FALSE(result, "The first and the second rows are EQUAL unexpectedly!");

    CHECK_FALSE(firstRow == secondRow, "Incorrect 'operator=='!");
    CHECK_TRUE(firstRow != secondRow, "Incorrect 'operator!='!");
}

/** Tests ungapped */
IMPLEMENT_TEST(MAlignmentRowUnitTests, ungapped_rowWithoutOffset) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
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
    MAlignment almnt("Test alignment");
    almnt.addRow("Test row", "---AG-T-", os);
    MAlignmentRow row = almnt.getRow(0);
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
    MAlignment almnt;
    MAlignmentRowTestUtils::initEmptyRow(almnt);
    U2OpStatusImpl os;
    almnt.crop(0, 1, os);
    CHECK_TRUE(os.getError().contains("Incorrect region was passed to MAlignment::crop"),
               QString("opStatus is %1").arg(os.getError()));
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideGap1) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(2, 15, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("--ACG--GTT-A-C-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideGap2) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(3, 15, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("-ACG--GTT-A-C--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_leftGapSide) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(7, 9, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("--GTT-A-C", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(3, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_rightGapSide) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(4, 11, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("ACG--GTT-A-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideSeq1) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(5, 6, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("CG--GT", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideSeq2) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(6, 4, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("G--G", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_fromZeroPosGap) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.crop(0, 4, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("---A", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(1, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_fromZeroPosChar) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(0, 17, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("A---ACG--GTT-A-C-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(4, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_lastPosExactly) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
    U2OpStatusImpl os;
    almnt.crop(7, 1, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_fromLastPos) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
    U2OpStatusImpl os;
    almnt.crop(7, 2, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideOneGap1) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(2, 2, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideOneGap2) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(2, 1, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideOneGapLong) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Test sequence", "A------GT--C-T", os);
    MAlignmentRow row = almnt.getRow(0);

    CHECK_EQUAL("A------GT--C-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_NO_ERROR(os);
    almnt.crop(2, 3, os);
    row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("---", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_insideCharsOne) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(5, 1, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("C", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_negativePosition) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(-1, 1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_TRUE(os.getError().contains("Incorrect region was passed to MAlignment::crop"),
               QString("opStatus is %1").arg(os.getError()));
    CHECK_EQUAL("A---ACG--GTT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_negativeNumOfChars) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(1, -1, os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_TRUE(os.getError().contains("Incorrect region was passed to MAlignment::crop"),
               QString("opStatus is %1").arg(os.getError()));
    CHECK_EQUAL("A---ACG--GTT-A-C---G", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_trailing) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithTrailingGaps(almnt);
    U2OpStatusImpl os;
    almnt.crop(2, 8, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("-GT--T--", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_trailingToGaps) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithTrailingGaps(almnt);
    U2OpStatusImpl os;
    almnt.crop(0, 9, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("CA-GT--T-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_cropTrailing) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithTrailingGaps(almnt);
    U2OpStatusImpl os;
    almnt.crop(9, 1, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("-", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_oneCharInGaps) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    almnt.crop(13, 1, os);
    CHECK_NO_ERROR(os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("A", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(0, row.getGapModel().count(), "number of gaps");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, crop_posMoreThanLength) {
    MAlignment almnt;
    MAlignmentRowTestUtils::initTestRowWithGaps(almnt);
    U2OpStatusImpl os;
    almnt.crop(13, 1, os);
    CHECK_TRUE(os.getError().contains("Incorrect region was passed to MAlignment::crop"),
               QString("opStatus is %1").arg(os.getError()));
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL(7, row.getRowLength(), "row length");
    CHECK_EQUAL("---AG-T", MAlignmentRowTestUtils::getRowData(row), "row data");
    CHECK_EQUAL(2, row.getGapModel().count(), "number of gaps");
}

/** Tests mid */
IMPLEMENT_TEST(MAlignmentRowUnitTests, mid_general) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowForModification(almnt);
    U2OpStatusImpl os;
    MAlignmentRow result = row.mid(4, 8, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("ACG--GTT------------", MAlignmentRowTestUtils::getRowData(result), "row data");
    CHECK_EQUAL(1, result.getGapModel().count(), "number of gaps");
}

/** Tests upperCase */
IMPLEMENT_TEST(MAlignmentRowUnitTests, upperCase_general) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Row name", "avn-*y-s", os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    row.toUpperCase();
    CHECK_EQUAL("AVN-*Y-S", MAlignmentRowTestUtils::getRowData(row), "row data");

    QString actualRowName = row.getName();
    CHECK_EQUAL("Row name", actualRowName, "row name");
}

/** Tests replaceChars */
IMPLEMENT_TEST(MAlignmentRowUnitTests, replaceChars_charToChar) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
    U2OpStatusImpl os;
    row.replaceChars('T', 'C', os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG-C--AC", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, replaceChars_nothingToReplace) {
    MAlignment almnt;
    MAlignmentRow row = MAlignmentRowTestUtils::initTestRowWithGapsInMiddle(almnt);
    U2OpStatusImpl os;
    row.replaceChars('~', '-', os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("GG-T--AT", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, replaceChars_tildasToGapsNoGaps) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Row name", "A~~CC~~~AG~AC~TG", os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    row.replaceChars('~', '-', os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A--CC---AG-AC-TG", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, replaceChars_tildasToGapsWithGaps) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Row name", "A~-CC~-~AG~AC-TG", os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    row.replaceChars('~', '-', os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A--CC---AG-AC-TG", MAlignmentRowTestUtils::getRowData(row), "row data");
}

IMPLEMENT_TEST(MAlignmentRowUnitTests, replaceChars_trailingGaps) {
    U2OpStatusImpl os;
    MAlignment almnt("Test alignment");
    almnt.addRow("Row name", "A~~CC~~~AG~AC~TG~", os);
    MAlignmentRow row = almnt.getRow(0);
    CHECK_NO_ERROR(os);
    row.replaceChars('~', '-', os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL("A--CC---AG-AC-TG-", MAlignmentRowTestUtils::getRowData(row), "row data");
}


} // namespace
