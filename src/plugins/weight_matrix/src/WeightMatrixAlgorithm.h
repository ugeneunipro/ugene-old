#ifndef _U2_WEIGHT_MATRIX_ALGORITHM_H_
#define _U2_WEIGHT_MATRIX_ALGORITHM_H_

#include <U2Algorithm/PWMConversionAlgorithm.h>

#include <U2Core/DNATranslation.h>

namespace U2 {

enum MatrixBuldTarget {
    FREQUENCY_MATRIX,
    WEIGHT_MATRIX
};

enum MatrixBuildType {
    PM_MONONUCLEOTIDE,
    PM_DINUCLEOTIDE
};

class PMBuildSettings {
public:
    PMBuildSettings(): type(PM_MONONUCLEOTIDE), target(FREQUENCY_MATRIX) {
        
    }
    QString                     algo;
    MatrixBuildType             type;
    MatrixBuldTarget            target;
};

class WeightMatrixAlgorithm : public QObject {
    Q_OBJECT
public:
    static float getScore(const char* seq, int len, const PWMatrix& m, DNATranslation* complMap);
};

} //namespace

#endif