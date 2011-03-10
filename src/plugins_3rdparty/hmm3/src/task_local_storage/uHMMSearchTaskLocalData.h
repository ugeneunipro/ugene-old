#ifndef _GB2_HMMER3_SEARCH_TL_DATA_H_
#define _GB2_HMMER3_SEARCH_TL_DATA_H_

#include <hmmer3/p7_config.h>

namespace U2 {

struct UHMM3SearchTaskLocalData {
    float flogsum_lookup[p7_LOGSUM_TBL];

    UHMM3SearchTaskLocalData();
}; // UHMM3SearchTaskLocalData

} // U2

#endif // _GB2_HMMER3_SEARCH_TL_DATA_H_
