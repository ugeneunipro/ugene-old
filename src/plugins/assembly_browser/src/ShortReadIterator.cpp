#include "ShortReadIterator.h"


namespace U2 {

ShortReadIterator::ShortReadIterator(const QByteArray & read_, QList<U2CigarToken> cigar_, int startPos /*= 0*/) : 
offsetInRead(0), read(read_), offsetInToken(0), offsetInCigar(0), cigar(cigar_)  
{
    for(int i = 0; i < startPos && hasNext();) {
        skipInsertion();
        U2CigarToken t = cigar.at(offsetInCigar);
        if(i + t.count <= startPos) {
            if(!isDeletion()) {
                offsetInRead += t.count;
            }
            i += t.count;
            offsetInToken += t.count;
            if(!hasNext()) break;
            advanceToNextToken();
        } else {
            offsetInToken = startPos - i;
            if(!isDeletion()) {
                offsetInRead += offsetInToken;
            }
            break;
        }
    }
}

bool ShortReadIterator::hasNext() const {
    return ( offsetInCigar != cigar.size()-1 || offsetInToken != cigar.last().count );
}

char ShortReadIterator::nextLetter() {
    assert(hasNext());
    if(offsetInToken != cigar.at(offsetInCigar).count) {
        offsetInToken++;
    } else {
        advanceToNextToken();
        offsetInToken = 1;
    }
    bool del = isDeletion();
    char c = del ? '-' : read.at(offsetInRead); //TODO: hardcoded '-'
    offsetInRead += !del;
    return c;
}

void ShortReadIterator::advanceToNextToken() {
    offsetInToken = 0;
    offsetInCigar++;
    skipInsertion();
}

bool ShortReadIterator::isInsertion() {
    return U2CigarOp_I == cigar.at(offsetInCigar).op;
}

bool ShortReadIterator::isDeletion() {
    U2CigarOp op = cigar.at(offsetInCigar).op;
    return U2CigarOp_D == op || U2CigarOp_N == op;
}

void ShortReadIterator::skipInsertion() {
    while(hasNext() && isInsertion()) {
        offsetInRead += cigar.at(offsetInCigar).count;
        offsetInCigar++;
    }
}

} //ns
