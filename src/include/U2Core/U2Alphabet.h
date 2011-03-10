#ifndef _U2_ALPHABET_H_
#define _U2_ALPHABET_H_

#include <U2Core/global.h>

namespace U2 {

/** 
    Bio-sequence alphabet. 
    TODO: UGENE1.x DNAAlphabet impl must be moved here after refactoring
*/
class U2CORE_EXPORT U2AlphabetId {
public:
    U2AlphabetId(){}
    U2AlphabetId(QString aid) : id(aid){}

    QString id;

};

} // namespace

#endif
