/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_DNA_STAT_PROFILE_TASK_H_
#define _U2_DNA_STAT_PROFILE_TASK_H_

#include <QtCore/QVector>
#include <U2Core/Task.h>

namespace U2 {

class ADVSequenceObjectContext;

class DNAStatProfileTask : public Task {
    Q_OBJECT
public:
    DNAStatProfileTask(ADVSequenceObjectContext *context);
    void run();
    ReportResult report();
    const QString& getResult() const { return resultText; }

private:
    void computeStats();
    double calcPi(const QByteArray& seq);
    double calcChargeState(const QMap<char,int>& countMap, double pH );
    ADVSequenceObjectContext* ctx;
    qint64 seqLen;
    QVector<qint64>         contentCounter;
    //TODO: optimize
    QMap<QByteArray, int>   diNuclCounter;
    int nA, nT, nC, nG;
    QString resultText;
    // static tables
    static QMap<char,double> pMWMap; // protein molecular weight
    static QMap<char,double> pKaMap; // pKa values
    static QMap<char,int> pChargeMap; // protein charges
    
};

}// namespace

#endif // _U2_DNA_STAT_POFILE_TASK_H_
