#ifndef _U2_BITS_TABLE_H_
#define _U2_BITS_TABLE_H_

#include <U2Core/DNAAlphabet.h>

namespace U2 {

class U2ALGORITHM_EXPORT BitsTable  {
public:
    quint32 NUCL_BITS[256];
    const static quint32 NUCL_BITS_SIZE;

    quint32 PROTEIN_BITS[256];
    const static quint32 PROTEIN_BITS_SIZE;

    quint32 RAW_BITS[256];
    const static quint32 RAW_BITS_SIZE;
    
    BitsTable();
    virtual ~BitsTable(){}

    quint32 getBitMaskCharBitsNum(DNAAlphabetType alType) const;
    const quint32* getBitMaskCharBits(DNAAlphabetType alType) const;

};

} //namespace

#endif
