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

#ifndef _U2_MALIGNMENT_ROW_UNIT_TESTS_H_
#define _U2_MALIGNMENT_ROW_UNIT_TESTS_H_

#include <U2Core/MAlignment.h>

#include <unittest.h>


namespace U2 {

class MAlignmentRowTestUtils {
public:
    static MAlignmentRow initTestRowWithGaps();
    static MAlignmentRow initTestRowWithGapsInMiddle();
    static MAlignmentRow initTestRowWithTrailingGaps();
    static MAlignmentRow initTestRowWithoutGaps();
    static MAlignmentRow initTestRowForModification();
    static MAlignmentRow initEmptyRow();
    static QString getRowData(MAlignmentRow row);

    static const int rowWithGapsLength;
    static const int rowWithGapsInMiddleLength;
    static const int rowWithoutGapsLength;

    static const QString rowWithGapsName;
};

/**
 * The row is created by adding it to an alignment!
 * It is created from a byte array / from a sequence.
 * Row core (start, end, length and bytes) and row length is also verified:
 *   ^ fromBytes          - create a row from a byte array, no trailing gaps
 *   ^ fromBytesTrailing  - create a row from a byte array, there are trailing gaps
 *   ^ fromBytesGaps      - create a row from a byte array, all items are gaps
 *   ^ oneTrailing        - create a row from a byte array, there is only one trailing gap (gap length = 1)
 *   ^ twoTrailing        - create a row from a byte array, there is only one trailing gap (gap length = 2)
 *   ^ oneMiddleGap       - create a row from a byte array, there is only one middle gap
 *   ^ noGaps             - create a row from a byte array, there is no gaps
 *   ^ fromSeq            - create a row from a sequence (without gaps) and a gap model
 *   ^ fromSeqTrailing    - create a row from a sequence (without gaps) and a gap model, there are trailing gaps
 *   ^ fromSeqWithGaps    - create a row from a sequence with gaps (opStatus is set to error)
 *   ^ gapPositionTooBig  - create a row from a sequence, a gap offset is bigger than the core length (opStatus is set to error)
 *   ^ negativeGapPos     - create a row from a sequence, the gap model is incorrect (negative gap position)
 *   ^ negativeGapOffset  - create a row from a sequence, the gap model is incorrect (negative gap offset)
 */
DECLARE_TEST(MAlignmentRowUnitTests, createRow_fromBytes);
DECLARE_TEST(MAlignmentRowUnitTests, createRow_fromBytesTrailing);
DECLARE_TEST(MAlignmentRowUnitTests, createRow_fromBytesGaps);
DECLARE_TEST(MAlignmentRowUnitTests, createRow_oneTrailing);
DECLARE_TEST(MAlignmentRowUnitTests, createRow_twoTrailing);
DECLARE_TEST(MAlignmentRowUnitTests, createRow_oneMiddleGap);
DECLARE_TEST(MAlignmentRowUnitTests, createRow_noGaps);
DECLARE_TEST(MAlignmentRowUnitTests, createRow_fromSeq);
DECLARE_TEST(MAlignmentRowUnitTests, createRow_fromSeqTrailing);
DECLARE_TEST(MAlignmentRowUnitTests, createRow_fromSeqWithGaps);
DECLARE_TEST(MAlignmentRowUnitTests, createRow_gapPositionTooBig);
DECLARE_TEST(MAlignmentRowUnitTests, createRow_negativeGapPos);
DECLARE_TEST(MAlignmentRowUnitTests, createRow_negativeGapOffset);

/**
 * Verify getting/setting of a row name:
 *   ^ rowFromBytes  - when a row has been created from a byte array
 *   ^ rowFromSeq    - when a row has been created from a sequence
 *   ^ setName       - a new name can be set
 */
DECLARE_TEST(MAlignmentRowUnitTests, rowName_rowFromBytes);
DECLARE_TEST(MAlignmentRowUnitTests, rowName_rowFromSeq);
DECLARE_TEST(MAlignmentRowUnitTests, rowName_setName);

/**
 * Verify "toByteArray" method:
 *   ^ noGaps                   - gap model is empty
 *   ^ gapsInBeginningAndMiddle - gaps are in the beginning of the sequence and in the middle
 *   ^ lengthTooShort           - the length is less than the core length (opStatus is set to error)
 *   ^ greaterLength            - the length is greater than the core length (additional gaps are appended to the end)
 *   ^ trailing                 - there are trailing gaps in the row
 */
DECLARE_TEST(MAlignmentRowUnitTests, toByteArray_noGaps);
DECLARE_TEST(MAlignmentRowUnitTests, toByteArray_gapsInBeginningAndMiddle);
DECLARE_TEST(MAlignmentRowUnitTests, toByteArray_incorrectLength);
DECLARE_TEST(MAlignmentRowUnitTests, toByteArray_greaterLength);
DECLARE_TEST(MAlignmentRowUnitTests, toByteArray_trailing);

/**
 * Verify simplifying of a row:
 *   ^ beginningAndMiddleGaps - removes all gaps, returns "true"
 *   ^ nothingToRemove        - there are no gaps, returns "false"
 */
DECLARE_TEST(MAlignmentRowUnitTests, simplify_gaps);
DECLARE_TEST(MAlignmentRowUnitTests, simplify_nothingToRemove);

/**
 * Verify appending of one row to another:
 *   ^ noGapBetweenRows   - lengthBefore exactly equals to the row length
 *   ^ gapBetweenRows     - lengthBefore is greater than the row length
 *   ^ offsetInAnotherRow - gap at the beginning of the appended row
 *   ^ trailingInFirst    - there are trailing gaps in the first row, lengthBefore is greater
 *   ^ trailingAndOffset  - there are trailing gaps in the first row, offset in the appended one
 *   ^ invalidLength      - length before appended row is too short => error
 */
DECLARE_TEST(MAlignmentRowUnitTests, append_noGapBetweenRows);
DECLARE_TEST(MAlignmentRowUnitTests, append_gapBetweenRows);
DECLARE_TEST(MAlignmentRowUnitTests, append_offsetInAnotherRow);
DECLARE_TEST(MAlignmentRowUnitTests, append_trailingInFirst);
DECLARE_TEST(MAlignmentRowUnitTests, append_trailingAndOffset);
DECLARE_TEST(MAlignmentRowUnitTests, append_invalidLength);

/**
 * Setting row content:
 *   ^ empty          - make the row empty
 *   ^ trailingGaps   - bytes contain trailing gaps
 *   ^ offsetNoGap    - offset is specified, the sequence has no gaps at the beginning
 *   ^ offsetGap      - offset is specified, the sequence has gaps at the beginning
 *   ^ emptyAndOffset - empty sequence + (offset > 0)
 */
DECLARE_TEST(MAlignmentRowUnitTests, setRowContent_empty);
DECLARE_TEST(MAlignmentRowUnitTests, setRowContent_trailingGaps);
DECLARE_TEST(MAlignmentRowUnitTests, setRowContent_offsetNoGap);
DECLARE_TEST(MAlignmentRowUnitTests, setRowContent_offsetGap);
DECLARE_TEST(MAlignmentRowUnitTests, setRowContent_emptyAndOffset);

/**
 * Inserting number of gaps into a row:
 *   ^ empty               - row is initially empty
 *   ^ toGapPosLeft        - there is a gap at the left side of the position (and a non-gap char at the right)
 *   ^ toGapPosRight       - there is a gap at the right side of the position
 *   ^ toGapPosInside      - gaps are inserted between gaps
 *   ^ insideChars         - gaps are inserted between chars
 *   ^ toZeroPosNoGap      - insert gaps to the beginning, there is no gap there
 *   ^ toZeroPosGap        - insert gaps to the beginning, there is already a gap offset
 *   ^ toLastPosNoGap      - insert gaps before the last char in the row
 *   ^ toLastPosGap        - insert a gap before the last gap in the row (between gaps)
 *   ^ toLastPosOneGap     - insert a gap before the last and the only gap in the row
 *   ^ noGapsYet           - insert to a row without gaps
 *   ^ onlyGaps            - the row consists of gaps only
 *   ^ oneChar             - the row consists of one char, insert offset to the beginning
 *   ^ tooBigPosition      - position is greater than the row length => skip
 *   ^ negativePosition    - negative position => skip
 *   ^ negativeNumOfChars  - negative chars count => error
 */
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_empty);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_toGapPosLeft);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_toGapPosRight);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_toGapPosInside);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_insideChars);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_toZeroPosNoGap);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_toZeroPosGap);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_toLastPosNoGap);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_toLastPosGap);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_toLastPosOneGap);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_noGapsYet);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_onlyGaps);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_oneChar);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_tooBigPosition);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_negativePosition);
DECLARE_TEST(MAlignmentRowUnitTests, insertGaps_negativeNumOfChars);

/**
 * Removing chars from a row:
 *   ^ empty               - row is initially empty => skip
 *   ^ insideGap1          - start and end positions are inside gaps areas
 *   ^ insideGap2          - the same as above, but shifted
 *   ^ leftGapSide         - 'pos' is a gap, there is a char at ('pos' - 1)
 *   ^ rightGapSide        - 'pos' is a char, there is a gap at ('pos' - 1)
 *   ^ insideSeq1          - start and end positions are inside chars areas
 *   ^ insideSeq2          - the same as above, but shifted
 *   ^ fromZeroPosGap      - 'pos' = 0, there is a gap
 *   ^ fromZeroPosChar     - 'pos' = 0, there is a char
 *   ^ lastPosExactly      - 'pos' is the last char in the row, 'count' = 1, no gaps at the cut end
 *   ^ fromLastPos         - 'pos' is the last char in the row, 'count' > 1, no gaps at the cut end
 *   ^ insideOneGap1       - a region inside a long gap is removed (middle and end gaps in "---")
 *   ^ insideOneGap2       - a gap inside a long gap is removed (middle in "---")
 *   ^ insideOneGapLong    - several gaps inside longer gaps region
 *   ^ insideTrailingGap   - remove gap chars inside a long trailing gap
 *   ^ insideCharsOne      - one char inside non-gap chars region
 *   ^ negativePosition    - negative 'pos' has been specified => error
 *   ^ negativeNumOfChars  - negative 'count' has been specified => error
 *   ^ gapsAtRowEnd1       - trailing gaps are not removed ('pos' + 'count' bigger than the row length is also verified)
 *   ^ gapsAtRowEnd2       - the same as above, but with the only gap at the end
 *   ^ onlyGapsAfterRemove - all non-gap chars are removed
 *   ^ emptyAfterRemove    - all chars and gaps are removed
 *   ^ oneCharInGaps       - remove a char with gaps at the left and right side
 */
DECLARE_TEST(MAlignmentRowUnitTests, remove_empty);
DECLARE_TEST(MAlignmentRowUnitTests, remove_insideGap1);
DECLARE_TEST(MAlignmentRowUnitTests, remove_insideGap2);
DECLARE_TEST(MAlignmentRowUnitTests, remove_leftGapSide);
DECLARE_TEST(MAlignmentRowUnitTests, remove_rightGapSide);
DECLARE_TEST(MAlignmentRowUnitTests, remove_insideSeq1);
DECLARE_TEST(MAlignmentRowUnitTests, remove_insideSeq2);
DECLARE_TEST(MAlignmentRowUnitTests, remove_fromZeroPosGap);
DECLARE_TEST(MAlignmentRowUnitTests, remove_fromZeroPosChar);
DECLARE_TEST(MAlignmentRowUnitTests, remove_lastPosExactly);
DECLARE_TEST(MAlignmentRowUnitTests, remove_fromLastPos);
DECLARE_TEST(MAlignmentRowUnitTests, remove_insideOneGap1);
DECLARE_TEST(MAlignmentRowUnitTests, remove_insideOneGap2);
DECLARE_TEST(MAlignmentRowUnitTests, remove_insideOneGapLong);
DECLARE_TEST(MAlignmentRowUnitTests, remove_insideTrailingGap);
DECLARE_TEST(MAlignmentRowUnitTests, remove_insideCharsOne);
DECLARE_TEST(MAlignmentRowUnitTests, remove_toBiggerPosition);
DECLARE_TEST(MAlignmentRowUnitTests, remove_negativePosition);
DECLARE_TEST(MAlignmentRowUnitTests, remove_negativeNumOfChars);
DECLARE_TEST(MAlignmentRowUnitTests, remove_gapsAtRowEnd1);
DECLARE_TEST(MAlignmentRowUnitTests, remove_gapsAtRowEnd2);
DECLARE_TEST(MAlignmentRowUnitTests, remove_onlyGapsAfterRemove);
DECLARE_TEST(MAlignmentRowUnitTests, remove_emptyAfterRemove);
DECLARE_TEST(MAlignmentRowUnitTests, remove_oneCharInGaps);

/**
 * Getting a char at the specified position:
 *   ^ allCharsNoOffset  - verify all indexes of a row without gap offset in the beginning
 *   ^ offsetAndTrailing - verify gaps at the beginning and end of a row
 *   ^ onlyCharsInRow    - there are no gaps in the row
 */
DECLARE_TEST(MAlignmentRowUnitTests, charAt_allCharsNoOffset);
DECLARE_TEST(MAlignmentRowUnitTests, charAt_offsetAndTrailing);
DECLARE_TEST(MAlignmentRowUnitTests, charAt_onlyCharsInRow);

/**
 * Checking if rows are equal (method "isRowContentEqual", "operator==", "operator!="):
 *   ^ sameContent         - rows contents are equal
 *   ^ noGaps              - rows contents are equal, there are no gaps in the rows
 *   ^ trailingInFirst     - rows contents are equal except there is a trailing gap in the first row
 *   ^ trailingInSecond    - rows contents are equal except there is a trailing gap in the second row
 *   ^ trailingInBoth      - rows contents are equal except trailing gaps, i.e. both rows have trailing gaps and sizes of the gaps are different
 *   ^ diffGapModelsGap    - gaps models are different (lengths of gaps are different)
 *   ^ diffGapModelsOffset - gaps models are different (offsets of gaps are different)
 *   ^ diffNumOfGaps       - gaps models are different (number of gaps differs)
 *   ^ diffSequences       - sequences differ
 */
DECLARE_TEST(MAlignmentRowUnitTests, rowsEqual_sameContent);
DECLARE_TEST(MAlignmentRowUnitTests, rowsEqual_noGaps);
DECLARE_TEST(MAlignmentRowUnitTests, rowsEqual_trailingInFirst);
DECLARE_TEST(MAlignmentRowUnitTests, rowsEqual_trailingInSecond);
DECLARE_TEST(MAlignmentRowUnitTests, rowsEqual_trailingInBoth);
DECLARE_TEST(MAlignmentRowUnitTests, rowsEqual_diffGapModelsGap);
DECLARE_TEST(MAlignmentRowUnitTests, rowsEqual_diffGapModelsOffset);
DECLARE_TEST(MAlignmentRowUnitTests, rowsEqual_diffNumOfGaps);
DECLARE_TEST(MAlignmentRowUnitTests, rowsEqual_diffSequences);

/**
 * Verify ungapped sequence length and getting of an ungapped position:
 *   ^ rowWithoutOffset  - verify the length and position for a row without gaps at the beginning
 *   ^ offsetTrailing    - verify the length and position for a row with gaps at the beginning
 */
DECLARE_TEST(MAlignmentRowUnitTests, ungapped_rowWithoutOffset);
DECLARE_TEST(MAlignmentRowUnitTests, ungapped_offsetTrailing);

/**
 * Cropping a row:
 *   ^ empty               - row is initially empty => skip
 *   ^ insideGap1          - start and end positions are inside gaps areas
 *   ^ insideGap2          - the same as above, but shifted
 *   ^ leftGapSide         - 'pos' is a gap, there is a char at ('pos' - 1)
 *   ^ rightGapSide        - 'pos' is a char, there is a gap at ('pos' - 1)
 *   ^ insideSeq1          - start and end positions are inside chars areas
 *   ^ insideSeq2          - the same as above, but shifted
 *   ^ fromZeroPosGap      - 'pos' = 0, there is a gap
 *   ^ fromZeroPosChar     - 'pos' = 0, there is a char
 *   ^ lastPosExactly      - 'pos' is the last char in the row, 'count' = 1, no gaps at the cut end
 *   ^ fromLastPos         - 'pos' is the last char in the row, 'count' > 1, no gaps at the cut end
 *   ^ insideOneGap1       - a region inside a long gap is removed (middle and end gaps in "---")
 *   ^ insideOneGap2       - a gap inside a long gap (middle in "---")
 *   ^ insideOneGapLong    - several gaps inside longer gaps region
 *   ^ insideCharsOne      - one char inside non-gap chars region
 *   ^ negativePosition    - negative 'pos' has been specified => error
 *   ^ negativeNumOfChars  - negative 'count' has been specified => error
 *   ^ trailing            - there are trailing gaps in the row
 *   ^ trailingToGaps      - there are trailing gaps in the row, the row is cropped to gaps only
 *   ^ cropTrailing        - trailing gaps are cropped
 *   ^ oneCharInGaps       - remove a char with gaps at the left and right side
 *   ^ posMoreThanLength   - the specified position is greater than the row length => make row empty
 */
DECLARE_TEST(MAlignmentRowUnitTests, crop_empty);
DECLARE_TEST(MAlignmentRowUnitTests, crop_insideGap1);
DECLARE_TEST(MAlignmentRowUnitTests, crop_insideGap2);
DECLARE_TEST(MAlignmentRowUnitTests, crop_leftGapSide);
DECLARE_TEST(MAlignmentRowUnitTests, crop_rightGapSide);
DECLARE_TEST(MAlignmentRowUnitTests, crop_insideSeq1);
DECLARE_TEST(MAlignmentRowUnitTests, crop_insideSeq2);
DECLARE_TEST(MAlignmentRowUnitTests, crop_fromZeroPosGap);
DECLARE_TEST(MAlignmentRowUnitTests, crop_fromZeroPosChar);
DECLARE_TEST(MAlignmentRowUnitTests, crop_lastPosExactly);
DECLARE_TEST(MAlignmentRowUnitTests, crop_fromLastPos);
DECLARE_TEST(MAlignmentRowUnitTests, crop_insideOneGap1);
DECLARE_TEST(MAlignmentRowUnitTests, crop_insideOneGap2);
DECLARE_TEST(MAlignmentRowUnitTests, crop_insideOneGapLong);
DECLARE_TEST(MAlignmentRowUnitTests, crop_insideCharsOne);
DECLARE_TEST(MAlignmentRowUnitTests, crop_negativePosition);
DECLARE_TEST(MAlignmentRowUnitTests, crop_negativeNumOfChars);
DECLARE_TEST(MAlignmentRowUnitTests, crop_trailing);
DECLARE_TEST(MAlignmentRowUnitTests, crop_trailingToGaps);
DECLARE_TEST(MAlignmentRowUnitTests, crop_cropTrailing);
DECLARE_TEST(MAlignmentRowUnitTests, crop_oneCharInGaps);
DECLARE_TEST(MAlignmentRowUnitTests, crop_posMoreThanLength);

/**
 * Getting mid of a row - only one case is verified as
 * mid uses "crop" method.
 */
DECLARE_TEST(MAlignmentRowUnitTests, mid_general);

/** Converting to upper case. It is also verified that the name of the row is not changed. */
DECLARE_TEST(MAlignmentRowUnitTests, upperCase_general);

/**
 * Replacing chars in a row:
 *   ^ charToChar           - all 'A' in a row are replaced by 'G'.
 *   ^ nothingToReplace     - no 'origChar' in a row to replace by a gap.
 *   ^ tildasToGapsNoGaps   - all 'origChar' ('~') are replaced by gaps.
 *   ^ tildasToGapsWithGaps - the row contains both gaps and 'origChar' ('~'), replaced by gaps.
                              Shifted gaps offset and merging of gaps is also verified.
 *   ^ trailingGaps         - trailing gaps are not removed.
 */
DECLARE_TEST(MAlignmentRowUnitTests, replaceChars_charToChar);
DECLARE_TEST(MAlignmentRowUnitTests, replaceChars_nothingToReplace);
DECLARE_TEST(MAlignmentRowUnitTests, replaceChars_tildasToGapsNoGaps);
DECLARE_TEST(MAlignmentRowUnitTests, replaceChars_tildasToGapsWithGaps);
DECLARE_TEST(MAlignmentRowUnitTests, replaceChars_trailingGaps);


} // namespace

DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_fromBytes)
DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_fromBytesTrailing)
DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_fromBytesGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_oneTrailing)
DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_twoTrailing)
DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_oneMiddleGap)
DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_noGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_fromSeq)
DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_fromSeqTrailing)
DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_fromSeqWithGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_gapPositionTooBig)
DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_negativeGapPos)
DECLARE_METATYPE(MAlignmentRowUnitTests, createRow_negativeGapOffset)
DECLARE_METATYPE(MAlignmentRowUnitTests, rowName_rowFromBytes)
DECLARE_METATYPE(MAlignmentRowUnitTests, rowName_rowFromSeq)
DECLARE_METATYPE(MAlignmentRowUnitTests, rowName_setName)
DECLARE_METATYPE(MAlignmentRowUnitTests, toByteArray_noGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, toByteArray_gapsInBeginningAndMiddle)
DECLARE_METATYPE(MAlignmentRowUnitTests, toByteArray_incorrectLength)
DECLARE_METATYPE(MAlignmentRowUnitTests, toByteArray_greaterLength)
DECLARE_METATYPE(MAlignmentRowUnitTests, toByteArray_trailing)
DECLARE_METATYPE(MAlignmentRowUnitTests, simplify_gaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, simplify_nothingToRemove)
DECLARE_METATYPE(MAlignmentRowUnitTests, append_noGapBetweenRows)
DECLARE_METATYPE(MAlignmentRowUnitTests, append_gapBetweenRows)
DECLARE_METATYPE(MAlignmentRowUnitTests, append_offsetInAnotherRow)
DECLARE_METATYPE(MAlignmentRowUnitTests, append_trailingInFirst)
DECLARE_METATYPE(MAlignmentRowUnitTests, append_trailingAndOffset)
DECLARE_METATYPE(MAlignmentRowUnitTests, append_invalidLength)
DECLARE_METATYPE(MAlignmentRowUnitTests, setRowContent_empty)
DECLARE_METATYPE(MAlignmentRowUnitTests, setRowContent_trailingGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, setRowContent_offsetNoGap)
DECLARE_METATYPE(MAlignmentRowUnitTests, setRowContent_offsetGap)
DECLARE_METATYPE(MAlignmentRowUnitTests, setRowContent_emptyAndOffset)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_empty)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_toGapPosLeft)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_toGapPosRight)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_toGapPosInside)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_insideChars)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_toZeroPosNoGap)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_toZeroPosGap)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_toLastPosNoGap)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_toLastPosGap)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_toLastPosOneGap)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_noGapsYet)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_onlyGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_oneChar)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_tooBigPosition)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_negativePosition)
DECLARE_METATYPE(MAlignmentRowUnitTests, insertGaps_negativeNumOfChars)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_empty)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_insideGap1)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_insideGap2)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_leftGapSide)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_rightGapSide)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_insideSeq1)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_insideSeq2)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_fromZeroPosGap)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_fromZeroPosChar)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_lastPosExactly)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_fromLastPos)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_insideOneGap1)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_insideOneGap2)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_insideOneGapLong)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_insideTrailingGap)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_insideCharsOne)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_negativePosition)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_negativeNumOfChars)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_gapsAtRowEnd1)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_gapsAtRowEnd2)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_onlyGapsAfterRemove)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_emptyAfterRemove)
DECLARE_METATYPE(MAlignmentRowUnitTests, remove_oneCharInGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, charAt_allCharsNoOffset)
DECLARE_METATYPE(MAlignmentRowUnitTests, charAt_offsetAndTrailing)
DECLARE_METATYPE(MAlignmentRowUnitTests, charAt_onlyCharsInRow)
DECLARE_METATYPE(MAlignmentRowUnitTests, rowsEqual_sameContent)
DECLARE_METATYPE(MAlignmentRowUnitTests, rowsEqual_noGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, rowsEqual_trailingInFirst)
DECLARE_METATYPE(MAlignmentRowUnitTests, rowsEqual_trailingInSecond)
DECLARE_METATYPE(MAlignmentRowUnitTests, rowsEqual_trailingInBoth)
DECLARE_METATYPE(MAlignmentRowUnitTests, rowsEqual_diffGapModelsGap)
DECLARE_METATYPE(MAlignmentRowUnitTests, rowsEqual_diffGapModelsOffset)
DECLARE_METATYPE(MAlignmentRowUnitTests, rowsEqual_diffNumOfGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, rowsEqual_diffSequences)
DECLARE_METATYPE(MAlignmentRowUnitTests, ungapped_rowWithoutOffset)
DECLARE_METATYPE(MAlignmentRowUnitTests, ungapped_offsetTrailing)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_empty)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_insideGap1)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_insideGap2)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_leftGapSide)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_rightGapSide)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_insideSeq1)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_insideSeq2)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_fromZeroPosGap)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_fromZeroPosChar)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_lastPosExactly)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_fromLastPos)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_insideOneGap1)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_insideOneGap2)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_insideOneGapLong)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_insideCharsOne)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_negativePosition)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_negativeNumOfChars)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_trailing)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_trailingToGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_cropTrailing)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_oneCharInGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, crop_posMoreThanLength)
DECLARE_METATYPE(MAlignmentRowUnitTests, mid_general)
DECLARE_METATYPE(MAlignmentRowUnitTests, upperCase_general)
DECLARE_METATYPE(MAlignmentRowUnitTests, replaceChars_charToChar)
DECLARE_METATYPE(MAlignmentRowUnitTests, replaceChars_nothingToReplace)
DECLARE_METATYPE(MAlignmentRowUnitTests, replaceChars_tildasToGapsNoGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, replaceChars_tildasToGapsWithGaps)
DECLARE_METATYPE(MAlignmentRowUnitTests, replaceChars_trailingGaps)

#endif
