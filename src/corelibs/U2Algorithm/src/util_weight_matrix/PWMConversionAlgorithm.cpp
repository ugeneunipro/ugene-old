#include "PWMConversionAlgorithm.h"

namespace U2 {

PWMConversionAlgorithmFactory::PWMConversionAlgorithmFactory(const QString& algoId, QObject* p):
QObject(p), algorithmId(algoId) 
{
}

PWMConversionAlgorithm::PWMConversionAlgorithm(PWMConversionAlgorithmFactory* _factory, QObject* p)
: QObject(p), factory(_factory)
{
}

}