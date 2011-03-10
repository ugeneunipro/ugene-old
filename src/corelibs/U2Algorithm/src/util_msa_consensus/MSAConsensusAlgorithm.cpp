#include "MSAConsensusAlgorithm.h" 
#include "MSAConsensusUtils.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/MAlignment.h>

#include <QtCore/QVector>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
// Factory    

MSAConsensusAlgorithmFactory::MSAConsensusAlgorithmFactory(const QString& algoId, ConsensusAlgorithmFlags _flags, QObject* p)
: QObject(p), algorithmId(algoId), flags(_flags)
{
}

ConsensusAlgorithmFlags MSAConsensusAlgorithmFactory::getAphabetFlags(const DNAAlphabet* al) {
    if (al->getType() == DNAAlphabet_AMINO) {
        return ConsensusAlgorithmFlag_Amino;
    } else if (al->getType() == DNAAlphabet_NUCL) {
        return ConsensusAlgorithmFlag_Nucleic;
    } 
    assert(al->getType() ==  DNAAlphabet_RAW);
    return ConsensusAlgorithmFlag_Raw;
}

//////////////////////////////////////////////////////////////////////////
// Algorithm

MSAConsensusAlgorithm::MSAConsensusAlgorithm(MSAConsensusAlgorithmFactory* _factory, QObject* p)
: QObject(p), factory(_factory) , threshold(0)
{
}

char MSAConsensusAlgorithm::getConsensusCharAndScore(const MAlignment& ma, int column, int& score) const {
    char consensusChar = getConsensusChar(ma, column);

    //now compute score using most freq character
    int nonGaps = 0;
    QVector<int> freqsByChar(256);
    uchar topChar = MSAConsensusUtils::getColumnFreqs(ma, column, freqsByChar, nonGaps);
    score = freqsByChar[topChar];

    return consensusChar;
}

void MSAConsensusAlgorithm::setThreshold(int val) {
    int newThreshold = qBound(getMinThreshold(), val, getMaxThreshold());
    if (newThreshold == threshold) {
        return;
    }
    threshold = newThreshold;
    emit si_thresholdChanged(newThreshold);
}

} //namespace
