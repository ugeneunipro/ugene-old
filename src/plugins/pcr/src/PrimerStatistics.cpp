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

#include <U2Core/U2SafePoints.h>

#include "PrimerGroupBox.h"

#include "PrimerStatistics.h"

namespace U2 {

bool PrimerStatistics::checkPcrPrimersPair(const QByteArray &forward, const QByteArray &reverse, QString &error) {
    PrimersPairStatistics calc(forward, reverse);
    error = calc.getFirstError();
    return error.isEmpty();
}

double PrimerStatistics::getMeltingTemperature(const QByteArray &sequence) {
    PrimerStatisticsCalculator calc(sequence);
    return calc.getTm();
}

double PrimerStatistics::getAnnealingTemperature(const QByteArray &product, const QByteArray &forwardPrimer, const QByteArray &reversePrimer) {
    double primersTm = (getMeltingTemperature(forwardPrimer) + getMeltingTemperature(reversePrimer)) / 2;
    double productTm = getMeltingTemperature(product);
    return 0.3 * primersTm + 0.7 * productTm - 14.9;
}

/************************************************************************/
/* PrimerStatisticsCalculator */
/************************************************************************/
const double PrimerStatisticsCalculator::GC_BOTTOM = 50.0;
const double PrimerStatisticsCalculator::GC_TOP = 60.0;
const double PrimerStatisticsCalculator::TM_BOTTOM = 55.0;
const double PrimerStatisticsCalculator::TM_TOP = 80.0;
const int PrimerStatisticsCalculator::CLAMP_BOTTOM = 1;
const int PrimerStatisticsCalculator::RUNS_TOP = 4;

PrimerStatisticsCalculator::PrimerStatisticsCalculator(const QByteArray &sequence)
: sequence(sequence), nA(0), nC(0), nG(0), nT(0), maxRun(0)
{
    CHECK(!sequence.isEmpty(), );

    int currentRun = 0;
    char prevC = sequence[0];
    foreach(const char c, sequence) {
        switch (c) {
            case 'A': nA++; break;
            case 'C': nC++; break;
            case 'G': nG++; break;
            case 'T': nT++; break;
            case 'N': break;
            default: FAIL(QString("Unexpected symbol: ") + c, );
        }
        if (prevC == c) {
            currentRun++;
        } else {
            if (currentRun > maxRun) {
                maxRun = currentRun;
            }
            currentRun = 1;
            prevC = c;
        }
    }
    if (currentRun > maxRun) {
        maxRun = currentRun;
    }
}

double PrimerStatisticsCalculator::getGC() const {
    if (sequence.isEmpty()) {
        return 0;
    }
    return 100.0 * (nG + nC) / double(sequence.length());
}

double PrimerStatisticsCalculator::getTm() const {
    CHECK(nA + nT + nG + nC > 0, 0.0);
    if (sequence.length() < 14) {
        return (nA + nT) * 2 + (nG + nC) * 4;
    }

    return 64.9 + 41 * (nG + nC - 16.4) / double(nA + nT + nG + nC);
}

int PrimerStatisticsCalculator::getGCClamp() const {
    QString right = sequence.right(5);
    int result = 0;
    foreach (char c, right.toLocal8Bit()) {
        if ('C' == c || 'G' == c) {
            result++;
        }
    }
    return result;
}

int PrimerStatisticsCalculator::getRuns() const {
    return maxRun;
}

bool PrimerStatisticsCalculator::isValidGC(QString &error) const {
    double value = getGC();
    CHECK_EXT(value >= GC_BOTTOM, error = PrimerStatistics::tr("Low GC-content"), false);
    CHECK_EXT(value <= GC_TOP, error = PrimerStatistics::tr("High GC-content"), false);
    return true;
}

bool PrimerStatisticsCalculator::isValidTm(QString &error) const {
    double value = getTm();
    CHECK_EXT(value >= TM_BOTTOM, error = PrimerStatistics::tr("Low melting temperature"), false);
    CHECK_EXT(value <= TM_TOP, error = PrimerStatistics::tr("High melting temperature"), false);
    return true;
}

bool PrimerStatisticsCalculator::isValidGCClamp(QString &error) const {
    int value = getGCClamp();
    CHECK_EXT(value >= CLAMP_BOTTOM, error = PrimerStatistics::tr("Low GC clamp"), false);
    return true;
}

bool PrimerStatisticsCalculator::isValidRuns(QString &error) const {
    int value = getRuns();
    CHECK_EXT(value <= RUNS_TOP, error = PrimerStatistics::tr("High base runs value"), false);
    return true;
}

/************************************************************************/
/* PrimersPairStatistics */
/************************************************************************/
const QString PrimersPairStatistics::TmString = "Tm" + QString::fromLatin1(" (\x00B0") + "C)";

namespace {
    const QString GC_RANGE = QString("%1-%2").arg(PrimerStatisticsCalculator::GC_BOTTOM).arg(PrimerStatisticsCalculator::GC_TOP);
    const QString TM_RANGE = QString("%1-%2").arg(PrimerStatisticsCalculator::TM_BOTTOM).arg(PrimerStatisticsCalculator::TM_TOP);
    const QString CLAMP_RANGE = QString("&gt;=%1 G or C at 3' end").arg(PrimerStatisticsCalculator::CLAMP_BOTTOM);
    const QString RUNS_RANGE = QString("&lt;=%1 base runs").arg(PrimerStatisticsCalculator::RUNS_TOP);
}

PrimersPairStatistics::PrimersPairStatistics(const QByteArray &forward, const QByteArray &reverse)
: forward(forward), reverse(reverse)
{

}

QString PrimersPairStatistics::getFirstError() const {
    static const QString forwardPrimer = PrimerStatistics::tr("Forward primer warning: ");
    static const QString reversePrimer = PrimerStatistics::tr("Reverse primer warning: ");

    QString result;
    // GC
    if (!forward.isValidGC(result)) {
        return forwardPrimer + result;
    }
    if (!reverse.isValidGC(result)) {
        return reversePrimer + result;
    }

    // Tm
    if (!forward.isValidTm(result)) {
        return forwardPrimer + result;
    }
    if (!reverse.isValidTm(result)) {
        return reversePrimer + result;
    }

    // GC clamp
    if (!forward.isValidGCClamp(result)) {
        return forwardPrimer + result;
    }
    if (!reverse.isValidGCClamp(result)) {
        return reversePrimer + result;
    }

    // Runs
    if (!forward.isValidRuns(result)) {
        return forwardPrimer + result;
    }
    if (!reverse.isValidRuns(result)) {
        return reversePrimer + result;
    }
    return "";
}

#define CREATE_COLUMN(name, width, center) \
    result += QString("<th width=\"%1%\"/><p %2><strong>%3</strong></p></th>").arg(width).arg(center ? "align=\"center\"" : "").arg(name);

#define CREATE_CELL(value, good, center) \
    result += QString("<td %1 %2>%3</td>").arg(good ? "" : " style=\"color: red;\"").arg(center ? " align=\"center\"" : "").arg(value);

#define CREATE_ROW(criteria, range, value1, value2, good1, good2) \
    result += "<tr>"; \
    CREATE_CELL(criteria, true, false); \
    CREATE_CELL(range, true, false); \
    CREATE_CELL(value1, good1, true); \
    CREATE_CELL(value2, good2, true); \
    result += "</tr>";

QString PrimersPairStatistics::generateReport() const {
    QString result;
    result += "<table>";
    result += "<tr>";
    CREATE_COLUMN(PrimerStatistics::tr("Criteria"), 30, false);
    CREATE_COLUMN(PrimerStatistics::tr("Valid Values"), 30, false);
    CREATE_COLUMN(PrimerStatistics::tr("Forward"), 20, true);
    CREATE_COLUMN(PrimerStatistics::tr("Reverse"), 20, true);
    result += "</tr>";

    QString e;
    CREATE_ROW("% GC",      GC_RANGE,       toString(forward.getGC()),              toString(reverse.getGC()),              forward.isValidGC(e),         reverse.isValidGC(e));
    CREATE_ROW(TmString,    TM_RANGE,       toString(forward.getTm()),              toString(reverse.getTm()),              forward.isValidTm(e),         reverse.isValidTm(e));
    CREATE_ROW("GC Clamp",  CLAMP_RANGE,    QString::number(forward.getGCClamp()),  QString::number(reverse.getGCClamp()),  forward.isValidGCClamp(e),    reverse.isValidGCClamp(e));
    CREATE_ROW("Runs",      RUNS_RANGE,     QString::number(forward.getRuns()),     QString::number(reverse.getRuns()),     forward.isValidRuns(e),       reverse.isValidRuns(e));
    result += "</table>";
    return result;
}

QString PrimersPairStatistics::toString(double value) {
    QString result = QString::number(value, 'f', 2);
    result.remove(QRegExp("\\.?0+$"));
    return result;
}

} // U2
