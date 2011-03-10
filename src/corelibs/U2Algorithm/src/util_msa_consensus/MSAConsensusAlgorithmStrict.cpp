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

char MSAConsensusAlgorithmStrict::getConsensusChar(const MAlignment& msa, int column) const {
    QVector<int> freqsByChar(256, 0);
    int nonGaps = 0;
    uchar topChar = MSAConsensusUtils::getColumnFreqs(msa, column, freqsByChar, nonGaps);

    //use gap is top char frequency is lower than threshold
    int nSeq = msa.getNumRows();
    int currentThreshold = getThreshold();
    int cntToUseGap = int(currentThreshold / 100.0 * nSeq);
    int topFreq = freqsByChar[topChar];
    char res = topFreq < cntToUseGap ? MAlignment_GapChar : (char)topChar;
    return res;
}

} //namespace
