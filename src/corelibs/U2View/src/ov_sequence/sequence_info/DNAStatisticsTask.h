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

#ifndef _U2_DNA_STATISTICS_TASK_H_
#define _U2_DNA_STATISTICS_TASK_H_

#include <U2Core/BackgroundTaskRunner.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2Type.h>
#include <QtCore/QVector>
#include <QtCore/QMap>

namespace U2 {

class DNAAlphabet;
class U2SequenceDbi;

struct DNAStatistics {
    DNAStatistics();
    qint64 length;
    double gcContent;
    double molarWeight;
    qint64 molarExtCoef;
    double meltingTm;

    double nmoleOD260;
    double mgOD260;

    double molecularWeight;
    double isoelectricPoint;

    void clear();
};

class U2VIEW_EXPORT DNAStatisticsTask : public BackgroundTask< DNAStatistics > {
public:
    DNAStatisticsTask(const DNAAlphabet* alphabet, const U2EntityRef seqRef, const U2Region& region = U2Region());
    virtual void run();
private:
    const DNAAlphabet*          alphabet;
    U2EntityRef                 seqRef;
    U2Region                    region;

    qint64 nA;
    qint64 nC;
    qint64 nG;
    qint64 nT;

    static QMap<char,double> pMWMap; // protein molecular weight
    static QMap<char,double> pKaMap; // pKa values
    static QMap<char,int> pChargeMap; // protein charges

    void computeStats();
    double calcPi(U2SequenceDbi* sequenceDbi);
    double calcChargeState(const QMap<char,int>& countMap, double pH );
};

} // namespace

#endif // _U2_DNA_STATISTICS_TASK_H_
