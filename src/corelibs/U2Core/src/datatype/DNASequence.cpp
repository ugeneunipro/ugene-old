#include "DNASequence.h"

namespace U2 {
    
DNASequence::DNASequence(const QString& name, const QByteArray& s, DNAAlphabet* a) : seq(s), alphabet(a), circular(false) {
    if (!name.isEmpty()) {
        info.insert(DNAInfo::ID, name);
    }
}

void DNASequence::setName( const QString& name )
{
    info.insert(DNAInfo::ID, name);
}


} //namespace
