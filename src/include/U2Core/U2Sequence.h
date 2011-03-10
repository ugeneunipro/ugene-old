#ifndef _U2_SEQUENCE_H_
#define _U2_SEQUENCE_H_

#include <U2Core/U2Type.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2Alphabet.h>

namespace U2 {

/** 
    Sequence representation. 
    'Length' field contains the overall length of all sequence parts.
*/
class U2CORE_EXPORT U2Sequence : public U2Object {
public:
    U2Sequence() : length (0), circular(false){}
    U2Sequence(U2DataId id, QString dbId, qint64 version) : U2Object(id, dbId, version), length(0){}
    
    /** Sequence alphabet id */
    U2AlphabetId    alphabet;
    
    /** Length of the sequence */
    qint64          length;
    
    /** A flag to mark that sequence is circular */
    bool            circular;
};

} // namespace

#endif
