#ifndef _U2_MSA_CONSENSUS_ALGORITHM_DEFAULT_H_
#define _U2_MSA_CONSENSUS_ALGORITHM_DEFAULT_H_

#include "MSAConsensusAlgorithm.h"
#include "BuiltInConsensusAlgorithms.h"

namespace U2 {

// Default algorithm is based on JalView experience. It also makes letters with low threshold lowercase
class U2ALGORITHM_EXPORT MSAConsensusAlgorithmFactoryDefault: public MSAConsensusAlgorithmFactory {
    Q_OBJECT
public:
    MSAConsensusAlgorithmFactoryDefault(QObject* p = NULL);
            
    virtual MSAConsensusAlgorithm* createAlgorithm(const MAlignment& ma, QObject* parent);

    virtual QString getDescription() const;

    virtual QString getName() const;

    virtual int getMinThreshold() const {return 1;}
    
    virtual int getMaxThreshold() const {return 100;}
    
    virtual int getDefaultThreshold() const {return 100;}

    virtual QString getThresholdSuffix() const {return QString("%");}

};


class U2ALGORITHM_EXPORT MSAConsensusAlgorithmDefault : public MSAConsensusAlgorithm {
    Q_OBJECT
public:
    MSAConsensusAlgorithmDefault(MSAConsensusAlgorithmFactoryDefault* f, QObject* p = NULL)
        : MSAConsensusAlgorithm(f, p){}

    virtual char getConsensusChar(const MAlignment& ma, int column) const {
        int countStub = 0;
        return getConsensusCharAndScore(ma, column, countStub);
    }

    virtual char getConsensusCharAndScore(const MAlignment& ma, int column, int& score) const;
};

}//namespace

#endif
