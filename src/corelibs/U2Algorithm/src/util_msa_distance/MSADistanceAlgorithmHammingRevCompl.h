#ifndef _U2_MSA_DISTANCE_ALGORITHM_HAMMING_COMPL_H_
#define _U2_MSA_DISTANCE_ALGORITHM_HAMMING_COMPL_H_

#include "MSADistanceAlgorithm.h"
#include "BuiltInDistanceAlgorithms.h"

namespace U2 {

// Hamming algorithm is based on Hamming distance between sequences
class U2ALGORITHM_EXPORT MSADistanceAlgorithmFactoryHammingRevCompl: public MSADistanceAlgorithmFactory {
    Q_OBJECT
public:
    MSADistanceAlgorithmFactoryHammingRevCompl(QObject* p = NULL);
            
    virtual MSADistanceAlgorithm* createAlgorithm(const MAlignment& ma, QObject* parent);

    virtual QString getDescription() const;

    virtual QString getName() const;

};


class U2ALGORITHM_EXPORT MSADistanceAlgorithmHammingRevCompl : public MSADistanceAlgorithm {
    Q_OBJECT
public:
    MSADistanceAlgorithmHammingRevCompl(MSADistanceAlgorithmFactoryHammingRevCompl* f, const MAlignment& ma)
        : MSADistanceAlgorithm(f, ma){}

    virtual void run();
};

}//namespace

#endif
