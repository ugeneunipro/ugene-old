#include "PWMConversionAlgorithmBVH.h"

#include <math.h>

namespace U2 {

PWMConversionAlgorithmFactoryBVH::PWMConversionAlgorithmFactoryBVH(QObject* p) 
: PWMConversionAlgorithmFactory(BuiltInPWMConversionAlgorithms::BVH_ALGO, p)
{
}

PWMConversionAlgorithm* PWMConversionAlgorithmFactoryBVH::createAlgorithm(QObject* p){
    return new PWMConversionAlgorithmBVH(this, p);
}

QString PWMConversionAlgorithmFactoryBVH::getName() const {
    return "Berg and von Hippel";
}

QString PWMConversionAlgorithmFactoryBVH::getDescription() const {
    return tr("Berg and von Hippel weight function");
}

PWMatrix PWMConversionAlgorithmBVH::convert(const PFMatrix& matrix) {
    int size = (matrix.getType() == PFM_MONONUCLEOTIDE) ? 4 : 16;
    int length = matrix.getLength();
    QVarLengthArray<int> best(length);
    qMemSet(best.data(), 0, length*sizeof(int));
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < length; j++) {
            if (best[j] < matrix.getValue(i, j)) {
                best[j] = matrix.getValue(i, j);
            }
        }
    }
    
    QVarLengthArray<float> res(size*length);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < length; j++) {
            res[matrix.index(i, j)] = log((matrix.getValue(i, j) + 0.5) / (best[j] + 0.5));
        }
    }
    PWMatrix w(res, (matrix.getType() == PFM_MONONUCLEOTIDE) ? PWM_MONONUCLEOTIDE : PWM_DINUCLEOTIDE);
    w.setInfo(UniprobeInfo(matrix.getProperties()));
    return w;
}

PWMConversionAlgorithmBVH::PWMConversionAlgorithmBVH(PWMConversionAlgorithmFactory* factory, QObject* p)
: PWMConversionAlgorithm(factory, p)
{
}

}