#ifndef _S3_KNOWN_MUTATIONS_UTILS_
#define _S3_KNOWN_MUTATIONS_UTILS_

#include <U2Core/KnownMutations.h>

#include <U2Core/U2Type.h>
#include <U2Core/U2OpStatusUtils.h>

namespace U2 {

class U2SequenceDbi;
class KnownMutationsDbi;

class U2CORE_EXPORT KnownMutationsUtils{
public:
    //return -1 if cannot convert
    static int getNumberByChrName(const QString& chrName);    

    static void referenceStatistics (const U2DataId& knownTrackId, KnownMutationsDbi* knownDbi, const U2DataId& seqId, U2SequenceDbi* seqDbi);
};
    
} //namespace

#endif //_S3_KNOWN_MUTATIONS_UTILS_
