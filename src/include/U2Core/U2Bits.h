#ifndef _U2_BITS_H_
#define _U2_BITS_H_

#include <U2Core/global.h>

#include <QtCore/QVector>
#include <QtCore/QByteArray>

namespace U2 {

class U2OpStatus;

/** 
    Utility class that can compress any string with a small alphabet size ( < 20-30 symbols types)
*/
class U2CORE_EXPORT U2BitCompression : public QObject {
public:    

    static QByteArray compress(const char* text, int uncompressedLen, int alphabetSize, const int* alphabetCharNums, U2OpStatus& os);

    static QByteArray uncompress(const char* data, const QByteArray& alphabetChars, U2OpStatus& os);

    static QVector<int> prepareCharNumsMask(const QByteArray& alphabetChars);

    static bool isCompressionNeeded(int textLen, int alphabetSize);

};

/** Bits manipulation helper */
class U2CORE_EXPORT U2Bits {
public:

    static int getBitInByte(int pos);

    static int getByteIndex(int bit);

    static int getNumberOfBytes(int nBits);
    
    static int getNumberOfBitsPerChar(int nChars);
        
    static QByteArray allocateBits(int nBits);

    static bool getBit(const uchar* bitSet, int idx);

    static void setBit(uchar* bits, int pos, bool val) { if (val) setBit(bits, pos); else clearBit(bits, pos);}
    
    static void setBit(uchar* bits, int pos);  

    static void clearBit(uchar* bits, int pos);

    static void setBits(uchar* dstBits, int pos, const uchar* srcBits, int nBits);        
       
    static int bitsRange2Int32(const uchar* bits, int pos, int len);

    static qint64 makeFirstNSignedBitsValue(int nBits);

    static qint8 readInt8(const uchar* bits, int pos);
    static void writeInt8(uchar* bits, int pos, qint8 val);
    
    static qint8 readInt16(const uchar* bits, int pos);
    static void writeInt16(uchar* bits, int pos, qint16 val);

    static qint8 readInt32(const uchar* bits, int pos);
    static void writeInt32(uchar* bits, int pos, qint32 val);
};

inline int U2Bits::getBitInByte(int pos) {
    return pos & 7; // use last 3 bits
}

inline int U2Bits::getByteIndex(int bit) {
    return (bit >> 3); // we store 8 bits (2^3) per word
}

inline int U2Bits::getNumberOfBytes(int nBits) {
    return nBits == 0 ? 0 : getByteIndex(nBits - 1) + 1;
} 

inline bool U2Bits::getBit(const uchar* bits, int idx) {
    const uchar* b = bits + + getByteIndex(idx);
    int bitInByte = getBitInByte(idx);
    bool res = bool(*b & uchar(1 << bitInByte));
    return res;
}

inline void U2Bits::setBit(uchar* bits, int pos) {
    int idx = getByteIndex(pos);
    uchar* b = bits + idx;
    int bitInByte = getBitInByte(pos);
    *b = *b | uchar(1 << bitInByte);
}

inline void U2Bits::clearBit(uchar* bits, int pos) {
    int idx = getByteIndex(pos);
    uchar* b = bits + idx;
    int bitInByte = getBitInByte(pos);
    *b = *b & ~uchar(1 << bitInByte);
}

inline qint64 U2Bits::makeFirstNSignedBitsValue(int nBits) {
    static qint64 allSignedBitsValue = 0xFFFFFFFFFFFFFFFFLL;
    qint64 res = allSignedBitsValue;
    res = res << nBits;
    return ~res;
}

} //namespace

#endif
