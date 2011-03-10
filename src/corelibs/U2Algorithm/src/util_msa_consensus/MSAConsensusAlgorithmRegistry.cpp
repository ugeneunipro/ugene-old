#include "MSAConsensusAlgorithmRegistry.h"

#include "MSAConsensusAlgorithmDefault.h"
#include "MSAConsensusAlgorithmStrict.h"
#include "MSAConsensusAlgorithmClustal.h"
#include "MSAConsensusAlgorithmLevitsky.h"

namespace U2 {

MSAConsensusAlgorithmRegistry::MSAConsensusAlgorithmRegistry(QObject* p) : QObject(p) {
    addAlgorithm(new MSAConsensusAlgorithmFactoryDefault());
    addAlgorithm(new MSAConsensusAlgorithmFactoryStrict());
    addAlgorithm(new MSAConsensusAlgorithmFactoryClustal());
    addAlgorithm(new MSAConsensusAlgorithmFactoryLevitsky());
}

MSAConsensusAlgorithmRegistry::~MSAConsensusAlgorithmRegistry() {
    QList<MSAConsensusAlgorithmFactory*> list = algorithms.values();
    foreach(MSAConsensusAlgorithmFactory* algo, list) {
        delete algo;
    }
}

MSAConsensusAlgorithmFactory* MSAConsensusAlgorithmRegistry::getAlgorithmFactory(const QString& algoId) {
    return algorithms.value(algoId);
}


void MSAConsensusAlgorithmRegistry::addAlgorithm(MSAConsensusAlgorithmFactory* algo) {
    const QString& id = algo->getId();
    MSAConsensusAlgorithmFactory* oldVersion = algorithms.value(id);
    if (oldVersion!=NULL) {
        delete oldVersion;
        oldVersion = NULL;
    }
    algorithms[id] = algo;
}

QStringList MSAConsensusAlgorithmRegistry::getAlgorithmIds() const  {
    QList<MSAConsensusAlgorithmFactory*> list = algorithms.values();
    QStringList result;
    foreach(MSAConsensusAlgorithmFactory* algo, list) {
        result.append(algo->getId());
    }
    return result;
}

QList<MSAConsensusAlgorithmFactory*> MSAConsensusAlgorithmRegistry::getAlgorithmFactories(ConsensusAlgorithmFlags flags) const {
    QList<MSAConsensusAlgorithmFactory*> allFactories = getAlgorithmFactories();
    QList<MSAConsensusAlgorithmFactory*> result;
    foreach(MSAConsensusAlgorithmFactory* f, allFactories) {
        if ((flags & f->getFlags()) == flags) {
            result.append(f);
        }
    }
    return result;
}

}//namespace
