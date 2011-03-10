#include "MSADistanceAlgorithmRegistry.h"

#include "MSADistanceAlgorithmHamming.h"
#include "MSADistanceAlgorithmHammingRevCompl.h"

namespace U2 {

MSADistanceAlgorithmRegistry::MSADistanceAlgorithmRegistry(QObject* p) : QObject(p) {
    addAlgorithm(new MSADistanceAlgorithmFactoryHamming());
    //addAlgorithm(new MSADistanceAlgorithmFactoryHammingRevCompl());
}

MSADistanceAlgorithmRegistry::~MSADistanceAlgorithmRegistry() {
    QList<MSADistanceAlgorithmFactory*> list = algorithms.values();
    foreach(MSADistanceAlgorithmFactory* algo, list) {
        delete algo;
    }
}

MSADistanceAlgorithmFactory* MSADistanceAlgorithmRegistry::getAlgorithmFactory(const QString& algoId) {
    return algorithms.value(algoId);
}


void MSADistanceAlgorithmRegistry::addAlgorithm(MSADistanceAlgorithmFactory* algo) {
    const QString& id = algo->getId();
    MSADistanceAlgorithmFactory* oldVersion = algorithms.value(id);
    if (oldVersion!=NULL) {
        delete oldVersion;
        oldVersion = NULL;
    }
    algorithms[id] = algo;
}

QStringList MSADistanceAlgorithmRegistry::getAlgorithmIds() const  {
    QList<MSADistanceAlgorithmFactory*> list = algorithms.values();
    QStringList result;
    foreach(MSADistanceAlgorithmFactory* algo, list) {
        result.append(algo->getId());
    }
    return result;
}

QList<MSADistanceAlgorithmFactory*> MSADistanceAlgorithmRegistry::getAlgorithmFactories(DistanceAlgorithmFlags flags) const {
    QList<MSADistanceAlgorithmFactory*> allFactories = getAlgorithmFactories();
    QList<MSADistanceAlgorithmFactory*> result;
    foreach(MSADistanceAlgorithmFactory* f, allFactories) {
        if ((flags & f->getFlags()) == flags) {
            result.append(f);
        }
    }
    return result;
}

}//namespace
