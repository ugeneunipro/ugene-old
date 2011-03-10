#ifndef _U2_BAM_BAI_READER_H_
#define _U2_BAM_BAI_READER_H_

#include <U2Core/IOAdapter.h>
#include "Index.h"

namespace U2 {
namespace BAM {

class BaiReader
{
public:
    BaiReader(IOAdapter &ioAdapter);
    Index readIndex();
private:
    void readBytes(char *buff, qint64 size);
    QByteArray readBytes(qint64 size);
    quint64 readUint64();
    qint32 readInt32();
    quint64 readUint32();

    IOAdapter &ioAdapter;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_BAI_READER_H_
