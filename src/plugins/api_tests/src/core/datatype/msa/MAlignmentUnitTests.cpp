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

#include "MAlignmentUnitTests.h"
#include "MAlignmentRowUnitTests.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequence.h>
#include <U2Core/U2Msa.h>
#include <U2Core/U2OpStatusUtils.h>

#include <QSet>


namespace U2 {

const int MAlignmentTestUtils::rowsNum = 2;
const int MAlignmentTestUtils::firstRowLength = 7;
const int MAlignmentTestUtils::secondRowLength = 9;

const QString MAlignmentTestUtils::alignmentName = "Test alignment name";


MAlignment MAlignmentTestUtils::initTestAlignment() {
    DNAAlphabetRegistry* alphabetRegistry = AppContext::getDNAAlphabetRegistry();
    DNAAlphabet* alphabet = alphabetRegistry->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());

    QByteArray firstSequence("---AG-T");
    QByteArray secondSequence("AG-CT-TAA");

    MAlignment almnt(alignmentName, alphabet);

    U2OpStatusImpl os;
    almnt.addRow("First row", firstSequence, os);
    CHECK_OP(os, MAlignment());

    almnt.addRow("Second row", secondSequence, os);
    CHECK_OP(os, MAlignment());

    return almnt;
}

QString MAlignmentTestUtils::getRowData(const MAlignment& almnt, int rowNum) {
    if (rowNum < 0 || rowNum > almnt.getNumRows()) {
        return "";
    }

    MAlignmentRow row = almnt.getRow(rowNum);

    return MAlignmentRowTestUtils::getRowData(row);
}

bool MAlignmentTestUtils::testAlignmentNotChanged(const MAlignment& almnt) {
    if (9 != almnt.getLength()) {
        return false;
    }

    if ("---AG-T" != MAlignmentTestUtils::getRowData(almnt, 0)) {
        return false;
    }

    if ("AG-CT-TAA" != MAlignmentTestUtils::getRowData(almnt, 1)) {
        return false;
    }
    
    return true;
}


/** Tests clear */
IMPLEMENT_TEST(MAlignmentUnitTests, clear_notEmpty) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    almnt.clear();
    QList<MAlignmentRow> rows = almnt.getRows();
    CHECK_EQUAL(0, rows.count(), "number of rows");
}

/** Tests name */
IMPLEMENT_TEST(MAlignmentUnitTests, name_ctor) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    CHECK_EQUAL(MAlignmentTestUtils::alignmentName, almnt.getName(), "alignment name");
}

IMPLEMENT_TEST(MAlignmentUnitTests, name_setName) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    QString newName = "Another name";
    almnt.setName(newName);
    CHECK_EQUAL(newName, almnt.getName(), "alignment name");
}

/** Tests alphabet */
IMPLEMENT_TEST(MAlignmentUnitTests, alphabet_ctor) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    if (NULL == almnt.getAlphabet()) {
        SetError("NULL alphabet!");
    }
    CHECK_EQUAL(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT(), almnt.getAlphabet()->getId(), "alphabet ID");
}

IMPLEMENT_TEST(MAlignmentUnitTests, alphabet_setAlphabet) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();

    DNAAlphabetRegistry* alphabetRegistry = AppContext::getDNAAlphabetRegistry();
    DNAAlphabet* newAlphabet = alphabetRegistry->findById(BaseDNAAlphabetIds::NUCL_DNA_EXTENDED());
    almnt.setAlphabet(newAlphabet);

    if (NULL == almnt.getAlphabet() || NULL == newAlphabet) {
        SetError("NULL alphabet!");
    }
    CHECK_EQUAL(newAlphabet->getId(), almnt.getAlphabet()->getId(), "new alignment ID");
}

/** Tests info */
IMPLEMENT_TEST(MAlignmentUnitTests, info_setGet) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    QVariantMap info;
    QString infoElementName = "Test element name";
    QString infoElementValue = "Test element value";
    info.insert(infoElementName, infoElementValue);
    almnt.setInfo(info);
    QVariantMap actualInfo = almnt.getInfo();
    QString actualValue = actualInfo.value(infoElementName).value<QString>();
    CHECK_EQUAL(infoElementValue, actualValue, "info element value");
}

/** Tests length */
IMPLEMENT_TEST(MAlignmentUnitTests, length_isEmptyFalse) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    CHECK_FALSE(almnt.isEmpty(), "Method isEmpty() returned 'true' unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, length_isEmptyTrue) {
    MAlignment almnt = MAlignment();
    CHECK_TRUE(almnt.isEmpty(), "Method isEmpty() returned 'false' unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, length_get) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    int expectedLength = 9; // the length of the longest row
    CHECK_EQUAL(expectedLength, almnt.getLength(), "alignment length");
}

IMPLEMENT_TEST(MAlignmentUnitTests, length_getForEmpty) {
    MAlignment almnt = MAlignment();
    CHECK_EQUAL(0, almnt.getLength(), "alignment length");
}

IMPLEMENT_TEST(MAlignmentUnitTests, length_setLessLength) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    int newLength = 5;
    almnt.setLength(newLength);
    CHECK_EQUAL(newLength, almnt.getLength(), "alignment length");

    CHECK_EQUAL("---AG", MAlignmentTestUtils::getRowData(almnt, 0), "first row data");
    CHECK_EQUAL("AG-CT", MAlignmentTestUtils::getRowData(almnt, 1), "second row data");
}

/** Tests numOfRows */
IMPLEMENT_TEST(MAlignmentUnitTests, numOfRows_notEmpty) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    CHECK_EQUAL(MAlignmentTestUtils::rowsNum, almnt.getNumRows(), "number of rows");
}

IMPLEMENT_TEST(MAlignmentUnitTests, numOfRows_empty) {
    MAlignment almnt = MAlignment();
    CHECK_EQUAL(0, almnt.getNumRows(), "number of rows");
}

/** Tests trim */
IMPLEMENT_TEST(MAlignmentUnitTests, trim_biggerLength) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    int newLength = 100;
    almnt.setLength(newLength);
    bool result = almnt.trim();
    CHECK_TRUE(result, "Method trim() returned 'false' unexpectedly!");
    CHECK_EQUAL(9, almnt.getLength(), "alignment length");
    CHECK_EQUAL("AG-CT-TAA", MAlignmentTestUtils::getRowData(almnt, 1), "row data");
}

IMPLEMENT_TEST(MAlignmentUnitTests, trim_leadingGapColumns) {
    QByteArray firstSequence("---AG-T");
    QByteArray secondSequence("--AG-CT-TA");

    MAlignment almnt = MAlignment("Alignment with leading gap columns", NULL);
    U2OpStatusImpl os;
    almnt.addRow("First row", firstSequence, os);
    CHECK_NO_ERROR(os);

    almnt.addRow("Second row", secondSequence, os);
    CHECK_NO_ERROR(os);

    bool result = almnt.trim();

    CHECK_TRUE(result, "Method trim() returned 'false' unexpectedly!");
    CHECK_EQUAL(8, almnt.getLength(), "alignment length");
    CHECK_EQUAL("-AG-T", MAlignmentTestUtils::getRowData(almnt, 0), "first row data");
    CHECK_EQUAL("AG-CT-TA", MAlignmentTestUtils::getRowData(almnt, 1), "second row data");
}

IMPLEMENT_TEST(MAlignmentUnitTests, trim_nothingToTrim) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    bool result = almnt.trim();
    CHECK_FALSE(result, "Method trim() returned 'true' unexpectedly!");
    CHECK_EQUAL(9, almnt.getLength(), "alignment length");
    CHECK_EQUAL("AG-CT-TAA", MAlignmentTestUtils::getRowData(almnt, 1), "row data");
}

IMPLEMENT_TEST(MAlignmentUnitTests, trim_rowWithoutGaps) {
    QByteArray seq("ACGTAGTCGATC");

    MAlignment almnt = MAlignment("Alignment", NULL);
    U2OpStatusImpl os;
    almnt.addRow("Row without gaps", seq, os);
    CHECK_NO_ERROR(os);

    bool result = almnt.trim();

    CHECK_FALSE(result, "Method trim() returned 'true' unexpectedly!");
    CHECK_EQUAL(12, almnt.getLength(), "alignment length");
    CHECK_EQUAL("ACGTAGTCGATC", MAlignmentTestUtils::getRowData(almnt, 0), "row data");
}

IMPLEMENT_TEST(MAlignmentUnitTests, trim_empty) {
    MAlignment almnt = MAlignment();
    bool result = almnt.trim();
    CHECK_FALSE(result, "Method trim() returned 'true' unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, trim_trailingGapInOne) {
    QByteArray firstSequence("ACGT");
    QByteArray secondSequence("CAC-");

    MAlignment almnt = MAlignment("Alignment", NULL);
    U2OpStatusImpl os;
    almnt.addRow("First row", firstSequence, os);
    CHECK_NO_ERROR(os);

    almnt.addRow("Second row", secondSequence, os);
    CHECK_NO_ERROR(os);

    bool result = almnt.trim();

    CHECK_FALSE(result, "Method trim() returned 'true' unexpectedly!");
    CHECK_EQUAL(4, almnt.getLength(), "alignment length");
    CHECK_EQUAL("ACGT", MAlignmentTestUtils::getRowData(almnt, 0), "first row data");
    CHECK_EQUAL("CAC-", MAlignmentTestUtils::getRowData(almnt, 1), "second row data");
    CHECK_EQUAL(1, almnt.getRow(1).getGapModel().size(), "number of gaps in the second row");
}

/** Tests simplify */
IMPLEMENT_TEST(MAlignmentUnitTests, simplify_withGaps) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    bool result = almnt.simplify();
    CHECK_TRUE(result, "Method simplify() returned 'false' unexpectedly!");
    CHECK_EQUAL(7, almnt.getLength(), "alignment length");
    CHECK_EQUAL("AGT", MAlignmentTestUtils::getRowData(almnt, 0), "first row data");
    CHECK_EQUAL("AGCTTAA", MAlignmentTestUtils::getRowData(almnt, 1), "second row data");
}

IMPLEMENT_TEST(MAlignmentUnitTests, simplify_withoutGaps) {
    QByteArray seq("ACGTAGTCGATC");

    MAlignment almnt = MAlignment("Alignment", NULL);
    U2OpStatusImpl os;
    almnt.addRow("Row without gaps", seq, os);
    CHECK_NO_ERROR(os);

    bool result = almnt.simplify();

    CHECK_FALSE(result, "Method simplify() returned 'true' unexpectedly!");
    CHECK_EQUAL(12, almnt.getLength(), "alignment length");
    CHECK_EQUAL("ACGTAGTCGATC", MAlignmentTestUtils::getRowData(almnt, 0), "row data");
}

IMPLEMENT_TEST(MAlignmentUnitTests, simplify_empty) {
    MAlignment almnt = MAlignment();
    bool result = almnt.simplify();
    CHECK_FALSE(result, "Method simplify() returned 'true' unexpectedly!");
}

/** Tests sortRows */
IMPLEMENT_TEST(MAlignmentUnitTests, sortRows_byNameAsc) {
    QByteArray firstSequence("AAAAAA");
    QByteArray secondSequence("CCCCCC");
    QByteArray thirdSequence("TTTTTT");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name", NULL);
    almnt.addRow("NameBA", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("NameAB", secondSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("NameAA", thirdSequence, os);
    CHECK_NO_ERROR(os);

    almnt.sortRowsByName();
    QStringList rowNames = almnt.getRowNames();
    CHECK_EQUAL(3, rowNames.count(), "number of rows");
    CHECK_EQUAL("NameAA", rowNames[0], "order");
    CHECK_EQUAL("NameAB", rowNames[1], "order");
    CHECK_EQUAL("NameBA", rowNames[2], "order");
    CHECK_EQUAL("TTTTTT", MAlignmentTestUtils::getRowData(almnt, 0), "first row sequence");
}

IMPLEMENT_TEST(MAlignmentUnitTests, sortRows_byNameDesc) {
    QByteArray firstSequence("AAAAAA");
    QByteArray secondSequence("CCCCCC");
    QByteArray thirdSequence("TTTTTT");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name", NULL);
    almnt.addRow("NameAA", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("NameBA", secondSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("NameAB", thirdSequence, os);
    CHECK_NO_ERROR(os);

    almnt.sortRowsByName(false);
    QStringList rowNames = almnt.getRowNames();
    CHECK_EQUAL(3, rowNames.count(), "number of rows");
    CHECK_EQUAL("NameBA", rowNames[0], "order");
    CHECK_EQUAL("NameAB", rowNames[1], "order");
    CHECK_EQUAL("NameAA", rowNames[2], "order");
    CHECK_EQUAL("CCCCCC", MAlignmentTestUtils::getRowData(almnt, 0), "first row sequence");
}

IMPLEMENT_TEST(MAlignmentUnitTests, sortRows_twoSimilar) {
    QByteArray firstSequence("AAAAAA");
    QByteArray secondSequence("CCCCCC");
    QByteArray thirdSequence("TTTTTT");
    QByteArray forthSequence("AAAAAA");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name");
    almnt.addRow("First", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second", secondSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Third", thirdSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Forth", forthSequence, os);
    CHECK_NO_ERROR(os);

    QVector<U2Region> unitedRegions;
    almnt.sortRowsBySimilarity(unitedRegions);
    QStringList rowNames = almnt.getRowNames();
    CHECK_EQUAL(4, rowNames.count(), "number of rows");
    CHECK_EQUAL("First", rowNames[0], "order");
    CHECK_EQUAL("Forth", rowNames[1], "order");
    CHECK_EQUAL("Second", rowNames[2], "order");
    CHECK_EQUAL("Third", rowNames[3], "order");

    CHECK_EQUAL("AAAAAA", MAlignmentTestUtils::getRowData(almnt, 0), "row1");
    CHECK_EQUAL("AAAAAA", MAlignmentTestUtils::getRowData(almnt, 1), "row2");
    CHECK_EQUAL("CCCCCC", MAlignmentTestUtils::getRowData(almnt, 2), "row3");
    CHECK_EQUAL("TTTTTT", MAlignmentTestUtils::getRowData(almnt, 3), "row4");
    CHECK_EQUAL(1, unitedRegions.count(), "number of similar regions");
    U2Region reg = unitedRegions[0];
    CHECK_EQUAL(0, reg.startPos, "similar rows region start position");
    CHECK_EQUAL(2, reg.length, "similar rows region length");
}

IMPLEMENT_TEST(MAlignmentUnitTests, sortRows_threeSimilar) {
    QByteArray firstSequence("AAAAAA");
    QByteArray secondSequence("AAAAAA");
    QByteArray thirdSequence("TTTTTT");
    QByteArray forthSequence("AAAAAA");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name");
    almnt.addRow("First", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second", secondSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Third", thirdSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Forth", forthSequence, os);
    CHECK_NO_ERROR(os);

    QVector<U2Region> unitedRegions;
    almnt.sortRowsBySimilarity(unitedRegions);
    QStringList rowNames = almnt.getRowNames();
    CHECK_EQUAL(4, rowNames.count(), "number of rows");
    CHECK_EQUAL("First", rowNames[0], "order");
    CHECK_EQUAL("Second", rowNames[1], "order");
    CHECK_EQUAL("Forth", rowNames[2], "order");
    CHECK_EQUAL("Third", rowNames[3], "order");

    CHECK_EQUAL("AAAAAA", MAlignmentTestUtils::getRowData(almnt, 0), "row1");
    CHECK_EQUAL("AAAAAA", MAlignmentTestUtils::getRowData(almnt, 1), "row2");
    CHECK_EQUAL("AAAAAA", MAlignmentTestUtils::getRowData(almnt, 2), "row3");
    CHECK_EQUAL("TTTTTT", MAlignmentTestUtils::getRowData(almnt, 3), "row4");

    CHECK_EQUAL(1, unitedRegions.count(), "number of similar regions");
    U2Region reg = unitedRegions[0];
    CHECK_EQUAL(0, reg.startPos, "similar rows region start position");
    CHECK_EQUAL(3, reg.length, "similar rows region length");
}

IMPLEMENT_TEST(MAlignmentUnitTests, sortRows_similarTwoRegions) {
    QByteArray firstSequence("AAAAAA");
    QByteArray secondSequence("CCCCCC");
    QByteArray thirdSequence("AAAAAA");
    QByteArray forthSequence("CCCCCC");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name");
    almnt.addRow("First", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second", secondSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Third", thirdSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Forth", forthSequence, os);
    CHECK_NO_ERROR(os);

    QVector<U2Region> unitedRegions;
    almnt.sortRowsBySimilarity(unitedRegions);
    QStringList rowNames = almnt.getRowNames();
    CHECK_EQUAL(4, rowNames.count(), "number of rows");
    CHECK_EQUAL("First", rowNames[0], "order");
    CHECK_EQUAL("Third", rowNames[1], "order");
    CHECK_EQUAL("Second", rowNames[2], "order");
    CHECK_EQUAL("Forth", rowNames[3], "order");

    CHECK_EQUAL("AAAAAA", MAlignmentTestUtils::getRowData(almnt, 0), "row1");
    CHECK_EQUAL("AAAAAA", MAlignmentTestUtils::getRowData(almnt, 1), "row2");
    CHECK_EQUAL("CCCCCC", MAlignmentTestUtils::getRowData(almnt, 2), "row3");
    CHECK_EQUAL("CCCCCC", MAlignmentTestUtils::getRowData(almnt, 3), "row4");

    CHECK_EQUAL(2, unitedRegions.count(), "number of similar regions");
    U2Region reg = unitedRegions[0];
    CHECK_EQUAL(0, reg.startPos, "similar rows first region start position");
    CHECK_EQUAL(2, reg.length, "similar rows first region length");
    reg = unitedRegions[1];
    CHECK_EQUAL(2, reg.startPos, "similar rows second region start position");
    CHECK_EQUAL(2, reg.length, "similar rows second region length");
}

/** Tests getRows */
IMPLEMENT_TEST(MAlignmentUnitTests, getRows_oneRow) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    MAlignmentRow row = almnt.getRow(0);
    CHECK_EQUAL("---AG-T", MAlignmentRowTestUtils::getRowData(row), "first row");
}

IMPLEMENT_TEST(MAlignmentUnitTests, getRows_severalRows) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    QList<MAlignmentRow> rows = almnt.getRows();
    CHECK_EQUAL(2, rows.count(), "number of rows");
    CHECK_EQUAL("---AG-T", MAlignmentRowTestUtils::getRowData(rows[0]), "first row");
    CHECK_EQUAL("AG-CT-TAA", MAlignmentRowTestUtils::getRowData(rows[1]), "second row");
}

IMPLEMENT_TEST(MAlignmentUnitTests, getRows_rowNames) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    QStringList rowNames = almnt.getRowNames();
    CHECK_EQUAL(2, rowNames.count(), "number of rows");
    CHECK_EQUAL("First row", rowNames[0], "first row name");
    CHECK_EQUAL("Second row", rowNames[1], "second row name");
}

/** Tests charAt */
IMPLEMENT_TEST(MAlignmentUnitTests, charAt_nonGapChar) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    char result = almnt.charAt(0, 3);
    CHECK_EQUAL('A', result, "char inside first row");
}

IMPLEMENT_TEST(MAlignmentUnitTests, charAt_gap) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    char result = almnt.charAt(1, 2);
    CHECK_EQUAL('-', result, "gap inside second row");
}

/** Tests insertGaps */
IMPLEMENT_TEST(MAlignmentUnitTests, insertGaps_validParams) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.insertGaps(0, 4, 3, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(10, almnt.getLength(), "alignment length");
    CHECK_EQUAL("---A---G-T", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
}

IMPLEMENT_TEST(MAlignmentUnitTests, insertGaps_toBeginningLength) {
    QByteArray firstSequence("ACGT");
    QByteArray secondSequence("ACC");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment");
    almnt.addRow("First", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second", secondSequence, os);
    CHECK_NO_ERROR(os);

    almnt.insertGaps(1, 0, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(5, almnt.getLength(), "alignment length");
    CHECK_EQUAL("--ACC", MAlignmentTestUtils::getRowData(almnt, 1), "second row");
}

IMPLEMENT_TEST(MAlignmentUnitTests, insertGaps_negativeRowIndex) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.insertGaps(-1, 4, 3, os);
    CHECK_EQUAL("Failed to insert gaps into an alignment!", os.getError(), "opStatus");
    CHECK_TRUE(MAlignmentTestUtils::testAlignmentNotChanged(almnt), "Alignment changed unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, insertGaps_tooBigRowIndex) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.insertGaps(2, 4, 3, os);
    CHECK_EQUAL("Failed to insert gaps into an alignment!", os.getError(), "opStatus");
    CHECK_TRUE(MAlignmentTestUtils::testAlignmentNotChanged(almnt), "Alignment changed unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, insertGaps_negativePos) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.insertGaps(0, -1, 3, os);
    CHECK_EQUAL("Failed to insert gaps into an alignment!", os.getError(), "opStatus");
    CHECK_TRUE(MAlignmentTestUtils::testAlignmentNotChanged(almnt), "Alignment changed unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, insertGaps_tooBigPos) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.insertGaps(0, 10, 3, os);
    CHECK_EQUAL("Failed to insert gaps into an alignment!", os.getError(), "opStatus");
    CHECK_TRUE(MAlignmentTestUtils::testAlignmentNotChanged(almnt), "Alignment changed unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, insertGaps_negativeCount) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.insertGaps(0, 4, -1, os);
    CHECK_EQUAL("Failed to insert gaps into an alignment!", os.getError(), "opStatus");
    CHECK_TRUE(MAlignmentTestUtils::testAlignmentNotChanged(almnt), "Alignment changed unexpectedly!");
}

/** Tests removeChars */
IMPLEMENT_TEST(MAlignmentUnitTests, removeChars_validParamsAndTrimmed) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.removeChars(1, 0, 2, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(6, almnt.getLength(), "alignment length");
    CHECK_EQUAL("--AG-T", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL("CT-TAA", MAlignmentTestUtils::getRowData(almnt, 1), "second row");
}

IMPLEMENT_TEST(MAlignmentUnitTests, removeChars_negativeRowIndex) {    
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.removeChars(-1, 0, 2, os);
    CHECK_EQUAL("Failed to remove chars from an alignment!", os.getError(), "opStatus");
    CHECK_TRUE(MAlignmentTestUtils::testAlignmentNotChanged(almnt), "Alignment changed unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, removeChars_tooBigRowIndex) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.removeChars(2, 0, 2, os);
    CHECK_EQUAL("Failed to remove chars from an alignment!", os.getError(), "opStatus");
    CHECK_TRUE(MAlignmentTestUtils::testAlignmentNotChanged(almnt), "Alignment changed unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, removeChars_negativePos) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.removeChars(1, -1, 2, os);
    CHECK_EQUAL("Failed to remove chars from an alignment!", os.getError(), "opStatus");
    CHECK_TRUE(MAlignmentTestUtils::testAlignmentNotChanged(almnt), "Alignment changed unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, removeChars_tooBigPos) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.removeChars(1, 10, 2, os);
    CHECK_EQUAL("Failed to remove chars from an alignment!", os.getError(), "opStatus");
    CHECK_TRUE(MAlignmentTestUtils::testAlignmentNotChanged(almnt), "Alignment changed unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, removeChars_negativeCount) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.removeChars(1, 0, -1, os);
    CHECK_EQUAL("Failed to remove chars from an alignment!", os.getError(), "opStatus");
    CHECK_TRUE(MAlignmentTestUtils::testAlignmentNotChanged(almnt), "Alignment changed unexpectedly!");
}

/** Tests removeRegion */
IMPLEMENT_TEST(MAlignmentUnitTests, removeRegion_validParams) {
    QByteArray firstSequence("---ACT");
    QByteArray secondSequence("A-CG-A");
    QByteArray thirdSequence("---CGA");
    QByteArray forthSequence("AAAAAA");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name");
    almnt.addRow("First", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second", secondSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Third", thirdSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Forth", forthSequence, os);
    CHECK_NO_ERROR(os);

    almnt.removeRegion(1, 1, 3, 2, false);

    CHECK_EQUAL(4, almnt.getNumRows(), "number of rows");
    CHECK_EQUAL("---ACT", MAlignmentTestUtils::getRowData(almnt, 0), "row1");
    CHECK_EQUAL("A-A", MAlignmentTestUtils::getRowData(almnt, 1), "row2");
    CHECK_EQUAL("-GA", MAlignmentTestUtils::getRowData(almnt, 2), "row3");
    CHECK_EQUAL("AAAAAA", MAlignmentTestUtils::getRowData(almnt, 3), "row4");
}

IMPLEMENT_TEST(MAlignmentUnitTests, removeRegion_removeEmpty) {
    QByteArray firstSequence("---ACT");
    QByteArray secondSequence("A-CG-A");
    QByteArray thirdSequence("--AC");
    QByteArray forthSequence("AAAAAA");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name");
    almnt.addRow("First", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second", secondSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Third", thirdSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Forth", forthSequence, os);
    CHECK_NO_ERROR(os);

    almnt.removeRegion(1, 1, 3, 2, true);

    CHECK_EQUAL(3, almnt.getNumRows(), "number of rows");
    CHECK_EQUAL("---ACT", MAlignmentTestUtils::getRowData(almnt, 0), "row1");
    CHECK_EQUAL("A-A", MAlignmentTestUtils::getRowData(almnt, 1), "row2");
    CHECK_EQUAL("AAAAAA", MAlignmentTestUtils::getRowData(almnt, 2), "row3");
}

IMPLEMENT_TEST(MAlignmentUnitTests, removeRegion_trimmed) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    almnt.removeRegion(0, 1, 2, 1, false);
    CHECK_EQUAL(6, almnt.getLength(), "alignment length");
    CHECK_EQUAL("--AG-T", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL("CT-TAA", MAlignmentTestUtils::getRowData(almnt, 1), "second row");
}

/** Tests renameRow */
IMPLEMENT_TEST(MAlignmentUnitTests, renameRow_validParams) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    QString newRowName = "New row name";
    almnt.renameRow(0, newRowName);
    MAlignmentRow actualRow = almnt.getRow(0);
    CHECK_EQUAL(newRowName, actualRow.getName(), "renamed row name");
}

/** Tests setRowContent */
IMPLEMENT_TEST(MAlignmentUnitTests, setRowContent_validParamsAndNotTrimmed) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    almnt.setRowContent(1, "---AC-");
    CHECK_EQUAL(7, almnt.getLength(), "alignment length");
    CHECK_EQUAL("---AG-T", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL("---AC-", MAlignmentTestUtils::getRowData(almnt, 1), "second row");
}

IMPLEMENT_TEST(MAlignmentUnitTests, setRowContent_lengthIsIncreased) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    almnt.setRowContent(0, "ACGT-ACA-ACA");
    CHECK_EQUAL(12, almnt.getLength(), "alignment length");
    CHECK_EQUAL("ACGT-ACA-ACA", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL("AG-CT-TAA", MAlignmentTestUtils::getRowData(almnt, 1), "second row");
}

/** Tests upperCase */
IMPLEMENT_TEST(MAlignmentUnitTests, upperCase_charsAndGaps) {
    QByteArray sequence1("mMva-ke");
    QByteArray sequence2("avn-*y-s");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment with chars in lower-case");
    almnt.addRow("First row", sequence1, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second row", sequence2, os);
    CHECK_NO_ERROR(os);

    almnt.toUpperCase();

    CHECK_EQUAL("MMVA-KE", MAlignmentTestUtils::getRowData(almnt, 0), "row1");
    CHECK_EQUAL("AVN-*Y-S", MAlignmentTestUtils::getRowData(almnt, 1), "row2");
}

/** Tests crop */
IMPLEMENT_TEST(MAlignmentUnitTests, crop_validParams) {
    QByteArray firstSequence("---ACT");
    QByteArray secondSequence("A-CG-A");
    QByteArray thirdSequence("---CGA");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name");
    almnt.addRow("First", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second", secondSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Third", thirdSequence, os);
    CHECK_NO_ERROR(os);

    U2Region region(1, 4);
    QSet<QString> rowNames;
    rowNames << "First" << "Second";

    almnt.crop(region, rowNames);

    CHECK_EQUAL(2, almnt.getNumRows(), "number of rows");
    CHECK_EQUAL(4, almnt.getLength(), "alignment length");
    CHECK_EQUAL("--AC", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL("-CG-", MAlignmentTestUtils::getRowData(almnt, 1), "second row");
}

/** Tests mid */
IMPLEMENT_TEST(MAlignmentUnitTests, mid_validParams) {
    QByteArray firstSequence("---ACT");
    QByteArray secondSequence("A");
    QByteArray thirdSequence("---CGA");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name");
    almnt.addRow("First", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second", secondSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Third", thirdSequence, os);
    CHECK_NO_ERROR(os);

    MAlignment almntResult = almnt.mid(2, 3);
    CHECK_EQUAL(3, almntResult.getNumRows(), "number of rows");
    CHECK_EQUAL("-AC", MAlignmentTestUtils::getRowData(almntResult, 0), "first row");
    CHECK_EQUAL("", MAlignmentTestUtils::getRowData(almntResult, 1), "second row");
    CHECK_EQUAL("-CG", MAlignmentTestUtils::getRowData(almntResult, 2), "third row");

    CHECK_EQUAL("---ACT", MAlignmentTestUtils::getRowData(almnt, 0), "first row of the original");
}

/** Tests addRow */
IMPLEMENT_TEST(MAlignmentUnitTests, addRow_appendRowFromBytes) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.addRow("Added row", "--AACT-GAG", os);
    CHECK_NO_ERROR(os);

    CHECK_EQUAL(3, almnt.getNumRows(), "number of rows");
    CHECK_EQUAL("---AG-T", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL("AG-CT-TAA", MAlignmentTestUtils::getRowData(almnt, 1), "second row");
    CHECK_EQUAL("--AACT-GAG", MAlignmentTestUtils::getRowData(almnt, 2), "third row");
    CHECK_EQUAL("Added row", almnt.getRowNames().at(2), "added row name");
    CHECK_EQUAL(10, almnt.getLength(), "alignment length");
}

IMPLEMENT_TEST(MAlignmentUnitTests, addRow_rowFromBytesToIndex) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.addRow("Added row", "--AACT-GAG", 1, os);
    CHECK_NO_ERROR(os);

    CHECK_EQUAL(3, almnt.getNumRows(), "number of rows");
    CHECK_EQUAL("---AG-T", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL("--AACT-GAG", MAlignmentTestUtils::getRowData(almnt, 1), "second row");
    CHECK_EQUAL("AG-CT-TAA", MAlignmentTestUtils::getRowData(almnt, 2), "third row");
    CHECK_EQUAL("Added row", almnt.getRowNames().at(1), "added row name");
    CHECK_EQUAL(10, almnt.getLength(), "alignment length");
}

IMPLEMENT_TEST(MAlignmentUnitTests, addRow_zeroBound) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.addRow("Added row", "--AACT-GAG", -2, os);
    CHECK_NO_ERROR(os);

    CHECK_EQUAL(3, almnt.getNumRows(), "number of rows");
    CHECK_EQUAL("--AACT-GAG", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL("---AG-T", MAlignmentTestUtils::getRowData(almnt, 1), "second row");
    CHECK_EQUAL("AG-CT-TAA", MAlignmentTestUtils::getRowData(almnt, 2), "third row");
    CHECK_EQUAL("Added row", almnt.getRowNames().at(0), "added row name");
    CHECK_EQUAL(10, almnt.getLength(), "alignment length");
}

IMPLEMENT_TEST(MAlignmentUnitTests, addRow_rowsNumBound) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.addRow("Added row", "--AACT-GAG", 3, os);
    CHECK_NO_ERROR(os);

    CHECK_EQUAL(3, almnt.getNumRows(), "number of rows");
    CHECK_EQUAL("---AG-T", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL("AG-CT-TAA", MAlignmentTestUtils::getRowData(almnt, 1), "second row");
    CHECK_EQUAL("--AACT-GAG", MAlignmentTestUtils::getRowData(almnt, 2), "third row");
    CHECK_EQUAL("Added row", almnt.getRowNames().at(2), "added row name");
    CHECK_EQUAL(10, almnt.getLength(), "alignment length");
}

/** Tests removeRow */
IMPLEMENT_TEST(MAlignmentUnitTests, removeRow_validIndex) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.removeRow(1, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(1, almnt.getNumRows(), "number of rows");
    CHECK_EQUAL("---AG-T", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL(9, almnt.getLength(), "alignment length");
}

IMPLEMENT_TEST(MAlignmentUnitTests, removeRow_negativeIndex) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.removeRow(-1, os);
    CHECK_EQUAL("Failed to remove a row!", os.getError(), "opStatus");
    CHECK_EQUAL(2, almnt.getNumRows(), "number of rows");
    CHECK_EQUAL("---AG-T", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL("AG-CT-TAA", MAlignmentTestUtils::getRowData(almnt, 1), "second row");
    CHECK_EQUAL(9, almnt.getLength(), "alignment length");
}

IMPLEMENT_TEST(MAlignmentUnitTests, removeRow_tooBigIndex) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.removeRow(2, os);
    CHECK_EQUAL("Failed to remove a row!", os.getError(), "opStatus");
    CHECK_EQUAL(2, almnt.getNumRows(), "number of rows");
    CHECK_EQUAL("---AG-T", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL("AG-CT-TAA", MAlignmentTestUtils::getRowData(almnt, 1), "second row");
    CHECK_EQUAL(9, almnt.getLength(), "alignment length");
}

IMPLEMENT_TEST(MAlignmentUnitTests, removeRow_emptyAlignment) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    U2OpStatusImpl os;
    almnt.removeRow(0, os);
    CHECK_NO_ERROR(os);
    almnt.removeRow(0, os);
    CHECK_NO_ERROR(os);
    CHECK_EQUAL(0, almnt.getNumRows(), "number of rows");
    CHECK_EQUAL(0, almnt.getLength(), "alignment length");
}

/** Tests moveRowsBlock */
IMPLEMENT_TEST(MAlignmentUnitTests, moveRowsBlock_positiveDelta) {
    QByteArray firstSequence("AAAAAA");
    QByteArray secondSequence("CCCCCC");
    QByteArray thirdSequence("GGGGGG");
    QByteArray forthSequence("TTTTTT");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name");
    almnt.addRow("First", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second", secondSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Third", thirdSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Forth", forthSequence, os);
    CHECK_NO_ERROR(os);

    almnt.moveRowsBlock(0, 2, 1);

    CHECK_EQUAL("GGGGGG", MAlignmentTestUtils::getRowData(almnt, 0), "row1");
    CHECK_EQUAL("AAAAAA", MAlignmentTestUtils::getRowData(almnt, 1), "row2");
    CHECK_EQUAL("CCCCCC", MAlignmentTestUtils::getRowData(almnt, 2), "row3");
    CHECK_EQUAL("TTTTTT", MAlignmentTestUtils::getRowData(almnt, 3), "row4");
}

IMPLEMENT_TEST(MAlignmentUnitTests, moveRowsBlock_negativeDelta) {
    QByteArray firstSequence("AAAAAA");
    QByteArray secondSequence("CCCCCC");
    QByteArray thirdSequence("GGGGGG");
    QByteArray forthSequence("TTTTTT");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name");
    almnt.addRow("First", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second", secondSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Third", thirdSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Forth", forthSequence, os);
    CHECK_NO_ERROR(os);

    almnt.moveRowsBlock(3, 1, -1);

    CHECK_EQUAL("AAAAAA", MAlignmentTestUtils::getRowData(almnt, 0), "row1");
    CHECK_EQUAL("CCCCCC", MAlignmentTestUtils::getRowData(almnt, 1), "row2");
    CHECK_EQUAL("TTTTTT", MAlignmentTestUtils::getRowData(almnt, 2), "row3");
    CHECK_EQUAL("GGGGGG", MAlignmentTestUtils::getRowData(almnt, 3), "row4");
}

/** Tests replaceChars */
IMPLEMENT_TEST(MAlignmentUnitTests, replaceChars_validParams) {
    QByteArray firstSequence("AGT.C.T");
    QByteArray secondSequence("A.CT.-AA");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name");
    almnt.addRow("First row", firstSequence, os);
    CHECK_NO_ERROR(os);
    almnt.addRow("Second row", secondSequence, os);
    CHECK_NO_ERROR(os);

    almnt.replaceChars(1, '.', '-');
    CHECK_EQUAL("AGT.C.T", MAlignmentTestUtils::getRowData(almnt, 0), "first sequence");
    CHECK_EQUAL("A-CT--AA", MAlignmentTestUtils::getRowData(almnt, 1), "second sequence");
}

/** Tests appendChars */
IMPLEMENT_TEST(MAlignmentUnitTests, appendChars_validParams) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    const char* str = "-AC-GT-";
    int length = 7;
    almnt.appendChars(0, str, length);
    CHECK_EQUAL("---AG-T-AC-GT-", MAlignmentTestUtils::getRowData(almnt, 0), "first row");
    CHECK_EQUAL(5, almnt.getRow(0).getGapModel().size(), "number of gaps");
    CHECK_EQUAL(14, almnt.getLength(), "alignment length");
}

/** Tests operPlusEqual */
IMPLEMENT_TEST(MAlignmentUnitTests, operPlusEqual_validParams) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    MAlignment almnt2 = MAlignmentTestUtils::initTestAlignment();

    almnt += almnt2;

    CHECK_EQUAL("---AG-T-----AG-T", MAlignmentTestUtils::getRowData(almnt, 0), "row1");
    CHECK_EQUAL("AG-CT-TAAAG-CT-TAA", MAlignmentTestUtils::getRowData(almnt, 1), "row2");

    CHECK_EQUAL(4, almnt.getRow(0).getGapModel().size(), "number of gaps");
    CHECK_EQUAL(18, almnt.getLength(), "alignment length");
}

/** Tests operNotEqual */
IMPLEMENT_TEST(MAlignmentUnitTests, operNotEqual_equal) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    MAlignment almnt2 = MAlignmentTestUtils::initTestAlignment();

    bool res = (almnt != almnt2);
    CHECK_FALSE(res, "Operator!= returned 'True' unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, operNotEqual_notEqual) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    MAlignment almnt2;

    bool res = (almnt != almnt2);
    CHECK_TRUE(res, "Operator!= returned 'False' unexpectedly!");
}

/** Tests hasEmptyGapModel */
IMPLEMENT_TEST(MAlignmentUnitTests, hasEmptyGapModel_gaps) {
    MAlignment almnt = MAlignmentTestUtils::initTestAlignment();
    bool res = almnt.hasEmptyGapModel();

    CHECK_FALSE(res, "Method hasEmptyGapModel() returned 'True' unexpectedly!");
}

IMPLEMENT_TEST(MAlignmentUnitTests, hasEmptyGapModel_noGaps) {
    QByteArray firstSequence("AAAAAA");

    U2OpStatusImpl os;
    MAlignment almnt("Alignment name");
    almnt.addRow("First", firstSequence, os);
    CHECK_NO_ERROR(os);

    bool res = almnt.hasEmptyGapModel();

    CHECK_TRUE(res, "Method hasEmptyGapModel() returned 'False' unexpectedly!");
}


} // namespace
