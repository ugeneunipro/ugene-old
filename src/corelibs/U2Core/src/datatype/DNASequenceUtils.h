/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_DNA_SEQUENCE_UTILS_H_
#define _U2_DNA_SEQUENCE_UTILS_H_

#include <U2Core/DNASequence.h>
#include <U2Core/U2OpStatus.h>


namespace U2 {

enum DnaSequencesMatchStatus {MatchExactly, DoNotMatch};


/** Utilities for DNASequences */
class U2CORE_EXPORT DNASequenceUtils {
public:
    /** Appends "appendedSequence" to "sequence" */
    static void append(DNASequence& sequence, const DNASequence& appendedSequence, U2OpStatus& os);

    /** Compares two sequences */
    static DnaSequencesMatchStatus compare(const DNASequence& firstSeq, const DNASequence& secondSec);

    /** Removes chars from 'startPos' (inclusive) to 'endPos' (non-inclusive) */
    static void removeChars(DNASequence& sequence, int startPos, int endPos, U2OpStatus& os);
    static void removeChars(QByteArray& sequence, int startPos, int endPos, U2OpStatus& os);

    /** Converts characters of the sequence to upper case */
    static void toUpperCase(DNASequence& sequence);

    /** Make the sequence empty (do not change name, alphabet, etc.) */
    static void makeEmpty(DNASequence& sequence);
};

} // namespace

#endif
