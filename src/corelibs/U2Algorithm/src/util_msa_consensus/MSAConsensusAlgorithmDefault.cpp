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

#include "MSAConsensusAlgorithmDefault.h"

#include <U2Core/MAlignment.h>
#include <QtCore/QVector>

namespace U2 {

MSAConsensusAlgorithmFactoryDefault::MSAConsensusAlgorithmFactoryDefault(QObject* p) 
: MSAConsensusAlgorithmFactory(BuiltInConsensusAlgorithms::DEFAULT_ALGO, ConsensusAlgorithmFlags_NuclAmino | ConsensusAlgorithmFlag_SupportThreshold, p)
{

}


QString MSAConsensusAlgorithmFactoryDefault::getDescription() const {
    return tr("Based on JalView algorithm. Returns '+' if there are 2 characters with high frequency. Returns symbol in lower case if the symbol content in a row is lower than the threshold specified.");
}

QString MSAConsensusAlgorithmFactoryDefault::getName() const {
    return tr("Default");
}


MSAConsensusAlgorithm* MSAConsensusAlgorithmFactoryDefault::createAlgorithm(const MAlignment&, QObject* p) {
    return new MSAConsensusAlgorithmDefault(this, p);
}

//////////////////////////////////////////////////////////////////////////
// Algorithm

char MSAConsensusAlgorithmDefault::getConsensusCharAndScore(const MAlignment& msa, int pos, int& cnt) const {
    //TODO: use var-length array!
    QVector<QPair<int, char> > freqs(32);
    int ch = MAlignment_GapChar;
    int nSeq = msa.getNumRows();
    for (int seq = 0; seq < nSeq; seq++) {
        uchar c = (uchar)msa.charAt(seq, pos);
        if (c >= 'A' && c <= 'Z') {
            int idx = c - 'A';
            assert(idx >=0 && idx <= freqs.size());
            freqs[idx].first++;
            freqs[idx].second = c;
        }
    }
    qSort(freqs);
    int p1 = freqs[freqs.size()-1].first;
    int p2 = freqs[freqs.size()-2].first;
    if (p1 == 0 || (p1 == 1 && nSeq > 1)) {
        ch = MAlignment_GapChar;
        cnt = 0;
    } else {
        int c1 = freqs[freqs.size()-1].second;
        ch = p2 == p1 ? '+' : c1;
        cnt = p1;
    }

    //lowercase alpha chars with < threshold% content
    int currentThreshold = getThreshold();
    int cntToUseLowerCase = int(currentThreshold / 100.0 * nSeq);
    if (cnt < cntToUseLowerCase && (ch >='A' && ch <='Z')) {
        ch = ch + ('a'-'A');
    }

    return ch;
}

} //namespace
