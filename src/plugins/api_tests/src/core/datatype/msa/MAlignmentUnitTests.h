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


#ifndef _U2_MALIGNMENT_UNIT_TESTS_H_
#define _U2_MALIGNMENT_UNIT_TESTS_H_

#include <U2Core/MAlignment.h>

#include <unittest.h>


namespace U2 {

class MAlignmentTestUtils {
public:
    static MAlignment initTestAlignment();
    static QString getRowData(const MAlignment&, int rowNum);
    static bool testAlignmentNotChanged(const MAlignment&);

    static const int rowsNum;
    static const int firstRowLength;
    static const int secondRowLength;

    static const QString alignmentName;
};

/** Clearing of a non-empty alignment */
DECLARE_TEST(MAlignmentUnitTests, clear_notEmpty);

/**
 * Alignment name:
 *   ^ ctor     - name, specified in the constructor
 *   ^ setName  - set a new name
 */
DECLARE_TEST(MAlignmentUnitTests, name_ctor);
DECLARE_TEST(MAlignmentUnitTests, name_setName);

/**
 * Alignment alphabet:
 *   ^ ctor        - alphabet, specified in the constructor
 *   ^ setAlphabet - set a new alphabet
 */
DECLARE_TEST(MAlignmentUnitTests, alphabet_ctor);
DECLARE_TEST(MAlignmentUnitTests, alphabet_setAlphabet);

/** Alignment info */
DECLARE_TEST(MAlignmentUnitTests, info_setGet);

/**
 * Alignment length:
 *   ^ isEmptyFalse   - method "isEmpty" returns "false" for a non-empty alignment
 *   ^ isEmptyTrue    - method "isEmpty" returns "true" for an empty alignment
 *   ^ get            - getting length of a non-empty alignment
 *   ^ getForEmpty    - getting length of an empty alignment
 *   ^ setLessLength  - set length less than the current one to a non-empty 
 *                      alignment, the alignment is cropped
 */
DECLARE_TEST(MAlignmentUnitTests, length_isEmptyFalse);
DECLARE_TEST(MAlignmentUnitTests, length_isEmptyTrue);
DECLARE_TEST(MAlignmentUnitTests, length_get);
DECLARE_TEST(MAlignmentUnitTests, length_getForEmpty);
DECLARE_TEST(MAlignmentUnitTests, length_setLessLength);

/**
 * Number of rows:
 *   ^ notEmpty - number of rows in a non-empty alignment
 *   ^ empty    - zero rows in an empty alignment
 */
DECLARE_TEST(MAlignmentUnitTests, numOfRows_notEmpty);
DECLARE_TEST(MAlignmentUnitTests, numOfRows_empty);

/**
 * Trimming an alignment:
 *   ^ biggerLength      - length bigger than a maximum row length was set, now it is trimmed
 *   ^ leadingGapColumns - leading gap columns are removed
 *   ^ nothingToTrim     - trim() returns "false"
 *   ^ rowWithoutGaps    - no errors when an alignment contains a row without gaps
 *   ^ empty             - trim() returns "false"
 *   ^ trailingGapInOne  - there are two rows and nothing to trim, one row has a trailing gap
 */
DECLARE_TEST(MAlignmentUnitTests, trim_biggerLength);
DECLARE_TEST(MAlignmentUnitTests, trim_leadingGapColumns);
DECLARE_TEST(MAlignmentUnitTests, trim_nothingToTrim);
DECLARE_TEST(MAlignmentUnitTests, trim_rowWithoutGaps);
DECLARE_TEST(MAlignmentUnitTests, trim_empty);
DECLARE_TEST(MAlignmentUnitTests, trim_trailingGapInOne);

/**
 * Removing all gaps from an alignment:
 *   ^ withGaps    - gaps in a non-empty alignment are removed, simplify returns "true"
 *   ^ withoutGaps - no gaps to remove, simplify returns "false"
 *   ^ empty       - an empty alignment, simplify returns "false"
 */
DECLARE_TEST(MAlignmentUnitTests, simplify_withGaps);
DECLARE_TEST(MAlignmentUnitTests, simplify_withoutGaps);
DECLARE_TEST(MAlignmentUnitTests, simplify_empty);

/**
 * Verify methods "sortRowsByName" and "sortRowsBySimilarity":
 *   ^ byNameAsc         - sort rows by name in ascending order
 *   ^ byNameDesc        - sort rows by name in descending order
 *   ^ twoSimilar        - sort rows by similarity, two rows are similar
 *   ^ threeSimilar      - sort rows by similarity, three rows are similar
 *   ^ similarTwoRegions - sort rows by similarity, two groups of similar sequences
 */
DECLARE_TEST(MAlignmentUnitTests, sortRows_byNameAsc);
DECLARE_TEST(MAlignmentUnitTests, sortRows_byNameDesc);
DECLARE_TEST(MAlignmentUnitTests, sortRows_twoSimilar);
DECLARE_TEST(MAlignmentUnitTests, sortRows_threeSimilar);
DECLARE_TEST(MAlignmentUnitTests, sortRows_similarTwoRegions);

/**
 * Getting rows and rows' names:
 *   ^ oneRow              - verify "getRow" method on a non-empty alignment with a valid row index
 *   ^ severalRows         - verify "getRows" method on a non-empty alignment
 *   ^ rowNames            - verify "getRowNames" method on a non-empty alignment
 */
DECLARE_TEST(MAlignmentUnitTests, getRows_oneRow);
DECLARE_TEST(MAlignmentUnitTests, getRows_severalRows);
DECLARE_TEST(MAlignmentUnitTests, getRows_rowNames);

/**
 * Getting character at the specified position:
 *   ^ nonGapChar - there is a non-gap char in the specified row/position
 *   ^ gap        - there is a gap in the specified row/position
 */
DECLARE_TEST(MAlignmentUnitTests, charAt_nonGapChar);
DECLARE_TEST(MAlignmentUnitTests, charAt_gap);

/**
 * Inserting gaps into an alignment:
 *   ^ validParams       - gaps are inserted into a row
 *   ^ toBeginningLength - a gap is inserted to a row beginning, the length of the alignment is properly recalculated
 *   ^ negativeRowIndex  - row index is negative => error
 *   ^ tooBigRowIndex    - row index is greater than the number of rows => error
 *   ^ negativePos       - position is negative => error
 *   ^ tooBigPos         - position is greater than the alignment length => error
 *   ^ negativeCount     - gaps count is negative => error
 */
DECLARE_TEST(MAlignmentUnitTests, insertGaps_validParams);
DECLARE_TEST(MAlignmentUnitTests, insertGaps_toBeginningLength);
DECLARE_TEST(MAlignmentUnitTests, insertGaps_negativeRowIndex);
DECLARE_TEST(MAlignmentUnitTests, insertGaps_tooBigRowIndex);
DECLARE_TEST(MAlignmentUnitTests, insertGaps_negativePos);
DECLARE_TEST(MAlignmentUnitTests, insertGaps_tooBigPos);
DECLARE_TEST(MAlignmentUnitTests, insertGaps_negativeCount);

/**
 * Removing chars from an alignment:
 *   ^ validParamsAndTrimmed - chars are removed, the alignment is trimmed (gaps columns are removed, length is recalculated)
 *   ^ negativeRowIndex      - row index is negative => error
 *   ^ tooBigRowIndex        - row index is greater than the number of rows => error
 *   ^ negativePos           - position is negative => error
 *   ^ tooBigPos             - position is greater than the alignment length => error
 *   ^ negativeCount         - gaps count is negative => error
 */
DECLARE_TEST(MAlignmentUnitTests, removeChars_validParamsAndTrimmed);
DECLARE_TEST(MAlignmentUnitTests, removeChars_negativeRowIndex);
DECLARE_TEST(MAlignmentUnitTests, removeChars_tooBigRowIndex);
DECLARE_TEST(MAlignmentUnitTests, removeChars_negativePos);
DECLARE_TEST(MAlignmentUnitTests, removeChars_tooBigPos);
DECLARE_TEST(MAlignmentUnitTests, removeChars_negativeCount);

/**
 * Removing a region:
 *   ^ validParams - remove a sub-alignment
 *   ^ removeEmpty - parameter removeEmptyRows is set to "True"
 *   ^ trimmed     - the alignment is trimmed after removing the region, the length has been modified
 */
DECLARE_TEST(MAlignmentUnitTests, removeRegion_validParams);
DECLARE_TEST(MAlignmentUnitTests, removeRegion_removeEmpty);
DECLARE_TEST(MAlignmentUnitTests, removeRegion_trimmed);

/** Renaming a row */
DECLARE_TEST(MAlignmentUnitTests, renameRow_validParams);

/**
 * Setting a new row content:
 *   ^ validParamsAndTrimmed - a row content is changed, the alignment is NOT trimmed
 *   ^ lengthIsIncreased     - a row content becomes longer than the initial alignment length
 */
DECLARE_TEST(MAlignmentUnitTests, setRowContent_validParamsAndNotTrimmed);
DECLARE_TEST(MAlignmentUnitTests, setRowContent_lengthIsIncreased);

/** Converting all rows to upper case */
DECLARE_TEST(MAlignmentUnitTests, upperCase_charsAndGaps);

/** Cropping an alignment */
DECLARE_TEST(MAlignmentUnitTests, crop_validParams);

/** Getting mid of an alignmentVerify method "mid" */
DECLARE_TEST(MAlignmentUnitTests, mid_validParams);

/**
 * Adding a new row to the alignment:
 *   ^ appendRowFromBytes  - a new row is created from bytes and appended to the end of the alignment
 *   ^ rowFromBytesToIndex - a new row is created from bytes and inserted to the specified index
 *   ^ zeroBound           - incorrect row index "-2" => the new row is inserted to the beginning
 *   ^ rowsNumBound        - incorrect row index more than the number of rows => the row is appended
 */
DECLARE_TEST(MAlignmentUnitTests, addRow_appendRowFromBytes);
DECLARE_TEST(MAlignmentUnitTests, addRow_rowFromBytesToIndex);
DECLARE_TEST(MAlignmentUnitTests, addRow_zeroBound);
DECLARE_TEST(MAlignmentUnitTests, addRow_rowsNumBound);

/**
 * Removing a row from the alignment:
 *   ^ validIndex     - row index is valid => the row is removed
 *   ^ negativeIndex  - row index is negative => error
 *   ^ tooBigIndex    - row index is bigger than the number of rows => error
 *   ^ emptyAlignment - all rows are removed from the alignment, the length is set to zero
 */
DECLARE_TEST(MAlignmentUnitTests, removeRow_validIndex);
DECLARE_TEST(MAlignmentUnitTests, removeRow_negativeIndex);
DECLARE_TEST(MAlignmentUnitTests, removeRow_tooBigIndex);
DECLARE_TEST(MAlignmentUnitTests, removeRow_emptyAlignment);

/**
 * Moving rows block:
 *   ^ positiveDelta - rows are moved downwards
 *   ^ negativeDelta - rows are moved upwards
 */
DECLARE_TEST(MAlignmentUnitTests, moveRowsBlock_positiveDelta);
DECLARE_TEST(MAlignmentUnitTests, moveRowsBlock_negativeDelta);

/** Replacing chars in an alignment row */
DECLARE_TEST(MAlignmentUnitTests, replaceChars_validParams);

/** Appending chars to an alignment row */
DECLARE_TEST(MAlignmentUnitTests, appendChars_validParams);

/** Verify operator+= */
DECLARE_TEST(MAlignmentUnitTests, operPlusEqual_validParams);

/**
 * Verify operator!= :
 *   ^ equal    - alignments are equal
 *   ^ notEqual - alignments are not equal (one of the alignments is empty)
 */
DECLARE_TEST(MAlignmentUnitTests, operNotEqual_equal);
DECLARE_TEST(MAlignmentUnitTests, operNotEqual_notEqual);

/**
 * Verify if the alignment has gaps:
 *   ^ gaps   - there are gaps in the alignment
 *   ^ noGaps - there are NO gaps in the alignment
 */
DECLARE_TEST(MAlignmentUnitTests, hasGaps_gaps);
DECLARE_TEST(MAlignmentUnitTests, hasGaps_noGaps);


} // namespace

DECLARE_METATYPE(MAlignmentUnitTests, clear_notEmpty);
DECLARE_METATYPE(MAlignmentUnitTests, name_ctor);
DECLARE_METATYPE(MAlignmentUnitTests, name_setName);
DECLARE_METATYPE(MAlignmentUnitTests, alphabet_ctor);
DECLARE_METATYPE(MAlignmentUnitTests, alphabet_setAlphabet);
DECLARE_METATYPE(MAlignmentUnitTests, info_setGet);
DECLARE_METATYPE(MAlignmentUnitTests, length_isEmptyFalse);
DECLARE_METATYPE(MAlignmentUnitTests, length_isEmptyTrue);
DECLARE_METATYPE(MAlignmentUnitTests, length_get);
DECLARE_METATYPE(MAlignmentUnitTests, length_getForEmpty);
DECLARE_METATYPE(MAlignmentUnitTests, length_setLessLength);
DECLARE_METATYPE(MAlignmentUnitTests, numOfRows_notEmpty);
DECLARE_METATYPE(MAlignmentUnitTests, numOfRows_empty);
DECLARE_METATYPE(MAlignmentUnitTests, trim_biggerLength);
DECLARE_METATYPE(MAlignmentUnitTests, trim_leadingGapColumns);
DECLARE_METATYPE(MAlignmentUnitTests, trim_nothingToTrim);
DECLARE_METATYPE(MAlignmentUnitTests, trim_rowWithoutGaps);
DECLARE_METATYPE(MAlignmentUnitTests, trim_empty);
DECLARE_METATYPE(MAlignmentUnitTests, trim_trailingGapInOne);
DECLARE_METATYPE(MAlignmentUnitTests, simplify_withGaps);
DECLARE_METATYPE(MAlignmentUnitTests, simplify_withoutGaps);
DECLARE_METATYPE(MAlignmentUnitTests, simplify_empty);
DECLARE_METATYPE(MAlignmentUnitTests, sortRows_byNameAsc);
DECLARE_METATYPE(MAlignmentUnitTests, sortRows_byNameDesc);
DECLARE_METATYPE(MAlignmentUnitTests, sortRows_twoSimilar);
DECLARE_METATYPE(MAlignmentUnitTests, sortRows_threeSimilar);
DECLARE_METATYPE(MAlignmentUnitTests, sortRows_similarTwoRegions);
DECLARE_METATYPE(MAlignmentUnitTests, getRows_oneRow);
DECLARE_METATYPE(MAlignmentUnitTests, getRows_severalRows);
DECLARE_METATYPE(MAlignmentUnitTests, getRows_rowNames);
DECLARE_METATYPE(MAlignmentUnitTests, charAt_nonGapChar);
DECLARE_METATYPE(MAlignmentUnitTests, charAt_gap);
DECLARE_METATYPE(MAlignmentUnitTests, insertGaps_validParams);
DECLARE_METATYPE(MAlignmentUnitTests, insertGaps_toBeginningLength);
DECLARE_METATYPE(MAlignmentUnitTests, insertGaps_negativeRowIndex);
DECLARE_METATYPE(MAlignmentUnitTests, insertGaps_tooBigRowIndex);
DECLARE_METATYPE(MAlignmentUnitTests, insertGaps_negativePos);
DECLARE_METATYPE(MAlignmentUnitTests, insertGaps_tooBigPos);
DECLARE_METATYPE(MAlignmentUnitTests, insertGaps_negativeCount);
DECLARE_METATYPE(MAlignmentUnitTests, removeChars_validParamsAndTrimmed);
DECLARE_METATYPE(MAlignmentUnitTests, removeChars_negativeRowIndex);
DECLARE_METATYPE(MAlignmentUnitTests, removeChars_tooBigRowIndex);
DECLARE_METATYPE(MAlignmentUnitTests, removeChars_negativePos);
DECLARE_METATYPE(MAlignmentUnitTests, removeChars_tooBigPos);
DECLARE_METATYPE(MAlignmentUnitTests, removeChars_negativeCount);
DECLARE_METATYPE(MAlignmentUnitTests, removeRegion_validParams);
DECLARE_METATYPE(MAlignmentUnitTests, removeRegion_removeEmpty);
DECLARE_METATYPE(MAlignmentUnitTests, removeRegion_trimmed);
DECLARE_METATYPE(MAlignmentUnitTests, renameRow_validParams);
DECLARE_METATYPE(MAlignmentUnitTests, setRowContent_validParamsAndNotTrimmed);
DECLARE_METATYPE(MAlignmentUnitTests, setRowContent_lengthIsIncreased);
DECLARE_METATYPE(MAlignmentUnitTests, upperCase_charsAndGaps)
DECLARE_METATYPE(MAlignmentUnitTests, crop_validParams);
DECLARE_METATYPE(MAlignmentUnitTests, mid_validParams);
DECLARE_METATYPE(MAlignmentUnitTests, addRow_appendRowFromBytes);
DECLARE_METATYPE(MAlignmentUnitTests, addRow_rowFromBytesToIndex);
DECLARE_METATYPE(MAlignmentUnitTests, addRow_zeroBound);
DECLARE_METATYPE(MAlignmentUnitTests, addRow_rowsNumBound);
DECLARE_METATYPE(MAlignmentUnitTests, removeRow_validIndex);
DECLARE_METATYPE(MAlignmentUnitTests, removeRow_negativeIndex);
DECLARE_METATYPE(MAlignmentUnitTests, removeRow_tooBigIndex);
DECLARE_METATYPE(MAlignmentUnitTests, removeRow_emptyAlignment);
DECLARE_METATYPE(MAlignmentUnitTests, moveRowsBlock_positiveDelta);
DECLARE_METATYPE(MAlignmentUnitTests, moveRowsBlock_negativeDelta);
DECLARE_METATYPE(MAlignmentUnitTests, replaceChars_validParams);
DECLARE_METATYPE(MAlignmentUnitTests, appendChars_validParams);
DECLARE_METATYPE(MAlignmentUnitTests, operPlusEqual_validParams);
DECLARE_METATYPE(MAlignmentUnitTests, operNotEqual_equal);
DECLARE_METATYPE(MAlignmentUnitTests, operNotEqual_notEqual);
DECLARE_METATYPE(MAlignmentUnitTests, hasGaps_gaps);
DECLARE_METATYPE(MAlignmentUnitTests, hasGaps_noGaps);


#endif

