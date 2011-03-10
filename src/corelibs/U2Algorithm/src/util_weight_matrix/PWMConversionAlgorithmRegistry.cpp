#include "PWMConversionAlgorithmRegistry.h"

#include "PWMConversionAlgorithmBVH.h"
#include "PWMConversionAlgorithmLOD.h"
#include "PWMConversionAlgorithmMCH.h"
#include "PWMConversionAlgorithmNLG.h"

namespace U2 {

PWMConversionAlgorithmRegistry::PWMConversionAlgorithmRegistry(QObject* p) : QObject(p) {
    addAlgorithm(new PWMConversionAlgorithmFactoryBVH());
    addAlgorithm(new PWMConversionAlgorithmFactoryLOD());
    addAlgorithm(new PWMConversionAlgorithmFactoryMCH());
    addAlgorithm(new PWMConversionAlgorithmFactoryNLG());
}

PWMConversionAlgorithmRegistry::~PWMConversionAlgorithmRegistry() {
    QList<PWMConversionAlgorithmFactory*> list = algorithms.values();
    foreach(PWMConversionAlgorithmFactory* algo, list) {
        delete algo;
    }
}

PWMConversionAlgorithmFactory* PWMConversionAlgorithmRegistry::getAlgorithmFactory(const QString& algoId) {
    return algorithms.value(algoId);
}


void PWMConversionAlgorithmRegistry::addAlgorithm(PWMConversionAlgorithmFactory* algo) {
    const QString& id = algo->getId();
    PWMConversionAlgorithmFactory* oldVersion = algorithms.value(id);
    if (oldVersion!=NULL) {
        delete oldVersion;
        oldVersion = NULL;
    }
    algorithms[id] = algo;
}

QStringList PWMConversionAlgorithmRegistry::getAlgorithmIds() const  {
    QList<PWMConversionAlgorithmFactory*> list = algorithms.values();
    QStringList result;
    foreach(PWMConversionAlgorithmFactory* algo, list) {
        result.append(algo->getId());
    }
    return result;
}

}//namespace
