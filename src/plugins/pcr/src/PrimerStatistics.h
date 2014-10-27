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
};

class PrimerStatisticsCalculator {
public:
    PrimerStatisticsCalculator(const QByteArray &sequence);

    double getGCContent() const;
    double getMeltingTemperature() const;

private:
    const QByteArray sequence;
    int nA;
    int nC;
    int nG;
    int nT;
};

} // U2

#endif // _U2_PRIMER_STATISTICS_H_
