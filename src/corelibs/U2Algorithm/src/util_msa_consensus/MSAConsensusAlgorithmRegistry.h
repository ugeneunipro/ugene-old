#ifndef _U2_MSA_CONSENSUS_ALGORITHM_REGISTRY_H_
#define _U2_MSA_CONSENSUS_ALGORITHM_REGISTRY_H_

#include "MSAConsensusAlgorithm.h"

#include <QtCore/QStringList>

namespace U2 {

class MSAConsensusAlgorithm;

class U2ALGORITHM_EXPORT MSAConsensusAlgorithmRegistry : public QObject {
    Q_OBJECT

public:
    MSAConsensusAlgorithmRegistry(QObject* p = NULL);

    ~MSAConsensusAlgorithmRegistry();

    MSAConsensusAlgorithmFactory* getAlgorithmFactory(const QString& algoId);

    void addAlgorithm(MSAConsensusAlgorithmFactory* algo);

    QStringList getAlgorithmIds() const ;

    QList<MSAConsensusAlgorithmFactory*> getAlgorithmFactories() const {return algorithms.values();}

    QList<MSAConsensusAlgorithmFactory*> getAlgorithmFactories(ConsensusAlgorithmFlags flags) const;

private:
    QMap<QString , MSAConsensusAlgorithmFactory*> algorithms;
};

}//namespace

#endif
