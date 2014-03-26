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

#include <U2Core/DNAAlphabet.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/DNASequenceObject.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <math.h>

#include "DNAStatisticsTask.h"

namespace U2 {

DNAStatistics::DNAStatistics() {
    clear();
}

void DNAStatistics::clear() {
    length = 0;

    gcContent = 0;
    molarWeight = 0;
    molarExtCoef = 0;
    meltingTm = 0;

    nmoleOD260 = 0;
    mgOD260 = 0;

    molecularWeight = 0;
    isoelectricPoint = 0;
}

static QMap<char,double> createProteinMWMap(){
    QMap<char,double> mwMap;

    mwMap.insert('A', 89.09); // ALA
    mwMap.insert('R', 174.20); // ARG
    mwMap.insert('N', 132.12); // ASN
    mwMap.insert('D', 133.10); // ASP
    mwMap.insert('B', 132.61); // ASX
    mwMap.insert('C', 121.15); // CYS
    mwMap.insert('Q', 146.15); // GLN
    mwMap.insert('E', 147.13); // GLU
    mwMap.insert('Z', 146.64); // GLX
    mwMap.insert( 'G', 75.07); // GLY
    mwMap.insert( 'H', 155.16); // HIS
    mwMap.insert( 'I',  131.17); // ILE
    mwMap.insert( 'L', 131.17); // LEU
    mwMap.insert( 'K', 146.19); // LYS
    mwMap.insert( 'M', 149.21); // MET
    mwMap.insert( 'F', 165.19); // PHE
    mwMap.insert( 'P', 115.13); //PRO
    mwMap.insert( 'S', 105.09); // SER
    mwMap.insert( 'T', 119.12); // THR
    mwMap.insert( 'W', 204.23); // TRP
    mwMap.insert( 'Y', 181.19); // TYR
    mwMap.insert( 'V', 117.15); // VAL

    return mwMap;
}

static QMap<char,double> createPKAMap() {
    QMap<char,double> res;

    res.insert('D', 4.0);
    res.insert('C', 8.5);
    res.insert('E', 4.4);
    res.insert('Y', 10.0);
    res.insert('c', 3.1); // CTERM
    res.insert('R', 12.0);
    res.insert('H', 6.5);
    res.insert('K', 10.4);
    res.insert('n',8.0); // NTERM

    return res;
}

static QMap<char,int> createChargeMap() {
    QMap<char,int> res;

    res.insert('D', -1);
    res.insert('C', -1);
    res.insert('E', -1);
    res.insert('Y', -1);
    res.insert('c', -1); // CTERM
    res.insert('R', 1);
    res.insert('H', 1);
    res.insert('K', 1);
    res.insert('n', 1); // NTERM

    return res;
}


QMap<char,double> DNAStatisticsTask::pMWMap = createProteinMWMap();
QMap<char,double> DNAStatisticsTask::pKaMap = createPKAMap();
QMap<char,int> DNAStatisticsTask::pChargeMap = createChargeMap();

DNAStatisticsTask::DNAStatisticsTask(ADVSequenceObjectContext *context, const U2Region& _region)
    : BackgroundTask< DNAStatistics > (tr("Calculate sequence statistics"), TaskFlag_None),
      ctx(context),
      region(_region),
      nA(0),
      nC(0),
      nG(0),
      nT(0)
{
    SAFE_POINT_EXT(ctx != NULL, setError(tr("Sequence context is NULL")), );
    if (region.isEmpty()) {
        region.startPos = 0;
        region.length = ctx->getSequenceLength();
    }
}

void DNAStatisticsTask::run() {
    computeStats();
}

void DNAStatisticsTask::computeStats(){
    result.clear();

    SAFE_POINT_EXT(region.endPos() <= ctx->getSequenceLength(), setError(tr("Statistics sequence region is not valid")), );
    CHECK(region.length != 0, );
    result.length = region.length;

    qint64 blockSize = 1024*1024;
    qint64 prevEnd = 0;

    const DNAAlphabet* al = ctx->getAlphabet();
    SAFE_POINT_EXT(al != NULL, setError(tr("Alphabet is NULL")), );

    do {
        if (isCanceled() || hasError()) {
            break;
        }
        U2Region r = region.intersect(U2Region(prevEnd, blockSize));
        prevEnd += blockSize;
        QByteArray seqBlock = ctx->getSequenceData(r);
        foreach(char c, seqBlock){
            if (c == 'A') {
                nA++;
            } else if (c == 'G') {
                nG++;
            } else if (c == 'T' || c == 'U') {
                nT++;
            } else if (c == 'C') {
                nC++;
            }

            if (al->isAmino()) {
                result.molecularWeight += pMWMap.value( c );
            }
        }
        stateInfo.setProgress( prevEnd * 100 / region.length );
    } while (prevEnd < region.endPos());

    // get alphabet type
    if (al->isNucleic()) {       
        result.gcContent = 100.0 * (nG + nC) / (double) region.length;

        // Calculating molar weight
        // Source: http://www.basic.northwestern.edu/biotools/oligocalc.html
        if (al->isRNA()) {
            result.molarWeight = nA * 329.21 + nT * 306.17 + nC * 305.18 + nG * 345.21 + 159.0;
        } else {
            result.molarWeight = nA * 313.21 + nT * 304.2 + nC * 289.18 + nG * 329.21 + 17.04;
        }

        result.molarExtCoef = nA*15400 + nT*8800 + nC*7300 + nG*11700;

        if (region.length < 15) {
            result.meltingTm = (nA+nT) * 2 + (nG + nC) * 4;
        } else {
            result.meltingTm = 64.9 + 41*(nG + nC-16.4)/(double)(nA+nT+nG+nC);
        }

        result.nmoleOD260 = (double)1000000 / result.molarExtCoef;

        result.mgOD260 = result.nmoleOD260 * result.molarWeight * 0.001;

    } else if (al->isAmino()) {
        static const double MWH2O = 18.0;
        result.molecularWeight = result.molecularWeight - (region.length - 1)*MWH2O;
        result.isoelectricPoint = calcPi();
    }
}

double DNAStatisticsTask::calcPi()
{
    QMap<char,int> countMap;

    qint64 blockSize = 1024*1024;
    qint64 prevEnd = 0;
    do {
        if (isCanceled() || hasError()) {
            break;
        }
        U2Region r = region.intersect(U2Region(prevEnd, blockSize));
        prevEnd += blockSize;
        QByteArray seqBlock = ctx->getSequenceData(r);
        foreach(char c, seqBlock){
            if ( pKaMap.contains( c ) ) {
                countMap[c]++;
            }
        }
    } while (prevEnd < region.endPos());

    countMap['c'] = 1;
    countMap['n'] = 1;

    static const double CUTOFF = 0.001;
    static const double INITIAL_CUTOFF = 2.0;

    double step = INITIAL_CUTOFF;
    double pH = 0;
    while (step > CUTOFF) {
        if ( calcChargeState(countMap,pH) > 0 ){
            pH += step;
        } else {
            step *= 0.5;
            pH -= step;
        }
    }
    return pH;
}

double DNAStatisticsTask::calcChargeState(const QMap<char,int>& countMap, double pH )
{
    double chargeState = 0.;
    QList<char> counts = countMap.keys();
    foreach(char r, counts) {
        if (isCanceled() || hasError()) {
            break;
        }
        double pKa = pKaMap.value(r);
        double charge = pChargeMap.value(r);
        chargeState += countMap.value(r)*charge/( 1 + pow(10.0, charge*(pH-pKa)) );
    }
    return chargeState;
}


} // namespace
