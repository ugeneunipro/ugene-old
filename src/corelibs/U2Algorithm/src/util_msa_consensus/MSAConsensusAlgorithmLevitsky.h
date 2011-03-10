#ifndef _U2_MSA_CONSENSUS_ALGORITHM_LEVITSKY_H_
#define _U2_MSA_CONSENSUS_ALGORITHM_LEVITSKY_H_

#include "MSAConsensusAlgorithm.h"
#include "BuiltInConsensusAlgorithms.h"

#include <QtCore/QVarLengthArray>

namespace U2 {

// DNA/RNA only consensus, that tries to
//    1) switch to extended nucleic alphabet when there is no 100% match
//    2) uses threshold score and selects the most rare (in whole alignment) symbol that has greater score
class U2ALGORITHM_EXPORT MSAConsensusAlgorithmFactoryLevitsky: public MSAConsensusAlgorithmFactory {
    Q_OBJECT
public:
    MSAConsensusAlgorithmFactoryLevitsky(QObject* p = NULL);
    
    virtual MSAConsensusAlgorithm* createAlgorithm(const MAlignment& ma, QObject* parent);

    virtual QString getDescription() const;

    virtual QString getName() const;

    virtual int getMinThreshold() const {return 50;}

    virtual int getMaxThreshold() const {return 100;}

    virtual int getDefaultThreshold() const {return 90;}

    virtual QString getThresholdSuffix() const {return QString("%");}

};

class U2ALGORITHM_EXPORT MSAConsensusAlgorithmLevitsky: public MSAConsensusAlgorithm {
    Q_OBJECT
public:
    MSAConsensusAlgorithmLevitsky(MSAConsensusAlgorithmFactoryLevitsky* f, const MAlignment& ma,  QObject* p = NULL);

    virtual char getConsensusChar(const MAlignment& ma, int column) const;

private:
    QVarLengthArray<int> globalFreqs;
};


}//namespace

#endif
