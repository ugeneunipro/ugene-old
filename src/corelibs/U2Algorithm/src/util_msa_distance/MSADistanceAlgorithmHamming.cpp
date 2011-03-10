#include "MSADistanceAlgorithmHamming.h"

#include <U2Core/MAlignment.h>

namespace U2 {

MSADistanceAlgorithmFactoryHamming::MSADistanceAlgorithmFactoryHamming(QObject* p) 
: MSADistanceAlgorithmFactory(BuiltInDistanceAlgorithms::HAMMING_ALGO, DistanceAlgorithmFlags_NuclAmino, p)
{

}


QString MSADistanceAlgorithmFactoryHamming::getDescription() const {
    return tr("Based on Hamming distance between two sequences");
}

QString MSADistanceAlgorithmFactoryHamming::getName() const {
    return tr("Hamming");
}


MSADistanceAlgorithm* MSADistanceAlgorithmFactoryHamming::createAlgorithm(const MAlignment& ma, QObject* p) {
    return new MSADistanceAlgorithmHamming(this, ma);
}

//////////////////////////////////////////////////////////////////////////
// Algorithm

void MSADistanceAlgorithmHamming::run() {
    int nSeq = ma.getNumRows();
    for (int i = 0; i < nSeq; i++) {
        for (int j = i; j < nSeq; j++) {
            int sim = 0;
            for (int k = 0; k < ma.getLength(); k++) {
                if (ma.charAt(i, k) == ma.charAt(j, k)) sim++;
            }
            lock.lock();
            distanceTable[i][j] = distanceTable[j][i] = sim;
            lock.unlock();
        }
    }
}

} //namespace
