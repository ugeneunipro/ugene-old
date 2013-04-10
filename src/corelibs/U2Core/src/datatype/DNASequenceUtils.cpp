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

#include "DNASequenceUtils.h"

#include <U2Core/Log.h>


namespace U2 {

void DNASequenceUtils::append(DNASequence& sequence, const DNASequence& appendedSequence, U2OpStatus& /* os */) {
    sequence.seq += appendedSequence.constSequence();
}

DnaSequencesMatchStatus DNASequenceUtils::compare(const DNASequence& firstSeq, const DNASequence& secondSec) {
    if (firstSeq.constSequence() == secondSec.constSequence()) {
        return MatchExactly;
    }
    else {
        return DoNotMatch;
    }
}

void DNASequenceUtils::removeChars(DNASequence& sequence, int startPos, int endPos, U2OpStatus& os) {
    removeChars(sequence.seq, startPos, endPos, os);
}

void DNASequenceUtils::removeChars(QByteArray& sequence, int startPos, int endPos, U2OpStatus& os) {
    if ((endPos <= startPos) || (startPos < 0) || (endPos > sequence.length())) {
        coreLog.trace(QString("Internal error: incorrect parameters was passed to DNASequenceUtils::removeChars,"
            "startPos '%1', endPos '%2', sequence length '%3'!").arg(startPos).arg(endPos).arg(sequence.length()));
        os.setError("Can't remove chars from a sequence.");
        return;
    }

    sequence.remove(startPos, endPos - startPos);
}

void DNASequenceUtils::toUpperCase(DNASequence& sequence) {
    sequence.seq = sequence.seq.toUpper();
}

void DNASequenceUtils::makeEmpty(DNASequence& sequence) {
    sequence.seq = QByteArray();
}

} // namespace