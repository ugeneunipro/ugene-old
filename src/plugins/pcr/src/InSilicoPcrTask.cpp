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

#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/L10n.h>

#include "PrimerStatistics.h"

#include "InSilicoPcrTask.h"

namespace U2 {

InSilicoPcrTaskSettings::InSilicoPcrTaskSettings()
: isCircular(false), forwardMismatches(0), reverseMismatches(0), maxProductSize(0), perfectMatch(0)
{

}

InSilicoPcrProduct::InSilicoPcrProduct()
: ta(0.0), forwardPrimerMatchLength(0), reversePrimerMatchLength(0)
{

}

InSilicoPcrTask::InSilicoPcrTask(const InSilicoPcrTaskSettings &settings)
: Task(tr("In Silico PCR"), TaskFlags(TaskFlag_ReportingIsSupported) | TaskFlag_ReportingIsEnabled | TaskFlags_FOSE_COSC),
settings(settings), forwardSearch(NULL), reverseSearch(NULL), minProductSize(0)
{
    GCOUNTER(cvar, tvar, "InSilicoPcrTask");
    minProductSize = qMax(settings.forwardPrimer.length(), settings.reversePrimer.length());
}

namespace {
    int getMaxError(const QByteArray &primer, uint mismatches) {
        return qMin(int(mismatches), primer.length() / 2);
    }
}

FindAlgorithmTaskSettings InSilicoPcrTask::getFindPatternSettings(U2Strand::Direction direction) {
    FindAlgorithmTaskSettings result;
    const DNAAlphabet *alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    SAFE_POINT_EXT(NULL != alphabet, setError(L10N::nullPointerError("DNA Alphabet")), result);
    DNATranslation *translator = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(alphabet);
    SAFE_POINT_EXT(NULL != translator, setError(L10N::nullPointerError("DNA Translator")), result);

    result.sequence = settings.sequence;
    result.searchIsCircular = settings.isCircular;
    result.searchRegion.length = settings.sequence.length();
    result.patternSettings = FindAlgorithmPatternSettings_Subst;
    result.strand = FindAlgorithmStrand_Both;

    if (U2Strand::Direct == direction) {
        result.pattern = settings.forwardPrimer;
        result.maxErr = getMaxError(settings.forwardPrimer, settings.forwardMismatches);
    } else {
        result.pattern = settings.reversePrimer;
        result.maxErr = getMaxError(settings.reversePrimer, settings.reverseMismatches);
    }

    result.complementTT = translator;

    return result;
}

void InSilicoPcrTask::prepare() {
    FindAlgorithmTaskSettings forwardSettings = getFindPatternSettings(U2Strand::Direct);
    CHECK_OP(stateInfo, );
    FindAlgorithmTaskSettings reverseSettings = getFindPatternSettings(U2Strand::Complementary);
    CHECK_OP(stateInfo, );
    forwardSearch = new FindAlgorithmTask(forwardSettings);
    reverseSearch = new FindAlgorithmTask(reverseSettings);
    addSubTask(forwardSearch);
    addSubTask(reverseSearch);
}

void InSilicoPcrTask::run() {
    QList<FindAlgorithmResult> forwardResults = forwardSearch->popResults();
    QList<FindAlgorithmResult> reverseResults = reverseSearch->popResults();
    algoLog.details(tr("Forward primers found: %1").arg(forwardResults.size()));
    algoLog.details(tr("Reverse primers found: %1").arg(reverseResults.size()));

    foreach (const FindAlgorithmResult &forward, forwardResults) {
        foreach (const FindAlgorithmResult &reverse, reverseResults) {
            CHECK(!isCanceled(), );
            if (forward.strand == reverse.strand) {
                continue;
            }
            PrimerBind leftBind = getPrimerBind(forward, reverse, U2Strand::Direct);
            PrimerBind rightBind = getPrimerBind(forward, reverse, U2Strand::Complementary);

            qint64 productSize = getProductSize(leftBind.region, rightBind.region);
            bool accepted = filter(leftBind, rightBind, productSize);
            if (accepted) {
                U2Region productRegion(leftBind.region.startPos, productSize);
                InSilicoPcrProduct product = createResult(leftBind.region, productRegion, rightBind.region, forward.strand.getDirection());
                results << product;
            }
        }
    }
}

InSilicoPcrTask::PrimerBind InSilicoPcrTask::getPrimerBind(const FindAlgorithmResult &forward, const FindAlgorithmResult &reverse, U2Strand::Direction direction) const {
    PrimerBind result;
    bool switched = forward.strand.isCompementary();
    if ((U2Strand::Direct == direction && switched) ||
        (U2Strand::Complementary == direction && !switched)) {
        result.primer = settings.reversePrimer;
        result.mismatches = settings.reverseMismatches;
        result.region = reverse.region;
    } else {
        result.primer = settings.forwardPrimer;
        result.mismatches = settings.forwardMismatches;
        result.region = forward.region;
    }
    return result;
}

bool InSilicoPcrTask::filter(const PrimerBind &leftBind, const PrimerBind &rightBind, qint64 productSize) const {
    CHECK(isCorrectProductSize(productSize, minProductSize), false);

    if (settings.perfectMatch > 0) {
        if (leftBind.mismatches > 0) {
            CHECK(checkPerfectMatch(leftBind.region, leftBind.primer, U2Strand::Direct), false);
        }
        if (rightBind.mismatches > 0) {
            CHECK(checkPerfectMatch(rightBind.region, rightBind.primer, U2Strand::Complementary), false);
        }
    }
    return true;
}

bool InSilicoPcrTask::isCorrectProductSize(qint64 productSize, qint64 minPrimerSize) const {
    return (productSize >= minPrimerSize) && (productSize <= qint64(settings.maxProductSize));
}

bool InSilicoPcrTask::checkPerfectMatch(const U2Region &region, QByteArray primer, U2Strand::Direction direction) const {
    const QByteArray sequence = getSequence(region, direction);
    SAFE_POINT(sequence.length() == primer.length(), L10N::internalError("Wrong match length"), false);

    int perfectMatch = qMin(sequence.length(), int(settings.perfectMatch));
    for (int i=0; i<perfectMatch; i++) {
        if (sequence.at(sequence.length() - 1 - i) != primer.at(primer.length() - 1 - i)) {
            return false;
        }
    }
    return true;
}

QByteArray InSilicoPcrTask::getSequence(const U2Region &region, U2Strand::Direction direction) const {
    const QByteArray sequence = settings.sequence.mid(region.startPos, region.length);
    if (U2Strand::Complementary == direction) {
        return DNASequenceUtils::reverseComplement(sequence);
    }
    return sequence;
}

QString InSilicoPcrTask::generateReport() const {
    PrimersPairStatistics calc(settings.forwardPrimer, settings.reversePrimer);
    QString spaces;
    for (int i=0; i<150; i++) {
        spaces += "&nbsp;";
    }
    return tr("Products found: %1").arg(results.size()) +
           "<br>" +
           spaces +
           "<br>" +
           tr("Primers details:") +
           calc.generateReport();
}

InSilicoPcrProduct InSilicoPcrTask::createResult(const U2Region &leftPrimer, const U2Region &product, const U2Region &rightPrimer, U2Strand::Direction direction) const {
    QByteArray productSequence = settings.sequence.mid(product.startPos, product.length);
    if (productSequence.length() < product.length) {
        assert(settings.isCircular);
        productSequence += settings.sequence.left(product.endPos() - settings.sequence.length());
    }

    InSilicoPcrProduct result;
    result.region = product;
    result.ta = PrimerStatistics::getAnnealingTemperature(productSequence, settings.forwardPrimer, settings.reversePrimer);
    result.forwardPrimerMatchLength = leftPrimer.length;
    result.reversePrimerMatchLength = rightPrimer.length;
    result.forwardPrimer = settings.forwardPrimer;
    result.reversePrimer = settings.reversePrimer;
    if (U2Strand::Complementary == direction) {
        qSwap(result.forwardPrimer, result.reversePrimer);
    }
    return result;
}

qint64 InSilicoPcrTask::getProductSize(const U2Region &left, const U2Region &right) const {
    qint64 result = right.endPos() - left.startPos;
    if (result < 0 && settings.isCircular) {
        return right.endPos() + (settings.sequence.length() - left.startPos);
    }
    return result;
}

const QList<InSilicoPcrProduct> & InSilicoPcrTask::getResults() const {
    return results;
}

const InSilicoPcrTaskSettings & InSilicoPcrTask::getSettings() const {
    return settings;
}

} // U2
