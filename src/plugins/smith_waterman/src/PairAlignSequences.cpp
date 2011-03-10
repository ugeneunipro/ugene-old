#include "PairAlignSequences.h"

namespace U2 {

void PairAlignSequences::setValues(int _score, U2Region const & _intervalSeq1) {
    score = _score;
    intervalSeq1 = _intervalSeq1;
}


PairAlignSequences::PairAlignSequences() {
    score = 0;
    isDNAComplemented = false;
    isAminoTranslated = false;
}

}//namespace
