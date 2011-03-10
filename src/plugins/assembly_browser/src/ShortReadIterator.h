#ifndef __SHORT_READ_ITERATOR_H__
#define __SHORT_READ_ITERATOR_H__

#include <U2Core/U2Assembly.h>

namespace U2 {

//TODO: enable handling for all CIGAR tokens. 
class ShortReadIterator {
public:
    ShortReadIterator(const QByteArray & read_, QList<U2CigarToken> cigar_, int startPos = 0); 

    bool hasNext() const;

    char nextLetter();

private:
    void advanceToNextToken();
    void skipInsertion();
    bool isInsertion();
    bool isDeletion();

    int offsetInRead;
    const QByteArray & read;

    int offsetInToken; // offset in token
    int offsetInCigar;  //
    QList<U2CigarToken> cigar;
};

} //ns
#endif 
