/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "MSAConsensusAlgorithmLevitsky.h"

#include <U2Core/MAlignment.h>

namespace U2 {

MSAConsensusAlgorithmFactoryLevitsky::MSAConsensusAlgorithmFactoryLevitsky(QObject* p)  
: MSAConsensusAlgorithmFactory(BuiltInConsensusAlgorithms::LEVITSKY_ALGO, 
                               ConsensusAlgorithmFlags(ConsensusAlgorithmFlag_Nucleic ) | ConsensusAlgorithmFlag_SupportThreshold,
                               p)
{
}

QString MSAConsensusAlgorithmFactoryLevitsky::getDescription() const  {
    return tr("The algorithm proposed by Victor Levitsky to work with DNA alignments.\n" \
        "Collects global alignment frequency for every symbol using extended (15 symbols) DNA alphabet first.\n" \
        "For every column selects the most rare symbol in the whole alignment with percentage in the column " \
        "greater or equals to the threshold value."
        );
}

QString MSAConsensusAlgorithmFactoryLevitsky::getName() const  {
    return tr("Levitsky");
}

MSAConsensusAlgorithm* MSAConsensusAlgorithmFactoryLevitsky::createAlgorithm(const MAlignment& ma, QObject* p) {
    return new MSAConsensusAlgorithmLevitsky(this, ma, p);
}

//////////////////////////////////////////////////////////////////////////
// Algorithm

/*
Code table from Victor Levitsky:
A       A       1              
T       T       1              
G       G       1              
C       C       1              
W       A,T     2       WEAK   
R       A,G     2       PURINE 
M       A,C     2       AMINO- (+ charge)       
K       T,G     2       KETO- (- charge)
Y       T,C     2       PYRIMIDINE     
S       G,C     2       STRONG 
B       T,G,C   3       not A (B is near with A in Latin alphabet)
V       A,G,C   3       not T (---||---)
H       A,T,C   3       not G (---||---)
D       A,T,G   3       not C (---||---)
N       A,T,G,C 4       Any

*/

static void registerHit(int* data, char c) {
    int idx = uchar(c);
    data[idx]++;
    switch(c) {
        case 'A':
            data['W']++;
            data['R']++;
            data['M']++;
            data['V']++;
            data['H']++;
            data['D']++;
            data['N']++;
            break;
        case 'C':
            data['M']++;
            data['Y']++;
            data['S']++;
            data['B']++;
            data['V']++;
            data['H']++;
            data['N']++;
            break;
        case 'G':
            data['R']++;
            data['K']++;
            data['S']++;
            data['B']++;
            data['V']++;
            data['D']++;
            data['N']++;
            break;
        case 'T':
        case 'U':
            data['W']++;
            data['K']++;
            data['Y']++;
            data['B']++;
            data['H']++;
            data['D']++;
            data['N']++;
            break;
    }
}

MSAConsensusAlgorithmLevitsky::MSAConsensusAlgorithmLevitsky(MSAConsensusAlgorithmFactoryLevitsky* f, const MAlignment& ma,  QObject* p) 
: MSAConsensusAlgorithm(f, p)
{
    globalFreqs.resize(256);
    qMemSet(globalFreqs.data(), 0, globalFreqs.size() * 4);

    int* freqsData = globalFreqs.data();
    int len = ma.getLength();
    foreach (const MAlignmentRow& row, ma.getRows()) {
        for (int i = 0; i < len; i++) {
            char c = row.charAt(i);
            registerHit(freqsData, c);
        }
    }
}


char MSAConsensusAlgorithmLevitsky::getConsensusChar(const MAlignment& msa, int column) const {
    // count local freqs first
    QVarLengthArray<int> localFreqs(256);
    qMemSet(localFreqs.data(), 0, localFreqs.size() * 4);

    int* freqsData = localFreqs.data();
    int nSeq = msa.getNumRows();
    for (int seq = 0; seq < nSeq; seq++) {
        char c = msa.charAt(seq, column);
        registerHit(freqsData, c);
    }

    //find all symbols with freq > threshold, select one with the lowest global freq
    char selectedChar = MAlignment_GapChar;
    int selectedGlobalFreq = nSeq * msa.getLength();
    int thresholdScore = getThreshold();
    int minFreq = int(float(nSeq) * thresholdScore / 100);
    for (int c = 'A'; c <= 'Y'; c++) {
        int localFreq = freqsData[uchar(c)];
        if (localFreq < minFreq) {
            continue;
        }
        int globalFreq = globalFreqs[uchar(c)];
        if (globalFreq < selectedGlobalFreq) {
            selectedGlobalFreq = globalFreq;
            selectedChar = c;
        }
    }
    return selectedChar;
}

} //namespace
