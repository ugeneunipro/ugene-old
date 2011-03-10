#include <cmath>

#include "uHMMSearchTaskLocalData.h"

namespace U2 {

UHMM3SearchTaskLocalData::UHMM3SearchTaskLocalData() {
    int i = 0;
    for( i = 0; i < p7_LOGSUM_TBL; ++i ) {
        flogsum_lookup[i] = log(1. + exp((double) -i / p7_INTSCALE));
    }
}

} // U2
