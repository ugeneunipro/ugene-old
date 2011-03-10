#include "WeightMatrixAlgorithm.h"

#include <U2Core/DIProperties.h>

namespace U2 {
    
float WeightMatrixAlgorithm::getScore(const char* seq, int len, const PWMatrix& m, DNATranslation* complMap) {
    int l = m.getLength();
    
    float lower = m.getMinSum(), upper = m.getMaxSum();
    QByteArray complMapper = (complMap != NULL) ? complMap->getOne2OneMapper() : QByteArray();
    assert ((upper - lower) > 1e-9);
    float curr = 0;
    if (m.getType() == PWM_MONONUCLEOTIDE) {
        for (int i = 0; i < len && i < l; i++) {       
            char c = (complMap != NULL) ? complMapper[uchar(seq[i])] : seq[i];
            curr += m.getValue(DiProperty::index(c), i);
        }
    } else {
        for (int i = 0; i < len && i < l; i++) {       
            char c1 = (complMap != NULL) ? complMapper[uchar(seq[i])] : seq[i];
            char c2 = (complMap != NULL) ? complMapper[uchar(seq[i+1])] : seq[i+1];
            curr += m.getValue(DiProperty::index(c1, c2), i);
        }
    }
    assert (curr >= lower);
    assert (curr <= upper);
    return (curr - lower) / (upper - lower);
}

} //namespace