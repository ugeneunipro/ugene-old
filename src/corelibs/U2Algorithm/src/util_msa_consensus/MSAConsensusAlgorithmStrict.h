#ifndef _U2_MSA_CONSENSUS_ALGORITHM_STRICT_H_
#define _U2_MSA_CONSENSUS_ALGORITHM_STRICT_H_

#include "MSAConsensusAlgorithm.h"
#include "BuiltInConsensusAlgorithms.h"

namespace U2 {

// Strict consensus: returns non-gap character only of all characters in the column are equal
class U2ALGORITHM_EXPORT MSAConsensusAlgorithmFactoryStrict: public MSAConsensusAlgorithmFactory {
    Q_OBJECT
public:
    MSAConsensusAlgorithmFactoryStrict(QObject* p = NULL);

    virtual MSAConsensusAlgorithm* createAlgorithm(const MAlignment& ma, QObject* parent);

    virtual QString getDescription() const;

    virtual QString getName() const;

    virtual int getMinThreshold() const {return 1;}

    virtual int getMaxThreshold() const {return 100;}

    virtual int getDefaultThreshold() const {return 100;}

    virtual QString getThresholdSuffix() const {return QString("%");}

};

class U2ALGORITHM_EXPORT MSAConsensusAlgorithmStrict : public MSAConsensusAlgorithm {
    Q_OBJECT
public:
    MSAConsensusAlgorithmStrict(MSAConsensusAlgorithmFactoryStrict* f, QObject* p = NULL) : MSAConsensusAlgorithm(f, p){}

    virtual char getConsensusChar(const MAlignment& ma, int column) const;
};


}//namespace

#endif
