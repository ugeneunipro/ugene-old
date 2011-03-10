#ifndef _U2_MSA_DISTANCE_ALGORITHM_HAMMING_H_
#define _U2_MSA_DISTANCE_ALGORITHM_HAMMING_H_

#include "MSADistanceAlgorithm.h"
#include "BuiltInDistanceAlgorithms.h"

namespace U2 {

// Hamming algorithm is based on Hamming distance between sequences
class U2ALGORITHM_EXPORT MSADistanceAlgorithmFactoryHamming: public MSADistanceAlgorithmFactory {
    Q_OBJECT
public:
    MSADistanceAlgorithmFactoryHamming(QObject* p = NULL);
            
    virtual MSADistanceAlgorithm* createAlgorithm(const MAlignment& ma, QObject* parent);

    virtual QString getDescription() const;

    virtual QString getName() const;

};


class U2ALGORITHM_EXPORT MSADistanceAlgorithmHamming : public MSADistanceAlgorithm {
    Q_OBJECT
public:
    MSADistanceAlgorithmHamming(MSADistanceAlgorithmFactoryHamming* f, const MAlignment& ma)
        : MSADistanceAlgorithm(f, ma){}

    virtual void run();
};

}//namespace

#endif
