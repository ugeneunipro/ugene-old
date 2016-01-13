/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SafePoints.h>

#include "DNASequenceUtils.h"

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
        coreLog.trace(L10N::internalError("incorrect parameters was passed to DNASequenceUtils::removeChars,"
            "startPos '%1', endPos '%2', sequence length '%3'!").arg(startPos).arg(endPos).arg(sequence.length()));
        os.setError("Can't remove chars from a sequence.");
        return;
    }

    sequence.remove(startPos, endPos - startPos);
}

void DNASequenceUtils::insertChars(QByteArray& sequence, int startPos, const QByteArray& newChars, U2OpStatus& os) {
    int endPos = startPos + newChars.length();
    CHECK_EXT(newChars.length() > 0, os.setError("Array of chars for replacing is empty!"), );
    if ((startPos < 0) || (endPos > sequence.length())) {
        coreLog.trace(L10N::internalError("incorrect parameters was passed to DNASequenceUtils::insertChars, "
            "startPos '%1', endPos '%2', sequence length '%3'!").arg(startPos).arg(endPos).arg(sequence.length()));
        os.setError("Can't remove chars from a sequence.");
        return;
    }

    sequence.insert(startPos, newChars);
}

void DNASequenceUtils::replaceChars(QByteArray& sequence, int startPos, const QByteArray& newChars, U2OpStatus& os) {
    int endPos = startPos + newChars.length();
    CHECK_EXT(newChars.length() > 0, os.setError("Array of chars for replacing is empty!"),);
    if ((startPos < 0) || (endPos > sequence.length())) {
        coreLog.trace(QString("Internal error: incorrect parameters was passed to DNASequenceUtils::replaceChars,"
            "startPos '%1', endPos '%2', sequence length '%3'!").arg(startPos).arg(endPos).arg(sequence.length()));
        os.setError("Can't remove chars from a sequence.");
        return;
    }

    sequence.replace(startPos, newChars.length(), newChars);
}

void DNASequenceUtils::toUpperCase(DNASequence& sequence) {
    sequence.seq = sequence.seq.toUpper();
}

void DNASequenceUtils::makeEmpty(DNASequence& sequence) {
    sequence.seq = QByteArray();
}

QByteArray DNASequenceUtils::reverse(const QByteArray &sequence) {
    QByteArray result = sequence;
    TextUtils::reverse(result.data(), result.length());
    return result;
}

QByteArray DNASequenceUtils::complement(const QByteArray &sequence) {
    const DNAAlphabet *alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    SAFE_POINT(NULL != alphabet, L10N::nullPointerError("DNA Alphabet"), "");

    DNATranslation *translator = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(alphabet);
    SAFE_POINT(NULL != translator, L10N::nullPointerError("DNA Translator"), "");

    QByteArray result(sequence.length(), 0);
    translator->translate(sequence.constData(), sequence.length(), result.data(), result.length());
    return result;
}

QByteArray DNASequenceUtils::reverseComplement(const QByteArray &sequence) {
    return reverse(complement(sequence));
}

} // namespace
