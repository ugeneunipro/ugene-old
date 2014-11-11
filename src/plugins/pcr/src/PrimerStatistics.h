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

#ifndef _U2_PRIMER_STATISTICS_H_
#define _U2_PRIMER_STATISTICS_H_

#include <QObject>

namespace U2 {

class PrimerStatistics : public QObject {
public:
    static bool checkPcrPrimersPair(const QByteArray &forward, const QByteArray &reverse, QString &error);
    static double getMeltingTemperature(const QByteArray &sequence);
    static double getAnnealingTemperature(const QByteArray &product, const QByteArray &forwardPrimer, const QByteArray &reversePrimer);

    static QString generatePrimersPairReport(const QByteArray &forward, const QByteArray &reverse);
};

class PrimerStatisticsCalculator {
public:
    enum Direction {Forward, Reverse, DoesntMatter};
    PrimerStatisticsCalculator(const QByteArray &sequence, Direction direction = DoesntMatter);

    double getGC() const;
    double getTm() const;
    int getGCClamp() const;
    int getRuns() const;

    bool isValidGC(QString &error) const;
    bool isValidTm(QString &error) const;
    bool isValidGCClamp(QString &error) const;
    bool isValidRuns(QString &error) const;

    static const double GC_BOTTOM;
    static const double GC_TOP;
    static const double TM_BOTTOM;
    static const double TM_TOP;
    static const int CLAMP_BOTTOM;
    static const int RUNS_TOP;

private:
    QString getMessage(const QString &error) const;

private:
    const QByteArray sequence;
    Direction direction;
    int nA;
    int nC;
    int nG;
    int nT;
    int maxRun;
};

class PrimersPairStatistics {
public:
    PrimersPairStatistics(const QByteArray &forward, const QByteArray &reverse);

    QString getFirstError() const;
    QString generateReport() const;

    static const QString TmString;
    static QString toString(double value);

private:
    PrimerStatisticsCalculator forward;
    PrimerStatisticsCalculator reverse;
};

} // U2

#endif // _U2_PRIMER_STATISTICS_H_
