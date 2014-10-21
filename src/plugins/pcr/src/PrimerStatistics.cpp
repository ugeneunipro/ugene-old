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

#include "PrimerStatistics.h"

namespace U2 {

bool PrimerStatistics::checkPcrPrimersPair(const QByteArray &forward, const QByteArray &reverse, QString &error) {
    if (forward.length() < 5) {
        error = tr("The forward primer is too short");
        return false;
    }
    if (reverse.length() < 5) {
        error = tr("The reverse primer is too short");
        return false;
    }
    return true;
}

double PrimerStatistics::getMeltingTemperature(const QByteArray &sequence) {
    int nA = 0;
    int nC = 0;
    int nG = 0;
    int nT = 0;

    foreach(const char c, sequence){
        switch (c) {
            case 'A': nA++; break;
            case 'C': nC++; break;
            case 'G': nG++; break;
            case 'T': nT++; break;
            default: FAIL(QString("Unexpected symbol: ") + c, 0.0);
        }
    }
    SAFE_POINT(sequence.length() == (nA + nT + nG + nC), "Unexpected symbols", 0.0);

    if (sequence.length() < 15) {
        return (nA + nT) * 2 + (nG + nC) * 4;
    }

    return 64.9 + 41 * (nG + nC - 16.4) / double(sequence.length());
}

} // U2
