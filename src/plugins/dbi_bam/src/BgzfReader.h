#ifndef _U2_BAM_BGZF_READER_H_
#define _U2_BAM_BGZF_READER_H_

//using 3rd-party zlib (not included in ugene bundle) on *nix
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
#include <zlib.h>
#else
#include "zlib.h"
#endif

#include <U2Core/IOAdapter.h>
#include "VirtualOffset.h"

namespace U2 {
namespace BAM {

class BgzfReader
{
public:
    BgzfReader(IOAdapter &ioAdapter);
    ~BgzfReader();

    qint64 read(char *buff, qint64 maxSize);
    qint64 skip(qint64 size);

    bool isEof()const;

    VirtualOffset getOffset()const;
    void seek(VirtualOffset offset);
private:
    void nextBlock();

    static const int BUFFER_SIZE = 16384;
    IOAdapter &ioAdapter;
    z_stream stream;
    char buffer[BUFFER_SIZE];
    quint64 headerOffset;
    bool endOfFile;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_BGZF_READER_H_
