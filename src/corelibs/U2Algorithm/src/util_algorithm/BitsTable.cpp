#include "BitsTable.h"

namespace U2 {

const quint32 BitsTable::NUCL_BITS_SIZE = 2;
const quint32 BitsTable::PROTEIN_BITS_SIZE = 5;
const quint32 BitsTable::RAW_BITS_SIZE = 8;

BitsTable::BitsTable () {
    for (int i = 256; --i >= 0;) {
        PROTEIN_BITS[i] = 0; NUCL_BITS[i] = 0; RAW_BITS[i] = i;
    }

    quint32 pBits= 1;
    PROTEIN_BITS['A'] = pBits++;
    PROTEIN_BITS['C'] = pBits++;
    PROTEIN_BITS['D'] = pBits++;
    PROTEIN_BITS['E'] = pBits++;
    PROTEIN_BITS['F'] = pBits++;
    PROTEIN_BITS['G'] = pBits++;
    PROTEIN_BITS['H'] = pBits++;
    PROTEIN_BITS['I'] = pBits++;
    PROTEIN_BITS['K'] = pBits++;
    PROTEIN_BITS['L'] = pBits++;
    PROTEIN_BITS['M'] = pBits++;
    PROTEIN_BITS['N'] = pBits++;
    PROTEIN_BITS['P'] = pBits++;
    PROTEIN_BITS['Q'] = pBits++;
    PROTEIN_BITS['R'] = pBits++;
    PROTEIN_BITS['S'] = pBits++;
    PROTEIN_BITS['T'] = pBits++;
    PROTEIN_BITS['V'] = pBits++;
    PROTEIN_BITS['W'] = pBits++;
    PROTEIN_BITS['Y'] = pBits++;
    PROTEIN_BITS['*'] = pBits;
    PROTEIN_BITS['+'] = pBits;
    PROTEIN_BITS['#'] = pBits;


    NUCL_BITS['A'] = 0x0; //00
    NUCL_BITS['C'] = 0x1; //01
    NUCL_BITS['T'] = 0x2; //10
    NUCL_BITS['G'] = 0x3; //11

    //todo: RNA?
}

quint32 BitsTable::getBitMaskCharBitsNum(DNAAlphabetType al) const {
    return al == DNAAlphabet_NUCL ? NUCL_BITS_SIZE : al == DNAAlphabet_AMINO ? PROTEIN_BITS_SIZE : RAW_BITS_SIZE;
}

const quint32* BitsTable::getBitMaskCharBits(DNAAlphabetType al) const {
    return al == DNAAlphabet_NUCL ? NUCL_BITS: al == DNAAlphabet_AMINO ? PROTEIN_BITS: RAW_BITS;
}

}//namespace

