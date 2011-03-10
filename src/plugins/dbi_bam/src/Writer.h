#ifndef _U2_BAM_WRITER_H_
#define _U2_BAM_WRITER_H_

#include <U2Core/IOAdapter.h>
#include "Header.h"
#include "Alignment.h"
#include "BgzfWriter.h"

namespace U2 {
namespace BAM {

class Writer
{
public:
    Writer(IOAdapter &ioAdapter);
    void writeHeader(const Header &header);
    void writeRead(const Alignment &alignment);
    void finish();
private:
    void writeBytes(const char *buffer, qint64 size);
    void writeBytes(const QByteArray &buffer);
    void writeInt32(qint32 value);
    void writeUint32(quint32 value);
    void writeInt16(qint16 value);
    void writeUint16(quint16 value);
    void writeInt8(qint8 value);
    void writeUint8(quint8 value);
    void writeFloat32(float value);
    void writeChar(char value);
    void writeString(const QByteArray &string);

    IOAdapter &ioAdapter;
    BgzfWriter writer;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_WRITER_H_
