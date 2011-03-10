#include "MSADistanceAlgorithm.h" 

#include <U2Core/DNAAlphabet.h>
#include <U2Core/MAlignment.h>

#include <QtCore/QVector>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
// Factory    

MSADistanceAlgorithmFactory::MSADistanceAlgorithmFactory(const QString& algoId, DistanceAlgorithmFlags _flags, QObject* p)
: QObject(p), algorithmId(algoId), flags(_flags)
{
}

DistanceAlgorithmFlags MSADistanceAlgorithmFactory::getAphabetFlags(const DNAAlphabet* al) {
    if (al->getType() == DNAAlphabet_AMINO) {
        return DistanceAlgorithmFlag_Amino;
    } else if (al->getType() == DNAAlphabet_NUCL) {
        return DistanceAlgorithmFlag_Nucleic;
    } 
    assert(al->getType() ==  DNAAlphabet_RAW);
    return DistanceAlgorithmFlag_Raw;
}

//////////////////////////////////////////////////////////////////////////
// Algorithm

MSADistanceAlgorithm::MSADistanceAlgorithm(MSADistanceAlgorithmFactory* _factory, const MAlignment& _ma)
: Task(tr("MSA distance algorithm \"%1\" task").arg(_factory->getName()), TaskFlag_None), factory(_factory), ma(_ma)
{    
    for (int i = 0; i < ma.getNumRows(); i++) {
        distanceTable.append(QVarLengthArray<int>(ma.getNumRows()));
        qMemSet(distanceTable[i].data(), 0, ma.getNumRows() * sizeof(int));
    }    
}

int MSADistanceAlgorithm::getSimilarity(int row1, int row2) {    
    lock.lock();
    int res = distanceTable[row1][row2];
    lock.unlock();
    return res;    
}

} //namespace
