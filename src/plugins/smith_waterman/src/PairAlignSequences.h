#ifndef PAIRALIGNMENTSTRINGS_H
#define PAIRALIGNMENTSTRINGS_H

#include <U2Core/U2Region.h>

namespace U2 {

class PairAlignSequences {
public:
    PairAlignSequences();
    void setValues(int _score, U2Region const & _intervalSeq1);
    
    U2Region intervalSeq1;    
    int score;

    bool isAminoTranslated;
    bool isDNAComplemented;

    static const char UP = 'u';
    static const char LEFT = 'l';
    static const char DIAG = 'd';
    static const char GAP_CHARACTER = '_';
};

}//namespace

#endif
