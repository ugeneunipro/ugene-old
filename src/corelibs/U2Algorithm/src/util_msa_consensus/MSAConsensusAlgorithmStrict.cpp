/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "MSAConsensusAlgorithmStrict.h"

#include <U2Core/MAlignment.h>
#include "MSAConsensusUtils.h"

#include <QtCore/QVector>

namespace U2 {


MSAConsensusAlgorithmFactoryStrict::MSAConsensusAlgorithmFactoryStrict(QObject* p)
: MSAConsensusAlgorithmFactory(BuiltInConsensusAlgorithms::STRICT_ALGO, ConsensusAlgorithmFlags_AllAlphabets | ConsensusAlgorithmFlag_SupportThreshold, p)
{
}


QString MSAConsensusAlgorithmFactoryStrict::getDescription() const  {
    return tr("The algorithm returns gap character ('-') if symbol frequency in a column is lower than threshold specified.");
}

QString MSAConsensusAlgorithmFactoryStrict::getName() const  {
    return tr("Strict");
}

MSAConsensusAlgorithm* MSAConsensusAlgorithmFactoryStrict::createAlgorithm(const MAlignment&, QObject* p) {
    return new MSAConsensusAlgorithmStrict(this, p);
}

//////////////////////////////////////////////////////////////////////////
// Algorithm

char MSAConsensusAlgorithmStrict::getConsensusChar(const MAlignment& msa, int column, const QVector<qint64> &seqIdx) const {
    QVector<int> freqsByChar(256, 0);
    int nonGaps = 0;
    uchar topChar = MSAConsensusUtils::getColumnFreqs(msa, column, freqsByChar, nonGaps, seqIdx);

    //use gap is top char frequency is lower than threshold
    int nSeq =( seqIdx.isEmpty() ? msa.getNumRows() : seqIdx.size());
    int currentThreshold = getThreshold();
    int cntToUseGap = int(currentThreshold / 100.0 * nSeq);
    int topFreq = freqsByChar[topChar];
    char res = topFreq < cntToUseGap ? MAlignment_GapChar : (char)topChar;
    return res;
}

} //namespace
