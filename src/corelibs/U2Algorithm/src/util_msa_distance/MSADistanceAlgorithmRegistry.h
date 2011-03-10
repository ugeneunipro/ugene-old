#ifndef _U2_MSA_DISTANCE_ALGORITHM_REGISTRY_H_
#define _U2_MSA_DISTANCE_ALGORITHM_REGISTRY_H_

#include "MSADistanceAlgorithm.h"

#include <QtCore/QStringList>

namespace U2 {

class MSADistanceAlgorithm;

class U2ALGORITHM_EXPORT MSADistanceAlgorithmRegistry : public QObject {
    Q_OBJECT

public:
    MSADistanceAlgorithmRegistry(QObject* p = NULL);

    ~MSADistanceAlgorithmRegistry();

    MSADistanceAlgorithmFactory* getAlgorithmFactory(const QString& algoId);

    void addAlgorithm(MSADistanceAlgorithmFactory* algo);

    QStringList getAlgorithmIds() const ;

    QList<MSADistanceAlgorithmFactory*> getAlgorithmFactories() const {return algorithms.values();}

    QList<MSADistanceAlgorithmFactory*> getAlgorithmFactories(DistanceAlgorithmFlags flags) const;

private:
    QMap<QString , MSADistanceAlgorithmFactory*> algorithms;
};

}//namespace

#endif
