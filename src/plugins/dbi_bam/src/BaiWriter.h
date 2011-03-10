#ifndef _U2_BAM_BAI_WRITER_H_
#define _U2_BAM_BAI_WRITER_H_

#include <U2Core/IOAdapter.h>
#include "Index.h"

namespace U2 {
namespace BAM {

class BaiWriter
{
public:
    BaiWriter(IOAdapter &ioAdapter);
    void writeIndex(const Index &index);
private:
    void writeBytes(const char *buff, qint64 size);
    void writeBytes(const QByteArray &buffer);
    void writeUint64(quint64 value);
    void writeInt32(qint32 value);
    void writeUint32(quint32 value);

    IOAdapter &ioAdapter;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_BAI_WRITER_H_
