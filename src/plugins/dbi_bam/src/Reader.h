#ifndef _U2_BAM_READER_H_
#define _U2_BAM_READER_H_

#include "Header.h"
#include "Alignment.h"
#include "BgzfReader.h"

namespace U2 {
namespace BAM {

class Reader
{
public:
    Reader(IOAdapter &ioAdapter);
    const Header &getHeader()const;
    Alignment readAlignment();
    bool isEof()const;
    VirtualOffset getOffset()const;
    void seek(VirtualOffset offset);
private:
    void readBytes(char *buffer, qint64 size);
    QByteArray readBytes(qint64 size);
    qint32 readInt32();
    quint32 readUint32();
    qint16 readInt16();
    quint16 readUint16();
    qint8 readInt8();
    quint8 readUint8();
    float readFloat32();
    char readChar();
    QByteArray readString();
    void readHeader();

    Header header;
    QHash<QByteArray, int> referencesMap;
    QHash<QByteArray, int> readGroupsMap;
    QHash<QByteArray, int> programsMap;
    IOAdapter &ioAdapter;
    BgzfReader reader;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_READER_H_
