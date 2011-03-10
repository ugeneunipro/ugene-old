#ifndef _U2_MSA_CONSENSUS_ALGORITHM_CLUSTAL_H_
#define _U2_MSA_CONSENSUS_ALGORITHM_CLUSTAL_H_

#include "MSAConsensusAlgorithm.h"
#include "BuiltInConsensusAlgorithms.h"

namespace U2 {

class U2ALGORITHM_EXPORT MSAConsensusAlgorithmFactoryClustal : public MSAConsensusAlgorithmFactory {
    Q_OBJECT
public:
    MSAConsensusAlgorithmFactoryClustal(QObject* p = NULL) 
            : MSAConsensusAlgorithmFactory(BuiltInConsensusAlgorithms::CLUSTAL_ALGO, ConsensusAlgorithmFlags_AllAlphabets, p){}

    virtual MSAConsensusAlgorithm* createAlgorithm(const MAlignment& ma, QObject* parent);

    virtual QString getDescription() const;

    virtual QString getName() const;

    virtual int getMinThreshold() const {return 0;}

    virtual int getMaxThreshold() const {return 0;}

    virtual int getDefaultThreshold() const {return 0;}
};

class U2ALGORITHM_EXPORT MSAConsensusAlgorithmClustal : public MSAConsensusAlgorithm {
    Q_OBJECT
public:
    MSAConsensusAlgorithmClustal(MSAConsensusAlgorithmFactoryClustal* f, QObject* p = NULL) : MSAConsensusAlgorithm(f, p){}

    virtual char getConsensusChar(const MAlignment& ma, int column) const;
};

}//namespace

#endif
